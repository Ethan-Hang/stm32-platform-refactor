/******************************************************************************
 * @file mpu.c
 *
 * @par dependencies
 * - mpu.h
 * - lv_port_mem_pool.h
 * - main.h (ST HAL: HAL_MPU_*)
 * - Debug.h
 *
 * @author Ethan-Hang
 *
 * @brief MPU guard bands around the LVGL heap pool.  See mpu.h for scope
 *        and rationale.
 *
 * Processing flow:
 *   main() -> mpu_protect_init()
 *               HAL_MPU_Disable
 *               region 0 <- guard band below pool, NO_ACCESS
 *               region 1 <- guard band above pool, NO_ACCESS
 *               HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT)   [sets MEMFAULTENA]
 *   ...
 *   stray write -> MemManage_Handler -> mpu_memmanage_report()
 *
 * @version V2.0 2026-08-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "mpu.h"
#include "main.h"
#include "lv_port_mem_pool.h"
#include "Debug.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/*
 * Region numbers.  Nothing else in this firmware programs the MPU, so the
 * two lowest slots are free.  Region priority only matters for overlap, and
 * these two overlap nothing but the PRIVDEFENA background map -- which is
 * always lowest priority, so the guards win.
 */
#define MPU_REGION_GUARD_LO         MPU_REGION_NUMBER0
#define MPU_REGION_GUARD_HI         MPU_REGION_NUMBER1

/* SCB->CFSR bits for the MemManage sub-register (MMFSR, CFSR[7:0]). */
#define MPU_MMFSR_MMARVALID         (1uL << 7)
#define MPU_MMFSR_MSTKERR           (1uL << 4)
#define MPU_MMFSR_MUNSTKERR         (1uL << 3)
#define MPU_MMFSR_DACCVIOL          (1uL << 1)
#define MPU_MMFSR_IACCVIOL          (1uL << 0)
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
volatile mpu_fault_info_t g_mpu_fault;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
/**
 * @brief      Program one 32 B NO_ACCESS guard region.
 *
 * @param[in]  number : MPU_REGION_NUMBERn slot to use.
 * @param[in]  base   : Guard base address; must be 32 B aligned.
 *
 * @return     None.
 *
 * @note       Memory attributes are set to normal, non-shareable, cacheable
 *             write-back -- matching how the rest of this SRAM is treated.
 *             With AccessPermission = NO_ACCESS they do not affect whether
 *             an access faults, but leaving them consistent with the
 *             surrounding SRAM avoids surprises if the region is ever
 *             relaxed for debugging.
 */
static void mpu_config_guard(uint8_t number, const void *base)
{
    MPU_Region_InitTypeDef init;

    init.Enable           = MPU_REGION_ENABLE;
    init.Number           = number;
    init.BaseAddress      = (uint32_t)base;
    init.Size             = MPU_REGION_SIZE_32B;
    init.AccessPermission = MPU_REGION_NO_ACCESS;
    init.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    init.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    init.IsCacheable      = MPU_ACCESS_CACHEABLE;
    init.IsBufferable     = MPU_ACCESS_BUFFERABLE;
    init.TypeExtField     = MPU_TEX_LEVEL0;
    init.SubRegionDisable = 0x00;

    HAL_MPU_ConfigRegion(&init);
}

void mpu_protect_init(void)
{
    const void *guard_lo = lv_port_mem_pool_guard_lo();
    const void *guard_hi = lv_port_mem_pool_guard_hi();

    HAL_MPU_Disable();

    mpu_config_guard(MPU_REGION_GUARD_LO, guard_lo);
    mpu_config_guard(MPU_REGION_GUARD_HI, guard_hi);

    /* PRIVILEGED_DEFAULT keeps the default memory map as the background
     * region, so only the two guard bands change behaviour.  This call also
     * sets SCB_SHCSR_MEMFAULTENA, without which a guard hit would escalate
     * straight to HardFault and lose MMFAR. */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

    DEBUG_OUT(i, MPU_LOG_TAG,
              "LVGL pool guarded: lo=0x%08lX pool=0x%08lX..0x%08lX hi=0x%08lX",
              (unsigned long)(uint32_t)guard_lo,
              (unsigned long)(uint32_t)lv_port_mem_pool_base(),
              (unsigned long)((uint32_t)lv_port_mem_pool_base() +
                              lv_port_mem_pool_size()),
              (unsigned long)(uint32_t)guard_hi);
}

void mpu_memmanage_report(void)
{
    uint32_t cfsr  = SCB->CFSR;
    uint32_t mmfar = SCB->MMFAR;
    uint32_t base  = (uint32_t)lv_port_mem_pool_base();
    uint32_t size  = lv_port_mem_pool_size();

    uint32_t zone = MPU_FAULT_ZONE_UNKNOWN;
    if (0u == (cfsr & MPU_MMFSR_MMARVALID))
    {
        /* Stacking/unstacking faults (MSTKERR/MUNSTKERR) do not update
         * MMFAR, so the address field must not be trusted here. */
        zone  = MPU_FAULT_ZONE_NO_ADDRESS;
        mmfar = 0u;
    }
    else if ((mmfar >= (base - LV_PORT_MEM_GUARD_SIZE)) && (mmfar < base))
    {
        zone = MPU_FAULT_ZONE_GUARD_LO;
    }
    else if ((mmfar >= (base + size)) &&
             (mmfar < (base + size + LV_PORT_MEM_GUARD_SIZE)))
    {
        zone = MPU_FAULT_ZONE_GUARD_HI;
    }

    /* Latch first, log second: if the RTT write below wedges, Ozone can
     * still read the whole story out of g_mpu_fault. */
    g_mpu_fault.count++;
    g_mpu_fault.cfsr      = cfsr;
    g_mpu_fault.mmfar     = mmfar;
    g_mpu_fault.zone      = zone;
    g_mpu_fault.pool_base = base;
    g_mpu_fault.pool_size = size;

    /* Best-effort logging from fault context.  elog's output lock is a
     * no-op in this port (elog_port.c) and SEGGER_RTT_Write masks
     * interrupts internally, so this cannot deadlock -- worst case it
     * interleaves with a log line that was mid-format when we faulted. */
    switch (zone)
    {
        case MPU_FAULT_ZONE_GUARD_LO:
            DEBUG_OUT(e, MPU_ERR_LOG_TAG,
                      "LVGL pool UNDERRUN at 0x%08lX (%ld B below pool base "
                      "0x%08lX) cfsr=0x%08lX",
                      (unsigned long)mmfar, (long)(base - mmfar),
                      (unsigned long)base, (unsigned long)cfsr);
            break;
        case MPU_FAULT_ZONE_GUARD_HI:
            DEBUG_OUT(e, MPU_ERR_LOG_TAG,
                      "LVGL pool OVERRUN at 0x%08lX (%ld B past pool end "
                      "0x%08lX) cfsr=0x%08lX",
                      (unsigned long)mmfar,
                      (long)(mmfar - (base + size)),
                      (unsigned long)(base + size), (unsigned long)cfsr);
            break;
        case MPU_FAULT_ZONE_NO_ADDRESS:
            DEBUG_OUT(e, MPU_ERR_LOG_TAG,
                      "MemManage without valid MMFAR, cfsr=0x%08lX%s%s",
                      (unsigned long)cfsr,
                      (cfsr & MPU_MMFSR_MSTKERR) ? " MSTKERR" : "",
                      (cfsr & MPU_MMFSR_MUNSTKERR) ? " MUNSTKERR" : "");
            break;
        default:
            DEBUG_OUT(e, MPU_ERR_LOG_TAG,
                      "MemManage at 0x%08lX outside the LVGL guards, "
                      "cfsr=0x%08lX%s%s",
                      (unsigned long)mmfar, (unsigned long)cfsr,
                      (cfsr & MPU_MMFSR_DACCVIOL) ? " DACCVIOL" : "",
                      (cfsr & MPU_MMFSR_IACCVIOL) ? " IACCVIOL" : "");
            break;
    }

    /* Clear the sticky MMFSR bits so a later fault reads clean. */
    SCB->CFSR = cfsr & 0xFFuL;
}

void mpu_hardfault_report(void)
{
    uint32_t hfsr = SCB->HFSR;

    /* FORCED means a configurable fault could not be taken and escalated
     * here.  Only then is the MemManage status meaningful; a genuine
     * HardFault (bus error on vector fetch, etc.) leaves MMFSR clear and
     * falls through to the plain report below. */
    if ((0u != (hfsr & SCB_HFSR_FORCED_Msk)) && (0u != (SCB->CFSR & 0xFFuL)))
    {
        mpu_memmanage_report();
        return;
    }

    DEBUG_OUT(e, MPU_ERR_LOG_TAG,
              "HardFault hfsr=0x%08lX cfsr=0x%08lX (not an LVGL guard hit)",
              (unsigned long)hfsr, (unsigned long)SCB->CFSR);
}
//******************************* Functions *********************************//

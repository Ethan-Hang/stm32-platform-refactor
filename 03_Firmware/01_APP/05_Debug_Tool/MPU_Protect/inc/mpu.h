/******************************************************************************
 * @file mpu.h
 *
 * @par dependencies
 * - stdint.h
 *
 * @author Ethan-Hang
 *
 * @brief MPU guard bands around the LVGL heap pool.
 *
 *        The LVGL pool is the one large, constantly-churned allocation
 *        arena in this firmware, and a write off either end of it lands in
 *        an unrelated .bss object -- corrupting it silently and surfacing
 *        much later as a HardFault with no usable provenance.  This module
 *        marks the 32 B guard band on each side of the pool (owned by
 *        lv_port_mem_pool.c) MPU_REGION_NO_ACCESS, converting that class of
 *        bug into a MemManage fault raised on the offending instruction,
 *        with the address in SCB->MMFAR.
 *
 *        Everything outside the two guard bands keeps the default memory
 *        map: mpu_protect_init() enables the MPU with PRIVDEFENA, so no
 *        other access pattern in the firmware changes behaviour.
 *
 * @note  Scope, stated plainly: the guards catch accesses that leave the
 *        pool. They do NOT bound-check individual lv_mem_alloc blocks --
 *        one LVGL allocation overrunning into the next stays inside the
 *        pool and is invisible here.  They also only see CPU accesses; the
 *        MPU does not police DMA.
 *
 * @version V2.0 2026-08-14
 * @upgrade 2.0: Rewritten.  V1 marked a 32 B region at 0x20000000 -- the
 *               base of .data, i.e. live variables -- as NO_ACCESS, and was
 *               never called or compiled (absent from app_sources.cmake,
 *               and mpu.h declared no prototype for MPU_Config).
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __MPU_H__
#define __MPU_H__

//******************************** Includes *********************************//
#include <stdint.h>
//******************************** Includes *********************************//

#ifdef __cplusplus
extern "C" {
#endif

//******************************** Defines **********************************//
/* Where a MemManage fault landed, as classified by mpu_memmanage_report(). */
#define MPU_FAULT_ZONE_UNKNOWN      (0u)   /* not one of our guard bands   */
#define MPU_FAULT_ZONE_GUARD_LO     (1u)   /* underrun below the pool      */
#define MPU_FAULT_ZONE_GUARD_HI     (2u)   /* overrun above the pool       */
#define MPU_FAULT_ZONE_NO_ADDRESS   (3u)   /* MMFAR not valid (see CFSR)   */
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
/**
 * Last MemManage fault, latched for post-mortem inspection in Ozone even
 * when the RTT log did not make it out.  `count` distinguishes "never
 * faulted" (0) from a latched fault, and counts re-entries.
 */
typedef struct
{
    uint32_t count;      /* number of MemManage faults taken so far        */
    uint32_t cfsr;       /* SCB->CFSR as latched (MMFSR is the low byte)   */
    uint32_t mmfar;      /* faulting address, valid only if MMARVALID      */
    uint32_t zone;       /* MPU_FAULT_ZONE_*                               */
    uint32_t pool_base;  /* pool bounds at fault time, for offline decode  */
    uint32_t pool_size;
} mpu_fault_info_t;

extern volatile mpu_fault_info_t g_mpu_fault;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
/**
 * @brief      Arm the guard bands and enable the MPU.
 *
 *             Must be called after startup has zero-filled .bss (the guards
 *             live there) and before LVGL touches the pool.  Enables the
 *             MPU with MPU_PRIVILEGED_DEFAULT, so all memory outside the
 *             two guard regions keeps the default map.
 *
 * @return     None.  A failure to configure a region cannot be recovered
 *             from here; the regions are compile-time constant, so any
 *             error is a programming error caught in bring-up.
 */
void mpu_protect_init(void);

/**
 * @brief      Latch and report a MemManage fault.  Called from
 *             MemManage_Handler; does not return a verdict -- the handler
 *             halts afterwards.
 *
 * @return     None.
 */
void mpu_memmanage_report(void);

/**
 * @brief      Handle a HardFault, forwarding to mpu_memmanage_report() when
 *             it turns out to be an escalated MemManage (HFSR.FORCED with a
 *             non-zero MMFSR).  Called from HardFault_Handler.
 *
 *             Escalation happens whenever configurable faults are masked at
 *             the moment of the violation -- notably inside the
 *             __disable_irq() windows in MCU_Core_IFlash -- so without this
 *             path a guard hit there would be indistinguishable from any
 *             other crash.
 *
 * @return     None.
 */
void mpu_hardfault_report(void);
//******************************* Functions *********************************//

#ifdef __cplusplus
}
#endif

#endif /* __MPU_H__ */

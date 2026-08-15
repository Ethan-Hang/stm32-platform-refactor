/******************************************************************************
 * @file mpu_selftest.c
 *
 * @par dependencies
 * - mpu_selftest.h
 * - mpu.h
 * - main.h (ST HAL: HAL_MPU_Disable, HAL_Delay, CMSIS MPU/SCB)
 * - lv_port_mem_pool.h
 * - Debug.h
 *
 * @author Ethan-Hang
 *
 * @brief Fault-injection self-test for the LVGL pool MPU guard bands.
 *        See mpu_selftest.h for scope, the recovery trick, and how to run
 *        it.  Whole file compiles away when MPU_SELFTEST_ENABLE is 0.
 *
 * @version V1.0 2026-08-15
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "mpu_selftest.h"

#if MPU_SELFTEST_ENABLE

#include "mpu.h"
#include "main.h"
#include "lv_port_mem_pool.h"
#include "Debug.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/* What the runner expects of the case currently in flight. */
#define MPU_SELFTEST_EXPECT_NONE        (0u)  /* not testing: handler halts */
#define MPU_SELFTEST_EXPECT_FAULT       (1u)  /* a guard hit is the point   */
#define MPU_SELFTEST_EXPECT_NO_FAULT    (2u)  /* negative control           */

/* SCB->CFSR bit needed to tell a data violation from a stacking fault. */
#define MPU_SELFTEST_MMFSR_DACCVIOL     (1uL << 1)

/*
 * RBAR reads back the base address plus the region number; the VALID bit is
 * write-only.  RASR is fully determined by mpu_config_guard(): XN=1,
 * AP=0b000 (NO_ACCESS), TEX=0, S=0, C=1, B=1, SRD=0, SIZE=4 (2^5 = 32 B),
 * ENABLE=1.  Comparing against the literal catches a silently mis-encoded
 * region -- notably the case where a non-32 B-aligned base bleeds into the
 * REGION field and reprograms a slot other than the intended one.
 */
#define MPU_SELFTEST_RBAR_ADDR_MASK     (0xFFFFFFE0uL)
#define MPU_SELFTEST_RBAR_REGION_MASK   (0x0000000FuL)
#define MPU_SELFTEST_EXPECTED_RASR      (0x10030009uL)

/*
 * Milliseconds to idle after each log line.  RTT terminal 0 shares a 2 KB
 * up-buffer that drops on overflow, and this test emits its whole report in
 * a tight burst at boot; pausing lets the viewer's poll drain it.
 *
 * HAL_Delay() is safe this early only because the HAL time base is TIM1
 * (stm32f4xx_hal_timebase_tim.c overrides HAL_InitTick, and HAL_Init()
 * starts it), so uwTick advances independently of the scheduler.  It would
 * hang if the time base were SysTick: this project's SysTick_Handler is
 * the CMSIS-RTOS one, which never calls HAL_IncTick().
 */
#define MPU_SELFTEST_LOG_DRAIN_MS       (30u)
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
volatile mpu_selftest_result_t g_mpu_selftest;

/*
 * File-level state shared with the fault handlers.  No locking: the test
 * runs from main() before osKernelStart(), and the only other context that
 * touches these is the fault handler for the access this file just issued.
 */
static volatile uint32_t g_expect  = MPU_SELFTEST_EXPECT_NONE;
static volatile uint32_t g_faulted = 0u;
static volatile uint32_t g_lastHfsr = 0u;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
/**
 * @brief      Tally one case and log its verdict.
 *
 * @param[in]  case_id : MPU_SELFTEST_CASE_*, also the failed_mask bit.
 * @param[in]  name    : Short case label for the log line.
 * @param[in]  pass    : Non-zero when every expectation was met.
 *
 * @param[out] None.
 *
 * @return     None.
 */
static void mpu_selftest_score(uint32_t case_id, const char *name,
                               uint32_t pass)
{
    /**
     * Latch into the result struct before logging, for the same reason
     * mpu_memmanage_report() does: RTT may never reach a host.
     **/
    g_mpu_selftest.ran++;
    if (0u != pass)
    {
        g_mpu_selftest.passed++;
    }
    else
    {
        g_mpu_selftest.failed_mask |= (1uL << case_id);
    }

    DEBUG_OUT(i, MPU_LOG_TAG, "selftest[%lu] %s: %s",
              (unsigned long)case_id, name,
              (0u != pass) ? "PASS" : "FAIL");
    HAL_Delay(MPU_SELFTEST_LOG_DRAIN_MS);
}

/**
 * @brief      Restore MPU protection after a recovered fault.
 *
 * @param[in]  None.
 *
 * @param[out] None.
 *
 * @return     None.
 *
 * @note       mpu_selftest_fault_recover() disables the MPU to let the
 *             faulting instruction retire, so every case that actually
 *             faulted must re-arm before the next one runs.  Cases that did
 *             not fault leave the MPU armed and skip the (logging) re-init.
 */
static void mpu_selftest_rearm(void)
{
    if (0u != g_faulted)
    {
        mpu_protect_init();
    }
}

/**
 * @brief      Read one guard region back out of the MPU and check it.
 *
 * @param[in]  number : Region slot to select via MPU->RNR.
 * @param[in]  base   : Guard base address the slot should be holding.
 *
 * @param[out] None.
 *
 * @return     Non-zero when base, region number and attributes all match.
 */
static uint32_t mpu_selftest_region_ok(uint32_t number, uint32_t base)
{
    uint32_t rbar;
    uint32_t rasr;

    /**
     * RNR selects which region RBAR/RASR alias; it must be written before
     * either is read.
     **/
    MPU->RNR = number;
    rbar     = MPU->RBAR;
    rasr     = MPU->RASR;

    DEBUG_OUT(i, MPU_LOG_TAG,
              "selftest region%lu rbar=0x%08lX rasr=0x%08lX",
              (unsigned long)number, (unsigned long)rbar,
              (unsigned long)rasr);

    return (uint32_t)(((rbar & MPU_SELFTEST_RBAR_ADDR_MASK) == base) &&
                      ((rbar & MPU_SELFTEST_RBAR_REGION_MASK) == number) &&
                      (MPU_SELFTEST_EXPECTED_RASR == rasr));
}

/**
 * @brief      Case CONFIG: the MPU is on, backgrounded, and both guard
 *             regions hold what mpu_protect_init() claimed to write.
 *
 * @param[in]  None.
 *
 * @param[out] None.
 *
 * @return     None.
 */
static void mpu_selftest_case_config(void)
{
    uint32_t ctrl_want = MPU_CTRL_ENABLE_Msk | MPU_CTRL_PRIVDEFENA_Msk;
    uint32_t pass;

    /**
     * Without MEMFAULTENA a guard hit escalates to HardFault and loses its
     * MMFAR, so the whole reporting path silently degrades -- check it here
     * rather than inferring it from case MASKED.
     **/
    pass = (uint32_t)((ctrl_want == (MPU->CTRL & ctrl_want)) &&
                      (0u != (SCB->SHCSR & SCB_SHCSR_MEMFAULTENA_Msk)));

    pass &= mpu_selftest_region_ok(
        0u, (uint32_t)lv_port_mem_pool_guard_lo());
    pass &= mpu_selftest_region_ok(
        1u, (uint32_t)lv_port_mem_pool_guard_hi());

    mpu_selftest_score(MPU_SELFTEST_CASE_CONFIG, "config", pass);
}

/**
 * @brief      Case POOL_RW: both ends of the pool itself stay accessible.
 *
 * @param[in]  None.
 *
 * @param[out] None.
 *
 * @return     None.
 *
 * @note       This is the control that catches a guard placed one region
 *             too low or too high -- which would otherwise look like a
 *             perfectly healthy set of guard hits in the cases below.
 *             Runs before lv_init(), so the two bytes are scratch; they are
 *             restored anyway to keep the pool byte-identical.
 */
static void mpu_selftest_case_pool_rw(void)
{
    volatile uint8_t *pool = (volatile uint8_t *)lv_port_mem_pool_base();
    uint32_t last          = lv_port_mem_pool_size() - 1u;
    uint32_t before        = g_mpu_fault.count;
    uint8_t saved_first    = pool[0];
    uint8_t saved_last     = pool[last];
    uint32_t pass;

    g_faulted = 0u;
    g_expect  = MPU_SELFTEST_EXPECT_NO_FAULT;

    pool[0]    = 0x5Au;
    pool[last] = 0xA5u;

    pass = (uint32_t)((0x5Au == pool[0]) && (0xA5u == pool[last]) &&
                      (before == g_mpu_fault.count) && (0u == g_faulted));

    g_expect   = MPU_SELFTEST_EXPECT_NONE;
    pool[0]    = saved_first;
    pool[last] = saved_last;
    mpu_selftest_rearm();

    mpu_selftest_score(MPU_SELFTEST_CASE_POOL_RW, "pool ends rw", pass);
}

/**
 * @brief      Case OUTSIDE: the byte just outside each guard is untouched.
 *
 * @param[in]  None.
 *
 * @param[out] None.
 *
 * @return     None.
 *
 * @note       Proves each guard covers 32 B and not more -- an oversized or
 *             misaligned region would fault here and turn healthy
 *             neighbours in .bss into no-go memory.  Reads only: these two
 *             addresses belong to whatever the linker parked either side of
 *             s_block, and writing them would corrupt it.
 */
static void mpu_selftest_case_outside(void)
{
    const uint8_t *lo  = (const uint8_t *)lv_port_mem_pool_guard_lo();
    const uint8_t *hi  = (const uint8_t *)lv_port_mem_pool_guard_hi();
    uint32_t before    = g_mpu_fault.count;
    uint32_t pass;

    g_faulted = 0u;
    g_expect  = MPU_SELFTEST_EXPECT_NO_FAULT;

    (void)*(volatile const uint8_t *)(lo - 1);
    (void)*(volatile const uint8_t *)(hi + LV_PORT_MEM_GUARD_SIZE);

    pass = (uint32_t)((before == g_mpu_fault.count) && (0u == g_faulted));

    g_expect = MPU_SELFTEST_EXPECT_NONE;
    mpu_selftest_rearm();

    mpu_selftest_score(MPU_SELFTEST_CASE_OUTSIDE, "guard width", pass);
}

/**
 * @brief      Shared body for the cases that must trap: touch @p addr and
 *             check how mpu_memmanage_report() classified the hit.
 *
 * @param[in]  case_id  : MPU_SELFTEST_CASE_* being run.
 * @param[in]  name     : Short case label for the log line.
 * @param[in]  addr     : Guard-band address to step on.
 * @param[in]  is_write : Non-zero to store, zero to load.
 * @param[in]  exp_zone : MPU_FAULT_ZONE_* the report is expected to pick.
 *
 * @param[out] None.
 *
 * @return     None.
 */
static void mpu_selftest_case_hit(uint32_t case_id, const char *name,
                                  volatile uint8_t *addr,
                                  uint32_t is_write, uint32_t exp_zone)
{
    uint32_t before = g_mpu_fault.count;
    uint32_t pass;

    g_faulted = 0u;
    g_expect  = MPU_SELFTEST_EXPECT_FAULT;

    if (0u != is_write)
    {
        *addr = 0xAAu;
    }
    else
    {
        (void)*addr;
    }

    g_expect = MPU_SELFTEST_EXPECT_NONE;
    mpu_selftest_rearm();

    /**
     * MMFAR must name the exact byte touched, not just "somewhere in the
     * guard": that is what makes the overrun distance in the error log
     * trustworthy.
     **/
    pass = (uint32_t)((0u != g_faulted) &&
                      ((before + 1u) == g_mpu_fault.count) &&
                      (exp_zone == g_mpu_fault.zone) &&
                      ((uint32_t)addr == g_mpu_fault.mmfar) &&
                      (0u != (g_mpu_fault.cfsr &
                              MPU_SELFTEST_MMFSR_DACCVIOL)));

    DEBUG_OUT(i, MPU_LOG_TAG,
              "selftest %s: zone=%lu mmfar=0x%08lX cfsr=0x%08lX",
              name, (unsigned long)g_mpu_fault.zone,
              (unsigned long)g_mpu_fault.mmfar,
              (unsigned long)g_mpu_fault.cfsr);

    mpu_selftest_score(case_id, name, pass);
}

/**
 * @brief      Case MASKED: a guard hit taken with PRIMASK set escalates to
 *             HardFault and is still classified correctly.
 *
 * @param[in]  None.
 *
 * @param[out] None.
 *
 * @return     None.
 *
 * @note       This is the MCU_Core_IFlash scenario: MemManage is a
 *             configurable fault, so __disable_irq() turns any guard hit
 *             inside those windows into a HardFault.  Passing here is what
 *             makes mpu_hardfault_report()'s HFSR.FORCED path real rather
 *             than theoretical.
 */
static void mpu_selftest_case_masked(void)
{
    volatile uint8_t *over  = (volatile uint8_t *)lv_port_mem_pool_base() +
                              lv_port_mem_pool_size();
    uint32_t before         = g_mpu_fault.count;
    uint32_t pass;

    g_faulted   = 0u;
    g_lastHfsr  = 0u;
    g_expect    = MPU_SELFTEST_EXPECT_FAULT;

    __disable_irq();
    *over = 0xC3u;
    __enable_irq();

    g_expect = MPU_SELFTEST_EXPECT_NONE;
    mpu_selftest_rearm();
    g_mpu_selftest.hfsr = g_lastHfsr;

    pass = (uint32_t)((0u != g_faulted) &&
                      ((before + 1u) == g_mpu_fault.count) &&
                      (MPU_FAULT_ZONE_GUARD_HI == g_mpu_fault.zone) &&
                      ((uint32_t)over == g_mpu_fault.mmfar) &&
                      (0u != (g_lastHfsr & SCB_HFSR_FORCED_Msk)));

    DEBUG_OUT(i, MPU_LOG_TAG,
              "selftest masked: zone=%lu mmfar=0x%08lX hfsr=0x%08lX",
              (unsigned long)g_mpu_fault.zone,
              (unsigned long)g_mpu_fault.mmfar,
              (unsigned long)g_lastHfsr);

    mpu_selftest_score(MPU_SELFTEST_CASE_MASKED, "masked hit", pass);
}

uint32_t mpu_selftest_fault_recover(void)
{
    if (MPU_SELFTEST_EXPECT_NONE == g_expect)
    {
        return 0u;
    }

    /**
     * One-shot: clearing the expectation here means a second, unplanned
     * fault halts in the handler as it normally would, instead of the
     * runner silently papering over it.
     **/
    g_expect   = MPU_SELFTEST_EXPECT_NONE;
    g_faulted  = 1u;

    /**
     * HFSR is write-1-to-clear and is not touched by mpu_hardfault_report();
     * latch it for case MASKED, then clear so the next case starts clean.
     **/
    g_lastHfsr = SCB->HFSR;
    SCB->HFSR  = g_lastHfsr;

    /**
     * Dropping MPU protection is what lets the exception return retire the
     * faulting instruction instead of re-faulting forever.  It re-executes
     * against a guard byte, which nothing in the firmware ever reads, and
     * mpu_selftest_rearm() puts the guards back before the next case.
     **/
    HAL_MPU_Disable();
    __DSB();
    __ISB();

    return 1u;
}

void mpu_selftest_run(void)
{
    volatile uint8_t *pool = (volatile uint8_t *)lv_port_mem_pool_base();
    uint32_t size          = lv_port_mem_pool_size();

    DEBUG_OUT(i, MPU_LOG_TAG, "selftest start (fault injection follows)");
    HAL_Delay(MPU_SELFTEST_LOG_DRAIN_MS);

    mpu_selftest_case_config();
    mpu_selftest_case_pool_rw();
    mpu_selftest_case_outside();

    /**
     * One byte below the pool and one byte past its end are the two
     * off-by-one overruns the guards exist to catch; the third reaches the
     * far end of the high guard by a load, proving reads trap too and that
     * the region spans its full 32 B.
     **/
    mpu_selftest_case_hit(MPU_SELFTEST_CASE_UNDERRUN, "underrun",
                          pool - 1, 1u, MPU_FAULT_ZONE_GUARD_LO);
    mpu_selftest_case_hit(MPU_SELFTEST_CASE_OVERRUN, "overrun",
                          pool + size, 1u, MPU_FAULT_ZONE_GUARD_HI);
    mpu_selftest_case_hit(MPU_SELFTEST_CASE_READ_HI, "overrun read",
                          pool + size + LV_PORT_MEM_GUARD_SIZE - 1u, 0u,
                          MPU_FAULT_ZONE_GUARD_HI);

    mpu_selftest_case_masked();

    DEBUG_OUT(i, MPU_LOG_TAG,
              "selftest done: %lu/%lu passed, failed_mask=0x%02lX, "
              "faults=%lu",
              (unsigned long)g_mpu_selftest.passed,
              (unsigned long)g_mpu_selftest.ran,
              (unsigned long)g_mpu_selftest.failed_mask,
              (unsigned long)g_mpu_fault.count);
    HAL_Delay(MPU_SELFTEST_LOG_DRAIN_MS);
}
//******************************* Functions *********************************//

#else /* !MPU_SELFTEST_ENABLE */

/* ISO C forbids an empty translation unit; nothing else needs this. */
typedef int mpu_selftest_disabled_t;

#endif /* MPU_SELFTEST_ENABLE */

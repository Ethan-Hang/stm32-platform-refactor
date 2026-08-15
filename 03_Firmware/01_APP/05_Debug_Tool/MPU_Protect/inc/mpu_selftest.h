/******************************************************************************
 * @file mpu_selftest.h
 *
 * @par dependencies
 * - stdint.h
 *
 * @author Ethan-Hang
 *
 * @brief Fault-injection self-test for the LVGL pool MPU guard bands.
 *
 *        The guard bands have no legitimate access path, so "never fired"
 *        is indistinguishable from "never armed".  This module proves the
 *        mechanism end to end by deliberately stepping on the guards and
 *        checking that mpu_memmanage_report() classified each hit the way
 *        it is supposed to -- including the negative controls, which prove
 *        the guards do not extend into the pool or beyond their 32 B.
 *
 *        Every fault case is recovered from, so one boot runs the whole
 *        matrix instead of one case per flash cycle.  Recovery is done by
 *        disabling the MPU inside the fault handler and returning: the
 *        faulting instruction re-executes, this time succeeding against a
 *        guard byte (which nothing else ever reads), and the runner then
 *        re-arms via mpu_protect_init().  No stacked-PC surgery, so no
 *        Thumb-2 instruction-width decoding to get wrong.
 *
 * Processing flow:
 *   main() -> mpu_protect_init() -> mpu_selftest_run()
 *               per case: arm expectation -> touch address
 *                 -> MemManage_Handler / HardFault_Handler
 *                      -> mpu_*_report()          (latches g_mpu_fault)
 *                      -> mpu_selftest_fault_recover()  (MPU off, return)
 *                 -> re-arm, compare, log PASS/FAIL
 *
 * @note  DEBUG ONLY.  MPU_SELFTEST_ENABLE is 0 in the shipping build, which
 *        compiles both entry points down to empty static inlines -- the
 *        fault handlers then keep their original bare while(1) shape.  Turn
 *        it on, flash, read RTT terminal 4, turn it back off.
 *
 * @note  Runs from main() before the scheduler starts, so it is
 *        single-threaded by construction and needs no locking around its
 *        file-level state.
 *
 * @version V1.0 2026-08-15
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __MPU_SELFTEST_H__
#define __MPU_SELFTEST_H__

//******************************** Includes *********************************//
#include <stdint.h>
//******************************** Includes *********************************//

#ifdef __cplusplus
extern "C" {
#endif

//******************************** Defines **********************************//
/**
 * Master switch.  0 = shipping build: no test code, no extra .bss, and the
 * fault handlers behave exactly as they did before this module existed.
 */
#define MPU_SELFTEST_ENABLE         0

/* Case numbers, also the bit positions used in `failed_mask`. */
#define MPU_SELFTEST_CASE_CONFIG    (0u)  /* region registers read back    */
#define MPU_SELFTEST_CASE_POOL_RW   (1u)  /* pool ends stay writable       */
#define MPU_SELFTEST_CASE_OUTSIDE   (2u)  /* guards are exactly 32 B wide  */
#define MPU_SELFTEST_CASE_UNDERRUN  (3u)  /* write one byte below the pool */
#define MPU_SELFTEST_CASE_OVERRUN   (4u)  /* write one byte past the pool  */
#define MPU_SELFTEST_CASE_READ_HI   (5u)  /* reads trap too, at guard end  */
#define MPU_SELFTEST_CASE_MASKED    (6u)  /* hit inside __disable_irq()    */

#define MPU_SELFTEST_CASE_COUNT     (7u)
//******************************** Defines **********************************//

#if MPU_SELFTEST_ENABLE

//******************************* Declaring *********************************//
/**
 * Verdict, latched for Ozone so the result survives an RTT buffer overflow
 * or a viewer that was not attached when the test ran.
 */
typedef struct
{
    uint32_t ran;          /* cases executed                               */
    uint32_t passed;       /* cases that met every expectation             */
    uint32_t failed_mask;  /* bit MPU_SELFTEST_CASE_* set = that case failed*/
    uint32_t hfsr;         /* SCB->HFSR from the masked (escalated) case   */
} mpu_selftest_result_t;

extern volatile mpu_selftest_result_t g_mpu_selftest;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
/**
 * @brief      Run the whole guard-band matrix and log a PASS/FAIL line per
 *             case plus a summary, on RTT terminal 4.
 *
 * @param[in]  None.
 *
 * @param[out] None.  Results also land in g_mpu_selftest.
 *
 * @return     None.
 *
 * @note       Must be called immediately after mpu_protect_init(), before
 *             anything touches the LVGL pool.  Leaves the MPU armed exactly
 *             as mpu_protect_init() left it, whatever the verdict.
 */
void mpu_selftest_run(void);

/**
 * @brief      Fault-handler hook: recover from an expected guard hit.
 *
 * @param[in]  None.
 *
 * @param[out] None.
 *
 * @return     1 when the fault was expected and the handler may return
 *             (the MPU has been disabled, so the faulting instruction will
 *             re-execute successfully); 0 when it was not, in which case
 *             the caller must halt as usual.
 */
uint32_t mpu_selftest_fault_recover(void);
//******************************* Functions *********************************//

#else /* !MPU_SELFTEST_ENABLE */

/* Shipping build: both hooks vanish, so callers need no #if of their own. */
static inline void mpu_selftest_run(void)
{
}

static inline uint32_t mpu_selftest_fault_recover(void)
{
    return 0u;
}

#endif /* MPU_SELFTEST_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* __MPU_SELFTEST_H__ */

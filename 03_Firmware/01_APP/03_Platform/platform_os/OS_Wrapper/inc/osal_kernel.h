/******************************************************************************
 * @file osal_kernel.h
 *
 * @par dependencies
 * - osal_common_types.h
 *
 * @author Ethan-Hang
 *
 * @brief OSAL kernel bring-up interfaces.
 *
 * @note These two calls bracket application task creation, which is why they
 *       are separate: every backend needs its object machinery alive before
 *       a task can be created, and the scheduler must start only once the
 *       tasks exist. main() therefore runs
 *
 *           osal_kernel_init();     <- heap, scheduler and timer subsystems
 *           app_task_init();        <- driver init + the bootstrap task
 *           osal_kernel_start();    <- idle thread, tick source, scheduling
 *
 *       and contains no RTOS-specific symbol of its own.
 *
 * @version V1.0 2026-8-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __OSAL_KERNEL_H__
#define __OSAL_KERNEL_H__

//******************************** Includes *********************************//
#include "osal_common_types.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief Bring up the RTOS so that objects can be created.
 *
 * Must be called after the clock tree and peripherals are configured and
 * before any other OSAL call. Creating a task, queue or semaphore before this
 * returns is undefined.
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_ERROR.
 */
int32_t osal_kernel_init(void);

/**
 * @brief Hand control to the scheduler.
 *
 * Does not return while the firmware is healthy: from here on the RTOS owns
 * the CPU. A return means the scheduler could not be started.
 *
 * @return OSAL_ERROR; there is no success path that returns.
 */
int32_t osal_kernel_start(void);

//******************************* Functions *********************************//

#endif /* __OSAL_KERNEL_H__ */

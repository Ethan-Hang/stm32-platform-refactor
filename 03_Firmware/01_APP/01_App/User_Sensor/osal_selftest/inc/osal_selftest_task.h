/******************************************************************************
 * @file osal_selftest_task.h
 *
 * @author Ethan-Hang
 *
 * @brief Backend-agnostic OSAL conformance test task.
 *
 * @note Enabled through USER_TASK_OSAL_SELFTEST in user_task_reso_config.h.
 *       Off by default; it is a bring-up aid, not a production task.
 *
 * @version V1.0 2026-8-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __OSAL_SELFTEST_TASK_H__
#define __OSAL_SELFTEST_TASK_H__

//******************************* Functions *********************************//
/**
 * @brief Run the OSAL conformance cases and print a PASS/FAIL summary to RTT.
 *
 * Never returns: idles once the summary has been emitted.
 *
 * @param[in] p_arg Unused.
 */
void osal_selftest_thread(void *p_arg);
//******************************* Functions *********************************//

#endif /* __OSAL_SELFTEST_TASK_H__ */

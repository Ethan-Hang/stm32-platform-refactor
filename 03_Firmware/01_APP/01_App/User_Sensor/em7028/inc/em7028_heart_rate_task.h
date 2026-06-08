/******************************************************************************
 * @file em7028_heart_rate_task.h
 *
 * @author Ethan-Hang
 *
 * @brief  EM7028 PPG heart-rate task entry + latest-result accessor.
 *
 * @version V1.0 2026-06-08
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/
#ifndef __EM7028_HEART_RATE_TASK_H__
#define __EM7028_HEART_RATE_TASK_H__

//******************************** Includes *********************************//
#include "platform_type.h"
//******************************** Includes *********************************//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      EM7028 heart-rate calculation task entry.
 *
 * @param[in]  argument : Unused.
 */
void em7028_heart_rate_task(void *argument);

/**
 * @brief      Read the most recent heart-rate algorithm output.
 *
 *             Lock-free latest-wins read of the task's volatile publish globals.
 *             Safe to call from any thread; bpm/conf may be from adjacent
 *             frames in a torn read, which is acceptable for a periodic display.
 *
 * @param[out] bpm  : Latest BPM (pass NULL to skip).
 * @param[out] conf : Latest confidence 0-100 (pass NULL to skip).
 *
 * @return     TRUE if the task has produced at least one frame; FALSE otherwise.
 */
BOOL_T em7028_hr_get_latest(UINT16_T *bpm, UINT8_T *conf);

#ifdef __cplusplus
}
#endif

#endif /* __EM7028_HEART_RATE_TASK_H__ */

/******************************************************************************
 * @file osal_config.h
 *
 * @par dependencies
 * - osal_common_types.h
 *
 * @author Ethan-Hang
 *
 * @brief OSAL compile-time configuration definitions.
 *
 * @version V1.0 2026-4-9
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once 
#ifndef __OSAL_CONFIG_H__
#define __OSAL_CONFIG_H__
//******************************** Includes *********************************//
#include "osal_common_types.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
/* FREERTOS_SUPPORT / RTTHREAD_SUPPORT and the OSAL_RTOS_SUPPORT selector come
 * from osal_common_types.h; only values that need the backend's own config
 * headers are derived here. */

#if (OSAL_RTOS_SUPPORT == FREERTOS_SUPPORT)
#include "FreeRTOSConfig.h"
/**
 * @brief Number of priority levels the OSAL scale spans, backend-defined.
 *
 * OSAL keeps the FreeRTOS convention that a larger number means a higher
 * priority. The RT-Thread implementation inverts it internally, because
 * RT-Thread numbers priorities the other way round.
 */
#define OSAL_PRIORITY_MAX (configMAX_PRIORITIES)
#elif (OSAL_RTOS_SUPPORT == RTTHREAD_SUPPORT)
#include "rtconfig.h"
/**
 * @brief Same span as the FreeRTOS backend's configMAX_PRIORITIES, so PRI_*
 *        resolve to identical numbers on both and the builds stay comparable.
 *
 * RT-Thread supports only 8 / 32 / 256 levels, so RT_THREAD_PRIORITY_MAX is
 * 256 and merely has to be wide enough to hold this scale; os_impl_task.c
 * asserts that at compile time.
 */
#define OSAL_PRIORITY_MAX (56)
#else
#error "OSAL_RTOS_SUPPORT must be FREERTOS_SUPPORT or RTTHREAD_SUPPORT"
#endif

/**
 * @brief Platform ISR context probe for Cortex-M targets.
 *
 * Host-side unit tests and non-ARM targets fall back to 0U.
 */
#if defined(__ARM_ARCH_6M__) || defined(__ARM_ARCH_7M__) ||                \
	defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_8M_BASE__) ||          \
	defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8_1M_MAIN__)
#include "cmsis_gcc.h"
#define OSAL_PLATFORM_IS_IN_ISR() (__get_IPSR() != 0U)
#else
#define OSAL_PLATFORM_IS_IN_ISR() (0U)
#endif

//******************************** Defines **********************************//

#endif // __OSAL_CONFIG_H__

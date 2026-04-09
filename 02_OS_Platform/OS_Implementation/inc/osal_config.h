/******************************************************************************
 * @file osal_config.h
 *
 * @par dependencies
 * - common_types.h
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

#ifndef __OSAL_CONFIG_H__
#define __OSAL_CONFIG_H__
//******************************** Includes *********************************//
#include "common_types.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
/**
 * @brief Enable FreeRTOS as the RTOS backend.
 */
#define FREERTOS_SUPPORT (1)

/**
 * @brief Select the active RTOS backend used by OSAL.
 */
#define OSAL_RTOS_SUPPORT (FREERTOS_SUPPORT)

//******************************** Defines **********************************//

#endif // __OSAL_CONFIG_H__

/******************************************************************************
 * @file osal_internal_task.h
 *
 * @par dependencies
 * - osal_task.h
 *
 * @author Ethan-Hang
 *
 * @brief OSAL internal task implementation interfaces.
 *
 * @version V1.0 2026-4-9
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __OSAL_INTERNAL_TASK_H__
#define __OSAL_INTERNAL_TASK_H__

//******************************** Includes *********************************//
#include "common_types.h"

//******************************** Includes *********************************//

//******************************* Declaring *********************************//
typedef void (*osal_task_entry_t)(void *p_arg);
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
/**
 * @brief Create task object.
 *
 * @param[out] p_task_handle Output task handle.
 * @param[in] p_task_name Task name.
 * @param[in] p_arg Task argument.
 * @param[in] task_entry Task entry function.
 * @param[in] stack_depth Task stack depth.
 * @param[in] priority Task priority.
 *
 * @return OSAL status code.
 */
int32_t osal_task_create_impl(osal_task_handle_t *p_task_handle,
                         const char *p_task_name, 
                         void *p_arg, 
                         osal_task_entry_t task_entry,
                         uint32_t stack_depth, 
                         uint32_t priority);

/**
 * @brief Delete task object.
 *
 * @param[in] task_handle Task handle.
 */
void osal_task_delete_impl   (osal_task_handle_t task_handle);

/**
 * @brief Delay current task.
 *
 * @param[in] ticks_to_delay Delay ticks.
 */
void osal_task_delay_impl    (osal_tick_type_t ticks_to_delay);

/**
 * @brief Yield current task.
 */
void osal_task_yield_impl    (void);

/**
 * @brief Get current OS tick count.
 *
 * @return Current OSAL tick count.
 */
osal_tick_type_t osal_task_get_tick_count_impl(void);
//******************************* Functions *********************************//

#endif /* __OSAL_INTERNAL_TASK_H__ */

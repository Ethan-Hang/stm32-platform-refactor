/******************************************************************************
 * @file osal_task.c
 *
 * @par dependencies
 * - osal_task.h
 * - osal_internal_task.h
 *
 * @author Ethan-Hang
 *
 * @brief OSAL task wrapper interfaces.
 *
 * @version V1.0 2026-4-9
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "osal_task.h"
#include "osal_internal_task.h"
#include "osal_error.h"

//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief Create task object.
 *
 * @param[out] p_task_handle Output task handle.
 * @param[in] p_task_name Task name.
 * @param[in] p_arg Task entry argument.
 * @param[in] task_entry Task entry function.
 * @param[in] stack_depth Task stack depth.
 * @param[in] priority Task priority.
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_INVALID_POINTER,
 *         OSAL_ERR_INVALID_SIZE or lower layer error code.
 */
int32_t osal_task_create(osal_task_handle_t *p_task_handle,
                         const char *p_task_name, 
                         void *p_arg, 
                         osal_task_entry_t task_entry,
                         uint32_t stack_depth, 
                         uint32_t priority)
{
    if ((NULL == p_task_handle) || (NULL == p_task_name) ||
        (NULL == task_entry))
    {
        return OSAL_INVALID_POINTER;
    }

    if (0U == stack_depth)
    {
        return OSAL_ERR_INVALID_SIZE;
    }

    return osal_task_create_impl(p_task_handle, p_task_name, p_arg, task_entry,
                                 stack_depth, priority);
}

/**
 * @brief Delete task object.
 *
 * @param[in] task_handle Task handle.
 */
void osal_task_delete(osal_task_handle_t task_handle)
{
    if (NULL == task_handle)
    {
        return;
    }

    osal_task_delete_impl(task_handle);
}

/**
 * @brief Delay current task.
 *
 * @param[in] ticks_to_delay Delay ticks.
 */
void osal_task_delay(osal_tick_type_t ticks_to_delay)
{
    osal_task_delay_impl(ticks_to_delay);
}

/**
 * @brief Yield current task.
 */
void osal_task_yield(void)
{
    osal_task_yield_impl();
}

/**
 * @brief Get current OS tick count.
 *
 * @return Current OSAL tick count.
 */
osal_tick_type_t osal_task_get_tick_count(void)
{
    return osal_task_get_tick_count_impl();
}

//******************************* Functions *********************************//

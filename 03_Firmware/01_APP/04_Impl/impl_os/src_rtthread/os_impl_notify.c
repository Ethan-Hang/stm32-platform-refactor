/******************************************************************************
 * @file os_impl_notify.c
 *
 * @par dependencies
 * - osal_internal_notify.h
 * - osal_error.h
 * - os_rtthread.h
 *
 * @author Ethan-Hang
 *
 * @brief OSAL task notification implementation for RT-Thread.
 *
 * @note RT-Thread has no per-thread notification value, so each OSAL task
 *       carries an rt_event as its notification slot: embedded in the task
 *       block for static tasks, heap-allocated on first use for dynamic ones.
 *       The slot is always reached through rt_thread.user_data, so threads
 *       RT-Thread created for itself are never mistaken for OSAL tasks.
 *
 *       rt_event is a pure bit set, which covers OSAL_NOTIFY_SET_BITS -- the
 *       only action the project uses (ISR raises a bit, handler task waits on
 *       it). The counting and value-passing actions have no equivalent and
 *       report OSAL_ERR_NOT_IMPLEMENTED rather than silently degrading to a
 *       bit set, which would look like it worked.
 *
 * @version V1.0 2026-8-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "board_types.h"
#include "osal_internal_notify.h"
#include "osal_error.h"
#include "osal_macros.h"

#include "os_rtthread.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief Send a task notification from an ISR.
 *
 * @param[in] task_handle Handle of the task to notify.
 * @param[in] value Notification value to send.
 * @param[in] action Action to perform on the receiving task's value.
 * @param[out] p_higher_priority_task_woken Optional wakeup flag from ISR.
 *
 * @return OSAL_SUCCESS on success, OSAL_ERROR when not in ISR or the task has
 *         no notification slot, OSAL_ERR_NOT_IMPLEMENTED for an action
 *         rt_event cannot express.
 */
INT32_t osal_notify_send_from_isr_impl(
    osal_task_handle_t task_handle,
    UINT32_t value,
    osal_notify_action_t action,
    osal_base_type_t *p_higher_priority_task_woken)
{
    struct rt_event *p_event;
    rt_err_t         result;

    if (!OSAL_IS_IN_ISR())
    {
        return OSAL_ERROR;
    }

    /* rt_event_send is ISR-safe and reschedules on interrupt exit itself. */
    if (RT_NULL != p_higher_priority_task_woken)
    {
        *p_higher_priority_task_woken = OSAL_FALSE;
    }

    if (OSAL_NOTIFY_SET_BITS != action)
    {
        return OSAL_ERR_NOT_IMPLEMENTED;
    }

    /* create == RT_FALSE: allocating from an ISR is not allowed. A static
       task always has its slot; a dynamic one gets it on its first wait, so
       by the time an ISR notifies it the slot exists. */
    p_event = osal_rtt_notify_slot((rt_thread_t)task_handle, RT_FALSE);
    if (RT_NULL == p_event)
    {
        return OSAL_ERROR;
    }

    result = rt_event_send(p_event, (rt_uint32_t)value);
    if (RT_EOK == result)
    {
        return OSAL_SUCCESS;
    }

    return OSAL_ERROR;
}

/**
 * @brief Wait for a task notification in task context.
 *
 * @param[in] bits_to_clear_on_entry Bits to clear before waiting.
 * @param[in] bits_to_clear_on_exit Bits to clear once satisfied.
 * @param[out] p_notification_value Optional pointer receiving the value.
 * @param[in] timeout Wait timeout in OSAL ticks.
 *
 * @return OSAL_SUCCESS on success, OSAL_ERR_IN_ISR in ISR context,
 *         OSAL_ERROR when no slot could be obtained, otherwise
 *         OSAL_NOTIFY_TIMEOUT.
 */
INT32_t osal_notify_wait_impl(UINT32_t bits_to_clear_on_entry,
                              UINT32_t bits_to_clear_on_exit,
                              UINT32_t *p_notification_value,
                              osal_tick_type_t timeout)
{
    struct rt_event *p_event;
    rt_uint32_t      recved = 0U;
    rt_uint32_t      wanted;
    rt_uint8_t       option;
    rt_err_t         result;
    rt_base_t        level;

    if (OSAL_IS_IN_ISR())
    {
        return OSAL_ERR_IN_ISR;
    }

    p_event = osal_rtt_notify_slot(rt_thread_self(), RT_TRUE);
    if (RT_NULL == p_event)
    {
        return OSAL_ERROR;
    }

    if (0U != bits_to_clear_on_entry)
    {
        level = rt_hw_interrupt_disable();
        p_event->set &= ~(rt_uint32_t)bits_to_clear_on_entry;
        rt_hw_interrupt_enable(level);
    }

    /* xTaskNotifyWait waits for any notification at all, so wait on every
       bit with OR. bits_to_clear_on_exit names what to consume; when it is
       empty the value is left pending, matching the FreeRTOS behaviour. */
    wanted = (0U != bits_to_clear_on_exit) ? (rt_uint32_t)bits_to_clear_on_exit
                                           : (rt_uint32_t)0xFFFFFFFFU;
    option = RT_EVENT_FLAG_OR;
    if (0U != bits_to_clear_on_exit)
    {
        option |= RT_EVENT_FLAG_CLEAR;
    }

    result = rt_event_recv(p_event,
                           wanted,
                           option,
                           osal_rtt_timeout(timeout),
                           &recved);

    if (RT_NULL != p_notification_value)
    {
        *p_notification_value = (UINT32_t)recved;
    }

    if (RT_EOK == result)
    {
        return OSAL_SUCCESS;
    }

    return OSAL_NOTIFY_TIMEOUT;
}

//******************************* Functions *********************************//

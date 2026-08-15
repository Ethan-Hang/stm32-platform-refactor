/******************************************************************************
 * @file os_impl_event_group.c
 *
 * @par dependencies
 * - osal_internal_event_group.h
 * - osal_error.h
 * - os_rtthread.h
 *
 * @author Ethan-Hang
 *
 * @brief OSAL event group implementation based on RT-Thread event APIs.
 *
 * @version V1.0 2026-8-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "board_types.h"
#include "osal_internal_event_group.h"
#include "osal_error.h"
#include "osal_macros.h"

#include "os_rtthread.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief Create an event group object.
 *
 * @param[out] p_handle Output event group handle.
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_EVT_GROUP_FAILURE.
 */
INT32_t osal_event_group_create_impl(osal_event_group_handle_t *p_handle)
{
    rt_event_t handle;

    handle = rt_event_create("osal_evt", RT_IPC_FLAG_FIFO);
    if (RT_NULL == handle)
    {
        return OSAL_EVT_GROUP_FAILURE;
    }

    *p_handle = (osal_event_group_handle_t)handle;
    return OSAL_SUCCESS;
}

/**
 * @brief Delete an event group object.
 *
 * @param[in] handle Event group handle.
 */
void osal_event_group_delete_impl(osal_event_group_handle_t handle)
{
    if (OSAL_IS_IN_ISR())
    {
        return;
    }

    (void)rt_event_delete((rt_event_t)handle);
}

/**
 * @brief Set bits in an event group.
 *
 * @param[in] handle Event group handle.
 * @param[in] bits Bit mask to set.
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_EVT_GROUP_FAILURE.
 */
INT32_t osal_event_group_set_bits_impl(osal_event_group_handle_t handle,
                                       UINT32_t bits)
{
    rt_err_t result;

    result = rt_event_send((rt_event_t)handle, (rt_uint32_t)bits);
    if (RT_EOK == result)
    {
        return OSAL_SUCCESS;
    }

    return OSAL_EVT_GROUP_FAILURE;
}

/**
 * @brief Clear bits in an event group.
 *
 * @param[in] handle Event group handle.
 * @param[in] bits Bit mask to clear.
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_EVT_GROUP_FAILURE.
 */
INT32_t osal_event_group_clear_bits_impl(osal_event_group_handle_t handle,
                                         UINT32_t bits)
{
    rt_event_t p_event = (rt_event_t)handle;
    rt_base_t  level;

    if (RT_NULL == p_event)
    {
        return OSAL_EVT_GROUP_FAILURE;
    }

    /* RT-Thread clears bits only as a side effect of rt_event_recv with
       RT_EVENT_FLAG_CLEAR, and offers no standalone clear. Touching the set
       directly is the equivalent of xEventGroupClearBits; interrupts are
       masked because rt_event_send() modifies the same word. */
    level = rt_hw_interrupt_disable();
    p_event->set &= ~(rt_uint32_t)bits;
    rt_hw_interrupt_enable(level);

    return OSAL_SUCCESS;
}

/**
 * @brief Wait for bits in an event group.
 *
 * @param[in] handle Event group handle.
 * @param[in] bits_to_wait Bit mask to wait for.
 * @param[in] clear_on_exit Clear the matched bits before returning.
 * @param[in] wait_for_all Wait for every bit rather than any one of them.
 * @param[in] timeout Wait timeout in OSAL ticks.
 * @param[out] p_bits_set Optional pointer receiving the bits that matched.
 *
 * @return OSAL_SUCCESS on success, OSAL_ERR_IN_ISR in ISR context,
 *         otherwise OSAL_ERROR_TIMEOUT.
 */
INT32_t osal_event_group_wait_bits_impl(osal_event_group_handle_t handle,
                                        UINT32_t bits_to_wait,
                                        BOOL clear_on_exit,
                                        BOOL wait_for_all,
                                        osal_tick_type_t timeout,
                                        UINT32_t *p_bits_set)
{
    rt_uint8_t  option;
    rt_uint32_t recved = 0U;
    rt_err_t    result;

    if (OSAL_IS_IN_ISR())
    {
        return OSAL_ERR_IN_ISR;
    }

    option = (wait_for_all) ? RT_EVENT_FLAG_AND : RT_EVENT_FLAG_OR;
    if (clear_on_exit)
    {
        option |= RT_EVENT_FLAG_CLEAR;
    }

    result = rt_event_recv((rt_event_t)handle,
                           (rt_uint32_t)bits_to_wait,
                           option,
                           osal_rtt_timeout(timeout),
                           &recved);

    if (RT_NULL != p_bits_set)
    {
        *p_bits_set = (UINT32_t)recved;
    }

    if (RT_EOK == result)
    {
        return OSAL_SUCCESS;
    }

    return OSAL_ERROR_TIMEOUT;
}

/**
 * @brief Set bits in an event group from ISR context.
 *
 * @param[in] handle Event group handle.
 * @param[in] bits Bit mask to set.
 * @param[out] p_higher_priority_task_woken Optional wakeup flag from ISR.
 *
 * @return OSAL_SUCCESS on success, OSAL_ERROR when not in ISR,
 *         otherwise OSAL_EVT_GROUP_FAILURE.
 */
INT32_t osal_event_group_set_bits_from_isr_impl(
    osal_event_group_handle_t handle,
    UINT32_t bits,
    osal_base_type_t *p_higher_priority_task_woken)
{
    rt_err_t result;

    if (!OSAL_IS_IN_ISR())
    {
        return OSAL_ERROR;
    }

    /* rt_event_send is ISR-safe and arranges its own reschedule on interrupt
       exit, so there is never a deferred yield to hand back. */
    if (RT_NULL != p_higher_priority_task_woken)
    {
        *p_higher_priority_task_woken = OSAL_FALSE;
    }

    result = rt_event_send((rt_event_t)handle, (rt_uint32_t)bits);
    if (RT_EOK == result)
    {
        return OSAL_SUCCESS;
    }

    return OSAL_EVT_GROUP_FAILURE;
}

//******************************* Functions *********************************//

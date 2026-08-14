/******************************************************************************
 * @file os_impl_queue.c
 *
 * @par dependencies
 * - osal_internal_queue.h
 * - osal_error.h
 * - os_rtthread.h
 *
 * @author Ethan-Hang
 *
 * @brief OSAL queue implementation based on RT-Thread message queue APIs.
 *
 * @version V1.0 2026-8-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "board_types.h"
#include "osal_internal_queue.h"
#include "osal_error.h"
#include "osal_macros.h"

#include "os_rtthread.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief Create a queue object.
 *
 * @param[out] p_queue_handle Output queue handle.
 * @param[in] queue_depth Number of items the queue can hold.
 * @param[in] item_size Size of one item in bytes.
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_ERROR.
 */
INT32_t osal_queue_create_impl(osal_queue_handle_t *p_queue_handle,
                               SIZE_t queue_depth,
                               SIZE_t item_size)
{
    rt_mq_t queue_handle;

    queue_handle = rt_mq_create("osal_q",
                                (rt_size_t)item_size,
                                (rt_size_t)queue_depth,
                                RT_IPC_FLAG_FIFO);
    if (RT_NULL == queue_handle)
    {
        return OSAL_ERROR;
    }

    *p_queue_handle = (osal_queue_handle_t)queue_handle;
    return OSAL_SUCCESS;
}

/**
 * @brief Delete a queue object.
 *
 * @param[in] queue_handle Queue handle.
 */
void osal_queue_delete_impl(osal_queue_handle_t queue_handle)
{
    if (OSAL_IS_IN_ISR())
    {
        return;
    }

    (void)rt_mq_delete((rt_mq_t)queue_handle);
}

/**
 * @brief Send an item to the back of the queue in task context.
 *
 * @param[in] queue_handle Queue handle.
 * @param[in] p_data Item to copy into the queue.
 * @param[in] timeout Wait timeout in OSAL ticks when the queue is full.
 *
 * @return OSAL_SUCCESS on success, OSAL_ERR_IN_ISR in ISR context,
 *         otherwise OSAL_QUEUE_FULL.
 */
INT32_t osal_queue_send_impl(osal_queue_handle_t queue_handle,
                             const void *p_data,
                             osal_tick_type_t timeout)
{
    rt_mq_t  p_queue = (rt_mq_t)queue_handle;
    rt_err_t result;

    if (OSAL_IS_IN_ISR())
    {
        return OSAL_ERR_IN_ISR;
    }

    result = rt_mq_send_wait(p_queue,
                             p_data,
                             p_queue->msg_size,
                             osal_rtt_timeout(timeout));
    if (RT_EOK == result)
    {
        return OSAL_SUCCESS;
    }

    return OSAL_QUEUE_FULL;
}

/**
 * @brief Receive an item from the queue in task context.
 *
 * @param[in] queue_handle Queue handle.
 * @param[out] p_data Buffer receiving one item.
 * @param[in] timeout Wait timeout in OSAL ticks when the queue is empty.
 *
 * @return OSAL_SUCCESS on success, OSAL_ERR_IN_ISR in ISR context,
 *         otherwise OSAL_QUEUE_TIMEOUT.
 */
INT32_t osal_queue_receive_impl(osal_queue_handle_t queue_handle,
                                void *p_data,
                                osal_tick_type_t timeout)
{
    rt_mq_t  p_queue = (rt_mq_t)queue_handle;
    rt_err_t result;

    if (OSAL_IS_IN_ISR())
    {
        return OSAL_ERR_IN_ISR;
    }

    result = rt_mq_recv(p_queue,
                        p_data,
                        p_queue->msg_size,
                        osal_rtt_timeout(timeout));
    if (RT_EOK == result)
    {
        return OSAL_SUCCESS;
    }

    return OSAL_QUEUE_TIMEOUT;
}

/**
 * @brief Send an item to the back of the queue from ISR context.
 *
 * @param[in] queue_handle Queue handle.
 * @param[in] p_data Item to copy into the queue.
 * @param[out] p_higher_priority_task_woken Optional wakeup flag from ISR.
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_QUEUE_FULL.
 */
INT32_t osal_queue_send_from_isr_impl(
    osal_queue_handle_t queue_handle,
    const void *p_data,
    osal_base_type_t *p_higher_priority_task_woken)
{
    rt_mq_t  p_queue = (rt_mq_t)queue_handle;
    rt_err_t result;

    /* rt_mq_send never blocks and is ISR-safe; the reschedule happens on
       interrupt exit, so no deferred yield is reported back. */
    if (RT_NULL != p_higher_priority_task_woken)
    {
        *p_higher_priority_task_woken = OSAL_FALSE;
    }

    result = rt_mq_send(p_queue, p_data, p_queue->msg_size);
    if (RT_EOK == result)
    {
        return OSAL_SUCCESS;
    }

    return OSAL_QUEUE_FULL;
}

/**
 * @brief Replace the content of a single-item mailbox.
 *
 * @note RT-Thread has no xQueueOverwrite. The pending message is drained
 *       through a scratch buffer under a scheduler lock and the new one is
 *       sent in its place, which is observably the same for the length-1
 *       queues this call is meant for. Items larger than
 *       OSAL_RTT_MAILBOX_ITEM_MAX are rejected instead of mishandled.
 *
 * @param[in] queue_handle Queue handle.
 * @param[in] p_data Item to copy into the queue.
 *
 * @return OSAL_SUCCESS on success, OSAL_ERR_IN_ISR in ISR context,
 *         OSAL_ERR_INVALID_SIZE when the item is too large, otherwise
 *         OSAL_ERROR.
 */
INT32_t osal_mailbox_overwrite_impl(osal_queue_handle_t queue_handle,
                                    const void *p_data)
{
    rt_mq_t   p_queue = (rt_mq_t)queue_handle;
    rt_uint8_t scratch[OSAL_RTT_MAILBOX_ITEM_MAX];
    rt_err_t  result;

    if (OSAL_IS_IN_ISR())
    {
        return OSAL_ERR_IN_ISR;
    }

    if (RT_NULL == p_queue)
    {
        return OSAL_ERROR;
    }

    if (p_queue->msg_size > (rt_size_t)OSAL_RTT_MAILBOX_ITEM_MAX)
    {
        return OSAL_ERR_INVALID_SIZE;
    }

    /* Scheduler lock rather than an interrupt lock: rt_mq_recv/rt_mq_send may
       reschedule, so no other thread may observe the queue mid-swap, while
       ISRs stay live. */
    rt_enter_critical();

    while (p_queue->entry > 0U)
    {
        if (RT_EOK != rt_mq_recv(p_queue, scratch, p_queue->msg_size, 0))
        {
            break;
        }
    }

    result = rt_mq_send(p_queue, p_data, p_queue->msg_size);

    rt_exit_critical();

    if (RT_EOK == result)
    {
        return OSAL_SUCCESS;
    }

    return OSAL_ERROR;
}

/**
 * @brief Report whether a queue currently holds anything.
 *
 * @param[in] p_queue_handle Pointer to the queue handle.
 *
 * @return OSAL_SUCCESS when at least one item is queued,
 *         OSAL_INVALID_POINTER on a NULL argument, otherwise
 *         OSAL_QUEUE_EMPTY.
 */
INT32_t osal_mailbox_peek_impl(osal_queue_handle_t *p_queue_handle)
{
    rt_mq_t p_queue;

    if (RT_NULL == p_queue_handle)
    {
        return OSAL_INVALID_POINTER;
    }

    p_queue = (rt_mq_t)(*p_queue_handle);
    if (RT_NULL == p_queue)
    {
        return OSAL_INVALID_POINTER;
    }

    /* entry is a single word; reading it is atomic in both contexts. */
    if (p_queue->entry > 0U)
    {
        return OSAL_SUCCESS;
    }

    return OSAL_QUEUE_EMPTY;
}

/**
 * @brief Count the items currently queued.
 *
 * @param[in] queue_handle Queue handle.
 *
 * @return Number of queued items, or 0 for a NULL handle.
 */
UINT32_t osal_queue_messages_waiting_impl(osal_queue_handle_t queue_handle)
{
    rt_mq_t p_queue = (rt_mq_t)queue_handle;

    if (RT_NULL == p_queue)
    {
        return 0U;
    }

    return (UINT32_t)p_queue->entry;
}

//******************************* Functions *********************************//

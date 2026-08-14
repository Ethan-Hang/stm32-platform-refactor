/******************************************************************************
 * @file os_rtthread.h
 *
 * @par dependencies
 * - rtthread.h
 * - osal_common_types.h
 * - osal_config.h
 *
 * @author Ethan-Hang
 *
 * @brief Shared RT-Thread helpers for the OSAL implementation layer.
 *
 * @note Counterpart of src_freertos/os_freertos.h. Everything here is about
 *       the two places where OSAL and RT-Thread disagree: how a timeout is
 *       spelled, and which direction priorities run.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __OS_RTTHREAD_H__
#define __OS_RTTHREAD_H__

//******************************** Includes *********************************//
#include <rtthread.h>
#include <rthw.h>            /* rt_hw_interrupt_disable / _enable */

#include "osal_common_types.h"
#include "osal_config.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/**
 * @brief Convert time value from milliseconds to OS tick count.
 *
 * Kept for parity with os_freertos.h. RT_TICK_PER_SECOND is 1000 in this
 * project, so ticks and milliseconds coincide, but the arithmetic is spelled
 * out rather than assumed.
 *
 * @param[in] osal_time_in_ms Time value in milliseconds.
 *
 * @return Converted tick count. OSAL_MAX_DELAY is preserved so wait-forever
 *         semantics survive the conversion.
 */
#define OS_MS_TO_TICKS(osal_time_in_ms)                                       \
    ((osal_time_in_ms == OSAL_MAX_DELAY) ? (OSAL_MAX_DELAY) :                 \
    ((osal_tick_type_t)(((osal_tick_type_t)(osal_time_in_ms) *                \
    (osal_tick_type_t)RT_TICK_PER_SECOND) / (osal_tick_type_t)1000U)))

/**
 * @brief Largest item size osal_mailbox_overwrite_impl() can replace.
 *
 * RT-Thread has no xQueueOverwrite equivalent, so the implementation drains
 * the pending message through a scratch buffer before sending the new one.
 * Larger items are rejected with OSAL_ERR_INVALID_SIZE rather than silently
 * mishandled.
 */
#define OSAL_RTT_MAILBOX_ITEM_MAX (64U)
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
/**
 * @brief Storage for one statically-created OSAL task.
 *
 * RT-Thread has no per-thread notification value, so the OSAL notification
 * slot is an rt_event embedded next to the thread. The thread must be the
 * first member: osal_task_handle_t doubles as rt_thread_t.
 *
 * Both members are reachable from the handle, but the notification slot is
 * always resolved through rt_thread.user_data (see osal_rtt_notify_slot),
 * never by casting, so that threads RT-Thread created for itself (idle,
 * timer) are never mistaken for OSAL tasks.
 *
 * sizeof == 188 B; OSAL_TCB_STORAGE_WORDS reserves 48 words = 192 B.
 */
typedef struct
{
    struct rt_thread thread; /**< @brief RT-Thread TCB; must stay first.    */
    struct rt_event  notify; /**< @brief OSAL task notification slot.       */
} OsalRttTaskBlock;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
/**
 * @brief Convert an OSAL timeout to the RT-Thread convention.
 *
 * @param[in] timeout OSAL timeout in ticks; OSAL_MAX_DELAY means forever.
 *
 * @return Tick count, or RT_WAITING_FOREVER.
 */
static inline rt_int32_t osal_rtt_timeout(osal_tick_type_t timeout)
{
    if (OSAL_MAX_DELAY == timeout)
    {
        return RT_WAITING_FOREVER;
    }

    return (rt_int32_t)timeout;
}

/**
 * @brief Convert an OSAL priority to the RT-Thread convention.
 *
 * OSAL follows FreeRTOS: a larger number is a higher priority. RT-Thread runs
 * the other way, 0 being the highest. Both scales span OSAL_PRIORITY_MAX
 * levels, so the mapping is a straight reflection.
 *
 * @param[in] priority OSAL priority, 0 .. OSAL_PRIORITY_MAX-1.
 *
 * @return RT-Thread priority, 0 .. OSAL_PRIORITY_MAX-1.
 */
static inline rt_uint8_t osal_rtt_priority(UINT32_t priority)
{
    if (priority >= (UINT32_t)OSAL_PRIORITY_MAX)
    {
        priority = (UINT32_t)OSAL_PRIORITY_MAX - 1U;
    }

    return (rt_uint8_t)(((UINT32_t)OSAL_PRIORITY_MAX - 1U) - priority);
}

/**
 * @brief Resolve the notification slot bound to a thread.
 *
 * Static tasks carry an embedded rt_event; dynamic ones get one allocated on
 * first use. Either way the slot is reached through user_data, which the
 * kernel itself only ever zeroes.
 *
 * @param[in] thread  Thread to look up.
 * @param[in] create  RT_TRUE to allocate a slot for a dynamic task that has
 *                    none yet.
 *
 * @return The thread's rt_event, or RT_NULL when it has no slot and none
 *         could be created.
 */
struct rt_event *osal_rtt_notify_slot(rt_thread_t thread, rt_bool_t create);
//******************************* Functions *********************************//

#endif /* __OS_RTTHREAD_H__ */

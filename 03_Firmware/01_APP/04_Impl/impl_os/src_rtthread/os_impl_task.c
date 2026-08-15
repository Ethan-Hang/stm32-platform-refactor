/******************************************************************************
 * @file os_impl_task.c
 *
 * @par dependencies
 * - osal_internal_task.h
 * - osal_error.h
 * - os_rtthread.h
 *
 * @author Ethan-Hang
 *
 * @brief OSAL task implementation based on RT-Thread thread APIs.
 *
 * @version V1.0 2026-8-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include <stddef.h>

#include "board_types.h"
#include "osal_internal_task.h"
#include "osal_error.h"
#include "osal_macros.h"

#include "os_rtthread.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/* The opaque OSAL TCB blob must hold the RT-Thread TCB plus the embedded
   notification slot. Checked here, the only TU that sees OsalRttTaskBlock. */
_Static_assert(sizeof(OsalRttTaskBlock) <= sizeof(OsalTaskTcbStorage),
               "OSAL_TCB_STORAGE_WORDS too small for OsalRttTaskBlock");

/* OSAL keeps the FreeRTOS priority span; RT-Thread's table has to cover it. */
_Static_assert(OSAL_PRIORITY_MAX <= RT_THREAD_PRIORITY_MAX,
               "OSAL_PRIORITY_MAX exceeds RT_THREAD_PRIORITY_MAX");

/* osal_task_handle_t is used directly as rt_thread_t, so the TCB must be the
   first member of the block. */
_Static_assert(offsetof(OsalRttTaskBlock, thread) == 0,
               "rt_thread must be the first member of OsalRttTaskBlock");
//******************************** Defines **********************************//

//****************************** Local Variables ****************************//
/* Nesting state for osal_critical_enter/exit; see those functions. */
static volatile rt_base_t   s_critical_level   = 0;
static volatile rt_uint32_t s_critical_nesting = 0U;
//****************************** Local Variables ****************************//

//***************************** Local Functions *****************************//
/**
 * @brief Resolve the notification slot bound to a thread.
 *
 * Declared in os_rtthread.h and used by os_impl_notify.c. Static tasks point
 * user_data at their embedded rt_event during creation; dynamic tasks get one
 * from the heap the first time a notification is waited on or sent.
 *
 * @param[in] thread Thread to look up.
 * @param[in] create RT_TRUE to allocate a slot when the thread has none.
 *
 * @return The thread's rt_event, or RT_NULL if it has none.
 */
struct rt_event *osal_rtt_notify_slot(rt_thread_t thread, rt_bool_t create)
{
    struct rt_event *p_event;
    rt_base_t        level;

    if (RT_NULL == thread)
    {
        return RT_NULL;
    }

    p_event = (struct rt_event *)thread->user_data;
    if ((RT_NULL != p_event) || (RT_FALSE == create))
    {
        return p_event;
    }

    /* Dynamic task, first use. Allocating from an ISR is not allowed, and the
       ISR path never creates: a task waits before anything notifies it. */
    if (OSAL_IS_IN_ISR())
    {
        return RT_NULL;
    }

    p_event = (struct rt_event *)rt_malloc(sizeof(struct rt_event));
    if (RT_NULL == p_event)
    {
        return RT_NULL;
    }

    rt_event_init(p_event, thread->name, RT_IPC_FLAG_FIFO);

    /* Publish under a lock so two waiters cannot both allocate. */
    level = rt_hw_interrupt_disable();
    if (RT_NULL == (struct rt_event *)thread->user_data)
    {
        thread->user_data = (rt_ubase_t)p_event;
        rt_hw_interrupt_enable(level);
    }
    else
    {
        rt_hw_interrupt_enable(level);
        rt_event_detach(p_event);
        rt_free(p_event);
        p_event = (struct rt_event *)thread->user_data;
    }

    return p_event;
}
//***************************** Local Functions *****************************//

//******************************* Functions *********************************//
/**
 * @brief Create a task object.
 *
 * @param[out] p_task_handle Output task handle.
 * @param[in] p_task_name Task name string.
 * @param[in] p_arg Task entry argument.
 * @param[in] task_entry Task entry function.
 * @param[in] stack_depth Task stack depth in words.
 * @param[in] priority Task priority.
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_ERROR.
 */
INT32_t osal_task_create_impl(osal_task_handle_t *p_task_handle,
                              const char *p_task_name,
                              void *p_arg,
                              osal_task_entry_t task_entry,
                              UINT32_t stack_depth,
                              UINT32_t priority)
{
    rt_thread_t thread;

    /* OSAL counts stack in words, RT-Thread in bytes. */
    thread = rt_thread_create(p_task_name,
                              (void (*)(void *))task_entry,
                              p_arg,
                              (rt_uint32_t)stack_depth * sizeof(UINT32_t),
                              osal_rtt_priority(priority),
                              RT_TICK_PER_SECOND / 100U);
    if (RT_NULL == thread)
    {
        return OSAL_ERROR;
    }

    if (RT_EOK != rt_thread_startup(thread))
    {
        (void)rt_thread_delete(thread);
        return OSAL_ERROR;
    }

    *p_task_handle = (osal_task_handle_t)thread;
    return OSAL_SUCCESS;
}

/**
 * @brief Create a task object from caller-provided static storage.
 *
 * @param[out] p_task_handle Output task handle.
 * @param[in] p_task_name Task name string.
 * @param[in] p_arg Task entry argument.
 * @param[in] task_entry Task entry function.
 * @param[in] stack_depth Stack depth in words.
 * @param[in] priority Task priority.
 * @param[in] p_stack Word-aligned stack buffer (>= stack_depth words).
 * @param[in] p_tcb TCB storage blob (>= sizeof(OsalRttTaskBlock)).
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_ERROR.
 */
INT32_t osal_task_create_static_impl(osal_task_handle_t *p_task_handle,
                                     const char *p_task_name,
                                     void *p_arg,
                                     osal_task_entry_t task_entry,
                                     UINT32_t stack_depth,
                                     UINT32_t priority,
                                     UINT32_t *p_stack,
                                     void *p_tcb)
{
    OsalRttTaskBlock *p_block = (OsalRttTaskBlock *)p_tcb;
    rt_err_t          err;

    err = rt_thread_init(&p_block->thread,
                         p_task_name,
                         (void (*)(void *))task_entry,
                         p_arg,
                         p_stack,
                         (rt_uint32_t)stack_depth * sizeof(UINT32_t),
                         osal_rtt_priority(priority),
                         RT_TICK_PER_SECOND / 100U);
    if (RT_EOK != err)
    {
        return OSAL_ERROR;
    }

    /* Bind the embedded notification slot before the thread can run, so no
       heap allocation is ever needed for a static task. */
    rt_event_init(&p_block->notify, p_task_name, RT_IPC_FLAG_FIFO);
    p_block->thread.user_data = (rt_ubase_t)&p_block->notify;

    if (RT_EOK != rt_thread_startup(&p_block->thread))
    {
        rt_event_detach(&p_block->notify);
        (void)rt_thread_detach(&p_block->thread);
        return OSAL_ERROR;
    }

    *p_task_handle = (osal_task_handle_t)&p_block->thread;
    return OSAL_SUCCESS;
}

/**
 * @brief Delete a task object.
 *
 * @note A task deleting itself is ordinary usage -- vTaskDelete() supports it
 *       and OSAL inherits that -- but it needs a different path here.
 *
 *       rt_thread_delete() and rt_thread_detach() both release the thread's
 *       built-in timer and then *return*, because they are written for
 *       deleting some other thread. Used on the running thread they return
 *       into a thread the kernel has already closed; execution carries on to
 *       the end of the entry function, whose stacked LR the kernel set to
 *       rt_thread_exit() (thread.c:167). That releases the same timer a
 *       second time and trips
 *
 *           (rt_object_get_type(&timer->parent) == RT_Object_Class_Timer)
 *           assertion failed at function:rt_timer_detach
 *
 *       So self-deletion goes straight to rt_thread_exit(), the path the
 *       kernel itself uses when an entry function returns: it handles static
 *       and dynamic threads alike and reschedules without coming back. Like
 *       vTaskDelete() on self, this call does not return.
 *
 * @param[in] task_handle Task handle.
 */
void osal_task_delete_impl(osal_task_handle_t task_handle)
{
    /* Not declared in any RT-Thread header: thread.c reaches it through the
       stacked LR rather than by name. It is a plain global, so an extern
       declaration is enough. */
    extern void rt_thread_exit(void);

    rt_thread_t      thread = (rt_thread_t)task_handle;
    struct rt_event *p_event;
    rt_bool_t        is_static;

    if (RT_NULL == thread)
    {
        return;
    }

    p_event           = (struct rt_event *)thread->user_data;
    thread->user_data = 0U;

    /* rt_thread inlines the rt_object fields in 4.0.3 rather than nesting a
       parent object, so the static/dynamic flag is read straight off type --
       the same test rt_object_is_systemobject() performs. */
    is_static = (0U != (thread->type & RT_Object_Class_Static)) ? RT_TRUE
                                                                : RT_FALSE;

    /* The notification slot is released first either way: for self-deletion
       rt_thread_exit() never comes back, so there is no later opportunity. A
       static task's slot lives in the caller's storage and is only detached;
       a dynamic task's came from the heap. */
    if (RT_NULL != p_event)
    {
        rt_event_detach(p_event);
        if (RT_FALSE == is_static)
        {
            rt_free(p_event);
        }
    }

    if (thread == rt_thread_self())
    {
        rt_thread_exit();
        return; /* not reached */
    }

    if (RT_TRUE == is_static)
    {
        (void)rt_thread_detach(thread);
    }
    else
    {
        (void)rt_thread_delete(thread);
    }
}

/**
 * @brief Delay current task for specified ticks.
 *
 * @param[in] ticks_to_delay Delay ticks.
 */
void osal_task_delay_impl(osal_tick_type_t ticks_to_delay)
{
    (void)rt_thread_delay((rt_tick_t)ticks_to_delay);
}

/**
 * @brief Yield current task in scheduler.
 */
void osal_task_yield_impl(void)
{
    (void)rt_thread_yield();
}

/**
 * @brief Get current system tick count in task context.
 *
 * @return Current OSAL tick count.
 */
osal_tick_type_t osal_task_get_tick_count_impl(void)
{
    return (osal_tick_type_t)rt_tick_get();
}

/**
 * @brief Convert milliseconds to RT-Thread ticks.
 *
 * @param[in] time_in_ms Duration in milliseconds; OSAL_MAX_DELAY passes
 *                       through so wait-forever survives the conversion.
 *
 * @return Equivalent tick count.
 */
osal_tick_type_t osal_ms_to_ticks_impl(UINT32_t time_in_ms)
{
    return OS_MS_TO_TICKS(time_in_ms);
}

/**
 * @brief Get current system tick count in ISR context.
 *
 * @return Current OSAL tick count.
 */
osal_tick_type_t osal_task_get_tick_count_from_isr_impl(void)
{
    /* rt_tick_get() is a plain read of a volatile word: ISR-safe as is. */
    return (osal_tick_type_t)rt_tick_get();
}

/**
 * @brief Get the handle of the currently running task.
 *
 * @return Current task handle.
 */
osal_task_handle_t osal_task_get_current_handle_impl(void)
{
    return (osal_task_handle_t)rt_thread_self();
}

/**
 * @brief Request a context switch on the way out of an ISR.
 *
 * @note Deliberately empty. RT-Thread's ISR-safe primitives set the pending
 *       switch themselves and rt_interrupt_leave() performs it, so there is
 *       nothing for the caller to trigger -- which is why every *_from_isr
 *       implementation here reports OSAL_FALSE. Callers still make the call
 *       so the same ISR code compiles against either backend.
 *
 * @param[in] higher_priority_task_woken Flag from an *_from_isr call.
 */
void osal_yield_from_isr_impl(osal_base_type_t higher_priority_task_woken)
{
    (void)higher_priority_task_woken;
}

/**
 * @brief Enter critical section by disabling interrupts.
 *
 * @note Mirrors vPortEnterCritical, which is interrupt-based and nesting
 *       counted -- not rt_enter_critical(), which only locks the scheduler
 *       and would leave ISRs running. RT-Thread's own primitives hand the
 *       saved PRIMASK back to the caller, so the nesting depth and the level
 *       to restore are tracked here. Updating them is safe without further
 *       protection because interrupts are already off at that point.
 */
void osal_critical_enter_impl(void)
{
    rt_base_t level = rt_hw_interrupt_disable();

    if (0U == s_critical_nesting)
    {
        s_critical_level = level;
    }

    s_critical_nesting++;
}

/**
 * @brief Exit critical section by restoring interrupts.
 */
void osal_critical_exit_impl(void)
{
    if (0U == s_critical_nesting)
    {
        return;
    }

    s_critical_nesting--;
    if (0U == s_critical_nesting)
    {
        rt_hw_interrupt_enable(s_critical_level);
    }
}

//******************************* Functions *********************************//

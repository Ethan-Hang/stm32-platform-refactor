/******************************************************************************
 * @file os_impl_timer.c
 *
 * @par dependencies
 * - osal_internal_timer.h
 * - osal_error.h
 * - os_rtthread.h
 *
 * @author Ethan-Hang
 *
 * @brief OSAL software timer implementation based on RT-Thread timer APIs.
 *
 * @note Timers are created with RT_TIMER_FLAG_SOFT_TIMER so callbacks run in
 *       the timer thread, matching the FreeRTOS daemon-task model rather than
 *       running them in the tick ISR.
 *
 *       The FreeRTOS backend's timeout arguments are command-queue block
 *       times; RT-Thread manipulates timers directly, so they have no meaning
 *       here and are accepted and ignored.
 *
 * @version V1.0 2026-8-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "board_types.h"
#include "osal_internal_timer.h"
#include "osal_error.h"
#include "osal_macros.h"

#include "os_rtthread.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief Create a software timer object.
 *
 * @param[out] p_timer_handle Output timer handle.
 * @param[in] p_timer_name Timer name string.
 * @param[in] period_ticks Timer period in OSAL ticks.
 * @param[in] auto_reload OSAL_TRUE for a periodic timer.
 * @param[in] p_arg Callback argument.
 * @param[in] callback Callback invoked when the timer expires.
 *
 * @return OSAL_SUCCESS on success, OSAL_TIMER_ERR_INVALID_ARGS on a bad
 *         argument, otherwise OSAL_TIMER_ERR_UNAVAILABLE.
 */
INT32_t osal_timer_create_impl(osal_timer_handle_t *p_timer_handle,
                               const char *p_timer_name,
                               osal_tick_type_t period_ticks,
                               osal_base_type_t auto_reload,
                               void *p_arg,
                               osal_timer_callback_t callback)
{
    rt_timer_t timer_handle;
    rt_uint8_t flag;

    if ((RT_NULL == p_timer_handle) || (RT_NULL == callback))
    {
        return OSAL_TIMER_ERR_INVALID_ARGS;
    }

    if (0U == period_ticks)
    {
        return OSAL_TIMER_ERR_INVALID_ARGS;
    }

    flag = RT_TIMER_FLAG_SOFT_TIMER;
    flag |= (OSAL_FALSE != auto_reload) ? RT_TIMER_FLAG_PERIODIC
                                        : RT_TIMER_FLAG_ONE_SHOT;

    timer_handle = rt_timer_create(p_timer_name,
                                   (void (*)(void *))callback,
                                   p_arg,
                                   (rt_tick_t)period_ticks,
                                   flag);
    if (RT_NULL == timer_handle)
    {
        return OSAL_TIMER_ERR_UNAVAILABLE;
    }

    *p_timer_handle = (osal_timer_handle_t)timer_handle;
    return OSAL_SUCCESS;
}

/**
 * @brief Delete a software timer object.
 *
 * @param[in] timer_handle Timer handle.
 */
void osal_timer_delete_impl(osal_timer_handle_t timer_handle)
{
    if (OSAL_IS_IN_ISR())
    {
        return;
    }

    (void)rt_timer_delete((rt_timer_t)timer_handle);
}

/**
 * @brief Start a software timer.
 *
 * @param[in] timer_handle Timer handle.
 * @param[in] timeout Ignored; see the file note.
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_TIMER_ERR_INTERNAL.
 */
INT32_t osal_timer_start_impl(osal_timer_handle_t timer_handle,
                              osal_tick_type_t timeout)
{
    (void)timeout;

    if (RT_EOK == rt_timer_start((rt_timer_t)timer_handle))
    {
        return OSAL_SUCCESS;
    }

    return OSAL_TIMER_ERR_INTERNAL;
}

/**
 * @brief Stop a software timer.
 *
 * @param[in] timer_handle Timer handle.
 * @param[in] timeout Ignored; see the file note.
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_TIMER_ERR_INTERNAL.
 */
INT32_t osal_timer_stop_impl(osal_timer_handle_t timer_handle,
                             osal_tick_type_t timeout)
{
    (void)timeout;

    if (RT_EOK == rt_timer_stop((rt_timer_t)timer_handle))
    {
        return OSAL_SUCCESS;
    }

    return OSAL_TIMER_ERR_INTERNAL;
}

/**
 * @brief Restart a software timer from zero, keeping its period.
 *
 * @param[in] timer_handle Timer handle.
 * @param[in] timeout Ignored; see the file note.
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_TIMER_ERR_INTERNAL.
 */
INT32_t osal_timer_reset_impl(osal_timer_handle_t timer_handle,
                              osal_tick_type_t timeout)
{
    rt_timer_t p_timer = (rt_timer_t)timer_handle;

    (void)timeout;

    /* xTimerReset restarts the countdown whether or not the timer is running;
       rt_timer_start on an active timer re-arms it, and stopping first makes
       the idle case behave identically. */
    (void)rt_timer_stop(p_timer);

    if (RT_EOK == rt_timer_start(p_timer))
    {
        return OSAL_SUCCESS;
    }

    return OSAL_TIMER_ERR_INTERNAL;
}

/**
 * @brief Change a timer's period and restart it.
 *
 * @param[in] timer_handle Timer handle.
 * @param[in] new_period_ticks New period in OSAL ticks.
 * @param[in] timeout Ignored; see the file note.
 *
 * @return OSAL_SUCCESS on success, OSAL_TIMER_ERR_INVALID_ARGS on a zero
 *         period, otherwise OSAL_TIMER_ERR_INTERNAL.
 */
INT32_t osal_timer_change_period_impl(osal_timer_handle_t timer_handle,
                                      osal_tick_type_t new_period_ticks,
                                      osal_tick_type_t timeout)
{
    rt_timer_t p_timer = (rt_timer_t)timer_handle;
    rt_tick_t  period  = (rt_tick_t)new_period_ticks;

    (void)timeout;

    if (0U == new_period_ticks)
    {
        return OSAL_TIMER_ERR_INVALID_ARGS;
    }

    /* SET_TIME on a running timer is not honoured until it is re-armed, so
       stop first. Like xTimerChangePeriod, this leaves the timer running. */
    (void)rt_timer_stop(p_timer);

    if (RT_EOK != rt_timer_control(p_timer, RT_TIMER_CTRL_SET_TIME, &period))
    {
        return OSAL_TIMER_ERR_INTERNAL;
    }

    if (RT_EOK == rt_timer_start(p_timer))
    {
        return OSAL_SUCCESS;
    }

    return OSAL_TIMER_ERR_INTERNAL;
}

//******************************* Functions *********************************//

/******************************************************************************
 * @file os_impl_sema.c
 *
 * @par dependencies
 * - osal_internal_sema.h
 * - osal_error.h
 * - os_rtthread.h
 *
 * @author Ethan-Hang
 *
 * @brief OSAL semaphore implementation based on RT-Thread semaphore APIs.
 *
 * @version V1.0 2026-8-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "board_types.h"
#include "osal_internal_sema.h"
#include "osal_error.h"
#include "osal_macros.h"

#include "os_rtthread.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief Create a counting semaphore object.
 *
 * @param[out] p_sema_handle Output semaphore handle.
 * @param[in] initial_count Initial semaphore count.
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_SEM_FAILURE.
 */
INT32_t osal_sema_create_impl(osal_sema_handle_t *p_sema_handle,
                              UINT32_t initial_count)
{
    rt_sem_t sema_handle;

    /* RT-Thread semaphores have no explicit maximum; the FreeRTOS backend
       derives one from initial_count only because xSemaphoreCreateCounting
       demands it. Nothing is lost by not modelling it here. */
    sema_handle = rt_sem_create("osal_sem",
                                (rt_uint32_t)initial_count,
                                RT_IPC_FLAG_FIFO);
    if (RT_NULL == sema_handle)
    {
        return OSAL_SEM_FAILURE;
    }

    *p_sema_handle = (osal_sema_handle_t)sema_handle;
    return OSAL_SUCCESS;
}

/**
 * @brief Delete a semaphore object.
 *
 * @param[in] sema_handle Semaphore handle.
 */
void osal_sema_delete_impl(osal_sema_handle_t sema_handle)
{
    if (OSAL_IS_IN_ISR())
    {
        return;
    }

    (void)rt_sem_delete((rt_sem_t)sema_handle);
}

/**
 * @brief Take (decrement) semaphore in task context.
 *
 * @param[in] sema_handle Semaphore handle.
 * @param[in] timeout Wait timeout in OSAL ticks.
 *
 * @return OSAL_SUCCESS on success, OSAL_ERR_IN_ISR in ISR context,
 *         otherwise OSAL_SEM_TIMEOUT.
 */
INT32_t osal_sema_take_impl(osal_sema_handle_t sema_handle,
                            osal_tick_type_t timeout)
{
    rt_err_t result;

    if (OSAL_IS_IN_ISR())
    {
        return OSAL_ERR_IN_ISR;
    }

    result = rt_sem_take((rt_sem_t)sema_handle, osal_rtt_timeout(timeout));
    if (RT_EOK == result)
    {
        return OSAL_SUCCESS;
    }

    return OSAL_SEM_TIMEOUT;
}

/**
 * @brief Give (increment) semaphore in task context.
 *
 * @param[in] sema_handle Semaphore handle.
 *
 * @return OSAL_SUCCESS on success, OSAL_ERR_IN_ISR in ISR context,
 *         otherwise OSAL_SEM_FAILURE.
 */
INT32_t osal_sema_give_impl(osal_sema_handle_t sema_handle)
{
    rt_err_t result;

    if (OSAL_IS_IN_ISR())
    {
        return OSAL_ERR_IN_ISR;
    }

    result = rt_sem_release((rt_sem_t)sema_handle);
    if (RT_EOK == result)
    {
        return OSAL_SUCCESS;
    }

    return OSAL_SEM_FAILURE;
}

/**
 * @brief Give semaphore from ISR context.
 *
 * @param[in] sema_handle Semaphore handle.
 * @param[out] p_higher_priority_task_woken Optional wakeup flag from ISR.
 *
 * @return OSAL_SUCCESS on success, OSAL_ERROR when not in ISR,
 *         otherwise OSAL_SEM_FAILURE.
 */
INT32_t osal_sema_give_from_isr_impl(
    osal_sema_handle_t sema_handle,
    osal_base_type_t *p_higher_priority_task_woken)
{
    rt_err_t result;

    if (!OSAL_IS_IN_ISR())
    {
        return OSAL_ERROR;
    }

    /* rt_sem_release is ISR-safe and schedules the switch itself on the way
       out of the interrupt, so there is no deferred yield to report. The flag
       is still cleared: callers pass it to osal_yield_from_isr(). */
    if (RT_NULL != p_higher_priority_task_woken)
    {
        *p_higher_priority_task_woken = OSAL_FALSE;
    }

    result = rt_sem_release((rt_sem_t)sema_handle);
    if (RT_EOK == result)
    {
        return OSAL_SUCCESS;
    }

    return OSAL_SEM_FAILURE;
}

//******************************* Functions *********************************//

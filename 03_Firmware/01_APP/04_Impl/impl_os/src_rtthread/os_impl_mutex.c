/******************************************************************************
 * @file os_impl_mutex.c
 *
 * @par dependencies
 * - osal_internal_mutex.h
 * - osal_error.h
 * - os_rtthread.h
 *
 * @author Ethan-Hang
 *
 * @brief OSAL mutex implementation based on RT-Thread mutex APIs.
 *
 * @note Both backends provide priority inheritance and recursive take, so
 *       this maps one to one.
 *
 * @version V1.0 2026-8-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "board_types.h"
#include "osal_internal_mutex.h"
#include "osal_error.h"
#include "osal_macros.h"

#include "os_rtthread.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief Create a mutex object.
 *
 * @param[out] p_mutex_handle Output mutex handle.
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_ERROR.
 */
INT32_t osal_mutex_create_impl(osal_mutex_handle_t *p_mutex_handle)
{
    rt_mutex_t mutex_handle;

    /* RT_IPC_FLAG_PRIO: wake the highest-priority waiter first, which is what
       priority inheritance is for. FIFO would defeat it. */
    mutex_handle = rt_mutex_create("osal_mtx", RT_IPC_FLAG_PRIO);
    if (RT_NULL == mutex_handle)
    {
        return OSAL_ERROR;
    }

    *p_mutex_handle = (osal_mutex_handle_t)mutex_handle;
    return OSAL_SUCCESS;
}

/**
 * @brief Delete a mutex object.
 *
 * @param[in] mutex_handle Mutex handle.
 */
void osal_mutex_delete_impl(osal_mutex_handle_t mutex_handle)
{
    if (OSAL_IS_IN_ISR())
    {
        return;
    }

    (void)rt_mutex_delete((rt_mutex_t)mutex_handle);
}

/**
 * @brief Take (lock) a mutex in task context.
 *
 * @param[in] mutex_handle Mutex handle.
 * @param[in] timeout Wait timeout in OSAL ticks.
 *
 * @return OSAL_SUCCESS on success, OSAL_ERR_IN_ISR in ISR context,
 *         otherwise OSAL_ERROR_TIMEOUT.
 */
INT32_t osal_mutex_take_impl(osal_mutex_handle_t mutex_handle,
                             osal_tick_type_t timeout)
{
    rt_err_t result;

    /* A mutex has an owner thread, so there is nothing to own it in an ISR.
       This is the rule the project's ISR handlers work around by waking a
       handler task through osal_notify. */
    if (OSAL_IS_IN_ISR())
    {
        return OSAL_ERR_IN_ISR;
    }

    result = rt_mutex_take((rt_mutex_t)mutex_handle, osal_rtt_timeout(timeout));
    if (RT_EOK == result)
    {
        return OSAL_SUCCESS;
    }

    return OSAL_ERROR_TIMEOUT;
}

/**
 * @brief Give (unlock) a mutex in task context.
 *
 * @param[in] mutex_handle Mutex handle.
 *
 * @return OSAL_SUCCESS on success, OSAL_ERR_IN_ISR in ISR context,
 *         otherwise OSAL_ERROR.
 */
INT32_t osal_mutex_give_impl(osal_mutex_handle_t mutex_handle)
{
    rt_err_t result;

    if (OSAL_IS_IN_ISR())
    {
        return OSAL_ERR_IN_ISR;
    }

    result = rt_mutex_release((rt_mutex_t)mutex_handle);
    if (RT_EOK == result)
    {
        return OSAL_SUCCESS;
    }

    return OSAL_ERROR;
}

//******************************* Functions *********************************//

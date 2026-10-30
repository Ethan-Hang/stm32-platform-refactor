/******************************************************************************
 * @file osal_sema.h
 *
 * @par dependencies
 * - osal_common_types.h
 *
 * @author Ethan-Hang
 *
 * @brief OSAL public semaphore wrapper interfaces.
 *
 * @version V1.0 2026-4-9
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __OSAL_SEMA_H__
#define __OSAL_SEMA_H__

//******************************** Includes *********************************//
#include "osal_common_types.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief Initialize semaphore object.
 *
 * @param[out] p_sema_handle Output semaphore handle.
 * @param[in] initial_count Initial count value.
 *
 * @return OSAL status code.
 */
int32_t osal_sema_init         (osal_sema_handle_t * p_sema_handle,
                                          uint32_t   initial_count);

/**
 * @brief Initialize a counting semaphore with an explicit ceiling.
 *
 * osal_sema_init() derives the ceiling from initial_count, so the
 * producer/consumer shape -- start empty, count up to N -- cannot be
 * expressed through it: initial_count 0 yields a ceiling of 1, i.e. a
 * binary semaphore that silently drops every give past the first.
 * This entry takes the two numbers separately.
 *
 * @param[out] p_sema_handle Output semaphore handle.
 * @param[in] max_count Ceiling. Must be >= 1 and >= initial_count.
 * @param[in] initial_count Initial count value, 0 for "start empty".
 *
 * @return OSAL_SUCCESS on success, OSAL_INVALID_POINTER when
 *         p_sema_handle is NULL, OSAL_INVALID_SEM_VALUE when the counts
 *         are inconsistent, otherwise a lower layer error code.
 *
 * @note The ceiling is enforced by the FreeRTOS backend only: RT-Thread
 *       semaphores carry no maximum (rt_sem_release stops at
 *       RT_SEM_VALUE_MAX, 65535). Do not lean on a failing give as your
 *       only overflow guard -- whoever owns the underlying resource has
 *       to refuse to over-commit it. Here that is the caller's ring
 *       buffer; see bsp_mpuxxxx_handler.c.
 */
int32_t osal_sema_init_counting(osal_sema_handle_t * p_sema_handle,
                                          uint32_t       max_count,
                                          uint32_t   initial_count);

/**
 * @brief Delete semaphore object.
 *
 * @param[in] sema_handle Semaphore handle.
 */
void    osal_sema_delete       (osal_sema_handle_t     sema_handle);

/**
 * @brief Take semaphore object.
 *
 * @param[in] sema_handle Semaphore handle.
 * @param[in] timeout Wait timeout in ticks.
 *
 * @return OSAL status code.
 */
int32_t osal_sema_take         (osal_sema_handle_t     sema_handle,
                                  osal_tick_type_t         timeout);

/**
 * @brief Give semaphore object.
 *
 * @param[in] sema_handle Semaphore handle.
 *
 * @return OSAL status code.
 */
int32_t osal_sema_give         (osal_sema_handle_t     sema_handle);

/**
 * @brief Give semaphore object from ISR.
 *
 * @param[in] sema_handle Semaphore handle.
 * @param[out] p_higher_priority_task_woken Optional wakeup flag.
 *
 * @return OSAL status code.
 */
int32_t osal_sema_give_from_isr(osal_sema_handle_t     sema_handle,
                                  osal_base_type_t * \
                                       p_higher_priority_task_woken);

//******************************* Functions *********************************//

#endif /* __OSAL_SEMA_H__ */

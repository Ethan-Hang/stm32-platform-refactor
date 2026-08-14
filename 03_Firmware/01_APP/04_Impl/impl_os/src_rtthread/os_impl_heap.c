/******************************************************************************
 * @file os_impl_heap.c
 *
 * @par dependencies
 * - osal_internal_heap.h
 * - os_rtthread.h
 *
 * @author Ethan-Hang
 *
 * @brief OSAL heap implementation based on RT-Thread memory APIs.
 *
 * @note The heap region itself is handed to rt_system_heap_init() during
 *       startup, not here; this file only forwards allocation requests.
 *
 * @version V1.0 2026-8-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "board_types.h"
#include "osal_internal_heap.h"

#include "os_rtthread.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief Allocate a block from the RTOS heap.
 *
 * @param[in] size Number of bytes to allocate.
 *
 * @return Pointer to the block, or NULL on failure.
 */
void *osal_heap_malloc_impl(SIZE_t size)
{
    return rt_malloc((rt_size_t)size);
}

/**
 * @brief Release a block back to the RTOS heap.
 *
 * @param[in] ptr Block previously returned by osal_heap_malloc_impl().
 */
void osal_heap_free_impl(void *ptr)
{
    rt_free(ptr);
}

//******************************* Functions *********************************//

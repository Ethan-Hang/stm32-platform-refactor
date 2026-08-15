/******************************************************************************
 * @file task_higher_water_monitor.c
 *
 * @par dependencies
 *
 * @author Ethan-Hang
 *
 * @brief Runtime stack high-water monitor task implementation.
 *
 * Processing flow:
 * Override weak task_higher_water_thread, periodically scan configured tasks,
 * then print high-water report to SEGGER RTT Terminal 1.
 * @version V1.0 2026-04-13
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "user_task_reso_config.h"
#include "platform_type.h"

/* Debug-only task: it reads the RTOS's own bookkeeping, which OSAL does not
   abstract and should not, so each backend is queried with its native API. */
#include "osal_config.h"
#if (OSAL_RTOS_SUPPORT == RTTHREAD_SUPPORT)
#include <rtthread.h>
#else
#include "FreeRTOS.h"
#include "task.h"
#endif

#include "Debug.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define STACK_MONITOR_PERIOD_TICKS  (1000U)

//******************************** Defines **********************************//

//******************************* Declaring *********************************//
extern usertaskcfg_t g_user_task_cfg[USER_TASK_NUM];

//******************************* Declaring *********************************//

//******************************* Functions *********************************//
static void stack_higher_water_report_once(void)
{
    DEBUG_OUT(i, STACK_MONITOR_LOG_TAG, "---------------------------------------------");

    for (UINT32_T i = 0; i < (UINT32_T)USER_TASK_NUM; i++)
    {
        const char        *task_name   = g_user_task_cfg[i].task_name;
        osal_task_handle_t task_handle = g_user_task_cfg[i].task_handle;

        if ((NULL == task_handle) || (NULL == task_name))
        {
            continue;
        }

#if (OSAL_RTOS_SUPPORT == RTTHREAD_SUPPORT)
        /* RT-Thread paints the whole stack with '#' in rt_thread_init and has
           no watermark call, so the untouched run is measured directly -- the
           same thing its own list_thread() command does. */
        rt_thread_t        thread = (rt_thread_t)task_handle;
        const rt_uint8_t  *p_byte = (const rt_uint8_t *)thread->stack_addr;
        const rt_uint8_t  *p_end  = p_byte + thread->stack_size;

        while ((p_byte < p_end) && ('#' == *p_byte))
        {
            p_byte++;
        }

        UINT32_T min_free_words = (UINT32_T)(p_byte -
                                  (const rt_uint8_t *)thread->stack_addr) /
                                  sizeof(UINT32_T);
#else
        UBaseType_t min_free_words = uxTaskGetStackHighWaterMark((TaskHandle_t)task_handle);
#endif
        UINT32_T    cfg_words      = (UINT32_T)g_user_task_cfg[i].stack_depth;
        UINT32_T    used_max_words = (cfg_words > (UINT32_T)min_free_words) ?
                                     (cfg_words - (UINT32_T)min_free_words) : 0U;

        DEBUG_OUT(i, STACK_MONITOR_LOG_TAG,
                  "[%-25s] cfg=%5luW  used_max=%5luW  min_free=%5luW",
                  task_name,
                  (unsigned long)cfg_words,
                  (unsigned long)used_max_words,
                  (unsigned long)min_free_words);
    }

    /**
     * Heap watermark: guards the heap trim (32 KB -> 28 KB -> 16 KB) that
     * funded the extfont glyph cache.  min_ever approaching zero means the
     * trim was too aggressive. Both backends are sized alike, so the numbers
     * are directly comparable.
     **/
#if (OSAL_RTOS_SUPPORT == RTTHREAD_SUPPORT)
    {
        rt_uint32_t total    = 0U;
        rt_uint32_t used     = 0U;
        rt_uint32_t max_used = 0U;

        /* RT-Thread reports peak usage rather than the smallest free block,
           so min_ever is derived to keep the two backends' output aligned. */
        rt_memory_info(&total, &used, &max_used);

        DEBUG_OUT(i, STACK_MONITOR_LOG_TAG,
                  "[heap] free=%5lu B  min_ever=%5lu B  of %lu B",
                  (unsigned long)(total - used),
                  (unsigned long)(total - max_used),
                  (unsigned long)total);
    }
#else
    DEBUG_OUT(i, STACK_MONITOR_LOG_TAG,
              "[heap] free=%5lu B  min_ever=%5lu B  of %lu B",
              (unsigned long)xPortGetFreeHeapSize(),
              (unsigned long)xPortGetMinimumEverFreeHeapSize(),
              (unsigned long)configTOTAL_HEAP_SIZE);
#endif

    DEBUG_OUT(i, STACK_MONITOR_LOG_TAG, "---------------------------------------------");
}

void task_higher_water_thread(void *argument)
{
    (void)argument;

    DEBUG_OUT(i, STACK_MONITOR_LOG_TAG, "stack monitor started");

    for (;;)
    {
        stack_higher_water_report_once();
        osal_task_delay(STACK_MONITOR_PERIOD_TICKS);
    }
}

//******************************* Functions *********************************//

/******************************************************************************
 * @file bsp_wt588_handler.c
 *
 * @par dependencies
 *
 * @author Ethan-Hang
 *
 * @brief Implement WT588 handler initialization and worker thread routines.
 *
 * Processing flow:
 * Instantiate driver, create queues/tasks, and clean up on init failure.
 * @version V1.0 2026--
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "bsp_wt588_handler.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define HANDLER_OS_INTERFACE(INST) ((INST)->handler_input_args->p_os_interface)
static bsp_wt588_handler_t s_wt588_handler_instance = {0};
static list_handler_t  s_play_request_list_instance = {0};

//******************************** Defines **********************************//

//******************************* Declaring *********************************//

//******************************* Declaring *********************************//

//******************************* Functions *********************************//
void wt588_handler_thread(void * const args)
{
    wt_handler_input_args_t *p_input_args = (wt_handler_input_args_t *)args;

    static bsp_wt588_driver_t wt588_driver_instance = {0};
    s_wt588_handler_instance.p_wt588_driver = &wt588_driver_instance;
    s_wt588_handler_instance.semaphore_handler            = NULL;
    s_wt588_handler_instance.voice_add_queue              = NULL;
    s_wt588_handler_instance.executor_queue               = NULL;
    s_wt588_handler_instance.busy_detection_queue         = NULL;
    s_wt588_handler_instance.voice_finish_queue           = NULL;
    s_wt588_handler_instance.executor_thread_handle       = NULL;
    s_wt588_handler_instance.busy_detection_thread_handle = NULL;
    s_wt588_handler_instance.p_play_request_list          \
                                 = &s_play_request_list_instance;

    wt_handler_status_t ret = wt588_handler_inst(
                                  &s_wt588_handler_instance, 
                                               p_input_args);
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "wt588_handler_thread inst failed, task suspended");
        while (1)
        {
            osal_task_delay(1000);
        }
    }

    uint32_t             add_cnt        =     0;
    uint32_t             finish_cnt     =     0;
    list_voice_node_t *p_node_add      =  NULL;
    list_voice_node_t *p_node_del      =  NULL;
    list_voice_node_t *p_node_highest  =  NULL;
    bool               is_higher_node  = false;

    DEBUG_OUT(i, WT588_HANDLER_LOG_TAG, "wt588_handler_thread running");
    while (1)
    {
        // Check for new play requests and add them to the list
        // do
        // {
            add_cnt = HANDLER_OS_INTERFACE(&s_wt588_handler_instance)->\
                      pf_os_queue_count(s_wt588_handler_instance.voice_add_queue);
            DEBUG_OUT(d, WT588_HANDLER_LOG_TAG,
                      "get voice_add_queue count: %u", add_cnt);
        // } while (add_cnt == 0);

        // DEBUG_OUT(d, WT588_HANDLER_LOG_TAG,
        //           "get voice_add_queue count: %u", add_cnt);
        for (uint32_t i = 0; i < add_cnt; i++)
        {
            HANDLER_OS_INTERFACE(&s_wt588_handler_instance)->\
            pf_os_queue_get(s_wt588_handler_instance.voice_add_queue,
                                                         &p_node_add,
                                                      OSAL_MAX_DELAY);
            s_wt588_handler_instance.p_play_request_list->pf_list_add_node(
                     s_wt588_handler_instance.p_play_request_list, p_node_add);
        }
        s_wt588_handler_instance.p_play_request_list->pf_list_sort(
                     s_wt588_handler_instance.p_play_request_list,
                     s_wt588_handler_instance.p_play_request_list->current_priority);

        // delete finished play requests from the list
        finish_cnt = HANDLER_OS_INTERFACE(&s_wt588_handler_instance)->\
                     pf_os_queue_count(
                         s_wt588_handler_instance.voice_finish_queue);
        DEBUG_OUT(d, WT588_HANDLER_LOG_TAG,
                  "get voice_finish_queue count: %u", finish_cnt);
        for (uint32_t i = 0; i < finish_cnt; i++)
        {
            HANDLER_OS_INTERFACE(&s_wt588_handler_instance)->\
            pf_os_queue_get(s_wt588_handler_instance.voice_finish_queue,
                                                            &p_node_del,
                                                         OSAL_MAX_DELAY);
            s_wt588_handler_instance.p_play_request_list->pf_list_del_node(
                     s_wt588_handler_instance.p_play_request_list, p_node_del);
        }
        for (uint8_t i = 1; i < WT588_MAX_PRIORITY; i++)
        {
            if (s_wt588_handler_instance.p_play_request_list->\
                                                 node_count_by_priority[i] > 1)
            {
                s_wt588_handler_instance.p_play_request_list->pf_list_sort(
                     s_wt588_handler_instance.p_play_request_list, i);
            }
        }

        // check if the highest priority play request has changed,
        // if so, send the new highest priority request to executor
        is_higher_node = \
              s_wt588_handler_instance.p_play_request_list->current_priority >\
              s_wt588_handler_instance.p_play_request_list->highest_priority;
        if (is_higher_node || 0 != add_cnt || 0 != finish_cnt)
        {
            p_node_highest = s_wt588_handler_instance.p_play_request_list->\
            pf_get_first_node(s_wt588_handler_instance.p_play_request_list);
            if (NULL != p_node_highest)
            {
                HANDLER_OS_INTERFACE(&s_wt588_handler_instance)->\
                pf_os_queue_send(s_wt588_handler_instance.executor_queue,
                                                         &p_node_highest,
                                                          OSAL_MAX_DELAY);
            }
        }

        // if the list is empty, reset current and highest priority
        // to lowest level (15)
        bool is_empty = s_wt588_handler_instance.p_play_request_list->\
        pf_list_is_empty(s_wt588_handler_instance.p_play_request_list);
        if (is_empty)
        {
            DEBUG_OUT(d, WT588_HANDLER_LOG_TAG, "play request list is empty");
            s_wt588_handler_instance.p_play_request_list->current_priority = 15;
            s_wt588_handler_instance.p_play_request_list->highest_priority = 15;
        }

        HANDLER_OS_INTERFACE(&s_wt588_handler_instance)->\
                              p_os_delay_interface->pf_os_delay_ms(200);
    }
}

#define WT588_BUSY_POLL_CNT      (500U)
#define WT588_BUSY_POLL_MS       (3U)
#define WT588_MAX_PLAY_RETRY     (3U)

static void wt588_executor_thread(void * const args)
{
    DEBUG_OUT(i, WT588_HANDLER_LOG_TAG, "wt588_executor_thread running");

    list_voice_node_t *p_node_exec = NULL;
    uint8_t            fail_cnt    =    0;
    bool               busy_found  = false;

    while (1)
    {
        HANDLER_OS_INTERFACE(&s_wt588_handler_instance)->\
        pf_os_queue_get(s_wt588_handler_instance.executor_queue,
                                                        &p_node_exec,
                                                     OSAL_MAX_DELAY);
        fail_cnt = 0;

retry_play:
        s_wt588_handler_instance.p_wt588_driver->pf_set_volume(
                 s_wt588_handler_instance.p_wt588_driver, p_node_exec->volume);
        s_wt588_handler_instance.p_wt588_driver->pf_start_play(
            s_wt588_handler_instance.p_wt588_driver, p_node_exec->volume_addr);
        DEBUG_OUT(d, WT588_HANDLER_LOG_TAG,
                  "start playing voice with volume %u, priority %u",
                  p_node_exec->volume, p_node_exec->priority);

        busy_found = false;
        for (uint16_t poll = 0; poll < WT588_BUSY_POLL_CNT; poll++)
        {
            if (s_wt588_handler_instance.p_wt588_driver->pf_is_busy(
                            s_wt588_handler_instance.p_wt588_driver))
            {
                busy_found = true;
                s_wt588_handler_instance.p_play_request_list->current_priority=
                s_wt588_handler_instance.p_play_request_list->highest_priority;

                HANDLER_OS_INTERFACE(&s_wt588_handler_instance)->\
                pf_os_queue_send(s_wt588_handler_instance.busy_detection_queue,
                                                                  &p_node_exec,
                                                               OSAL_MAX_DELAY);
                DEBUG_OUT(d, WT588_HANDLER_LOG_TAG,
                   "voice with priority %u is playing, sent to busy detection",
                                                        p_node_exec->priority);
                break;
            }
            HANDLER_OS_INTERFACE(&s_wt588_handler_instance)->\
                p_os_delay_interface->pf_os_delay_ms(WT588_BUSY_POLL_MS);
        }

        if (!busy_found)
        {
            fail_cnt++;
            if (fail_cnt < WT588_MAX_PLAY_RETRY)
            {
                DEBUG_OUT(w, WT588_HANDLER_LOG_TAG,
                          "busy not detected, retry %u/%u (priority=%u)",
                          fail_cnt, WT588_MAX_PLAY_RETRY,
                          p_node_exec->priority);
                HANDLER_OS_INTERFACE(&s_wt588_handler_instance)->\
                    p_os_delay_interface->pf_os_delay_ms(50);
                goto retry_play;
            }
            // max retries exhausted, node kept in list for next dispatch
            DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                      "play failed after %u retries, node kept (priority=%u)",
                      (unsigned int)WT588_MAX_PLAY_RETRY,
                      p_node_exec->priority);
        }
    }
}

static void wt588_busy_detection_thread(void * const args)
{
    // todo: wait for initialize complete

    list_voice_node_t *p_node_busy = NULL;

    DEBUG_OUT(i, WT588_HANDLER_LOG_TAG, "wt588_busy_detection_thread running");
    while (1)
    {
        HANDLER_OS_INTERFACE(&s_wt588_handler_instance)->\
        pf_os_queue_get(s_wt588_handler_instance.busy_detection_queue,
                                                         &p_node_busy,
                                                       OSAL_MAX_DELAY);

        while (s_wt588_handler_instance.p_wt588_driver->pf_is_busy(
                                      s_wt588_handler_instance.p_wt588_driver))
        {
            HANDLER_OS_INTERFACE(&s_wt588_handler_instance)->\
                p_os_delay_interface->pf_os_delay_ms(3);
        }

        HANDLER_OS_INTERFACE(&s_wt588_handler_instance)->\
        pf_os_queue_send(s_wt588_handler_instance.voice_finish_queue,
                                                        &p_node_busy,
                                                     OSAL_MAX_DELAY);
    }
}

static wt_handler_status_t wt588_handler_init(
                               bsp_wt588_handler_t  * const p_handler_instance)
{
    wt_handler_status_t  ret = WT_HANDLER_OK; 
    wt588_status_t driverRet =   WT588_ERROR;
    list_status_t    linkret =       LIST_OK;
    /************ 1.Checking input parameters ************/
    /************* 2.Checking the Resources **************/
    // has been checked in wt588_handler_inst
    driverRet = wt588_driver_inst(
            p_handler_instance->p_wt588_driver,
            (wt_sys_interface_t *)p_handler_instance->handler_input_args->
                                      p_os_interface->p_os_delay_interface,
            p_handler_instance->handler_input_args->p_busy_interface,
            p_handler_instance->handler_input_args->p_gpio_interface,
            p_handler_instance->handler_input_args->p_pwm_dma_interface);
    ret = wt588_status_to_handler_status(driverRet);
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "wt588_handler_init driver instance failed");
        goto exit;
    }

    /*************** 3. Creating Resources ***************/
    list_malloc_interface_t *p_list_malloc = \
    (list_malloc_interface_t *)HANDLER_OS_INTERFACE(p_handler_instance)->\
                                                           p_os_heap_interface;
    linkret = list_handler_cunstruct(p_handler_instance->p_play_request_list,
                                     p_list_malloc);
    if (LIST_OK != linkret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "wt588_handler_init list handler construct failed");
        ret = WT_HANDLER_ERROR;
        goto exit;
    }

    DEBUG_OUT(i, WT588_HANDLER_LOG_TAG,
              "wt588_handler_init resource create started");
    ret = HANDLER_OS_INTERFACE(p_handler_instance)->pf_os_mutex_create(
            &p_handler_instance->semaphore_handler);
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "wt588_handler_init semaphore create failed");
        goto exit;
    }

    ret = HANDLER_OS_INTERFACE(p_handler_instance)->pf_os_queue_create(
        &p_handler_instance->voice_add_queue, 10, sizeof(list_voice_node_t *));
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "wt588_handler_init handler queue create failed");
        goto cleanup_semaphore_handler;
    }

    ret = HANDLER_OS_INTERFACE(p_handler_instance)->pf_os_queue_create(
           &p_handler_instance->executor_queue, 10, sizeof(list_voice_node_t *));
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "wt588_handler_init executor queue create failed");
        goto cleanup_handler_queue;
    }

    ret = HANDLER_OS_INTERFACE(p_handler_instance)->pf_os_queue_create(
            &p_handler_instance->busy_detection_queue,
            10,
            sizeof(list_voice_node_t *));
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "wt588_handler_init busy detection queue create failed");
        goto cleanup_executor_queue;
    }

    ret = HANDLER_OS_INTERFACE(p_handler_instance)->pf_os_queue_create(
            &p_handler_instance->voice_finish_queue,
            10,
            sizeof(list_voice_node_t *));
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "wt588_handler_init voice finish queue create failed");
        goto cleanup_busy_detection_queue;
    }
    DEBUG_OUT(d, WT588_HANDLER_LOG_TAG,
              "wt588_handler_init queue setup successful");

    ret = HANDLER_OS_INTERFACE(p_handler_instance)->pf_task_create(
            &p_handler_instance->executor_thread_handle,
            NULL,
            wt588_executor_thread,
            "wt588_executor_thread",
            128 * 4,
            24);
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                           "wt588_handler_init executor thread create failed");
        goto cleanup_voice_finish_queue;
    }

    ret = HANDLER_OS_INTERFACE(p_handler_instance)->pf_task_create(
            &p_handler_instance->busy_detection_thread_handle,
            NULL,
            wt588_busy_detection_thread,
            "wt588_busy_detection_thread",
            128 * 4,
            24);
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                     "wt588_handler_init busy detection thread create failed");
        goto cleanup_executor_thread;
    }
    DEBUG_OUT(d, WT588_HANDLER_LOG_TAG,
              "wt588_handler_init resources create successful");

    DEBUG_OUT(d, WT588_HANDLER_LOG_TAG, "wt588_handler_init successful");

    goto exit;

cleanup_executor_thread:
    (void)HANDLER_OS_INTERFACE(p_handler_instance)->pf_task_delete(
            p_handler_instance->executor_thread_handle);
    p_handler_instance->executor_thread_handle = NULL;

cleanup_voice_finish_queue:
    (void)HANDLER_OS_INTERFACE(p_handler_instance)->pf_os_queue_delete(
            p_handler_instance->voice_finish_queue);
    p_handler_instance->voice_finish_queue = NULL;

cleanup_busy_detection_queue:
    (void)HANDLER_OS_INTERFACE(p_handler_instance)->pf_os_queue_delete(
        p_handler_instance->busy_detection_queue);
    p_handler_instance->busy_detection_queue = NULL;

cleanup_executor_queue:
    (void)HANDLER_OS_INTERFACE(p_handler_instance)->pf_os_queue_delete(
            p_handler_instance->executor_queue);
    p_handler_instance->executor_queue = NULL;

cleanup_handler_queue:
    (void)HANDLER_OS_INTERFACE(p_handler_instance)->pf_os_queue_delete(
            p_handler_instance->voice_add_queue);
    p_handler_instance->voice_add_queue = NULL;

cleanup_semaphore_handler:
    (void)HANDLER_OS_INTERFACE(p_handler_instance)->pf_os_mutex_delete(
            p_handler_instance->semaphore_handler);

exit:
    return ret;
}

wt_handler_status_t wt588_handler_inst(    
                             bsp_wt588_handler_t  * const   p_handler_instance,
                         wt_handler_input_args_t  * const p_handler_input_args)
{
    DEBUG_OUT(i, WT588_HANDLER_LOG_TAG, "wt588_handler_inst start");
    wt_handler_status_t ret = WT_HANDLER_OK;
    /************ 1.Checking input parameters ************/
    if (NULL == p_handler_instance                       || 
        NULL == p_handler_input_args)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                 "wt588 handler inst input error parameter");
        ret = WT_HANDLER_ERRORPARAMETER;
        return ret;
    }

    /************* 2.Checking the Resources **************/
    if (NULL == p_handler_instance->p_wt588_driver)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                 "wt588 handler inst input error resource 0");
        ret = WT_HANDLER_ERRORRESOURCE;
        return ret;
    }
    if (NULL == p_handler_input_args->list_malloc_interface                  ||
        NULL == p_handler_input_args->p_busy_interface                       ||
        NULL == p_handler_input_args->p_gpio_interface                       ||
        NULL == p_handler_input_args->p_os_interface                         ||
        NULL == p_handler_input_args->p_os_interface->p_os_delay_interface   ||
        NULL == p_handler_input_args->p_os_interface->p_os_heap_interface    ||
        NULL == p_handler_input_args->p_pwm_dma_interface)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                 "wt588 handler inst input error resource 1");
        ret = WT_HANDLER_ERRORRESOURCE;
        return ret;
    }
    if (
        NULL == p_handler_input_args->list_malloc_interface->pf_list_malloc  ||
        NULL == p_handler_input_args->list_malloc_interface->pf_list_free    ||
        NULL == p_handler_input_args->p_busy_interface->pf_is_busy           ||
        NULL == p_handler_input_args->p_gpio_interface->pf_wt_gpio_init      ||
        NULL == p_handler_input_args->p_gpio_interface->pf_wt_gpio_deinit    ||
        NULL == p_handler_input_args->p_os_interface->pf_task_create         ||
        NULL == p_handler_input_args->p_os_interface->pf_task_delete         ||
        NULL == p_handler_input_args->p_os_interface->pf_os_queue_create     ||
        NULL == p_handler_input_args->p_os_interface->pf_os_queue_delete     ||
        NULL == p_handler_input_args->p_os_interface->pf_os_queue_send       ||
        NULL == p_handler_input_args->p_os_interface->pf_os_queue_get        ||
        NULL == p_handler_input_args->p_os_interface->pf_os_queue_count      ||        
        NULL == p_handler_input_args->p_os_interface->p_os_delay_interface->\
                                                           pf_os_delay_ms    ||
        NULL == p_handler_input_args->p_os_interface->p_os_heap_interface->\
                                                           pf_os_malloc      ||
        NULL == p_handler_input_args->p_os_interface->p_os_heap_interface->\
                                                           pf_os_free        ||
        NULL == p_handler_input_args->p_pwm_dma_interface->pf_pwm_dma_init   ||
        NULL == p_handler_input_args->p_pwm_dma_interface->pf_pwm_dma_deinit ||
        NULL == p_handler_input_args->p_pwm_dma_interface->pf_pwm_dma_send_byte
        )
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                 "wt588 handler inst input error resource function");
        ret = WT_HANDLER_ERRORRESOURCE;
        return ret;
    }

    /**************** 3.Mount interfaces *****************/
    p_handler_instance->handler_input_args = p_handler_input_args;

    ret = wt588_handler_init(p_handler_instance);
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                 "wt588 handler inst init failed");
        return ret;
    }

    DEBUG_OUT(d, WT588_HANDLER_LOG_TAG, "wt588_handler_inst successful");

    return ret;
}

wt_handler_status_t wt588_handler_play_request(uint8_t volume_addr,
                                               uint8_t volume,
                                               uint8_t priority)
{
    wt_handler_status_t ret = WT_HANDLER_OK;

    if (NULL == s_wt588_handler_instance.voice_add_queue)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "play_request: handler not initialised");
        return WT_HANDLER_ERRORRESOURCE;
    }
    if (priority >= WT588_MAX_PRIORITY)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "play_request: invalid priority %u", priority);
        return WT_HANDLER_ERRORPARAMETER;
    }

    wt_os_heap_interface_t *p_heap =
        HANDLER_OS_INTERFACE(&s_wt588_handler_instance)->p_os_heap_interface;

    list_voice_node_t *p_node =
        (list_voice_node_t *)p_heap->pf_os_malloc(sizeof(list_voice_node_t));
    if (NULL == p_node)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "play_request: malloc failed");
        return WT_HANDLER_ERRORNOMEMORY;
    }

    p_node->volume_addr = volume_addr;
    p_node->volume      = volume;
    p_node->priority    = priority;
    p_node->next        = NULL;
    p_node->prev        = NULL;

    ret = HANDLER_OS_INTERFACE(&s_wt588_handler_instance)->
          pf_os_queue_send(s_wt588_handler_instance.voice_add_queue,
                           &p_node, OSAL_MAX_DELAY);
    if (WT_HANDLER_OK != ret)
    {
        p_heap->pf_os_free(p_node);
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "play_request: queue send failed");
    }
    else
    {
        DEBUG_OUT(i, WT588_HANDLER_LOG_TAG,
                  "play_request: addr=0x%02X vol=0x%02X pri=%u queued",
                  volume_addr, volume, priority);
    }

    //test hardware
    // wt588_status_t drv_ret = s_wt588_handler_instance.p_wt588_driver->pf_start_play(
    //     s_wt588_handler_instance.p_wt588_driver, volume_addr);
    // DEBUG_OUT(i, WT588_HANDLER_LOG_TAG,
    //           "play_request: start play addr=0x%02X, drv_ret=%d",
    //           volume_addr, (int)drv_ret);

    return ret;
}
//******************************* Functions *********************************//

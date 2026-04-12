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
#define HANDLER_OS_INTERFACE(INST)   (INST->handler_input_args->p_os_interface)

//******************************** Defines **********************************//

//******************************* Declaring *********************************//

//******************************* Declaring *********************************//

//******************************* Functions *********************************//
static void wt588_handler_thread(void * const args)
{
    wt_os_interface_t *p_os_interface = (wt_os_interface_t *)args;
    
    DEBUG_OUT(i, WT588_HANDLER_LOG_TAG, "wt588_handler_thread running");
    while (1)
    {
        p_os_interface->p_os_delay_interface->pf_os_delay_ms(1000);
    }
}

static void wt588_executor_thread(void * const args)
{
    wt_os_interface_t *p_os_interface = (wt_os_interface_t *)args;
    
    DEBUG_OUT(i, WT588_HANDLER_LOG_TAG, "wt588_executor_thread running");
    while (1)
    {
        p_os_interface->p_os_delay_interface->pf_os_delay_ms(1000);

    }
}

static void wt588_busy_detection_thread(void * const args)
{
    wt_os_interface_t *p_os_interface = (wt_os_interface_t *)args;
    
    DEBUG_OUT(i, WT588_HANDLER_LOG_TAG, "wt588_busy_detection_thread running");
    while (1)
    {
        p_os_interface->p_os_delay_interface->pf_os_delay_ms(1000);
    }
}

static wt_handler_status_t wt588_handler_init(
                               bsp_wt588_handler_t  * const p_handler_instance)
{
    wt_handler_status_t ret = WT_HANDLER_OK; 
    wt588_status_t driverRet = WT588_ERROR;
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
    DEBUG_OUT(i, WT588_HANDLER_LOG_TAG,
              "wt588_handler_init driver instance successful");

    p_handler_instance->handler_queue                = NULL;
    p_handler_instance->executor_queue               = NULL;
    p_handler_instance->busy_detection_queue         = NULL;
    p_handler_instance->voice_finish_queue           = NULL;

    p_handler_instance->handler_thread_handle        = NULL;
    p_handler_instance->executor_thread_handle       = NULL;
    p_handler_instance->busy_detection_thread_handle = NULL;

    ret = HANDLER_OS_INTERFACE(p_handler_instance)->pf_os_queue_create(
            &p_handler_instance->handler_queue, 10, sizeof(list_voice_node_t));
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "wt588_handler_init handler queue create failed");
        goto exit;
    }

    ret = HANDLER_OS_INTERFACE(p_handler_instance)->pf_os_queue_create(
           &p_handler_instance->executor_queue, 10, sizeof(list_voice_node_t));
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "wt588_handler_init executor queue create failed");
        goto cleanup_handler_queue;
    }

    ret = HANDLER_OS_INTERFACE(p_handler_instance)->pf_os_queue_create(
            &p_handler_instance->busy_detection_queue,
            10,
            sizeof(list_voice_node_t));
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "wt588_handler_init busy detection queue create failed");
        goto cleanup_executor_queue;
    }

    ret = HANDLER_OS_INTERFACE(p_handler_instance)->pf_os_queue_create(
            &p_handler_instance->voice_finish_queue,
            10,
            sizeof(list_voice_node_t));
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "wt588_handler_init voice finish queue create failed");
        goto cleanup_busy_detection_queue;
    }
    DEBUG_OUT(d, WT588_HANDLER_LOG_TAG,
              "wt588_handler_init queue setup successful");

    ret = HANDLER_OS_INTERFACE(p_handler_instance)->pf_task_create(
            &p_handler_instance->handler_thread_handle,
            HANDLER_OS_INTERFACE(p_handler_instance),
            wt588_handler_thread,
            "wt588_handler_thread",
            128 * 4,
            24);
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "wt588_handler_init handler thread create failed");
        goto cleanup_voice_finish_queue;
    }

    ret = HANDLER_OS_INTERFACE(p_handler_instance)->pf_task_create(
            &p_handler_instance->executor_thread_handle,
            HANDLER_OS_INTERFACE(p_handler_instance),
            wt588_executor_thread,
            "wt588_executor_thread",
            128 * 4,
            24);
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "wt588_handler_init executor thread create failed");
        goto cleanup_handler_thread;
    }

    ret = HANDLER_OS_INTERFACE(p_handler_instance)->pf_task_create(
            &p_handler_instance->busy_detection_thread_handle,
            HANDLER_OS_INTERFACE(p_handler_instance),
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
              "wt588_handler_init thread create successful");

    DEBUG_OUT(d, WT588_HANDLER_LOG_TAG, "wt588_handler_init successful");

    goto exit;

cleanup_executor_thread:
    (void)HANDLER_OS_INTERFACE(p_handler_instance)->pf_task_delete(
            p_handler_instance->executor_thread_handle);
    p_handler_instance->executor_thread_handle = NULL;

cleanup_handler_thread:
    (void)HANDLER_OS_INTERFACE(p_handler_instance)->pf_task_delete(
            p_handler_instance->handler_thread_handle);
    p_handler_instance->handler_thread_handle = NULL;

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
            p_handler_instance->handler_queue);
    p_handler_instance->handler_queue = NULL;

exit:
    return ret;
}

wt_handler_status_t wt588_handler_inst(    
                            bsp_wt588_handler_t  * const   p_handler_instance,
                        wt_handler_input_args_t  * const p_handler_input_args)
{
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
                 "wt588 handler inst input error resource");
        ret = WT_HANDLER_ERRORRESOURCE;
        return ret;
    }
    if (NULL == p_handler_input_args->list_malloc_interface                  ||
        NULL == p_handler_input_args->p_busy_interface                       ||
        NULL == p_handler_input_args->p_gpio_interface                       ||
        NULL == p_handler_input_args->p_os_interface                         ||
        NULL == p_handler_input_args->p_os_interface->p_os_delay_interface   ||
        NULL == p_handler_input_args->p_pwm_dma_interface)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                 "wt588 handler inst input error resource");
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
        NULL == p_handler_input_args->p_os_interface->p_os_delay_interface->\
                                                           pf_os_delay_ms    ||
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
//******************************* Functions *********************************//

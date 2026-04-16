/******************************************************************************
 * @file bsp_mpuxxxx_handler.c
 *
 * @par dependencies
 * - bsp_mpuxxxx_handler.h
 *
 * @author Ethan-Hang
 *
 * @brief   MPU6050 handler layer implementation.
 *          Provides handler instantiation, callback
 *          registration and the FreeRTOS task thread
 *          for DMA-based data acquisition.
 *
 * Processing flow:
 *   1. mpuxxxx_handler_thread() receives input args.
 *   2. Circular buffer is initialized.
 *   3. Handler instance is created via
 *      mpuxxxx_hanlder_inst().
 *   4. Task loop polls DMA complete flag; on flag
 *      set, sends a msg to the unpack queue.
 *
 * @version V1.0 2026-3-2
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "bsp_mpuxxxx_handler.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define HANLDER_IS_INITED                              true
#define HANLDER_NOT_INITED                            false

static bool g_mpuxxxx_hanler_is_inited         = HANLDER_NOT_INITED;
static bsp_mpuxxxx_hanlder_t g_mpuxxxx_handler_instance =       {0};

extern void (*pf_pin_interrupt_callback)(void const * const, void * const);
extern void (*pf_dma_interrupt_callback)(void const * const, void * const);


//******************************** Defines **********************************//

//******************************* Declaring *********************************//
/**
 * @brief   Register the GPIO pin interrupt callback.
 *          Assigns the given function pointer to
 *          pf_pin_interrupt_callback.
 * 
 * @param[in] callback : Function pointer to the pin
 *                       interrupt handler.
 * 
 * @param[out] : None
 * 
 * @return  None
 * 
 * */
void register_callback(void (*callback)(void const * const, void * const))
{
    pf_pin_interrupt_callback = callback;
}

/**
 * @brief   Register the DMA complete interrupt callback.
 *          Assigns the given function pointer to
 *          pf_dma_interrupt_callback.
 * 
 * @param[in] callback : Function pointer to the DMA
 *                       interrupt handler.
 * 
 * @param[out] : None
 * 
 * @return  None
 * 
 * */
void register_callback_dma(void (*callback)(void const * const, void * const))
{
    pf_dma_interrupt_callback = callback;
}

/**
 * @brief   Internal initialization of the MPU handler.
 *          Creates the OS unpack queue, then calls
 *          the driver initialization. Skips if the
 *          handler is already initialized.
 * 
 * @param[in] p_handler : Pointer to the handler
 *                        instance to be initialized.
 * 
 * @param[out] : None
 * 
 * @return  MPUXXXX_OK             - Success
 *          MPUXXXX_ERRORPARAMETER - Queue/driver error
 * 
 * */
static mpuxxxx_status_t mpuxxxx_hanlder_init(bsp_mpuxxxx_hanlder_t * const p_handler)
{
    if (HANLDER_IS_INITED == g_mpuxxxx_hanler_is_inited)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx handler has been inited!");
        return MPUXXXX_OK;
    }

    mpuxxxx_status_t ret = MPUXXXX_OK;

    ret = p_handler->p_input_args->p_os_interface->pf_os_queue_create(
                                                    p_handler->queue_length,
                                                 p_handler->queue_item_size,
                               (void **)(&p_handler->p_unpack_queue_handler) );
    if (ret != MPUXXXX_OK)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxx handler inst os queue create failed\n");
        return ret;
    }
    DEBUG_OUT(d, MPUXXXX_LOG_TAG, "mpuxxxx handler os queue created successfully");

    ret = bsp_mpuxxxx_driver_inst(
        p_handler->p_driver,
        p_handler->p_input_args->p_iic_driver,
        p_handler->p_input_args->p_timebase,
        p_handler->p_input_args->p_delay_interface,
#if OS_SUPPORTING
        p_handler->p_input_args->p_yield_interface,
        p_handler->p_input_args->p_os_interface,
#endif // OS_SUPPORTING
        register_callback,
        register_callback_dma
    );
    if (ret != MPUXXXX_OK)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxx handler inst driver init failed\n");
        return ret;
    }
    DEBUG_OUT(d, MPUXXXX_LOG_TAG, "mpuxxxx handler driver initialized successfully");

    return ret;
} 

/**
 * @brief   Instantiate the MPU handler.
 *          Binds the input arguments to the handler
 *          and triggers the internal init routine.
 * 
 * @param[in] p_handler    : Pointer to the handler
 *                           instance.
 * @param[in] p_input_args : Pointer to the init args,
 *                           including OS, IIC, delay
 *                           and yield interfaces.
 * 
 * @param[out] : None
 * 
 * @return  MPUXXXX_OK             - Success
 *          MPUXXXX_ERRORPARAMETER - NULL pointer input
 *          Other                  - Init error
 * 
 * */
mpuxxxx_status_t mpuxxxx_hanlder_inst(bsp_mpuxxxx_hanlder_t      *   p_handler, 
                                    mpuxxxx_handler_input_args_t *p_input_args)
{
    mpuxxxx_status_t ret = MPUXXXX_OK;
    if (NULL == p_handler || NULL == p_input_args)
    {
        ret = MPUXXXX_ERRORPARAMETER;
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx handler input error parameter!");
        return ret;
    }

    p_handler->p_input_args = p_input_args;

    ret = mpuxxxx_hanlder_init(p_handler);
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx handler init failed!");
        return ret;
    }
    g_mpuxxxx_hanler_is_inited = HANLDER_IS_INITED;
    
    return ret;
}                                    

/**
 * @brief   Get a read-only pointer to the handler instance.
 *          External modules must use this instead of extern.
 *
 * @return  Const pointer to the singleton handler instance.
 *
 * */
bsp_mpuxxxx_hanlder_t const * mpuxxxx_handler_get_instance(void)
{
    if (g_mpuxxxx_hanler_is_inited != HANLDER_IS_INITED)
    {
        return NULL;
    }
    return &g_mpuxxxx_handler_instance;
}

/**
 * @brief   FreeRTOS task for the MPU handler.
 *          Initializes the circular buffer, driver
 *          and handler instance. Runs a polling loop
 *          that monitors the DMA completion flag;
 *          on flag asserted, sends a notification
 *          to the unpack queue.
 * 
 * @param[in] argument : Pointer to
 *                       mpuxxxx_handler_input_args_t
 *                       containing all required OS,
 *                       IIC, delay and yield
 *                       interfaces.
 * 
 * @param[out] : None
 * 
 * @return  None
 * 
 * */
void mpuxxxx_handler_thread(void *argument)
{
    mpuxxxx_status_t ret = MPUXXXX_OK;
    if (NULL == argument)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx handler thread input error parameter!");
        return;
    }

    mpuxxxx_handler_input_args_t *p_input_args =\
                  (mpuxxxx_handler_input_args_t *)argument;

    circular_buffer_init(circular_buffer_get_instance(), 10);

    bsp_mpuxxxx_driver_t driver;
    g_mpuxxxx_handler_instance.p_driver               = &driver;
    g_mpuxxxx_handler_instance.p_unpack_queue_handler = NULL;
    g_mpuxxxx_handler_instance.queue_item_size        = 1;
    g_mpuxxxx_handler_instance.queue_length           = 20;

    ret = mpuxxxx_hanlder_inst(&g_mpuxxxx_handler_instance, p_input_args);
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx handler inst failed!");
        return;
    }

    for (;;)
    {
        if (mpuxxxx_dma_flag_get())
        {
            uint32_t token = 1U;
            ret = g_mpuxxxx_handler_instance.p_input_args->p_os_interface->   \
            pf_os_queue_send(g_mpuxxxx_handler_instance.p_unpack_queue_handler,
                                                                     &token, 0);

            mpuxxxx_dma_flag_set(false);
        }
        if (MPUXXXX_OK != ret)
        {
            DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx handler send queue failed!");
        }

        p_input_args->p_yield_interface->pf_rtos_yield(1);
    }
}
//******************************* Declaring *********************************//




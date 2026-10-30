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
#include "board_types.h"
#include "bsp_mpuxxxx_handler.h"
#include "osal_error.h"
#include "osal_sema.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define HANLDER_IS_INITED                              true
#define HANLDER_NOT_INITED                            false

/* Mirrors the driver-private constants used for the MPU6050 DMA read. */
#define MPU6050_IIC_MEM_SIZE_8BIT                          1U

static BOOL g_mpuxxxx_hanler_is_inited         = HANLDER_NOT_INITED;
static bsp_mpuxxxx_hanlder_t g_mpuxxxx_handler_instance =       {0};

//******************************** Defines **********************************//

//******************************* Declaring *********************************//
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

    mpuxxxx_deps_t const * const p_deps = p_handler->p_input_args->p_deps;

    /* Counting semaphore rather than a queue: nothing is transported, only
       counted, and the ring already holds the bytes. Ceiling is the ring's
       usable depth so the two can never disagree. Created through OSAL
       directly -- this object belongs to the handler / consumer pair, not
       to the chip, so it has no business in the driver's deps vtable. */
    if (OSAL_SUCCESS != osal_sema_init_counting(&p_handler->unpack_sema_handle,
                                                MPUXXXX_RING_SLOTS - 1U,
                                                0U))
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                     "mpuxxx handler inst unpack sema create failed\n");
        return MPUXXXX_ERRORNOMEMORY;
    }
    DEBUG_OUT(d, MPUXXXX_LOG_TAG, "mpuxxxx handler unpack sema created successfully");

    /* deps carry the bus / OS / IRQ wiring and the two ISR trampolining
       hooks; cfg carries the tuning. Both come straight from integration,
       the handler does not interpret either. */
    ret = bsp_mpuxxxx_driver_inst(p_handler->p_driver,
                                  p_deps,
                                  p_handler->p_input_args->p_cfg);
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

    circular_buffer_init(circular_buffer_get_instance(), MPUXXXX_RING_SLOTS);

    bsp_mpuxxxx_driver_t driver = {0};
    g_mpuxxxx_handler_instance.p_driver           = &driver;
    g_mpuxxxx_handler_instance.unpack_sema_handle = NULL;
    g_mpuxxxx_handler_instance.dropped_count      = 0;

    /* The task this thread runs on is the one the data-ready EXTI has to
       wake; bsp_mpuxxxx_driver_inst() captures it itself now, since ctx is
       opaque and the handler can no longer reach into the driver object. */
    ret = mpuxxxx_hanlder_inst(&g_mpuxxxx_handler_instance, p_input_args);
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx handler inst failed!");
        return;
    }

    iic_driver_interface_t const *p_iic = p_input_args->p_deps->p_iic_interface;
    os_interface_t         const *p_os  = p_input_args->p_deps->p_os_interface;

    const UINT16_t k_dev_addr = (UINT16_t)((MPU_ADDR << 1) | 1);

    /* Read back whatever INT_ENABLE the applied cfg actually produced, so
       the mask / re-arm pair below cannot drift away from the tuning. */
    UINT8_t k_int_on = 0;
    if (MPUXXXX_OK != driver.ops->pf_read_reg(&driver, MPU_INT_EN_REG,
                                              &k_int_on))
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                     "mpuxxxx handler read INT_EN failed, using DATA_RDY");
        k_int_on = (UINT8_t)DATA_RDY_EN_BIT(1);
    }

    for (;;)
    {
        UINT32_t notify_val = 0;

        /* 1. Wait for data-ready EXTI notification from int_interrupt_callback. */
        ret = p_os->pf_os_semaphore_wait_notify(0, 
                                                OSAL_MAX_DELAY,
                                                &notify_val, 
                                                OSAL_MAX_DELAY);
        if (MPUXXXX_OK != ret)
        {
            continue;
        }

        circular_buffer_t *p_cbuf = circular_buffer_get_instance();

        /* 2. Refuse to over-commit the ring. The semaphore counts slots,
              but only FreeRTOS enforces its ceiling, so the guard that
              actually holds on both backends is this one -- asked of the
              structure that owns the storage. Without it a lagging
              consumer does not lose one packet, it acquires a permanent
              phase lag and eventually reads the slot the DMA is writing. */
        if (p_cbuf->pf_is_full(p_cbuf))
        {
            g_mpuxxxx_handler_instance.dropped_count++;
            DEBUG_OUT(w, MPUXXXX_LOG_TAG,
                         "mpuxxxx ring full, packet dropped (total %u)",
                         (unsigned int)g_mpuxxxx_handler_instance.dropped_count);
            (void)driver.ops->pf_write_reg(&driver, MPU_INT_EN_REG, k_int_on);
            continue;
        }

        /* 3. Mask MPU-side interrupt so no new EXTI fires while the DMA
              transfer is in flight. I2C write; internal bus mutex. */
        (void)driver.ops->pf_write_reg(&driver, MPU_INT_EN_REG,
                                      (UINT8_t)COLOSE_ALL);

        /* 4. Submit DMA read and wait for completion in I2C abstraction. */
        UINT8_t *p_wbuff = p_cbuf->pf_get_wbuffer_addr(p_cbuf);
        ret = p_iic->pf_iic_mem_read_dma(p_iic->hi2c,
                                         k_dev_addr,
                                         MPU_ACCEL_XOUTH_REG,
                                         MPU6050_IIC_MEM_SIZE_8BIT,
                                         p_wbuff,
                                         MPUXXXX_DATA_PACKET_SIZE);
        if (MPUXXXX_OK != ret)
        {
            DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                         "mpuxxxx handler start dma failed, ret:%d", (int)ret);
            (void)driver.ops->pf_write_reg(&driver, MPU_INT_EN_REG, k_int_on);
            continue;
        }

        /* 5. Commit the slot, then hand the consumer one permit for it.
              Order matters: the slot must be readable before the permit
              that points at it exists. */
        p_cbuf->pf_data_writed(p_cbuf);

        if (OSAL_SUCCESS !=
                osal_sema_give(g_mpuxxxx_handler_instance.unpack_sema_handle))
        {
            DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                         "mpuxxxx unpack sema give failed");
        }

        /* 6. Re-arm MPU INT for the next sample. */
        (void)driver.ops->pf_write_reg(&driver, MPU_INT_EN_REG, k_int_on);
    }
}

//******************************* Declaring *********************************//

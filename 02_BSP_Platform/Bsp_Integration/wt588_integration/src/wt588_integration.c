/******************************************************************************
 * @file wt588_intergration.c
 *
 * @par dependencies
 * - wt588_intergration.h
 * - tim.h
 * - osal_wrapper_adapter.h
 * - osal_error.h
 *
 * @author Ethan-Hang
 *
 * @brief Dependency injection for the WT588 voice handler.
 *
 * Provides concrete implementations for every interface the WT588 handler
 * needs (GPIO, busy-detect, PWM-DMA, OS tasks/queues/mutexes, heap) and
 * wires them into wt588_handler_input_args / wt588_handler_instance, which
 * are passed to wt588_handler_inst() at startup.
 *
 * PWM-DMA protocol (WT588F02B one-wire serial, single-byte command):
 *   cmp_buff[0..6]  = 0     → 5.6 ms start-low (7 × 800 µs periods)
 *   cmp_buff[7..14] = CCR   → 8 data bits LSB-first
 *                             '1' → CCR = 600 (high 600 µs / low 200 µs)
 *                             '0' → CCR = 200 (high 200 µs / low 600 µs)
 *   cmp_buff[15]    = 800   → guard period (one full high period)
 *   TIM2 ARR = 800, PSC gives 1 µs tick → period = 800 µs
 *
 * Changing the target MCU or RTOS requires only editing this file.
 *
 * @version V1.0 2026-04-16
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "wt588_integration.h"
#include "tim.h"
#include "main.h"
#include "osal_wrapper_adapter.h"
#include "osal_error.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/** Timeout (ms) to wait for an ongoing DMA transfer to finish. */
#define WT588_DMA_SEND_TIMEOUT_MS   (200U)

/** DMA buffer length: 7 start bits + 8 data bits + 1 guard = 16 words. */
#define WT588_DMA_BUFF_LEN          (16U)

/** CCR value for data bit '1'. */
#define WT588_CCR_BIT1              (600U)

/** CCR value for data bit '0'. */
#define WT588_CCR_BIT0              (200U)

/** CCR value for the guard period (full high period). */
#define WT588_CCR_GUARD             (800U)
//******************************** Defines **********************************//

//******************************* Declaring *********************************//

/* ---------- Forward declarations of adapter functions --------------------- */
static wt588_status_t     gpio_init_adapter        (void);
static void               gpio_deinit_adapter      (void);
static bool               busy_is_busy_adapter     (void);
static wt588_status_t     pwm_dma_init_adapter     (void);
static void               pwm_dma_deinit_adapter   (void);
static wt588_status_t     pwm_dma_send_byte_adapter(uint8_t data);

static wt_handler_status_t task_create_adapter (void **const   task_handle,
                                                void          *parameters,
                                       void (*entry)(void *const args),
                                                char const    *name,
                                                uint16_t       stack_depth,
                                                uint32_t       priority);
static wt_handler_status_t task_delete_adapter (void *const task_handle);

static wt_handler_status_t queue_create_adapter(void **const  queue_handle,
                                                uint32_t const queue_length,
                                                uint32_t const item_size);
static wt_handler_status_t queue_delete_adapter(void *const queue_handle);
static wt_handler_status_t queue_send_adapter  (void *const queue_handle,
                                                void *const item,
                                                uint32_t    timeout);
static wt_handler_status_t queue_get_adapter   (void *const queue_handle,
                                                void *const item,
                                                uint32_t    timeout);
static uint32_t            queue_count_adapter (void *const queue_handle);

static wt_handler_status_t mutex_create_adapter(void **const mutex_handle);
static wt_handler_status_t mutex_delete_adapter(void *const  mutex_handle);
static wt_handler_status_t mutex_lock_adapter  (void *const  mutex_handle,
                                                uint32_t     timeout);
static wt_handler_status_t mutex_unlock_adapter(void *const  mutex_handle);

static void os_delay_ms_adapter(uint32_t ms);

//******************************* Declaring *********************************//

//******************************* Variables *********************************//

/** DMA busy flag: set before HAL_DMA_Start_IT, cleared in DMA-complete ISR. */
static volatile bool s_tx_busy = false;

/** DMA CCR buffer — static so it remains valid during the async DMA transfer. */
static uint32_t s_cmp_buff[WT588_DMA_BUFF_LEN];

//******************************* Variables *********************************//

//******************************* Functions *********************************//

/* ========================================================================= */
/*  PWM-DMA ISR callback                                                      */
/* ========================================================================= */

/**
 * @brief DMA transfer-complete callback registered with HAL.
 *
 * Stops the TIM2 PWM output and clears the busy flag.
 */
static void wt588_dma_cplt_callback(DMA_HandleTypeDef *hdma)
{
    if (hdma == htim2.hdma[TIM_DMA_ID_UPDATE])
    {
        HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
        s_tx_busy = false;
    }
}

/* ========================================================================= */
/*  GPIO interface                                                             */
/* ========================================================================= */

/**
 * @brief GPIO init — TIM2_CH1 alternate function is already configured by
 *        MX_TIM2_Init() in STM32CubeMX-generated code, so nothing extra
 *        is required here.
 */
static wt588_status_t gpio_init_adapter(void)
{
    return WT588_OK;
}

static void gpio_deinit_adapter(void)
{
}

/* ========================================================================= */
/*  Busy interface                                                             */
/* ========================================================================= */

/**
 * @brief Report busy state by reading the hardware WT_BUSY pin.
 *        High level = busy (audio playing), low level = idle.
 */
static bool busy_is_busy_adapter(void)
{
    return (HAL_GPIO_ReadPin(WT_BUSY_GPIO_Port, WT_BUSY_Pin) == GPIO_PIN_SET);
}

/* ========================================================================= */
/*  PWM-DMA interface                                                         */
/* ========================================================================= */

/**
 * @brief Initialise the PWM-DMA path.
 *
 * Registers the DMA-complete callback, enables the TIM2 update-event DMA
 * request, then waits 200 ms for the WT588F02B to complete its power-on
 * sequence.
 */
static wt588_status_t pwm_dma_init_adapter(void)
{
    HAL_DMA_RegisterCallback(htim2.hdma[TIM_DMA_ID_UPDATE],
                             HAL_DMA_XFER_CPLT_CB_ID,
                             wt588_dma_cplt_callback);
    __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_UPDATE);
    osal_task_delay(200U); /* WT588F02B power-on delay */
    return WT588_OK;
}

static void pwm_dma_deinit_adapter(void)
{
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
    __HAL_TIM_DISABLE_DMA(&htim2, TIM_DMA_UPDATE);
}

/**
 * @brief Send one command byte to the WT588F02B via TIM2 PWM + DMA.
 *
 * Blocks (via RTOS delay) until any previous transfer completes, then
 * builds the CCR buffer and starts a new non-blocking DMA transfer.
 *
 * @param data  Command byte (play index, stop code, or volume code).
 * @return WT588_OK on success, WT588_ERRORTIMEOUT if previous tx did not
 *         finish within WT588_DMA_SEND_TIMEOUT_MS.
 */
static wt588_status_t pwm_dma_send_byte_adapter(uint8_t data)
{
    /* Wait for any previous transfer to finish */
    uint32_t timeout_ms = WT588_DMA_SEND_TIMEOUT_MS;
    while (s_tx_busy && (timeout_ms > 0U))
    {
        osal_task_delay(1U);
        timeout_ms--;
    }
    if (s_tx_busy)
    {
        return WT588_ERRORTIMEOUT;
    }

    /* Build CCR buffer:
     *   [0..6]  = 0       → 5.6 ms start-low (7 zero-duty periods)
     *   [7..14] = CCR     → 8 data bits, LSB first
     *   [15]    = GUARD   → one full high guard period
     */
    for (uint8_t i = 0U; i < 7U; i++)
    {
        s_cmp_buff[i] = 0U;
    }
    uint8_t d = data;
    for (uint8_t i = 7U; i < 15U; i++)
    {
        s_cmp_buff[i] = ((d & 0x01U) != 0U) ? WT588_CCR_BIT1 : WT588_CCR_BIT0;
        d >>= 1U;
    }
    s_cmp_buff[15] = WT588_CCR_GUARD;

    /* Start PWM and DMA */
    s_tx_busy = true;
    HAL_StatusTypeDef hal_ret;

    hal_ret = HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    DEBUG_OUT(d, WT588_LOG_TAG,
              "pwm_dma_send: PWM_Start ret=%d", (int)hal_ret);

    hal_ret = HAL_DMA_Start_IT(htim2.hdma[TIM_DMA_ID_UPDATE],
                               (uint32_t)s_cmp_buff,
                               (uint32_t)&htim2.Instance->CCR1,
                               WT588_DMA_BUFF_LEN);
    DEBUG_OUT(d, WT588_LOG_TAG,
              "pwm_dma_send: DMA_Start_IT ret=%d, DMA_State=%d",
              (int)hal_ret,
              (int)htim2.hdma[TIM_DMA_ID_UPDATE]->State);

    if (HAL_OK != hal_ret)
    {
        s_tx_busy = false;
        HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
        return WT588_ERROR;
    }

    return WT588_OK;
}

/* ========================================================================= */
/*  OS adapter functions                                                      */
/* ========================================================================= */

static void os_delay_ms_adapter(uint32_t ms)
{
    (void)osal_task_delay((osal_tick_type_t)ms);
}

static wt_handler_status_t task_create_adapter(void **const   task_handle,
                                               void          *parameters,
                                      void (*entry)(void *const args),
                                               char const    *name,
                                               uint16_t       stack_depth,
                                               uint32_t       priority)
{
    int32_t ret = osal_task_create(
                      (osal_task_handle_t *)task_handle,
                      name,
                      parameters,
                      (osal_task_entry_t)entry,
                      (uint32_t)stack_depth,
                      priority);
    return (OSAL_SUCCESS == ret) ? WT_HANDLER_OK : WT_HANDLER_ERROR;
}

static wt_handler_status_t task_delete_adapter(void *const task_handle)
{
    int32_t ret = osal_task_delete((osal_task_handle_t)task_handle);
    return (OSAL_SUCCESS == ret) ? WT_HANDLER_OK : WT_HANDLER_ERROR;
}

static wt_handler_status_t queue_create_adapter(void **const  queue_handle,
                                                uint32_t const queue_length,
                                                uint32_t const item_size)
{
    int32_t ret = osal_queue_create(
                      (osal_queue_handle_t *)queue_handle,
                      (size_t)queue_length,
                      (size_t)item_size);
    return (OSAL_SUCCESS == ret) ? WT_HANDLER_OK : WT_HANDLER_ERROR;
}

static wt_handler_status_t queue_delete_adapter(void *const queue_handle)
{
    osal_queue_delete((osal_queue_handle_t)queue_handle);
    return WT_HANDLER_OK;
}

static wt_handler_status_t queue_send_adapter(void *const queue_handle,
                                              void *const item,
                                              uint32_t    timeout)
{
    int32_t ret = osal_queue_send((osal_queue_handle_t)queue_handle,
                                  item,
                                  (osal_tick_type_t)timeout);
    return (OSAL_SUCCESS == ret) ? WT_HANDLER_OK : WT_HANDLER_ERROR;
}

static wt_handler_status_t queue_get_adapter(void *const queue_handle,
                                             void *const item,
                                             uint32_t    timeout)
{
    int32_t ret = osal_queue_receive((osal_queue_handle_t)queue_handle,
                                     item,
                                     (osal_tick_type_t)timeout);
    return (OSAL_SUCCESS == ret) ? WT_HANDLER_OK : WT_HANDLER_ERROR;
}

static uint32_t            queue_count_adapter (void *const queue_handle)
{
    int32_t ret = osal_queue_messages_waiting((osal_queue_handle_t)queue_handle);
    return (ret >= 0) ? (uint32_t)ret : 0U;
}

static wt_handler_status_t mutex_create_adapter(void **const mutex_handle)
{
    int32_t ret = osal_mutex_init((osal_mutex_handle_t *)mutex_handle);
    return (OSAL_SUCCESS == ret) ? WT_HANDLER_OK : WT_HANDLER_ERROR;
}

static wt_handler_status_t mutex_delete_adapter(void *const mutex_handle)
{
    osal_mutex_delete((osal_mutex_handle_t)mutex_handle);
    return WT_HANDLER_OK;
}

static wt_handler_status_t mutex_lock_adapter(void *const mutex_handle,
                                              uint32_t    timeout)
{
    int32_t ret = osal_mutex_take((osal_mutex_handle_t)mutex_handle,
                                  (osal_tick_type_t)timeout);
    if (OSAL_SUCCESS == ret)
    {
        return WT_HANDLER_OK;
    }
    return (OSAL_ERROR_TIMEOUT == ret) ? WT_HANDLER_ERRORTIMEOUT
                                       : WT_HANDLER_ERROR;
}

static wt_handler_status_t mutex_unlock_adapter(void *const mutex_handle)
{
    int32_t ret = osal_mutex_give((osal_mutex_handle_t)mutex_handle);
    return (OSAL_SUCCESS == ret) ? WT_HANDLER_OK : WT_HANDLER_ERROR;
}

/* ========================================================================= */
/*  Static interface structs                                                  */
/* ========================================================================= */

static wt_gpio_interface_t s_gpio_interface = {
    .pf_wt_gpio_init   = gpio_init_adapter,
    .pf_wt_gpio_deinit = gpio_deinit_adapter,
};

static wt_busy_interface_t s_busy_interface = {
    .pf_is_busy = busy_is_busy_adapter,
};

static wt_pwm_dma_interface_t s_pwm_dma_interface = {
    .pf_pwm_dma_init      = pwm_dma_init_adapter,
    .pf_pwm_dma_deinit    = pwm_dma_deinit_adapter,
    .pf_pwm_dma_send_byte = pwm_dma_send_byte_adapter,
};

static wt_os_delay_interface_t s_os_delay_interface = {
    .pf_os_delay_ms = os_delay_ms_adapter,
};

static wt_os_heap_interface_t s_os_heap_interface = {
    .pf_os_malloc = osal_heap_malloc,
    .pf_os_free   = osal_heap_free,
};

static list_malloc_interface_t s_list_malloc_interface = {
    .pf_list_malloc = osal_heap_malloc,
    .pf_list_free   = osal_heap_free,
};

static wt_os_interface_t s_os_interface = {
    .pf_task_create         = task_create_adapter,
    .pf_task_delete         = task_delete_adapter,
    .pf_os_queue_create     = queue_create_adapter,
    .pf_os_queue_delete     = queue_delete_adapter,
    .pf_os_queue_send       = queue_send_adapter,
    .pf_os_queue_get        = queue_get_adapter,
    .pf_os_queue_count      = queue_count_adapter,
    .pf_os_mutex_create     = mutex_create_adapter,
    .pf_os_mutex_delete     = mutex_delete_adapter,
    .pf_os_mutex_lock       = mutex_lock_adapter,
    .pf_os_mutex_unlock     = mutex_unlock_adapter,
    .p_os_delay_interface   = &s_os_delay_interface,
    .p_os_heap_interface    = &s_os_heap_interface,
};

/* ========================================================================= */
/*  Exported instances                                                       */
/* ========================================================================= */

/**
 * @brief Assembled input args — all const-pointer fields initialised here.
 *        Passed to wt588_handler_inst() as the second argument.
 */
wt_handler_input_args_t wt588_handler_input_args = {
    .p_os_interface        = &s_os_interface,
    .p_busy_interface      = &s_busy_interface,
    .p_gpio_interface      = &s_gpio_interface,
    .p_pwm_dma_interface   = &s_pwm_dma_interface,
    .list_malloc_interface = &s_list_malloc_interface,
};

//******************************* Functions *********************************//

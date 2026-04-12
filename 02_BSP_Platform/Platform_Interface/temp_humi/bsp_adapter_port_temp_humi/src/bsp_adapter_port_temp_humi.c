/******************************************************************************
 * @file bsp_adapter_port_temp_humi.c
 *
 * @par dependencies
 * - i2c_port.h
 * - bsp_temp_humi_xxx_handler.h
 * - bsp_adapter_port_temp_humi.h
 * - bsp_wrapper_temp_humi.h
 * - osal_wrapper_adapter.h
 * - osal_error.h
 * - Debug.h
 *
 * @author Ethan-Hang
 *
 * @brief Adapter port for AHT21 temperature/humidity sensor.
 *
 * Provides concrete implementations for bsp_wrapper_temp_humi's vtable.
 * Each read function is made synchronous from the caller's perspective:
 *
 *   caller ─► aht21_read_temp_humi()
 *                ├─ bsp_temp_humi_xxx_read()  ── posts event to handler queue
 *                └─ osal_event_group_wait_bits()  ── blocks caller
 *                       ↑
 *              handler thread reads AHT21 → calls aht21_data_ready_cb()
 *                → stores result in s_temp_result / s_humi_result
 *                → sets event group bits  →  caller unblocks
 *   caller ◄── *temp / *humi filled with fresh values
 *
 * The event group is created lazily on the first read call.
 * Handler and driver layers are NOT modified.
 *
 * @version V1.0 2026-04-12
 * @version V2.0 2026-04-12
 * @upgrade 2.0: Event-group synchronisation added in porting layer.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "i2c_port.h"
#include "bsp_temp_humi_xxx_handler.h"
#include "bsp_adapter_port_temp_humi.h"
#include "bsp_wrapper_temp_humi.h"
#include "osal_wrapper_adapter.h"
#include "osal_error.h"
#include "Debug.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/** Wait up to 5 s for the handler to complete a read before giving up. */
#define ADAPTER_EG_READ_TIMEOUT_TICKS  (5000U)

/** Event group bit set by aht21_data_ready_cb when temperature is ready. */
#define ADAPTER_EG_BIT_TEMP  (1U << 0)
/** Event group bit set by aht21_data_ready_cb when humidity is ready. */
#define ADAPTER_EG_BIT_HUMI  (1U << 1)
/** Combined mask for both bits. */
#define ADAPTER_EG_BIT_BOTH  (ADAPTER_EG_BIT_TEMP | ADAPTER_EG_BIT_HUMI)
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
static void aht21_drv_init      (temp_humi_drv_t * const dev);
static void aht21_drv_deinit    (temp_humi_drv_t * const dev);
static void aht21_read_temp     (temp_humi_drv_t * const dev, float * const temp);
static void aht21_read_humi     (temp_humi_drv_t * const dev, float * const humi);
static void aht21_read_temp_humi(temp_humi_drv_t * const dev,
                                           float * const temp,
                                           float * const humi);
static void aht21_data_ready_cb (float *temp, float *humi);
//******************************* Declaring *********************************//

//******************************** Variables ********************************//
/**
 * Result storage written by aht21_data_ready_cb() and read back by the
 * aht21_read_* functions after the event group unblocks them.
 * Named "result" (not "last") to avoid confusion with the handler's
 * lifetime-based data-age tracking.
 */
static volatile float              s_temp_result  = 0.0f;
static volatile float              s_humi_result  = 0.0f;

/**
 * Event group created lazily on the first read call.
 * Bits ADAPTER_EG_BIT_TEMP / _HUMI are set by aht21_data_ready_cb().
 */
static osal_event_group_handle_t   s_eg_handle    = NULL;

/**
 * Type of the in-flight read request; used by aht21_data_ready_cb() to
 * decide which bits to set.
 */
static temp_humi_data_type_event_t s_pending_type = TEMP_HUMI_EVENT_BOTH;
//******************************** Variables ********************************//

//******************************* Functions *********************************//

/* ---------- Internal helpers ---------------------------------------------- */

/**
 * @brief Ensure the event group has been created (lazy init).
 *
 * Safe to call multiple times; returns false only if creation fails.
 */
static bool adapter_eg_ensure_init(void)
{
    if (NULL != s_eg_handle)
    {
        return true;
    }

    int32_t ret = osal_event_group_create(&s_eg_handle);
    if (OSAL_SUCCESS != ret)
    {
        DEBUG_OUT(e, TEMP_HUMI_ERR_LOG_TAG,
                  "adapter: event group create failed (%d)", (int)ret);
        return false;
    }

    return true;
}

/**
 * @brief Submit a read event and block until the callback signals completion.
 *
 * @param[in] event_type  Which data to read.
 * @param[in] bits        Event group bit(s) to wait for.
 */
static void adapter_sync_read(temp_humi_data_type_event_t event_type,
                               uint32_t                    bits)
{
    if (!adapter_eg_ensure_init())
    {
        return;
    }

    s_pending_type = event_type;

    temp_humi_xxx_event_t event = {
        .event_type  = event_type,
        .lifetime    = 0,
        .pf_callback = aht21_data_ready_cb,
    };

    temp_humi_status_t ret = bsp_temp_humi_xxx_read(&event);
    if (TEMP_HUMI_OK != ret)
    {
        DEBUG_OUT(e, TEMP_HUMI_ERR_LOG_TAG,
                  "adapter_sync_read: post failed (%d)", (int)ret);
        return;
    }

    int32_t eg_ret = osal_event_group_wait_bits(
                        s_eg_handle,
                        bits,
                        true,                          /* clear_on_exit */
                        false,                         /* wait_for_all  */
                        ADAPTER_EG_READ_TIMEOUT_TICKS,
                        NULL);

    if (OSAL_SUCCESS != eg_ret)
    {
        DEBUG_OUT(e, TEMP_HUMI_ERR_LOG_TAG,
                  "adapter_sync_read: wait timeout");
    }
}

/**
 * @brief Callback invoked by the handler thread after sensor data is read.
 *
 * Runs in the handler task context.  Stores the sensor values and sets the
 * event group bits so the waiting caller task is unblocked.
 */
static void aht21_data_ready_cb(float *temp, float *humi)
{
    uint32_t bits = 0U;

    if (temp != NULL) { s_temp_result = *temp; }
    if (humi != NULL) { s_humi_result = *humi; }

    switch (s_pending_type)
    {
    case TEMP_HUMI_EVENT_TEMP:
        bits = ADAPTER_EG_BIT_TEMP;
        break;
    case TEMP_HUMI_EVENT_HUMI:
        bits = ADAPTER_EG_BIT_HUMI;
        break;
    case TEMP_HUMI_EVENT_BOTH:
    default:
        bits = ADAPTER_EG_BIT_BOTH;
        break;
    }

    if (NULL != s_eg_handle)
    {
        (void)osal_event_group_set_bits(s_eg_handle, bits);
    }
}

/* ---------- Wrapper vtable implementations -------------------------------- */

bool drv_adapter_temp_humi_register(void)
{
    temp_humi_drv_t temp_humi_drv = {
        .idx                         = 0,
        .dev_id                      = 0,
        .pf_temp_humi_drv_init       = aht21_drv_init,
        .pf_temp_humi_drv_deinit     = aht21_drv_deinit,
        .pf_temp_humi_read_temp      = aht21_read_temp,
        .pf_temp_humi_read_humi      = aht21_read_humi,
        .pf_temp_humi_read_all       = aht21_read_temp_humi,
    };

    return drv_adapter_temp_humi_mount(0, &temp_humi_drv);
}

static void aht21_drv_init(temp_humi_drv_t * const dev)
{
    /* Handler thread initialises itself at startup. */
    (void)dev;
}

static void aht21_drv_deinit(temp_humi_drv_t * const dev)
{
    (void)dev;
}

/**
 * @brief Read temperature — synchronous from caller's perspective.
 *
 * Posts a TEMP event to the handler queue then blocks on the event group
 * until aht21_data_ready_cb() signals completion.
 */
static void aht21_read_temp(temp_humi_drv_t * const dev, float * const temp)
{
    (void)dev;
    adapter_sync_read(TEMP_HUMI_EVENT_TEMP, ADAPTER_EG_BIT_TEMP);
    if (temp != NULL) { *temp = s_temp_result; }
}

/**
 * @brief Read humidity — synchronous from caller's perspective.
 */
static void aht21_read_humi(temp_humi_drv_t * const dev, float * const humi)
{
    (void)dev;
    adapter_sync_read(TEMP_HUMI_EVENT_HUMI, ADAPTER_EG_BIT_HUMI);
    if (humi != NULL) { *humi = s_humi_result; }
}

/**
 * @brief Read temperature and humidity — synchronous from caller's perspective.
 */
static void aht21_read_temp_humi(temp_humi_drv_t * const dev,
                                          float * const temp,
                                          float * const humi)
{
    (void)dev;
    adapter_sync_read(TEMP_HUMI_EVENT_BOTH, ADAPTER_EG_BIT_BOTH);
    if (temp != NULL) { *temp = s_temp_result; }
    if (humi != NULL) { *humi = s_humi_result; }
}

/* ---------- IIC interface ------------------------------------------------- */

aht21_status_t iic_init_myown(void *bus)
{
    (void)bus;
    return AHT21_OK;
}

aht21_status_t iic_deinit_myown(void *bus)
{
    (void)bus;
    return AHT21_OK;
}

aht21_status_t iic_start_myown(void *bus)
{
    (void)bus;
    SENSOR_SOFTWARE_I2C_START();
    return AHT21_OK;
}

aht21_status_t iic_stop_myown(void *bus)
{
    (void)bus;
    SENSOR_SOFTWARE_I2C_STOP();
    return AHT21_OK;
}

aht21_status_t iic_wait_ack_myown(void *bus)
{
    (void)bus;
    core_i2c_status_t ret = SENSOR_SOFTWARE_I2C_WAIT_ACK();
    return (ret == CORE_I2C_OK) ? AHT21_OK : AHT21_ERROR;
}

aht21_status_t iic_send_ack_myown(void *bus)
{
    (void)bus;
    SENSOR_SOFTWARE_I2C_SEND_ACK();
    return AHT21_OK;
}

aht21_status_t iic_send_no_ack_myown(void *bus)
{
    (void)bus;
    SENSOR_SOFTWARE_I2C_SEND_NACK();
    return AHT21_OK;
}

aht21_status_t iic_send_byte_myown(void *bus, uint8_t const data)
{
    (void)bus;
    SENSOR_SOFTWARE_I2C_SEND_BYTE(data);
    return AHT21_OK;
}

aht21_status_t iic_receive_byte_myown(void *bus, uint8_t *const data)
{
    (void)bus;
    SENSOR_SOFTWARE_I2C_RECEIVE_BYTE(data);
    return AHT21_OK;
}

aht21_iic_driver_interface_t aht21_iic_driver_interface = {
    .pf_iic_init         = iic_init_myown,
    .pf_iic_deinit       = iic_deinit_myown,
    .pf_iic_start        = iic_start_myown,
    .pf_iic_stop         = iic_stop_myown,
    .pf_iic_wait_ack     = iic_wait_ack_myown,
    .pf_iic_send_ack     = iic_send_ack_myown,
    .pf_iic_send_no_ack  = iic_send_no_ack_myown,
    .pf_iic_send_byte    = iic_send_byte_myown,
    .pf_iic_receive_byte = iic_receive_byte_myown,
    .pf_critical_enter   = (aht21_status_t (*)(void))osal_critical_enter,
    .pf_critical_exit    = (aht21_status_t (*)(void))osal_critical_exit,
};

/* ---------- Timebase / yield interfaces ----------------------------------- */

uint32_t get_tick_count_ms(void)
{
    return HAL_GetTick();
}

aht21_timebase_interface_t aht21_timebase_interface = {
    .pf_get_tick_count_ms = get_tick_count_ms,
};

void rtos_yield(uint32_t const xms)
{
    osal_task_delay(xms);
}

aht21_yield_interface_t aht21_yield_interface = {
    .pf_rtos_yield = rtos_yield,
};

/* ---------- OS queue adapters (used by bsp_temp_humi_xxx_handler) --------- */

static temp_humi_status_t os_queue_create_adapter(uint32_t const item_num,
                                                   uint32_t const item_size,
                                                   void **  const queue_handler)
{
    int32_t ret = osal_queue_create(
                        (osal_queue_handle_t *)queue_handler,
                        (size_t)item_num,
                        (size_t)item_size);
    return (ret == OSAL_SUCCESS) ? TEMP_HUMI_OK : TEMP_HUMI_ERROR;
}

static temp_humi_status_t os_queue_put_adapter(void *  const queue_handler,
                                               void *  const item,
                                               uint32_t      timeout)
{
    int32_t ret = osal_queue_send(
                        (osal_queue_handle_t)queue_handler,
                        item,
                        (osal_tick_type_t)timeout);
    return (ret == OSAL_SUCCESS) ? TEMP_HUMI_OK : TEMP_HUMI_ERROR;
}

static temp_humi_status_t os_queue_get_adapter(void *  const queue_handler,
                                               void *  const msg,
                                               uint32_t      timeout)
{
    int32_t ret = osal_queue_receive(
                        (osal_queue_handle_t)queue_handler,
                        msg,
                        (osal_tick_type_t)timeout);
    return (ret == OSAL_SUCCESS) ? TEMP_HUMI_OK : TEMP_HUMI_ERROR;
}

static temp_humi_status_t os_queue_delete_adapter(void * const queue_handler)
{
    osal_queue_delete((osal_queue_handle_t)queue_handler);
    return TEMP_HUMI_OK;
}

static handler_os_queue_t os_queue_interface = {
    .pf_os_queue_create = os_queue_create_adapter,
    .pf_os_queue_put    = os_queue_put_adapter,
    .pf_os_queue_get    = os_queue_get_adapter,
    .pf_os_queue_delete = os_queue_delete_adapter,
};

/* ---------- Input args exported to user_task_reso_config ------------------ */

temp_humi_handler_input_arg_t input_arg = {
    .p_iic_driver_interface = &aht21_iic_driver_interface,
    .p_os_queue_interface   = &os_queue_interface,
    .p_timebase_interface   = &aht21_timebase_interface,
    .p_yield_interface      = &aht21_yield_interface,
};

//******************************* Functions *********************************//

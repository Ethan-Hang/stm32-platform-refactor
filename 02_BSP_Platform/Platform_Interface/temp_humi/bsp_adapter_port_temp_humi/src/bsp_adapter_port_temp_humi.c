/******************************************************************************
 * @file
 *
 * @par dependencies
 *
 * @author Ethan-Hang
 *
 * @brief
 *
 * Processing flow:
 *
 *
 * @version V1.0 2026--
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "i2c_port.h"
#include "bsp_temp_humi_xxx_handler.h"

#include "bsp_adapter_port_temp_humi.h"
#include "osal_wrapper_adapter.h"
#include "osal_error.h"

#include "Debug.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//

//******************************** Defines **********************************//

//******************************* Declaring *********************************//

//******************************* Declaring *********************************//

//******************************* Functions *********************************//
static void aht21_drv_init      (temp_humi_drv_t * const  dev);
static void aht21_drv_deinit    (temp_humi_drv_t * const  dev);
static void aht21_read_temp     (temp_humi_drv_t * const  dev,
                                           float * const temp);
static void aht21_read_humi     (temp_humi_drv_t * const  dev,
                                           float * const humi);
static void aht21_read_temp_humi(temp_humi_drv_t * const  dev,
                                           float * const temp,
                                           float * const humi);
static void aht21_callback(float *temp, float *humi);

bool drv_adapter_temp_humi_register(void)
{
    temp_humi_drv_t temp_humi_drv = {
        .idx = 0,
        .dev_id = 0,
        .pf_temp_humi_drv_init       = aht21_drv_init,
        .pf_temp_humi_drv_deinit     = aht21_drv_deinit,
        .pf_temp_humi_read_humi      = aht21_read_humi,
        .pf_temp_humi_read_temp      = aht21_read_temp,
        .pf_temp_humi_read_temp_humi = aht21_read_temp_humi
    };

    return drv_adapter_temp_humi_mount(0, &temp_humi_drv);
}

/* Latest values updated by async callback */
static float s_last_temp = 0.0f;
static float s_last_humi = 0.0f;

static void aht21_drv_init(temp_humi_drv_t * const dev)
{
    /* Handler thread initializes itself on startup */
    (void)dev;
}

static void aht21_drv_deinit(temp_humi_drv_t * const dev)
{
    (void)dev;
}

static void aht21_read_temp(temp_humi_drv_t * const dev,
                                     float * const temp)
{
    (void)dev;
    temp_humi_xxx_event_t event = {
        .event_type  = TEMP_HUMI_EVENT_TEMP,
        .lifetime    = 0,
        .pf_callback = aht21_callback
    };
    (void)bsp_temp_humi_xxx_read(&event);
    if (temp != NULL)
    {
        *temp = s_last_temp;
    }
}

static void aht21_read_humi(temp_humi_drv_t * const dev,
                                     float * const humi)
{
    (void)dev;
    temp_humi_xxx_event_t event = {
        .event_type  = TEMP_HUMI_EVENT_HUMI,
        .lifetime    = 0,
        .pf_callback = aht21_callback
    };
    (void)bsp_temp_humi_xxx_read(&event);
    if (humi != NULL)
    {
        *humi = s_last_humi;
    }
}

static void aht21_read_temp_humi(temp_humi_drv_t * const dev,
                                          float * const temp,
                                          float * const humi)
{
    (void)dev;
    temp_humi_xxx_event_t event = {
        .event_type  = TEMP_HUMI_EVENT_BOTH,
        .lifetime    = 0,
        .pf_callback = aht21_callback
    };
    (void)bsp_temp_humi_xxx_read(&event);
    if (temp != NULL)
    {
        *temp = s_last_temp;
    }
    if (humi != NULL)
    {
        *humi = s_last_humi;
    }
}

static void aht21_callback(float *temp, float *humi)
{
    if (temp != NULL) { s_last_temp = *temp; }
    if (humi != NULL) { s_last_humi = *humi; }
}


//******************************* Functions *********************************//


aht21_status_t iic_init_myown(void *bus)
{
    /* Software IIC GPIO already configured by HAL init */
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

/* IIC r-byte   interface */
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
    .pf_critical_exit    = (aht21_status_t (*)(void))osal_critical_exit};

uint32_t get_tick_count_ms(void)
{
    return HAL_GetTick();
}

aht21_timebase_interface_t aht21_timebase_interface = {
    .pf_get_tick_count_ms = get_tick_count_ms
};

void rtos_yield(uint32_t const xms)
{
    osal_task_delay(xms);
}
aht21_yield_interface_t aht21_yield_interface = {.pf_rtos_yield = rtos_yield};

/* ---------- OS queue adapters (maps OSAL queue API → handler interface) ---------- */

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

static handler_os_queue_t os_interface = {
    .pf_os_queue_create = os_queue_create_adapter,
    .pf_os_queue_put    = os_queue_put_adapter,
    .pf_os_queue_get    = os_queue_get_adapter,
    .pf_os_queue_delete = os_queue_delete_adapter,
};

/* ------------------------------------------------------------------------------ */

temp_humi_handler_input_arg_t input_arg = {
    .p_iic_driver_interface = &aht21_iic_driver_interface,
    .p_os_queue_interface   = &os_interface,
    .p_timebase_interface   = &aht21_timebase_interface,
    .p_yield_interface      = &aht21_yield_interface};

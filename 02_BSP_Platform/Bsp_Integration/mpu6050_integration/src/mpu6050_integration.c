#include "bsp_mpuxxxx_handler.h"

#include "main.h"

#include "osal_wrapper_adapter.h"

#include "FreeRTOS.h"
#include "task.h"   /* xTaskNotifyGive / xTaskNotifyFromISR / xTaskNotifyWait */

#include "i2c.h"
#include "dma.h"


mpuxxxx_status_t iic_driver_init(void const *constiic_bus)
{
    // has already inited in main.c
    return MPUXXXX_OK;
}

mpuxxxx_status_t iic_driver_deinit(void const *constiic_bus)
{
    __HAL_RCC_I2C2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3);
    return MPUXXXX_OK;
}

mpuxxxx_status_t iic_mem_read(void *hi2c, uint16_t dst_address,
                              uint16_t mem_addr, uint16_t mem_size,
                              uint8_t *p_data, uint16_t size, uint32_t timeout)
{
    HAL_StatusTypeDef hal_ret;
    hal_ret = HAL_I2C_Mem_Read(hi2c, dst_address, mem_addr, mem_size, p_data,
                               size, timeout);
    if (hal_ret != HAL_OK)
    {
        DEBUG_OUT(e, HAL_IIC_ERR_LOG_TAG,
                  "iic mem read failed, hal:%d dst:0x%04X mem:0x%04X size:%u",
                  hal_ret, (unsigned int)dst_address, (unsigned int)mem_addr,
                  (unsigned int)size);
        return MPUXXXX_ERROR;
    }
    return MPUXXXX_OK;
}

mpuxxxx_status_t iic_mem_write(void *hi2c, uint16_t dst_address,
                               uint16_t mem_addr, uint16_t mem_size,
                               uint8_t *p_data, uint16_t size, uint32_t timeout)
{
    HAL_StatusTypeDef hal_ret;
    hal_ret = HAL_I2C_Mem_Write(hi2c, dst_address, mem_addr, mem_size, p_data,
                                size, timeout);
    if (hal_ret != HAL_OK)
    {
        DEBUG_OUT(e, HAL_IIC_ERR_LOG_TAG,
                  "iic mem write failed, hal:%d dst:0x%04X mem:0x%04X size:%u",
                  hal_ret, (unsigned int)dst_address, (unsigned int)mem_addr,
                  (unsigned int)size);
        return MPUXXXX_ERROR;
    }
    return MPUXXXX_OK;
}

mpuxxxx_status_t iic_mem_read_dma(void *hi2c, uint16_t dst_address,
                                  uint16_t mem_addr, uint16_t mem_size,
                                  uint8_t *p_data, uint16_t size)
{
    HAL_StatusTypeDef hal_ret;
    hal_ret = HAL_I2C_Mem_Read_DMA(hi2c, dst_address, mem_addr, mem_size,
                                   p_data, size);
    if (hal_ret != HAL_OK)
    {
        DEBUG_OUT(
            e, HAL_IIC_ERR_LOG_TAG,
            "iic mem read dma failed, hal:%d dst:0x%04X mem:0x%04X size:%u",
            hal_ret, (unsigned int)dst_address, (unsigned int)mem_addr,
            (unsigned int)size);
        return MPUXXXX_ERROR;
    }
    return MPUXXXX_OK;
}

iic_driver_interface_t mpuxxxx_iic_driver_instance = {
    .hi2c                = &hi2c3,
    .pf_iic_deinit       = iic_driver_deinit,
    .pf_iic_init         = iic_driver_init,
    .pf_iic_mem_read     = iic_mem_read,
    .pf_iic_mem_write    = iic_mem_write,
    .pf_iic_mem_read_dma = iic_mem_read_dma};

timebase_interface_t timebase_interface = {
    .pf_get_tick_count = HAL_GetTick,
};

/*****************************************************************************/
#if OS_SUPPORTING
static void osal_yield_wrapper(uint32_t const ms)
{
    osal_task_delay((osal_tick_type_t)ms);
}

yield_interface_t yield_interface = {
    .pf_rtos_yield = osal_yield_wrapper,
};
#endif
/*****************************************************************************/
static void dwt_delay_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

static void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000U);
    while ((DWT->CYCCNT - start) < ticks);
}

static void delay_ms(uint32_t ms)
{
    delay_us(ms * 1000U);
}

delay_interface_t delay_interface = {
    .pf_delay_init = dwt_delay_init,
    .pf_delay_us   = delay_us,
    .pf_delay_ms   = delay_ms,
};
/*****************************************************************************/
// os interface
#if OS_SUPPORTING
static mpuxxxx_status_t os_queue_create(uint32_t const queue_size,
                                        uint32_t const item_size,
                                        void         **queue_handle)
{
    int32_t ret = osal_queue_create((osal_queue_handle_t *)queue_handle,
                                   (size_t)queue_size, (size_t)item_size);
    return (ret == 0) ? MPUXXXX_OK : MPUXXXX_ERROR;
}

static mpuxxxx_status_t os_queue_delete(void *const queue_handle)
{
    osal_queue_delete((osal_queue_handle_t)queue_handle);
    return MPUXXXX_OK;
}

static mpuxxxx_status_t os_queue_put(void *const queue_handle,
                                     void *const item,
                                     uint32_t const timeout)
{
    int32_t ret = osal_queue_send((osal_queue_handle_t)queue_handle, item,
                                 (osal_tick_type_t)timeout);
    return (ret == 0) ? MPUXXXX_OK : MPUXXXX_ERROR;
}

static mpuxxxx_status_t os_queue_put_isr(void *const queue_handle,
                                         void *const item,
                                         long *const HigherPriorityTaskWoken)
{
    int32_t ret = osal_queue_send_from_isr(
        (osal_queue_handle_t)queue_handle, item,
        (osal_base_type_t *)HigherPriorityTaskWoken);
    return (ret == 0) ? MPUXXXX_OK : MPUXXXX_ERROR;
}

static mpuxxxx_status_t os_queue_get(void *const queue_handle,
                                     void *const item,
                                     uint32_t const timeout)
{
    int32_t ret = osal_queue_receive((osal_queue_handle_t)queue_handle, item,
                                    (osal_tick_type_t)timeout);
    return (ret == 0) ? MPUXXXX_OK : MPUXXXX_ERROR;
}

static mpuxxxx_status_t os_semaphore_create_mutex(void **mutex_handle)
{
    int32_t ret = osal_mutex_init((osal_mutex_handle_t *)mutex_handle);
    return (ret == 0) ? MPUXXXX_OK : MPUXXXX_ERROR;
}

static mpuxxxx_status_t os_semaphore_delete_mutex(void *const mutex_handle)
{
    osal_mutex_delete((osal_mutex_handle_t)mutex_handle);
    return MPUXXXX_OK;
}

static mpuxxxx_status_t os_semaphore_lock_mutex(void *const    mutex_handle,
                                                 uint32_t const timeout)
{
    int32_t ret = osal_mutex_take((osal_mutex_handle_t)mutex_handle,
                                 (osal_tick_type_t)timeout);
    return (ret == 0) ? MPUXXXX_OK : MPUXXXX_ERROR;
}

static mpuxxxx_status_t os_semaphore_unlock_mutex(void *const mutex_handle)
{
    int32_t ret = osal_mutex_give((osal_mutex_handle_t)mutex_handle);
    return (ret == 0) ? MPUXXXX_OK : MPUXXXX_ERROR;
}

static mpuxxxx_status_t os_semaphore_create_binary(void **binary_handle)
{
    int32_t ret = osal_sema_init((osal_sema_handle_t *)binary_handle, 0);
    return (ret == 0) ? MPUXXXX_OK : MPUXXXX_ERROR;
}

static mpuxxxx_status_t os_semaphore_delete_binary(void *const binary_handle)
{
    osal_sema_delete((osal_sema_handle_t)binary_handle);
    return MPUXXXX_OK;
}

static mpuxxxx_status_t os_semaphore_signal_binary(void *const binary_handle)
{
    int32_t ret = osal_sema_give((osal_sema_handle_t)binary_handle);
    return (ret == 0) ? MPUXXXX_OK : MPUXXXX_ERROR;
}

static mpuxxxx_status_t os_semaphore_wait_binary(void *const binary_handle)
{
    int32_t ret = osal_sema_take((osal_sema_handle_t)binary_handle,
                                OSAL_MAX_DELAY);
    return (ret == 0) ? MPUXXXX_OK : MPUXXXX_ERROR;
}

/* Task notification: no OSAL wrapper, keep FreeRTOS direct calls */
static mpuxxxx_status_t
os_semaphore_signal_notify_isr(void *const notify_handle, uint32_t ulValue,
                               uint32_t    eAction,
                               long *const HigherPriorityTaskWoken)
{
    xTaskNotifyFromISR(notify_handle, ulValue, (eNotifyAction)eAction,
                       HigherPriorityTaskWoken);
    return MPUXXXX_OK;
}

static mpuxxxx_status_t os_semaphore_wait_notify(uint32_t  ulBitsToClearOnEntry,
                                                  uint32_t  ulBitsToClearOnExit,
                                                  uint32_t *pulNotificationValue,
                                                  uint32_t  timeout)
{
    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
    return MPUXXXX_OK;
}

os_interface_t os_interface = {
    .pf_os_queue_create          = os_queue_create,
    .pf_os_queue_delete          = os_queue_delete,
    .pf_os_queue_receive         = os_queue_get,
    .pf_os_queue_send            = os_queue_put,
    .pf_os_queue_send_isr        = os_queue_put_isr,
    .pf_os_mutex_create          = os_semaphore_create_mutex,
    .pf_os_mutex_delete          = os_semaphore_delete_mutex,
    .pf_os_mutex_lock            = os_semaphore_lock_mutex,
    .pf_os_mutex_unlock          = os_semaphore_unlock_mutex,
    .pf_os_semaphore_create      = os_semaphore_create_binary,
    .pf_os_semaphore_delete      = os_semaphore_delete_binary,
    .pf_os_semaphore_take        = os_semaphore_wait_binary,
    .pf_os_semaphore_give        = os_semaphore_signal_binary,
    .pf_os_semaephore_notify_isr = os_semaphore_signal_notify_isr,
    .pf_os_semaphore_wait_notify = os_semaphore_wait_notify};

#endif


hardware_interrupt_interface_t interrupt_interface = {
    .pf_irq_clear_pending =
        (mpuxxxx_status_t (*)(void))HAL_NVIC_ClearPendingIRQ,
    .pf_irq_deinit        = NULL,
    .pf_irq_disable       = NULL,
    .pf_irq_disable_clock = NULL,
    .pf_irq_enable        = NULL,
    .pf_irq_enable_clock  = NULL,
    .pf_irq_init          = NULL};

mpuxxxx_handler_input_args_t mpu6050_input_args = {
    .p_iic_driver      = &mpuxxxx_iic_driver_instance,
    .p_timebase        = &timebase_interface,
    .p_delay_interface = &delay_interface,
    .p_yield_interface = &yield_interface,
    .p_os_interface    = &os_interface};

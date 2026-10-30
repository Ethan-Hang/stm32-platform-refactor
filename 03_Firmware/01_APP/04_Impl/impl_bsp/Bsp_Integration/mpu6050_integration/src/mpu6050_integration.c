/******************************************************************************
 * @file mpu6050_integration.c
 *
 * @par dependencies
 * - bsp_mpuxxxx_handler.h
 * - main.h
 * - osal_wrapper_adapter.h
 * - i2c_port.h     (MCU-port I2C abstraction)
 * - systick_port.h (MCU-port ms timebase abstraction)
 * - dwt_port.h     (MCU-port us busy-wait abstraction)
 *
 * @author Ethan-Hang
 *
 * @brief Dependency injection for the MPU6050 motion sensor handler.
 *
 * Provides concrete implementations for I2C (mem read/write/DMA), yield, OS
 * (task notification from ISR, current-task lookup) and interrupt (EXTI)
 * interfaces, plus the two ISR trampoline registration hooks.  Wires them
 * into the mpu6050_deps aggregate and the mpu6050_input_args struct passed
 * to mpuxxxx_handler_thread() at startup.
 *
 * @version V1.0 2026-04-10
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#include "board_types.h"
#include "bsp_mpuxxxx_handler.h"

#include "osal_wrapper_adapter.h"

#include "i2c_port.h"
#include "i2c.h"            /* hi2c3 handle — board wiring token */

#define MPU6050_I2C_DMA_WAIT_TIMEOUT_MS   10U


/**
 * @brief  I2C bus init hook.  CubeMX MX_I2C3_Init() runs before the scheduler
 *         starts, so this is a no-op.
 *
 * @param[in] iic_bus : Unused (interface contract).
 *
 * @return MPUXXXX_OK
 */
mpuxxxx_status_t iic_driver_init(void const *const iic_bus)
{
    (void)iic_bus;
    return MPUXXXX_OK;
}

/**
 * @brief  I2C bus deinit hook (no-op for HAL passthrough).
 *
 * @param[in] iic_bus : Unused.
 *
 * @return MPUXXXX_OK
 */
mpuxxxx_status_t iic_driver_deinit(void const *const iic_bus)
{
    (void)iic_bus;
    return MPUXXXX_OK;
}

/**
 * @brief  Blocking I2C memory read through the MCU port abstraction.
 *         Routes to SENSOR_HARDWARE_I2C_MEM_READ (MCU_I2C_BUS_2).
 *
 * @param[in]  hi2c         : Bus handle (unused).
 * @param[in]  dst_address  : 7-bit slave address shifted left by 1.
 * @param[in]  mem_addr     : Internal register address.
 * @param[in]  mem_size     : 1 -> 8-bit reg addr, 2 -> 16-bit reg addr.
 * @param[out] p_data       : Destination buffer.
 * @param[in]  size         : Number of bytes to read.
 * @param[in]  timeout      : Timeout in ms.
 *
 * @return MPUXXXX_OK on success, MPUXXXX_ERROR on bus failure.
 */
mpuxxxx_status_t iic_mem_read(void *hi2c,
                              UINT16_t dst_address,
                              UINT16_t mem_addr,
                              UINT16_t mem_size,
                              UINT8_t *p_data,
                              UINT16_t size,
                              UINT32_t timeout)
{
    platform_err_t ret = SENSOR_HARDWARE_I2C_MEM_READ(
        dst_address, mem_addr, mem_size, p_data, size, timeout);
    if (ret != PLATFORM_OK)
    {
        DEBUG_OUT(e, HAL_IIC_ERR_LOG_TAG,
                  "iic mem read failed, ret:%d dst:0x%04X mem:0x%04X size:%u",
                  ret, (unsigned int)dst_address, (unsigned int)mem_addr,
                  (unsigned int)size);
        return MPUXXXX_ERROR;
    }
    return MPUXXXX_OK;
}

/**
 * @brief  Blocking I2C memory write through the MCU port abstraction.
 *         Routes to SENSOR_HARDWARE_I2C_MEM_WRITE (MCU_I2C_BUS_2).
 *
 * @param[in] hi2c         : Bus handle (unused).
 * @param[in] dst_address  : 7-bit slave address shifted left by 1.
 * @param[in] mem_addr     : Internal register address.
 * @param[in] mem_size     : 1 -> 8-bit reg addr, 2 -> 16-bit reg addr.
 * @param[in] p_data       : Source buffer.
 * @param[in] size         : Number of bytes to write.
 * @param[in] timeout      : Timeout in ms.
 *
 * @return MPUXXXX_OK on success, MPUXXXX_ERROR on bus or timeout failure.
 */
mpuxxxx_status_t iic_mem_write(void *hi2c,
                               UINT16_t dst_address,
                               UINT16_t mem_addr,
                               UINT16_t mem_size,
                               UINT8_t *p_data,
                               UINT16_t size,
                               UINT32_t timeout)
{
    platform_err_t ret = SENSOR_HARDWARE_I2C_MEM_WRITE(
        dst_address, mem_addr, mem_size, p_data, size, timeout);
    if (ret != PLATFORM_OK)
    {
        if (PLATFORM_ERR_TIMEOUT == ret)
        {
            DEBUG_OUT(e, HAL_IIC_ERR_LOG_TAG,
                      "iic mem write timeout, dst:0x%04X mem:0x%04X size:%u",
                      (unsigned int)dst_address, (unsigned int)mem_addr,
                      (unsigned int)size);
        }
        DEBUG_OUT(e, HAL_IIC_ERR_LOG_TAG,
                  "iic mem write failed, ret:%d dst:0x%04X mem:0x%04X size:%u",
                  ret, (unsigned int)dst_address, (unsigned int)mem_addr,
                  (unsigned int)size);
        return MPUXXXX_ERROR;
    }
    return MPUXXXX_OK;
}

/**
 * @brief  Non-blocking DMA I2C memory read through the MCU port abstraction.
 *         Routes to SENSOR_HARDWARE_I2C_MEM_READ_DMA (MCU_I2C_BUS_2).
 *         Blocks internally on a semaphore inside the I2C abstraction until
 *         the DMA transfer completes or times out.
 *
 * @param[in]  hi2c         : Bus handle (unused).
 * @param[in]  dst_address  : 7-bit slave address shifted left by 1.
 * @param[in]  mem_addr     : Internal register address.
 * @param[in]  mem_size     : 1 -> 8-bit reg addr, 2 -> 16-bit reg addr.
 * @param[out] p_data       : Destination buffer (valid after return).
 * @param[in]  size         : Number of bytes to read.
 *
 * @return MPUXXXX_OK on completion, MPUXXXX_ERROR on timeout or bus failure.
 */
mpuxxxx_status_t iic_mem_read_dma(void *hi2c,
                                  UINT16_t dst_address,
                                  UINT16_t mem_addr,
                                  UINT16_t mem_size,
                                  UINT8_t *p_data,
                                  UINT16_t size)
{
    platform_err_t ret = SENSOR_HARDWARE_I2C_MEM_READ_DMA(
        dst_address, mem_addr, mem_size, p_data, size,
        MPU6050_I2C_DMA_WAIT_TIMEOUT_MS);
    if (ret != PLATFORM_OK)
    {
        DEBUG_OUT(
            e, HAL_IIC_ERR_LOG_TAG,
            "iic mem read dma failed, ret:%d dst:0x%04X mem:0x%04X size:%u",
            ret, (unsigned int)dst_address, (unsigned int)mem_addr,
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
    .pf_iic_mem_read_dma = iic_mem_read_dma,
};

/*****************************************************************************/
#if OS_SUPPORTING
static void osal_yield_wrapper(UINT32_t const ms)
{
    osal_task_delay((osal_tick_type_t)ms);
}

yield_interface_t yield_interface = {
    .pf_rtos_yield = osal_yield_wrapper,
};
#endif
/*****************************************************************************/
/*****************************************************************************/
// os interface
#if OS_SUPPORTING
static mpuxxxx_status_t
os_semaphore_signal_notify_isr(void *const notify_handle, UINT32_t ulValue,
                               UINT32_t    eAction,
                               long *const HigherPriorityTaskWoken)
{
    INT32_t ret = osal_notify_send_from_isr(
        (osal_task_handle_t)notify_handle,
        ulValue,
        (osal_notify_action_t)eAction,
        (osal_base_type_t *)HigherPriorityTaskWoken);
    return (ret == 0) ? MPUXXXX_OK : MPUXXXX_ERROR;
}

static mpuxxxx_status_t os_semaphore_wait_notify(UINT32_t  ulBitsToClearOnEntry,
                                                  UINT32_t  ulBitsToClearOnExit,
                                                  UINT32_t *pulNotificationValue,
                                                  UINT32_t  timeout)
{
    INT32_t ret = osal_notify_wait(ulBitsToClearOnEntry, ulBitsToClearOnExit,
                                   pulNotificationValue,
                                   (osal_tick_type_t)timeout);
    /* DMA complete: release bus mutex in task context (mutex cannot be given
       from ISR; the ISR only sends the task notification above). */
    return (ret == 0) ? MPUXXXX_OK : MPUXXXX_ERROR;
}

static void *os_get_task_handle(void)
{
    return (void *)osal_task_get_current_handle();
}

os_interface_t os_interface = {
    .pf_os_semaephore_notify_isr = os_semaphore_signal_notify_isr,
    .pf_os_semaphore_wait_notify = os_semaphore_wait_notify,
    .pf_os_get_task_handle       = os_get_task_handle
};

#endif


hardware_interrupt_interface_t interrupt_interface = {
    .pf_irq_deinit = NULL,
};

/*===================== EXTI / DMA trampoline wiring ========================*/
/*
 * These two slots are owned by 01_App/User_Isr_handlers: HAL_GPIO_EXTI_Callback
 * (GPIO_PIN_5) and HAL_I2C_MemRxCpltCallback (hi2c3) call through them with the
 * driver object as first argument. Handing an MCU vector to a driver is board
 * wiring, so the hooks belong here next to the rest of the deps -- V1 kept them
 * in the handler, which is why the handler had to know about EXTI at all.
 */
extern void (*pf_pin_interrupt_callback)(void const * const, void * const);
extern void (*pf_dma_interrupt_callback)(void const * const, void * const);

/**
 * @brief  Publish the driver's data-ready EXTI trampoline to the ISR layer.
 *
 * @param[in] callback : Trampoline supplied by bsp_mpuxxxx_driver_inst().
 *
 * @return None.
 */
static void mpu6050_register_pin_callback(
                       void (*callback)(void const * const, void * const))
{
    pf_pin_interrupt_callback = callback;
}

/**
 * @brief  Publish the driver's I2C RX-DMA trampoline to the ISR layer.
 *
 * @param[in] callback : Trampoline supplied by bsp_mpuxxxx_driver_inst().
 *
 * @return None.
 */
static void mpu6050_register_dma_callback(
                       void (*callback)(void const * const, void * const))
{
    pf_dma_interrupt_callback = callback;
}

/*========================== deps -- board bindings =========================*/
/**
 * @brief Everything the MPU6050 driver needs from THIS board: bus, OS, yield,
 *        IRQ teardown and the two ISR trampoline slots. const: the driver
 *        stores it by pointer and never re-binds it.
 */
static mpuxxxx_deps_t const mpu6050_deps = {
    .p_iic_interface          = &mpuxxxx_iic_driver_instance,
    .p_interrupt_interface    = &interrupt_interface,
#if OS_SUPPORTING
    .p_yield_interface        = &yield_interface,
    .p_os_interface           = &os_interface,
#endif // OS_SUPPORTING
    .pf_callback_register     = mpu6050_register_pin_callback,
    .pf_callback_register_dma = mpu6050_register_dma_callback,
};

/*=========================== cfg -- device tuning ==========================*/
/**
 * @brief Tuning for this board's MPU6050. Reproduces the register picture V1
 *        hard-coded across mpuxxxx_init() + mpuxxxx_fifo_init(), with two
 *        deliberate corrections:
 *
 *  - sample_rate_div is 19 decimal -> 1000 / (1 + 19) = the 50 Hz V1's own
 *    comment claimed. V1 wrote SMPLRT_DIV_BIT(0x19) = 25 -> 38.46 Hz.
 *  - int_level is ACTIVE_HIGH. The MPU INT line (PB5) is wired to an EXTI
 *    configured GPIO_MODE_IT_RISING in gpio.c, so an active-high pulse fires
 *    the vector on assertion; V1's INT_LEVEL_BIT(1) (active low) meant the
 *    rising edge only arrived when the 50 us pulse released.
 *
 *  pwr_mgmt1 keeps CLKSEL = 1 (PLL with X-axis gyro reference), which is what
 *  V1 wrote after WHO_AM_I; the driver masks DEVICE_RESET out of this field.
 *  user_ctrl / fifo_enable keep the FIFO fed by accel + all three gyro axes,
 *  as V1's fifo_init() left it (the handler reads the data registers over
 *  DMA rather than the FIFO, but the chip-side picture stays unchanged).
 */
static mpuxxxx_cfg_t const mpu6050_cfg = {
    .gyro_fsr              = MPUXXXX_GYRO_FSR_2000DPS,
    .accel_fsr             = MPUXXXX_ACCEL_FSR_4G,
    .dlpf                  = MPUXXXX_DLPF_10HZ,
    .sample_rate_div       = 19,

    .data_ready_int_enable = true,
    .int_level             = MPUXXXX_INT_ACTIVE_HIGH,

    .motion_threshold      = 0,

    .user_ctrl             = FIFO_EN_BIT(1),
    .fifo_enable           = ACCEL_FIFO_EN_BIT(1) |
                             XG_FIFO_EN_BIT(1)    |
                             YG_FIFO_EN_BIT(1)    |
                             ZG_FIFO_EN_BIT(1),
    .pwr_mgmt1             = CLKSEL_BIT(0x01),
    .pwr_mgmt2             = STBY_XG_BIT(0) | STBY_YG_BIT(0) |
                             STBY_ZG_BIT(0) | STBY_XA_BIT(0) |
                             STBY_YA_BIT(0) | STBY_ZA_BIT(0),
};

mpuxxxx_handler_input_args_t mpu6050_input_args = {
    .p_deps = &mpu6050_deps,
    .p_cfg  = &mpu6050_cfg,
};

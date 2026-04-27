/******************************************************************************
 * @file    iic_hal.c
 *
 * @par dependencies
 *
 * @author Ethan-Hang
 *
 * @brief  Software I2C implementation with open-drain GPIO simulation.
 *
 * Processing flow:
 *   1. Call i2c_init() to configure GPIO pins
 *   2. Use i2c_start / i2c_stop / i2c_send_byte / i2c_receive_byte
 *      for raw bus access, or use the high-level read/write helpers.
 *
 * @version V1.0 2026-4-12
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "iic_hal.h"
#include "dwt_port.h"
//******************************** Includes *********************************//

//******************************* Private helpers ***************************//
static void sda_input_mode(iic_bus_t *bus)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.Pin   = bus->iic_sda_pin;
    GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(bus->iic_sda_port, &GPIO_InitStructure);
}

static void sda_output_mode(iic_bus_t *bus)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.Pin   = bus->iic_sda_pin;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStructure.Pull  = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(bus->iic_sda_port, &GPIO_InitStructure);
}

static void sda_output(iic_bus_t *bus, uint8_t val)
{
    HAL_GPIO_WritePin(bus->iic_sda_port, bus->iic_sda_pin,
                      val > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void scl_output(iic_bus_t *bus, uint8_t val)
{
    HAL_GPIO_WritePin(bus->iic_scl_port, bus->iic_scl_pin,
                      val > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t sda_input(iic_bus_t *bus)
{
    return (HAL_GPIO_ReadPin(bus->iic_sda_port, bus->iic_sda_pin) == GPIO_PIN_SET)
               ? 1U
               : 0U;
}
//******************************* Private helpers ***************************//

//******************************* Bus control *******************************//
void i2c_init(iic_bus_t *bus)
{
    if (NULL == bus)
    {
        return;
    }

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.Pin   = bus->iic_sda_pin;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStructure.Pull  = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(bus->iic_sda_port, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = bus->iic_scl_pin;
    HAL_GPIO_Init(bus->iic_scl_port, &GPIO_InitStructure);

    sda_output(bus, 1);
    scl_output(bus, 1);
}

void i2c_deinit(iic_bus_t *bus)
{
    if (NULL == bus)
    {
        return;
    }

    HAL_GPIO_DeInit(bus->iic_sda_port, bus->iic_sda_pin);
    HAL_GPIO_DeInit(bus->iic_scl_port, bus->iic_scl_pin);
}

void i2c_start(iic_bus_t *bus)
{
    if (NULL == bus)
    {
        return;
    }

    sda_output(bus, 1);
    core_dwt_delay_us(1);
    scl_output(bus, 1);
    core_dwt_delay_us(1);
    sda_output(bus, 0);  /* SDA falls while SCL is high -> START */
    core_dwt_delay_us(1);
    scl_output(bus, 0);
    core_dwt_delay_us(1);
}

void i2c_stop(iic_bus_t *bus)
{
    if (NULL == bus)
    {
        return;
    }

    scl_output(bus, 0);
    core_dwt_delay_us(1);
    sda_output(bus, 0);
    core_dwt_delay_us(1);
    scl_output(bus, 1);
    core_dwt_delay_us(1);
    sda_output(bus, 1);  /* SDA rises while SCL is high -> STOP */
    core_dwt_delay_us(1);
}

/**
 * @brief  Send one byte MSB-first; leaves SCL low on exit.
 */
void i2c_send_byte(iic_bus_t *bus, uint8_t byte)
{
    if (NULL == bus)
    {
        return;
    }

    for (uint8_t i = 8; i > 0; i--)
    {
        scl_output(bus, 0);
        core_dwt_delay_us(1);
        sda_output(bus, (byte >> (i - 1)) & 0x01);
        core_dwt_delay_us(1);
        scl_output(bus, 1);
        core_dwt_delay_us(1);
    }
    scl_output(bus, 0);  /* pull SCL low, ready for ACK clock */
    core_dwt_delay_us(1);
}

/**
 * @brief  Wait for ACK from slave.
 * @retval  IIC_HAL_OK    ACK received
 *          IIC_HAL_NACK  NACK / timeout
 *          IIC_HAL_ERROR null bus pointer
 */
iic_hal_status_t i2c_wait_ack(iic_bus_t *bus)
{
    if (NULL == bus)
    {
        return IIC_HAL_ERROR;
    }

    sda_input_mode(bus);
    scl_output(bus, 1);

    for (uint8_t timeout = 5; timeout > 0; timeout--)
    {
        if (0 == sda_input(bus))  /* SDA low = ACK */
        {
            scl_output(bus, 0);
            sda_output_mode(bus);
            core_dwt_delay_us(1);
            return IIC_HAL_OK;
        }
        core_dwt_delay_us(1);
    }

    scl_output(bus, 0);
    sda_output_mode(bus);
    core_dwt_delay_us(1);
    return IIC_HAL_NACK;
}

void i2c_send_ack(iic_bus_t *bus)
{
    if (NULL == bus)
    {
        return;
    }

    sda_output(bus, 0);
    core_dwt_delay_us(1);
    scl_output(bus, 1);
    core_dwt_delay_us(1);
    scl_output(bus, 0);
    core_dwt_delay_us(1);
}

void i2c_send_not_ack(iic_bus_t *bus)
{
    if (NULL == bus)
    {
        return;
    }

    sda_output(bus, 1);
    core_dwt_delay_us(1);
    scl_output(bus, 1);
    core_dwt_delay_us(1);
    scl_output(bus, 0);
    core_dwt_delay_us(1);
}

/**
 * @brief  Receive one byte MSB-first; caller must send ACK/NACK afterwards.
 */
uint8_t i2c_receive_byte(iic_bus_t *bus)
{
    uint8_t byte = 0;

    if (NULL == bus)
    {
        return 0;
    }

    sda_input_mode(bus);

    for (uint8_t i = 0; i < 8; i++)
    {
        byte <<= 1;
        scl_output(bus, 0);
        core_dwt_delay_us(2);
        scl_output(bus, 1);
        core_dwt_delay_us(1);
        byte |= sda_input(bus);
    }

    scl_output(bus, 0);
    sda_output_mode(bus);
    return byte;
}
//******************************* Bus control *******************************//

//******************************* High-level API ****************************//

/**
 * @brief  Write a single register byte.
 * @retval  IIC_HAL_OK    success
 *          IIC_HAL_NACK  slave NACK
 *          IIC_HAL_ERROR null pointer
 */
iic_hal_status_t i2c_write_one_byte(iic_bus_t *bus, uint8_t device_addr,
                                    uint8_t reg, uint8_t data)
{
    if (NULL == bus)
    {
        return IIC_HAL_ERROR;
    }

    i2c_start(bus);

    i2c_send_byte(bus, device_addr << 1);
    if (IIC_HAL_OK != i2c_wait_ack(bus))
    {
        i2c_stop(bus);
        return IIC_HAL_NACK;
    }

    i2c_send_byte(bus, reg);
    if (IIC_HAL_OK != i2c_wait_ack(bus))
    {
        i2c_stop(bus);
        return IIC_HAL_NACK;
    }

    i2c_send_byte(bus, data);
    if (IIC_HAL_OK != i2c_wait_ack(bus))
    {
        i2c_stop(bus);
        return IIC_HAL_NACK;
    }

    i2c_stop(bus);
    return IIC_HAL_OK;
}

/**
 * @brief  Write multiple bytes to consecutive registers.
 * @retval  IIC_HAL_OK / IIC_HAL_NACK / IIC_HAL_ERROR
 */
iic_hal_status_t i2c_write_multi_byte(iic_bus_t *bus, uint8_t device_addr,
                                      uint8_t reg, uint8_t length,
                                      const uint8_t *buff)
{
    if (NULL == bus)
    {
        return IIC_HAL_ERROR;
    }

    i2c_start(bus);

    i2c_send_byte(bus, device_addr << 1);
    if (IIC_HAL_OK != i2c_wait_ack(bus))
    {
        i2c_stop(bus);
        return IIC_HAL_NACK;
    }

    i2c_send_byte(bus, reg);
    if (IIC_HAL_OK != i2c_wait_ack(bus))
    {
        i2c_stop(bus);
        return IIC_HAL_NACK;
    }

    for (uint8_t i = 0; i < length; i++)
    {
        i2c_send_byte(bus, buff[i]);
        if (IIC_HAL_OK != i2c_wait_ack(bus))
        {
            i2c_stop(bus);
            return IIC_HAL_NACK;
        }
    }

    i2c_stop(bus);
    return IIC_HAL_OK;
}

/**
 * @brief  Write a raw byte stream (no register byte, address already in buff).
 * @retval  IIC_HAL_OK / IIC_HAL_NACK / IIC_HAL_ERROR
 */
iic_hal_status_t i2c_write_bytes(iic_bus_t *bus, uint8_t device_addr,
                                 uint8_t length, const uint8_t *buff)
{
    if (NULL == bus)
    {
        return IIC_HAL_ERROR;
    }

    i2c_start(bus);

    i2c_send_byte(bus, device_addr << 1);
    if (IIC_HAL_OK != i2c_wait_ack(bus))
    {
        i2c_stop(bus);
        return IIC_HAL_NACK;
    }

    for (uint8_t i = 0; i < length; i++)
    {
        i2c_send_byte(bus, buff[i]);
        if (IIC_HAL_OK != i2c_wait_ack(bus))
        {
            i2c_stop(bus);
            return IIC_HAL_NACK;
        }
    }

    i2c_stop(bus);
    return IIC_HAL_OK;
}

/**
 * @brief  Read a single register byte.
 * @param  data  Output: received byte.
 * @retval  IIC_HAL_OK / IIC_HAL_NACK / IIC_HAL_ERROR
 */
iic_hal_status_t i2c_read_one_byte(iic_bus_t *bus, uint8_t device_addr,
                                   uint8_t reg, uint8_t *data)
{
    if (NULL == bus || NULL == data)
    {
        return IIC_HAL_ERROR;
    }

    i2c_start(bus);
    i2c_send_byte(bus, device_addr << 1);
    if (IIC_HAL_OK != i2c_wait_ack(bus))
    {
        i2c_stop(bus);
        return IIC_HAL_NACK;
    }

    i2c_send_byte(bus, reg);
    if (IIC_HAL_OK != i2c_wait_ack(bus))
    {
        i2c_stop(bus);
        return IIC_HAL_NACK;
    }

    i2c_start(bus);  /* repeated START for read */
    i2c_send_byte(bus, (device_addr << 1) | 0x01);
    if (IIC_HAL_OK != i2c_wait_ack(bus))
    {
        i2c_stop(bus);
        return IIC_HAL_NACK;
    }

    *data = i2c_receive_byte(bus);
    i2c_send_not_ack(bus);
    i2c_stop(bus);

    return IIC_HAL_OK;
}

/**
 * @brief  Read multiple bytes from consecutive registers.
 * @retval  IIC_HAL_OK / IIC_HAL_NACK / IIC_HAL_ERROR
 */
iic_hal_status_t i2c_read_multi_byte(iic_bus_t *bus, uint8_t device_addr,
                                     uint8_t reg, uint8_t length,
                                     uint8_t *buff)
{
    if (NULL == bus)
    {
        return IIC_HAL_ERROR;
    }

    i2c_start(bus);
    i2c_send_byte(bus, device_addr << 1);
    if (IIC_HAL_OK != i2c_wait_ack(bus))
    {
        i2c_stop(bus);
        return IIC_HAL_NACK;
    }

    i2c_send_byte(bus, reg);
    if (IIC_HAL_OK != i2c_wait_ack(bus))
    {
        i2c_stop(bus);
        return IIC_HAL_NACK;
    }

    i2c_start(bus);  /* repeated START for read */
    i2c_send_byte(bus, (device_addr << 1) | 0x01);
    if (IIC_HAL_OK != i2c_wait_ack(bus))
    {
        i2c_stop(bus);
        return IIC_HAL_NACK;
    }

    for (uint8_t i = 0; i < length; i++)
    {
        buff[i] = i2c_receive_byte(bus);
        if (i < length - 1)
        {
            i2c_send_ack(bus);
        }
    }
    i2c_send_not_ack(bus);
    i2c_stop(bus);

    return IIC_HAL_OK;
}

/**
 * @brief  Read a raw byte stream (no register write phase).
 * @retval  IIC_HAL_OK / IIC_HAL_NACK / IIC_HAL_ERROR
 */
iic_hal_status_t i2c_read_bytes(iic_bus_t *bus, uint8_t device_addr,
                                uint8_t length, uint8_t *buff)
{
    if (NULL == bus)
    {
        return IIC_HAL_ERROR;
    }

    i2c_start(bus);
    i2c_send_byte(bus, (device_addr << 1) | 0x01);
    if (IIC_HAL_OK != i2c_wait_ack(bus))
    {
        i2c_stop(bus);
        return IIC_HAL_NACK;
    }

    for (uint8_t i = 0; i < length; i++)
    {
        buff[i] = i2c_receive_byte(bus);
        if (i < length - 1)
        {
            i2c_send_ack(bus);
        }
    }
    i2c_send_not_ack(bus);
    i2c_stop(bus);

    return IIC_HAL_OK;
}
//******************************* High-level API ****************************//

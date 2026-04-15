/******************************************************************************
 * @file    i2c_port.c
 *
 * @par dependencies
 *
 * @author Ethan-Hang
 *
 * @brief  MCU-level I2C port layer.
 *         Wraps both hardware I2C (HAL) and software I2C (iic_hal) behind a
 *         unified core_i2c_status_t interface.
 *
 * Processing flow:
 *   Select bus by core_i2c_bus_t index; dispatch to hard or soft driver.
 *
 * @version V1.0 2026-4-12
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "i2c_port.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//

//******************************** Defines **********************************//

//******************************* Declaring *********************************//
static i2c_port_t i2c_port[CORE_I2C_BUS_MAX] =
{
    [CORE_I2C_BUS_1] = {
        .core_iic_state  = HARDWARE_I2C,
        .hard_iic_handle = &hi2c3,
        .os_mutexid      = NULL
    },

    [CORE_I2C_BUS_2] = {
        .core_iic_state  = SOFTWARE_I2C,
        .soft_iic_bus_inst = {
            .iic_scl_port = SOFT_I2C1_SCL_GPIO_Port,
            .iic_sda_port = SOFG_I2C1_SDA_GPIO_Port,
            .iic_scl_pin  = SOFT_I2C1_SCL_Pin,
            .iic_sda_pin  = SOFG_I2C1_SDA_Pin
        },
    }
};
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
core_i2c_status_t core_i2c_port_init(core_i2c_bus_t bus)
{
    if (bus >= CORE_I2C_BUS_MAX)
    {
        return CORE_I2C_ERROR;
    }

    if (i2c_port[bus].core_iic_state != HARDWARE_I2C)
    {
        return CORE_I2C_OK;
    }

    int32_t ret = osal_mutex_init(&i2c_port[bus].os_mutexid);
    return (ret == 0) ? CORE_I2C_OK : CORE_I2C_ERROR;
}

core_i2c_status_t core_hard_i2c_send_byte   (core_i2c_bus_t      bus,
                                                   uint16_t dev_addr,
                                                   uint8_t *    data,
                                                   uint16_t     size,
                                                   uint32_t  timeout)
{
    if (bus >= CORE_I2C_BUS_MAX || NULL == data)
    {
        return CORE_I2C_ERROR;
    }

    if (osal_mutex_take(i2c_port[bus].os_mutexid,
                        (osal_tick_type_t)timeout) != 0)
    {
        return CORE_I2C_TIMEOUT;
    }

    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(
        i2c_port[bus].hard_iic_handle, dev_addr, data, size, timeout);

    osal_mutex_give(i2c_port[bus].os_mutexid);

    return (ret == HAL_OK) ? CORE_I2C_OK : CORE_I2C_ERROR;
}

core_i2c_status_t core_hard_i2c_receive_byte(core_i2c_bus_t      bus,
                                                   uint16_t dev_addr,
                                                   uint8_t *    data,
                                                   uint16_t     size,
                                                   uint32_t  timeout)
{
    if (bus >= CORE_I2C_BUS_MAX || NULL == data)
    {
        return CORE_I2C_ERROR;
    }

    if (osal_mutex_take(i2c_port[bus].os_mutexid,
                        (osal_tick_type_t)timeout) != 0)
    {
        return CORE_I2C_TIMEOUT;
    }

    HAL_StatusTypeDef ret = HAL_I2C_Master_Receive(
        i2c_port[bus].hard_iic_handle, dev_addr, data, size, timeout);

    osal_mutex_give(i2c_port[bus].os_mutexid);

    return (ret == HAL_OK) ? CORE_I2C_OK : CORE_I2C_ERROR;
}

core_i2c_status_t core_hard_i2c_mem_write   (core_i2c_bus_t      bus,
                                                   uint16_t dev_addr,
                                                   uint16_t mem_addr,
                                                   uint16_t \
                                                        mem_add_size,
                                                   uint8_t *    data,
                                                   uint16_t     size,
                                                   uint32_t  timeout)
{
    if (bus >= CORE_I2C_BUS_MAX || NULL == data)
    {
        return CORE_I2C_ERROR;
    }

    if (osal_mutex_take(i2c_port[bus].os_mutexid,
                        (osal_tick_type_t)timeout) != 0)
    {
        return CORE_I2C_TIMEOUT;
    }

    HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(
        i2c_port[bus].hard_iic_handle, dev_addr, mem_addr,
        mem_add_size, data, size, timeout);

    osal_mutex_give(i2c_port[bus].os_mutexid);

    return (ret == HAL_OK) ? CORE_I2C_OK : CORE_I2C_ERROR;
}

core_i2c_status_t core_hard_i2c_mem_read    (core_i2c_bus_t      bus,
                                                   uint16_t dev_addr,
                                                   uint16_t mem_addr,
                                                   uint16_t \
                                                        mem_add_size,
                                                   uint8_t *    data,
                                                   uint16_t     size,
                                                   uint32_t  timeout)
{
    if (bus >= CORE_I2C_BUS_MAX || NULL == data)
    {
        return CORE_I2C_ERROR;
    }

    if (osal_mutex_take(i2c_port[bus].os_mutexid,
                        (osal_tick_type_t)timeout) != 0)
    {
        return CORE_I2C_TIMEOUT;
    }

    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(
        i2c_port[bus].hard_iic_handle, dev_addr, mem_addr,
        mem_add_size, data, size, timeout);

    osal_mutex_give(i2c_port[bus].os_mutexid);

    return (ret == HAL_OK) ? CORE_I2C_OK : CORE_I2C_ERROR;
}

core_i2c_status_t core_hard_i2c_mem_read_dma(core_i2c_bus_t      bus,
                                                   uint16_t dev_addr,
                                                   uint16_t mem_addr,
                                                   uint16_t \
                                                        mem_add_size,
                                                   uint8_t *    data,
                                                   uint16_t     size)
{
    if (bus >= CORE_I2C_BUS_MAX || NULL == data)
    {
        return CORE_I2C_ERROR;
    }

    /* No mutex here: this function is called from ISR context (EXTI callback
       triggers the DMA read). Bus serialization is handled by the upper-layer
       driver via interrupt enable/disable. */
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read_DMA(
        i2c_port[bus].hard_iic_handle, dev_addr, mem_addr,
        mem_add_size, data, size);

    return (ret == HAL_OK) ? CORE_I2C_OK : CORE_I2C_ERROR;
}


core_i2c_status_t core_soft_i2c_start(core_i2c_bus_t bus)
{
    if (bus >= CORE_I2C_BUS_MAX)
    {
        return CORE_I2C_ERROR;
    }

    i2c_start(&i2c_port[bus].soft_iic_bus_inst);
    return CORE_I2C_OK;
}

core_i2c_status_t core_soft_i2c_stop(core_i2c_bus_t bus)
{
    if (bus >= CORE_I2C_BUS_MAX)
    {
        return CORE_I2C_ERROR;
    }

    i2c_stop(&i2c_port[bus].soft_iic_bus_inst);
    return CORE_I2C_OK;
}

core_i2c_status_t core_soft_i2c_send_byte(core_i2c_bus_t bus, uint8_t data)
{
    if (bus >= CORE_I2C_BUS_MAX)
    {
        return CORE_I2C_ERROR;
    }

    i2c_send_byte(&i2c_port[bus].soft_iic_bus_inst, data);
    return CORE_I2C_OK;
}

core_i2c_status_t core_soft_i2c_wait_ack(core_i2c_bus_t bus)
{
    if (bus >= CORE_I2C_BUS_MAX)
    {
        return CORE_I2C_ERROR;
    }

    iic_hal_status_t ack = i2c_wait_ack(&i2c_port[bus].soft_iic_bus_inst);
    if (IIC_HAL_OK == ack)
    {
        return CORE_I2C_OK;
    }
    return CORE_I2C_ERROR;
}

core_i2c_status_t core_soft_i2c_receive_byte(core_i2c_bus_t bus, uint8_t *data)
{
    if (bus >= CORE_I2C_BUS_MAX || NULL == data)
    {
        return CORE_I2C_ERROR;
    }

    *data = i2c_receive_byte(&i2c_port[bus].soft_iic_bus_inst);
    return CORE_I2C_OK;
}

core_i2c_status_t core_soft_i2c_send_ack(core_i2c_bus_t bus)
{
    if (bus >= CORE_I2C_BUS_MAX)
    {
        return CORE_I2C_ERROR;
    }

    i2c_send_ack(&i2c_port[bus].soft_iic_bus_inst);
    return CORE_I2C_OK;
}

core_i2c_status_t core_soft_i2c_send_nack(core_i2c_bus_t bus)
{
    if (bus >= CORE_I2C_BUS_MAX)
    {
        return CORE_I2C_ERROR;
    }

    i2c_send_not_ack(&i2c_port[bus].soft_iic_bus_inst);
    return CORE_I2C_OK;
}

//******************************* Functions *********************************//

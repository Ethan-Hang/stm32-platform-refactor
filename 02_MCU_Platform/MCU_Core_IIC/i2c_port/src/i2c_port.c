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
        .hard_iic_handle = &hi2c3
    },

    [CORE_I2C_BUS_2] = {
        .core_iic_state  = SOFTWARE_I2C,
        .soft_iic_bus_inst = {
            .iic_sda_port = SOFT_I2C1_SCL_GPIO_Port,
            .iic_scl_port = SOFG_I2C1_SDA_GPIO_Port,
            .iic_sda_pin  = SOFT_I2C1_SCL_Pin,
            .iic_scl_pin  = SOFG_I2C1_SDA_Pin
        },
    }
};
//******************************* Declaring *********************************//

//******************************* Functions *********************************//

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

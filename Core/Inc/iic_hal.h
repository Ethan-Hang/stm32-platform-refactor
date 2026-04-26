/******************************************************************************
 * @file    iic_hal.h
 *
 * @author  Ethan-Hang
 *
 * @brief   Software I2C public interface.
 *          All GPIO pins are driven as open-drain + external pull-up.
 *
 * @version V1.0 2026-4-12
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __IIC_HAL_H__
#define __IIC_HAL_H__

//******************************** Includes *********************************//
#include "stm32f4xx_hal.h"

//******************************** Includes *********************************//

//******************************** Types ************************************//
typedef enum
{
    IIC_HAL_OK    = 0,  /* ACK received / operation success */
    IIC_HAL_NACK  = 1,  /* NACK or timeout                  */
    IIC_HAL_ERROR = 2   /* invalid parameter (null pointer)  */
} iic_hal_status_t;

typedef struct
{
    GPIO_TypeDef *iic_sda_port;
    GPIO_TypeDef *iic_scl_port;
    uint16_t      iic_sda_pin;
    uint16_t      iic_scl_pin;
} iic_bus_t;

//******************************** Types ************************************//

//******************************* API ***************************************//

/* Initialisation */
void i2c_init(iic_bus_t *bus);
void i2c_deinit(iic_bus_t *bus);

/* Bus primitives */
void             i2c_start(iic_bus_t *bus);
void             i2c_stop(iic_bus_t *bus);
void             i2c_send_byte(iic_bus_t *bus, uint8_t byte);
iic_hal_status_t i2c_wait_ack(iic_bus_t *bus);
void             i2c_send_ack(iic_bus_t *bus);
void             i2c_send_not_ack(iic_bus_t *bus);
uint8_t          i2c_receive_byte(iic_bus_t *bus);

/* High-level helpers */
iic_hal_status_t i2c_write_one_byte(iic_bus_t *bus, uint8_t device_addr,
                                    uint8_t reg, uint8_t data);
iic_hal_status_t i2c_write_multi_byte(iic_bus_t *bus, uint8_t device_addr,
                                      uint8_t reg, uint8_t length,
                                      const uint8_t *buff);
iic_hal_status_t i2c_write_bytes(iic_bus_t *bus, uint8_t device_addr,
                                 uint8_t length, const uint8_t *buff);
iic_hal_status_t i2c_read_one_byte(iic_bus_t *bus, uint8_t device_addr,
                                   uint8_t reg, uint8_t *data);
iic_hal_status_t i2c_read_multi_byte(iic_bus_t *bus, uint8_t device_addr,
                                     uint8_t reg, uint8_t length,
                                     uint8_t *buff);
iic_hal_status_t i2c_read_bytes(iic_bus_t *bus, uint8_t device_addr,
                                uint8_t length, uint8_t *buff);

//******************************* API ***************************************//

#endif /* __IIC_HAL_H__ */

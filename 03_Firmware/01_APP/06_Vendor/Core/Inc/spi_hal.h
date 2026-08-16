/******************************************************************************
 * @file spi_hal.h
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
 * @version V1.0 2026-04-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __SPI_HAL_H__
#define __SPI_HAL_H__

//******************************** Includes *********************************//
#include "main.h"
#include "gpio.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//

typedef enum
{
    SPI_HAL_OK    = 0,
    SPI_HAL_ERROR = 1 
} spi_hal_status_t;

typedef struct
{
    GPIO_TypeDef *        spi_sck_port;
    GPIO_TypeDef *       spi_miso_port;
    GPIO_TypeDef *       spi_mosi_port;
    GPIO_TypeDef *         spi_cs_port;

    uint16_t               spi_sck_pin;
    uint16_t              spi_miso_pin;
    uint16_t              spi_mosi_pin;
    uint16_t                spi_cs_pin;
} spi_bus_t;

//******************************** Defines **********************************//

//******************************* Declaring *********************************//

//******************************* Declaring *********************************//

//******************************* Functions *********************************//
void spi_init(spi_bus_t *bus);
void spi_deinit(spi_bus_t *bus);

/* CS control */
void spi_cs_select(spi_bus_t *bus);
void spi_cs_deselect(spi_bus_t *bus);

/* Single-byte primitives without CS — use these for multi-byte transactions */
void    spi_write_byte(spi_bus_t *bus, uint8_t byte);
uint8_t spi_read_byte(spi_bus_t *bus);
uint8_t spi_readwrite_byte(spi_bus_t *bus, uint8_t tx_byte);

/* Legacy wrappers: CS included */
void    spi_send_byte(spi_bus_t *bus, uint8_t byte);
uint8_t spi_receive_byte(spi_bus_t *bus);
uint8_t spi_transfer_byte(spi_bus_t *bus, uint8_t tx_byte);

//******************************* Functions *********************************//

#endif /* __SPI_HAL_H__ */

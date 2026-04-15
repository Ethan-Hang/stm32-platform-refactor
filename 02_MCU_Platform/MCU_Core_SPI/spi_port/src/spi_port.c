/******************************************************************************
 * @file    spi_port.c
 *
 * @par dependencies
 *
 * @author Ethan-Hang
 *
 * @brief  MCU-level SPI port layer.
 *         Wraps both hardware SPI (HAL) and software SPI (spi_hal) behind a
 *         unified core_spi_status_t interface.
 *
 * Processing flow:
 *   Select bus by core_spi_bus_t index; dispatch to hard or soft driver.
 *
 * @version V1.0 2026-4-15
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "spi_port.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//

//******************************** Defines **********************************//

//******************************* Declaring *********************************//
static spi_port_t spi_port[CORE_SPI_BUS_MAX] =
{
    [CORE_SPI_BUS_1] = {
        .core_spi_state  = SOFTWARE_SPI,
        .soft_spi_bus_inst = {
            .spi_sck_port  = SOFT_SPI1_SCK_GPIO_Port,
            .spi_miso_port = SOFT_SPI1_MISO_GPIO_Port,
            .spi_mosi_port = SOFT_SPI1_MOSI_GPIO_Port,
            .spi_cs_port   = SOFT_SPI1_CS_GPIO_Port,
            .spi_sck_pin   = SOFT_SPI1_SCK_Pin,
            .spi_miso_pin  = SOFT_SPI1_MISO_Pin,
            .spi_mosi_pin  = SOFT_SPI1_MOSI_Pin,
            .spi_cs_pin    = SOFT_SPI1_CS_Pin,
        }
    },
};
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
core_spi_status_t core_spi_port_init(core_spi_bus_t bus)
{
    if (bus >= CORE_SPI_BUS_MAX)
    {
        return CORE_SPI_ERROR;
    }

    if (spi_port[bus].core_spi_state != HARDWARE_SPI)
    {
        spi_init(&spi_port[bus].soft_spi_bus_inst);
        return CORE_SPI_OK;
    }

#ifdef HAL_SPI_MODULE_ENABLED
    int32_t ret = osal_mutex_init(&spi_port[bus].os_mutexid);
    return (ret == 0) ? CORE_SPI_OK : CORE_SPI_ERROR;
#else
    return CORE_SPI_ERROR;
#endif
}

#ifdef HAL_SPI_MODULE_ENABLED
/* ---------- Hardware SPI CS control ---------- */

core_spi_status_t core_hard_spi_cs_select(core_spi_bus_t bus)
{
    if (bus >= CORE_SPI_BUS_MAX)
    {
        return CORE_SPI_ERROR;
    }

    HAL_GPIO_WritePin(spi_port[bus].hard_spi_cs_port,
                      spi_port[bus].hard_spi_cs_pin,
                      GPIO_PIN_RESET);
    return CORE_SPI_OK;
}

core_spi_status_t core_hard_spi_cs_deselect(core_spi_bus_t bus)
{
    if (bus >= CORE_SPI_BUS_MAX)
    {
        return CORE_SPI_ERROR;
    }

    HAL_GPIO_WritePin(spi_port[bus].hard_spi_cs_port,
                      spi_port[bus].hard_spi_cs_pin,
                      GPIO_PIN_SET);
    return CORE_SPI_OK;
}

/* ---------- Hardware SPI bus primitives ---------- */

core_spi_status_t core_hard_spi_transmit(core_spi_bus_t bus,
                                              uint8_t *data,
                                              uint16_t  size,
                                              uint32_t  timeout)
{
    if (bus >= CORE_SPI_BUS_MAX || NULL == data)
    {
        return CORE_SPI_ERROR;
    }

    if (osal_mutex_take(spi_port[bus].os_mutexid,
                        (osal_tick_type_t)timeout) != 0)
    {
        return CORE_SPI_TIMEOUT;
    }

    HAL_StatusTypeDef ret = HAL_SPI_Transmit(
        spi_port[bus].hard_spi_handle, data, size, timeout);

    osal_mutex_give(spi_port[bus].os_mutexid);

    return (ret == HAL_OK) ? CORE_SPI_OK : CORE_SPI_ERROR;
}

core_spi_status_t core_hard_spi_receive(core_spi_bus_t bus,
                                             uint8_t *data,
                                             uint16_t  size,
                                             uint32_t  timeout)
{
    if (bus >= CORE_SPI_BUS_MAX || NULL == data)
    {
        return CORE_SPI_ERROR;
    }

    if (osal_mutex_take(spi_port[bus].os_mutexid,
                        (osal_tick_type_t)timeout) != 0)
    {
        return CORE_SPI_TIMEOUT;
    }

    HAL_StatusTypeDef ret = HAL_SPI_Receive(
        spi_port[bus].hard_spi_handle, data, size, timeout);

    osal_mutex_give(spi_port[bus].os_mutexid);

    return (ret == HAL_OK) ? CORE_SPI_OK : CORE_SPI_ERROR;
}

core_spi_status_t core_hard_spi_transmit_receive(core_spi_bus_t bus,
                                                       uint8_t *tx_data,
                                                       uint8_t *rx_data,
                                                       uint16_t   size,
                                                       uint32_t timeout)
{
    if (bus >= CORE_SPI_BUS_MAX || NULL == tx_data || NULL == rx_data)
    {
        return CORE_SPI_ERROR;
    }

    if (osal_mutex_take(spi_port[bus].os_mutexid,
                        (osal_tick_type_t)timeout) != 0)
    {
        return CORE_SPI_TIMEOUT;
    }

    HAL_StatusTypeDef ret = HAL_SPI_TransmitReceive(
        spi_port[bus].hard_spi_handle, tx_data, rx_data, size, timeout);

    osal_mutex_give(spi_port[bus].os_mutexid);

    return (ret == HAL_OK) ? CORE_SPI_OK : CORE_SPI_ERROR;
}

core_spi_status_t core_hard_spi_transmit_dma(core_spi_bus_t bus,
                                                   uint8_t *data,
                                                   uint16_t  size)
{
    if (bus >= CORE_SPI_BUS_MAX || NULL == data)
    {
        return CORE_SPI_ERROR;
    }

    /* DMA transfer is non-blocking; take mutex here, caller releases via
       core_hard_spi_dma_complete() or the DMA-complete callback. */
    if (osal_mutex_take(spi_port[bus].os_mutexid, OSAL_MAX_DELAY) != 0)
    {
        return CORE_SPI_TIMEOUT;
    }

    HAL_StatusTypeDef ret = HAL_SPI_Transmit_DMA(
        spi_port[bus].hard_spi_handle, data, size);

    if (ret != HAL_OK)
    {
        osal_mutex_give(spi_port[bus].os_mutexid);
        return CORE_SPI_ERROR;
    }

    return CORE_SPI_OK;
}

core_spi_status_t core_hard_spi_receive_dma(core_spi_bus_t bus,
                                                  uint8_t *data,
                                                  uint16_t  size)
{
    if (bus >= CORE_SPI_BUS_MAX || NULL == data)
    {
        return CORE_SPI_ERROR;
    }

    if (osal_mutex_take(spi_port[bus].os_mutexid, OSAL_MAX_DELAY) != 0)
    {
        return CORE_SPI_TIMEOUT;
    }

    HAL_StatusTypeDef ret = HAL_SPI_Receive_DMA(
        spi_port[bus].hard_spi_handle, data, size);

    if (ret != HAL_OK)
    {
        osal_mutex_give(spi_port[bus].os_mutexid);
        return CORE_SPI_ERROR;
    }

    return CORE_SPI_OK;
}

void core_hard_spi_dma_complete(core_spi_bus_t bus)
{
    if (bus >= CORE_SPI_BUS_MAX)
    {
        return;
    }
    osal_mutex_give(spi_port[bus].os_mutexid);
}
#endif /* HAL_SPI_MODULE_ENABLED */

/* ---------- Software SPI bus primitives ---------- */

core_spi_status_t core_soft_spi_cs_select(core_spi_bus_t bus)
{
    if (bus >= CORE_SPI_BUS_MAX)
    {
        return CORE_SPI_ERROR;
    }

    spi_cs_select(&spi_port[bus].soft_spi_bus_inst);
    return CORE_SPI_OK;
}

core_spi_status_t core_soft_spi_cs_deselect(core_spi_bus_t bus)
{
    if (bus >= CORE_SPI_BUS_MAX)
    {
        return CORE_SPI_ERROR;
    }

    spi_cs_deselect(&spi_port[bus].soft_spi_bus_inst);
    return CORE_SPI_OK;
}

core_spi_status_t core_soft_spi_write_byte(core_spi_bus_t bus, uint8_t byte)
{
    if (bus >= CORE_SPI_BUS_MAX)
    {
        return CORE_SPI_ERROR;
    }

    spi_write_byte(&spi_port[bus].soft_spi_bus_inst, byte);
    return CORE_SPI_OK;
}

core_spi_status_t core_soft_spi_read_byte(core_spi_bus_t bus, uint8_t *byte)
{
    if (bus >= CORE_SPI_BUS_MAX || NULL == byte)
    {
        return CORE_SPI_ERROR;
    }

    *byte = spi_read_byte(&spi_port[bus].soft_spi_bus_inst);
    return CORE_SPI_OK;
}

core_spi_status_t core_soft_spi_readwrite_byte(core_spi_bus_t bus,
                                                    uint8_t  tx_byte,
                                                    uint8_t *rx_byte)
{
    if (bus >= CORE_SPI_BUS_MAX || NULL == rx_byte)
    {
        return CORE_SPI_ERROR;
    }

    *rx_byte = spi_readwrite_byte(&spi_port[bus].soft_spi_bus_inst, tx_byte);
    return CORE_SPI_OK;
}

//******************************* Functions *********************************//

/******************************************************************************
 * @file spi_hal.c
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

//******************************** Includes *********************************//
#include "spi_hal.h"
#include "dwt_port.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//

//******************************** Defines **********************************//

//******************************* Declaring *********************************//

//******************************* Declaring *********************************//

//******************************* Functions *********************************//
static void spi_sck_output(spi_bus_t *bus, uint8_t val)
{
    if (NULL == bus)
    {
        return;
    }

    HAL_GPIO_WritePin(bus->spi_sck_port, bus->spi_sck_pin,
                      val > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void spi_mosi_output(spi_bus_t *bus, uint8_t val)
{
    if (NULL == bus)
    {
        return;
    }

    HAL_GPIO_WritePin(bus->spi_mosi_port, bus->spi_mosi_pin,
                      val > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t spi_miso_input(spi_bus_t *bus)
{
    if (NULL == bus)
    {
        return 0;
    }

    return (HAL_GPIO_ReadPin(bus->spi_miso_port, bus->spi_miso_pin) == GPIO_PIN_SET) ? 1 : 0;
}

static void spi_cs_output(spi_bus_t *bus, uint8_t val)
{
    if (NULL == bus)
    {
        return;
    }

    HAL_GPIO_WritePin(bus->spi_cs_port, bus->spi_cs_pin,
                      val > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* ---------- CS control ---------- */

void spi_cs_select(spi_bus_t *bus)
{
    spi_cs_output(bus, 0);
}

void spi_cs_deselect(spi_bus_t *bus)
{
    spi_cs_output(bus, 1);
}

/* ---------- single-byte primitives (no CS control) ---------- */

void spi_write_byte(spi_bus_t *bus, uint8_t byte)
{
    if (NULL == bus)
    {
        return;
    }

    for (uint8_t i = 8; i > 0; i--)
    {
        // spi mode 0: MOSI must be valid before rising edge (slave samples on rising edge)
        spi_mosi_output(bus, (byte >> (i - 1)) & 0x01);
        core_dwt_delay_us(3);
        spi_sck_output(bus, 1);
        core_dwt_delay_us(3);
        spi_sck_output(bus, 0);
    }
}

uint8_t spi_read_byte(spi_bus_t *bus)
{
    if (NULL == bus)
    {
        return 0;
    }

    uint8_t byte = 0;

    for (uint8_t i = 8; i > 0; i--)
    {
        // spi mode 0: sample MISO after rising edge
        core_dwt_delay_us(3);
        spi_sck_output(bus, 1);
        core_dwt_delay_us(3);
        byte = (byte << 1) | spi_miso_input(bus);
        spi_sck_output(bus, 0);
    }

    return byte;
}

uint8_t spi_readwrite_byte(spi_bus_t *bus, uint8_t tx_byte)
{
    if (NULL == bus)
    {
        return 0;
    }

    uint8_t rx_byte = 0;

    for (uint8_t i = 8; i > 0; i--)
    {
        // spi mode 0: MOSI valid before rising edge, sample MISO after rising edge
        spi_mosi_output(bus, (tx_byte >> (i - 1)) & 0x01);
        core_dwt_delay_us(3);
        spi_sck_output(bus, 1);
        core_dwt_delay_us(3);
        rx_byte = (rx_byte << 1) | spi_miso_input(bus);
        spi_sck_output(bus, 0);
    }

    return rx_byte;
}

/* ---------- legacy wrappers (CS included, kept for compatibility) ---------- */

void spi_send_byte(spi_bus_t *bus, uint8_t byte)
{
    spi_cs_select(bus);
    spi_write_byte(bus, byte);
    spi_cs_deselect(bus);
}

uint8_t spi_receive_byte(spi_bus_t *bus)
{
    spi_cs_select(bus);
    uint8_t byte = spi_read_byte(bus);
    spi_cs_deselect(bus);
    return byte;
}

uint8_t spi_transfer_byte(spi_bus_t *bus, uint8_t tx_byte)
{
    spi_cs_select(bus);
    uint8_t rx_byte = spi_readwrite_byte(bus, tx_byte);
    spi_cs_deselect(bus);
    return rx_byte;
}

void spi_init(spi_bus_t *bus)
{
    if (NULL == bus)
    {
        return;
    }

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull  = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStructure.Pin   = bus->spi_sck_pin;
    HAL_GPIO_Init(bus->spi_sck_port, &GPIO_InitStructure);
    
    GPIO_InitStructure.Pin = bus->spi_mosi_pin;
    HAL_GPIO_Init(bus->spi_mosi_port, &GPIO_InitStructure);
    
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pin  = bus->spi_miso_pin;
    HAL_GPIO_Init(bus->spi_miso_port, &GPIO_InitStructure);

    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pin  = bus->spi_cs_pin;
    HAL_GPIO_Init(bus->spi_cs_port, &GPIO_InitStructure);

    HAL_GPIO_WritePin(bus->spi_cs_port, bus->spi_cs_pin, GPIO_PIN_SET); // Set CS high (inactive)
}

//******************************* Functions *********************************//

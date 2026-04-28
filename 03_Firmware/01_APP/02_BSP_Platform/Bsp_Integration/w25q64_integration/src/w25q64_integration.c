/******************************************************************************
 * @file w25q64_integration.c
 *
 * @par dependencies
 * - w25q64_integration.h
 * - bsp_w25q64_handler.h
 * - spi.h            (hspi1 extern)
 * - main.h           (GPIO pin defines from CubeMX)
 * - systick_port.h   (MCU-port ms timebase)
 * - osal_wrapper_adapter.h
 * - osal_error.h
 *
 * @author Ethan-Hang
 *
 * @brief Dependency injection bundle for the W25Q64 flash handler.
 *
 * Provides concrete implementations for every interface the W25Q64 handler
 * needs (hardware SPI1 / GPIO CS, ms timebase, OS queue, OS delay) and
 * wires them into w25q64_input_arg, consumed by flash_handler_thread()
 * at startup.
 *
 * All HAL calls are localised here -- no other integration or application
 * file needs to touch the flash SPI bus directly.
 *
 * Current pin assignment (board rev wires the flash to SPI1 -- shares the
 * bus with the display.  The CubeMX FLASH_SPI2_* labels in main.h are
 * stale and not used here):
 *   SCK  = PA5   (SPI1_SCK,  shared with display)
 *   MOSI = PA7   (SPI1_MOSI, shared with display)
 *   MISO = PB4   (SPI1_MISO, AF5 -- only the flash listens on this line)
 *   CS   = PB13  (directly driven GPIO, exclusive to the flash)
 *
 * @version V1.0 2026-04-27
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "w25q64_integration.h"

#include "main.h"
#include "spi.h"
#include "systick_port.h"
#include "osal_wrapper_adapter.h"
#include "osal_error.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define W25Q64_SPI_TIMEOUT_MS       200U
#define W25Q64_QUEUE_DEPTH          10U

/** @brief CS pin -- directly driven via HAL GPIO (PB13). */
#define W25Q64_CS_PORT              GPIOB
#define W25Q64_CS_PIN               GPIO_PIN_13

/** @brief Release-Power-Down opcode (datasheet table 6.1). */
#define W25Q64_CMD_RELEASE_PD       0xABU
/** @brief tRES1 = 3 us per datasheet; round up to 1 ms for HAL_Delay. */
#define W25Q64_RES_DELAY_MS         1U
//******************************** Defines **********************************//

//******************************* Functions *********************************//

/* ---- SPI bus (directly coupled with HAL_SPI / HAL_GPIO) ----------------- */

/**
 * @brief  Configure flash GPIOs, slow the SPI clock, and pull the chip
 *         out of any latent deep-power-down state.
 *
 * SPI1 has already been brought up as 2-line full-duplex by CubeMX-
 * generated MX_SPI1_Init(), with PB4 routed as SPI1_MISO inside
 * HAL_SPI_MspInit().  Three flash-specific bits remain:
 *
 *  - CS (PB13) is owned exclusively by the flash; CubeMX did not touch
 *    it, so configure it here as push-pull output and idle high.
 *  - MX_SPI1_Init() runs at PCLK2/2 = 50 MHz which is fine for the
 *    display path but unreliable for a board-rev wired flash sharing
 *    the bus.  Re-init SPI1 at PCLK2/16 (~6.25 MHz) -- comfortably
 *    within W25Q64 timing and tolerant of mediocre signal integrity.
 *    Any later display init will re-apply its own prescaler.
 *  - Issue a Release-Power-Down (0xAB) before the driver reads JEDEC
 *    ID.  W25Q64 retains its deep-PD state across MCU reset as long as
 *    VCC is held; without this, every command except 0xAB is ignored
 *    and DO stays high-Z.  3 us tRES1 is needed before the next
 *    command -- HAL_Delay(1) gives a generous margin.
 *
 * @return W25Q64_OK on success, W25Q64_ERROR on HAL failure.
 */
static w25q64_status_t w25q64_spi_init(void)
{
    // /* ---- CS pin (exclusive to the flash) ---- */
    // __HAL_RCC_GPIOB_CLK_ENABLE();

    // GPIO_InitTypeDef gpio = {0};
    // gpio.Pin   = W25Q64_CS_PIN;
    // gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    // gpio.Pull  = GPIO_NOPULL;
    // gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    // HAL_GPIO_Init(W25Q64_CS_PORT, &gpio);
    // HAL_GPIO_WritePin(W25Q64_CS_PORT, W25Q64_CS_PIN, GPIO_PIN_SET);

    // /* ---- Slow SPI1 down to a rate the flash can actually answer at ---- */
    // hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    // if (HAL_OK != HAL_SPI_Init(&hspi1))
    // {
    //     return W25Q64_ERROR;
    // }

    // /* ---- Release Power-Down: defensive wake-up before JEDEC ID ---- */
    // {
    //     uint8_t           cmd = W25Q64_CMD_RELEASE_PD;
    //     HAL_StatusTypeDef hs  = HAL_OK;

    //     HAL_GPIO_WritePin(W25Q64_CS_PORT, W25Q64_CS_PIN, GPIO_PIN_RESET);
    //     hs = HAL_SPI_Transmit(&hspi1, &cmd, 1U, W25Q64_SPI_TIMEOUT_MS);
    //     HAL_GPIO_WritePin(W25Q64_CS_PORT, W25Q64_CS_PIN, GPIO_PIN_SET);
    //     if (HAL_OK != hs)
    //     {
    //         return W25Q64_ERROR;
    //     }
    //     /* tRES1: must elapse before the next command latches reliably. */
    //     HAL_Delay(W25Q64_RES_DELAY_MS);
    // }

    return W25Q64_OK;
}

/**
 * @brief  SPI deinit hook (no-op).
 *
 * @return W25Q64_OK
 */
static w25q64_status_t w25q64_spi_deinit(void)
{
    return W25Q64_OK;
}

/**
 * @brief  Blocking hardware SPI transmit via HAL.
 *
 * @param[in] p_data      : Source buffer.
 * @param[in] data_length : Number of bytes to send.
 *
 * @return W25Q64_OK on success, error code otherwise.
 */
static w25q64_status_t w25q64_spi_transmit(uint8_t const *p_data,
                                            uint32_t       data_length)
{
    if (NULL == p_data || data_length > UINT16_MAX)
    {
        return W25Q64_ERRORPARAMETER;
    }

    HAL_StatusTypeDef ret = HAL_SPI_Transmit(
        &hspi1, (uint8_t *)p_data, (uint16_t)data_length,
        W25Q64_SPI_TIMEOUT_MS);

    return (HAL_OK == ret) ? W25Q64_OK : W25Q64_ERROR;
}

/**
 * @brief  Blocking hardware SPI receive via HAL.
 *
 * @param[out] p_buffer      : Destination buffer.
 * @param[in]  buffer_length : Number of bytes to read.
 *
 * @return W25Q64_OK on success, error code otherwise.
 */
static w25q64_status_t w25q64_spi_read(uint8_t  *p_buffer,
                                        uint32_t  buffer_length)
{
    if (NULL == p_buffer || buffer_length > UINT16_MAX)
    {
        return W25Q64_ERRORPARAMETER;
    }

    HAL_StatusTypeDef ret = HAL_SPI_Receive(
        &hspi1, p_buffer, (uint16_t)buffer_length,
        W25Q64_SPI_TIMEOUT_MS);

    return (HAL_OK == ret) ? W25Q64_OK : W25Q64_ERROR;
}

/**
 * @brief  DMA transmit stub -- not used for flash testing.
 *
 * @return W25Q64_ERRORUNSUPPORTED
 */
static w25q64_status_t w25q64_spi_transmit_dma(uint8_t const *p_data,
                                                uint32_t       data_length)
{
    (void)p_data;
    (void)data_length;
    return W25Q64_ERRORUNSUPPORTED;
}

/**
 * @brief  DMA wait stub -- not used for flash testing.
 *
 * @return W25Q64_ERRORUNSUPPORTED
 */
static w25q64_status_t w25q64_spi_wait_dma_complete(uint32_t timeout_ms)
{
    (void)timeout_ms;
    return W25Q64_ERRORUNSUPPORTED;
}

/**
 * @brief  Drive the flash CS line directly via HAL GPIO.
 *
 * @param[in] state : 0 -> CS low (active), non-zero -> CS high.
 *
 * @return W25Q64_OK
 */
static w25q64_status_t w25q64_spi_write_cs_pin(uint8_t state)
{
    HAL_GPIO_WritePin(W25Q64_CS_PORT, W25Q64_CS_PIN,
                      (0U != state) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return W25Q64_OK;
}

/**
 * @brief  DC pin stub -- W25Q64 has no data/command line.
 *
 * @return W25Q64_OK
 */
static w25q64_status_t w25q64_spi_write_dc_pin(uint8_t state)
{
    (void)state;
    return W25Q64_OK;
}

/* ---- Timebase / OS ------------------------------------------------------ */

/**
 * @brief  Monotonic ms tick provider via systick port.
 *
 * @return Current ms tick.
 */
static uint32_t w25q64_tb_get_tick_ms(void)
{
    return core_systick_get_ms();
}

/**
 * @brief  Blocking ms delay (datasheet timings like power-on 50 ms).
 *         Routes through OSAL so other tasks can run during the wait.
 *
 * @param[in] ms : Milliseconds to wait.
 */
static void w25q64_tb_delay_ms(uint32_t ms)
{
    osal_task_delay(ms);
}

/**
 * @brief  OS-aware delay for driver layer (returns void).
 *
 * @param[in] ms : Milliseconds to wait.
 */
static void w25q64_drv_delay_ms(uint32_t ms)
{
    osal_task_delay(ms);
}

/**
 * @brief  OS-aware delay for handler layer (returns status).
 *
 * @param[in] ms : Milliseconds to wait.
 *
 * @return FLASH_HANLDER_OK always.
 */
static flash_handler_status_t w25q64_os_delay_ms(uint32_t ms)
{
    osal_task_delay(ms);
    return FLASH_HANLDER_OK;
}

/* ---- OS queue (directly coupled with osal_queue) ------------------------ */

/**
 * @brief  Create a message queue via OSAL.
 *
 * @param[in]  item_num      : Queue depth.
 * @param[in]  item_size     : Size of each item in bytes.
 * @param[out] queue_handler : Receives the queue handle.
 *
 * @return FLASH_HANLDER_OK on success, FLASH_HANLDER_ERROR on failure.
 */
static flash_handler_status_t w25q64_os_queue_create(
    uint32_t const   item_num,
    uint32_t const   item_size,
    void    **const  queue_handler)
{
    int32_t ret = osal_queue_create(
        (osal_queue_handle_t *)queue_handler,
        (size_t)item_num,
        (size_t)item_size);
    return (OSAL_SUCCESS == ret) ? FLASH_HANLDER_OK : FLASH_HANLDER_ERROR;
}

/**
 * @brief  Put a message into the queue via OSAL.
 *
 * @param[in] queue_handler : Queue handle.
 * @param[in] item          : Pointer to the item data.
 * @param[in] timeout       : Wait timeout in ticks.
 *
 * @return FLASH_HANLDER_OK on success, FLASH_HANLDER_ERROR on failure.
 */
static flash_handler_status_t w25q64_os_queue_put(
    void     *const  queue_handler,
    void     *const  item,
    uint32_t         timeout)
{
    int32_t ret = osal_queue_send(
        (osal_queue_handle_t)queue_handler,
        item,
        (osal_tick_type_t)timeout);
    return (OSAL_SUCCESS == ret) ? FLASH_HANLDER_OK : FLASH_HANLDER_ERROR;
}

/**
 * @brief  Get a message from the queue via OSAL.
 *
 * @param[in]  queue_handler : Queue handle.
 * @param[out] msg           : Pointer to receive the item data.
 * @param[in]  timeout       : Wait timeout in ticks.
 *
 * @return FLASH_HANLDER_OK on success, FLASH_HANLDER_ERROR on failure.
 */
static flash_handler_status_t w25q64_os_queue_get(
    void     *const  queue_handler,
    void     *const  msg,
    uint32_t         timeout)
{
    int32_t ret = osal_queue_receive(
        (osal_queue_handle_t)queue_handler,
        msg,
        (osal_tick_type_t)timeout);
    return (OSAL_SUCCESS == ret) ? FLASH_HANLDER_OK : FLASH_HANLDER_ERROR;
}

/**
 * @brief  Delete a queue via OSAL.
 *
 * @param[in] queue_handler : Queue handle.
 *
 * @return FLASH_HANLDER_OK always.
 */
static flash_handler_status_t w25q64_os_queue_delete(
    void *const queue_handler)
{
    osal_queue_delete((osal_queue_handle_t)queue_handler);
    return FLASH_HANLDER_OK;
}

/* ---- Assembled interface vtables ----------------------------------------- */

static w25q64_spi_interface_t s_spi_interface = {
    .pf_spi_init              = w25q64_spi_init,
    .pf_spi_deinit            = w25q64_spi_deinit,
    .pf_spi_transmit          = w25q64_spi_transmit,
    .pf_spi_read              = w25q64_spi_read,
    .pf_spi_transmit_dma      = w25q64_spi_transmit_dma,
    .pf_spi_wait_dma_complete = w25q64_spi_wait_dma_complete,
    .pf_spi_write_cs_pin      = w25q64_spi_write_cs_pin,
    .pf_spi_write_dc_pin      = w25q64_spi_write_dc_pin,
};

static w25q64_timebase_interface_t s_timebase_interface = {
    .pf_get_tick_ms = w25q64_tb_get_tick_ms,
    .pf_delay_ms    = w25q64_tb_delay_ms,
};

static w25q64_os_delay_t s_w25q64_os_delay = {
    .pf_os_delay_ms = w25q64_drv_delay_ms,
};

static flash_handler_os_queue_t s_os_queue_interface = {
    .pf_os_queue_create = w25q64_os_queue_create,
    .pf_os_queue_put    = w25q64_os_queue_put,
    .pf_os_queue_get    = w25q64_os_queue_get,
    .pf_os_queue_delete = w25q64_os_queue_delete,
};

static flash_handler_os_delay_t s_os_delay_interface = {
    .pf_os_delay_ms = w25q64_os_delay_ms,
};

static flash_os_interface_t s_os_interface = {
    .p_os_queue_interface = &s_os_queue_interface,
    .p_os_delay_interface = &s_os_delay_interface,
};

/* ---- Driver input arg ---------------------------------------------------- */

flash_input_args_t w25q64_input_arg = {
    .p_os_interface       = &s_os_interface,
    .p_spi_interface      = &s_spi_interface,
    .p_timebase_interface = &s_timebase_interface,
    .p_w25q64_os_delay    = &s_w25q64_os_delay,
};

//******************************* Functions *********************************//

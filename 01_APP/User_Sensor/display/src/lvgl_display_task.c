/******************************************************************************
 * @file lvgl_display_task.c
 *
 * @par dependencies
 * - main.h, spi.h, i2c.h
 * - bsp_st7789_driver.h
 * - bsp_cst816t_driver.h
 * - lvgl.h
 * - lv_port_disp.h
 * - lv_port_indev.h
 * - gui_guider.h
 *
 * @author Ethan-Hang
 *
 * @brief LVGL bring-up task: owns ST7789 (SPI1) + CST816T (I2C1) drivers,
 *        wires them into LVGL's display + pointer indev, and hands the
 *        screen off to the NXP gui_guider-generated UI (setup_ui).
 *
 * Processing flow:
 *   bind ST7789 + CST816T HAL passthroughs ->
 *   bsp_st7789_driver_inst -> pf_st7789_init ->
 *   bsp_cst816t_inst -> probe chip id ->
 *   lv_init -> lv_port_disp_init -> lv_port_indev_init ->
 *   setup_ui(&guider_ui) ->
 *   loop { lv_timer_handler; delay 5 ms }
 *
 * @note  CST816T is polled by LVGL's indev loop every LV_INDEV_DEF_READ_PERIOD
 *        (10 ms).  PB2 EXTI is left wired but unused here — gui_guider drives
 *        all touch interaction through LVGL's own gesture decoder.
 *
 * @version V1.0 2026-04-25
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "main.h"
#include "spi.h"
#include "i2c.h"
#include "stm32f4xx_hal.h"

#include "user_task_reso_config.h"
#include "bsp_st7789_driver.h"
#include "bsp_st7789_reg.h"
#include "bsp_cst816t_driver.h"
#include "Debug.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "gui_guider.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/* Visible panel geometry — gui_guider screens are designed for 240x280. */
#define LV_TASK_PANEL_WIDTH       240U
#define LV_TASK_PANEL_HEIGHT      280U
#define LV_TASK_PANEL_X_OFFSET    0U
#define LV_TASK_PANEL_Y_OFFSET    20U

#define LV_TASK_BOOT_WAIT_MS      2000U
#define LV_TASK_TIMER_PERIOD_MS   5U

#define LV_TASK_SPI_TX_TIMEOUT_MS 100U
#define LV_TASK_IIC_TIMEOUT_MS    1000U
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
/* gui_guider.h declares `extern lv_ui guider_ui` but the generated tree does
 * not provide a definition; supply it here so setup_ui()/screen handlers can
 * link.  Keeping it file-private to lvgl_display_task gives one owner. */
lv_ui guider_ui;

static bsp_st7789_driver_t              s_lv_driver;
static st7789_spi_interface_t           s_lv_spi;
static st7789_timebase_interface_t      s_lv_timebase;
static st7789_os_interface_t            s_lv_os;
static const st7789_panel_config_t      s_lv_panel = {
    .width    = LV_TASK_PANEL_WIDTH,
    .height   = LV_TASK_PANEL_HEIGHT,
    .x_offset = LV_TASK_PANEL_X_OFFSET,
    .y_offset = LV_TASK_PANEL_Y_OFFSET,
};

static bsp_cst816t_driver_t             s_touch_driver;
static cst816t_iic_driver_interface_t   s_touch_iic;
static cst816t_timebase_interface_t     s_touch_timebase;
static cst816t_delay_interface_t        s_touch_delay;
static cst816t_os_delay_interface_t     s_touch_os;
static void (*s_touch_int_cb)(void *, void *) = NULL;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
/* ---- ST7789 SPI passthrough (SPI1) -------------------------------------- */
static st7789_status_t lv_spi_init(void)         { return ST7789_OK; }
static st7789_status_t lv_spi_deinit(void)       { return ST7789_OK; }

static st7789_status_t lv_spi_transmit(uint8_t const *p_data,
                                       uint32_t       data_length)
{
    if (data_length > UINT16_MAX)
    {
        return ST7789_ERRORPARAMETER;
    }
    HAL_StatusTypeDef hs =
        HAL_SPI_Transmit(&hspi1, (uint8_t *)p_data, (uint16_t)data_length,
                         LV_TASK_SPI_TX_TIMEOUT_MS);
    return (HAL_OK == hs) ? ST7789_OK : ST7789_ERROR;
}

static st7789_status_t lv_spi_transmit_dma(uint8_t const *p_data,
                                           uint32_t       data_length)
{
    if (data_length > UINT16_MAX)
    {
        return ST7789_ERRORPARAMETER;
    }
    HAL_StatusTypeDef hs =
        HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)p_data, (uint16_t)data_length);
    return (HAL_OK == hs) ? ST7789_OK : ST7789_ERROR;
}

static st7789_status_t lv_spi_wait_dma_complete(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY)
    {
        if ((HAL_GetTick() - start) > timeout_ms)
        {
            return ST7789_ERRORTIMEOUT;
        }
    }
    return ST7789_OK;
}

static st7789_status_t lv_spi_write_cs_pin(uint8_t state)
{
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin,
                      (0U != state) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return ST7789_OK;
}

static st7789_status_t lv_spi_write_dc_pin(uint8_t state)
{
    HAL_GPIO_WritePin(SPI1_DC_GPIO_Port, SPI1_DC_Pin,
                      (0U != state) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return ST7789_OK;
}

static st7789_status_t lv_spi_write_rst_pin(uint8_t state)
{
    HAL_GPIO_WritePin(SPI1_RST_GPIO_Port, SPI1_RST_Pin,
                      (0U != state) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return ST7789_OK;
}

static uint32_t lv_tb_get_tick_ms(void)            { return HAL_GetTick(); }
static void     lv_tb_delay_ms(uint32_t ms)        { osal_task_delay(ms); }
static void     lv_os_delay_ms(uint32_t ms)        { osal_task_delay(ms); }

static st7789_status_t lv_disp_driver_bind(void)
{
    s_lv_spi.pf_spi_init              = lv_spi_init;
    s_lv_spi.pf_spi_deinit            = lv_spi_deinit;
    s_lv_spi.pf_spi_transmit          = lv_spi_transmit;
    s_lv_spi.pf_spi_transmit_dma      = lv_spi_transmit_dma;
    s_lv_spi.pf_spi_wait_dma_complete = lv_spi_wait_dma_complete;
    s_lv_spi.pf_spi_write_cs_pin      = lv_spi_write_cs_pin;
    s_lv_spi.pf_spi_write_dc_pin      = lv_spi_write_dc_pin;
    s_lv_spi.pf_spi_write_rst_pin     = lv_spi_write_rst_pin;

    s_lv_timebase.pf_get_tick_ms      = lv_tb_get_tick_ms;
    s_lv_timebase.pf_delay_ms         = lv_tb_delay_ms;

    s_lv_os.pf_os_delay_ms            = lv_os_delay_ms;

    return bsp_st7789_driver_inst(&s_lv_driver, &s_lv_spi, &s_lv_timebase,
                                  &s_lv_os, &s_lv_panel);
}

/* ---- CST816T I2C passthrough (I2C1) ------------------------------------- */
static cst816t_status_t lv_iic_init(void const * const hi2c)
{
    (void)hi2c;
    return CST816T_OK;
}
static cst816t_status_t lv_iic_deinit(void const * const hi2c)
{
    (void)hi2c;
    return CST816T_OK;
}

static cst816t_status_t lv_iic_mem_write(void    *i2c,
                                         uint16_t des_addr,
                                         uint16_t mem_addr,
                                         uint16_t mem_size,
                                         uint8_t *p_data,
                                         uint16_t size,
                                         uint32_t timeout)
{
    (void)i2c;
    HAL_StatusTypeDef hs = HAL_I2C_Mem_Write(&hi2c1, des_addr, mem_addr,
                                             mem_size, p_data, size, timeout);
    return (HAL_OK == hs) ? CST816T_OK : CST816T_ERROR;
}

static cst816t_status_t lv_iic_mem_read(void    *i2c,
                                        uint16_t des_addr,
                                        uint16_t mem_addr,
                                        uint16_t mem_size,
                                        uint8_t *p_data,
                                        uint16_t size,
                                        uint32_t timeout)
{
    (void)i2c;
    HAL_StatusTypeDef hs = HAL_I2C_Mem_Read(&hi2c1, des_addr, mem_addr,
                                            mem_size, p_data, size, timeout);
    return (HAL_OK == hs) ? CST816T_OK : CST816T_ERROR;
}

static uint32_t lv_touch_tb_get_tick_count(void)        { return HAL_GetTick(); }
static void     lv_touch_delay_init(void)               { /* nothing */ }
static void     lv_touch_delay_ms(uint32_t const ms)    { osal_task_delay(ms); }
static void     lv_touch_delay_us(uint32_t const us)    { (void)us; }
static void     lv_touch_os_yield(uint32_t const ms)    { osal_task_delay(ms); }

static cst816t_status_t lv_touch_driver_bind(void)
{
    s_touch_iic.hi2c              = (void *)&hi2c1;
    s_touch_iic.pf_iic_init       = lv_iic_init;
    s_touch_iic.pf_iic_deinit     = lv_iic_deinit;
    s_touch_iic.pf_iic_mem_write  = lv_iic_mem_write;
    s_touch_iic.pf_iic_mem_read   = lv_iic_mem_read;

    s_touch_timebase.pf_get_tick_count = lv_touch_tb_get_tick_count;

    s_touch_delay.pf_delay_init   = lv_touch_delay_init;
    s_touch_delay.pf_delay_ms     = lv_touch_delay_ms;
    s_touch_delay.pf_delay_us     = lv_touch_delay_us;

    s_touch_os.pf_rtos_yield      = lv_touch_os_yield;

    return bsp_cst816t_inst(&s_touch_driver, &s_touch_iic, &s_touch_timebase,
                            &s_touch_delay, &s_touch_os, &s_touch_int_cb);
}

/* ---- Task entry --------------------------------------------------------- */
/**
 * @brief      LVGL + gui_guider task entry: brings up display, touch, then
 *             hands control to the NXP-generated UI.
 *
 * @param[in]  argument : Unused.
 *
 * @return     None.
 * */
void lvgl_display_task(void *argument)
{
    (void)argument;

    osal_task_delay(LV_TASK_BOOT_WAIT_MS);

    DEBUG_OUT(i, ST7789_LOG_TAG, "lvgl_display_task start (gui_guider)");

    /* 1. Display (ST7789 / SPI1) */
    st7789_status_t dret = lv_disp_driver_bind();
    if (ST7789_OK != dret)
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG, "st7789 inst failed = %d", (int)dret);
        for (;;)
        {
            osal_task_delay(1000U);
        }
    }
    dret = s_lv_driver.pf_st7789_init(&s_lv_driver);
    if (ST7789_OK != dret)
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG, "st7789 init failed = %d", (int)dret);
        for (;;)
        {
            osal_task_delay(1000U);
        }
    }
    (void)s_lv_driver.pf_st7789_fill_color(&s_lv_driver, BLACK);

    /* 2. Touch (CST816T / I2C1) */
    cst816t_status_t cret = lv_touch_driver_bind();
    if (CST816T_OK != cret)
    {
        DEBUG_OUT(e, CST816T_ERR_LOG_TAG, "cst816t inst failed = %d",
                  (int)cret);
        /* Display still works, so don't block — LVGL will run without
         * touch and the screen is usable for a visual smoke test. */
    }
    else
    {
        uint8_t chip_id = 0u;
        cret = s_touch_driver.pf_cst816t_get_chip_id(s_touch_driver, &chip_id);
        if (CST816T_OK == cret)
        {
            DEBUG_OUT(i, CST816T_LOG_TAG, "cst816t chip_id = 0x%02X",
                      (unsigned)chip_id);
        }
    }

    /* 3. LVGL core + display port */
    lv_init();
    if (!lv_port_disp_init(&s_lv_driver))
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG, "lvgl disp port init failed");
        for (;;)
        {
            osal_task_delay(1000U);
        }
    }

    /* 4. LVGL pointer indev (skip if touch bind failed earlier) */
    if (CST816T_OK == cret)
    {
        if (!lv_port_indev_init(&s_touch_driver))
        {
            DEBUG_OUT(e, CST816T_ERR_LOG_TAG, "lvgl indev port init failed");
        }
    }

    /* 5. Hand off to gui_guider's generated UI */
    setup_ui(&guider_ui);
    DEBUG_OUT(i, ST7789_LOG_TAG, "lvgl_display_task: gui_guider UI loaded");

    /* 6. LVGL service loop */
    for (;;)
    {
        lv_timer_handler();
        osal_task_delay(LV_TASK_TIMER_PERIOD_MS);
    }
}
//******************************* Functions *********************************//

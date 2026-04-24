/******************************************************************************
 * @file lvgl_display_task.c
 *
 * @par dependencies
 * - main.h (pin defines)
 * - spi.h  (hspi1)
 * - bsp_st7789_driver.h
 * - lvgl.h
 * - lv_port_disp.h
 *
 * @author Ethan-Hang
 *
 * @brief LVGL bring-up task: owns a dedicated ST7789 driver instance, wires
 *        it into LVGL and pumps lv_timer_handler() periodically.  Draws a
 *        simple "Hello LVGL" screen so the port can be verified visually.
 *
 * Processing flow:
 *   bind SPI/timebase/os vtables ->
 *   bsp_st7789_driver_inst -> pf_st7789_init ->
 *   lv_init -> lv_port_disp_init -> build demo UI ->
 *   loop { lv_timer_handler; delay }
 *
 * @version V1.0 2026-04-24
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
#include "stm32f4xx_hal.h"

#include "user_task_reso_config.h"
#include "bsp_st7789_driver.h"
#include "bsp_st7789_reg.h"
#include "Debug.h"

#include "lvgl.h"
#include "lv_port_disp.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/* Visible panel geometry.  Default is a 1.69" 240x280 ST7789 whose visible
 * window is RAM rows 20..299 (i.e. the 240x320 controller is centered on the
 * 240x280 panel, leaving 20 dead rows top and bottom in RAM).
 * Known alternates:
 *   1.3" 240x240, centered : HEIGHT=240, Y_OFFSET=40
 *   1.3" 240x240, edge     : HEIGHT=240, Y_OFFSET=80
 *   2.0"/2.4" 240x320 full : HEIGHT=320, Y_OFFSET=0
 * Keep LV_PORT_DISP_VER_RES (lv_port_disp.c) in sync with HEIGHT.
 */
#define LV_TASK_PANEL_WIDTH       240U
#define LV_TASK_PANEL_HEIGHT      280U
#define LV_TASK_PANEL_X_OFFSET    0U
#define LV_TASK_PANEL_Y_OFFSET    20U

#define LV_TASK_BOOT_WAIT_MS      2000U
#define LV_TASK_TIMER_PERIOD_MS   5U

#define LV_TASK_SPI_TX_TIMEOUT_MS 100U
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
static bsp_st7789_driver_t         s_lv_driver;
static st7789_spi_interface_t      s_lv_spi;
static st7789_timebase_interface_t s_lv_timebase;
static st7789_os_interface_t       s_lv_os;
static const st7789_panel_config_t s_lv_panel = {
    .width    = LV_TASK_PANEL_WIDTH,
    .height   = LV_TASK_PANEL_HEIGHT,
    .x_offset = LV_TASK_PANEL_X_OFFSET,
    .y_offset = LV_TASK_PANEL_Y_OFFSET,
};
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
/* ---- SPI bus (HAL passthrough) ------------------------------------------ */
static st7789_status_t lv_spi_init(void)
{
    return ST7789_OK;
}

static st7789_status_t lv_spi_deinit(void)
{
    return ST7789_OK;
}

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

/* ---- Timebase / OS ------------------------------------------------------ */
static uint32_t lv_tb_get_tick_ms(void)
{
    return HAL_GetTick();
}

static void lv_tb_delay_ms(uint32_t ms)
{
    osal_task_delay(ms);
}

static void lv_os_delay_ms(uint32_t ms)
{
    osal_task_delay(ms);
}

/* ---- Wire-up ------------------------------------------------------------ */
static st7789_status_t lv_driver_bind(void)
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

/* Animation demo widgets.  Spinner runs on its own internal animation, so the
 * screen redraws constantly without any extra plumbing.  Bar + counter refresh
 * every 500 ms from a dedicated lv_timer. */
static lv_obj_t *s_demo_bar;
static lv_obj_t *s_demo_counter;
static lv_obj_t *s_demo_ball;
static uint32_t  s_demo_tick;

static void lv_demo_tick_cb(lv_timer_t *t)
{
    (void)t;
    s_demo_tick++;

    const int32_t v = (int32_t)((s_demo_tick * 7U) % 101U);
    lv_bar_set_value(s_demo_bar, v, LV_ANIM_ON);

    lv_label_set_text_fmt(s_demo_counter, "frame %lu", (unsigned long)s_demo_tick);
}

static void lv_demo_ball_anim_cb(void *obj, int32_t v)
{
    lv_obj_set_x((lv_obj_t *)obj, v);
}

static void lv_demo_build_ui(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x101820), LV_PART_MAIN);

    /* Title */
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "ST7789 + LVGL");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 6);

    /* Animated loading spinner */
    lv_obj_t *spinner = lv_spinner_create(scr, 1500, 60);
    lv_obj_set_size(spinner, 80, 80);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0x1E90FF), LV_PART_INDICATOR);

    /* Progress bar cycling 0..100 via lv_timer */
    s_demo_bar = lv_bar_create(scr);
    lv_obj_set_size(s_demo_bar, 200, 12);
    lv_obj_align(s_demo_bar, LV_ALIGN_CENTER, 0, 44);
    lv_bar_set_range(s_demo_bar, 0, 100);
    lv_obj_set_style_bg_color(s_demo_bar, lv_color_hex(0xFFA500), LV_PART_INDICATOR);

    /* Frame counter label */
    s_demo_counter = lv_label_create(scr);
    lv_label_set_text(s_demo_counter, "frame 0");
    lv_obj_set_style_text_color(s_demo_counter, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
    lv_obj_align(s_demo_counter, LV_ALIGN_CENTER, 0, 68);

    /* Bouncing ball driven by lv_anim */
    s_demo_ball = lv_obj_create(scr);
    lv_obj_set_size(s_demo_ball, 20, 20);
    lv_obj_set_style_radius(s_demo_ball, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_demo_ball, lv_color_hex(0xFF3355), LV_PART_MAIN);
    lv_obj_set_style_border_width(s_demo_ball, 0, LV_PART_MAIN);
    lv_obj_align(s_demo_ball, LV_ALIGN_BOTTOM_LEFT, 0, -6);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, s_demo_ball);
    lv_anim_set_exec_cb(&a, lv_demo_ball_anim_cb);
    lv_anim_set_values(&a, 0, LV_TASK_PANEL_WIDTH - 20);
    lv_anim_set_time(&a, 1200);
    lv_anim_set_playback_time(&a, 1200);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);

    /* Drive bar+counter */
    lv_timer_create(lv_demo_tick_cb, 50, NULL);
}

/* ---- Task entry --------------------------------------------------------- */
void lvgl_display_task(void *argument)
{
    (void)argument;

    osal_task_delay(LV_TASK_BOOT_WAIT_MS);

    DEBUG_OUT(i, ST7789_LOG_TAG, "lvgl_display_task start");

    st7789_status_t ret = lv_driver_bind();
    if (ST7789_OK != ret)
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG, "lvgl driver bind failed = %d",
                  (int)ret);
        for (;;)
        {
            osal_task_delay(1000U);
        }
    }

    ret = s_lv_driver.pf_st7789_init(&s_lv_driver);
    if (ST7789_OK != ret)
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG, "lvgl st7789_init failed = %d",
                  (int)ret);
        for (;;)
        {
            osal_task_delay(1000U);
        }
    }

    (void)s_lv_driver.pf_st7789_fill_color(&s_lv_driver, BLACK);

    lv_init();

    if (!lv_port_disp_init(&s_lv_driver))
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG, "lvgl disp port init failed");
        for (;;)
        {
            osal_task_delay(1000U);
        }
    }

    lv_demo_build_ui();

    DEBUG_OUT(i, ST7789_LOG_TAG, "lvgl_display_task running");

    for (;;)
    {
        lv_timer_handler();
        osal_task_delay(LV_TASK_TIMER_PERIOD_MS);
    }
}
//******************************* Functions *********************************//

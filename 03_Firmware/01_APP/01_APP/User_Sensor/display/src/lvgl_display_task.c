/******************************************************************************
 * @file lvgl_display_task.c
 *
 * @par dependencies
 * - bsp_wrapper_display.h
 * - bsp_wrapper_touch.h
 * - lvgl.h
 * - lv_port_disp.h
 * - lv_port_indev.h
 * - gui_guider.h
 *
 * @author Ethan-Hang
 *
 * @brief LVGL bring-up task.  Drives the display + touch entirely through
 *        the platform wrapper APIs (no direct driver coupling), wires them
 *        into LVGL's display + pointer indev, then hands control to the
 *        gui_guider-generated UI.
 *
 * Processing flow:
 *   display_drv_init -> display_fill_color(BLACK) ->
 *   touch_drv_init  -> touch_get_chip_id (probe) ->
 *   lv_init -> lv_port_disp_init -> lv_port_indev_init ->
 *   setup_ui(&guider_ui) ->
 *   loop { lv_timer_handler; delay 5 ms }
 *
 * @note  Driver instances and HAL passthroughs live in the BSP integration
 *        + adapter layer (drv_adapter_display_register /
 *        drv_adapter_touch_register, called once from platform_io_register).
 *        The task only sees the abstract wrapper APIs.
 *
 * @version V1.0 2026-04-25
 * @version V2.0 2026-04-26
 * @upgrade 2.0: Removed the local ST7789 / CST816T driver bind code; the
 *               task now drives both peripherals through bsp_wrapper_display
 *               and bsp_wrapper_touch.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "user_task_reso_config.h"
#include "bsp_wrapper_display.h"
#include "bsp_wrapper_touch.h"
#include "Debug.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "gui_guider.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define LV_TASK_BOOT_WAIT_MS      2000U
#define LV_TASK_TIMER_PERIOD_MS   5U

/* RGB565 black, used to clear the panel before LVGL takes over. */
#define LV_TASK_BG_COLOR_BLACK    0x0000U
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
/**
 * gui_guider.h declares `extern lv_ui guider_ui` but the generated tree
 * does not provide a definition; supply it here so setup_ui() and the
 * generated screen handlers link.  Keeping it file-private to this task
 * gives one owner.
 **/
lv_ui guider_ui;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
/**
 * @brief      LVGL + gui_guider task entry: brings up display and touch
 *             through the platform wrapper APIs, then hands control to
 *             the gui_guider-generated UI.
 *
 * @param[in]  argument : Unused.
 *
 * @return     None.
 * */
void lvgl_display_task(void *argument)
{
    (void)argument;

    /**
     * Boot wait gives lower-priority init paths (e.g. logging, queues) time
     * to come up before we start hammering the display.
     **/
    osal_task_delay(LV_TASK_BOOT_WAIT_MS);

    DEBUG_OUT(i, ST7789_LOG_TAG, "lvgl_display_task start (gui_guider)");

    /* 1. Display bring-up via wrapper. */
    display_drv_init();
    wp_display_status_t dret = display_fill_color(LV_TASK_BG_COLOR_BLACK);
    if (WP_DISPLAY_OK != dret)
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG,
                  "display_fill_color failed = %d", (int)dret);
        for (;;)
        {
            osal_task_delay(1000U);
        }
    }

    /* 2. Touch bring-up via wrapper.  Display still works without touch,
     *    so a probe failure is logged but does not block the task. */
    touch_drv_init();
    uint8_t chip_id = 0u;
    wp_touch_status_t tret = touch_get_chip_id(&chip_id);
    bool touch_ok = (WP_TOUCH_OK == tret);
    if (touch_ok)
    {
        DEBUG_OUT(i, CST816T_LOG_TAG, "touch chip_id = 0x%02X",
                  (unsigned)chip_id);
    }
    else
    {
        DEBUG_OUT(e, CST816T_ERR_LOG_TAG,
                  "touch chip_id probe failed = %d", (int)tret);
    }

    /* 3. LVGL core + display port. */
    lv_init();
    if (!lv_port_disp_init())
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG, "lvgl disp port init failed");
        for (;;)
        {
            osal_task_delay(1000U);
        }
    }

    /* 4. LVGL pointer indev (skip if touch probe failed). */
    if (touch_ok)
    {
        if (!lv_port_indev_init())
        {
            DEBUG_OUT(e, CST816T_ERR_LOG_TAG, "lvgl indev port init failed");
        }
    }

    /* 5. Hand off to gui_guider's generated UI. */
    setup_ui(&guider_ui);
    DEBUG_OUT(i, ST7789_LOG_TAG, "lvgl_display_task: gui_guider UI loaded");

    /* 6. LVGL service loop. */
    for (;;)
    {
        lv_timer_handler();
        osal_task_delay(LV_TASK_TIMER_PERIOD_MS);
    }
}
//******************************* Functions *********************************//

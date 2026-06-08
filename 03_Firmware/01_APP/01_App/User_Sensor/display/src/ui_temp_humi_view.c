/******************************************************************************
 * @file ui_temp_humi_view.c
 *
 * @par dependencies
 * - ui_temp_humi_view.h
 * - bsp_wrapper_temp_humi.h
 * - lvgl.h
 * - gui_guider.h
 * - Debug.h
 *
 * @author Ethan-Hang
 *
 * @brief  Temperature and humidity -> LVGL label binding.  An lv_timer (500 ms) polls
 *         temp_humi_read_temp_sync() and updates guider_ui.under_up_label_1.
 *
 *
 * @version V1.0 2026-06-08
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "platform_type.h"
#include <stddef.h>

#include "bsp_wrapper_temp_humi.h"

#include "ui_temp_humi_view.h"
#include "lvgl.h"
#include "gui_guider.h"
#include "Debug.h"


//******************************** Includes *********************************//

//******************************** Defines **********************************//
/** Display refresh cadence.  Algorithm runs at 40 Hz; 2 Hz on screen is plenty. */
#define UI_TEMP_HUMI_VIEW_PERIOD_MS     500U


#define UI_TEMP_HUMI_VIEW_MIN       0.0f
#define UI_TEMP_HUMI_VIEW_MAX       100.0f
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
/** Single UI handle captured at registration (gui_guider is a singleton). */
static lv_ui *s_ui = NULL;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//

/**
 * @brief      lv_timer callback: refresh the temperature label if the under_up
 *             screen is currently active.
 *
 * @param[in]  timer : Unused.
 */
static void ui_temp_humi_view_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    if ((NULL == s_ui) || (NULL == s_ui->under_up_label_2))
    {
        return;
    }

    /* Pointer-value compare only (no deref of a possibly-stale under_up):
     * a destroyed screen can never be the active screen, so equality here
     * guarantees both under_up and under_up_label_2 are live. */
    if (lv_scr_act() != s_ui->under_up)
    {
        return;
    }

    FLOAT32_T Temp = 0.0f;

    temp_humi_read_temp_sync(&Temp, UI_TEMP_HUMI_VIEW_PERIOD_MS);
    if ((Temp >= UI_TEMP_HUMI_VIEW_MIN) && (Temp <= UI_TEMP_HUMI_VIEW_MAX))
    {
        lv_label_set_text_fmt(s_ui->under_up_label_2, "温度%u.%01u", (UINT8_T)Temp, (UINT8_T)(Temp * 10) % 10);
    }
    else
    {
        lv_label_set_text(s_ui->under_up_label_2, "温度--");
    }

}

void ui_temp_humi_view_register(lv_ui *ui)
{
    if (NULL == ui)
    {
        DEBUG_OUT(e, LVGL_LOG_TAG, "ui_temp_humi_view_register: ui is NULL");
        return;
    }

    /* Handle intentionally not retained: the view is always-on, so the timer
     * lives for the process lifetime and is never deleted. */
    lv_timer_t *t = lv_timer_create(ui_temp_humi_view_timer_cb,
                                    UI_TEMP_HUMI_VIEW_PERIOD_MS, NULL);
    if (NULL == t)
    {
        DEBUG_OUT(e, LVGL_LOG_TAG, "ui_temp_humi_view_register: lv_timer_create failed");
        return;
    }

    /* Publish the handle only after the timer is live, so a populated s_ui
     * always implies an armed callback. */
    s_ui = ui;

    DEBUG_OUT(i, LVGL_LOG_TAG, "ui_temp_humi_view registered (%u ms)",
              (unsigned)UI_TEMP_HUMI_VIEW_PERIOD_MS);
}

//******************************* Functions *********************************//

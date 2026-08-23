/******************************************************************************
 * @file ui_temp_humi_view.c
 *
 * @par dependencies
 * - ui_temp_humi_view.h
 * - bsp_wrapper_temp_humi.h
 * - lvgl.h
 * - Debug.h
 *
 * @author Ethan-Hang
 *
 * @brief  Temperature and humidity -> LVGL label binding.  An lv_timer
 *         (500 ms) polls temp_humi_read_temp_sync() and updates the
 *         under_up screen's body-temperature label.
 *
 *         The label is bound explicitly by setup_scr_under_up() and dropped
 *         again from LV_EVENT_DELETE; it is never derived from lv_scr_act().
 *         See ui_hr_view.h for why that gate was unsound -- gui_guider's
 *         ui_load_scr_animation() frees the outgoing screen's children while
 *         that screen is still the active one.
 *
 * @version V1.0 2026-06-08
 * @version V2.0 2026-08-15  Explicit bind/auto-unbind instead of caching an
 *                           lv_ui child pointer behind an lv_scr_act() gate.
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
#include "Debug.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/** Display refresh cadence.  Algorithm runs at 40 Hz; 2 Hz on screen is plenty. */
#define UI_TEMP_HUMI_VIEW_PERIOD_MS     500U


#define UI_TEMP_HUMI_VIEW_MIN       0.0f
#define UI_TEMP_HUMI_VIEW_MAX       100.0f
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
/** Currently live temperature label; NULL when under_up is not built. */
static lv_obj_t *s_label = NULL;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//

/**
 * @brief      LV_EVENT_DELETE callback: drop the pointer LVGL is about to
 *             free.
 *
 * @param[in]  e : Event.
 *
 * @return     None.
 */
static void ui_temp_humi_view_label_deleted_cb(lv_event_t *e)
{
    if (s_label == lv_event_get_target(e))
    {
        s_label = NULL;
    }
}

/**
 * @brief      lv_timer callback: refresh the temperature label while it is
 *             bound.
 *
 * @param[in]  timer : Unused.
 *
 * @return     None.
 */
static void ui_temp_humi_view_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    if (NULL == s_label)
    {
        return;
    }

    FLOAT32_T Temp = 0.0f;

    temp_humi_read_temp_sync(&Temp, UI_TEMP_HUMI_VIEW_PERIOD_MS);
    if ((Temp >= UI_TEMP_HUMI_VIEW_MIN) && (Temp <= UI_TEMP_HUMI_VIEW_MAX))
    {
        /* "体温" (not "温度"): the GUI-Guider font alimama_10 only carries
         * glyphs used by the project labels, and the new UI says 体温. */
        lv_label_set_text_fmt(s_label, "体温%u.%01u", (UINT8_T)Temp, (UINT8_T)(Temp * 10) % 10);
    }
    else
    {
        lv_label_set_text(s_label, "体温--");
    }

}

void ui_temp_humi_view_bind(lv_obj_t *p_label)
{
    s_label = p_label;

    if (NULL != p_label)
    {
        lv_obj_add_event_cb(p_label, ui_temp_humi_view_label_deleted_cb,
                            LV_EVENT_DELETE, NULL);
    }
}

void ui_temp_humi_view_register(void)
{
    /* Handle intentionally not retained: the view is always-on, so the timer
     * lives for the process lifetime and is never deleted. */
    lv_timer_t *t = lv_timer_create(ui_temp_humi_view_timer_cb,
                                    UI_TEMP_HUMI_VIEW_PERIOD_MS, NULL);
    if (NULL == t)
    {
        DEBUG_OUT(e, LVGL_LOG_TAG, "ui_temp_humi_view_register: lv_timer_create failed");
        return;
    }

    DEBUG_OUT(i, LVGL_LOG_TAG, "ui_temp_humi_view registered (%u ms)",
              (unsigned)UI_TEMP_HUMI_VIEW_PERIOD_MS);
}

//******************************* Functions *********************************//

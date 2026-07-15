/******************************************************************************
 * @file ui_hr_view.c
 *
 * @par dependencies
 * - ui_hr_view.h
 * - em7028_heart_rate_task.h
 * - lvgl.h
 * - gui_guider.h
 * - Debug.h
 *
 * @author Ethan-Hang
 *
 * @brief  Heart-rate -> LVGL label binding.  An lv_timer (500 ms) polls
 *         em7028_hr_get_latest() and writes the BPM onto whichever
 *         heart-rate display is currently on screen:
 *
 *           - under_up_label_1 ("心率%u/分") on the pull-up status screen
 *           - Heart_label_2 ("%u/min") on the dedicated Heart screen
 *
 *         Screens are created/destroyed on navigation (gui_guider
 *         auto_del); their label pointers are only valid while their
 *         screen is active.  The callback therefore gates on lv_scr_act()
 *         before touching either label.  The setup_scr_* defaults
 *         ("心率64/分" / "72/min") are overwritten within one period.
 *
 * @version V1.0 2026-06-08
 * @version V1.1 2026-06-11  Also drive Heart_label_2 on the Heart screen.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "platform_type.h"
#include <stddef.h>

#include "ui_hr_view.h"
#include "em7028_heart_rate_task.h"
#include "lvgl.h"
#include "gui_guider.h"
#include "Debug.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/** Display refresh cadence.  Algorithm runs at 40 Hz; 2 Hz on screen is plenty. */
#define UI_HR_VIEW_PERIOD_MS     100U

/** Minimum confidence (0-100) to treat a reading as valid. */
#define UI_HR_VIEW_MIN_CONF      30U

/** Plausible BPM window (matches hr_algo IBI 250-1500 ms bounds). */
#define UI_HR_VIEW_BPM_MIN       40U
#define UI_HR_VIEW_BPM_MAX       240U
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
/** Single UI handle captured at registration (gui_guider is a singleton). */
static lv_ui *s_ui = NULL;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//

/**
 * @brief      lv_timer callback: refresh the heart-rate label of whichever
 *             heart-rate-bearing screen (under_up / Heart) is active.
 *
 * @param[in]  timer : Unused.
 */
static void ui_hr_view_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    if (NULL == s_ui)
    {
        return;
    }

    /* Pointer-value compare only (no deref of a possibly-stale screen):
     * a destroyed screen can never be the active screen, so equality here
     * guarantees the screen and its labels are live. */
    lv_obj_t *act          = lv_scr_act();
    BOOL_T    on_under_up  = (act == s_ui->under_up) &&
                             (NULL != s_ui->under_up_label_1);
    BOOL_T    on_heart_scr = (act == s_ui->Heart) &&
                             (NULL != s_ui->Heart_label_2);

    if (!on_under_up && !on_heart_scr)
    {
        return;
    }

    UINT16_T bpm  = 0U;
    UINT8_T  conf = 0U;
    BOOL_T   have = em7028_hr_get_latest(&bpm, &conf);
    BOOL_T   valid = have &&
                     (conf >= UI_HR_VIEW_MIN_CONF) &&
                     (bpm  >= UI_HR_VIEW_BPM_MIN)  &&
                     (bpm  <= UI_HR_VIEW_BPM_MAX);

    if (on_under_up)
    {
        if (valid)
        {
            lv_label_set_text_fmt(s_ui->under_up_label_1, "心率%u/分",
                                  (UINT8_T)bpm);
        }
        else
        {
            lv_label_set_text(s_ui->under_up_label_1, "心率--/分");
        }
    }

    if (on_heart_scr)
    {
        /* ASCII-only format: Heart_label_2 uses alimama_16, whose packed
         * glyph set is driven by the GUI-Guider label texts ("72/min"). */
        if (valid)
        {
            lv_label_set_text_fmt(s_ui->Heart_label_2, "%u/min",
                                  (UINT8_T)bpm);
        }
        else
        {
            lv_label_set_text(s_ui->Heart_label_2, "--/min");
        }
    }
}

void ui_hr_view_register(lv_ui *ui)
{
    if (NULL == ui)
    {
        DEBUG_OUT(e, LVGL_LOG_TAG, "ui_hr_view_register: ui is NULL");
        return;
    }

    /* Handle intentionally not retained: the view is always-on, so the timer
     * lives for the process lifetime and is never deleted. */
    lv_timer_t *t = lv_timer_create(ui_hr_view_timer_cb,
                                    UI_HR_VIEW_PERIOD_MS, NULL);
    if (NULL == t)
    {
        DEBUG_OUT(e, LVGL_LOG_TAG, "ui_hr_view_register: lv_timer_create failed");
        return;
    }

    /* Publish the handle only after the timer is live, so a populated s_ui
     * always implies an armed callback. */
    s_ui = ui;

    DEBUG_OUT(i, LVGL_LOG_TAG, "ui_hr_view registered (%u ms)",
              (unsigned)UI_HR_VIEW_PERIOD_MS);
}

//******************************* Functions *********************************//

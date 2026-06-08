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
 *         em7028_hr_get_latest() and writes guider_ui.under_up_label_1.
 *
 *         The under_up screen is created/destroyed on navigation (gui_guider
 *         auto_del); its label pointer is only valid while it is the active
 *         screen.  The callback therefore gates on lv_scr_act() == under_up
 *         before touching the label.  setup_scr_under_up() resets the text to
 *         "心率64/分" on entry; this timer overwrites it within one period.
 *
 * @version V1.0 2026-06-08
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
#define UI_HR_VIEW_PERIOD_MS     500U

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
 * @brief      lv_timer callback: refresh the heart-rate label if the under_up
 *             screen is currently active.
 *
 * @param[in]  timer : Unused.
 */
static void ui_hr_view_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    if ((NULL == s_ui) || (NULL == s_ui->under_up_label_1))
    {
        return;
    }

    /* Pointer-value compare only (no deref of a possibly-stale under_up):
     * a destroyed screen can never be the active screen, so equality here
     * guarantees both under_up and under_up_label_1 are live. */
    if (lv_scr_act() != s_ui->under_up)
    {
        return;
    }

    UINT16_T bpm  = 0U;
    UINT8_T  conf = 0U;
    BOOL_T   have = em7028_hr_get_latest(&bpm, &conf);

    if (have &&
        (conf >= UI_HR_VIEW_MIN_CONF) &&
        (bpm  >= UI_HR_VIEW_BPM_MIN)  &&
        (bpm  <= UI_HR_VIEW_BPM_MAX))
    {
        lv_label_set_text_fmt(s_ui->under_up_label_1, "心率%u/分", (unsigned)bpm);
    }
    else
    {
        lv_label_set_text(s_ui->under_up_label_1, "心率--/分");
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

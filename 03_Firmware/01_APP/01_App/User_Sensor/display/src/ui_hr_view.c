/******************************************************************************
 * @file ui_hr_view.c
 *
 * @par dependencies
 * - ui_hr_view.h
 * - em7028_heart_rate_task.h
 * - lvgl.h
 * - Debug.h
 *
 * @author Ethan-Hang
 *
 * @brief  Heart-rate -> LVGL label binding.  An lv_timer (100 ms) polls
 *         em7028_hr_get_latest() and writes the BPM onto every label that is
 *         currently bound:
 *
 *           - under_up_label_1 ("心率%u/分") on the pull-up status screen
 *           - Heart_label_2 ("%u/min") on the dedicated Heart screen
 *
 *         Screens are created/destroyed on navigation (gui_guider auto_del),
 *         so a label pointer is only valid while its screen lives.  Ownership
 *         of that fact sits here: setup_scr_*() binds the label as it creates
 *         it, and the LV_EVENT_DELETE callback below clears the slot the
 *         instant LVGL frees it.  Nothing is ever inferred from lv_scr_act()
 *         -- see the header for why that gate was unsound.
 *
 * Processing flow:
 *   setup_scr_Heart()    -> ui_hr_view_bind_heart(label)    -> slot armed
 *   lv_obj_clean(screen) -> LV_EVENT_DELETE                 -> slot cleared
 *   lv_timer_handler()   -> ui_hr_view_timer_cb()           -> armed slots only
 *
 * @version V1.0 2026-06-08
 * @version V1.1 2026-06-11  Also drive Heart_label_2 on the Heart screen.
 * @version V2.0 2026-08-15  Explicit bind/auto-unbind instead of caching
 *                           lv_ui child pointers behind an lv_scr_act() gate.
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
#include "Debug.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/** Display refresh cadence.  Algorithm runs at 40 Hz; 10 Hz on screen is plenty. */
#define UI_HR_VIEW_PERIOD_MS     100U

/** Minimum confidence (0-100) to treat a reading as valid. */
#define UI_HR_VIEW_MIN_CONF      30U

/** Plausible BPM window (matches hr_algo IBI 250-1500 ms bounds). */
#define UI_HR_VIEW_BPM_MIN       40U
#define UI_HR_VIEW_BPM_MAX       240U

/**
 * @brief Heart-rate label slots.  One per screen that shows a BPM reading.
 *        Internal: screens reach them through the two named bind hooks, so
 *        the enum never has to travel to lvgl_ui.
 */
typedef enum
{
    UI_HR_VIEW_SLOT_UNDER_UP = 0,   /**< under_up_label_1, "心率%u/分". */
    UI_HR_VIEW_SLOT_HEART,          /**< Heart_label_2,    "%u/min".   */
    UI_HR_VIEW_SLOT_CNT
} ui_hr_view_slot_t;
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
/** Currently live label per slot; NULL when that screen is not built. */
static lv_obj_t *s_label[UI_HR_VIEW_SLOT_CNT];

/**
 * Per-slot text.  under_up uses the CJK font packed for that screen; Heart
 * uses alimama_16, whose packed glyph set is driven by the GUI-Guider label
 * texts ("72/min"), hence ASCII only there.
 */
static const char *const s_fmt_valid[UI_HR_VIEW_SLOT_CNT] =
{
    [UI_HR_VIEW_SLOT_UNDER_UP] = "心率%u/分",
    [UI_HR_VIEW_SLOT_HEART]    = "%u/min",
};

static const char *const s_text_idle[UI_HR_VIEW_SLOT_CNT] =
{
    [UI_HR_VIEW_SLOT_UNDER_UP] = "心率--/分",
    [UI_HR_VIEW_SLOT_HEART]    = "--/min",
};
//******************************* Declaring *********************************//

//******************************* Functions *********************************//

/**
 * @brief      LV_EVENT_DELETE callback: drop the pointer LVGL is about to
 *             free.
 *
 * @param[in]  e : Event; user data carries the slot index.
 *
 * @return     None.
 *
 * @note       Guarded against a stale label: only the pointer we actually
 *             hold may clear the slot, so a late delete for a replaced label
 *             cannot unbind its successor.
 */
static void ui_hr_view_label_deleted_cb(lv_event_t *e)
{
    SIZE_T    slot   = (SIZE_T)lv_event_get_user_data(e);
    lv_obj_t *target = lv_event_get_target(e);

    if ((slot < (SIZE_T)UI_HR_VIEW_SLOT_CNT) && (s_label[slot] == target))
    {
        s_label[slot] = NULL;
    }
}

/**
 * @brief      Arm one slot with a freshly created label.
 *
 * @param[in]  slot    : Slot to arm.
 * @param[in]  p_label : Label object; NULL clears the slot.
 *
 * @return     None.
 */
static void ui_hr_view_bind(ui_hr_view_slot_t slot, lv_obj_t *p_label)
{
    s_label[slot] = p_label;

    if (NULL != p_label)
    {
        lv_obj_add_event_cb(p_label, ui_hr_view_label_deleted_cb,
                            LV_EVENT_DELETE, (void *)(SIZE_T)slot);
    }
}

/**
 * @brief      lv_timer callback: refresh every bound heart-rate label.
 *
 * @param[in]  timer : Unused.
 *
 * @return     None.
 */
static void ui_hr_view_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    UINT16_T bpm  = 0U;
    UINT8_T  conf = 0U;
    BOOL_T   have = em7028_hr_get_latest(&bpm, &conf);
    BOOL_T   valid = have &&
                     (conf >= UI_HR_VIEW_MIN_CONF) &&
                     (bpm  >= UI_HR_VIEW_BPM_MIN)  &&
                     (bpm  <= UI_HR_VIEW_BPM_MAX);

    for (SIZE_T slot = 0U; slot < (SIZE_T)UI_HR_VIEW_SLOT_CNT; slot++)
    {
        if (NULL == s_label[slot])
        {
            continue;
        }

        if (valid)
        {
            lv_label_set_text_fmt(s_label[slot], s_fmt_valid[slot],
                                  (UINT8_T)bpm);
        }
        else
        {
            lv_label_set_text(s_label[slot], s_text_idle[slot]);
        }
    }
}

void ui_hr_view_bind_heart(lv_obj_t *p_label)
{
    ui_hr_view_bind(UI_HR_VIEW_SLOT_HEART, p_label);
}

void ui_hr_view_bind_under_up(lv_obj_t *p_label)
{
    ui_hr_view_bind(UI_HR_VIEW_SLOT_UNDER_UP, p_label);
}

void ui_hr_view_register(void)
{
    /* Handle intentionally not retained: the view is always-on, so the timer
     * lives for the process lifetime and is never deleted. */
    lv_timer_t *t = lv_timer_create(ui_hr_view_timer_cb,
                                    UI_HR_VIEW_PERIOD_MS, NULL);
    if (NULL == t)
    {
        DEBUG_OUT(e, LVGL_LOG_TAG, "ui_hr_view_register: lv_timer_create failed");
        return;
    }

    DEBUG_OUT(i, LVGL_LOG_TAG, "ui_hr_view registered (%u ms)",
              (unsigned)UI_HR_VIEW_PERIOD_MS);
}

//******************************* Functions *********************************//

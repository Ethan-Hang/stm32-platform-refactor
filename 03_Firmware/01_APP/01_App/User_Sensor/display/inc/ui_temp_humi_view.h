/******************************************************************************
 * @file ui_temp_humi_view.h
 *
 * @author Ethan-Hang
 *
 * @brief  Temperature and humidity -> LVGL label binding.  Owns an lv_timer that polls the
 *         sensor accessor and updates the under_up screen's temperature and humidity
 *         labels, entirely within the LVGL thread.
 *
 *         The label itself is bound by setup_scr_under_up() through
 *         ui_temp_humi_view_bind() (declared in custom.h, alongside the
 *         heart-rate hooks) and dropped again on LV_EVENT_DELETE.  See
 *         ui_hr_view.h for the lifetime rationale.
 *
 * @version V1.0 2026-06-08
 * @version V2.0 2026-08-15  Explicit bind/auto-unbind instead of caching an
 *                           lv_ui child pointer behind an lv_scr_act() gate.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/
#ifndef __UI_TEMP_HUMI_VIEW_H__
#define __UI_TEMP_HUMI_VIEW_H__

//******************************** Includes *********************************//
#include "custom.h"     /* ui_temp_humi_view_bind */
//******************************** Includes *********************************//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      Register the temperature and humidity display lv_timer.
 *
 *             Must be called exactly once, from inside the LVGL thread, after
 *             setup_ui().  The timer callback runs in lv_timer_handler()
 *             context, so all widget access is single-threaded / safe.
 *
 *             Independent of the label binding: the timer is a no-op while
 *             the under_up screen is not built.
 *
 * @return     None.
 */
void ui_temp_humi_view_register(void);

#ifdef __cplusplus
}
#endif

#endif /* __UI_TEMP_HUMI_VIEW_H__ */

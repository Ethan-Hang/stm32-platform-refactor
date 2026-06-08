/******************************************************************************
 * @file ui_hr_view.h
 *
 * @author Ethan-Hang
 *
 * @brief  Heart-rate -> LVGL label binding.  Owns an lv_timer that polls the
 *         EM7028 heart-rate accessor and updates the under_up screen's BPM
 *         label, entirely within the LVGL thread.
 *
 * @version V1.0 2026-06-08
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/
#ifndef __UI_HR_VIEW_H__
#define __UI_HR_VIEW_H__

//******************************** Includes *********************************//
#include "gui_guider.h"
//******************************** Includes *********************************//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      Register the heart-rate display lv_timer.
 *
 *             Must be called exactly once, from inside the LVGL thread, after
 *             setup_ui().  The timer callback runs in lv_timer_handler()
 *             context, so all widget access is single-threaded / safe.
 *
 * @param[in]  ui : gui_guider UI handle (guider_ui).
 */
void ui_hr_view_register(lv_ui *ui);

#ifdef __cplusplus
}
#endif

#endif /* __UI_HR_VIEW_H__ */

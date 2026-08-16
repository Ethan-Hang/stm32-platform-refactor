/******************************************************************************
 * @file ui_hr_view.h
 *
 * @author Ethan-Hang
 *
 * @brief  Heart-rate -> LVGL label binding.  Owns an lv_timer that polls the
 *         EM7028 heart-rate accessor and updates every heart-rate label that
 *         is currently bound, entirely within the LVGL thread.
 *
 *         Binding is explicit and lifetime-safe: a screen hands its label to
 *         ui_hr_view_bind_heart() / ui_hr_view_bind_under_up() while building
 *         it, and the view drops the pointer from LV_EVENT_DELETE.  Those two
 *         hooks are declared in custom.h (the header the generated screens
 *         already include) so lvgl_ui keeps no compile-time dependency on
 *         01_App; this file pulls them in from there rather than restating
 *         them.
 *
 *         The bindings must NOT be replaced by a lookup off lv_scr_act():
 *         gui_guider's ui_load_scr_animation() calls
 *         lv_obj_clean(lv_scr_act()) and only then starts the load animation,
 *         so between the clean and the animation's start callback the screen
 *         is still the active one while all of its children are already freed
 *         (200 ms wide on the "< Menu" buttons, which pass delay = 200).
 *
 * @version V1.0 2026-06-08
 * @version V1.1 2026-06-11  Also drive Heart_label_2 on the Heart screen.
 * @version V2.0 2026-08-15  Explicit bind/auto-unbind instead of caching
 *                           lv_ui child pointers behind an lv_scr_act() gate
 *                           (use-after-free on Heart -> menu).
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/
#ifndef __UI_HR_VIEW_H__
#define __UI_HR_VIEW_H__

//******************************** Includes *********************************//
#include "custom.h"     /* ui_hr_view_bind_heart / _under_up */
//******************************** Includes *********************************//

#ifdef __cplusplus
extern "C" {
#endif

//******************************* Functions *********************************//
/**
 * @brief      Register the heart-rate display lv_timer.
 *
 *             Must be called exactly once, from inside the LVGL thread, after
 *             setup_ui().  The timer callback runs in lv_timer_handler()
 *             context, so all widget access is single-threaded / safe.
 *
 *             Independent of the label bindings: the timer simply skips any
 *             slot whose screen is not currently built.
 *
 * @return     None.
 */
void ui_hr_view_register(void);
//******************************* Functions *********************************//

#ifdef __cplusplus
}
#endif

#endif /* __UI_HR_VIEW_H__ */

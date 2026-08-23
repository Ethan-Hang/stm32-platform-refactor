/*
* Copyright 2023 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef __CUSTOM_H_
#define __CUSTOM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

void custom_init(lv_ui *ui);

/* NOTE (project-owned change): heart-rate label binding hooks, implemented by
   01_App/User_Sensor/display/src/ui_hr_view.c.  Declared here -- the header
   the generated screens already include -- so lvgl_ui gains no compile-time
   dependency on 01_App; ui_hr_view.h includes this file, so there is still
   exactly one declaration of each.

   A screen calls its hook from the "custom code" slot of setup_scr_*(), where
   the label is provably live; the view drops the pointer again from
   LV_EVENT_DELETE.  Passing NULL clears the binding.  Do NOT replace this with
   a lookup off lv_scr_act(): ui_load_scr_animation() calls
   lv_obj_clean(lv_scr_act()) before starting the load animation, so the
   outgoing screen stays "active" for the whole animation delay with all of its
   children already freed. */
void ui_hr_view_bind_heart(lv_obj_t *p_label);
void ui_hr_view_bind_under_up(lv_obj_t *p_label);

/* Same contract, for the body-temperature label; implemented by
   01_App/User_Sensor/display/src/ui_temp_humi_view.c. */
void ui_temp_humi_view_bind(lv_obj_t *p_label);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

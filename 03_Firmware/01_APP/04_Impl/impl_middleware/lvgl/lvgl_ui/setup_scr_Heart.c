/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"



void setup_scr_Heart(lv_ui *ui)
{
    //Write codes Heart
    ui->Heart = lv_obj_create(NULL);
    lv_obj_set_size(ui->Heart, 240, 280);
    lv_obj_set_scrollbar_mode(ui->Heart, LV_SCROLLBAR_MODE_OFF);

    //Write style for Heart, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Heart, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->Heart, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->Heart, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Heart_btn_1
    ui->Heart_btn_1 = lv_btn_create(ui->Heart);
    ui->Heart_btn_1_label = lv_label_create(ui->Heart_btn_1);
    lv_label_set_text(ui->Heart_btn_1_label, "< Menu");
    lv_label_set_long_mode(ui->Heart_btn_1_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->Heart_btn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->Heart_btn_1, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->Heart_btn_1_label, LV_PCT(100));
    lv_obj_set_pos(ui->Heart_btn_1, 11, 9);
    lv_obj_set_size(ui->Heart_btn_1, 63, 21);

    //Write style for Heart_btn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Heart_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->Heart_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Heart_btn_1, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Heart_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Heart_btn_1, lv_color_hex(0xff0050), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Heart_btn_1, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->Heart_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->Heart_btn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Heart_cont_1
    ui->Heart_cont_1 = lv_obj_create(ui->Heart);
    lv_obj_set_pos(ui->Heart_cont_1, 0, 280);
    lv_obj_set_size(ui->Heart_cont_1, 240, 235);
    lv_obj_set_scrollbar_mode(ui->Heart_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for Heart_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->Heart_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Heart_cont_1, 40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->Heart_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->Heart_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->Heart_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->Heart_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->Heart_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Heart_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Heart_img_1
    ui->Heart_img_1 = lv_img_create(ui->Heart_cont_1);
    lv_obj_add_flag(ui->Heart_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->Heart_img_1, &_heart32x32_alpha_37x32_ext);
    lv_img_set_pivot(ui->Heart_img_1, 50,50);
    lv_img_set_angle(ui->Heart_img_1, 0);
    lv_obj_set_pos(ui->Heart_img_1, 96, 8);
    lv_obj_set_size(ui->Heart_img_1, 37, 32);

    //Write style for Heart_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->Heart_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->Heart_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Heart_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->Heart_img_1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Heart_label_1
    ui->Heart_label_1 = lv_label_create(ui->Heart_cont_1);
    lv_label_set_text(ui->Heart_label_1, "启动血氧检测");
    lv_label_set_long_mode(ui->Heart_label_1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->Heart_label_1, 70, 69);
    lv_obj_set_size(ui->Heart_label_1, 100, 17);

    //Write style for Heart_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->Heart_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Heart_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Heart_label_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Heart_label_1, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->Heart_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->Heart_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->Heart_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->Heart_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->Heart_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->Heart_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->Heart_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->Heart_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->Heart_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Heart_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Heart_slider_1
    ui->Heart_slider_1 = lv_slider_create(ui->Heart_cont_1);
    lv_slider_set_range(ui->Heart_slider_1, 0, 100);
    lv_slider_set_mode(ui->Heart_slider_1, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_value(ui->Heart_slider_1, 50, LV_ANIM_OFF);
    lv_obj_set_pos(ui->Heart_slider_1, 26, 64);
    lv_obj_set_size(ui->Heart_slider_1, 183, 23);

    //Write style for Heart_slider_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Heart_slider_1, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->Heart_slider_1, lv_color_hex(0xff0049), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->Heart_slider_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Heart_slider_1, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui->Heart_slider_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Heart_slider_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for Heart_slider_1, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Heart_slider_1, 255, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->Heart_slider_1, lv_color_hex(0xff0034), LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->Heart_slider_1, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Heart_slider_1, 8, LV_PART_INDICATOR|LV_STATE_DEFAULT);

    //Write style for Heart_slider_1, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Heart_slider_1, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->Heart_slider_1, lv_color_hex(0xfa0026), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->Heart_slider_1, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Heart_slider_1, 20, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes Heart_img_2
    ui->Heart_img_2 = lv_img_create(ui->Heart_cont_1);
    lv_obj_add_flag(ui->Heart_img_2, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->Heart_img_2, &_BIGHeart_alpha_93x85_ext);
    lv_img_set_pivot(ui->Heart_img_2, 50,50);
    lv_img_set_angle(ui->Heart_img_2, 0);
    lv_obj_set_pos(ui->Heart_img_2, 71, 122);
    lv_obj_set_size(ui->Heart_img_2, 93, 85);

    //Write style for Heart_img_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->Heart_img_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->Heart_img_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Heart_img_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->Heart_img_2, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Heart_label_2
    ui->Heart_label_2 = lv_label_create(ui->Heart_cont_1);
    lv_label_set_text(ui->Heart_label_2, "72/min");
    lv_label_set_long_mode(ui->Heart_label_2, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->Heart_label_2, 83, 153);
    lv_obj_set_size(ui->Heart_label_2, 70, 19);

    //Write style for Heart_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->Heart_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Heart_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Heart_label_2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Heart_label_2, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->Heart_label_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->Heart_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->Heart_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->Heart_label_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->Heart_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->Heart_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->Heart_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->Heart_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->Heart_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Heart_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of Heart.
    /* NOTE (project-owned change): hand the BPM label to ui_hr_view while it
       is provably live.  The view drops it again from LV_EVENT_DELETE, so it
       never touches a freed widget during a screen transition.  Re-apply
       after a GUI-Guider re-export. */
    ui_hr_view_bind_heart(ui->Heart_label_2);

    //Update current screen layout.
    lv_obj_update_layout(ui->Heart);

    //Init events for screen.
    events_init_Heart(ui);
}

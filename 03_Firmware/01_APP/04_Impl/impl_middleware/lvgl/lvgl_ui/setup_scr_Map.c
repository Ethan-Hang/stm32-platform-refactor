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



void setup_scr_Map(lv_ui *ui)
{
    //Write codes Map
    ui->Map = lv_obj_create(NULL);
    lv_obj_set_size(ui->Map, 240, 280);
    lv_obj_set_scrollbar_mode(ui->Map, LV_SCROLLBAR_MODE_OFF);

    //Write style for Map, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Map, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->Map, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->Map, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Map_btn_1
    ui->Map_btn_1 = lv_btn_create(ui->Map);
    ui->Map_btn_1_label = lv_label_create(ui->Map_btn_1);
    lv_label_set_text(ui->Map_btn_1_label, "< Menu");
    lv_label_set_long_mode(ui->Map_btn_1_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->Map_btn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->Map_btn_1, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->Map_btn_1_label, LV_PCT(100));
    lv_obj_set_pos(ui->Map_btn_1, 10, 9);
    lv_obj_set_size(ui->Map_btn_1, 63, 21);

    //Write style for Map_btn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Map_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->Map_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Map_btn_1, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Map_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Map_btn_1, lv_color_hex(0x2200ff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Map_btn_1, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->Map_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->Map_btn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Map_label_1
    ui->Map_label_1 = lv_label_create(ui->Map);
    lv_label_set_text(ui->Map_label_1, "电子围栏预警");
    lv_label_set_long_mode(ui->Map_label_1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->Map_label_1, 71, 59);
    lv_obj_set_size(ui->Map_label_1, 100, 15);

    //Write style for Map_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->Map_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Map_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Map_label_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Map_label_1, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->Map_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->Map_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->Map_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->Map_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->Map_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->Map_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->Map_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->Map_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->Map_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Map_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Map_img_1
    ui->Map_img_1 = lv_img_create(ui->Map);
    lv_obj_add_flag(ui->Map_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->Map_img_1, &_location32x32_alpha_32x32_ext);
    lv_img_set_pivot(ui->Map_img_1, 50,50);
    lv_img_set_angle(ui->Map_img_1, 0);
    lv_obj_set_pos(ui->Map_img_1, 104, 23);
    lv_obj_set_size(ui->Map_img_1, 32, 32);

    //Write style for Map_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->Map_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->Map_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Map_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->Map_img_1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Map_cont_1
    ui->Map_cont_1 = lv_obj_create(ui->Map);
    lv_obj_set_pos(ui->Map_cont_1, 24, 79);
    lv_obj_set_size(ui->Map_cont_1, 200, 200);
    lv_obj_set_scrollbar_mode(ui->Map_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for Map_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->Map_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Map_cont_1, 100, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->Map_cont_1, 116, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->Map_cont_1, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->Map_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->Map_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->Map_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->Map_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->Map_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Map_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Map_cont_2
    ui->Map_cont_2 = lv_obj_create(ui->Map_cont_1);
    lv_obj_set_pos(ui->Map_cont_2, 27, 88);
    lv_obj_set_size(ui->Map_cont_2, 50, 50);
    lv_obj_set_scrollbar_mode(ui->Map_cont_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for Map_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->Map_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Map_cont_2, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->Map_cont_2, 94, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->Map_cont_2, lv_color_hex(0xff9600), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->Map_cont_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->Map_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->Map_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->Map_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->Map_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Map_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Map_cont_3
    ui->Map_cont_3 = lv_obj_create(ui->Map_cont_1);
    lv_obj_set_pos(ui->Map_cont_3, 116, 33);
    lv_obj_set_size(ui->Map_cont_3, 50, 50);
    lv_obj_set_scrollbar_mode(ui->Map_cont_3, LV_SCROLLBAR_MODE_OFF);

    //Write style for Map_cont_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->Map_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Map_cont_3, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->Map_cont_3, 94, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->Map_cont_3, lv_color_hex(0xff0027), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->Map_cont_3, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->Map_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->Map_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->Map_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->Map_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Map_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Map_img_2
    ui->Map_img_2 = lv_img_create(ui->Map);
    lv_obj_add_flag(ui->Map_img_2, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->Map_img_2, &_location20x20_alpha_20x20_ext);
    lv_img_set_pivot(ui->Map_img_2, 50,50);
    lv_img_set_angle(ui->Map_img_2, 0);
    lv_obj_set_pos(ui->Map_img_2, 79, 237);
    lv_obj_set_size(ui->Map_img_2, 20, 20);

    //Write style for Map_img_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->Map_img_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->Map_img_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Map_img_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->Map_img_2, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of Map.


    //Update current screen layout.
    lv_obj_update_layout(ui->Map);

    //Init events for screen.
    events_init_Map(ui);
}

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



void setup_scr_List_3(lv_ui *ui)
{
    //Write codes List_3
    ui->List_3 = lv_obj_create(NULL);
    lv_obj_set_size(ui->List_3, 240, 280);
    lv_obj_set_scrollbar_mode(ui->List_3, LV_SCROLLBAR_MODE_OFF);

    //Write style for List_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->List_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->List_3, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->List_3, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes List_3_btn_1
    ui->List_3_btn_1 = lv_btn_create(ui->List_3);
    ui->List_3_btn_1_label = lv_label_create(ui->List_3_btn_1);
    lv_label_set_text(ui->List_3_btn_1_label, "< Home");
    lv_label_set_long_mode(ui->List_3_btn_1_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->List_3_btn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->List_3_btn_1, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->List_3_btn_1_label, LV_PCT(100));
    lv_obj_set_pos(ui->List_3_btn_1, 12, 9);
    lv_obj_set_size(ui->List_3_btn_1, 66, 24);

    //Write style for List_3_btn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->List_3_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->List_3_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->List_3_btn_1, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->List_3_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->List_3_btn_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->List_3_btn_1, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->List_3_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->List_3_btn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes List_3_label_1
    ui->List_3_label_1 = lv_label_create(ui->List_3);
    lv_label_set_text(ui->List_3_label_1, "^");
    lv_label_set_long_mode(ui->List_3_label_1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->List_3_label_1, 110, 76);
    lv_obj_set_size(ui->List_3_label_1, 20, 10);

    //Write style for List_3_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->List_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->List_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->List_3_label_1, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->List_3_label_1, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->List_3_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->List_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->List_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->List_3_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->List_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->List_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->List_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->List_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->List_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->List_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes List_3_img_1
    ui->List_3_img_1 = lv_img_create(ui->List_3);
    lv_obj_add_flag(ui->List_3_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->List_3_img_1, &_heart32x32_alpha_32x32_ext);
    lv_img_set_pivot(ui->List_3_img_1, 50,50);
    lv_img_set_angle(ui->List_3_img_1, 0);
    lv_obj_set_pos(ui->List_3_img_1, 100, 30);
    lv_obj_set_size(ui->List_3_img_1, 32, 32);

    //Write style for List_3_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->List_3_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->List_3_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->List_3_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->List_3_img_1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes List_3_img_2
    ui->List_3_img_2 = lv_img_create(ui->List_3);
    lv_obj_add_flag(ui->List_3_img_2, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->List_3_img_2, &_weater32x32_alpha_32x32_ext);
    lv_img_set_pivot(ui->List_3_img_2, 50,50);
    lv_img_set_angle(ui->List_3_img_2, 0);
    lv_obj_set_pos(ui->List_3_img_2, 190, 80);
    lv_obj_set_size(ui->List_3_img_2, 32, 32);

    //Write style for List_3_img_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->List_3_img_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->List_3_img_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->List_3_img_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->List_3_img_2, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes List_3_img_3
    ui->List_3_img_3 = lv_img_create(ui->List_3);
    lv_obj_add_flag(ui->List_3_img_3, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->List_3_img_3, &_qrcode32x32_alpha_32x32_ext);
    lv_img_set_pivot(ui->List_3_img_3, 50,50);
    lv_img_set_angle(ui->List_3_img_3, 0);
    lv_obj_set_pos(ui->List_3_img_3, 20, 80);
    lv_obj_set_size(ui->List_3_img_3, 32, 32);

    //Write style for List_3_img_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->List_3_img_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->List_3_img_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->List_3_img_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->List_3_img_3, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes List_3_img_4
    ui->List_3_img_4 = lv_img_create(ui->List_3);
    lv_obj_add_flag(ui->List_3_img_4, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->List_3_img_4, &_set32x32_alpha_32x32_ext);
    lv_img_set_pivot(ui->List_3_img_4, 50,50);
    lv_img_set_angle(ui->List_3_img_4, 0);
    lv_obj_set_pos(ui->List_3_img_4, 190, 170);
    lv_obj_set_size(ui->List_3_img_4, 32, 32);

    //Write style for List_3_img_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->List_3_img_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->List_3_img_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->List_3_img_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->List_3_img_4, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes List_3_img_5
    ui->List_3_img_5 = lv_img_create(ui->List_3);
    lv_obj_add_flag(ui->List_3_img_5, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->List_3_img_5, &_syteam32x32_alpha_32x32_ext);
    lv_img_set_pivot(ui->List_3_img_5, 50,50);
    lv_img_set_angle(ui->List_3_img_5, 0);
    lv_obj_set_pos(ui->List_3_img_5, 150, 220);
    lv_obj_set_size(ui->List_3_img_5, 32, 32);

    //Write style for List_3_img_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->List_3_img_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->List_3_img_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->List_3_img_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->List_3_img_5, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes List_3_img_6
    ui->List_3_img_6 = lv_img_create(ui->List_3);
    lv_obj_add_flag(ui->List_3_img_6, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->List_3_img_6, &_NFC32x32_alpha_32x32_ext);
    lv_img_set_pivot(ui->List_3_img_6, 50,50);
    lv_img_set_angle(ui->List_3_img_6, 0);
    lv_obj_set_pos(ui->List_3_img_6, 70, 220);
    lv_obj_set_size(ui->List_3_img_6, 32, 32);

    //Write style for List_3_img_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->List_3_img_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->List_3_img_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->List_3_img_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->List_3_img_6, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes List_3_img_7
    ui->List_3_img_7 = lv_img_create(ui->List_3);
    lv_obj_add_flag(ui->List_3_img_7, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->List_3_img_7, &_location32x32_alpha_32x32_ext);
    lv_img_set_pivot(ui->List_3_img_7, 50,50);
    lv_img_set_angle(ui->List_3_img_7, 0);
    lv_obj_set_pos(ui->List_3_img_7, 20, 170);
    lv_obj_set_size(ui->List_3_img_7, 32, 32);

    //Write style for List_3_img_7, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->List_3_img_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->List_3_img_7, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->List_3_img_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->List_3_img_7, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes List_3_cont_1
    ui->List_3_cont_1 = lv_obj_create(ui->List_3);
    lv_obj_set_pos(ui->List_3_cont_1, 0, 40);
    lv_obj_set_size(ui->List_3_cont_1, 240, 240);
    lv_obj_set_scrollbar_mode(ui->List_3_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for List_3_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->List_3_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->List_3_cont_1, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->List_3_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->List_3_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->List_3_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->List_3_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->List_3_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->List_3_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes List_3_btn_2
    ui->List_3_btn_2 = lv_btn_create(ui->List_3);
    ui->List_3_btn_2_label = lv_label_create(ui->List_3_btn_2);
    lv_label_set_text(ui->List_3_btn_2_label, "");
    lv_label_set_long_mode(ui->List_3_btn_2_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->List_3_btn_2_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->List_3_btn_2, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->List_3_btn_2_label, LV_PCT(100));
    lv_obj_set_pos(ui->List_3_btn_2, 70, 95);
    lv_obj_set_size(ui->List_3_btn_2, 100, 100);

    //Write style for List_3_btn_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->List_3_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->List_3_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->List_3_btn_2, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->List_3_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->List_3_btn_2, &_logo_100x100_ext, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->List_3_btn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->List_3_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->List_3_btn_2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->List_3_btn_2, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->List_3_btn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->List_3_btn_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of List_3.


    //Update current screen layout.
    lv_obj_update_layout(ui->List_3);

    //Init events for screen.
    events_init_List_3(ui);
}

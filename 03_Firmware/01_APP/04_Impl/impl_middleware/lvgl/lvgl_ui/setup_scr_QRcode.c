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



void setup_scr_QRcode(lv_ui *ui)
{
    //Write codes QRcode
    ui->QRcode = lv_obj_create(NULL);
    lv_obj_set_size(ui->QRcode, 240, 280);
    lv_obj_set_scrollbar_mode(ui->QRcode, LV_SCROLLBAR_MODE_OFF);

    //Write style for QRcode, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->QRcode, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->QRcode, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->QRcode, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes QRcode_btn_1
    ui->QRcode_btn_1 = lv_btn_create(ui->QRcode);
    ui->QRcode_btn_1_label = lv_label_create(ui->QRcode_btn_1);
    lv_label_set_text(ui->QRcode_btn_1_label, "< Menu");
    lv_label_set_long_mode(ui->QRcode_btn_1_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->QRcode_btn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->QRcode_btn_1, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->QRcode_btn_1_label, LV_PCT(100));
    lv_obj_set_pos(ui->QRcode_btn_1, 9, 8);
    lv_obj_set_size(ui->QRcode_btn_1, 63, 21);

    //Write style for QRcode_btn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->QRcode_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->QRcode_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->QRcode_btn_1, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->QRcode_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->QRcode_btn_1, lv_color_hex(0x1bff00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->QRcode_btn_1, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->QRcode_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->QRcode_btn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes QRcode_img_1
    ui->QRcode_img_1 = lv_img_create(ui->QRcode);
    lv_obj_add_flag(ui->QRcode_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_img_set_src(ui->QRcode_img_1, &_qrcode32x32_alpha_32x32_ext);
    lv_img_set_pivot(ui->QRcode_img_1, 50,50);
    lv_img_set_angle(ui->QRcode_img_1, 0);
    lv_obj_set_pos(ui->QRcode_img_1, 105, 32);
    lv_obj_set_size(ui->QRcode_img_1, 32, 32);

    //Write style for QRcode_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_recolor_opa(ui->QRcode_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(ui->QRcode_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->QRcode_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->QRcode_img_1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes QRcode_label_1
    ui->QRcode_label_1 = lv_label_create(ui->QRcode);
    lv_label_set_text(ui->QRcode_label_1, "二维码组网公钥");
    lv_label_set_long_mode(ui->QRcode_label_1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->QRcode_label_1, 58, 70);
    lv_obj_set_size(ui->QRcode_label_1, 126, 18);

    //Write style for QRcode_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->QRcode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->QRcode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->QRcode_label_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->QRcode_label_1, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->QRcode_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->QRcode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->QRcode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->QRcode_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->QRcode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->QRcode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->QRcode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->QRcode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->QRcode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->QRcode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes QRcode_btn_2
    ui->QRcode_btn_2 = lv_btn_create(ui->QRcode);
    ui->QRcode_btn_2_label = lv_label_create(ui->QRcode_btn_2);
    lv_label_set_text(ui->QRcode_btn_2_label, "Connnect");
    lv_label_set_long_mode(ui->QRcode_btn_2_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->QRcode_btn_2_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->QRcode_btn_2, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->QRcode_btn_2_label, LV_PCT(100));
    lv_obj_set_pos(ui->QRcode_btn_2, 52, 230);
    lv_obj_set_size(ui->QRcode_btn_2, 142, 44);

    //Write style for QRcode_btn_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->QRcode_btn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->QRcode_btn_2, lv_color_hex(0xff9d00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->QRcode_btn_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->QRcode_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->QRcode_btn_2, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->QRcode_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->QRcode_btn_2, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->QRcode_btn_2, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->QRcode_btn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->QRcode_btn_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes QRcode_cont_1
    ui->QRcode_cont_1 = lv_obj_create(ui->QRcode);
    lv_obj_set_pos(ui->QRcode_cont_1, 31, 97);
    lv_obj_set_size(ui->QRcode_cont_1, 186, 122);
    lv_obj_set_scrollbar_mode(ui->QRcode_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for QRcode_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->QRcode_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->QRcode_cont_1, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->QRcode_cont_1, 216, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->QRcode_cont_1, lv_color_hex(0x111111), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->QRcode_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->QRcode_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->QRcode_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->QRcode_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->QRcode_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->QRcode_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes QRcode_label_2
    ui->QRcode_label_2 = lv_label_create(ui->QRcode_cont_1);
    lv_label_set_text(ui->QRcode_label_2, "a721b32e217f9ea21");
    lv_label_set_long_mode(ui->QRcode_label_2, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->QRcode_label_2, 3, 50);
    lv_obj_set_size(ui->QRcode_label_2, 179, 18);

    //Write style for QRcode_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->QRcode_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->QRcode_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->QRcode_label_2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->QRcode_label_2, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->QRcode_label_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->QRcode_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->QRcode_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->QRcode_label_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->QRcode_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->QRcode_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->QRcode_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->QRcode_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->QRcode_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->QRcode_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of QRcode.


    //Update current screen layout.
    lv_obj_update_layout(ui->QRcode);

    //Init events for screen.
    events_init_QRcode(ui);
}

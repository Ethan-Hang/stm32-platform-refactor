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



void setup_scr_Systeamupdate_cheak(lv_ui *ui)
{
    //Write codes Systeamupdate_cheak
    ui->Systeamupdate_cheak = lv_obj_create(NULL);
    lv_obj_set_size(ui->Systeamupdate_cheak, 240, 280);
    lv_obj_set_scrollbar_mode(ui->Systeamupdate_cheak, LV_SCROLLBAR_MODE_OFF);

    //Write style for Systeamupdate_cheak, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Systeamupdate_cheak, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->Systeamupdate_cheak, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->Systeamupdate_cheak, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Systeamupdate_cheak_label_1
    ui->Systeamupdate_cheak_label_1 = lv_label_create(ui->Systeamupdate_cheak);
    lv_label_set_text(ui->Systeamupdate_cheak_label_1, "型号");
    lv_label_set_long_mode(ui->Systeamupdate_cheak_label_1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->Systeamupdate_cheak_label_1, 94, 19);
    lv_obj_set_size(ui->Systeamupdate_cheak_label_1, 35, 14);

    //Write style for Systeamupdate_cheak_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->Systeamupdate_cheak_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Systeamupdate_cheak_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Systeamupdate_cheak_label_1, lv_color_hex(0x474747), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Systeamupdate_cheak_label_1, &lv_font_alimama_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->Systeamupdate_cheak_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->Systeamupdate_cheak_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->Systeamupdate_cheak_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->Systeamupdate_cheak_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->Systeamupdate_cheak_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->Systeamupdate_cheak_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->Systeamupdate_cheak_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->Systeamupdate_cheak_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->Systeamupdate_cheak_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Systeamupdate_cheak_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Systeamupdate_cheak_label_2
    ui->Systeamupdate_cheak_label_2 = lv_label_create(ui->Systeamupdate_cheak);
    lv_label_set_text(ui->Systeamupdate_cheak_label_2, "GLL_AL01");
    lv_label_set_long_mode(ui->Systeamupdate_cheak_label_2, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->Systeamupdate_cheak_label_2, 78, 42);
    lv_obj_set_size(ui->Systeamupdate_cheak_label_2, 70, 14);

    //Write style for Systeamupdate_cheak_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->Systeamupdate_cheak_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Systeamupdate_cheak_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Systeamupdate_cheak_label_2, lv_color_hex(0x474747), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Systeamupdate_cheak_label_2, &lv_font_alimama_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->Systeamupdate_cheak_label_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->Systeamupdate_cheak_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->Systeamupdate_cheak_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->Systeamupdate_cheak_label_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->Systeamupdate_cheak_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->Systeamupdate_cheak_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->Systeamupdate_cheak_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->Systeamupdate_cheak_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->Systeamupdate_cheak_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Systeamupdate_cheak_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Systeamupdate_cheak_label_3
    ui->Systeamupdate_cheak_label_3 = lv_label_create(ui->Systeamupdate_cheak);
    lv_label_set_text(ui->Systeamupdate_cheak_label_3, "检测到新版本");
    lv_label_set_long_mode(ui->Systeamupdate_cheak_label_3, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->Systeamupdate_cheak_label_3, 66, 105);
    lv_obj_set_size(ui->Systeamupdate_cheak_label_3, 100, 19);

    //Write style for Systeamupdate_cheak_label_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->Systeamupdate_cheak_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Systeamupdate_cheak_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Systeamupdate_cheak_label_3, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Systeamupdate_cheak_label_3, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->Systeamupdate_cheak_label_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->Systeamupdate_cheak_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->Systeamupdate_cheak_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->Systeamupdate_cheak_label_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->Systeamupdate_cheak_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->Systeamupdate_cheak_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->Systeamupdate_cheak_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->Systeamupdate_cheak_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->Systeamupdate_cheak_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Systeamupdate_cheak_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Systeamupdate_cheak_label_4
    ui->Systeamupdate_cheak_label_4 = lv_label_create(ui->Systeamupdate_cheak);
    lv_label_set_text(ui->Systeamupdate_cheak_label_4, "当前版本");
    lv_label_set_long_mode(ui->Systeamupdate_cheak_label_4, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->Systeamupdate_cheak_label_4, 34, 159);
    lv_obj_set_size(ui->Systeamupdate_cheak_label_4, 66, 19);

    //Write style for Systeamupdate_cheak_label_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->Systeamupdate_cheak_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Systeamupdate_cheak_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Systeamupdate_cheak_label_4, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Systeamupdate_cheak_label_4, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->Systeamupdate_cheak_label_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->Systeamupdate_cheak_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->Systeamupdate_cheak_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->Systeamupdate_cheak_label_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->Systeamupdate_cheak_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->Systeamupdate_cheak_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->Systeamupdate_cheak_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->Systeamupdate_cheak_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->Systeamupdate_cheak_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Systeamupdate_cheak_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Systeamupdate_cheak_label_5
    ui->Systeamupdate_cheak_label_5 = lv_label_create(ui->Systeamupdate_cheak);
    lv_label_set_text(ui->Systeamupdate_cheak_label_5, "是否升级至");
    lv_label_set_long_mode(ui->Systeamupdate_cheak_label_5, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->Systeamupdate_cheak_label_5, 21, 190);
    lv_obj_set_size(ui->Systeamupdate_cheak_label_5, 81, 19);

    //Write style for Systeamupdate_cheak_label_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->Systeamupdate_cheak_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Systeamupdate_cheak_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Systeamupdate_cheak_label_5, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Systeamupdate_cheak_label_5, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->Systeamupdate_cheak_label_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->Systeamupdate_cheak_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->Systeamupdate_cheak_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->Systeamupdate_cheak_label_5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->Systeamupdate_cheak_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->Systeamupdate_cheak_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->Systeamupdate_cheak_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->Systeamupdate_cheak_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->Systeamupdate_cheak_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Systeamupdate_cheak_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Systeamupdate_cheak_label_6
    ui->Systeamupdate_cheak_label_6 = lv_label_create(ui->Systeamupdate_cheak);
    lv_label_set_text(ui->Systeamupdate_cheak_label_6, "XX:XX:XX");
    lv_label_set_long_mode(ui->Systeamupdate_cheak_label_6, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->Systeamupdate_cheak_label_6, 135, 159);
    lv_obj_set_size(ui->Systeamupdate_cheak_label_6, 78, 14);

    //Write style for Systeamupdate_cheak_label_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->Systeamupdate_cheak_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Systeamupdate_cheak_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Systeamupdate_cheak_label_6, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Systeamupdate_cheak_label_6, &lv_font_alimama_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->Systeamupdate_cheak_label_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->Systeamupdate_cheak_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->Systeamupdate_cheak_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->Systeamupdate_cheak_label_6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->Systeamupdate_cheak_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->Systeamupdate_cheak_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->Systeamupdate_cheak_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->Systeamupdate_cheak_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->Systeamupdate_cheak_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Systeamupdate_cheak_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Systeamupdate_cheak_label_7
    ui->Systeamupdate_cheak_label_7 = lv_label_create(ui->Systeamupdate_cheak);
    lv_label_set_text(ui->Systeamupdate_cheak_label_7, "XX:XX:XX");
    lv_label_set_long_mode(ui->Systeamupdate_cheak_label_7, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->Systeamupdate_cheak_label_7, 135, 190);
    lv_obj_set_size(ui->Systeamupdate_cheak_label_7, 78, 14);

    //Write style for Systeamupdate_cheak_label_7, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->Systeamupdate_cheak_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Systeamupdate_cheak_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Systeamupdate_cheak_label_7, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Systeamupdate_cheak_label_7, &lv_font_alimama_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->Systeamupdate_cheak_label_7, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->Systeamupdate_cheak_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->Systeamupdate_cheak_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->Systeamupdate_cheak_label_7, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->Systeamupdate_cheak_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->Systeamupdate_cheak_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->Systeamupdate_cheak_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->Systeamupdate_cheak_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->Systeamupdate_cheak_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Systeamupdate_cheak_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Systeamupdate_cheak_btn_1
    ui->Systeamupdate_cheak_btn_1 = lv_btn_create(ui->Systeamupdate_cheak);
    ui->Systeamupdate_cheak_btn_1_label = lv_label_create(ui->Systeamupdate_cheak_btn_1);
    lv_label_set_text(ui->Systeamupdate_cheak_btn_1_label, "X");
    lv_label_set_long_mode(ui->Systeamupdate_cheak_btn_1_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->Systeamupdate_cheak_btn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->Systeamupdate_cheak_btn_1, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->Systeamupdate_cheak_btn_1_label, LV_PCT(100));
    lv_obj_set_pos(ui->Systeamupdate_cheak_btn_1, 27, 229);
    lv_obj_set_size(ui->Systeamupdate_cheak_btn_1, 72, 35);

    //Write style for Systeamupdate_cheak_btn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Systeamupdate_cheak_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->Systeamupdate_cheak_btn_1, lv_color_hex(0x404040), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->Systeamupdate_cheak_btn_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->Systeamupdate_cheak_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Systeamupdate_cheak_btn_1, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Systeamupdate_cheak_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Systeamupdate_cheak_btn_1, lv_color_hex(0xb7b7b7), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Systeamupdate_cheak_btn_1, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->Systeamupdate_cheak_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->Systeamupdate_cheak_btn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes Systeamupdate_cheak_btn_2
    ui->Systeamupdate_cheak_btn_2 = lv_btn_create(ui->Systeamupdate_cheak);
    ui->Systeamupdate_cheak_btn_2_label = lv_label_create(ui->Systeamupdate_cheak_btn_2);
    lv_label_set_text(ui->Systeamupdate_cheak_btn_2_label, "√");
    lv_label_set_long_mode(ui->Systeamupdate_cheak_btn_2_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->Systeamupdate_cheak_btn_2_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->Systeamupdate_cheak_btn_2, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->Systeamupdate_cheak_btn_2_label, LV_PCT(100));
    lv_obj_set_pos(ui->Systeamupdate_cheak_btn_2, 135, 229);
    lv_obj_set_size(ui->Systeamupdate_cheak_btn_2, 72, 35);

    //Write style for Systeamupdate_cheak_btn_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Systeamupdate_cheak_btn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->Systeamupdate_cheak_btn_2, lv_color_hex(0xd25300), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->Systeamupdate_cheak_btn_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->Systeamupdate_cheak_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->Systeamupdate_cheak_btn_2, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Systeamupdate_cheak_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Systeamupdate_cheak_btn_2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Systeamupdate_cheak_btn_2, &lv_font_alimama_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->Systeamupdate_cheak_btn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->Systeamupdate_cheak_btn_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of Systeamupdate_cheak.


    //Update current screen layout.
    lv_obj_update_layout(ui->Systeamupdate_cheak);

    //Init events for screen.
    events_init_Systeamupdate_cheak(ui);
}

/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

typedef struct
{
  
	lv_obj_t *Clock_1;
	bool Clock_1_del;
	lv_obj_t *Clock_1_label_1;
	lv_obj_t *Clock_1_arc_1;
	lv_obj_t *Clock_1_arc_2;
	lv_obj_t *Clock_1_arc_3;
	lv_obj_t *Clock_1_arc_4;
	lv_obj_t *Clock_1_img_1;
	lv_obj_t *Clock_1_img_2;
	lv_obj_t *Clock_1_img_3;
	lv_obj_t *Clock_1_img_4;
	lv_obj_t *Clock_1_img_5;
	lv_obj_t *Clock_1_label_2;
	lv_obj_t *Clock_1_label_3;
	lv_obj_t *Clock_1_label_4;
	lv_obj_t *Clock_1_label_5;
	lv_obj_t *Clock_1_label_6;
	lv_obj_t *Clock_1_label_7;
	lv_obj_t *Clock_1_label_8;
	lv_obj_t *Clock_1_label_9;
	lv_obj_t *Clock_1_cont_1;
	lv_obj_t *Clock_1_btn_1;
	lv_obj_t *Clock_1_btn_1_label;
	lv_obj_t *Clock_1_btn_2;
	lv_obj_t *Clock_1_btn_2_label;
	lv_obj_t *Clock_1_btn_3;
	lv_obj_t *Clock_1_btn_3_label;
	lv_obj_t *Clock_1_img_6;
	lv_obj_t *Clock_1_img_7;
	lv_obj_t *Clock_1_label_11;
	lv_obj_t *Clock_1_img_8;
	lv_obj_t *Clock_1_img_9;
	lv_obj_t *Clock_1_img_10;
	lv_obj_t *Clock_1_label_10;
	lv_obj_t *Clock_1_label_12;
	lv_obj_t *Clock_1_cont_2;
	lv_obj_t *Clock_2;
	bool Clock_2_del;
	lv_obj_t *Clock_2_img_1;
	lv_obj_t *Clock_2_label_1;
	lv_obj_t *Clock_2_label_2;
	lv_obj_t *Clock_2_cont_1;
	lv_obj_t *Clock_2_btn_3;
	lv_obj_t *Clock_2_btn_3_label;
	lv_obj_t *Clock_2_btn_2;
	lv_obj_t *Clock_2_btn_2_label;
	lv_obj_t *Clock_2_btn_1;
	lv_obj_t *Clock_2_btn_1_label;
	lv_obj_t *Clock_2_img_6;
	lv_obj_t *Clock_2_img_5;
	lv_obj_t *Clock_2_label_5;
	lv_obj_t *Clock_2_img_4;
	lv_obj_t *Clock_2_img_3;
	lv_obj_t *Clock_2_img_2;
	lv_obj_t *Clock_2_label_4;
	lv_obj_t *Clock_2_label_3;
	lv_obj_t *Clock_2_cont_2;
	lv_obj_t *Clock_3;
	bool Clock_3_del;
	lv_obj_t *Clock_3_analog_clock_1;
	lv_obj_t *Clock_3_img_1;
	lv_obj_t *Clock_3_img_2;
	lv_obj_t *Clock_3_img_3;
	lv_obj_t *Clock_3_label_1;
	lv_obj_t *Clock_3_label_2;
	lv_obj_t *Clock_3_cont_1;
	lv_obj_t *Clock_3_cont_2;
	lv_obj_t *Clock_3_img_4;
	lv_obj_t *Clock_3_img_5;
	lv_obj_t *Clock_3_img_6;
	lv_obj_t *Clock_3_cont_3;
	lv_obj_t *Clock_3_btn_3;
	lv_obj_t *Clock_3_btn_3_label;
	lv_obj_t *Clock_3_btn_2;
	lv_obj_t *Clock_3_btn_2_label;
	lv_obj_t *Clock_3_btn_1;
	lv_obj_t *Clock_3_btn_1_label;
	lv_obj_t *Clock_3_img_11;
	lv_obj_t *Clock_3_img_10;
	lv_obj_t *Clock_3_label_5;
	lv_obj_t *Clock_3_img_9;
	lv_obj_t *Clock_3_img_8;
	lv_obj_t *Clock_3_img_7;
	lv_obj_t *Clock_3_label_4;
	lv_obj_t *Clock_3_label_3;
	lv_obj_t *Clock_3_cont_4;
	lv_obj_t *top_lap;
	bool top_lap_del;
	lv_obj_t *top_lap_label_1;
	lv_obj_t *top_lap_img_1;
	lv_obj_t *top_lap_cont_1;
	lv_obj_t *top_lap_img_2;
	lv_obj_t *top_lap_cont_2;
	lv_obj_t *top_lap_img_3;
	lv_obj_t *top_lap_cont_3;
	lv_obj_t *top_lap_img_4;
	lv_obj_t *top_lap_cont_4;
	lv_obj_t *top_lap_img_5;
	lv_obj_t *top_lap_slider_1;
	lv_obj_t *under_up;
	bool under_up_del;
	lv_obj_t *under_up_img_1;
	lv_obj_t *under_up_img_6;
	lv_obj_t *under_up_img_2;
	lv_obj_t *under_up_cont_1;
	lv_obj_t *under_up_cont_2;
	lv_obj_t *under_up_cont_3;
	lv_obj_t *under_up_img_3;
	lv_obj_t *under_up_label_1;
	lv_obj_t *under_up_img_4;
	lv_obj_t *under_up_label_2;
	lv_obj_t *under_up_img_5;
	lv_obj_t *under_up_label_3;
	lv_obj_t *under_up_cont_4;
	lv_obj_t *under_up_cont_5;
	lv_obj_t *under_up_img_8;
	lv_obj_t *under_up_img_7;
	lv_obj_t *List_1;
	bool List_1_del;
	lv_obj_t *List_1_btn_1;
	lv_obj_t *List_1_btn_1_label;
	lv_obj_t *List_1_cont_1;
	lv_obj_t *List_1_cont_2;
	lv_obj_t *List_1_img_1;
	lv_obj_t *List_1_label_1;
	lv_obj_t *List_1_cont_3;
	lv_obj_t *List_1_img_2;
	lv_obj_t *List_1_label_2;
	lv_obj_t *List_1_cont_4;
	lv_obj_t *List_1_img_3;
	lv_obj_t *List_1_label_3;
	lv_obj_t *List_1_cont_5;
	lv_obj_t *List_1_img_4;
	lv_obj_t *List_1_label_4;
	lv_obj_t *List_1_cont_6;
	lv_obj_t *List_1_img_5;
	lv_obj_t *List_1_label_5;
	lv_obj_t *List_1_cont_7;
	lv_obj_t *List_1_img_6;
	lv_obj_t *List_1_label_6;
	lv_obj_t *List_1_cont_8;
	lv_obj_t *List_1_img_7;
	lv_obj_t *List_1_label_7;
	lv_obj_t *List_1_btn_2;
	lv_obj_t *List_1_btn_2_label;
	lv_obj_t *List_2;
	bool List_2_del;
	lv_obj_t *List_2_btn_2;
	lv_obj_t *List_2_btn_2_label;
	lv_obj_t *List_2_cont_1;
	lv_obj_t *List_2_cont_8;
	lv_obj_t *List_2_img_7;
	lv_obj_t *List_2_label_7;
	lv_obj_t *List_2_cont_7;
	lv_obj_t *List_2_img_6;
	lv_obj_t *List_2_label_6;
	lv_obj_t *List_2_cont_6;
	lv_obj_t *List_2_img_5;
	lv_obj_t *List_2_label_5;
	lv_obj_t *List_2_cont_5;
	lv_obj_t *List_2_img_4;
	lv_obj_t *List_2_label_4;
	lv_obj_t *List_2_cont_4;
	lv_obj_t *List_2_img_3;
	lv_obj_t *List_2_label_3;
	lv_obj_t *List_2_cont_3;
	lv_obj_t *List_2_img_2;
	lv_obj_t *List_2_label_2;
	lv_obj_t *List_2_cont_2;
	lv_obj_t *List_2_img_1;
	lv_obj_t *List_2_label_1;
	lv_obj_t *List_3;
	bool List_3_del;
	lv_obj_t *List_3_btn_1;
	lv_obj_t *List_3_btn_1_label;
	lv_obj_t *List_3_label_1;
	lv_obj_t *List_3_img_1;
	lv_obj_t *List_3_img_2;
	lv_obj_t *List_3_img_3;
	lv_obj_t *List_3_img_4;
	lv_obj_t *List_3_img_5;
	lv_obj_t *List_3_img_6;
	lv_obj_t *List_3_img_7;
	lv_obj_t *List_3_cont_1;
	lv_obj_t *List_3_btn_2;
	lv_obj_t *List_3_btn_2_label;
	lv_obj_t *Heart;
	bool Heart_del;
	lv_obj_t *Heart_btn_1;
	lv_obj_t *Heart_btn_1_label;
	lv_obj_t *Heart_cont_1;
	lv_obj_t *Heart_img_1;
	lv_obj_t *Heart_label_1;
	lv_obj_t *Heart_slider_1;
	lv_obj_t *Heart_img_2;
	lv_obj_t *Heart_label_2;
	lv_obj_t *Map;
	bool Map_del;
	lv_obj_t *Map_btn_1;
	lv_obj_t *Map_btn_1_label;
	lv_obj_t *Map_label_1;
	lv_obj_t *Map_img_1;
	lv_obj_t *Map_cont_1;
	lv_obj_t *Map_cont_2;
	lv_obj_t *Map_cont_3;
	lv_obj_t *Map_img_2;
	lv_obj_t *NFC;
	bool NFC_del;
	lv_obj_t *NFC_btn_1;
	lv_obj_t *NFC_btn_1_label;
	lv_obj_t *NFC_img_1;
	lv_obj_t *NFC_label_1;
	lv_obj_t *NFC_label_2;
	lv_obj_t *NFC_cont_1;
	lv_obj_t *NFC_label_3;
	lv_obj_t *NFC_label_4;
	lv_obj_t *NFC_label_5;
	lv_obj_t *NFC_label_6;
	lv_obj_t *NFC_label_7;
	lv_obj_t *NFC_label_8;
	lv_obj_t *QRcode;
	bool QRcode_del;
	lv_obj_t *QRcode_btn_1;
	lv_obj_t *QRcode_btn_1_label;
	lv_obj_t *QRcode_img_1;
	lv_obj_t *QRcode_label_1;
	lv_obj_t *QRcode_btn_2;
	lv_obj_t *QRcode_btn_2_label;
	lv_obj_t *QRcode_cont_1;
	lv_obj_t *QRcode_label_2;
	lv_obj_t *Systeamupdate;
	bool Systeamupdate_del;
	lv_obj_t *Systeamupdate_btn_1;
	lv_obj_t *Systeamupdate_btn_1_label;
	lv_obj_t *Systeamupdate_img_1;
	lv_obj_t *Systeamupdate_label_1;
	lv_obj_t *Systeamupdate_label_2;
	lv_obj_t *Systeamupdate_btn_2;
	lv_obj_t *Systeamupdate_btn_2_label;
	lv_obj_t *Systeamupdate_cheak;
	bool Systeamupdate_cheak_del;
	lv_obj_t *Systeamupdate_cheak_label_1;
	lv_obj_t *Systeamupdate_cheak_label_2;
	lv_obj_t *Systeamupdate_cheak_label_3;
	lv_obj_t *Systeamupdate_cheak_label_4;
	lv_obj_t *Systeamupdate_cheak_label_5;
	lv_obj_t *Systeamupdate_cheak_label_6;
	lv_obj_t *Systeamupdate_cheak_label_7;
	lv_obj_t *Systeamupdate_cheak_btn_1;
	lv_obj_t *Systeamupdate_cheak_btn_1_label;
	lv_obj_t *Systeamupdate_cheak_btn_2;
	lv_obj_t *Systeamupdate_cheak_btn_2_label;
	lv_obj_t *Set;
	bool Set_del;
	lv_obj_t *Set_btn_1;
	lv_obj_t *Set_btn_1_label;
	lv_obj_t *Set_cont_1;
	lv_obj_t *Set_label_1;
	lv_obj_t *Set_cont_2;
	lv_obj_t *Set_cont_3;
	lv_obj_t *Set_label_2;
	lv_obj_t *Set_cont_4;
	lv_obj_t *Set_cont_5;
	lv_obj_t *Set_label_3;
	lv_obj_t *Set_cont_6;
	lv_obj_t *Error;
	bool Error_del;
	lv_obj_t *Error_img_1;
	lv_obj_t *Error_label_1;
	lv_obj_t *Error_cont_1;
	lv_obj_t *Error_label_2;
	lv_obj_t *Error_label_3;
	lv_obj_t *Error_label_4;
	lv_obj_t *Error_label_5;
	lv_obj_t *Error_label_6;
}lv_ui;

typedef void (*ui_setup_scr_t)(lv_ui * ui);

void ui_init_style(lv_style_t * style);

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del, ui_setup_scr_t setup_scr,
                           lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay, bool is_clean, bool auto_del);

void ui_animation(void * var, int32_t duration, int32_t delay, int32_t start_value, int32_t end_value, lv_anim_path_cb_t path_cb,
                       uint16_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time, uint32_t playback_delay,
                       lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb, lv_anim_ready_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);


void init_scr_del_flag(lv_ui *ui);

void setup_ui(lv_ui *ui);

void init_keyboard(lv_ui *ui);

extern lv_ui guider_ui;


void setup_scr_Clock_1(lv_ui *ui);
void setup_scr_Clock_2(lv_ui *ui);
void setup_scr_Clock_3(lv_ui *ui);
void setup_scr_top_lap(lv_ui *ui);
void setup_scr_under_up(lv_ui *ui);
void setup_scr_List_1(lv_ui *ui);
void setup_scr_List_2(lv_ui *ui);
void setup_scr_List_3(lv_ui *ui);
void setup_scr_Heart(lv_ui *ui);
void setup_scr_Map(lv_ui *ui);
void setup_scr_NFC(lv_ui *ui);
void setup_scr_QRcode(lv_ui *ui);
void setup_scr_Systeamupdate(lv_ui *ui);
void setup_scr_Systeamupdate_cheak(lv_ui *ui);
void setup_scr_Set(lv_ui *ui);
void setup_scr_Error(lv_ui *ui);
LV_IMG_DECLARE(_sheshidu_alpha_10x10_ext);
LV_IMG_DECLARE(_wather16x16_alpha_16x16_ext);
LV_IMG_DECLARE(_heart16x16_alpha_16x16_ext);
LV_IMG_DECLARE(_KLL16x16_alpha_16x16_ext);
LV_IMG_DECLARE(_foot16x16_alpha_16x16_ext);
LV_IMG_DECLARE(_BT32_alpha_32x32_ext);
LV_IMG_DECLARE(_mianti_0_alpha_32x32_ext);
LV_IMG_DECLARE(_zhengdong_0_alpha_32x32_ext);
LV_IMG_DECLARE(_copesss_alpha_32x32_ext);
LV_IMG_DECLARE(_weater32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_MDLBG_alpha_240x280_ext);
LV_IMG_DECLARE(_BT32_alpha_32x32_ext);
LV_IMG_DECLARE(_mianti_0_alpha_32x32_ext);
LV_IMG_DECLARE(_zhengdong_0_alpha_32x32_ext);
LV_IMG_DECLARE(_copesss_alpha_32x32_ext);
LV_IMG_DECLARE(_weater32x32_alpha_32x32_ext);

LV_IMG_DECLARE(_biaopan1_200x200_ext);
LV_IMG_DECLARE(_time_alpha_50x8_ext);
LV_IMG_DECLARE(_fen_alpha_80x8_ext);
LV_IMG_DECLARE(_miao_alpha_70x5);
LV_IMG_DECLARE(_watchdight1_alpha_60x60_ext);
LV_IMG_DECLARE(_watchdight3_alpha_60x60_ext);
LV_IMG_DECLARE(_watchdight2_alpha_60x60_ext);
LV_IMG_DECLARE(_Ellipse_alpha_40x40_ext);
LV_IMG_DECLARE(_Stime_alpha_16x8_ext);
LV_IMG_DECLARE(_Sfen_alpha_21x6_ext);
LV_IMG_DECLARE(_BT32_alpha_32x32_ext);
LV_IMG_DECLARE(_mianti_0_alpha_32x32_ext);
LV_IMG_DECLARE(_zhengdong_0_alpha_32x32_ext);
LV_IMG_DECLARE(_copesss_alpha_32x32_ext);
LV_IMG_DECLARE(_weater32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_power_hight_alpha_32x32_ext);
LV_IMG_DECLARE(_BT32_alpha_32x32_ext);
LV_IMG_DECLARE(_location_alpha_32x32_ext);
LV_IMG_DECLARE(_taiwan_alpha_32x32_ext);
LV_IMG_DECLARE(_nfc_alpha_32x32_ext);

LV_IMG_DECLARE(_liangdu_47x47_ext);
LV_IMG_DECLARE(_ZNZBG_alpha_100x100_ext);
LV_IMG_DECLARE(_arw_alpha_50x40_ext);
LV_IMG_DECLARE(_ZNZ_alpha_50x50_ext);
LV_IMG_DECLARE(_heart32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_tiwen_alpha_32x32_ext);
LV_IMG_DECLARE(_pa_alpha_32x32_ext);
LV_IMG_DECLARE(_location32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_location32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_heart32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_location32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_NFC32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_qrcode32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_syteam32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_weater32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_set32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_heart32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_location32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_NFC32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_qrcode32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_syteam32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_weater32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_set32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_heart32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_weater32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_qrcode32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_set32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_syteam32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_NFC32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_location32x32_alpha_32x32_ext);

LV_IMG_DECLARE(_logo_100x100_ext);
LV_IMG_DECLARE(_heart32x32_alpha_37x32_ext);
LV_IMG_DECLARE(_BIGHeart_alpha_93x85_ext);
LV_IMG_DECLARE(_location32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_location20x20_alpha_20x20_ext);
LV_IMG_DECLARE(_NFC32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_qrcode32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_syteam32x32_alpha_32x32_ext);
LV_IMG_DECLARE(_error_alpha_48x48_ext);

LV_FONT_DECLARE(lv_font_interttf_24)
LV_FONT_DECLARE(lv_font_interttf_10)
LV_FONT_DECLARE(lv_font_interttf_82)
LV_FONT_DECLARE(lv_font_alimama_16)
LV_FONT_DECLARE(lv_font_alimama_36)
LV_FONT_DECLARE(lv_font_digitaldreamfatnarrow_36)
LV_FONT_DECLARE(lv_font_alimama_12)
LV_FONT_DECLARE(lv_font_alimama_10)
LV_FONT_DECLARE(lv_font_interttf_16)


#ifdef __cplusplus
}
#endif
#endif

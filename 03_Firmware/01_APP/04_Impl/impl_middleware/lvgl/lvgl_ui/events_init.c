/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif

bool Clock_btn_1_is_click=0;
bool Clock_btn_2_is_click=0;
bool Clock_btn_3_is_click=0;
bool Clock1_btn_1_is_click=0;
bool Clock1_btn_2_is_click=0;
bool Clock1_btn_3_is_click=0;
bool Clock2_btn_1_is_click=0;
bool Clock2_btn_2_is_click=0;
bool Clock2_btn_3_is_click=0;

uint8_t screen_index=0;

static int16_t move_distace;
static uint8_t current_moving_index = 0;
lv_anim_t move_anim;
static lv_obj_t* list_1_obj[8];
static bool is_List_visible_1;
static bool is_List_visible_2;
static bool is_List_visible_3;
static bool is_List_visible_4;
static bool is_List_visible_5;
static bool is_List_visible_6;
static bool is_List_visible_7;
static bool is_List_visible_btn;

static uint8_t get_last_visible_index(void) {
    bool *visible_array[7] = {&is_List_visible_1, &is_List_visible_2, &is_List_visible_3,
                              &is_List_visible_4, &is_List_visible_5, &is_List_visible_6, &is_List_visible_7
                             };

    for(int8_t i = 6; i >= 0; i--) {
        if(visible_array[i]) {
            return i;
        }
    }

    return 0;  // 如果都不可见，返回0
}

// 向上移动其他项目的动画
static void move_items_up(void) {
    bool *visible_array[7] = {&is_List_visible_1, &is_List_visible_2, &is_List_visible_3,
                              &is_List_visible_4, &is_List_visible_5, &is_List_visible_6, &is_List_visible_7
                             };


    for(uint8_t i = current_moving_index + 1; i < 7; i++) {
        if(visible_array[i]) {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, list_1_obj[i]);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_time(&a, 500);
            lv_anim_set_values(&a, i * 90, (i-1) * 90);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
            lv_anim_start(&a);
        }
    }

    // 获取最后一个可见项的索引
    uint8_t last_visible = get_last_visible_index();

    // 显示添加按钮
    lv_obj_clear_flag(list_1_obj[7], LV_OBJ_FLAG_HIDDEN);
    is_List_visible_btn = true;

    // 移动添加按钮
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, list_1_obj[7]);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_time(&a, 500);
    lv_anim_set_values(&a, (last_visible + 1) * 90, (last_visible ) * 90);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

void clear_flage(lv_anim_t *a) {
    lv_obj_t* obj=a->var;
    lv_obj_add_flag(obj,LV_OBJ_FLAG_CLICKABLE);
// 设置该项为不可见
    if(list_1_obj[0] == obj) {
        is_List_visible_1 = false;
    }
    else if(list_1_obj[1] == obj)
    {
        is_List_visible_2 = false;
    }
    else if(list_1_obj[2] == obj) {
        is_List_visible_3 = false;
    }
    else if(list_1_obj[3] == obj) {
        is_List_visible_4 = false;
    }
    else if(list_1_obj[4] == obj) {
        is_List_visible_5 = false;
    }
    else if(list_1_obj[5] == obj) {
        is_List_visible_6 = false;
    }
    else if(list_1_obj[6] == obj) {
        is_List_visible_7 = false;
    }

//  move_items_up();
}
// 如果所有项都已显示,隐藏添加按钮
bool all_visible = true;
static uint16_t level;
#include <stdlib.h>
static uint16_t last_angle = 0;
static lv_sqrt_res_t radius;
#define CENTER_X 120
#define CENTER_Y 140
#define menu_3_cnt 7
// 初始坐标
static lv_point_t initial_points[menu_3_cnt] = {
    {100, 30},
    {190, 80},
    {190, 170},
    {150, 220},
    {70, 220},
    {20, 170},
    {20, 80}
};
//起始点
lv_point_t start_point;
bool is_pissing=0;
lv_obj_t* scroll_obj[menu_3_cnt];
int get_touch_quadrant() {
    // 获取屏幕分辨率
    lv_coord_t screen_width = lv_disp_get_hor_res(NULL);
    lv_coord_t screen_height = lv_disp_get_ver_res(NULL);

    // 计算屏幕中心
    lv_point_t center = {
        .x = screen_width / 2,
        .y = screen_height / 2
    };

    // 获取触摸点
    lv_indev_t *indev = lv_indev_get_act();
    lv_point_t point;
    lv_indev_get_point(indev, &point);

    // 计算相对坐标（注意 y 轴方向反转）
    int32_t rel_x = point.x - center.x;
    int32_t rel_y = -(point.y - center.y);

    // 判断象限
    if (rel_x > 0 && rel_y > 0) {
        return 1;
    } else if (rel_x < 0 && rel_y > 0) {
        return 2;
    } else if (rel_x < 0 && rel_y < 0) {
        return 3;
    } else if (rel_x > 0 && rel_y < 0) {
        return 4;
    } else {
        return 0; // 位于坐标轴上
    }
}
lv_anim_t screen_loaded_anim;
uint16_t mode=0;

static void Clock_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_LONG_PRESSED:
    {
        ui_animation(guider_ui.Clock_1_cont_2, 200, 0, lv_obj_get_x(guider_ui.Clock_1_cont_2), 120, &lv_anim_path_overshoot, 1, 0, 0, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
        lv_obj_clear_flag(guider_ui.Clock_1_cont_2, LV_OBJ_FLAG_HIDDEN);
        ui_animation(guider_ui.Clock_1_cont_1, 200, 0, lv_obj_get_x(guider_ui.Clock_1_cont_1), 0, &lv_anim_path_overshoot, 1, 0, 0, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
        break;
    }
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        switch(dir) {
        case LV_DIR_RIGHT:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.Clock_2, guider_ui.Clock_2_del, &guider_ui.Clock_1_del, setup_scr_Clock_2, LV_SCR_LOAD_ANIM_OVER_RIGHT, 200, 0, true, true);
            screen_index=1;
            break;
        }
        case LV_DIR_BOTTOM:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.top_lap, guider_ui.top_lap_del, &guider_ui.Clock_1_del, setup_scr_top_lap, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 200, 200, true, true);
            break;
        }
        case LV_DIR_TOP:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.under_up, guider_ui.under_up_del, &guider_ui.Clock_1_del, setup_scr_under_up, LV_SCR_LOAD_ANIM_OVER_TOP, 200, 200, true, true);
            break;
        }
        case LV_DIR_LEFT:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.List_1, guider_ui.List_1_del, &guider_ui.Clock_1_del, setup_scr_List_1, LV_SCR_LOAD_ANIM_OVER_LEFT, 200, 200, true, true);
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

static void Clock_1_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock_btn_1_is_click) {
            Clock_btn_1_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_1_img_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_1_img_6, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_1_btn_1, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock_btn_1_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_1_img_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_1_img_6, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_1_btn_1, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }

        break;
    }
    default:
        break;
    }
}

static void Clock_1_btn_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock_btn_2_is_click) {
            Clock_btn_2_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_1_img_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_1_img_7, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_1_btn_2, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock_btn_2_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_1_img_7, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_1_img_7, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_1_btn_2, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }

        break;
    }
    default:
        break;
    }
}

static void Clock_1_btn_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock_btn_3_is_click) {
            Clock_btn_3_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_1_img_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_1_img_8, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_1_btn_3, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock_btn_3_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_1_img_8, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_1_img_8, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_1_btn_3, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }

        break;
    }
    default:
        break;
    }
}

static void Clock_1_img_6_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock_btn_1_is_click) {
            Clock_btn_1_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_1_img_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_1_img_6, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_1_btn_1, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock_btn_1_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_1_img_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_1_img_6, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_1_btn_1, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }

        break;
    }
    default:
        break;
    }
}

static void Clock_1_img_7_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock_btn_2_is_click) {
            Clock_btn_2_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_1_img_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_1_img_7, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_1_btn_2, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock_btn_2_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_1_img_7, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_1_img_7, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_1_btn_2, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }

        break;
    }
    default:
        break;
    }
}

static void Clock_1_img_8_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock_btn_3_is_click) {
            Clock_btn_3_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_1_img_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_1_img_8, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_1_btn_3, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock_btn_3_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_1_img_8, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_1_img_8, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_1_btn_3, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }

        break;
    }
    default:
        break;
    }
}

static void Clock_1_cont_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_LONG_PRESSED:
    {
        lv_obj_add_flag(guider_ui.Clock_1_cont_2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_x(guider_ui.Clock_1_cont_2, 0);
        ui_animation(guider_ui.Clock_1_cont_1, 200, 0, lv_obj_get_x(guider_ui.Clock_1_cont_1), -140, &lv_anim_path_overshoot, 1, 0, 0, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
        break;
    }
    default:
        break;
    }
}

void events_init_Clock_1 (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->Clock_1, Clock_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_1_btn_1, Clock_1_btn_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_1_btn_2, Clock_1_btn_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_1_btn_3, Clock_1_btn_3_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_1_img_6, Clock_1_img_6_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_1_img_7, Clock_1_img_7_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_1_img_8, Clock_1_img_8_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_1_cont_2, Clock_1_cont_2_event_handler, LV_EVENT_ALL, ui);
}

static void Clock_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_LONG_PRESSED:
    {
        ui_animation(guider_ui.Clock_2_cont_2, 200, 0, lv_obj_get_x(guider_ui.Clock_2_cont_2), 120, &lv_anim_path_overshoot, 1, 0, 0, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
        lv_obj_clear_flag(guider_ui.Clock_2_cont_2, LV_OBJ_FLAG_HIDDEN);
        ui_animation(guider_ui.Clock_2_cont_1, 200, 0, lv_obj_get_x(guider_ui.Clock_2_cont_1), 0, &lv_anim_path_overshoot, 1, 0, 0, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
        break;
    }
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        switch(dir) {
        case LV_DIR_RIGHT:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.Clock_3, guider_ui.Clock_3_del, &guider_ui.Clock_2_del, setup_scr_Clock_3, LV_SCR_LOAD_ANIM_OVER_RIGHT, 200, 0, true, true);
            screen_index=2;
            break;
        }
        case LV_DIR_TOP:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.under_up, guider_ui.under_up_del, &guider_ui.Clock_2_del, setup_scr_under_up, LV_SCR_LOAD_ANIM_OVER_TOP, 200, 200, true, true);
            break;
        }
        case LV_DIR_BOTTOM:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.top_lap, guider_ui.top_lap_del, &guider_ui.Clock_2_del, setup_scr_top_lap, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 200, 200, true, true);
            break;
        }
        case LV_DIR_LEFT:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.List_1, guider_ui.List_1_del, &guider_ui.Clock_2_del, setup_scr_List_1, LV_SCR_LOAD_ANIM_OVER_LEFT, 200, 200, true, true);
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

static void Clock_2_img_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_LONG_PRESSED:
    {
        lv_obj_clear_flag(guider_ui.Clock_2_cont_2, LV_OBJ_FLAG_HIDDEN);
        ui_animation(guider_ui.Clock_2_cont_2, 200, 0, lv_obj_get_x(guider_ui.Clock_2_cont_2), 120, &lv_anim_path_overshoot, 1, 0, 0, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
        ui_animation(guider_ui.Clock_2_cont_1, 200, 0, lv_obj_get_x(guider_ui.Clock_2_cont_1), 0, &lv_anim_path_overshoot, 1, 0, 0, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
        break;
    }
    default:
        break;
    }
}

static void Clock_2_btn_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock1_btn_3_is_click) {
            Clock1_btn_3_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_2_img_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_2_img_6, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_2_btn_3, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock1_btn_3_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_2_img_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_2_img_6, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_2_btn_3, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }

        break;
    }
    default:
        break;
    }
}

static void Clock_2_btn_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock1_btn_2_is_click) {
            Clock1_btn_2_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_2_img_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_2_img_5, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_2_btn_2, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock1_btn_2_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_2_img_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_2_img_5, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_2_btn_2, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }

        break;
    }
    default:
        break;
    }
}

static void Clock_2_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock1_btn_1_is_click) {
            Clock1_btn_1_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_2_img_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_2_img_4, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_2_btn_1, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock1_btn_1_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_2_img_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_2_img_4, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_2_btn_1, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }

        break;
    }
    default:
        break;
    }
}

static void Clock_2_img_6_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock1_btn_1_is_click) {
            Clock1_btn_1_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_2_img_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_2_img_6, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_2_btn_3, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock1_btn_1_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_2_img_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_2_img_6, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_2_btn_3, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }

        break;
    }
    default:
        break;
    }
}

static void Clock_2_img_5_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock1_btn_2_is_click) {
            Clock1_btn_2_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_2_img_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_2_img_5, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_2_btn_2, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock1_btn_2_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_2_img_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_2_img_5, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_2_btn_2, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }

        break;
    }
    default:
        break;
    }
}

static void Clock_2_img_4_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock1_btn_1_is_click) {
            Clock1_btn_1_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_2_img_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_2_img_4, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_2_btn_1, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock1_btn_1_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_2_img_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_2_img_4, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_2_btn_1, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }

        break;
    }
    default:
        break;
    }
}

static void Clock_2_cont_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_LONG_PRESSED:
    {
        lv_obj_add_flag(guider_ui.Clock_2_cont_2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_x(guider_ui.Clock_2_cont_2, 0);
        ui_animation(guider_ui.Clock_2_cont_1, 200, 0, lv_obj_get_x(guider_ui.Clock_2_cont_1), -140, &lv_anim_path_overshoot, 1, 0, 0, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
        break;
    }
    default:
        break;
    }
}

void events_init_Clock_2 (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->Clock_2, Clock_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_2_img_1, Clock_2_img_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_2_btn_3, Clock_2_btn_3_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_2_btn_2, Clock_2_btn_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_2_btn_1, Clock_2_btn_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_2_img_6, Clock_2_img_6_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_2_img_5, Clock_2_img_5_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_2_img_4, Clock_2_img_4_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_2_cont_2, Clock_2_cont_2_event_handler, LV_EVENT_ALL, ui);
}

static void Clock_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_LONG_PRESSED:
    {
        lv_obj_clear_flag(guider_ui.Clock_3_cont_4, LV_OBJ_FLAG_HIDDEN);
        ui_animation(guider_ui.Clock_3_cont_4, 200, 0, lv_obj_get_x(guider_ui.Clock_3_cont_4), 0, &lv_anim_path_overshoot, 1, 0, 0, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
        ui_animation(guider_ui.Clock_3_cont_3, 200, 0, lv_obj_get_x(guider_ui.Clock_3_cont_3), 0, &lv_anim_path_overshoot, 1, 0, 0, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
        break;
    }
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        switch(dir) {
        case LV_DIR_RIGHT:
        {
            lv_indev_wait_release(lv_indev_get_act());
            screen_index=0;
            ui_load_scr_animation(&guider_ui, &guider_ui.Clock_1, guider_ui.Clock_1_del, &guider_ui.Clock_3_del, setup_scr_Clock_1, LV_SCR_LOAD_ANIM_OVER_RIGHT, 200, 0, false, false);
            break;
        }
        case LV_DIR_TOP:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.under_up, guider_ui.under_up_del, &guider_ui.Clock_3_del, setup_scr_under_up, LV_SCR_LOAD_ANIM_OVER_TOP, 200, 0, false, false);
            break;
        }
        case LV_DIR_BOTTOM:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.top_lap, guider_ui.top_lap_del, &guider_ui.Clock_3_del, setup_scr_top_lap, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 200, 0, false, false);
            break;
        }
        case LV_DIR_LEFT:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.List_1, guider_ui.List_1_del, &guider_ui.Clock_3_del, setup_scr_List_1, LV_SCR_LOAD_ANIM_OVER_LEFT, 200, 200, false, false);
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

static void Clock_3_analog_clock_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_LONG_PRESSED:
    {
        lv_obj_clear_flag(guider_ui.Clock_3_cont_4, LV_OBJ_FLAG_HIDDEN);
        ui_animation(guider_ui.Clock_3_cont_4, 200, 0, lv_obj_get_x(guider_ui.Clock_3_cont_4), 120, &lv_anim_path_overshoot, 1, 0, 0, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
        ui_animation(guider_ui.Clock_3_cont_3, 200, 0, lv_obj_get_x(guider_ui.Clock_3_cont_3), 0, &lv_anim_path_overshoot, 1, 0, 0, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
        break;
    }
    default:
        break;
    }
}

static void Clock_3_btn_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock2_btn_3_is_click) {
            Clock2_btn_3_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_3_img_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_3_img_11, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_3_btn_3, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock2_btn_3_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_3_img_11, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_3_img_11, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_3_btn_3, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }

        break;
    }
    default:
        break;
    }
}

static void Clock_3_btn_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock2_btn_2_is_click) {
            Clock2_btn_2_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_3_img_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_3_img_10, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_3_btn_2, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock2_btn_2_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_3_img_10, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_3_img_10, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_3_btn_2, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }
        break;
    }
    default:
        break;
    }
}

static void Clock_3_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock2_btn_1_is_click) {
            Clock2_btn_1_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_3_img_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_3_img_9, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_3_btn_1, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock2_btn_1_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_3_img_9, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_3_img_9, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_3_btn_1, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }
        break;
    }
    default:
        break;
    }
}

static void Clock_3_img_11_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock2_btn_3_is_click) {
            Clock2_btn_3_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_3_img_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_3_img_11, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_3_btn_3, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock2_btn_3_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_3_img_11, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_3_img_11, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_3_btn_3, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }
        break;
    }
    default:
        break;
    }
}

static void Clock_3_img_10_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock2_btn_2_is_click) {
            Clock2_btn_2_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_3_img_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_3_img_10, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_3_btn_2, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock2_btn_2_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_3_img_10, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_3_img_10, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_3_btn_2, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }
        break;
    }
    default:
        break;
    }
}

static void Clock_3_img_9_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        if(Clock2_btn_1_is_click) {
            Clock2_btn_1_is_click=0;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_3_img_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_3_img_9, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_3_btn_1, lv_color_hex(0x5a5a5a), LV_PART_MAIN|LV_STATE_DEFAULT);
        } else {
            Clock2_btn_1_is_click=1;
            lv_obj_set_style_img_recolor_opa(guider_ui.Clock_3_img_9, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(guider_ui.Clock_3_img_9, lv_color_hex(0x313131), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(guider_ui.Clock_3_btn_1, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
        }
        break;
    }
    default:
        break;
    }
}

static void Clock_3_cont_4_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_LONG_PRESSED:
    {
        lv_obj_add_flag(guider_ui.Clock_3_cont_4, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_x(guider_ui.Clock_3_cont_4, 0);
        ui_animation(guider_ui.Clock_3_cont_3, 200, 0, lv_obj_get_x(guider_ui.Clock_3_cont_3), -140, &lv_anim_path_overshoot, 1, 0, 0, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
        break;
    }
    default:
        break;
    }
}

void events_init_Clock_3 (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->Clock_3, Clock_3_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_3_analog_clock_1, Clock_3_analog_clock_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_3_btn_3, Clock_3_btn_3_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_3_btn_2, Clock_3_btn_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_3_btn_1, Clock_3_btn_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_3_img_11, Clock_3_img_11_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_3_img_10, Clock_3_img_10_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_3_img_9, Clock_3_img_9_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Clock_3_cont_4, Clock_3_cont_4_event_handler, LV_EVENT_ALL, ui);
}

static void top_lap_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        switch(dir) {
        case LV_DIR_TOP:
        {
            lv_indev_wait_release(lv_indev_get_act());
            switch (screen_index)
            {
            case 0:
                /* code */
                lv_indev_wait_release(lv_indev_get_act());
                ui_load_scr_animation(&guider_ui, &guider_ui.Clock_1, guider_ui.Clock_1_del, &guider_ui.under_up_del, setup_scr_Clock_1, LV_SCR_LOAD_ANIM_MOVE_TOP, 200, 0, true, true);
                break;
            case 1:
                lv_indev_wait_release(lv_indev_get_act());
                ui_load_scr_animation(&guider_ui, &guider_ui.Clock_2, guider_ui.Clock_2_del, &guider_ui.top_lap_del, setup_scr_Clock_2, LV_SCR_LOAD_ANIM_MOVE_TOP, 200, 0, true, true);
                break;
            case 2:
                lv_indev_wait_release(lv_indev_get_act());
                ui_load_scr_animation(&guider_ui, &guider_ui.Clock_3, guider_ui.Clock_3_del, &guider_ui.top_lap_del, setup_scr_Clock_3, LV_SCR_LOAD_ANIM_MOVE_TOP, 200, 0, true, true);
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

void events_init_top_lap (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->top_lap, top_lap_event_handler, LV_EVENT_ALL, ui);
}

static void under_up_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        switch(dir) {
        case LV_DIR_BOTTOM:
        {
            lv_indev_wait_release(lv_indev_get_act());
            switch (screen_index)
            {
            case 0:
                /* code */
                lv_indev_wait_release(lv_indev_get_act());
                ui_load_scr_animation(&guider_ui, &guider_ui.Clock_1, guider_ui.Clock_1_del, &guider_ui.under_up_del, setup_scr_Clock_1, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 200, 0, true, true);
                break;
            case 1:
                lv_indev_wait_release(lv_indev_get_act());
                ui_load_scr_animation(&guider_ui, &guider_ui.Clock_2, guider_ui.Clock_2_del, &guider_ui.top_lap_del, setup_scr_Clock_2, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 200, 0, true, true);
                break;
            case 2:
                lv_indev_wait_release(lv_indev_get_act());
                ui_load_scr_animation(&guider_ui, &guider_ui.Clock_3, guider_ui.Clock_3_del, &guider_ui.top_lap_del, setup_scr_Clock_3, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 200, 0, true, true);
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

void events_init_under_up (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->under_up, under_up_event_handler, LV_EVENT_ALL, ui);
}

static void List_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        list_1_obj[0]=guider_ui.List_1_cont_2;
        list_1_obj[1]=guider_ui.List_1_cont_3;
        list_1_obj[2]=guider_ui.List_1_cont_4;
        list_1_obj[3]=guider_ui.List_1_cont_5;
        list_1_obj[4]=guider_ui.List_1_cont_6;
        list_1_obj[5]=guider_ui.List_1_cont_7;
        list_1_obj[6]=guider_ui.List_1_cont_8;
        list_1_obj[7]=guider_ui.List_1_btn_2;
        break;
    }
    default:
        break;
    }
}

static void List_1_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        switch (screen_index)
        {
        case 0:
            /* code */
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.Clock_1, guider_ui.Clock_1_del, &guider_ui.List_1_del, setup_scr_Clock_1, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 200, true, true);
            break;
        case 1:
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.Clock_2, guider_ui.Clock_2_del, &guider_ui.List_1_del, setup_scr_Clock_2, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 200, true, true);
            break;
        case 2:
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.Clock_3, guider_ui.Clock_3_del, &guider_ui.List_1_del, setup_scr_Clock_3, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 200, true, true);
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

static void List_1_cont_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Heart, guider_ui.Heart_del, &guider_ui.List_1_del, setup_scr_Heart, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    case LV_EVENT_PRESSING:
    {
        lv_obj_t* obj = lv_event_get_target(e); //获取事件产生的对象
        lv_indev_t* indev = lv_indev_get_act();  //获取活动界面输入设备
        lv_point_t vect;
        lv_indev_get_vect(indev, &vect); //获取vect point
        lv_coord_t x = lv_obj_get_x(obj) + vect.x; //计算x
        move_distace=x;
        // 获取当前移动项的索引
        for(uint8_t i = 0; i < 7; i++) {
            if(list_1_obj[i] == obj) {
                current_moving_index = i;
                break;
            }
        }
        if(move_distace<100&&move_distace>0) {
            lv_obj_set_x(obj, x); //移动对象到x,y
        }
        if(move_distace>100) {
            lv_obj_clear_flag(obj,LV_OBJ_FLAG_CLICKABLE);
            lv_anim_init(&move_anim);
            lv_anim_set_var(&move_anim, obj);                      // 要移动的对象
            lv_anim_set_exec_cb(&move_anim, (lv_anim_exec_xcb_t)lv_obj_set_x); // 修改x坐标
            lv_anim_set_time(&move_anim,(lv_coord_t)1500);                      // 动画持续时间500ms
            lv_anim_set_values(&move_anim,lv_obj_get_x(obj), 240); // 从当前位置到240
            lv_anim_set_path_cb(&move_anim, lv_anim_path_overshoot); // 使用overshoot路径
            lv_anim_set_ready_cb(&move_anim,(lv_anim_ready_cb_t)clear_flage);
            lv_anim_start(&move_anim);
        }
        break;
    }
    default:
        break;
    }
}

static void List_1_img_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Heart, guider_ui.Heart_del, &guider_ui.List_1_del, setup_scr_Heart, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_1_label_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Heart, guider_ui.Heart_del, &guider_ui.List_1_del, setup_scr_Heart, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_1_cont_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Map, guider_ui.Map_del, &guider_ui.List_1_del, setup_scr_Map, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    case LV_EVENT_PRESSING:
    {
        lv_obj_t* obj = lv_event_get_target(e); //获取事件产生的对象
        lv_indev_t* indev = lv_indev_get_act();  //获取活动界面输入设备
        lv_point_t vect;
        lv_indev_get_vect(indev, &vect); //获取vect point
        lv_coord_t x = lv_obj_get_x(obj) + vect.x; //计算x
        move_distace=x;
        // 获取当前移动项的索引
        for(uint8_t i = 0; i < 7; i++) {
            if(list_1_obj[i] == obj) {
                current_moving_index = i;
                break;
            }
        }
        if(move_distace<100&&move_distace>0) {
            lv_obj_set_x(obj, x); //移动对象到x,y
        }
        if(move_distace>100) {
            lv_obj_clear_flag(obj,LV_OBJ_FLAG_CLICKABLE);
            lv_anim_init(&move_anim);
            lv_anim_set_var(&move_anim, obj);                      // 要移动的对象
            lv_anim_set_exec_cb(&move_anim, (lv_anim_exec_xcb_t)lv_obj_set_x); // 修改x坐标
            lv_anim_set_time(&move_anim,(lv_coord_t)1500);                      // 动画持续时间500ms
            lv_anim_set_values(&move_anim,lv_obj_get_x(obj), 240); // 从当前位置到240
            lv_anim_set_path_cb(&move_anim, lv_anim_path_overshoot); // 使用overshoot路径
            lv_anim_set_ready_cb(&move_anim,(lv_anim_ready_cb_t)clear_flage);
            lv_anim_start(&move_anim);
        }
        break;
    }
    default:
        break;
    }
}

static void List_1_img_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Map, guider_ui.Map_del, &guider_ui.List_1_del, setup_scr_Map, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_1_label_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Map, guider_ui.Map_del, &guider_ui.List_1_del, setup_scr_Map, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_1_cont_4_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.NFC, guider_ui.NFC_del, &guider_ui.List_1_del, setup_scr_NFC, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    case LV_EVENT_PRESSING:
    {
        lv_obj_t* obj = lv_event_get_target(e); //获取事件产生的对象
        lv_indev_t* indev = lv_indev_get_act();  //获取活动界面输入设备
        lv_point_t vect;
        lv_indev_get_vect(indev, &vect); //获取vect point
        lv_coord_t x = lv_obj_get_x(obj) + vect.x; //计算x
        move_distace=x;
        // 获取当前移动项的索引
        for(uint8_t i = 0; i < 7; i++) {
            if(list_1_obj[i] == obj) {
                current_moving_index = i;
                break;
            }
        }
        if(move_distace<100&&move_distace>0) {
            lv_obj_set_x(obj, x); //移动对象到x,y
        }
        if(move_distace>100) {
            lv_obj_clear_flag(obj,LV_OBJ_FLAG_CLICKABLE);
            lv_anim_init(&move_anim);
            lv_anim_set_var(&move_anim, obj);                      // 要移动的对象
            lv_anim_set_exec_cb(&move_anim, (lv_anim_exec_xcb_t)lv_obj_set_x); // 修改x坐标
            lv_anim_set_time(&move_anim,(lv_coord_t)1500);                      // 动画持续时间500ms
            lv_anim_set_values(&move_anim,lv_obj_get_x(obj), 240); // 从当前位置到240
            lv_anim_set_path_cb(&move_anim, lv_anim_path_overshoot); // 使用overshoot路径
            lv_anim_set_ready_cb(&move_anim,(lv_anim_ready_cb_t)clear_flage);
            lv_anim_start(&move_anim);
        }
        break;
    }
    default:
        break;
    }
}

static void List_1_img_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.NFC, guider_ui.NFC_del, &guider_ui.List_1_del, setup_scr_NFC, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_1_label_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.NFC, guider_ui.NFC_del, &guider_ui.List_1_del, setup_scr_NFC, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_1_cont_5_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.QRcode, guider_ui.QRcode_del, &guider_ui.List_1_del, setup_scr_QRcode, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    case LV_EVENT_PRESSING:
    {
        lv_obj_t* obj = lv_event_get_target(e); //获取事件产生的对象
        lv_indev_t* indev = lv_indev_get_act();  //获取活动界面输入设备
        lv_point_t vect;
        lv_indev_get_vect(indev, &vect); //获取vect point
        lv_coord_t x = lv_obj_get_x(obj) + vect.x; //计算x
        move_distace=x;
        // 获取当前移动项的索引
        for(uint8_t i = 0; i < 7; i++) {
            if(list_1_obj[i] == obj) {
                current_moving_index = i;
                break;
            }
        }
        if(move_distace<100&&move_distace>0) {
            lv_obj_set_x(obj, x); //移动对象到x,y
        }
        if(move_distace>100) {
            lv_obj_clear_flag(obj,LV_OBJ_FLAG_CLICKABLE);
            lv_anim_init(&move_anim);
            lv_anim_set_var(&move_anim, obj);                      // 要移动的对象
            lv_anim_set_exec_cb(&move_anim, (lv_anim_exec_xcb_t)lv_obj_set_x); // 修改x坐标
            lv_anim_set_time(&move_anim,(lv_coord_t)1500);                      // 动画持续时间500ms
            lv_anim_set_values(&move_anim,lv_obj_get_x(obj), 240); // 从当前位置到240
            lv_anim_set_path_cb(&move_anim, lv_anim_path_overshoot); // 使用overshoot路径
            lv_anim_set_ready_cb(&move_anim,(lv_anim_ready_cb_t)clear_flage);
            lv_anim_start(&move_anim);
        }
        break;
    }
    default:
        break;
    }
}

static void List_1_img_4_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.QRcode, guider_ui.QRcode_del, &guider_ui.List_1_del, setup_scr_QRcode, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_1_label_4_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.QRcode, guider_ui.QRcode_del, &guider_ui.List_1_del, setup_scr_QRcode, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_1_cont_6_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Systeamupdate, guider_ui.Systeamupdate_del, &guider_ui.List_1_del, setup_scr_Systeamupdate, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    case LV_EVENT_PRESSING:
    {
        lv_obj_t* obj = lv_event_get_target(e); //获取事件产生的对象
        lv_indev_t* indev = lv_indev_get_act();  //获取活动界面输入设备
        lv_point_t vect;
        lv_indev_get_vect(indev, &vect); //获取vect point
        lv_coord_t x = lv_obj_get_x(obj) + vect.x; //计算x
        move_distace=x;
        // 获取当前移动项的索引
        for(uint8_t i = 0; i < 7; i++) {
            if(list_1_obj[i] == obj) {
                current_moving_index = i;
                break;
            }
        }
        if(move_distace<100&&move_distace>0) {
            lv_obj_set_x(obj, x); //移动对象到x,y
        }
        if(move_distace>100) {
            lv_obj_clear_flag(obj,LV_OBJ_FLAG_CLICKABLE);
            lv_anim_init(&move_anim);
            lv_anim_set_var(&move_anim, obj);                      // 要移动的对象
            lv_anim_set_exec_cb(&move_anim, (lv_anim_exec_xcb_t)lv_obj_set_x); // 修改x坐标
            lv_anim_set_time(&move_anim,(lv_coord_t)1500);                      // 动画持续时间500ms
            lv_anim_set_values(&move_anim,lv_obj_get_x(obj), 240); // 从当前位置到240
            lv_anim_set_path_cb(&move_anim, lv_anim_path_overshoot); // 使用overshoot路径
            lv_anim_set_ready_cb(&move_anim,(lv_anim_ready_cb_t)clear_flage);
            lv_anim_start(&move_anim);
        }
        break;
    }
    default:
        break;
    }
}

static void List_1_img_5_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Systeamupdate, guider_ui.Systeamupdate_del, &guider_ui.List_1_del, setup_scr_Systeamupdate, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_1_label_5_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Systeamupdate, guider_ui.Systeamupdate_del, &guider_ui.List_1_del, setup_scr_Systeamupdate, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_1_cont_7_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        break;
    }
    case LV_EVENT_PRESSING:
    {
        lv_obj_t* obj = lv_event_get_target(e); //获取事件产生的对象
        lv_indev_t* indev = lv_indev_get_act();  //获取活动界面输入设备
        lv_point_t vect;
        lv_indev_get_vect(indev, &vect); //获取vect point
        lv_coord_t x = lv_obj_get_x(obj) + vect.x; //计算x
        move_distace=x;
        // 获取当前移动项的索引
        for(uint8_t i = 0; i < 7; i++) {
            if(list_1_obj[i] == obj) {
                current_moving_index = i;
                break;
            }
        }
        if(move_distace<100&&move_distace>0) {
            lv_obj_set_x(obj, x); //移动对象到x,y
        }
        if(move_distace>100) {
            lv_obj_clear_flag(obj,LV_OBJ_FLAG_CLICKABLE);
            lv_anim_init(&move_anim);
            lv_anim_set_var(&move_anim, obj);                      // 要移动的对象
            lv_anim_set_exec_cb(&move_anim, (lv_anim_exec_xcb_t)lv_obj_set_x); // 修改x坐标
            lv_anim_set_time(&move_anim,(lv_coord_t)1500);                      // 动画持续时间500ms
            lv_anim_set_values(&move_anim,lv_obj_get_x(obj), 240); // 从当前位置到240
            lv_anim_set_path_cb(&move_anim, lv_anim_path_overshoot); // 使用overshoot路径
            lv_anim_set_ready_cb(&move_anim,(lv_anim_ready_cb_t)clear_flage);
            lv_anim_start(&move_anim);
        }
        break;
    }
    default:
        break;
    }
}

static void List_1_cont_8_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Set, guider_ui.Set_del, &guider_ui.List_1_del, setup_scr_Set, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    case LV_EVENT_PRESSING:
    {
        lv_obj_t* obj = lv_event_get_target(e); //获取事件产生的对象
        lv_indev_t* indev = lv_indev_get_act();  //获取活动界面输入设备
        lv_point_t vect;
        lv_indev_get_vect(indev, &vect); //获取vect point
        lv_coord_t x = lv_obj_get_x(obj) + vect.x; //计算x
        move_distace=x;
        // 获取当前移动项的索引
        for(uint8_t i = 0; i < 7; i++) {
            if(list_1_obj[i] == obj) {
                current_moving_index = i;
                break;
            }
        }
        if(move_distace<100&&move_distace>0) {
            lv_obj_set_x(obj, x); //移动对象到x,y
        }
        if(move_distace>100) {
            lv_obj_clear_flag(obj,LV_OBJ_FLAG_CLICKABLE);
            lv_anim_init(&move_anim);
            lv_anim_set_var(&move_anim, obj);                      // 要移动的对象
            lv_anim_set_exec_cb(&move_anim, (lv_anim_exec_xcb_t)lv_obj_set_x); // 修改x坐标
            lv_anim_set_time(&move_anim,(lv_coord_t)1500);                      // 动画持续时间500ms
            lv_anim_set_values(&move_anim,lv_obj_get_x(obj), 240); // 从当前位置到240
            lv_anim_set_path_cb(&move_anim, lv_anim_path_overshoot); // 使用overshoot路径
            lv_anim_set_ready_cb(&move_anim,(lv_anim_ready_cb_t)clear_flage);
            lv_anim_start(&move_anim);
        }
        break;
    }
    default:
        break;
    }
}

static void List_1_img_7_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Set, guider_ui.Set_del, &guider_ui.List_1_del, setup_scr_Set, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_1_label_7_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Set, guider_ui.Set_del, &guider_ui.List_1_del, setup_scr_Set, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_1_btn_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        bool* visible_array[7] = {&is_List_visible_1, &is_List_visible_2, &is_List_visible_3,
                                  &is_List_visible_4, &is_List_visible_5, &is_List_visible_6, &is_List_visible_7
                                 };

        // 查找第一个被移除的项
        for(uint8_t i = 0; i < 7; i++) {
            if(!(*visible_array[i])) {
                // 重新显示该项
                *visible_array[i] = true;
                lv_obj_clear_flag(list_1_obj[i], LV_OBJ_FLAG_HIDDEN);

                // 设置初始位置并启动动画
                // lv_obj_set_y(list_1_obj[i], 610);
                lv_anim_t a;
                lv_anim_init(&a);
                lv_anim_set_var(&a, list_1_obj[i]);
                lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
                lv_anim_set_time(&a, 500);
                lv_anim_set_values(&a, lv_obj_get_x(list_1_obj[i]), 0);
                lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
                lv_anim_start(&a);

                for(uint8_t j = 0; j < 7; j++) {
                    if(!(*visible_array[j])) {
                        all_visible = false;
                        break;
                    }
                }
                if(all_visible) {
                    lv_obj_add_flag(list_1_obj[7], LV_OBJ_FLAG_HIDDEN);
                    is_List_visible_btn = false;
                }

            }
        }
        break;
    }
    default:
        break;
    }
}

void events_init_List_1 (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->List_1, List_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_btn_1, List_1_btn_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_cont_2, List_1_cont_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_img_1, List_1_img_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_label_1, List_1_label_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_cont_3, List_1_cont_3_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_img_2, List_1_img_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_label_2, List_1_label_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_cont_4, List_1_cont_4_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_img_3, List_1_img_3_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_label_3, List_1_label_3_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_cont_5, List_1_cont_5_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_img_4, List_1_img_4_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_label_4, List_1_label_4_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_cont_6, List_1_cont_6_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_img_5, List_1_img_5_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_label_5, List_1_label_5_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_cont_7, List_1_cont_7_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_cont_8, List_1_cont_8_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_img_7, List_1_img_7_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_label_7, List_1_label_7_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_1_btn_2, List_1_btn_2_event_handler, LV_EVENT_ALL, ui);
}

static void List_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCROLL:
    {
        lv_obj_t *scroll_container = lv_event_get_target(e);
        int32_t scroll_y = lv_obj_get_scroll_y(scroll_container); // 获取滚动偏移量
        if(scroll_y>40) {
            uint16_t get_y=(scroll_y-40);
            level=(get_y/70);
            uint16_t over_len=(get_y%70);
            uint16_t chage_width=(over_len*240)/70;
            switch(level) {
            case 0:
                lv_obj_set_width(guider_ui.List_2_cont_8,300-chage_width);
                lv_obj_set_width(guider_ui.List_2_cont_5,chage_width);
                lv_obj_set_width(guider_ui.List_2_cont_3,240);
                lv_obj_set_width(guider_ui.List_2_cont_4,240);
                lv_obj_set_width(guider_ui.List_2_cont_6,240);
                lv_obj_set_width(guider_ui.List_2_cont_7,240);

                // lv_obj_set_height(guider_ui.List_2_cont_8,90-chage_width);
                // lv_obj_set_height(guider_ui.List_2_cont_5,chage_width);
                // lv_obj_set_height(guider_ui.List_2_cont_3,70);
                // lv_obj_set_height(guider_ui.List_2_cont_4,70);
                // lv_obj_set_height(guider_ui.List_2_cont_6,70);
                // lv_obj_set_height(guider_ui.List_2_cont_7,70);

                break;
            case 1:
                lv_obj_set_width(guider_ui.List_2_cont_7,320-chage_width);
                lv_obj_set_width(guider_ui.List_2_cont_4,chage_width);
                lv_obj_set_width(guider_ui.List_2_cont_3,240);
                lv_obj_set_width(guider_ui.List_2_cont_5,240);
                lv_obj_set_width(guider_ui.List_2_cont_6,240);
                lv_obj_set_width(guider_ui.List_2_cont_8,240);

                // lv_obj_set_height(guider_ui.List_2_cont_7,90-chage_width);
                // lv_obj_set_height(guider_ui.List_2_cont_4,chage_width);
                // lv_obj_set_height(guider_ui.List_2_cont_3,70);
                // lv_obj_set_height(guider_ui.List_2_cont_5,70);
                // lv_obj_set_height(guider_ui.List_2_cont_6,70);
                // lv_obj_set_height(guider_ui.List_2_cont_8,70);
                break;
            case 2:
                lv_obj_set_width(guider_ui.List_2_cont_6,320-chage_width);
                lv_obj_set_width(guider_ui.List_2_cont_3,chage_width+30);
                lv_obj_set_width(guider_ui.List_2_cont_5,240);
                lv_obj_set_width(guider_ui.List_2_cont_4,240);
                lv_obj_set_width(guider_ui.List_2_cont_8,240);
                lv_obj_set_width(guider_ui.List_2_cont_7,240);

                // lv_obj_set_height(guider_ui.List_2_cont_6,90-chage_width);
                // lv_obj_set_height(guider_ui.List_2_cont_3,chage_width);
                // lv_obj_set_height(guider_ui.List_2_cont_4,70);
                // lv_obj_set_height(guider_ui.List_2_cont_5,70);
                // lv_obj_set_height(guider_ui.List_2_cont_7,70);
                // lv_obj_set_height(guider_ui.List_2_cont_8,70);
                break;
            case 3:


                if((360-chage_width)>240) {
                    lv_obj_set_width(guider_ui.List_2_cont_5,240);
                } else {
                    lv_obj_set_width(guider_ui.List_2_cont_5,360-chage_width);
                }

                lv_obj_set_width(guider_ui.List_2_cont_2,chage_width+30);

                lv_obj_set_width(guider_ui.List_2_cont_3,240);
                lv_obj_set_width(guider_ui.List_2_cont_4,240);
                lv_obj_set_width(guider_ui.List_2_cont_6,240);
                lv_obj_set_width(guider_ui.List_2_cont_7,240);
                break;
            case 4:
                lv_obj_set_width(guider_ui.List_2_cont_2,chage_width+52);
                // lv_obj_set_height(guider_ui.List_2_cont_2,chage_width);
                break;
            default:
                break;

            }

        }







        break;
    }
    case LV_EVENT_SCROLL_END:
    {

        switch(level) {
        case 0:
            lv_obj_scroll_to_y(guider_ui.List_2,0,LV_ANIM_ON);
            break;
        case 1:
            lv_obj_scroll_to_y(guider_ui.List_2,120,LV_ANIM_ON);
            break;
        case 2:
            lv_obj_scroll_to_y(guider_ui.List_2,200,LV_ANIM_ON);
            break;
        case 3:
            lv_obj_scroll_to_y(guider_ui.List_2,280,LV_ANIM_ON);
            break;
        case 4:
            lv_obj_scroll_to_y(guider_ui.List_2,380,LV_ANIM_ON);
            break;
        }
        break;
    }
    default:
        break;
    }
}

static void List_2_btn_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        switch (screen_index)
        {
        case 0:
            /* code */
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.Clock_1, guider_ui.Clock_1_del, &guider_ui.List_2_del, setup_scr_Clock_1, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 200, true, true);
            break;
        case 1:
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.Clock_2, guider_ui.Clock_2_del, &guider_ui.List_2_del, setup_scr_Clock_2, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 200, true, true);
            break;
        case 2:
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.Clock_3, guider_ui.Clock_3_del, &guider_ui.List_2_del, setup_scr_Clock_3, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 200, true, true);
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

static void List_2_cont_8_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Heart, guider_ui.Heart_del, &guider_ui.List_2_del, setup_scr_Heart, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_img_7_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Heart, guider_ui.Heart_del, &guider_ui.List_2_del, setup_scr_Heart, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_label_7_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Heart, guider_ui.Heart_del, &guider_ui.List_2_del, setup_scr_Heart, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_cont_7_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Map, guider_ui.Map_del, &guider_ui.List_2_del, setup_scr_Map, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_img_6_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Map, guider_ui.Map_del, &guider_ui.List_2_del, setup_scr_Map, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_label_6_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Map, guider_ui.Map_del, &guider_ui.List_2_del, setup_scr_Map, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_cont_6_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.NFC, guider_ui.NFC_del, &guider_ui.List_2_del, setup_scr_NFC, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_img_5_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.NFC, guider_ui.NFC_del, &guider_ui.List_2_del, setup_scr_NFC, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_label_5_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.NFC, guider_ui.NFC_del, &guider_ui.List_2_del, setup_scr_NFC, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_cont_5_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.QRcode, guider_ui.QRcode_del, &guider_ui.List_2_del, setup_scr_QRcode, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_img_4_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.QRcode, guider_ui.QRcode_del, &guider_ui.List_2_del, setup_scr_QRcode, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_label_4_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.QRcode, guider_ui.QRcode_del, &guider_ui.List_2_del, setup_scr_QRcode, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_cont_4_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Systeamupdate, guider_ui.Systeamupdate_del, &guider_ui.List_2_del, setup_scr_Systeamupdate, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_img_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Systeamupdate, guider_ui.Systeamupdate_del, &guider_ui.List_2_del, setup_scr_Systeamupdate, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_label_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Systeamupdate, guider_ui.Systeamupdate_del, &guider_ui.List_2_del, setup_scr_Systeamupdate, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_cont_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_PRESSING:
    {

        break;
    }
    default:
        break;
    }
}

static void List_2_cont_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Set, guider_ui.Set_del, &guider_ui.List_2_del, setup_scr_Set, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    case LV_EVENT_PRESSING:
    {

        break;
    }
    default:
        break;
    }
}

static void List_2_img_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Set, guider_ui.Set_del, &guider_ui.List_2_del, setup_scr_Set, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void List_2_label_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SHORT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Set, guider_ui.Set_del, &guider_ui.List_2_del, setup_scr_Set, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

void events_init_List_2 (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->List_2, List_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_btn_2, List_2_btn_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_cont_8, List_2_cont_8_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_img_7, List_2_img_7_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_label_7, List_2_label_7_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_cont_7, List_2_cont_7_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_img_6, List_2_img_6_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_label_6, List_2_label_6_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_cont_6, List_2_cont_6_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_img_5, List_2_img_5_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_label_5, List_2_label_5_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_cont_5, List_2_cont_5_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_img_4, List_2_img_4_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_label_4, List_2_label_4_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_cont_4, List_2_cont_4_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_img_3, List_2_img_3_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_label_3, List_2_label_3_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_cont_3, List_2_cont_3_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_cont_2, List_2_cont_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_img_1, List_2_img_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_2_label_1, List_2_label_1_event_handler, LV_EVENT_ALL, ui);
}

static void List_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        scroll_obj[0]=guider_ui.List_3_img_1;
        scroll_obj[1]=guider_ui.List_3_img_2;
        scroll_obj[2]=guider_ui.List_3_img_4;
        scroll_obj[3]=guider_ui.List_3_img_5;
        scroll_obj[4]=guider_ui.List_3_img_6;
        scroll_obj[5]=guider_ui.List_3_img_7;
        scroll_obj[6]=guider_ui.List_3_img_3;
        break;
    }
    default:
        break;
    }
}

static void List_3_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        switch (screen_index)
        {
        case 0:
            /* code */
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.Clock_1, guider_ui.Clock_1_del, &guider_ui.List_3_del, setup_scr_Clock_1, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 200, true, true);
            break;
        case 1:
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.Clock_2, guider_ui.Clock_2_del, &guider_ui.List_3_del, setup_scr_Clock_2, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 200, true, true);
            break;
        case 2:
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.Clock_3, guider_ui.Clock_3_del, &guider_ui.List_3_del, setup_scr_Clock_3, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 200, true, true);
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

static void List_3_cont_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_PRESSED:
    {
        lv_indev_t *indev = lv_indev_get_act();
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        start_point.x=point.x;
        start_point.y=point.y;


        //move layer
        lv_obj_move_to_index(guider_ui.List_3_btn_2,11);

        is_pissing =1;
        break;
    }
    case LV_EVENT_PRESSING:
    {
        lv_indev_t *indev = lv_indev_get_act();
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        int16_t dx=(point.x-start_point.x);
        int16_t dy=(point.y-start_point.y);

        if(LV_ABS(dx)>20||LV_ABS(dy)>20) {
            if(is_pissing) {
                switch(get_touch_quadrant()) {
                case 1:
                    if ((dx ^ dy) >= 0) {
                        if(dx>0) {
                            for(int i=0; i<menu_3_cnt; i++) {

                                if(i==6) {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[0].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[0].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);

                                    // 执行反向位移操作
                                    lv_obj_t* temp = scroll_obj[menu_3_cnt - 1];
                                    for (int i = menu_3_cnt - 2; i >= 0; i--) {
                                        scroll_obj[i + 1] = scroll_obj[i];
                                    }
                                    scroll_obj[0] = temp;

                                } else {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[i+1].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[i+1].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);
                                }


                            }
                        } else {
                            for(int i=0; i<menu_3_cnt; i++) {
                                if(i==0) {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[6].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[6].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);

                                } else {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[i-1].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[i-1].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);
                                    if(i==6) {
                                        lv_obj_t* temp = scroll_obj[0];
                                        for (int i = 0; i < menu_3_cnt - 1; i++) {
                                            scroll_obj[i] = scroll_obj[i + 1];
                                        }
                                        scroll_obj[menu_3_cnt - 1] = temp;
                                    }
                                }
                            }

                        }


                    }
                    break;
                case 2:
                    if ((dx ^ dy) < 0) {
                        if(dx>0) {
                            for(int i=0; i<menu_3_cnt; i++) {

                                if(i==6) {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[0].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[0].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);

                                    // 执行反向位移操作
                                    lv_obj_t* temp = scroll_obj[menu_3_cnt - 1];
                                    for (int i = menu_3_cnt - 2; i >= 0; i--) {
                                        scroll_obj[i + 1] = scroll_obj[i];
                                    }
                                    scroll_obj[0] = temp;

                                } else {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[i+1].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[i+1].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);
                                }


                            }
                        } else {
                            for(int i=0; i<menu_3_cnt; i++) {
                                if(i==0) {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[6].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[6].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);

                                } else {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[i-1].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[i-1].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);
                                    if(i==6) {
                                        lv_obj_t* temp = scroll_obj[0];
                                        for (int i = 0; i < menu_3_cnt - 1; i++) {
                                            scroll_obj[i] = scroll_obj[i + 1];
                                        }
                                        scroll_obj[menu_3_cnt - 1] = temp;
                                    }
                                }
                            }

                        }


                    }
                    break;
                case 3:

                    if ((dx ^ dy) >= 0) {
                        if(dx>0) {
                            for(int i=0; i<menu_3_cnt; i++) {
                                if(i==0) {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[6].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[6].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);

                                } else {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[i-1].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[i-1].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);
                                    if(i==6) {
                                        lv_obj_t* temp = scroll_obj[0];
                                        for (int i = 0; i < menu_3_cnt - 1; i++) {
                                            scroll_obj[i] = scroll_obj[i + 1];
                                        }
                                        scroll_obj[menu_3_cnt - 1] = temp;
                                    }
                                }
                            }
                        } else {
                            for(int i=0; i<menu_3_cnt; i++) {

                                if(i==6) {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[0].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[0].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);

                                    // 执行反向位移操作
                                    lv_obj_t* temp = scroll_obj[menu_3_cnt - 1];
                                    for (int i = menu_3_cnt - 2; i >= 0; i--) {
                                        scroll_obj[i + 1] = scroll_obj[i];
                                    }
                                    scroll_obj[0] = temp;

                                } else {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[i+1].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[i+1].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);
                                }


                            }

                        }


                    }
                    break;
                case 4:


                    if ((dx ^ dy) < 0) {
                        if(dx>0) {
                            for(int i=0; i<menu_3_cnt; i++) {
                                if(i==0) {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[6].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[6].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);

                                } else {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[i-1].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[i-1].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);
                                    if(i==6) {
                                        lv_obj_t* temp = scroll_obj[0];
                                        for (int i = 0; i < menu_3_cnt - 1; i++) {
                                            scroll_obj[i] = scroll_obj[i + 1];
                                        }
                                        scroll_obj[menu_3_cnt - 1] = temp;
                                    }
                                }
                            }
                        } else {
                            for(int i=0; i<menu_3_cnt; i++) {

                                if(i==6) {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[0].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[0].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);

                                    // 执行反向位移操作
                                    lv_obj_t* temp = scroll_obj[menu_3_cnt - 1];
                                    for (int i = menu_3_cnt - 2; i >= 0; i--) {
                                        scroll_obj[i + 1] = scroll_obj[i];
                                    }
                                    scroll_obj[0] = temp;

                                } else {
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].x,initial_points[i+1].x,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_x, NULL, NULL, NULL);
                                    ui_animation(scroll_obj[i],500,0,initial_points[i].y,initial_points[i+1].y,&lv_anim_path_overshoot,0,0,0,0, (lv_anim_exec_xcb_t)lv_obj_set_y, NULL, NULL, NULL);
                                }


                            }

                        }


                    }
                    break;
                }
                is_pissing=0;
            }

        }







        break;
    }
    case LV_EVENT_PRESS_LOST:
    {
        lv_obj_move_to_index(guider_ui.List_3_btn_2,12);
        break;
    }
    default:
        break;
    }
}

static void List_3_btn_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {

        if(scroll_obj[0]==guider_ui.List_3_img_1) {
            ui_load_scr_animation(&guider_ui, &guider_ui.Heart, guider_ui.Heart_del, &guider_ui.List_3_del, setup_scr_Heart, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        }
        if(scroll_obj[0]==guider_ui.List_3_img_4) {
            ui_load_scr_animation(&guider_ui, &guider_ui.Set, guider_ui.Set_del, &guider_ui.List_3_del, setup_scr_Set, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        }
        if(scroll_obj[0]==guider_ui.List_3_img_5) {
            ui_load_scr_animation(&guider_ui, &guider_ui.Systeamupdate, guider_ui.Systeamupdate_del, &guider_ui.List_3_del, setup_scr_Systeamupdate, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        }
        if(scroll_obj[0]==guider_ui.List_3_img_6) {
            ui_load_scr_animation(&guider_ui, &guider_ui.NFC, guider_ui.NFC_del, &guider_ui.List_3_del, setup_scr_NFC, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        }
        if(scroll_obj[0]==guider_ui.List_3_img_7) {
            ui_load_scr_animation(&guider_ui, &guider_ui.Map, guider_ui.Map_del, &guider_ui.List_3_del, setup_scr_Map, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        }
        if(scroll_obj[0]==guider_ui.List_3_img_3) {
            ui_load_scr_animation(&guider_ui, &guider_ui.QRcode, guider_ui.QRcode_del, &guider_ui.List_3_del, setup_scr_QRcode, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        }
        break;
    }
    default:
        break;
    }
}

void events_init_List_3 (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->List_3, List_3_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_3_btn_1, List_3_btn_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_3_cont_1, List_3_cont_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->List_3_btn_2, List_3_btn_2_event_handler, LV_EVENT_ALL, ui);
}

static void Heart_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_anim_init(&screen_loaded_anim);


        /* 设置动画目标为cont控件 */
        lv_anim_set_var(&screen_loaded_anim,guider_ui.Heart_cont_1 );

        /* 使用Y轴坐标设置函数 */
        lv_anim_set_exec_cb(&screen_loaded_anim, (lv_anim_exec_xcb_t)lv_obj_set_y);

        /* 动画参数配置 */
        lv_anim_set_time(&screen_loaded_anim, 500); // 持续500ms
        lv_anim_set_values(&screen_loaded_anim, lv_obj_get_y(guider_ui.Heart_cont_1), 40);
        lv_anim_set_path_cb(&screen_loaded_anim, lv_anim_path_overshoot); // 线性运动

        /* 启动动画 */
        lv_anim_start(&screen_loaded_anim);
        break;
    }
    default:
        break;
    }
}

static void Heart_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {

        switch(mode) {
        case 0:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_1, guider_ui.List_1_del, &guider_ui.Set_del, setup_scr_List_1, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        case 1:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_2, guider_ui.List_2_del, &guider_ui.Set_del, setup_scr_List_2, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        case 2:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_3, guider_ui.List_3_del, &guider_ui.Set_del, setup_scr_List_3, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        }
        break;
    }
    default:
        break;
    }
}

void events_init_Heart (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->Heart, Heart_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Heart_btn_1, Heart_btn_1_event_handler, LV_EVENT_ALL, ui);
}

static void Map_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {

        break;
    }
    default:
        break;
    }
}

static void Map_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {

        switch(mode) {
        case 0:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_1, guider_ui.List_1_del, &guider_ui.Set_del, setup_scr_List_1, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        case 1:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_2, guider_ui.List_2_del, &guider_ui.Set_del, setup_scr_List_2, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        case 2:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_3, guider_ui.List_3_del, &guider_ui.Set_del, setup_scr_List_3, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        }
        break;
    }
    default:
        break;
    }
}

void events_init_Map (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->Map, Map_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Map_btn_1, Map_btn_1_event_handler, LV_EVENT_ALL, ui);
}

static void NFC_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {

        break;
    }
    default:
        break;
    }
}

static void NFC_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {

        switch(mode) {
        case 0:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_1, guider_ui.List_1_del, &guider_ui.Set_del, setup_scr_List_1, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        case 1:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_2, guider_ui.List_2_del, &guider_ui.Set_del, setup_scr_List_2, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        case 2:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_3, guider_ui.List_3_del, &guider_ui.Set_del, setup_scr_List_3, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        }
        break;
    }
    default:
        break;
    }
}

void events_init_NFC (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->NFC, NFC_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->NFC_btn_1, NFC_btn_1_event_handler, LV_EVENT_ALL, ui);
}

static void QRcode_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {

        break;
    }
    default:
        break;
    }
}

static void QRcode_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {

        switch(mode) {
        case 0:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_1, guider_ui.List_1_del, &guider_ui.Set_del, setup_scr_List_1, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        case 1:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_2, guider_ui.List_2_del, &guider_ui.Set_del, setup_scr_List_2, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        case 2:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_3, guider_ui.List_3_del, &guider_ui.Set_del, setup_scr_List_3, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        }
        break;
    }
    default:
        break;
    }
}

void events_init_QRcode (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->QRcode, QRcode_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->QRcode_btn_1, QRcode_btn_1_event_handler, LV_EVENT_ALL, ui);
}

static void Systeamupdate_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {

        break;
    }
    default:
        break;
    }
}

static void Systeamupdate_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {

        switch(mode) {
        case 0:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_1, guider_ui.List_1_del, &guider_ui.Set_del, setup_scr_List_1, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        case 1:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_2, guider_ui.List_2_del, &guider_ui.Set_del, setup_scr_List_2, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        case 2:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_3, guider_ui.List_3_del, &guider_ui.Set_del, setup_scr_List_3, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        }
        break;
    }
    default:
        break;
    }
}

static void Systeamupdate_btn_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Systeamupdate_cheak, guider_ui.Systeamupdate_cheak_del, &guider_ui.Systeamupdate_del, setup_scr_Systeamupdate_cheak, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

void events_init_Systeamupdate (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->Systeamupdate, Systeamupdate_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Systeamupdate_btn_1, Systeamupdate_btn_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Systeamupdate_btn_2, Systeamupdate_btn_2_event_handler, LV_EVENT_ALL, ui);
}

static void Systeamupdate_cheak_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {

        break;
    }
    default:
        break;
    }
}

static void Systeamupdate_cheak_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.Systeamupdate, guider_ui.Systeamupdate_del, &guider_ui.Systeamupdate_cheak_del, setup_scr_Systeamupdate, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void Systeamupdate_cheak_btn_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {

        break;
    }
    default:
        break;
    }
}

void events_init_Systeamupdate_cheak (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->Systeamupdate_cheak, Systeamupdate_cheak_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Systeamupdate_cheak_btn_1, Systeamupdate_cheak_btn_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Systeamupdate_cheak_btn_2, Systeamupdate_cheak_btn_2_event_handler, LV_EVENT_ALL, ui);
}

static void Set_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {

        switch (mode) {
        case (0):
        {
            lv_obj_set_style_bg_color(guider_ui.Set_cont_6, lv_color_hex(0xffffff), LV_PART_MAIN);
            lv_obj_set_style_border_color(guider_ui.Set_cont_6, lv_color_hex(0x686868), LV_PART_MAIN);
            lv_obj_set_style_bg_color(guider_ui.Set_cont_4, lv_color_hex(0xffffff), LV_PART_MAIN);
            lv_obj_set_style_border_color(guider_ui.Set_cont_4, lv_color_hex(0x686868), LV_PART_MAIN);
            lv_obj_set_style_bg_color(guider_ui.Set_cont_2, lv_color_hex(0x00ff04), LV_PART_MAIN);
            lv_obj_set_style_border_color(guider_ui.Set_cont_2, lv_color_hex(0x076600), LV_PART_MAIN);
            break;
        }
        case (1):
        {
            lv_obj_set_style_bg_color(guider_ui.Set_cont_6, lv_color_hex(0xffffff), LV_PART_MAIN);
            lv_obj_set_style_border_color(guider_ui.Set_cont_6, lv_color_hex(0x686868), LV_PART_MAIN);
            lv_obj_set_style_bg_color(guider_ui.Set_cont_4, lv_color_hex(0x37ff00), LV_PART_MAIN);
            lv_obj_set_style_border_color(guider_ui.Set_cont_4, lv_color_hex(0x036f01), LV_PART_MAIN);
            lv_obj_set_style_bg_color(guider_ui.Set_cont_2, lv_color_hex(0xffffff), LV_PART_MAIN);
            lv_obj_set_style_border_color(guider_ui.Set_cont_2, lv_color_hex(0x686868), LV_PART_MAIN);
            break;
        }
        case (2):
        {
            lv_obj_set_style_bg_color(guider_ui.Set_cont_6, lv_color_hex(0x2bff00), LV_PART_MAIN);
            lv_obj_set_style_border_color(guider_ui.Set_cont_6, lv_color_hex(0x007a02), LV_PART_MAIN);
            lv_obj_set_style_bg_color(guider_ui.Set_cont_4, lv_color_hex(0xffffff), LV_PART_MAIN);
            lv_obj_set_style_border_color(guider_ui.Set_cont_4, lv_color_hex(0x686868), LV_PART_MAIN);
            lv_obj_set_style_bg_color(guider_ui.Set_cont_2, lv_color_hex(0xffffff), LV_PART_MAIN);
            lv_obj_set_style_border_color(guider_ui.Set_cont_2, lv_color_hex(0x686868), LV_PART_MAIN);
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

static void Set_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {

        switch(mode) {
        case 0:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_1, guider_ui.List_1_del, &guider_ui.Set_del, setup_scr_List_1, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        case 1:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_2, guider_ui.List_2_del, &guider_ui.Set_del, setup_scr_List_2, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        case 2:
            ui_load_scr_animation(&guider_ui, &guider_ui.List_3, guider_ui.List_3_del, &guider_ui.Set_del, setup_scr_List_3, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
            break;
        }
        break;
    }
    default:
        break;
    }
}

static void Set_cont_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        mode=0;
        lv_obj_set_style_bg_color(guider_ui.Set_cont_6, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_set_style_border_color(guider_ui.Set_cont_6, lv_color_hex(0x686868), LV_PART_MAIN);
        lv_obj_set_style_bg_color(guider_ui.Set_cont_4, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_set_style_border_color(guider_ui.Set_cont_4, lv_color_hex(0x686868), LV_PART_MAIN);
        lv_obj_set_style_bg_color(guider_ui.Set_cont_2, lv_color_hex(0x04ff00), LV_PART_MAIN);
        lv_obj_set_style_border_color(guider_ui.Set_cont_2, lv_color_hex(0x185c00), LV_PART_MAIN);
        break;
    }
    default:
        break;
    }
}

static void Set_cont_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        mode=1;
        lv_obj_set_style_bg_color(guider_ui.Set_cont_6, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_set_style_border_color(guider_ui.Set_cont_6, lv_color_hex(0x686868), LV_PART_MAIN);
        lv_obj_set_style_bg_color(guider_ui.Set_cont_4, lv_color_hex(0x11ff00), LV_PART_MAIN);
        lv_obj_set_style_border_color(guider_ui.Set_cont_4, lv_color_hex(0x166600), LV_PART_MAIN);
        lv_obj_set_style_bg_color(guider_ui.Set_cont_2, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_set_style_border_color(guider_ui.Set_cont_2, lv_color_hex(0x686868), LV_PART_MAIN);
        break;
    }
    default:
        break;
    }
}

static void Set_cont_5_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        mode=2;
        lv_obj_set_style_bg_color(guider_ui.Set_cont_6, lv_color_hex(0x1eff00), LV_PART_MAIN);
        lv_obj_set_style_border_color(guider_ui.Set_cont_6, lv_color_hex(0x0b6100), LV_PART_MAIN);
        lv_obj_set_style_bg_color(guider_ui.Set_cont_4, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_set_style_border_color(guider_ui.Set_cont_4, lv_color_hex(0x686868), LV_PART_MAIN);
        lv_obj_set_style_bg_color(guider_ui.Set_cont_2, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_set_style_border_color(guider_ui.Set_cont_2, lv_color_hex(0x686868), LV_PART_MAIN);
        break;
    }
    default:
        break;
    }
}

void events_init_Set (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->Set, Set_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Set_btn_1, Set_btn_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Set_cont_1, Set_cont_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Set_cont_3, Set_cont_3_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Set_cont_5, Set_cont_5_event_handler, LV_EVENT_ALL, ui);
}


void events_init(lv_ui *ui)
{

}

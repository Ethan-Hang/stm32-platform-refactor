/******************************************************************************
 * @file lv_port_indev.c
 *
 * @par dependencies
 * - lvgl.h
 * - bsp_cst816t_driver.h
 *
 * @author Ethan-Hang
 *
 * @brief LVGL pointer indev port over the CST816T touch driver.
 *
 * Processing flow:
 *   LVGL polls indev_read_cb every LV_INDEV_DEF_READ_PERIOD ms (10 ms by
 *   default) -> we read FingerNum + X/Y over I2C and translate to
 *   lv_indev_data_t {state=PRESSED|RELEASED, point}.  CST816T's gesture
 *   register is not consumed here — gesture decoding is left to the app
 *   on top of LVGL's own click/long-press/swipe events.
 *
 * @version V1.0 2026-04-25
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "lv_port_indev.h"

#include <stddef.h>

#include "lvgl.h"
#include "Debug.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define LV_PORT_INDEV_PANEL_WIDTH       240u
#define LV_PORT_INDEV_PANEL_HEIGHT      280u
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
static bsp_cst816t_driver_t  *s_p_driver = NULL;
static lv_indev_drv_t         s_indev_drv;
static lv_point_t             s_last_point = {0, 0};
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
/**
 * @brief      LVGL indev read callback.  Polled every 10 ms by lv_timer.
 *
 * @param[in]  drv  : Indev driver pointer (unused).
 *
 * @param[out] data : Output state + coordinates LVGL feeds into widgets.
 *
 * @return     None.
 *
 * @note       Holds the last reported coordinate while the finger is up so
 *             LVGL receives a clean PRESSED -> RELEASED transition at the
 *             same point, which makes click / long-press recognition work.
 * */
static void lv_port_indev_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    (void)drv;

    if ((NULL == s_p_driver) ||
        (NULL == s_p_driver->pf_cst816t_get_finger_num) ||
        (NULL == s_p_driver->pf_cst816t_get_xy_axis))
    {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }

    uint8_t finger_num = 0u;
    cst816t_status_t ret =
        s_p_driver->pf_cst816t_get_finger_num(*s_p_driver, &finger_num);
    if (CST816T_OK != ret)
    {
        data->state = LV_INDEV_STATE_RELEASED;
        data->point = s_last_point;
        return;
    }

    if (0u == finger_num)
    {
        data->state = LV_INDEV_STATE_RELEASED;
        data->point = s_last_point;
        return;
    }

    cst816t_xy_t xy = {0u, 0u};
    ret = s_p_driver->pf_cst816t_get_xy_axis(*s_p_driver, &xy);
    if (CST816T_OK != ret)
    {
        data->state = LV_INDEV_STATE_RELEASED;
        data->point = s_last_point;
        return;
    }

    /**
     * Clamp to panel rectangle so a stray off-edge sample (CST816T can
     * occasionally report values just past the active area) does not
     * land inside an invalid widget hit-test.
     **/
    if (xy.x_pos >= LV_PORT_INDEV_PANEL_WIDTH)
    {
        xy.x_pos = (uint16_t)(LV_PORT_INDEV_PANEL_WIDTH - 1u);
    }
    if (xy.y_pos >= LV_PORT_INDEV_PANEL_HEIGHT)
    {
        xy.y_pos = (uint16_t)(LV_PORT_INDEV_PANEL_HEIGHT - 1u);
    }

    s_last_point.x = (lv_coord_t)xy.x_pos;
    s_last_point.y = (lv_coord_t)xy.y_pos;
    data->state    = LV_INDEV_STATE_PRESSED;
    data->point    = s_last_point;
}

bool lv_port_indev_init(bsp_cst816t_driver_t *p_driver)
{
    if ((NULL == p_driver) ||
        (NULL == p_driver->pf_cst816t_get_finger_num) ||
        (NULL == p_driver->pf_cst816t_get_xy_axis))
    {
        return false;
    }

    s_p_driver = p_driver;

    lv_indev_drv_init(&s_indev_drv);
    s_indev_drv.type    = LV_INDEV_TYPE_POINTER;
    s_indev_drv.read_cb = lv_port_indev_read_cb;

    lv_indev_t *indev = lv_indev_drv_register(&s_indev_drv);
    return (NULL != indev);
}
//******************************* Functions *********************************//

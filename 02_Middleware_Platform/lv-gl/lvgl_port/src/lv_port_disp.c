/******************************************************************************
 * @file lv_port_disp.c
 *
 * @par dependencies
 * - lvgl.h
 * - bsp_st7789_driver.h
 *
 * @author Ethan-Hang
 *
 * @brief LVGL display port over the ST7789 driver.
 *
 * Processing flow:
 *   lv_disp_drv_t.flush_cb -> ST7789 driver pf_st7789_draw_image -> SPI DMA
 *
 * A single partial draw buffer (one panel-width x 20 lines of RGB565) sits in
 * .bss.  When LVGL asks for a flush, we forward the rectangle straight into
 * the driver; the driver swaps RGB565 into big-endian on its own scratch tile
 * before DMA, so lv_color_t (host-endian RGB565) can be passed through as a
 * plain uint16_t* without byte swapping.
 *
 * @version V1.0 2026-04-24
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "lv_port_disp.h"

#include <stddef.h>
#include <string.h>

#include "lvgl.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define LV_PORT_DISP_HOR_RES        240U
#define LV_PORT_DISP_VER_RES        280U
#define LV_PORT_DISP_BUF_LINES      20U
#define LV_PORT_DISP_BUF_PIXELS     (LV_PORT_DISP_HOR_RES * LV_PORT_DISP_BUF_LINES)
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
static bsp_st7789_driver_t * s_p_driver = NULL;
static lv_disp_draw_buf_t    s_draw_buf;
static lv_disp_drv_t         s_disp_drv;
static lv_color_t            s_buf1[LV_PORT_DISP_BUF_PIXELS];
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
static void lv_port_flush_cb(lv_disp_drv_t *  disp_drv,
                             const lv_area_t *area,
                             lv_color_t *     color_p)
{
    if ((NULL == s_p_driver) ||
        (NULL == s_p_driver->pf_st7789_draw_image))
    {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    const uint16_t x     = (uint16_t)area->x1;
    const uint16_t y     = (uint16_t)area->y1;
    const uint16_t width = (uint16_t)(area->x2 - area->x1 + 1);
    const uint16_t height = (uint16_t)(area->y2 - area->y1 + 1);

    /* lv_color_t is a 16-bit RGB565 union whose .full matches the host-endian
     * uint16_t the driver expects.  The memory layout is identical so we can
     * cast the buffer directly. */
    (void)s_p_driver->pf_st7789_draw_image(s_p_driver,
                                           x, y,
                                           width, height,
                                           (uint16_t const *)color_p);

    lv_disp_flush_ready(disp_drv);
}

bool lv_port_disp_init(bsp_st7789_driver_t *p_driver)
{
    if ((NULL == p_driver) || (NULL == p_driver->pf_st7789_draw_image))
    {
        return false;
    }

    s_p_driver = p_driver;

    lv_disp_draw_buf_init(&s_draw_buf, s_buf1, NULL, LV_PORT_DISP_BUF_PIXELS);

    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res  = LV_PORT_DISP_HOR_RES;
    s_disp_drv.ver_res  = LV_PORT_DISP_VER_RES;
    s_disp_drv.flush_cb = lv_port_flush_cb;
    s_disp_drv.draw_buf = &s_draw_buf;

    lv_disp_t *disp = lv_disp_drv_register(&s_disp_drv);
    return (NULL != disp);
}
//******************************* Functions *********************************//

/******************************************************************************
 * @file lv_port_disp.c
 *
 * @par dependencies
 * - lvgl.h
 * - bsp_wrapper_display.h
 *
 * @author Ethan-Hang
 *
 * @brief LVGL display port over the bsp_wrapper_display abstraction.
 *
 * Processing flow:
 *   lv_disp_drv_t.flush_cb -> display_flush_async() -> single SPI DMA
 *   -> TX-complete ISR -> lv_disp_flush_ready().
 *
 * Two partial draw buffers (one panel-width x 20 lines of RGB565 each) sit
 * in .bss.  LVGL renders the next chunk into one buffer while the previous
 * chunk shifts out of the other over SPI DMA, so render and transfer
 * overlap.  LV_COLOR_16_SWAP=1 makes LVGL render in panel byte order, so
 * the buffer is DMA'd as-is with no per-pixel swap copy.
 *
 * @version V1.0 2026-04-24
 * @version V2.0 2026-04-26
 * @version V3.0 2026-06-10
 * @upgrade 2.0: Routed flush callback through bsp_wrapper_display so the
 *               LVGL port no longer depends on the ST7789 driver type.
 * @upgrade 3.0: Double buffer + display_flush_async: zero-copy DMA flush,
 *               completion signalled from the TX-DMA ISR.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "lv_port_disp.h"

#include "board_types.h"
#include <string.h>

#include "lvgl.h"
#include "bsp_wrapper_display.h"
#include "Debug.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define LV_PORT_DISP_HOR_RES        240U
#define LV_PORT_DISP_VER_RES        284U
#define LV_PORT_DISP_BUF_LINES      20U
#define LV_PORT_DISP_BUF_PIXELS     (LV_PORT_DISP_HOR_RES * LV_PORT_DISP_BUF_LINES)
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
static lv_disp_draw_buf_t    s_draw_buf;
static lv_disp_drv_t         s_disp_drv;
static lv_color_t            s_buf1[LV_PORT_DISP_BUF_PIXELS];
static lv_color_t            s_buf2[LV_PORT_DISP_BUF_PIXELS];
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
/**
 * @brief Async flush completion, called from the SPI TX-DMA-complete ISR.
 *        Hands the just-transferred buffer back to LVGL (flag write only,
 *        ISR-safe) so rendering of the next chunk can start immediately.
 *
 * @param[in] arg : The lv_disp_drv_t that issued the flush.
 *
 * @return None.
 */
static void lv_port_flush_done_cb(void *arg)
{
    lv_disp_flush_ready((lv_disp_drv_t *)arg);
}

/**
 * @brief LVGL flush callback.  Dispatches the dirty rectangle as one DMA
 *        transfer straight from the LVGL draw buffer and returns without
 *        waiting; lv_disp_flush_ready fires from the DMA-complete ISR.
 *
 *        With LV_COLOR_16_SWAP=1 lv_color_t bytes are already in panel
 *        order, matching display_flush_async's contract; the cast is
 *        layout-compatible.
 *
 * @param[in] disp_drv  : LVGL display driver context.
 * @param[in] area      : Dirty rectangle in screen coordinates.
 * @param[in] color_p   : Pixel buffer for that rectangle.
 */
static void lv_port_flush_cb(lv_disp_drv_t *  disp_drv,
                             const lv_area_t *area,
                             lv_color_t *     color_p)
{
    const UINT16_t x      = (UINT16_t)area->x1;
    const UINT16_t y      = (UINT16_t)area->y1;
    const UINT16_t width  = (UINT16_t)(area->x2 - area->x1 + 1);
    const UINT16_t height = (UINT16_t)(area->y2 - area->y1 + 1);

    platform_err_t ret = display_flush_async(x, y, width, height,
                                             (UINT16_t const *)color_p,
                                             lv_port_flush_done_cb,
                                             disp_drv);
    if (PLATFORM_IS_ERR(ret))
    {
        /**
         * Dispatch failed: done_cb will never fire, so release the buffer
         * here (dropping this chunk) to keep LVGL's refresh loop alive.
         **/
        DEBUG_OUT(e, LVGL_LOG_TAG, "flush_async dispatch failed = %d",
                  (int)ret);
        lv_disp_flush_ready(disp_drv);
    }
}

BOOL lv_port_disp_init(void)
{
    lv_disp_draw_buf_init(&s_draw_buf, s_buf1, s_buf2,
                          LV_PORT_DISP_BUF_PIXELS);

    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res  = LV_PORT_DISP_HOR_RES;
    s_disp_drv.ver_res  = LV_PORT_DISP_VER_RES;
    s_disp_drv.flush_cb = lv_port_flush_cb;
    s_disp_drv.draw_buf = &s_draw_buf;

    lv_disp_t *disp = lv_disp_drv_register(&s_disp_drv);
    return (NULL != disp);
}
//******************************* Functions *********************************//

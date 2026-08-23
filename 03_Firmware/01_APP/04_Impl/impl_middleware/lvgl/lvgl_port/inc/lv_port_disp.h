/******************************************************************************
 * @file lv_port_disp.h
 *
 * @par dependencies
 * - lvgl.h
 * - bsp_wrapper_display.h
 *
 * @author Ethan-Hang
 *
 * @brief LVGL display port over the bsp_wrapper_display abstraction.
 *
 * @version V1.0 2026-04-24
 * @version V2.0 2026-04-26
 * @upgrade 2.0: Decoupled from bsp_st7789_driver.  Flush callback now calls
 *               display_draw_image() so the LVGL port has no compile-time
 *               dependency on a specific driver.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __LV_PORT_DISP_H__
#define __LV_PORT_DISP_H__

//******************************** Includes *********************************//
#include "board_types.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief Register an LVGL display that flushes through the bsp_wrapper_display
 *        abstraction.  Must be called after lv_init() and after the display
 *        adapter has been registered (drv_adapter_display_register) and
 *        initialised (display_drv_init).
 *
 * @return true on success, false if LVGL rejects the registration.
 */
BOOL lv_port_disp_init(void);

/**
 * Flush-path profiling counters, accumulated since the last _take() call.
 *
 * `dma_cycles` measures dispatch -> TX-DMA-complete, i.e. how long the panel
 * bus was actually busy shifting pixels.  Comparing it against the wall time
 * covered by the same window separates "the SPI link is the bottleneck" from
 * "LVGL spent the frame rendering" (typically waiting on external-flash
 * asset reads), which need opposite fixes.
 */
typedef struct
{
    UINT32_t flush_cnt;    /* flush_cb invocations                          */
    UINT32_t flush_px;     /* pixels handed to the panel                    */
    UINT32_t dma_cycles;   /* CPU cycles with a flush DMA in flight         */
    UINT32_t err_cnt;      /* dispatch failures (chunk dropped)             */
} lv_port_disp_perf_t;

/**
 * @brief  Snapshot the flush counters and reset them to zero.
 *
 *         The ISR-side accumulation is re-entrant with this read, so the
 *         copy-and-clear runs with interrupts masked; the window is a few
 *         dozen cycles.
 *
 * @param[out] out : Receives the counters accumulated since the previous
 *                   call.  Ignored when NULL (counters still reset).
 */
void lv_port_disp_perf_take(lv_port_disp_perf_t *out);
//******************************* Functions *********************************//

#endif /* __LV_PORT_DISP_H__ */

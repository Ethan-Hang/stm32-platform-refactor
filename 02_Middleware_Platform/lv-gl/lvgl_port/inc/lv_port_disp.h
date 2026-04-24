/******************************************************************************
 * @file lv_port_disp.h
 *
 * @par dependencies
 * - lvgl.h
 * - bsp_st7789_driver.h
 *
 * @author Ethan-Hang
 *
 * @brief LVGL display port over the ST7789 driver.
 *
 * @version V1.0 2026-04-24
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __LV_PORT_DISP_H__
#define __LV_PORT_DISP_H__

//******************************** Includes *********************************//
#include <stdint.h>
#include <stdbool.h>

#include "bsp_st7789_driver.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief Register an LVGL display that flushes through the supplied ST7789
 *        driver instance.  Must be called after lv_init() and after the driver
 *        has been instanced and initialized.
 *
 * @param[in] p_driver  Ready-to-use ST7789 driver instance.
 *
 * @return true on success, false if arguments are invalid or LVGL rejects
 *         the display registration.
 */
bool lv_port_disp_init(bsp_st7789_driver_t *p_driver);
//******************************* Functions *********************************//

#endif /* __LV_PORT_DISP_H__ */

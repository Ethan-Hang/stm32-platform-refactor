/******************************************************************************
 * @file lv_port_indev.h
 *
 * @par dependencies
 * - lvgl.h
 * - bsp_cst816t_driver.h
 *
 * @author Ethan-Hang
 *
 * @brief LVGL pointer input-device port over the CST816T touch driver.
 *
 * @version V1.0 2026-04-25
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __LV_PORT_INDEV_H__
#define __LV_PORT_INDEV_H__

//******************************** Includes *********************************//
#include <stdint.h>
#include <stdbool.h>

#include "bsp_cst816t_driver.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief      Register an LVGL pointer indev backed by the supplied CST816T
 *             driver instance.  Must be called after lv_init() and after the
 *             touch controller has been instanced + initialized.
 *
 * @param[in]  p_driver : Ready-to-use CST816T driver instance.
 *
 * @return     true on success, false on argument or LVGL registration error.
 * */
bool lv_port_indev_init(bsp_cst816t_driver_t *p_driver);
//******************************* Functions *********************************//

#endif /* __LV_PORT_INDEV_H__ */

/******************************************************************************
 * @file bsp_adapter_port_display.h
 *
 * @par dependencies
 * - bsp_wrapper_display.h
 *
 * @author Ethan-Hang
 *
 * @brief Adapter port interface for the ST7789 display controller.
 *
 * Registers a concrete ST7789 driver implementation into the
 * bsp_wrapper_display vtable.  The ST7789 driver instance is owned by the
 * adapter and instantiated from st7789_input_arg supplied by the
 * integration layer.
 *
 * @version V1.0 2026-04-25
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __BSP_ADAPTER_PORT_DISPLAY_H__
#define __BSP_ADAPTER_PORT_DISPLAY_H__

//******************************** Includes *********************************//
#include "bsp_wrapper_display.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief Instantiate the ST7789 driver from st7789_input_arg and register
 *        it into the bsp_wrapper_display vtable.
 *
 * Call once during BSP initialisation, before any wrapper API call.
 */
void drv_adapter_display_register(void);
//******************************* Functions *********************************//

#endif /* __BSP_ADAPTER_PORT_DISPLAY_H__ */

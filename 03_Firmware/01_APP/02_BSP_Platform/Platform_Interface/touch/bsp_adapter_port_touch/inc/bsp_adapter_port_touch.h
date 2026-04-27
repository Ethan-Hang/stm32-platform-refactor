/******************************************************************************
 * @file bsp_adapter_port_touch.h
 *
 * @par dependencies
 * - bsp_wrapper_touch.h
 *
 * @author Ethan-Hang
 *
 * @brief Adapter port interface for the CST816T capacitive touch controller.
 *
 * Registers a concrete CST816T driver implementation into the
 * bsp_wrapper_touch vtable.  The CST816T driver instance is owned by the
 * adapter and instantiated from cst816t_input_arg supplied by the
 * integration layer.
 *
 * @version V1.0 2026-04-26
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __BSP_ADAPTER_PORT_TOUCH_H__
#define __BSP_ADAPTER_PORT_TOUCH_H__

//******************************** Includes *********************************//
#include "bsp_wrapper_touch.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief Instantiate the CST816T driver from cst816t_input_arg and
 *        register it into the bsp_wrapper_touch vtable.
 *
 * Call once during BSP initialisation, before any wrapper API call.
 */
void drv_adapter_touch_register(void);
//******************************* Functions *********************************//

#endif /* __BSP_ADAPTER_PORT_TOUCH_H__ */

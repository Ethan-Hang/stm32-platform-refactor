/******************************************************************************
 * @file platform_io_register.c
 *
 * @par dependencies
 * - bsp_adapter_port_temp_humi.h
 * - bsp_adapter_port_motion.h
 *
 * @author Ethan-Hang
 *
 * @brief Centralized registration of all BSP platform IO adapters.
 *
 * @details Processing flow:
 *          platform_io_register()
 *              └─ drv_adapter_temp_humi_register()   mount temp/humi adapter
 *              └─ drv_adapter_motion_register()       mount motion adapter
 *
 * @version V1.0 2026-4-16
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "platform_io_register.h"
#include "bsp_adapter_port_temp_humi.h"
#include "bsp_adapter_port_motion.h"
//******************************** Includes *********************************//

//********************************* Macros **********************************//

//********************************* Macros **********************************//

//******************************* Variables *********************************//

//******************************* Variables *********************************//

//******************************* Functions *********************************//

/**
 * @brief Register all BSP platform IO adapters.
 */
void platform_io_register(void)
{
    drv_adapter_temp_humi_register();
    drv_adapter_motion_register();
}

//******************************* Functions *********************************//

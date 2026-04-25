/******************************************************************************
 * @file platform_io_register.c
 *
 * @par dependencies
 * - bsp_adapter_port_temp_humi.h
 * - bsp_adapter_port_motion.h
 * - bsp_adapter_port_audio.h
 * - bsp_adapter_port_display.h
 * - bsp_adapter_port_touch.h
 *
 * @author Ethan-Hang
 *
 * @brief Centralized registration of all BSP platform IO adapters.
 *
 * @details Processing flow:
 *          platform_io_register()
 *              └─ drv_adapter_temp_humi_register()   mount temp/humi adapter
 *              └─ drv_adapter_motion_register()      mount motion adapter
 *              └─ drv_adapter_audio_register()       mount audio adapter
 *              └─ drv_adapter_display_register()     mount display adapter
 *                                                    (instantiates st7789
 *                                                     from st7789_input_arg)
 *              └─ drv_adapter_touch_register()       mount touch adapter
 *                                                    (instantiates cst816t
 *                                                     from cst816t_input_arg)
 *
 * @version V1.0 2026-4-16
 * @version V2.0 2026-4-26
 * @upgrade 2.0: Added touch adapter registration.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "platform_io_register.h"
#include "bsp_adapter_port_temp_humi.h"
#include "bsp_adapter_port_motion.h"
#include "bsp_adapter_port_audio.h"
#include "bsp_adapter_port_display.h"
#include "bsp_adapter_port_touch.h"
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
    drv_adapter_audio_register();
    drv_adapter_display_register();
    drv_adapter_touch_register();
}

//******************************* Functions *********************************//

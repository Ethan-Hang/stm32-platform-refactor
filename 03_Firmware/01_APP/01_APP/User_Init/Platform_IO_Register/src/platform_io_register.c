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
#include "i2c_port.h"
#include "spi_port.h"
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
 *
 * MCU-port buses are initialised first so the bus-level mutex exists
 * before any adapter touches the bus.  FreeRTOS allows mutex creation
 * before the scheduler starts, which is the context this runs in
 * (called from main() ahead of osKernelInitialize()).
 */
void platform_io_register(void)
{
    /* MCU port buses (mutex creation; HAL handles already initialised). */
    (void)core_i2c_port_init(CORE_I2C_BUS_1);   /* MPU6050 / I2C3       */
    (void)core_i2c_port_init(CORE_I2C_BUS_2);   /* CST816T / I2C1       */
    (void)core_spi_port_init(CORE_SPI_BUS_1);   /* ST7789  / SPI1       */

    /* BSP adapter mounts. */
    drv_adapter_temp_humi_register();
    drv_adapter_motion_register();
    drv_adapter_audio_register();
    drv_adapter_display_register();
    drv_adapter_touch_register();
}

//******************************* Functions *********************************//

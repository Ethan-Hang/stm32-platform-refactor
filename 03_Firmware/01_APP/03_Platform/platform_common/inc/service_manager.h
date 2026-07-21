/******************************************************************************
 * @file service_manager.h
 *
 * @par dependencies
 * - platform_device.h
 *
 * @author Ethan-Hang
 *
 * @brief Provide the platform service manager lifecycle interface.
 *
 * Processing flow:
 *
 * 1. Registers platform service objects.
 * 2. Drives init/start/process/stop/deinit through platform_object_t.
 * 3. Keeps service startup and shutdown order explicit.
 * 
 * @version V1.0 2026-07-21
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __SERVICE_MANAGER_H__
#define __SERVICE_MANAGER_H__

//******************************** Includes *********************************//
#include "platform_device.h"

//******************************** Includes *********************************//

//****************************** Typedefines ********************************//
typedef struct
{
    UINT32_T service_count;
    platform_object_state_t state;
    UINT8_T ready;
} service_manager_data_t;

//****************************** Typedefines ********************************//

//******************************* Functions *********************************//
platform_err_t service_manager_register(platform_device_t *p_service);
platform_err_t service_manager_init(void);
platform_err_t service_manager_start(void);
platform_err_t service_manager_process(void);
platform_err_t service_manager_stop(void);
platform_err_t service_manager_deinit(void);
UINT32_T service_manager_get_count(void);
platform_err_t service_manager_get_data(service_manager_data_t *p_data);

//******************************* Functions *********************************//

#endif /* __SERVICE_MANAGER_H__ */

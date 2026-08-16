/******************************************************************************
 * @file platform_service.h
 *
 * @par dependencies
 * - platform_object.h
 *
 * @author Ethan-Hang
 *
 * @brief Provide the common platform service base object.
 *
 * Processing flow:
 *
 * 1. Use platform_object_t as the first field of each service object.
 * 2. Classify the service by platform_service_class_t.
 * 3. Bind cfg/ctx/data/ops for the four-tuple service model.
 * 
 * Lifecycle is stored in the embedded platform_object_t.
 * 
 * @version V1.0 2026-07-16
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __PLATFORM_SERVICE_H__
#define __PLATFORM_SERVICE_H__

//******************************** Includes *********************************//
#include "platform_object.h"

//******************************** Includes *********************************//

//****************************** Typedefines ********************************//

/* Smart watch service class. */
typedef enum
{
    PLATFORM_SERVICE_CLASS_SYSTEM,   /* System orchestration service.        */
    PLATFORM_SERVICE_CLASS_SENSOR,   /* Sensor data acquisition service.     */
    PLATFORM_SERVICE_CLASS_BATTERY,  /* Battery and power management service.*/
    PLATFORM_SERVICE_CLASS_POWER,    /* Power management service.            */
    PLATFORM_SERVICE_CLASS_STORAGE,  /* Storage management service.          */
    PLATFORM_SERVICE_CLASS_BACKLIGHT,/* Display backlight control service.   */
    PLATFORM_SERVICE_CLASS_BLE,      /* Bluetooth low energy service.        */
    PLATFORM_SERVICE_CLASS_OTA,      /* Over-the-air update service.         */
} platform_service_class_t;

/* Common management face of every platform service. */
typedef struct
{
    platform_object_t object;               /* Common identity and lifecycle. */
    platform_service_class_t service_class; /* Service class.                 */
    void const * p_cfg;                     /* Pointer to the service config. */
    void       * p_ctx;                     /* Pointer to the service context.*/
    void       *p_data;                     /* Pointer to the service data.   */
    void const * p_ops;                     /* Pointer to the service ops.    */
} platform_service_t;

//****************************** Typedefines ********************************//

//******************************* Functions *********************************//
/**
 * @brief Initializes a platform service without data and ops slots.
 *
 * Steps:
 *  1. Initializes the service object identity as PLATFORM_OBJECT_SERVICE.
 *  2. Binds lifecycle to the embedded platform_object_t.
 *  3. Records service class, cfg and ctx pointers.
 *
 * @param[in] p_svc         : Pointer to the platform service target.
 * @param[in] p_name        : Pointer to the service name string.
 * @param[in] service_class : Smart watch service class.
 * @param[in] p_cfg         : Pointer to the static configuration.
 * @param[in] p_ctx         : Pointer to the runtime context.
 * @param[in] p_lifecycle   : Pointer to the lifecycle callback table.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_service_init(platform_service_t *p_svc,
                                     const char *p_name,
                                     platform_service_class_t service_class,
                                     void const *p_cfg,
                                     void *p_ctx,
                                     platform_lifecycle_ops_t const *p_lifecycle);

/**
 * @brief Initializes a platform service with the full four-tuple model.
 *
 * Steps:
 *  1. Initializes the service object identity as PLATFORM_OBJECT_SERVICE.
 *  2. Binds lifecycle to the embedded platform_object_t.
 *  3. Records service class and all four-tuple pointers (cfg/ctx/data/ops).
 *
 * @param[in] p_svc         : Pointer to the platform service target.
 * @param[in] p_name        : Pointer to the service name string.
 * @param[in] service_class : Smart watch service class.
 * @param[in] p_cfg         : Pointer to the static configuration.
 * @param[in] p_ctx         : Pointer to the runtime context.
 * @param[in] p_data        : Pointer to the current service data.
 * @param[in] p_ops         : Pointer to service behavior operations.
 * @param[in] p_lifecycle   : Pointer to the lifecycle callback table.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_service_model_init(
                                  platform_service_t *p_svc,
                                  const char *p_name,
                                  platform_service_class_t service_class,
                                  void const *p_cfg,
                                  void *p_ctx,
                                  void *p_data,
                                  void const *p_ops,
                                  platform_lifecycle_ops_t const *p_lifecycle);

//******************************* Functions *********************************//

#endif /* __PLATFORM_SERVICE_H__ */ 

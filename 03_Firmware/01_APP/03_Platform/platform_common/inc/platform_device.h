/******************************************************************************
 * @file platform_device.h
 *
 * @par dependencies
 * - platform_object.h
 * 
 * @author Ethan-Hang
 *
 * @brief Provide the common platform object identity and lifecycle model.
 *
 * platform_object_t is the shared base identity for platform devices,
 * services, managers and application objects.
 * 
 * Processing flow:
 *
 * 1. Concrete objects embed platform_object_t as their first field.
 * 2. Managers register objects and drive the lifecycle uniformly.
 * 3. Lifecycle is a cross-cutting platform ability, not a per-type concern.
 *
 * @version V1.0 2026-07-16
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __PLATFORM_DEVICE_H__
#define __PLATFORM_DEVICE_H__

//******************************** Includes *********************************//
#include "platform_object.h"

//******************************** Includes *********************************//

//****************************** Typedefines ********************************//
/* Smart watch device class. */
typedef enum
{
    PLATFORM_DEVICE_CLASS_DISPLAY,    /* Display panel device.               */
    PLATFORM_DEVICE_CLASS_TOUCH,      /* Touch panel device.                 */
    PLATFORM_DEVICE_CLASS_IMU,        /* Motion sensor device.               */
    PLATFORM_DEVICE_CLASS_TEMP_HUMI,  /* Temperature and humidity device.    */
    PLATFORM_DEVICE_CLASS_HEART_RATE, /* Heart-rate sensor device.           */
    PLATFORM_DEVICE_CLASS_BATTERY,    /* Battery and charger monitor.        */
    PLATFORM_DEVICE_CLASS_STORAGE,    /* External or internal storage.       */
    PLATFORM_DEVICE_CLASS_BACKLIGHT,  /* Display backlight device.           */
    PLATFORM_DEVICE_CLASS_MOTOR,      /* Vibration motor device.             */
    PLATFORM_DEVICE_CLASS_CPU,        /* CPU or core clock control object.   */
    PLATFORM_DEVICE_CLASS_CLOCK,      /* System clock tree control object.   */
    PLATFORM_DEVICE_CLASS_POWER,      /* PMIC or board power control object. */
    PLATFORM_DEVICE_CLASS_RTC,        /* Real-time clock device.             */
    PLATFORM_DEVICE_CLASS_KEY,        /* Physical key or button device.      */
} platform_device_class_t;

/* Static device IO capabilities. */
typedef enum
{
    PLATFORM_DEVICE_CAP_READ     = 1u << 0, /* Supports read operations.     */
    PLATFORM_DEVICE_CAP_WRITE    = 1u << 1, /* Supports write operations.    */
    PLATFORM_DEVICE_CAP_CONTROL  = 1u << 2, /* Supports control operations.  */
    PLATFORM_DEVICE_CAP_IRQ      = 1u << 3, /* Supports interrupt operations.*/
    PLATFORM_DEVICE_CAP_DMA      = 1u << 4, /* Supports DMA operations.      */
    PLATFORM_DEVICE_CAP_SLEEP    = 1u << 5, /* Supports sleep operations.    */
    PLATFORM_DEVICE_CAP_WAKEUP   = 1u << 6, /* Supports wakeup operations.   */
    PLATFORM_DEVICE_CAP_PERIODIC = 1u << 7, /* Supports periodic operations. */
} platform_device_cap_t;

/* Common management face of every platform device. */
typedef struct
{
    platform_object_t             object; /* Common identity + lifecycle.    */
    platform_device_class_t device_class; /* Device class.                   */
    UINT32_T                         cap; /* Static IO capability flags.     */
} platform_device_t;

//****************************** Typedefines ********************************//

//******************************* Functions *********************************//
/**
 * @brief Initializes the common management fields of a platform device.
 *
 * Steps:
 *  1. Initializes the device object identity as PLATFORM_OBJECT_DEVICE.
 *  2. Binds lifecycle to the embedded platform_object_t.
 *  3. Records the device class and static capability flags.
 *
 * @param[in] p_dev       : Pointer to the platform device target.
 * @param[in] p_name      : Pointer to the device name string.
 * @param[in] dev_class   : Smart watch device class.
 * @param[in] caps        : Static IO capability flags.
 * @param[in] p_self      : Pointer to the concrete device object.
 * @param[in] p_lifecycle : Pointer to the lifecycle callback table.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_device_init(platform_device_t *p_dev,
                                    const char *name,
                                    platform_device_class_t device_class,
                                    UINT32_T caps,
                                    void *p_self,
                                    platform_lifecycle_ops_t const *p_lifecycle);

//******************************* Functions *********************************//

#endif /* __PLATFORM_DEVICE_H__ */

/******************************************************************************
 * @file platform_device.c
 *
 * @par dependencies
 * - platform_device.h
 * - Debug.h
 * 
 * @author Ethan-Hang
 *
 * @brief Implement the common platform device base object.
 *
 * Processing flow:
 *
 * 1. Build the object identity of a platform device.
 * 2. Record device class and IO capabilities.
 * 
 * @version V1.0 2026-07-17
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "platform_device.h"

#include "Debug.h"

//******************************** Includes *********************************//

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
                                    const char *p_name,
                                    platform_device_class_t device_class,
                                    UINT32_T caps,
                                    void *p_self,
                                    platform_lifecycle_ops_t const *p_lifecycle)
{
    platform_err_t ret = PLATFORM_OK;

    if (NULL == p_dev)
    {
        DEBUG_OUT(e, PLATFORM_ERR_TAG, "platform_device_init: p_dev is NULL");
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }

    if (NULL == p_self)
    {
        p_self = p_dev;
    }

    ret = platform_object_init(&p_dev->object, 
                                p_name, 
                                PLATFORM_OBJECT_DEVICE, 
                                p_self, 
                                NULL, 
                                p_lifecycle);
    if (PLATFORM_IS_ERR(ret))
    {
        DEBUG_OUT(e, PLATFORM_ERR_TAG, "platform_device_init: "
                             "platform_object_init failed with error %d", ret);
        return ret;
    }

    p_dev->device_class = device_class;
    p_dev->cap = caps;

    return ret;
}

//******************************* Functions *********************************//

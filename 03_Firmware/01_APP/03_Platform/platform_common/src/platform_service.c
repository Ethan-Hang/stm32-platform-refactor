/******************************************************************************
 * @file platform_service.c
 *
 * @par dependencies
 * - platform_service.h
 * 
 * - Debug.h
 *
 * @author Ethan-Hang
 *
 * @brief Implement the common platform service base object.
 *
 * Processing flow:
 *
 * 1. Build the object identity of a platform service.
 * 2. Record the service class.
 * 3. Bind cfg/ctx/data/ops for the four-tuple model.
 *
 * @version V1.0 2026-07-17
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "platform_service.h"

#include "Debug.h"

//******************************** Includes *********************************//

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
                                     platform_lifecycle_ops_t const *p_lifecycle)
{
    return platform_service_model_init(p_svc, 
                                       p_name, 
                                       service_class, 
                                       p_cfg, 
                                       p_ctx, 
                                       NULL, 
                                       NULL, 
                                       p_lifecycle);
}

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
                                  platform_lifecycle_ops_t const *p_lifecycle)
{
    platform_err_t ret = PLATFORM_OK;

    if (NULL == p_svc)
    {
        DEBUG_OUT(e, PLATFORM_ERR_TAG, 
                                 "platform_service_model_init: p_svc is NULL");
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }

    ret = platform_object_init(&p_svc->object, 
                                p_name, 
                                PLATFORM_OBJECT_SERVICE, 
                                p_svc, 
                                NULL, 
                                p_lifecycle);
    if (PLATFORM_IS_ERR(ret))
    {
        DEBUG_OUT(e, PLATFORM_ERR_TAG, "platform_service_model_init: "
                             "platform_object_init failed with error %d", ret);
        return ret;
    }

    p_svc->service_class = service_class;
    p_svc->p_cfg  = p_cfg;
    p_svc->p_ctx  = p_ctx;
    p_svc->p_data = p_data;
    p_svc->p_ops  = p_ops;

    return ret;
}

//******************************* Functions *********************************//

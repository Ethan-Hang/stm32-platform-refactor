/******************************************************************************
 * @file platform_object.c
 *
 * @par dependencies
 *
 * @author Ethan-Hang
 *
 * @brief
 *
 * Processing flow:
 *
 *
 * @version V1.0 2026-07-17
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "platform_object.h"

#include "Debug.h"

//******************************** Includes *********************************//

//******************************* Functions *********************************//
static platform_err_t platform_object_run_hook(platform_object_t *p_obj, 
                                               platform_err_t (*pf_hook_cb)(void *p_self),
                                               platform_object_state_t new_state);

/**
 * @brief Initializes the common identity and lifecycle binding of a platform object.
 *
 * Steps:
 *  1. Checks whether the object pointer is valid.
 *  2. Writes the object magic, name, type and initial state.
 *  3. Binds p_self, p_parent and lifecycle callback table.
 *  4. Clears extension flags and user data.
 *
 * @param[in] p_obj        : Pointer to the platform object target.
 * @param[in] p_name       : Pointer to the object name string.
 * @param[in] type         : Platform object high-level type.
 * @param[in] p_self       : Pointer to the concrete owner object.
 * @param[in] p_parent     : Pointer to the parent or manager object.
 * @param[in] p_lifecycle  : Pointer to the lifecycle callback table.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_object_init(platform_object_t *p_obj,
                                    const char *p_name,
                                    platform_object_type_t type,
                                    void *p_self,
                                    void *p_parent,
                                    platform_lifecycle_ops_t const *p_lifecycle)
{
    platform_err_t ret = PLATFORM_OK;

    if (NULL == p_obj)
    {
        ret = PLATFORM_ERR_NULL_PTR;
        DEBUG_OUT(e, PLATFORM_ERR_TAG,
                                 "platform_object_init: p_obj is NULL");
        return ret;
    }
    if (NULL == p_self)
    {
        p_self = p_obj;
    }

    p_obj->magic       = PLATFORM_OBJECT_MAGIC;
    p_obj->name        = p_name;
    p_obj->type        = type;
    p_obj->state       = PLATFORM_OBJECT_CREATED;
    p_obj->p_self      = p_self;
    p_obj->p_parent    = p_parent;
    p_obj->p_lifecycle = p_lifecycle;
    p_obj->user_data   = NULL;
    p_obj->flags       = 0u;

    return ret;
}

/**
 * @brief Updates the lifecycle state record of a platform object.
 *
 * @param[in] p_obj : Pointer to the platform object target.
 * @param[in] state : New object lifecycle state.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_object_set_state(platform_object_t *p_obj, 
                                         platform_object_state_t state)
{
    if (NULL == p_obj)
    {
        DEBUG_OUT(e, PLATFORM_ERR_TAG,
                                 "platform_object_set_state: p_obj is NULL");
        return PLATFORM_ERR_NULL_PTR;
    }

    p_obj->state = state;
    
    return PLATFORM_OK;
}

/**
 * @brief Updates the parent pointer of a platform object.
 *
 * @param[in] p_obj    : Pointer to the platform object target.
 * @param[in] p_parent : Pointer to the parent or manager object.
 *
 * @return platform_err_t : Status of the function.
 */                   
platform_err_t platform_object_set_parent(platform_object_t *p_obj, 
                                                       void *p_parent)
{
    if (NULL == p_obj)
    {
        DEBUG_OUT(e, PLATFORM_ERR_TAG,
                                 "platform_object_set_parent: p_obj is NULL");
        return PLATFORM_ERR_NULL_PTR;
    }

    p_obj->p_parent = p_parent;
    return PLATFORM_OK;
}

/**
 * @brief Checks whether a platform object pointer matches the expected type.
 *
 * @param[in] p_obj : Pointer to the platform object target.
 * @param[in] type  : Expected platform object high-level type.
 *
 * @return BOOL_T : TRUE if the object pointer is valid.
 */
BOOL_T platform_object_is_valid(platform_object_t *p_obj,
                                platform_object_type_t type)
{
    return ((NULL != p_obj) &&
            (PLATFORM_OBJECT_MAGIC == p_obj->magic) &&
            (type == p_obj->type));
}

/**
 * @brief Moves the object into registered state.
 *
 * @param[in] p_obj    : Pointer to the platform object target.
 * @param[in] p_parent : Pointer to the parent or manager object.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_object_lifecycle_register(platform_object_t *p_obj,
                                                            void *p_parent)
{
    platform_err_t ret = PLATFORM_OK;

    if (NULL == p_obj)
    {
        DEBUG_OUT(e, PLATFORM_ERR_TAG,
                          "platform_object_lifecycle_register: p_obj is NULL");
        return PLATFORM_ERR_NULL_PTR;
    }

    ret = platform_object_set_parent(p_obj, p_parent);
    if (PLATFORM_IS_ERR(ret))
    {
        DEBUG_OUT(e, PLATFORM_ERR_TAG,
                       "platform_object_lifecycle_register: "
                       "platform_object_set_parent failed with error %d", ret);
        return ret;
    }

    ret = platform_object_set_state(p_obj, PLATFORM_OBJECT_REGISTERED);

    return ret;
}

/**
 * @brief Executes one lifecycle stage through the object's lifecycle table.
 *
 * @param[in] p_obj : Pointer to the platform object target.
 * @param[in] stage : Lifecycle stage to execute.
 *
 * @return platform_err_t : Status of the function.
 */
platform_err_t platform_object_lifecycle_run(platform_object_t *p_obj,
                                             platform_lifecycle_stage_t stage)
{
    if (NULL == p_obj)
    {
        DEBUG_OUT(e, PLATFORM_ERR_TAG,
                          "platform_object_lifecycle_run: p_obj is NULL");
        return PLATFORM_ERR_NULL_PTR;
    }

    switch(stage)
    {
        case PLATFORM_LIFECYCLE_STAGE_INIT:
            return platform_object_lifecycle_init(p_obj);    
        case PLATFORM_LIFECYCLE_STAGE_START:
            return platform_object_lifecycle_start(p_obj);   
        case PLATFORM_LIFECYCLE_STAGE_PROCESS:
            return platform_object_lifecycle_process(p_obj); 
        case PLATFORM_LIFECYCLE_STAGE_STOP:
            return platform_object_lifecycle_stop(p_obj);    
        case PLATFORM_LIFECYCLE_STAGE_SLEEP:
            return platform_object_lifecycle_sleep(p_obj);   
        case PLATFORM_LIFECYCLE_STAGE_WAKEUP:
            return platform_object_lifecycle_wakeup(p_obj);  
        case PLATFORM_LIFECYCLE_STAGE_DEINIT:
            return platform_object_lifecycle_deinit(p_obj);  

        default:
            break;
    }

    return PLATFORM_ERR_RESERVED;
}

platform_err_t platform_object_lifecycle_init(platform_object_t *p_obj)
{
    platform_err_t ret = PLATFORM_OK;
    if (NULL == p_obj)
    {
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }

    if (PLATFORM_OBJECT_INITIALIZED == p_obj->state ||
        PLATFORM_OBJECT_STARTED == p_obj->state)
    {
        return PLATFORM_OK;
    }
    if (NULL == p_obj->p_lifecycle || NULL == p_obj->p_lifecycle->init)
    {
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }

    ret = platform_object_run_hook(p_obj, 
                                   p_obj->p_lifecycle->init,
                                   PLATFORM_OBJECT_INITIALIZED);
    return ret;
}

platform_err_t platform_object_lifecycle_start(platform_object_t *p_obj)
{
    platform_err_t ret = PLATFORM_OK;
    if (NULL == p_obj)
    {
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }
    
    if (PLATFORM_OBJECT_STARTED == p_obj->state)
    {
        return PLATFORM_OK;
    }

    if (PLATFORM_OBJECT_INITIALIZED != p_obj->state &&
        PLATFORM_OBJECT_STOPPED != p_obj->state)
    {
        return PLATFORM_ERR_NOT_INITIALIZED;
    }

    if (NULL == p_obj->p_lifecycle || NULL == p_obj->p_lifecycle->start)
    {
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }

    ret = platform_object_run_hook(p_obj, 
                                   p_obj->p_lifecycle->start,
                                   PLATFORM_OBJECT_STARTED);
    return ret;
}

platform_err_t platform_object_lifecycle_process(platform_object_t *p_obj)
{
    platform_err_t ret = PLATFORM_OK;
    if (NULL == p_obj)
    {
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }
    
    if (PLATFORM_OBJECT_STARTED != p_obj->state)
    {
        ret = PLATFORM_OK;
        return ret;
    }

    if (NULL == p_obj->p_lifecycle || NULL == p_obj->p_lifecycle->process)
    {
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }

    ret = p_obj->p_lifecycle->process(p_obj->p_self);
    if (PLATFORM_IS_ERR(ret))
    {
        p_obj->state = PLATFORM_OBJECT_ERROR;
        return ret;
    }

    return ret;
}

platform_err_t platform_object_lifecycle_stop(platform_object_t *p_obj)
{
    platform_err_t ret = PLATFORM_OK;
    if (NULL == p_obj)
    {
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }
    if (PLATFORM_OBJECT_STARTED != p_obj->state)
    {
        p_obj->state = PLATFORM_OBJECT_STOPPED;
        return PLATFORM_OK;
    }
    if (NULL == p_obj->p_lifecycle || NULL == p_obj->p_lifecycle->stop)
    {
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }

    return platform_object_run_hook(p_obj, 
                                    p_obj->p_lifecycle->stop,
                                    PLATFORM_OBJECT_STOPPED);
}

platform_err_t platform_object_lifecycle_sleep(platform_object_t *p_obj)
{
    platform_err_t ret = PLATFORM_OK;
    if (NULL == p_obj)
    {
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }
    if (PLATFORM_OBJECT_STARTED != p_obj->state)
    {
        return PLATFORM_OK;
    }
    if (NULL == p_obj->p_lifecycle || NULL == p_obj->p_lifecycle->sleep)
    {
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }

    return platform_object_run_hook(p_obj,
                                    p_obj->p_lifecycle->sleep,
                                    PLATFORM_OBJECT_STARTED);
}

platform_err_t platform_object_lifecycle_wakeup(platform_object_t *p_obj)
{
    platform_err_t ret = PLATFORM_OK;
    if (NULL == p_obj)
    {
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }
    if (PLATFORM_OBJECT_STARTED != p_obj->state)
    {
        return PLATFORM_OK;
    }
    if (NULL == p_obj->p_lifecycle || NULL == p_obj->p_lifecycle->wakeup)
    {
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }

    return platform_object_run_hook(p_obj, 
                                    p_obj->p_lifecycle->wakeup, 
                                    PLATFORM_OBJECT_STARTED);
}

platform_err_t platform_object_lifecycle_deinit(platform_object_t *p_obj)
{
    platform_err_t ret = PLATFORM_OK;
    if (NULL == p_obj)
    {
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }

    if (NULL == p_obj->p_lifecycle || NULL == p_obj->p_lifecycle->deinit)
    {
        ret = PLATFORM_ERR_NULL_PTR;
        return ret;
    }

    return platform_object_run_hook(p_obj,
                                    p_obj->p_lifecycle->deinit,
                                    PLATFORM_OBJECT_DEINITIALIZED);
}

static platform_err_t platform_object_run_hook(platform_object_t *p_obj, 
                                               platform_err_t (*pf_hook_cb)(void *p_self),
                                               platform_object_state_t new_state)
{
    platform_err_t ret = PLATFORM_OK;

    if (NULL == p_obj)
    {
        DEBUG_OUT(e, PLATFORM_ERR_TAG,
                          "platform_object_run_hook: p_obj is NULL");
        return PLATFORM_ERR_NULL_PTR;
    }

    if (NULL != pf_hook_cb)
    {
        ret = pf_hook_cb(p_obj->p_self);
        if (PLATFORM_IS_ERR(ret))
        {
            p_obj->state = PLATFORM_OBJECT_ERROR;
            return ret;
        }
    }

    p_obj->state = new_state;
    return ret;
}

//******************************* Functions *********************************//

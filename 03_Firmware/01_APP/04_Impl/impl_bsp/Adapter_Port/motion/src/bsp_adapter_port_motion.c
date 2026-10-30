/******************************************************************************
 * @file bsp_adapter_port_motion.c
 *
 * @par dependencies
 * - bsp_mpuxxxx_handler.h
 * - bsp_adapter_port_motion.h
 *
 * @author Ethan-Hang
 *
 * @brief MPU6050 adapter implementation for the motion wrapper layer.
 *        Implements the motion_drv_t vtable using the MPU6050 handler
 *        and circular buffer APIs, then registers itself into the wrapper.
 *
 * Processing flow:
 *   mpuxxxx_drv_get_req()
 *     → spins until the handler instance exists
 *     → takes one permit from the handler's counting semaphore, i.e. claims
 *       one ring slot the handler has committed
 *   mpuxxxx_get_data_addr()
 *     → returns the current read-slot address from the circular buffer
 *   mpuxxxx_read_data_done()
 *     → advances the circular buffer read pointer
 *
 * @version V1.0 2026-04-16
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "board_types.h"
#include "bsp_mpuxxxx_handler.h"
#include "bsp_adapter_port_motion.h"

#include "osal_common_types.h"
#include "osal_error.h"
#include "osal_sema.h"
#include "osal_task.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//

/**
 * @brief   Adapter init — no hardware action needed; MPU6050 is initialised
 *          by its own handler task.
 *
 * @param[in] dev : Pointer to the motion driver vtable (unused).
 */
static void mpuxxxx_drv_init(motion_drv_t *const dev)
{
    (void)dev;
}

/**
 * @brief   Adapter deinit — no hardware action needed here.
 *
 * @param[in] dev : Pointer to the motion driver vtable (unused).
 */
static void mpuxxxx_drv_deinit(motion_drv_t *const dev)
{
    (void)dev;
}

/**
 * @brief   Block until a new DMA data packet is available.
 *          Spins (with a yield) while the handler has not been instantiated,
 *          then takes one permit from its counting semaphore.
 *
 * @param[in] dev : Pointer to the motion driver vtable (unused).
 *
 * @return  PLATFORM_OK              - One ring slot has been claimed.
 *          PLATFORM_ERR_NO_RESOURCE - Semaphore missing, or take failed.
 */
static platform_err_t mpuxxxx_drv_get_req(motion_drv_t *const dev)
{
    (void)dev;
    bsp_mpuxxxx_hanlder_t const *p_handler = mpuxxxx_handler_get_instance();
    INT32_t                      ret       = OSAL_SUCCESS;

    /* Wait until the handler is initialized */
    while (NULL == p_handler)
    {
        p_handler = mpuxxxx_handler_get_instance();
        osal_task_delay(50);
    }

    if (NULL == p_handler->unpack_sema_handle)
    {
        return PLATFORM_ERR_NO_RESOURCE;
    }

    /* Take one permit, i.e. claim one committed ring slot. Blocks while the
     * count is zero, which is exactly "the handler has produced nothing new"
     * — no payload is transported, the bytes are already in the ring. */
    ret = osal_sema_take(p_handler->unpack_sema_handle, OSAL_MAX_DELAY);
    if (OSAL_SUCCESS != ret)
    {
        DEBUG_OUT(e, UNPACK_ERR_LOG_TAG,
                    "mpuxxxx unpack task take sema failed, ret=%d", (int)ret);
        return PLATFORM_ERR_NO_RESOURCE;
    }

    return PLATFORM_OK;
}

/**
 * @brief   Return the address of the current raw data read-slot.
 *          The caller must not advance the read pointer until
 *          mpuxxxx_read_data_done() is called.
 *
 * @param[in] dev : Pointer to the motion driver vtable (unused).
 *
 * @return  Pointer to the 14-byte MPU6050 data packet, or NULL on error.
 */
static UINT8_t *mpuxxxx_get_data_addr(motion_drv_t *const dev)
{
    (void)dev;
    circular_buffer_t *p_cbuf = circular_buffer_get_instance();
    return p_cbuf->pf_get_rbuffer_addr(p_cbuf);
}

/**
 * @brief   Advance the circular buffer read pointer after data is consumed.
 *
 * @param[in] dev : Pointer to the motion driver vtable (unused).
 */
static void mpuxxxx_read_data_done(motion_drv_t *const dev)
{
    (void)dev;
    circular_buffer_t *p_cbuf = circular_buffer_get_instance();
    p_cbuf->pf_data_readed(p_cbuf);
}

/**
 * @brief   Register the MPU6050 driver into the motion wrapper.
 *          Fills the vtable and calls drv_adapter_motion_mount().
 *
 * @return  true  - Registration successful.
 *          false - Mount failed.
 */
BOOL drv_adapter_motion_register(void)
{
    motion_drv_t motion_drv = {
        .dev_id                    =                       0,
        .idx                       =                       0,
        .user_data                 =                    NULL,
        .pf_motion_drv_init        =        mpuxxxx_drv_init,
        .pf_motion_drv_deinit      =      mpuxxxx_drv_deinit,
        .pf_motion_drv_get_req     =     mpuxxxx_drv_get_req,
        .pf_motion_get_data_addr   =   mpuxxxx_get_data_addr,
        .pf_motion_read_data_done  =  mpuxxxx_read_data_done,
    };

    return drv_adapter_motion_mount(0, &motion_drv);
}

//******************************* Functions *********************************//

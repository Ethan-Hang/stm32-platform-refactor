/******************************************************************************
 * @file bsp_mpuxxxx_handler.h
 *
 * @par dependencies
 * - bsp_mpu6050_driver_refactor.h
 *
 * @author Ethan-Hang
 *
 * @brief Declare MPUXXXX handler context and task-level integration APIs.
 *
 * Processing flow:
 * Bind external interfaces, initialize handler resources, then run task loop.
 * @version V1.0 2026-3-2
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/
#ifndef __BSP_MPUXXXX_HANLDER_H__
#define __BSP_MPUXXXX_HANLDER_H__

//******************************** Includes *********************************//
#include "board_types.h"
#include "bsp_mpuxxxx_driver.h"
#include "osal_common_types.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
/**
 * @brief   What the integration layer hands the handler task.
 *
 * The four separate interface pointers of V1 collapse into the driver's own
 * deps aggregate: the handler no longer needs to know how those bindings are
 * laid out, it just forwards the aggregate to bsp_mpuxxxx_driver_inst().
 * The cfg rides along so the board -- not the driver -- picks the tuning.
 */
typedef struct
{
    /** Board bindings, forwarded verbatim to the driver.             **/
    mpuxxxx_deps_t                 const * const            p_deps;

    /** Initial tuning; NULL takes MPUXXXX_CFG_DEFAULT.               **/
    mpuxxxx_cfg_t                  const * const             p_cfg;
} mpuxxxx_handler_input_args_t;

/** Slots in the raw-packet ring. One is reserved to keep "full" apart
 *  from "empty", so at most MPUXXXX_RING_SLOTS-1 packets are in flight. **/
#define MPUXXXX_RING_SLOTS                                          (10)

typedef struct
{
    mpuxxxx_handler_input_args_t
                           const *            p_input_args;
    bsp_mpuxxxx_driver_t         *                p_driver;

    /**
     * Counts committed ring slots: the handler gives once per packet it
     * writes, the unpack consumer takes one before reading a slot. It
     * replaces the old 20-deep queue, which carried a token nobody read
     * and whose depth disagreed with the ring's -- once eleven tokens
     * were queued the consumer was handed slots the DMA had already
     * overwritten. Ceiling and ring now come from one constant.
     **/
    osal_sema_handle_t                     unpack_sema_handle;

    /** Packets dropped because the consumer had not freed a slot.      **/
    UINT32_t                                     dropped_count;
}bsp_mpuxxxx_hanlder_t;

//******************************** Defines **********************************//

//******************************* Declaring *********************************//
/**
 * @brief   Instantiate the MPU handler.
 *          Binds the input arguments to the handler
 *          and triggers the internal init routine.
 * 
 * @param[in] p_handler    : Pointer to the handler
 *                           instance.
 * @param[in] p_input_args : Pointer to the init args,
 *                           including OS, IIC, delay
 *                           and yield interfaces.
 * 
 * @param[out] : None
 * 
 * @return  MPUXXXX_OK             - Success
 *          MPUXXXX_ERRORPARAMETER - NULL pointer input
 *          Other                  - Init error
 * 
 * */
mpuxxxx_status_t mpuxxxx_hanlder_inst(bsp_mpuxxxx_hanlder_t      *   p_handler,
                                    mpuxxxx_handler_input_args_t *p_input_args);

/**
 * @brief   Get a read-only pointer to the singleton handler instance.
 *          External modules must call this instead of using extern.
 *
 * @return  Const pointer to the handler instance.
 *
 * */
bsp_mpuxxxx_hanlder_t const * mpuxxxx_handler_get_instance(void);

                                    /**
 * @brief   FreeRTOS task for the MPU handler.
 *          Initializes the circular buffer, driver
 *          and handler instance. Runs a polling loop
 *          that monitors the DMA completion flag;
 *          on flag asserted, sends a notification
 *          to the unpack queue.
 * 
 * @param[in] argument : Pointer to
 *                       mpuxxxx_handler_input_args_t
 *                       containing all required OS,
 *                       IIC, delay and yield
 *                       interfaces.
 * 
 * @param[out] : None
 * 
 * @return  None
 * 
 * */
void mpuxxxx_handler_thread(void *argument);

//******************************* Declaring *********************************//

#endif // end of __BSP_MPUXXXX_HANLDER_H__

/******************************************************************************
 * @file bsp_mpuxxxx_driver.h
 *
 * @par dependencies
 * - stdint.h
 * - stdbool.h
 * - string.h
 * 
 * - circular_buffer.h
 *
 * - bsp_mpu6050_reg.h
 * - bsp_mpu6050_reg_bit.h
 * 
 * - Debug.h
 * 
 * @author Ethan-Hang
 *
 * @brief Define MPUXXXX driver interfaces, data structures, and API surface.
 *
 * Processing flow:
 * Collect HAL/OS adapters and expose unified MPUXXXX driver operations.
 * @version V1.0 2025-1-6
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once

#ifndef __BSP_MPUXXXX_DRIVER_H__
#define __BSP_MPUXXXX_DRIVER_H__

//******************************** Includes *********************************//
#include "board_types.h"
#include <stdio.h>
#include <string.h>

#include "circular_buffer.h"

#include "bsp_mpu6050_reg.h"
#include "bsp_mpu6050_reg_bit.h"

#include "Debug.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define OS_SUPPORTING                (1)

typedef enum
{
    MPUXXXX_OK                      = 0,    /* Operation successful          */
    MPUXXXX_ERROR                   = 1,    /* General error                 */
    MPUXXXX_ERRORTIMEOUT            = 2,    /* Timeout error                 */
    MPUXXXX_ERRORRESOURCE           = 3,    /* Resource unavailable          */
    MPUXXXX_ERRORPARAMETER          = 4,    /* Invalid parameter             */
    MPUXXXX_ERRORNOMEMORY           = 5,    /* Out of memory                 */
    MPUXXXX_ERRORUNSUPPORTED        = 6,    /* Unsupported feature           */
    MPUXXXX_ERRORISR                = 7,    /* ISR context error             */
    MPUXXXX_RESERVED                = 0xFF, /* MPUXXXX Reserved              */
} mpuxxxx_status_t;

/*********************** Core Layer **********************/
/*      From HAL Layer: IIC Port     */
typedef struct 
{
    /*       hi2c pointer to a i2c handle structure      */
    void                     *                        hi2c;

    /*         i2c init and deinit interfaces            */
    mpuxxxx_status_t (*pf_iic_init        )               \
                                      (void const * const);
    mpuxxxx_status_t (*pf_iic_deinit      )               \
                                      (void const * const);

    /**/
    mpuxxxx_status_t (*pf_iic_mem_write   ) (void    *i2c,
                                        UINT16_t des_addr,
                                        UINT16_t mem_addr,
                                        UINT16_t mem_size,
                                        UINT8_t   *p_data,
                                        UINT16_t     size,
                                        UINT32_t  timeout);
    mpuxxxx_status_t (*pf_iic_mem_read    ) (void    *i2c,
                                        UINT16_t des_addr,
                                        UINT16_t mem_addr,
                                        UINT16_t mem_size,
                                        UINT8_t   *p_data,
                                        UINT16_t     size,
                                        UINT32_t  timeout);                              
    mpuxxxx_status_t (*pf_iic_mem_read_dma) (void    *i2c,
                                        UINT16_t des_addr,
                                        UINT16_t mem_addr,
                                        UINT16_t mem_size,
                                        UINT8_t   *p_data,
                                        UINT16_t     size);
} iic_driver_interface_t;

/*       Interrupt Of MPUXXXX        */
typedef struct
{
    mpuxxxx_status_t (*pf_irq_deinit       )        (void);
} hardware_interrupt_interface_t;


/************************ Os Layer ***********************/
#if OS_SUPPORTING
/*        Yield Interface            */
typedef struct
{
    void             (*pf_rtos_yield       ) (UINT32_t \
                                                 const ms);
} yield_interface_t;

/*          Os Interface             */
typedef struct
{
    /*       os queue interface      */
    mpuxxxx_status_t (*pf_os_queue_create    ) (UINT32_t const   queue_length,
                                                UINT32_t const     queue_size,
                                                void  ** const  queue_handler);
    mpuxxxx_status_t (*pf_os_queue_send      ) (void  *  const  queue_handler,
                                                void  *  const       item_ptr,
                                                UINT32_t const      wait_time);
    mpuxxxx_status_t (*pf_os_queue_receive   ) (void  *  const  queue_handler,
                                                void  *  const       item_ptr,
                                                UINT32_t const      wait_time);
    /* os task notification interface */
    mpuxxxx_status_t (*pf_os_semaphore_wait_notify)\
                                               (UINT32_t ulBitsToClearOnEntry, 
                                                UINT32_t  ulBitsToClearOnExit, 
                                                UINT32_t*pulNotificationValue,
                                                UINT32_t              timeout);                                                      
    mpuxxxx_status_t (*pf_os_semaephore_notify_isr)\
                                               (void  *  const binary_handler,
                                                UINT32_t const        ulValue,
                                                UINT32_t const        eAction,
                                                long  *  const \
                                                      HigherPriorityTaskWoken);
    void *           (*pf_os_get_task_handle )                          (void);
} os_interface_t;
#endif // OS_SUPPORTING

/*      Data Structure of MPUXXXX    */
typedef struct 
{
    /*     raw accelerometer data    */
    INT16_t                accel_x_raw;
    INT16_t                accel_y_raw;
    INT16_t                accel_z_raw;
    
    /*  processed accelerometer data */
    DOUBLE                   accel_x_g;
    DOUBLE                   accel_y_g;
    DOUBLE                   accel_z_g;

    /*      raw gyroscope data       */
    INT16_t                 gyro_x_raw;
    INT16_t                 gyro_y_raw;
    INT16_t                 gyro_z_raw;

    /*    processed gyroscope data   */
    DOUBLE                  gyro_x_dps;
    DOUBLE                  gyro_y_dps;
    DOUBLE                  gyro_z_dps;

    /*     processed temperature     */
    FLOAT                temperature_c;

    /* kalman filter processed angles*/
    DOUBLE              kalman_angle_x;
    DOUBLE              kalman_angle_y;
} mpuxxxx_data_t;

typedef struct bsp_mpuxxxx_driver
{
    /*      Driver Private Data     */
    BOOL                                                          private_data;

    /*          core layer          */
    iic_driver_interface_t              const  *         p_iic_driver_instance;
    hardware_interrupt_interface_t      const  *          p_interrupt_instance;

    /*          os layer            */
#if OS_SUPPORTING
    yield_interface_t                   const  *              p_yield_instance;
    os_interface_t                      const  *                 p_os_instance;
    void                                const  *                 queue_hanlder;
    void                                const  *                 mutex_handler;
    void                                const  *             semaphore_handler;
    void                                const  *                notify_handler;
#endif // OS_SUPPORTING

    /*       callback functions      */
    void (*pf_dma_completed_callback)                                   (void);
    void (*pf_int_interrupt_callback)                                   (void);

    /*  interface of mpuxxxx driver  */
    mpuxxxx_status_t (*pf_deinit               ) (struct bsp_mpuxxxx_driver   \
                                                                const * const);
    mpuxxxx_status_t (*pf_sleep                ) (struct bsp_mpuxxxx_driver   \
                                                                const * const);
    mpuxxxx_status_t (*pf_wakeup               ) (struct bsp_mpuxxxx_driver   \
                                                                const * const);
    mpuxxxx_status_t (*pf_set_gyro_fsr         ) (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                                      UINT8_t);
    mpuxxxx_status_t (*pf_set_accel_fsr        ) (struct bsp_mpuxxxx_driver   \
                                                                const * const,
                                                                      UINT8_t);
    mpuxxxx_status_t (*pf_set_lpf              ) (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                                      UINT8_t);
    mpuxxxx_status_t (*pf_set_rate             ) (struct bsp_mpuxxxx_driver   \
                                                                const * const,
                                                                      UINT8_t);
    mpuxxxx_status_t (*pf_set_interrupt_enable ) (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                                      UINT8_t);
    mpuxxxx_status_t (*pf_set_motion_threshold ) (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                                      UINT8_t);
    mpuxxxx_status_t (*pf_set_INT_level        ) (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                                      UINT8_t);
    mpuxxxx_status_t (*pf_set_user_ctrl        ) (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                                      UINT8_t);
    mpuxxxx_status_t (*pf_set_pwr_mgmt1_reg    ) (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                                      UINT8_t);
    mpuxxxx_status_t (*pf_set_pwr_mgmt2_reg    ) (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                                      UINT8_t);
    mpuxxxx_status_t (*pf_set_fifo_en_reg      ) (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                                      UINT8_t);    
    mpuxxxx_status_t (*pf_get_temperature      ) (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                       mpuxxxx_data_t * const);
    mpuxxxx_status_t (*pf_get_accel            ) (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                       mpuxxxx_data_t * const);
    mpuxxxx_status_t (*pf_get_gyro             ) (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                       mpuxxxx_data_t * const);
    mpuxxxx_status_t (*pf_get_all_data         ) (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                       mpuxxxx_data_t * const);
    mpuxxxx_status_t (*pf_get_interrupt_status_reg)                           \
                                                 (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                              UINT8_t * const);
    mpuxxxx_status_t (*pf_read_fifo_packet     ) (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                       mpuxxxx_data_t * const);
    mpuxxxx_status_t (*pf_read_fifo_isr_occur  ) (struct bsp_mpuxxxx_driver   \
                                                                const * const, 
                                                       mpuxxxx_data_t * const);                         

} bsp_mpuxxxx_driver_t;

//******************************** Defines **********************************//

//******************************* Declaring *********************************//
mpuxxxx_status_t bsp_mpuxxxx_driver_inst(
           bsp_mpuxxxx_driver_t                 * const       p_mpuxxxx_driver,

           iic_driver_interface_t         const * const p_iic_driver_interface,
           hardware_interrupt_interface_t const * const  p_interrupt_interface,

#if OS_SUPPORTING
           yield_interface_t              const * const      p_yield_interface,
           os_interface_t                 const * const         p_os_interface,
#endif // OS_SUPPORTING
           void (*callback_register    )
                           (void (*callback)(void const * const, void* const)),
           void (*callback_register_dma)
                           (void (*callback)(void const * const, void* const))
);

//******************************* Declaring *********************************//

#endif // end of __BSP_MPUXXXX_DRIVER_H__

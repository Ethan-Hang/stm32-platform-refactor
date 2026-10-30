/******************************************************************************
 * @file bsp_mpuxxxx_driver.h
 *
 * @par dependencies
 * - stdio.h
 * - string.h
 * - board_types.h
 * - circular_buffer.h
 * - bsp_mpu6050_reg.h
 * - bsp_mpu6050_reg_bit.h
 * - Debug.h
 *
 * @author Ethan-Hang
 *
 * @brief MPUXXXX driver surface, split into five orthogonal parts.
 *
 *   deps : what this chip NEEDS from the board (I2C / IRQ / OS).
 *          Bound once by *_inst(), read-only for the rest of the run.
 *          Changes when the MCU or the RTOS changes.
 *   cfg  : device tuning written INTO the chip registers.
 *          Re-appliable at runtime through pf_apply_config().
 *          Changes when the sensor is re-tuned, not when the MCU moves.
 *   ctx  : per-instance private state (PIMPL). Holds the cached cfg and
 *          every value derived from it. Never touched by upper layers.
 *   data : one complete sample snapshot; an out-parameter type only.
 *   ops  : what this chip CAN DO. One const table shared by all
 *          instances, so it lands in .rodata instead of per-instance RAM.
 *
 * The driver object therefore holds only three members: a deps pointer,
 * a ctx pointer and an ops pointer. cfg and data are parameter types,
 * not members.
 *
 * Processing flow:
 * 1. Integration layer fills a const mpuxxxx_deps_t.
 * 2. bsp_mpuxxxx_driver_inst() binds deps + ops, claims a ctx, applies
 *    the cfg (or MPUXXXX_CFG_DEFAULT when the caller passes NULL).
 * 3. Handler thread calls ops->pf_read_fifo_packet() and friends.
 * 4. ops->pf_apply_config() re-tunes the chip without re-binding deps.
 *
 * @version V2.0 2026-8-26
 *
 * @note 1 tab == 4 spaces!
 *****************************************************************************/

#ifndef __BSP_MPU6050_DRIVER_H__
#define __BSP_MPU6050_DRIVER_H__

//******************************** Includes *********************************//
#include <stdio.h>
#include <string.h>

#include "board_types.h"

#include "circular_buffer.h"

#include "bsp_mpu6050_reg.h"
#include "bsp_mpu6050_reg_bit.h"

#include "Debug.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define OS_SUPPORTING                                                     (1)

/** Number of driver instances served from the static ctx pool.           **/
#define MPUXXXX_MAX_INSTANCES                                             (1)

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

/*========================= 0. deps -- board bindings =======================*/
/*
 * Everything below is supplied by the integration layer and describes the
 * BOARD, not the chip. None of it can be written into an MPU register, and
 * none of it may be swapped once *_inst() has returned -- an I2C vtable
 * exchanged mid-session would corrupt an in-flight DMA read.
 */

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

#if OS_SUPPORTING
/*        Yield Interface            */
typedef struct
{
    void             (*pf_rtos_yield       ) (UINT32_t \
                                                 const ms);
} yield_interface_t;

/**
 *          Os Interface
 *
 * Only what the DRIVER itself reaches for: the ISR needs to wake a task,
 * and *_inst() needs to learn which task that is. The three queue entries
 * that used to sit here were never called by the driver -- they existed so
 * the handler could build its own producer/consumer object through the
 * chip's deps. That object is now a counting semaphore the handler creates
 * through OSAL directly, since it belongs to the handler / unpack pair and
 * not to the MPU.
 **/
typedef struct
{
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

/**
 * @brief One aggregate holding every downward binding.
 *
 * Grouping the four interfaces behind a single pointer replaces six
 * pointer members in the driver object with one, and lets the whole
 * aggregate be declared const so the compiler -- not a comment --
 * enforces "bound once, never re-bound".
 *
 * The two ISR trampolines are board wiring as well: they hand an
 * MCU-level EXTI / DMA vector to this driver, so they belong here and
 * not in ops.
 **/
typedef struct
{
    iic_driver_interface_t         const *              p_iic_interface;
    hardware_interrupt_interface_t const *        p_interrupt_interface;

#if OS_SUPPORTING
    yield_interface_t              const *            p_yield_interface;
    os_interface_t                 const *               p_os_interface;
#endif // OS_SUPPORTING

    /**
     * Hook the MPU data-ready EXTI line to this driver instance.
     * Called from *_inst() only AFTER the chip is configured, so a
     * stray edge cannot reach a half-built ctx.
     **/
    void (*pf_callback_register    )(void (*callback)(void const * const,
                                                      void       * const));
    /** Same, for the I2C RX-DMA completion vector.                   **/
    void (*pf_callback_register_dma)(void (*callback)(void const * const,
                                                      void       * const));
} mpuxxxx_deps_t;

/*======================= 1. cfg -- device tuning ===========================*/
/*
 * Typed replacements for the twelve pf_set_* entries of V1. The old
 * surface took raw UINT8_t register payloads, so every call site had to
 * spell out a shift macro (FS_SEL_BIT(0x03)) and nothing stopped a gyro
 * value from reaching an accel register.
 */

/** Gyroscope full-scale range -- GYRO_CONFIG (0x1B) FS_SEL[4:3].         **/
typedef enum
{
    MPUXXXX_GYRO_FSR_250DPS         = 0,    /* scale 131.0  LSB/dps          */
    MPUXXXX_GYRO_FSR_500DPS         = 1,    /* scale  65.5  LSB/dps          */
    MPUXXXX_GYRO_FSR_1000DPS        = 2,    /* scale  32.8  LSB/dps          */
    MPUXXXX_GYRO_FSR_2000DPS        = 3,    /* scale  16.4  LSB/dps          */
} mpuxxxx_gyro_fsr_t;

/** Accelerometer full-scale range -- ACCEL_CONFIG (0x1C) AFS_SEL[4:3].   **/
typedef enum
{
    MPUXXXX_ACCEL_FSR_2G            = 0,    /* scale 16384.0 LSB/g           */
    MPUXXXX_ACCEL_FSR_4G            = 1,    /* scale  8192.0 LSB/g           */
    MPUXXXX_ACCEL_FSR_8G            = 2,    /* scale  4096.0 LSB/g           */
    MPUXXXX_ACCEL_FSR_16G           = 3,    /* scale  2048.0 LSB/g           */
} mpuxxxx_accel_fsr_t;

/**
 * Digital low-pass filter -- CONFIG (0x1A) DLPF_CFG[2:0].
 * Anything but _260HZ drops the gyro output rate from 8 kHz to 1 kHz,
 * which is what makes the sample-rate divider below mean 1000 / (1+n).
 **/
typedef enum
{
    MPUXXXX_DLPF_260HZ              = 0,    /* gyro output rate 8 kHz        */
    MPUXXXX_DLPF_184HZ              = 1,    /* gyro output rate 1 kHz        */
    MPUXXXX_DLPF_94HZ               = 2,
    MPUXXXX_DLPF_44HZ               = 3,
    MPUXXXX_DLPF_21HZ               = 4,
    MPUXXXX_DLPF_10HZ               = 5,
    MPUXXXX_DLPF_5HZ                = 6,
} mpuxxxx_dlpf_t;

/** INT pin polarity -- INT_PIN_CFG (0x37) INT_LEVEL[7].                  **/
typedef enum
{
    MPUXXXX_INT_ACTIVE_HIGH         = 0,
    MPUXXXX_INT_ACTIVE_LOW          = 1,
} mpuxxxx_int_level_t;

/**
 * @brief Device tuning applied in one shot by pf_apply_config().
 *
 * Every field maps to a chip register and survives a re-apply at
 * runtime. Nothing board-specific belongs here -- that is what
 * mpuxxxx_deps_t is for.
 *
 * Note on sample_rate_div: sample rate = 1000 / (1 + div) Hz whenever
 * dlpf != MPUXXXX_DLPF_260HZ. V1 hard-coded SMPLRT_DIV_BIT(0x19) while
 * its own comment computed 1000 / (1 + 19) = 50 Hz -- 0x19 is 25, so
 * the shipped rate is 38.46 Hz, not 50 Hz. A named decimal field makes
 * that class of hex/decimal slip visible at the call site.
 **/
typedef struct
{
    /* Ranges and filtering                                           */
    mpuxxxx_gyro_fsr_t              gyro_fsr;
    mpuxxxx_accel_fsr_t             accel_fsr;
    mpuxxxx_dlpf_t                  dlpf;
    UINT8_t                         sample_rate_div;

    /* Data-ready interrupt -- INT_ENABLE (0x38) / INT_PIN_CFG (0x37) */
    BOOL                            data_ready_int_enable;
    mpuxxxx_int_level_t             int_level;

    /* Motion-detection threshold -- MOT_THR (0x1F), 0 disables       */
    UINT8_t                         motion_threshold;

    /* Raw register payloads kept as escape hatches. These carry FIFO,
     * clock-source and standby bit patterns that have no useful enum
     * yet; pf_write_reg covers one-off pokes outside this struct.    */
    UINT8_t                         user_ctrl;
    UINT8_t                         fifo_enable;
    UINT8_t                         pwr_mgmt1;
    UINT8_t                         pwr_mgmt2;
} mpuxxxx_cfg_t;

/**
 * Default tuning -- reproduces the sequence V1 hard-coded inside
 * bsp_mpuxxxx_driver_inst(), except that sample_rate_div is spelled in
 * decimal so it means the 50 Hz the original comment intended. Pass
 * NULL as the cfg argument of *_inst() to take this.
 **/
#define MPUXXXX_CFG_DEFAULT                                                   \
{                                                                             \
    .gyro_fsr               = MPUXXXX_GYRO_FSR_2000DPS,                       \
    .accel_fsr              = MPUXXXX_ACCEL_FSR_2G,                           \
    .dlpf                   = MPUXXXX_DLPF_10HZ,                              \
    .sample_rate_div        = 19,                                             \
    .data_ready_int_enable  = true,                                           \
    .int_level              = MPUXXXX_INT_ACTIVE_HIGH,                        \
    .motion_threshold       = 0,                                              \
    .user_ctrl              = 0,                                              \
    .fifo_enable            = 0,                                              \
    .pwr_mgmt1              = 0,                                              \
    .pwr_mgmt2              = 0,                                              \
}

/*======================= 2. data -- sample snapshot ========================*/

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

/*======================= 3. ctx -- private state ===========================*/

/**
 * @brief Opaque per-instance state. Defined in the .c file only.
 *
 * This is where the three file-level statics of V1 move to:
 *   g_mpuxxx_is_inited  -> ctx->is_init
 *   gs_gyro_scale       -> ctx->gyro_scale
 *   gs_accel_scale      -> ctx->accel_scale
 *
 * The two scale factors are not configuration: they are values DERIVED
 * from cfg.gyro_fsr / cfg.accel_fsr and recomputed inside
 * pf_apply_config. Holding them as file-level statics made all fourteen
 * raw-to-engineering conversions in the driver read one global, so a
 * second instance on a different FSR would silently rescale its
 * neighbour samples. Per-instance ctx removes that by construction.
 *
 * The OS handles (queue / mutex / semaphore / notify / task) live here
 * for the same reason: they are created per instance, not supplied by
 * the board, and so belong to neither deps nor cfg.
 **/
typedef struct mpuxxxx_ctx mpuxxxx_ctx_t;

/*======================= 4. ops -- device capability =======================*/

typedef struct bsp_mpuxxxx_driver bsp_mpuxxxx_driver_t;

/**
 * @brief What an MPUXXXX can do. One const table shared by every
 *        instance, so these pointers live in .rodata instead of being
 *        copied into each driver object.
 *
 * The twelve pf_set_* entries of V1 collapse into pf_apply_config; the
 * pf_read_reg / pf_write_reg pair stays as the escape hatch for
 * registers that have no typed cfg field yet.
 **/
typedef struct
{
    /* Lifecycle                                                      */
    mpuxxxx_status_t (*pf_deinit      )(bsp_mpuxxxx_driver_t *const self);
    mpuxxxx_status_t (*pf_sleep       )(bsp_mpuxxxx_driver_t *const self);
    mpuxxxx_status_t (*pf_wakeup      )(bsp_mpuxxxx_driver_t *const self);

    /* Re-tune the chip. Caches the cfg and recomputes the scales.    */
    mpuxxxx_status_t (*pf_apply_config)(
                                     bsp_mpuxxxx_driver_t *const self,
                                     mpuxxxx_cfg_t  const *const  cfg);

    /* Read back the cfg currently in force.                          */
    mpuxxxx_status_t (*pf_get_config  )(
                                     bsp_mpuxxxx_driver_t *const self,
                                     mpuxxxx_cfg_t        *const  cfg);

    /* Sampling                                                       */
    mpuxxxx_status_t (*pf_get_temperature)(
                                     bsp_mpuxxxx_driver_t *const self,
                                     mpuxxxx_data_t       *const data);
    mpuxxxx_status_t (*pf_get_accel      )(
                                     bsp_mpuxxxx_driver_t *const self,
                                     mpuxxxx_data_t       *const data);
    mpuxxxx_status_t (*pf_get_gyro       )(
                                     bsp_mpuxxxx_driver_t *const self,
                                     mpuxxxx_data_t       *const data);
    mpuxxxx_status_t (*pf_get_all_data   )(
                                     bsp_mpuxxxx_driver_t *const self,
                                     mpuxxxx_data_t       *const data);

    /* FIFO path                                                      */
    mpuxxxx_status_t (*pf_read_fifo_packet   )(
                                     bsp_mpuxxxx_driver_t *const self,
                                     mpuxxxx_data_t       *const data);
    mpuxxxx_status_t (*pf_read_fifo_isr_occur)(
                                     bsp_mpuxxxx_driver_t *const self,
                                     mpuxxxx_data_t       *const data);

    /* Status and raw register escape hatch                           */
    mpuxxxx_status_t (*pf_get_interrupt_status_reg)(
                                     bsp_mpuxxxx_driver_t *const self,
                                     UINT8_t              *const  val);
    mpuxxxx_status_t (*pf_read_reg    )(
                                     bsp_mpuxxxx_driver_t *const self,
                                     UINT8_t                  reg_addr,
                                     UINT8_t              *const  val);
    mpuxxxx_status_t (*pf_write_reg   )(
                                     bsp_mpuxxxx_driver_t *const self,
                                     UINT8_t                  reg_addr,
                                     UINT8_t                      val);
} mpuxxxx_ops_t;

/*========================= The driver object ===============================*/

/**
 * @brief Three members only: a binding, a state handle, a capability
 *        table. cfg and data never appear here -- they are the in and
 *        out parameter types of ops respectively.
 **/
struct bsp_mpuxxxx_driver
{
    /** Downward bindings, const: bound by *_inst(), never re-bound.  **/
    mpuxxxx_deps_t const *                                       deps;

    /** Private per-instance state, opaque to every caller.           **/
    mpuxxxx_ctx_t       *                                         ctx;

    /** Upward capability table, const, shared across instances.      **/
    mpuxxxx_ops_t const *                                         ops;
};
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
/**
 * @brief      Bind deps + ops to a driver object, claim a ctx from the
 *             static pool, verify WHO_AM_I and apply the initial cfg.
 *
 * @param[in]  p_driver : Caller-owned driver object to populate.
 *
 * @param[in]  p_deps   : Board bindings. Must outlive p_driver; the
 *                        aggregate is stored by pointer, not copied.
 *
 * @param[in]  p_cfg    : Initial tuning, copied into ctx. Pass NULL to
 *                        take MPUXXXX_CFG_DEFAULT.
 *
 * @param[out] p_driver : Populated on MPUXXXX_OK, untouched otherwise.
 *
 * @return     MPUXXXX_OK on success.
 *             MPUXXXX_ERRORPARAMETER when p_driver or p_deps is NULL.
 *             MPUXXXX_ERRORNOMEMORY  when the ctx pool is exhausted.
 *             MPUXXXX_ERROR          when WHO_AM_I does not match.
 *
 * */
mpuxxxx_status_t bsp_mpuxxxx_driver_inst(
                        bsp_mpuxxxx_driver_t       * const     p_driver,
                        mpuxxxx_deps_t       const * const       p_deps,
                        mpuxxxx_cfg_t        const * const        p_cfg);

//******************************* Declaring *********************************//

#endif // end of __BSP_MPU6050_DRIVER_H__

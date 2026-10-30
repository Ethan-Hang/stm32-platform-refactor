/******************************************************************************
 * @file       bsp_mpuxxxx_driver.c
 *
 * @par        dependencies
 * - bsp_mpu6050_driver_refactor.h
 *
 * @author     Ethan-Hang
 *
 * @brief      MPUxxxx (e.g. MPU6050) driver implementation, deps / cfg / ctx
 *             / data / ops split.
 *
 * Processing flow:
 * - bsp_mpuxxxx_driver_inst() binds the board deps, claims a ctx from the
 *   static pool, brings the bus up, resets + wakes the chip, verifies
 *   WHO_AM_I and applies the initial cfg.
 * - Everything a caller can do afterwards goes through the shared const ops
 *   table; the twelve pf_set_* entries of V1 collapse into one
 *   pf_apply_config() plus the pf_read_reg / pf_write_reg escape hatch.
 * - The two scale factors are derived from cfg.gyro_fsr / cfg.accel_fsr and
 *   cached per instance in ctx, so a second instance on a different FSR can
 *   no longer rescale its neighbour's samples.
 *
 * @version    V2.0 2026-8-26
 *
 * @note       1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "bsp_mpuxxxx_driver.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define TIME_OUT_MS                                            1000
#define IIC_MEM_SIZE_8BIT                               0x00000001U
#define MPUXXXX_ALL_DATA_LEN                                     14
#define MPUXXXX_FIFO_PACKET_LEN                                  12
#define MPUXXXX_RESET_DELAY_MS                                  100

/** Gyro scale (LSB/dps) indexed by mpuxxxx_gyro_fsr_t.                   **/
static DOUBLE const s_gyro_scale_lut [4] = {131.0, 65.5, 32.8, 16.4};

/** Accel scale (LSB/g) indexed by mpuxxxx_accel_fsr_t.                   **/
static DOUBLE const s_accel_scale_lut[4] = {16384.0, 8192.0, 4096.0, 2048.0};

/** The tuning taken when *_inst() is handed a NULL cfg.                  **/
static mpuxxxx_cfg_t const s_cfg_default = MPUXXXX_CFG_DEFAULT;

/**
 * @brief      Per-instance private state -- the PIMPL body the header only
 *             forward-declares.
 *
 * V1 kept is_init / gyro_scale / accel_scale as file-level statics, so every
 * instance shared one set. They live here instead, one set per claimed slot.
 **/
struct mpuxxxx_ctx
{
    /** Slot bookkeeping for the static pool below.                       **/
    BOOL                                                      in_use;

    /** Set once the chip answered WHO_AM_I and took its first cfg.       **/
    BOOL                                                     is_init;

    /** The tuning currently in force, returned by pf_get_config().       **/
    mpuxxxx_cfg_t                                                cfg;

    /** Derived from cfg.gyro_fsr / cfg.accel_fsr by apply_config().      **/
    DOUBLE                                                gyro_scale;
    DOUBLE                                               accel_scale;

    /** Task notified from the data-ready EXTI trampoline. Captured in
     *  *_inst(), which runs on the handler thread.                       **/
    void                                             *notify_handle;
};

/** Static ctx pool -- no malloc in a BSP driver.                          **/
static mpuxxxx_ctx_t s_ctx_pool[MPUXXXX_MAX_INSTANCES];

//******************************** Defines **********************************//

//******************************* Declaring *********************************//

/*======================= Pool and guard helpers ============================*/

/**
 * @brief      Claim the first free slot of the static ctx pool.
 *
 * @param[in]  : None.
 *
 * @param[out] : None.
 *
 * @return     Pointer to a zeroed ctx, or NULL when the pool is exhausted.
 * */
static mpuxxxx_ctx_t *mpuxxxx_ctx_claim(void)
{
    for (UINT32_t i = 0; i < MPUXXXX_MAX_INSTANCES; i++)
    {
        if (false == s_ctx_pool[i].in_use)
        {
            memset(&s_ctx_pool[i], 0, sizeof(s_ctx_pool[i]));
            s_ctx_pool[i].in_use = true;
            return &s_ctx_pool[i];
        }
    }
    return NULL;
}

/**
 * @brief      Return a slot to the static ctx pool.
 *
 * @param[in]  p_ctx : Slot to release; NULL is tolerated.
 *
 * @param[out]       : None.
 *
 * @return     None.
 * */
static void mpuxxxx_ctx_release(mpuxxxx_ctx_t * const p_ctx)
{
    if (NULL != p_ctx)
    {
        memset(p_ctx, 0, sizeof(*p_ctx));
    }
}

/**
 * @brief      Common entry guard: object shape plus "the chip is up".
 *
 * @param[in]  self : Driver instance to validate.
 *
 * @param[out]      : None.
 *
 * @return     MPUXXXX_OK             when the instance is usable.
 *             MPUXXXX_ERRORPARAMETER when self / deps / ctx is NULL.
 *             MPUXXXX_ERRORRESOURCE  when the chip is not initialized yet.
 * */
static mpuxxxx_status_t mpuxxxx_check_ready(
                                     bsp_mpuxxxx_driver_t const * const self)
{
    if (NULL == self       ||
        NULL == self->deps ||
        NULL == self->ctx  ||
        NULL == self->deps->p_iic_interface)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx input error parameter!");
        return MPUXXXX_ERRORPARAMETER;
    }

    if (false == self->ctx->is_init)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx not initialized!");
        return MPUXXXX_ERRORRESOURCE;
    }

    return MPUXXXX_OK;
}

/*========================= Bus access helpers ==============================*/

/**
 * @brief      Write a register block over I2C.
 *
 * @param[in]  self   : Driver instance (deps already validated).
 *
 * @param[in]  reg    : First register address.
 *
 * @param[in]  p_data : Payload to send.
 *
 * @param[in]  len    : Payload length in bytes.
 *
 * @param[out]        : None.
 *
 * @return     Status code from the bound I2C write.
 * */
static inline mpuxxxx_status_t mpuxxxx_write_reg(
                                     bsp_mpuxxxx_driver_t const * const self,
                                                             UINT16_t    reg,
                                                             UINT8_t *p_data,
                                                             UINT16_t    len)
{
    return self->deps->p_iic_interface->pf_iic_mem_write(
                                        self->deps->p_iic_interface->hi2c,
                                        (MPU_ADDR << 1) | 0,
                                        reg,
                                        IIC_MEM_SIZE_8BIT,
                                        p_data,
                                        len,
                                        TIME_OUT_MS);
}

/**
 * @brief      Read a register block over I2C.
 *
 * @param[in]  self   : Driver instance (deps already validated).
 *
 * @param[in]  reg    : First register address.
 *
 * @param[in]  len    : Byte count to read.
 *
 * @param[out] p_data : Receive buffer, len bytes.
 *
 * @return     Status code from the bound I2C read.
 * */
static inline mpuxxxx_status_t mpuxxxx_read_reg(
                                     bsp_mpuxxxx_driver_t const * const self,
                                                             UINT16_t    reg,
                                                             UINT8_t *p_data,
                                                             UINT16_t    len)
{
    return self->deps->p_iic_interface->pf_iic_mem_read(
                                        self->deps->p_iic_interface->hi2c,
                                        (MPU_ADDR << 1) | 1,
                                        reg,
                                        IIC_MEM_SIZE_8BIT,
                                        p_data,
                                        len,
                                        TIME_OUT_MS);
}

/**
 * @brief      Write one byte to one register, naming it on failure.
 *
 * @param[in]  self : Driver instance.
 *
 * @param[in]  reg  : Register address.
 *
 * @param[in]  val  : Value to write.
 *
 * @param[out]      : None.
 *
 * @return     mpuxxxx_status_t MPUXXXX_OK on success; error code otherwise.
 * */
static mpuxxxx_status_t mpuxxxx_write_byte(
                                     bsp_mpuxxxx_driver_t const * const self,
                                                             UINT8_t     reg,
                                                             UINT8_t     val)
{
    UINT8_t          data = val;
    mpuxxxx_status_t ret  = mpuxxxx_write_reg(self, reg, &data, 1);

    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx write register 0x%02X failed!", reg);
    }
    return ret;
}

/**
 * @brief      Sleep the calling context for the given number of ms.
 *
 * @param[in]  self : Driver instance.
 *
 * @param[in]  ms   : Delay in milliseconds.
 *
 * @param[out]      : None.
 *
 * @return     None.
 * */
static void mpuxxxx_yield(bsp_mpuxxxx_driver_t const * const self,
                                                  UINT32_t          ms)
{
#if OS_SUPPORTING
    if (NULL != self->deps->p_yield_interface &&
        NULL != self->deps->p_yield_interface->pf_rtos_yield)
    {
        self->deps->p_yield_interface->pf_rtos_yield(ms);
    }
#else
    (void)self;
    (void)ms;
#endif // OS_SUPPORTING
}

/*=========================== Sample decoding ===============================*/

/**
 * @brief      Assemble one big-endian 16-bit sample from a byte pair.
 *
 * @param[in]  p_buf : Pointer to the high byte.
 *
 * @param[out]       : None.
 *
 * @return     The signed sample.
 * */
static inline INT16_t mpuxxxx_be16(UINT8_t const * const p_buf)
{
    return (INT16_t)(((UINT16_t)(*(p_buf)) << 8) | (UINT16_t)(*(p_buf + 1)));
}

/**
 * @brief      Decode a 6-byte accel block into raw counts and g.
 *
 * @param[in]  p_ctx  : Instance state carrying the accel scale.
 *
 * @param[in]  p_buf  : ACCEL_XOUT_H .. ACCEL_ZOUT_L.
 *
 * @param[out] p_data : Destination snapshot.
 *
 * @return     None.
 * */
static void mpuxxxx_decode_accel(mpuxxxx_ctx_t  const * const  p_ctx,
                                 UINT8_t        const * const  p_buf,
                                 mpuxxxx_data_t       * const p_data)
{
    p_data->accel_x_raw = mpuxxxx_be16(p_buf    );
    p_data->accel_y_raw = mpuxxxx_be16(p_buf + 2);
    p_data->accel_z_raw = mpuxxxx_be16(p_buf + 4);

    p_data->accel_x_g   = ((DOUBLE)p_data->accel_x_raw / p_ctx->accel_scale);
    p_data->accel_y_g   = ((DOUBLE)p_data->accel_y_raw / p_ctx->accel_scale);
    p_data->accel_z_g   = ((DOUBLE)p_data->accel_z_raw / p_ctx->accel_scale);
}

/**
 * @brief      Decode a 6-byte gyro block into raw counts and dps.
 *
 * @param[in]  p_ctx  : Instance state carrying the gyro scale.
 *
 * @param[in]  p_buf  : GYRO_XOUT_H .. GYRO_ZOUT_L.
 *
 * @param[out] p_data : Destination snapshot.
 *
 * @return     None.
 * */
static void mpuxxxx_decode_gyro(mpuxxxx_ctx_t  const * const  p_ctx,
                                UINT8_t        const * const  p_buf,
                                mpuxxxx_data_t       * const p_data)
{
    p_data->gyro_x_raw = mpuxxxx_be16(p_buf    );
    p_data->gyro_y_raw = mpuxxxx_be16(p_buf + 2);
    p_data->gyro_z_raw = mpuxxxx_be16(p_buf + 4);

    p_data->gyro_x_dps = ((DOUBLE)p_data->gyro_x_raw / p_ctx->gyro_scale);
    p_data->gyro_y_dps = ((DOUBLE)p_data->gyro_y_raw / p_ctx->gyro_scale);
    p_data->gyro_z_dps = ((DOUBLE)p_data->gyro_z_raw / p_ctx->gyro_scale);
}

/**
 * @brief      Decode the 2-byte temperature block into Celsius.
 *
 * @param[in]  p_buf  : TEMP_OUT_H .. TEMP_OUT_L.
 *
 * @param[out] p_data : Destination snapshot.
 *
 * @return     None.
 * */
static void mpuxxxx_decode_temp(UINT8_t        const * const  p_buf,
                                mpuxxxx_data_t       * const p_data)
{
    p_data->temperature_c = ((FLOAT)mpuxxxx_be16(p_buf) / 340.0f + 36.53f);
}

/*============================ ops -- lifecycle =============================*/

/**
 * @brief      Release the bus and the IRQ line, then hand the ctx slot back.
 *
 * @param[in]  self : Driver instance.
 *
 * @param[out]      : None.
 *
 * @return     mpuxxxx_status_t MPUXXXX_OK on success; error code otherwise.
 * */
static mpuxxxx_status_t mpuxxxx_deinit(bsp_mpuxxxx_driver_t * const self)
{
    mpuxxxx_status_t ret = mpuxxxx_check_ready(self);
    if (MPUXXXX_OK != ret)
    {
        return ret;
    }

    if (NULL != self->deps->p_iic_interface->pf_iic_deinit)
    {
        ret = self->deps->p_iic_interface->pf_iic_deinit(
                                        self->deps->p_iic_interface->hi2c);
        if (MPUXXXX_OK != ret)
        {
            DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx iic deinit failed!");
        }
    }

    if (NULL != self->deps->p_interrupt_interface &&
        NULL != self->deps->p_interrupt_interface->pf_irq_deinit)
    {
        (void)self->deps->p_interrupt_interface->pf_irq_deinit();
    }

    mpuxxxx_ctx_release(self->ctx);
    self->ctx = NULL;

    return ret;
}

/**
 * @brief      Put the device into sleep mode.
 *
 * @param[in]  self : Driver instance.
 *
 * @param[out]      : None.
 *
 * @return     mpuxxxx_status_t MPUXXXX_OK on success; error code otherwise.
 * */
static mpuxxxx_status_t mpuxxxx_sleep(bsp_mpuxxxx_driver_t * const self)
{
    mpuxxxx_status_t ret = mpuxxxx_check_ready(self);
    if (MPUXXXX_OK != ret)
    {
        return ret;
    }

    /* Keep every other PWR_MGMT1 bit -- the clock source above all -- as
       the cached cfg left it. V1 wrote a bare SLEEP_BIT(1) here and so
       silently dropped CLKSEL back to the internal oscillator.          */
    UINT8_t pwr1 = (UINT8_t)(self->ctx->cfg.pwr_mgmt1 | (UINT8_t)SLEEP_BIT(1));

    return mpuxxxx_write_byte(self, MPU_PWR_MGMT1_REG, pwr1);
}

/**
 * @brief      Wake the device from sleep.
 *
 * @param[in]  self : Driver instance.
 *
 * @param[out]      : None.
 *
 * @return     mpuxxxx_status_t MPUXXXX_OK on success; error code otherwise.
 * */
static mpuxxxx_status_t mpuxxxx_wakeup(bsp_mpuxxxx_driver_t * const self)
{
    mpuxxxx_status_t ret = mpuxxxx_check_ready(self);
    if (MPUXXXX_OK != ret)
    {
        return ret;
    }

    UINT8_t pwr1 = (UINT8_t)(self->ctx->cfg.pwr_mgmt1 & (UINT8_t)~SLEEP_BIT(1));

    return mpuxxxx_write_byte(self, MPU_PWR_MGMT1_REG, pwr1);
}

/*============================ ops -- tuning ================================*/

/**
 * @brief      Reject a cfg whose typed fields fall outside their register
 *             encoding, before a single byte reaches the bus.
 *
 * @param[in]  p_cfg : Candidate tuning.
 *
 * @param[out]       : None.
 *
 * @return     MPUXXXX_OK or MPUXXXX_ERRORPARAMETER.
 * */
static mpuxxxx_status_t mpuxxxx_cfg_validate(mpuxxxx_cfg_t const * const p_cfg)
{
    if (p_cfg->gyro_fsr  > MPUXXXX_GYRO_FSR_2000DPS      ||
        p_cfg->accel_fsr > MPUXXXX_ACCEL_FSR_16G         ||
        p_cfg->dlpf      > MPUXXXX_DLPF_5HZ              ||
        p_cfg->int_level > MPUXXXX_INT_ACTIVE_LOW)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx cfg out of range!");
        return MPUXXXX_ERRORPARAMETER;
    }
    return MPUXXXX_OK;
}

/**
 * @brief      Push a whole tuning into the chip and cache what it implies.
 *
 * Register order matters and mirrors what V1 spread across twelve setters
 * plus mpuxxxx_fifo_init(): rate / filter / ranges -> motion -> INT wiring
 * -> FIFO sources -> FIFO enable -> power. FIFO_EN_REG has to be programmed
 * before USER_CTRL turns the FIFO on, otherwise the first packets are
 * assembled from stale sources.
 *
 * @param[in]  self  : Driver instance.
 *
 * @param[in]  p_cfg : Tuning to apply; NULL takes MPUXXXX_CFG_DEFAULT.
 *
 * @param[out]       : None.
 *
 * @return     mpuxxxx_status_t MPUXXXX_OK on success; error code otherwise.
 * */
static mpuxxxx_status_t mpuxxxx_apply_config(
                                    bsp_mpuxxxx_driver_t * const   self,
                                    mpuxxxx_cfg_t  const * const  p_cfg)
{
    if (NULL == self                 ||
        NULL == self->deps           ||
        NULL == self->ctx            ||
        NULL == self->deps->p_iic_interface)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx input error parameter!");
        return MPUXXXX_ERRORPARAMETER;
    }

    mpuxxxx_cfg_t const cfg = (NULL == p_cfg) ? s_cfg_default : *p_cfg;

    mpuxxxx_status_t ret = mpuxxxx_cfg_validate(&cfg);
    if (MPUXXXX_OK != ret)
    {
        return ret;
    }

    /* INT pin: polarity from cfg, INT_RD_CLEAR always on so a latched
       status bit cannot hold the line after the handler read the data
       registers.                                                        */
    UINT8_t int_pin = (UINT8_t)( (UINT8_t)INT_RD_CLEAR_BIT(1) |
    ((MPUXXXX_INT_ACTIVE_LOW == cfg.int_level) ? (UINT8_t)INT_LEVEL_BIT(1) : 0) );

    /* INT sources: data-ready is typed; motion follows a non-zero
       threshold, FIFO overflow follows USER_CTRL actually enabling FIFO. */
    UINT8_t int_en = (UINT8_t)COLOSE_ALL;

    if (cfg.data_ready_int_enable)
    {
        int_en |= (UINT8_t)DATA_RDY_EN_BIT(1);
    }
    if (0 != cfg.motion_threshold)
    {
        int_en |= (UINT8_t)MOT_EN_BIT(1);
    }
    if (0 != (cfg.user_ctrl & (UINT8_t)FIFO_EN_BIT(1)))
    {
        int_en |= (UINT8_t)FIFO_OVERFLOW_EN_BIT(1);
    }

    /* DEVICE_RESET is never a tuning: letting it through here would wipe
       every register this very function just wrote.                      */
    UINT8_t pwr1 = (UINT8_t)(cfg.pwr_mgmt1 & (UINT8_t)~DEVICE_RESET_BIT(1));

    struct
    {
        UINT8_t reg;
        UINT8_t val;
    } const writes[] =
    {
        { MPU_SAMPLE_RATE_REG, (UINT8_t)SMPLRT_DIV_BIT(cfg.sample_rate_div) },
        { MPU_CFG_REG,         (UINT8_t)DLPF_CFG_BIT(cfg.dlpf)              },
        { MPU_GYRO_CFG_REG,    (UINT8_t)FS_SEL_BIT(cfg.gyro_fsr)            },
        { MPU_ACCEL_CFG_REG,   (UINT8_t)AFS_SEL_BIT(cfg.accel_fsr)          },
        { MPU_MOTION_DET_REG,  (UINT8_t)MOT_THR_BIT(cfg.motion_threshold)   },
        { MPU_INTBP_CFG_REG,   int_pin                                      },
        { MPU_INT_EN_REG,      int_en                                       },
        { MPU_FIFO_EN_REG,     cfg.fifo_enable                              },
        { MPU_USER_CTRL_REG,   cfg.user_ctrl                                },
        { MPU_PWR_MGMT1_REG,   pwr1                                         },
        { MPU_PWR_MGMT2_REG,   cfg.pwr_mgmt2                                },
    };

    for (UINT32_t i = 0; i < (sizeof(writes) / sizeof(writes[0])); i++)
    {
        ret = mpuxxxx_write_byte(self, writes[i].reg, writes[i].val);
        if (MPUXXXX_OK != ret)
        {
            return ret;
        }
    }

    /* Cache only after the whole sequence landed, so a half-applied cfg
       never becomes the value pf_get_config() reports.                   */
    self->ctx->cfg         = cfg;
    self->ctx->gyro_scale  = s_gyro_scale_lut [cfg.gyro_fsr  & 0x03];
    self->ctx->accel_scale = s_accel_scale_lut[cfg.accel_fsr & 0x03];

    return MPUXXXX_OK;
}

/**
 * @brief      Read back the tuning currently in force.
 *
 * @param[in]  self  : Driver instance.
 *
 * @param[out] p_cfg : Receives the cached cfg.
 *
 * @return     mpuxxxx_status_t MPUXXXX_OK on success; error code otherwise.
 * */
static mpuxxxx_status_t mpuxxxx_get_config(
                                     bsp_mpuxxxx_driver_t * const   self,
                                     mpuxxxx_cfg_t        * const  p_cfg)
{
    mpuxxxx_status_t ret = mpuxxxx_check_ready(self);
    if (MPUXXXX_OK != ret)
    {
        return ret;
    }

    if (NULL == p_cfg)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx input error parameter!");
        return MPUXXXX_ERRORPARAMETER;
    }

    *p_cfg = self->ctx->cfg;

    return MPUXXXX_OK;
}

/*=========================== ops -- sampling ===============================*/

/**
 * @brief      Read temperature and convert to Celsius.
 *
 * @param[in]  self   : Driver instance.
 *
 * @param[out] p_data : Output snapshot.
 *
 * @return     mpuxxxx_status_t MPUXXXX_OK on success; error code otherwise.
 * */
static mpuxxxx_status_t mpuxxxx_get_temperature(
                                     bsp_mpuxxxx_driver_t * const   self,
                                     mpuxxxx_data_t       * const p_data)
{
    mpuxxxx_status_t ret = mpuxxxx_check_ready(self);
    if (MPUXXXX_OK != ret)
    {
        return ret;
    }

    if (NULL == p_data)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx input error parameter!");
        return MPUXXXX_ERRORPARAMETER;
    }

    UINT8_t temp_buf[2] = {0};

    ret = mpuxxxx_read_reg(self, MPU_TEMP_OUTH_REG, temp_buf, 2);
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx read MPU_TEMP_OUTH_REG register failed!");
        return ret;
    }

    mpuxxxx_decode_temp(temp_buf, p_data);

    return ret;
}

/**
 * @brief      Read accelerometer data and convert to g.
 *
 * @param[in]  self   : Driver instance.
 *
 * @param[out] p_data : Output snapshot.
 *
 * @return     mpuxxxx_status_t MPUXXXX_OK on success; error code otherwise.
 * */
static mpuxxxx_status_t mpuxxxx_get_accel(
                                     bsp_mpuxxxx_driver_t * const   self,
                                     mpuxxxx_data_t       * const p_data)
{
    mpuxxxx_status_t ret = mpuxxxx_check_ready(self);
    if (MPUXXXX_OK != ret)
    {
        return ret;
    }

    if (NULL == p_data)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx input error parameter!");
        return MPUXXXX_ERRORPARAMETER;
    }

    UINT8_t accel_buf[6] = {0};

    ret = mpuxxxx_read_reg(self, MPU_ACCEL_XOUTH_REG, accel_buf, 6);
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx read MPU_ACCEL_XOUTH_REG register failed!");
        return ret;
    }

    mpuxxxx_decode_accel(self->ctx, accel_buf, p_data);

    return ret;
}

/**
 * @brief      Read gyroscope data and convert to dps.
 *
 * @param[in]  self   : Driver instance.
 *
 * @param[out] p_data : Output snapshot.
 *
 * @return     mpuxxxx_status_t MPUXXXX_OK on success; error code otherwise.
 * */
static mpuxxxx_status_t mpuxxxx_get_gyro(
                                     bsp_mpuxxxx_driver_t * const   self,
                                     mpuxxxx_data_t       * const p_data)
{
    mpuxxxx_status_t ret = mpuxxxx_check_ready(self);
    if (MPUXXXX_OK != ret)
    {
        return ret;
    }

    if (NULL == p_data)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx input error parameter!");
        return MPUXXXX_ERRORPARAMETER;
    }

    UINT8_t gyro_buf[6] = {0};

    ret = mpuxxxx_read_reg(self, MPU_GYRO_XOUTH_REG, gyro_buf, 6);
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx read MPU_GYRO_XOUTH_REG register failed!");
        return ret;
    }

    mpuxxxx_decode_gyro(self->ctx, gyro_buf, p_data);

    return ret;
}

/**
 * @brief      Read accel, temperature and gyro in one 14-byte burst.
 *
 * @param[in]  self   : Driver instance.
 *
 * @param[out] p_data : Output snapshot.
 *
 * @return     mpuxxxx_status_t MPUXXXX_OK on success; error code otherwise.
 * */
static mpuxxxx_status_t mpuxxxx_get_all_data(
                                     bsp_mpuxxxx_driver_t * const   self,
                                     mpuxxxx_data_t       * const p_data)
{
    mpuxxxx_status_t ret = mpuxxxx_check_ready(self);
    if (MPUXXXX_OK != ret)
    {
        return ret;
    }

    if (NULL == p_data)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx input error parameter!");
        return MPUXXXX_ERRORPARAMETER;
    }

    UINT8_t data_buf[MPUXXXX_ALL_DATA_LEN] = {0};

    ret = mpuxxxx_read_reg(self, MPU_ACCEL_XOUTH_REG,
                           data_buf, MPUXXXX_ALL_DATA_LEN);
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx read MPU_ACCEL_XOUTH_REG register failed!");
        return ret;
    }

    mpuxxxx_decode_accel(self->ctx, data_buf,     p_data);
    mpuxxxx_decode_temp (           data_buf + 6, p_data);
    mpuxxxx_decode_gyro (self->ctx, data_buf + 8, p_data);

    return ret;
}

/*============================= ops -- FIFO =================================*/

/**
 * @brief      Pop one 12-byte accel+gyro packet out of the chip FIFO.
 *
 * V1 looped over every queued packet and wrote them to p_data[0..n-1],
 * although every call site owns exactly one mpuxxxx_data_t -- a silent
 * overrun of the caller's frame. The V2 surface hands out one snapshot per
 * call, so this pops the oldest packet only and leaves the rest queued.
 *
 * @param[in]  self   : Driver instance.
 *
 * @param[out] p_data : Output snapshot. temperature_c is left untouched:
 *                      the packet carries accel + gyro only.
 *
 * @return     MPUXXXX_OK            when a packet was popped.
 *             MPUXXXX_ERRORRESOURCE when the FIFO holds less than a packet.
 * */
static mpuxxxx_status_t mpuxxxx_read_fifo_packet(
                                     bsp_mpuxxxx_driver_t * const   self,
                                     mpuxxxx_data_t       * const p_data)
{
    mpuxxxx_status_t ret = mpuxxxx_check_ready(self);
    if (MPUXXXX_OK != ret)
    {
        return ret;
    }

    if (NULL == p_data)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx input error parameter!");
        return MPUXXXX_ERRORPARAMETER;
    }

    UINT8_t cnt_buf[2] = {0};

    ret = mpuxxxx_read_reg(self, MPU_FIFO_CNTH_REG, cnt_buf, 2);
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx read MPU_FIFO_CNTH_REG register failed!");
        return ret;
    }

    UINT16_t fifo_len = (UINT16_t)( ((UINT16_t)cnt_buf[0] << 8) |
                                     (UINT16_t)cnt_buf[1] );
    if (MPUXXXX_FIFO_PACKET_LEN > fifo_len)
    {
        return MPUXXXX_ERRORRESOURCE;
    }

    UINT8_t fifo_buf[MPUXXXX_FIFO_PACKET_LEN] = {0};

    ret = mpuxxxx_read_reg(self, MPU_FIFO_RW_REG,
                           fifo_buf, MPUXXXX_FIFO_PACKET_LEN);
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx read MPU_FIFO_RW_REG register failed!");
        return ret;
    }

    mpuxxxx_decode_accel(self->ctx, fifo_buf,     p_data);
    mpuxxxx_decode_gyro (self->ctx, fifo_buf + 6, p_data);

    return ret;
}

/**
 * @brief      FIFO drain path taken from the data-ready ISR notification.
 *             Still a placeholder, exactly as in V1.
 *
 * @param[in]  self   : Driver instance.
 *
 * @param[out] p_data : Output snapshot.
 *
 * @return     mpuxxxx_status_t MPUXXXX_OK on success; error code otherwise.
 * */
static mpuxxxx_status_t mpuxxxx_read_fifo_isr_occur(
                                     bsp_mpuxxxx_driver_t * const   self,
                                     mpuxxxx_data_t       * const p_data)
{
    mpuxxxx_status_t ret = mpuxxxx_check_ready(self);
    if (MPUXXXX_OK != ret)
    {
        return ret;
    }

    if (NULL == p_data)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx input error parameter!");
        return MPUXXXX_ERRORPARAMETER;
    }

    return ret;
}

/*==================== ops -- status and escape hatch =======================*/

/**
 * @brief      Read the interrupt status register.
 *
 * @param[in]  self  : Driver instance.
 *
 * @param[out] p_val : Interrupt status value.
 *
 * @return     mpuxxxx_status_t MPUXXXX_OK on success; error code otherwise.
 * */
static mpuxxxx_status_t mpuxxxx_get_interrupt_status_reg(
                                     bsp_mpuxxxx_driver_t * const  self,
                                     UINT8_t              * const p_val)
{
    mpuxxxx_status_t ret = mpuxxxx_check_ready(self);
    if (MPUXXXX_OK != ret)
    {
        return ret;
    }

    if (NULL == p_val)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx input error parameter!");
        return MPUXXXX_ERRORPARAMETER;
    }

    ret = mpuxxxx_read_reg(self, MPU_INT_STA_REG, p_val, 1);
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx read MPU_INT_STA_REG register failed!");
        return ret;
    }

    DEBUG_OUT(i, MPUXXXX_LOG_TAG,
              "mpuxxxx interrupt status register: 0x%02X", *p_val);

    return ret;
}

/**
 * @brief      Raw single-register read, for registers with no cfg field.
 *
 * @param[in]  self     : Driver instance.
 *
 * @param[in]  reg_addr : Register address.
 *
 * @param[out] p_val    : Register value.
 *
 * @return     mpuxxxx_status_t MPUXXXX_OK on success; error code otherwise.
 * */
static mpuxxxx_status_t mpuxxxx_read_reg_raw(
                                     bsp_mpuxxxx_driver_t * const     self,
                                                       UINT8_t    reg_addr,
                                     UINT8_t              * const    p_val)
{
    mpuxxxx_status_t ret = mpuxxxx_check_ready(self);
    if (MPUXXXX_OK != ret)
    {
        return ret;
    }

    if (NULL == p_val)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx input error parameter!");
        return MPUXXXX_ERRORPARAMETER;
    }

    ret = mpuxxxx_read_reg(self, reg_addr, p_val, 1);
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx read register 0x%02X failed!", reg_addr);
    }

    return ret;
}

/**
 * @brief      Raw single-register write, for registers with no cfg field.
 *
 * @param[in]  self     : Driver instance.
 *
 * @param[in]  reg_addr : Register address.
 *
 * @param[in]  val      : Value to write.
 *
 * @param[out]          : None.
 *
 * @return     mpuxxxx_status_t MPUXXXX_OK on success; error code otherwise.
 * */
static mpuxxxx_status_t mpuxxxx_write_reg_raw(
                                     bsp_mpuxxxx_driver_t * const     self,
                                                       UINT8_t    reg_addr,
                                                       UINT8_t         val)
{
    mpuxxxx_status_t ret = mpuxxxx_check_ready(self);
    if (MPUXXXX_OK != ret)
    {
        return ret;
    }

    return mpuxxxx_write_byte(self, reg_addr, val);
}

/**
 * @brief      The one capability table every instance shares. const, so it
 *             lands in .rodata instead of being copied per driver object.
 * */
static mpuxxxx_ops_t const s_mpuxxxx_ops =
{
    .pf_deinit                   =                        mpuxxxx_deinit,
    .pf_sleep                    =                         mpuxxxx_sleep,
    .pf_wakeup                   =                        mpuxxxx_wakeup,

    .pf_apply_config             =                  mpuxxxx_apply_config,
    .pf_get_config               =                    mpuxxxx_get_config,

    .pf_get_temperature          =               mpuxxxx_get_temperature,
    .pf_get_accel                =                     mpuxxxx_get_accel,
    .pf_get_gyro                 =                      mpuxxxx_get_gyro,
    .pf_get_all_data             =                  mpuxxxx_get_all_data,

    .pf_read_fifo_packet         =              mpuxxxx_read_fifo_packet,
    .pf_read_fifo_isr_occur      =           mpuxxxx_read_fifo_isr_occur,

    .pf_get_interrupt_status_reg =      mpuxxxx_get_interrupt_status_reg,
    .pf_read_reg                 =                  mpuxxxx_read_reg_raw,
    .pf_write_reg                =                 mpuxxxx_write_reg_raw,
};

/*========================== ISR trampolines ================================*/

/**
 * @brief      Data-ready EXTI trampoline.
 *
 * Under an RTOS this only notifies the handler thread: the DMA read needs
 * the bus mutex, and taking a mutex from ISR context is forbidden. Doing
 * the DMA here used to return HAL_BUSY whenever another task held the bus,
 * and the MPU thread then never progressed.
 *
 * @param[in]  this   : The driver instance, handed over by the vector.
 *
 * @param[in]  p_data : Bare-metal only: snapshot to fill synchronously.
 *
 * @param[out]        : None.
 *
 * @return     None.
 * */
void int_interrupt_callback(void const * const this, void * const p_data)
{
    if (NULL == this)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx int interrupt callback input error parameter!");
        return;
    }

    bsp_mpuxxxx_driver_t *p_driver = (bsp_mpuxxxx_driver_t *)this;

#if !(OS_SUPPORTING)
    /* Bare-metal path: read everything out synchronously right here. */
    mpuxxxx_status_t ret = mpuxxxx_get_all_data(p_driver,
                                                (mpuxxxx_data_t *)p_data);
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx get all data failed in interrupt callback!");
    }
#else
    (void)p_data;

    if (NULL == p_driver->ctx                                        ||
        NULL == p_driver->ctx->notify_handle                         ||
        NULL == p_driver->deps                                       ||
        NULL == p_driver->deps->p_os_interface                       ||
        NULL == p_driver->deps->p_os_interface->pf_os_semaephore_notify_isr)
    {
        return;
    }

    long higher_priority_task_woken = 0;
    /* eAction = 1 : OSAL_NOTIFY_SET_BITS (mirrors FreeRTOS eSetBits).
       Any non-zero value wakes the handler; bits are cleared on wait exit. */
    p_driver->deps->p_os_interface->pf_os_semaephore_notify_isr(
                                          p_driver->ctx->notify_handle,
                                          1U,
                                          1U,
                                          &higher_priority_task_woken);
#endif // !(OS_SUPPORTING)
}

/**
 * @brief      I2C RX-DMA completion trampoline. Completion is waited on
 *             inside the I2C abstraction; the hook stays so the HAL weak
 *             callback path keeps a target.
 *
 * @param[in]  this   : Driver instance (unused).
 *
 * @param[in]  p_data : Unused.
 *
 * @param[out]        : None.
 *
 * @return     None.
 * */
void dma_interrupt_callback(void const * const this, void * const p_data)
{
    (void)this;
    (void)p_data;
}

/*============================ Instantiation ================================*/

/**
 * @brief      Validate that every binding this driver actually calls is
 *             present, before anything is mounted.
 *
 * @param[in]  p_deps : Board bindings.
 *
 * @param[out]        : None.
 *
 * @return     MPUXXXX_OK or MPUXXXX_ERRORPARAMETER.
 * */
static mpuxxxx_status_t mpuxxxx_deps_validate(
                                     mpuxxxx_deps_t const * const p_deps)
{
    if (NULL == p_deps->p_iic_interface ||
        NULL == p_deps->p_interrupt_interface)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx deps input error parameter\n");
        return MPUXXXX_ERRORPARAMETER;
    }

    iic_driver_interface_t const * const p_iic = p_deps->p_iic_interface;

    if (NULL == p_iic->hi2c             ||
        NULL == p_iic->pf_iic_init      ||
        NULL == p_iic->pf_iic_deinit    ||
        NULL == p_iic->pf_iic_mem_write ||
        NULL == p_iic->pf_iic_mem_read  ||
        NULL == p_iic->pf_iic_mem_read_dma)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx iic_driver_interface input error parameter\n");
        return MPUXXXX_ERRORPARAMETER;
    }

#if OS_SUPPORTING
    if (NULL == p_deps->p_yield_interface                ||
        NULL == p_deps->p_yield_interface->pf_rtos_yield ||
        NULL == p_deps->p_os_interface)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx os / yield interface input error parameter\n");
        return MPUXXXX_ERRORPARAMETER;
    }
#endif // OS_SUPPORTING

    return MPUXXXX_OK;
}

/**
 * @brief      Bring the chip up: bus, hard reset, wake, WHO_AM_I.
 *
 * @param[in]  self : Driver instance with deps and ctx already mounted.
 *
 * @param[out]      : None.
 *
 * @return     mpuxxxx_status_t MPUXXXX_OK on success; error code otherwise.
 * */
static mpuxxxx_status_t mpuxxxx_bring_up(bsp_mpuxxxx_driver_t * const self)
{
    mpuxxxx_status_t ret = self->deps->p_iic_interface->pf_iic_init(
                                       self->deps->p_iic_interface->hi2c);
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx iic init failed!");
        return ret;
    }

    /* 1. Hard reset, then let the internal sequencer settle. */
    ret = mpuxxxx_write_byte(self, MPU_PWR_MGMT1_REG,
                             (UINT8_t)DEVICE_RESET_BIT(1));
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx device reset failed!");
        return ret;
    }

    mpuxxxx_yield(self, MPUXXXX_RESET_DELAY_MS);

    /* 2. Clear SLEEP so the register file answers. */
    ret = mpuxxxx_write_byte(self, MPU_PWR_MGMT1_REG, (UINT8_t)SLEEP_BIT(0));
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx wakeup failed!");
        return ret;
    }

    /* 3. WHO_AM_I before any tuning is written. */
    UINT8_t device_id = 0;

    ret = mpuxxxx_read_reg(self, MPU_DEVICE_ID_REG, &device_id, 1);
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx read device ID register failed!");
        return ret;
    }

    if (MPU_ID != device_id)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx device ID verify failed! read: 0x%02X, "
                  "expected: 0x%02X", device_id, MPU_ID);
        return MPUXXXX_ERROR;
    }

    return MPUXXXX_OK;
}

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
 * */
mpuxxxx_status_t bsp_mpuxxxx_driver_inst(
                        bsp_mpuxxxx_driver_t       * const     p_driver,
                        mpuxxxx_deps_t       const * const       p_deps,
                        mpuxxxx_cfg_t        const * const        p_cfg)
{
    DEBUG_OUT(i, MPUXXXX_LOG_TAG, "mpuxxxx driver inst start...\n");

    /************ 1. Checking input parameters ***********/
    if (NULL == p_driver || NULL == p_deps)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG,
                  "mpuxxxx driver inst input error parameter\n");
        return MPUXXXX_ERRORPARAMETER;
    }

    /************* 2. Checking the Resources *************/
    mpuxxxx_status_t ret = mpuxxxx_deps_validate(p_deps);
    if (MPUXXXX_OK != ret)
    {
        return ret;
    }

    /*************** 3. Claim a ctx slot *****************/
    mpuxxxx_ctx_t *p_ctx = mpuxxxx_ctx_claim();
    if (NULL == p_ctx)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx ctx pool exhausted!\n");
        return MPUXXXX_ERRORNOMEMORY;
    }

    /* Seed the cache so a read taken before the first apply_config cannot
       divide by zero; apply_config overwrites all three fields.          */
    p_ctx->cfg         = (NULL == p_cfg) ? s_cfg_default : *p_cfg;
    p_ctx->gyro_scale  = s_gyro_scale_lut [0];
    p_ctx->accel_scale = s_accel_scale_lut[0];

#if OS_SUPPORTING
    /* *_inst() runs on the handler thread, so this is the task the EXTI
       trampoline has to wake. V1 had the handler poke the driver object
       from outside; with ctx opaque, that binding belongs in here.       */
    if (NULL != p_deps->p_os_interface->pf_os_get_task_handle)
    {
        p_ctx->notify_handle =
                          p_deps->p_os_interface->pf_os_get_task_handle();
    }
#endif // OS_SUPPORTING

    /************** 4. Mount the interfaces **************/
    bsp_mpuxxxx_driver_t driver = {0};

    driver.deps = p_deps;
    driver.ctx  = p_ctx;
    driver.ops  = &s_mpuxxxx_ops;

    /************** 5. Bring the chip up *****************/
    ret = mpuxxxx_bring_up(&driver);
    if (MPUXXXX_OK != ret)
    {
        mpuxxxx_ctx_release(p_ctx);
        return ret;
    }

    /* is_init has to precede apply_config: every ops entry guards on it. */
    p_ctx->is_init = true;

    ret = mpuxxxx_apply_config(&driver, p_cfg);
    if (MPUXXXX_OK != ret)
    {
        DEBUG_OUT(e, MPUXXXX_ERR_LOG_TAG, "mpuxxxx apply config failed!\n");
        mpuxxxx_ctx_release(p_ctx);
        return ret;
    }

    /************** 6. Publish and arm ISRs **************/
    *p_driver = driver;

    /* Registered only now, so a stray edge cannot reach a half-built ctx. */
    if (NULL != p_deps->pf_callback_register)
    {
        p_deps->pf_callback_register(int_interrupt_callback);
    }
    if (NULL != p_deps->pf_callback_register_dma)
    {
        p_deps->pf_callback_register_dma(dma_interrupt_callback);
    }

    DEBUG_OUT(i, MPUXXXX_LOG_TAG, "mpuxxxx driver inst success\n");

    return ret;
}

//******************************* Declaring *********************************//

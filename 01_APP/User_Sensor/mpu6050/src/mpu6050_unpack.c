/******************************************************************************
 * @file mpu6050_unpack.c
 *
 * @par dependencies
 * - bsp_wrapper_motion.h
 * - Debug.h
 *
 * @author Ethan-Hang
 *
 * @brief FreeRTOS task that unpacks raw MPU6050 DMA packets.
 *        Uses the motion wrapper API; has no direct dependency on the
 *        MPU6050 driver or circular buffer implementation.
 *
 * Processing flow:
 *   1. motion_drv_get_req() blocks until a DMA packet is ready.
 *   2. motion_get_data_addr() retrieves the raw 14-byte packet address.
 *   3. Accel / gyro / temperature fields are extracted and converted.
 *   4. motion_read_data_done() advances the ring buffer read pointer.
 *
 * Packet layout (MPU6050, 14 bytes):
 *   [0:1]   ACCEL_X  [2:3]   ACCEL_Y  [4:5]   ACCEL_Z
 *   [6:7]   TEMP
 *   [8:9]   GYRO_X   [10:11] GYRO_Y   [12:13] GYRO_Z
 *
 * @version V1.0 2026-04-16
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "bsp_wrapper_motion.h"

#include "osal_task.h"

#include "Debug.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//

/**
 * @brief   FreeRTOS unpack task for MPU6050 motion data.
 *          Loops indefinitely: waits for a DMA-complete notification,
 *          parses the raw packet, prints the result, then releases the slot.
 *
 * @param[in] argument : Unused task argument.
 */
void unpack_task_thread(void *argument)
{
    // osal_task_delay(200);
    (void)argument;

    DEBUG_OUT(i, UNPACK_LOG_TAG, "unpack_task is running...");

    wp_motion_status_t ret          = WP_MOTION_OK;
    int16_t            temperature  =                0;

    /* Parsed output values */
    float  temperature_c = 0.0f;
    int16_t accel_x_raw  = 0, accel_y_raw = 0, accel_z_raw = 0;
    int16_t gyro_x_raw   = 0, gyro_y_raw  = 0, gyro_z_raw  = 0;
    float  accel_x_g     = 0.0f, accel_y_g = 0.0f, accel_z_g = 0.0f;
    float  gyro_x_dps    = 0.0f, gyro_y_dps = 0.0f, gyro_z_dps = 0.0f;

    for (;;)
    {
        /* Block until the handler task sends a DMA-complete notification. */
        ret = motion_drv_get_req();
        if (WP_MOTION_OK != ret)
        {
            DEBUG_OUT(e, UNPACK_ERR_LOG_TAG,
                      "motion_drv_get_req failed, ret=%d", (int)ret);
            continue;
        }

        /* Obtain a pointer to the raw 14-byte packet. */
        uint8_t *addr = motion_get_data_addr();
        if (NULL == addr)
        {
            DEBUG_OUT(e, UNPACK_ERR_LOG_TAG, "motion_get_data_addr returned NULL");
            motion_read_data_done();
            continue;
        }

        DEBUG_OUT(i, UNPACK_LOG_TAG, "unpack_task: addr = %p", addr);

        /* ---- Parse raw bytes (big-endian, MSB first) ---- */
        accel_x_raw  = (int16_t)(*(addr + 0)  << 8 | *(addr + 1));
        accel_y_raw  = (int16_t)(*(addr + 2)  << 8 | *(addr + 3));
        accel_z_raw  = (int16_t)(*(addr + 4)  << 8 | *(addr + 5));
        temperature  = (int16_t)(*(addr + 6)  << 8 | *(addr + 7));
        gyro_x_raw   = (int16_t)(*(addr + 8)  << 8 | *(addr + 9));
        gyro_y_raw   = (int16_t)(*(addr + 10) << 8 | *(addr + 11));
        gyro_z_raw   = (int16_t)(*(addr + 12) << 8 | *(addr + 13));

        /* ---- Convert to engineering units ---- */
        temperature_c = 36.53f + (float)temperature / 340.0f;

        accel_x_g  = (float)accel_x_raw / 16384.0f;   /* ±2 g FSR */
        accel_y_g  = (float)accel_y_raw / 16384.0f;
        accel_z_g  = (float)accel_z_raw / 16384.0f;

        gyro_x_dps = (float)gyro_x_raw  /   131.0f;   /* ±250 dps FSR */
        gyro_y_dps = (float)gyro_y_raw  /   131.0f;
        gyro_z_dps = (float)gyro_z_raw  /   131.0f;

        DEBUG_OUT(i, UNPACK_LOG_TAG,
                  "UnpackThread\r\n temp=%.2f C\r\n "
                  "ax=%.3f g  ay=%.3f g  az=%.3f g\r\n "
                  "gx=%.2f dps  gy=%.2f dps  gz=%.2f dps",
                  (double)temperature_c,
                  (double)accel_x_g,  (double)accel_y_g,  (double)accel_z_g,
                  (double)gyro_x_dps, (double)gyro_y_dps, (double)gyro_z_dps);

        /* Release the circular buffer slot so the writer can reuse it. */
        motion_read_data_done();
        // osal_task_delay(1000);
    }
}

//******************************* Functions *********************************//

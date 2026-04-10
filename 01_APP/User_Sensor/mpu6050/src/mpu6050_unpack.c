#include "bsp_mpuxxxx_handler.h"
#include "circular_buffer.h"

extern circular_buffer_t    g_circular_buffer;
extern bsp_mpuxxxx_hanlder_t g_mpuxxxx_handler_instance;

void unpack_task_thread(void *argument)
{
    DEBUG_OUT(i, UNPACK_LOG_TAG, "unpack_task is running...");

    mpuxxxx_status_t ret           = MPUXXXX_OK;
    uint8_t          received_data =  0;
    int16_t          temperature   =  0;
    mpuxxxx_data_t   mpu6050_data  = {0};
    for (;;)
    {
        ret = g_mpuxxxx_handler_instance.p_input_args->p_os_interface->\
                                                           pf_os_queue_receive(
                             g_mpuxxxx_handler_instance.p_unpack_queue_handler,
                                                                &received_data,
                                                                    0xFFFFFFFF);
        if (MPUXXXX_OK != ret)
        {
            DEBUG_OUT(e, UNPACK_ERR_LOG_TAG,
                      "mpuxxxx unpack task receive queue failed!");
        }
        uint8_t *addr =
            g_circular_buffer.pf_get_rbuffer_addr(&g_circular_buffer);
        DEBUG_OUT(i, UNPACK_LOG_TAG, "unpack_task: addr = %p\n", addr);

        temperature                = (int16_t)(*(addr + 6) << 8 | *(addr + 7));
        mpu6050_data.temperature_c = 36.53 + temperature / 340.0;

        mpu6050_data.accel_x_raw   = (int16_t)(*(addr + 0) << 8 | *(addr + 1));
        mpu6050_data.accel_y_raw   = (int16_t)(*(addr + 2) << 8 | *(addr + 3));
        mpu6050_data.accel_z_raw   = (int16_t)(*(addr + 4) << 8 | *(addr + 5));

        mpu6050_data.accel_x_g     = mpu6050_data.accel_x_raw / 16384.0;
        mpu6050_data.accel_y_g     = mpu6050_data.accel_y_raw / 16384.0;
        mpu6050_data.accel_z_g     = mpu6050_data.accel_z_raw / 16384.0;

        mpu6050_data.gyro_x_raw    = (int16_t)(*(addr + 8 ) << 8 | *(addr + 9 ));
        mpu6050_data.gyro_y_raw    = (int16_t)(*(addr + 10) << 8 | *(addr + 11));
        mpu6050_data.gyro_z_raw    = (int16_t)(*(addr + 12) << 8 | *(addr + 13));

        mpu6050_data.gyro_x_dps    = mpu6050_data.gyro_x_raw / 131.0;
        mpu6050_data.gyro_y_dps    = mpu6050_data.gyro_y_raw / 131.0;
        mpu6050_data.gyro_z_dps    = mpu6050_data.gyro_z_raw / 131.0;

        DEBUG_OUT(i, UNPACK_LOG_TAG,"UnpackThread\r\n temp=%f\r\n "
                   "ax=%f\r\n ay=%f\r\n az=%f\r\n gx=%f\r\n gy=%f\r\n gz=%f\r\n" ,
                    mpu6050_data.temperature_c, mpu6050_data.accel_x_g ,
                    mpu6050_data.accel_y_g    , mpu6050_data.accel_z_g ,
                    mpu6050_data.gyro_x_dps   , mpu6050_data.gyro_y_dps,
                    mpu6050_data.gyro_z_dps);

        g_circular_buffer.pf_data_readed(&g_circular_buffer);
    }
}
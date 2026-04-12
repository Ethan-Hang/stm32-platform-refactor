/******************************************************************************
 * @file
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
 * @version V1.0 2026--
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "user_task_reso_config.h"

#include "bsp_mpuxxxx_handler.h"
#include "bsp_temp_humi_xxx_handler.h"
#include "bsp_adapter_port_temp_humi.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//

//******************************** Defines **********************************//

//******************************* Declaring *********************************//
extern mpuxxxx_handler_input_args_t   mpu6050_input_args;
extern temp_humi_handler_input_arg_t  input_arg;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
void mpu6050_handler_thread(void *argument);
void unpack_task_thread(void *argument);
void temp_humi_handler_thread(void *argument);

usertaskcfg_t g_user_task_cfg[USER_TASK_NUM] =
{
    {"mpu6050_handler_thread",   mpuxxxx_handler_thread,      1024, PRI_NORMAL+1, NULL, &mpu6050_input_args},
    {"unpack_task_thread",       unpack_task_thread,           1024, PRI_NORMAL, NULL, NULL               },
    {"temp_humi_handler_thread", temp_humi_handler_thread,      1024, PRI_NORMAL, NULL, &input_arg         },
};

__attribute__((weak)) void mpu6050_handler_thread(void *argument)
{
    for (;;)
    {
        osal_task_delay(1000);
    }
}

__attribute__((weak)) void unpack_task_thread(void *argument)
{
    for (;;)
    {
        osal_task_delay(1000);
    }
}

__attribute__((weak)) void temp_humi_handler_thread(void *argument)
{
    for (;;)
    {
        osal_task_delay(1000);
    }
}

//******************************* Functions *********************************//

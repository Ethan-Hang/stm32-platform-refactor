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

//******************************** Includes *********************************//

//******************************** Defines **********************************//

//******************************** Defines **********************************//

//******************************* Declaring *********************************//
extern mpuxxxx_handler_input_args_t mpu6050_input_args;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
void mpu6050_handler_thread(void *argument);
void unpack_task_thread(void *argument);

usertaskcfg_t user_task_cfg[USER_TASK_NUM] =
{
    {"mpu6050_handler_thread", mpuxxxx_handler_thread, 1024, PRI_NORMAL, NULL, &mpu6050_input_args},
    {"unpack_task_thread", unpack_task_thread, 1024, PRI_NORMAL, NULL, NULL},
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

//******************************* Functions *********************************//

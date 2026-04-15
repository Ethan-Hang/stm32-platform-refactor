/******************************************************************************
 * @file user_task_reso_config.c
 *
 * @par dependencies
 *
 * @author Ethan-Hang
 *
 * @brief Configure user task resource table and weak fallback task entries.
 *
 * Processing flow:
 * Define g_user_task_cfg and provide weak default task functions.
 * @version V1.0 2026-04-10
 * @version V2.0 2026-04-12
 * @upgrade 2.0: Added temp_humi_test_task_a / _b for concurrent sync-read
 *               concurrency validation.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "user_task_reso_config.h"

#include "bsp_mpuxxxx_handler.h"
#include "aht21_integration.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//

//******************************** Defines **********************************//

//******************************* Declaring *********************************//
extern mpuxxxx_handler_input_args_t   mpu6050_input_args;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
void mpuxxxx_handler_thread(void *argument);
void unpack_task_thread(void *argument);
void temp_humi_handler_thread(void *argument);
void temp_humi_test_task_a(void *argument);
void temp_humi_test_task_b(void *argument);
void task_higher_water_thread(void *argument);

usertaskcfg_t g_user_task_cfg[USER_TASK_NUM] =
{
    [USER_TASK_MPU6050_HANDLER] = {
        .task_name = "mpu6050_handler_thread",
        .func_pointer = mpuxxxx_handler_thread,
        .stack_depth = 512,
        .priority = PRI_NORMAL + 1,
        .task_handle = NULL,
        .argument = &mpu6050_input_args
    },
    [USER_TASK_UNPACK_TASK] = {
        .task_name = "unpack_task_thread",
        .func_pointer = unpack_task_thread,
        .stack_depth = 512,
        .priority = PRI_NORMAL,
        .task_handle = NULL,
        .argument = NULL
    },

    [USER_TASK_TEMP_HUMI_HANDLER] = {
        .task_name = "temp_humi_handler_thread",
        .func_pointer = temp_humi_handler_thread,
        .stack_depth = 512,
        .priority = PRI_NORMAL,
        .task_handle = NULL,
        .argument = &aht21_input_arg
    },

    [USER_TASK_TASK_HIGHER_WATER] = {
        .task_name = "task_higher_water_thread",
        .func_pointer = task_higher_water_thread,
        .stack_depth = 512,
        .priority = PRI_NORMAL + 2,
        .task_handle = NULL,
        .argument = &aht21_input_arg
    }

    // [USER_TASK_TEMP_HUMI_TEST_A] = {
    //     .task_name = "temp_humi_test_task_a",
    //     .func_pointer = temp_humi_test_task_a,
    //     .stack_depth = 1024,
    //     .priority = PRI_NORMAL,
    //     .task_handle = NULL,
    //     .argument = NULL
    // },

    // [USER_TASK_TEMP_HUMI_TEST_B] = {
    //     .task_name = "temp_humi_test_task_b",
    //     .func_pointer = temp_humi_test_task_b,
    //     .stack_depth = 1024,
    //     .priority = PRI_NORMAL,
    //     .task_handle = NULL,
    //     .argument = NULL
    // },

};

__attribute__((weak)) void mpuxxxx_handler_thread(void *argument)
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

__attribute__((weak)) void task_higher_water_thread(void *argument)
{
    for (;;)
    {
        osal_task_delay(1000);
    }
}

__attribute__((weak)) void temp_humi_test_task_a(void *argument)
{
    for (;;)
    {
        osal_task_delay(1000);
    }
}

__attribute__((weak)) void temp_humi_test_task_b(void *argument)
{
    for (;;)
    {
        osal_task_delay(1000);
    }
}

//******************************* Functions *********************************//

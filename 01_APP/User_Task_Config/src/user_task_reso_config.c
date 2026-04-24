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

#include "aht21_integration.h"
#include "mpu6050_integration.h"
#include "wt588_integration.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//

//******************************** Defines **********************************//

//******************************* Declaring *********************************//

//******************************* Declaring *********************************//

//******************************* Functions *********************************//
void mpuxxxx_handler_thread(void *argument);
void unpack_task_thread(void *argument);
void temp_humi_handler_thread(void *argument);
void wt588_handler_thread(void *argument);

void temp_humi_test_task_a(void *argument);
void temp_humi_test_task_b(void *argument);
void task_higher_water_thread(void *argument);
void wt588_test_task(void *argument);
void st7789_hal_test_task(void *argument);
void lvgl_display_task(void *argument);

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

    [USER_TASD_WT588_HANDLER] = {
        .task_name = "wt588_handler_thread",
        .func_pointer = wt588_handler_thread,
        .stack_depth = 512,
        .priority = PRI_NORMAL - 2,
        .task_handle = NULL,
        .argument = &wt588_handler_input_args
    },

    [USER_TASK_TASK_HIGHER_WATER] = {
        .task_name = "task_higher_water_thread",
        .func_pointer = task_higher_water_thread,
        .stack_depth = 512,
        .priority = PRI_NORMAL + 2,
        .task_handle = NULL,
        .argument = NULL
    },

    [USER_TASK_TEMP_HUMI_TEST_A] = {
        .task_name = "temp_humi_test_task_a",
        .func_pointer = temp_humi_test_task_a,
        .stack_depth = 512,
        .priority = PRI_NORMAL,
        .task_handle = NULL,
        .argument = NULL
    },

    [USER_TASK_TEMP_HUMI_TEST_B] = {
        .task_name = "temp_humi_test_task_b",
        .func_pointer = temp_humi_test_task_b,
        .stack_depth = 512,
        .priority = PRI_NORMAL,
        .task_handle = NULL,
        .argument = NULL
    },

    [USER_TASK_WT588_TEST] = {
        .task_name = "wt588_test_task",
        .func_pointer = wt588_test_task,
        .stack_depth = 256,
        .priority = PRI_HARD_REALTIME,
        .task_handle = NULL,
        .argument = NULL
    },

    [USER_TASK_LVGL_DISPLAY] = {
        .task_name = "lvgl_display_task",
        .func_pointer = lvgl_display_task,
        .stack_depth = 2048,
        .priority = PRI_NORMAL,
        .task_handle = NULL,
        .argument = NULL
    },

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

__attribute__((weak)) void wt588_handler_thread(void *argument)
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

__attribute__((weak)) void wt588_test_task(void *argument)
{
    for (;;)
    {
        osal_task_delay(1000);
    }
}

__attribute__((weak)) void st7789_hal_test_task(void *argument)
{
    for (;;)
    {
        osal_task_delay(1000);
    }
}

__attribute__((weak)) void lvgl_display_task(void *argument)
{
    for (;;)
    {
        osal_task_delay(1000);
    }
}

//******************************* Functions *********************************//

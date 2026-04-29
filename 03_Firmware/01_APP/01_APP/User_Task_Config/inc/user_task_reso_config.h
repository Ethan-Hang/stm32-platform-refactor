/******************************************************************************
 * @file user_task_reso_config.h
 *
 * @par dependencies
 *
 * @author Ethan-Hang
 *
 * @brief Define user task configuration types, priorities and task table size.
 *
 * Processing flow:
 * Expose usertaskcfg_t and task priority/resource macros for app init.
 * @version V1.0 2026-04-10
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __USER_TASK_RESO_CONFIG_H__
#define __USER_TASK_RESO_CONFIG_H__

//******************************** Includes *********************************//
#include "osal_wrapper_adapter.h"
#include "FreeRTOSConfig.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define PRI_EMERGENCY     (configMAX_PRIORITIES - 1)
#define PRI_HARD_REALTIME (PRI_EMERGENCY - 4)
#define PRI_SOFT_REALTIME (PRI_HARD_REALTIME - 5)
#define PRI_NORMAL        (PRI_SOFT_REALTIME - 7)
#define PRI_BACKGROUND    (1)
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
#define USER_TASK_MPU6050_HANDLER   1
#define USER_TASK_UNPACK_TASK       1
#define USER_TASK_TEMP_HUMI_HANDLER 1
#define USER_TASD_WT588_HANDLER     1
#define USER_TASK_TEMP_HUMI_TEST_A  1
#define USER_TASK_TEMP_HUMI_TEST_B  1
#define USER_LVGL_TEST_TASK         0
#define USER_TASK_WT588_TEST        1
#define USER_TASK_W25Q64_HANDLER    0
#define USER_TASK_W25Q64_HAL_TEST   0
#define USER_TASK_W25Q64_MOCK       0
#define USER_TASK_TASK_HIGHER_WATER 0
#define USER_TASK_EM7028_HAL_TEST   1

typedef enum
{
#if USER_TASK_MPU6050_HANDLER
    USER_TASK_MPU6050_HANDLER_IDX = 0,
#endif
#if USER_TASK_UNPACK_TASK
    USER_TASK_UNPACK_TASK_IDX,
#endif
#if USER_TASK_TEMP_HUMI_HANDLER
    USER_TASK_TEMP_HUMI_HANDLER_IDX,
#endif
#if USER_TASD_WT588_HANDLER
    USER_TASD_WT588_HANDLER_IDX,
#endif
#if USER_TASK_TASK_HIGHER_WATER
    USER_TASK_TASK_HIGHER_WATER_IDX,
#endif
#if USER_TASK_TEMP_HUMI_TEST_A
    USER_TASK_TEMP_HUMI_TEST_A_IDX,
#endif
#if USER_TASK_TEMP_HUMI_TEST_B
    USER_TASK_TEMP_HUMI_TEST_B_IDX,
#endif
#if USER_TASK_WT588_TEST
    USER_TASK_WT588_TEST_IDX,
#endif
#if USER_LVGL_TEST_TASK
    USER_LVGL_TEST_TASK_IDX,
#endif
#if USER_TASK_W25Q64_MOCK
    USER_TASK_W25Q64_MOCK_IDX,
#endif
#if USER_TASK_W25Q64_HANDLER
    USER_TASK_W25Q64_HANDLER_IDX,
#endif
#if USER_TASK_W25Q64_HAL_TEST
    USER_TASK_W25Q64_HAL_TEST_IDX,
#endif
#if USER_TASK_EM7028_HAL_TEST
    USER_TASK_EM7028_HAL_TEST_IDX,
#endif
    USER_TASK_NUM
} usertaskid_t;

typedef struct
{
    const char        *      task_name;
    osal_task_entry_t     func_pointer;
    size_t                 stack_depth;
    uint32_t                  priority;
    osal_task_handle_t     task_handle;
    void              *       argument;
} usertaskcfg_t;

//******************************* Declaring *********************************//

//******************************* Functions *********************************//

//******************************* Functions *********************************//

#endif /* __USER_TASK_RESO_CONFIG_H__ */

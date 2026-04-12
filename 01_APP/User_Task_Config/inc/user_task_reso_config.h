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
 * @version V1.0 2026--
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
#define USER_TASK_NUM     5
#define PRI_EMERGENCY     (configMAX_PRIORITIES - 1)
#define PRI_HARD_REALTIME (PRI_EMERGENCY - 4)
#define PRI_SOFT_REALTIME (PRI_HARD_REALTIME - 5)
#define PRI_NORMAL        (PRI_SOFT_REALTIME - 7)
#define PRI_BACKGROUND    (1)
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
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

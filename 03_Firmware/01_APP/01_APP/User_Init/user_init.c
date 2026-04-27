/******************************************************************************
 * @file user_init.c
 *
 * @par dependencies
 *
 * @author Ethan-Hang
 *
 * @brief Create application tasks from the user task configuration table.
 *
 * Processing flow:
 * Start init task, create configured tasks, rollback on failure, then exit.
 * @version V1.0 2026--
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "osal_wrapper_adapter.h"

#include "user_task_reso_config.h"
#include "user_init.h"
#include "Debug.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//

//******************************** Defines **********************************//

//******************************* Declaring *********************************//
osal_task_handle_t user_init_task_handle;

extern usertaskcfg_t g_user_task_cfg[USER_TASK_NUM];
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
static void user_init_task_function(void *argument)
{
    int32_t ret = 0;

    for (int8_t i = 0; i < USER_TASK_NUM; i++)
    {
        ret = osal_task_create(
                    &g_user_task_cfg[i].task_handle,
                     g_user_task_cfg[i].task_name,
                     g_user_task_cfg[i].argument,
                     g_user_task_cfg[i].func_pointer,
                     g_user_task_cfg[i].stack_depth,
                     g_user_task_cfg[i].priority);
        if (ret == 0)
        {
            DEBUG_OUT(i, USER_INIT_LOG_TAG,
                      "task [%s] created ok", g_user_task_cfg[i].task_name);
        }
        else
        {
            DEBUG_OUT(e, USER_INIT_ERR_LOG_TAG,
                      "task [%s] create failed, ret=%d",
                      g_user_task_cfg[i].task_name, (int)ret);
            while (i >= 0)
            {
                osal_task_delete(g_user_task_cfg[i--].task_handle);
            }
        }
    }

    osal_task_delete(user_init_task_handle);
}

void user_apptask_init(void)
{
    osal_task_create(&user_init_task_handle, "user_Init_Task", NULL,
                   user_init_task_function, 1024, PRI_HARD_REALTIME);
}

//******************************* Functions *********************************//

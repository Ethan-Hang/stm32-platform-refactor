/******************************************************************************
 * @file os_impl_kernel.c
 *
 * @par dependencies
 * - osal_kernel.h
 * - osal_error.h
 * - FreeRTOS.h
 * - task.h
 * - cmsis_os2.h
 *
 * @author Ethan-Hang
 *
 * @brief OSAL kernel bring-up implementation for FreeRTOS.
 *
 * @note This file replaces what CubeMX put in Core/Src/freertos.c. The task
 *       creation that used to live in MX_FREERTOS_Init() moved to
 *       app_task_init() in the application layer, which is backend-neutral;
 *       what is left here is the FreeRTOS-specific bring-up plus the stack
 *       overflow hook the kernel demands.
 *
 * @version V1.0 2026-8-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "board_types.h"
#include "osal_kernel.h"
#include "osal_error.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//
/**
 * @brief Bring up FreeRTOS so that objects can be created.
 *
 * @note FreeRTOS builds its heap and object lists lazily, so there is nothing
 *       to allocate here; osKernelInitialize() only moves the CMSIS-RTOS
 *       state machine out of "inactive" so osKernelStart() will be accepted.
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_ERROR.
 */
int32_t osal_kernel_init(void)
{
    if (osOK != osKernelInitialize())
    {
        return OSAL_ERROR;
    }

    return OSAL_SUCCESS;
}

/**
 * @brief Hand control to the FreeRTOS scheduler.
 *
 * @return OSAL_ERROR; a return at all means the scheduler did not start,
 *         which for FreeRTOS means the idle task or timer task could not be
 *         allocated.
 */
int32_t osal_kernel_start(void)
{
    (void)osKernelStart();

    return OSAL_ERROR;
}

/**
 * @brief FreeRTOS stack overflow hook.
 *
 * @note Required by the kernel because configCHECK_FOR_STACK_OVERFLOW is 2;
 *       stack_macros.h references it unconditionally. RT-Thread has no
 *       counterpart -- RT_USING_OVERFLOW_CHECK asserts inside the kernel --
 *       which is why this lives in the backend-specific file.
 *
 *       Spins with interrupts off so a debugger can read pcTaskName off the
 *       call stack; returning would resume a task whose stack is already
 *       corrupt.
 *
 * @param[in] xTask Handle of the offending task.
 * @param[in] pcTaskName Name of the offending task.
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;

    taskDISABLE_INTERRUPTS();
    for (;;)
    {
    }
}

//******************************* Functions *********************************//

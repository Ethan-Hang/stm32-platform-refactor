/******************************************************************************
 * @file os_impl_kernel.c
 *
 * @par dependencies
 * - osal_kernel.h
 * - osal_error.h
 * - os_rtthread.h
 *
 * @author Ethan-Hang
 *
 * @brief OSAL kernel bring-up implementation for RT-Thread.
 *
 * @note RT_USING_USER_MAIN is off, so components.c and its $Sub$$main entry
 *       are not vendored: this file performs the startup sequence that
 *       rtthread_startup() would otherwise run, split across init and start
 *       so the application can create its tasks in between.
 *
 *       The tick source lives here too. HAL drives its own timebase from TIM1
 *       (Core/Src/stm32f4xx_hal_timebase_tim.c overrides HAL_InitTick), so
 *       SysTick is free for the kernel and the two never contend for it.
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

#include "os_rtthread.h"

#include "stm32f4xx.h"
#include "SEGGER_RTT.h"
#include "SEGGER_SYSVIEW_RTThread.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/**
 * @brief Size of the RT-Thread system heap.
 *
 * @note A static .bss array of exactly configTOTAL_HEAP_SIZE, deliberately
 *       rather than RT-Thread's usual "__bss_end to the end of RAM". Two
 *       reasons:
 *
 *       - the two backends then have identical dynamic memory, so a failure
 *         seen on one and not the other is a real behavioural difference and
 *         not a memory-budget artefact;
 *       - overrunning RAM fails at link time, loudly. Sizing the heap from
 *         whatever is left over instead lets a growing .bss quietly shrink
 *         the heap until an allocation fails at runtime.
 *
 *       If this is ever changed to span the leftovers, it must stop below
 *       _estack by at least _Min_Stack_Size (the MSP stack that ISRs run on)
 *       and must never reach RTT_RAM at 0x2001E400, which holds the RTT
 *       control block and SystemView buffers shared with the bootloader.
 */
#define OSAL_RTT_HEAP_SIZE (16U * 1024U)
//******************************** Defines **********************************//

//****************************** Local Variables ****************************//
/* 8-byte aligned to match RT_ALIGN_SIZE. */
static rt_uint8_t s_rtt_heap[OSAL_RTT_HEAP_SIZE] __attribute__((aligned(8)));
//****************************** Local Variables ****************************//

//******************************* Functions *********************************//
/**
 * @brief Bring up RT-Thread so that objects can be created.
 *
 * @note Interrupts are deliberately left enabled, unlike rtthread_startup(),
 *       which masks them for the whole of board and application init. The
 *       peripherals are already initialised and their ISRs already armed by
 *       the time main() reaches here, and the driver init that follows in
 *       app_task_init() uses HAL delays that need the TIM1 timebase running.
 *       This matches what the FreeRTOS backend does between
 *       osKernelInitialize() and osKernelStart(), so the exposure window is
 *       the same on both and the two builds stay comparable.
 *
 * @return OSAL_SUCCESS on success, otherwise OSAL_ERROR.
 */
int32_t osal_kernel_init(void)
{
    /* The heap has to exist first: rt_system_scheduler_init() and every
       object created by app_task_init() allocate from it. */
    rt_system_heap_init((void *)s_rtt_heap,
                        (void *)(s_rtt_heap + OSAL_RTT_HEAP_SIZE));

    rt_system_timer_init();
    rt_system_scheduler_init();

    return OSAL_SUCCESS;
}

/**
 * @brief Hand control to the RT-Thread scheduler.
 *
 * @return OSAL_ERROR; a return at all means the scheduler did not start.
 */
int32_t osal_kernel_start(void)
{
    /* Soft timer daemon: created here rather than in init so it cannot be
       scheduled before the application's tasks exist. */
    rt_system_timer_thread_init();
    rt_thread_idle_init();

    /* SystemView's RT-Thread hooks. Deliberately after rt_thread_idle_init():
       the integration caches the idle thread handle to recognise idle time,
       so the handle has to exist. SEGGER_SYSVIEW_Conf() itself already ran
       from main(), before the kernel was brought up. */
    (void)SEGGER_SYSVIEW_RTThread_Init();

    /* From here to the first thread switch the kernel must not be reentered.
     *
     * SysTick is enabled a few lines below but the scheduler only starts at
     * the end of this function, so without this mask a tick can land in
     * between: rt_tick_increase() calls rt_schedule(), which races
     * rt_hw_context_switch_to() for rt_thread_switch_interrupt_flag and
     * rt_interrupt_to_thread. rt_hw_context_switch() skips updating
     * from_thread when the flag is already set (context_gcc.S), so a tick
     * landing in that window makes PendSV save or restore against the wrong
     * thread -- which shows up later as a callee-saved register that did not
     * survive a switch, with no fault to point at.
     *
     * This is what rtthread_startup() does with its rt_hw_interrupt_disable()
     * at the top; interrupts come back on inside rt_system_scheduler_start(),
     * whose rt_hw_context_switch_to() ends in CPSIE I. Masking only here, and
     * not in osal_kernel_init(), keeps interrupts live for app_task_init()'s
     * driver init, which needs the HAL timebase running.
     */
    (void)rt_hw_interrupt_disable();

    /* PendSV must be the lowest-priority exception on the core: it is where
     * the context switch happens, and it has to run only after every ISR has
     * finished. Cortex-M resets it to priority 0, the highest.
     *
     * RT-Thread does set this itself, but not until rt_hw_context_switch_to()
     * inside rt_system_scheduler_start() (context_gcc.S writes NVIC_SYSPRI2).
     * That is after SysTick is enabled below, so it is set here to close the
     * gap rather than relying on when the vendor code gets round to it.
     */
    NVIC_SetPriority(PendSV_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);

    /* Kernel tick. SysTick_Config() sets the reload, clears the counter and
       enables the interrupt; it also drops SysTick to the lowest priority,
       which is set again here so the intent is not buried in CMSIS. */
    if (0U != SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND))
    {
        return OSAL_ERROR;
    }

    NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);

    rt_system_scheduler_start();

    return OSAL_ERROR;
}

/**
 * @brief Kernel console sink for rt_kprintf().
 *
 * @note Overrides the RT_WEAK stub in kservice.c. Everything the kernel says
 *       comes through here: RT_ASSERT failures, the hard fault register dump
 *       from rt_hw_hard_fault_exception(), and the startup banner.
 *
 *       Writes straight to SEGGER RTT rather than through EasyLogger, because
 *       the two most important callers run in fault and assert context, where
 *       the logging framework's buffering and the RTOS itself cannot be
 *       relied on. Output lands on terminal 0, the same catch-all terminal
 *       unrouted project tags use.
 *
 *       The terminal is not switched: log_sink_rtt selects a terminal, writes
 *       and restores 0, so 0 is the resting state. Console output racing with
 *       that sequence could land on another terminal, which is harmless and
 *       cannot happen in the fault path anyway, where nothing else runs.
 *
 *       RT-Thread terminates its lines with a bare "\n"; RTT Viewer expects
 *       "\r\n", so runs are emitted between the translations.
 *
 * @param[in] str NUL-terminated string to emit.
 */
void rt_hw_console_output(const char *str)
{
    const char *p_run = str;
    const char *p_cur = str;

    if (RT_NULL == str)
    {
        return;
    }

    while ('\0' != *p_cur)
    {
        if ('\n' == *p_cur)
        {
            if (p_cur > p_run)
            {
                SEGGER_RTT_Write(0, p_run, (unsigned)(p_cur - p_run));
            }
            SEGGER_RTT_Write(0, "\r\n", 2u);
            p_run = p_cur + 1;
        }
        p_cur++;
    }

    if (p_cur > p_run)
    {
        SEGGER_RTT_Write(0, p_run, (unsigned)(p_cur - p_run));
    }
}

/**
 * @brief Kernel tick interrupt.
 *
 * @note Under the FreeRTOS backend this vector is supplied by cmsis_os2.c;
 *       here it belongs to the kernel bring-up. rt_interrupt_enter/leave
 *       bracket it so rt_tick_increase() can reschedule on the way out.
 */
void SysTick_Handler(void)
{
    rt_interrupt_enter();
    rt_tick_increase();
    rt_interrupt_leave();
}

//******************************* Functions *********************************//

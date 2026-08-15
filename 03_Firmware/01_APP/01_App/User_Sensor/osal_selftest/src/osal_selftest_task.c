/******************************************************************************
 * @file osal_selftest_task.c
 *
 * @par dependencies
 * - osal_wrapper_adapter.h
 * - Debug.h
 *
 * @author Ethan-Hang
 *
 * @brief Backend-agnostic OSAL conformance test.
 *
 * Exercises every OSAL primitive the firmware relies on and prints a PASS/FAIL
 * table to RTT. Written against osal_* only -- no rt_* or x* calls -- so the
 * same binary logic runs on both backends and the two runs can be diffed line
 * by line. A case that passes under FreeRTOS and fails under RT-Thread is a
 * real semantic difference, not a test artefact.
 *
 * Three cases exist because the RT-Thread mapping had to make a decision that
 * could silently be wrong:
 *
 *   priority order   OSAL keeps the FreeRTOS convention (larger = higher) and
 *                    the RT-Thread implementation reflects it, because
 *                    RT-Thread numbers priorities the other way. Getting this
 *                    backwards still boots: it surfaces as watchdog resets and
 *                    dropped frames much later. The test pins the ordering
 *                    down by observing which task actually runs first.
 *   mailbox overwrite RT-Thread has no xQueueOverwrite, so the implementation
 *                    drains and refills under a scheduler lock. The test
 *                    checks that a full mailbox is replaced rather than
 *                    rejected.
 *   task notify      RT-Thread has no per-thread notification value; the
 *                    implementation embeds an rt_event in each task. The test
 *                    covers the task-context path (the ISR path needs real
 *                    interrupt context and is exercised by the touch and
 *                    motion drivers).
 *
 * @version V1.0 2026-8-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "osal_wrapper_adapter.h"
#include "platform_type.h"
#include "platform_def.h"
#include "osal_error.h"

#include "osal_selftest_task.h"
#include "user_task_reso_config.h"
#include "Debug.h"

#include <string.h>
//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define SELFTEST_CASE_MAX        (16U)

/** @brief Generous enough that a scheduling hiccup does not read as failure. */
#define SELFTEST_WAIT_TICKS      (1000U)

/** @brief Notification bit used by the notify case. */
#define SELFTEST_NOTIFY_BIT      (1UL << 3)

/** @brief Event group bits used by the event case. */
#define SELFTEST_EVENT_BIT_A     (1UL << 0)
#define SELFTEST_EVENT_BIT_B     (1UL << 1)

/** @brief Items exchanged through the queue case. */
#define SELFTEST_QUEUE_DEPTH     (4U)
#define SELFTEST_QUEUE_ROUNDS    (5U)

/** @brief Period and settle time for the timer case. */
#define SELFTEST_TIMER_PERIOD    (50U)
#define SELFTEST_TIMER_SETTLE    (260U)

/** @brief Stack depth for the helper tasks, in words. */
#define SELFTEST_HELPER_STACK    (192U)
//******************************** Defines **********************************//

//****************************** Local Variables ****************************//
static const char *s_case_name[SELFTEST_CASE_MAX];
static BOOL_T      s_case_pass[SELFTEST_CASE_MAX];
static UINT32_T    s_case_count = 0U;

static osal_sema_handle_t        s_sema        = NULL;
static osal_mutex_handle_t       s_mutex       = NULL;
static osal_queue_handle_t       s_queue       = NULL;
static osal_queue_handle_t       s_mailbox     = NULL;
static osal_event_group_handle_t s_event_group = NULL;
static osal_timer_handle_t       s_timer       = NULL;

static volatile UINT32_T s_timer_fired   = 0U;
static volatile BOOL_T   s_producer_done = false;

/**
 * @brief Order in which the priority-probe tasks were scheduled.
 *
 * Each probe appends its own priority level as it starts. With a correct
 * mapping the higher OSAL priority runs first.
 */
static volatile UINT32_T s_prio_order[2];
static volatile UINT32_T s_prio_index = 0U;


//****************************** Local Variables ****************************//

//***************************** Local Functions *****************************//
/**
 * @brief Record one case result and log it.
 *
 * @param[in] p_name Case name.
 * @param[in] pass   true when the case succeeded.
 */
static void selftest_record(const char *p_name, BOOL_T pass)
{
    if (s_case_count < SELFTEST_CASE_MAX)
    {
        s_case_name[s_case_count] = p_name;
        s_case_pass[s_case_count] = pass;
        s_case_count++;
    }

    DEBUG_OUT(i, OSAL_SELFTEST_LOG_TAG, "  [%s] %s",
              (pass) ? "PASS" : "FAIL", p_name);
}

/**
 * @brief Timer callback for the software timer case.
 *
 * @param[in] p_arg Unused.
 */
static void selftest_timer_cb(void *p_arg)
{
    PLATFORM_UNUSED(p_arg);
    s_timer_fired++;
}

/**
 * @brief Producer task feeding the semaphore and queue cases.
 *
 * @param[in] p_arg Unused.
 */
static void selftest_producer(void *p_arg)
{
    UINT32_T i;

    PLATFORM_UNUSED(p_arg);

    for (i = 0U; i < SELFTEST_QUEUE_ROUNDS; i++)
    {
        UINT32_T payload = i + 1U;

        osal_task_delay(osal_ms_to_ticks(20U));
        (void)osal_sema_give(s_sema);
        (void)osal_queue_send(s_queue, &payload, SELFTEST_WAIT_TICKS);
    }

    (void)osal_event_group_set_bits(s_event_group, SELFTEST_EVENT_BIT_A);

    s_producer_done = true;

    /* osal_task_delete() rejects NULL rather than reading it as "self"
       the way vTaskDelete does, so the handle has to be fetched.
       Returning from a task entry instead is fatal on both backends. */
    (void)osal_task_delete(osal_task_get_current_handle());
}

/**
 * @brief Priority probe: records that it ran, then exits.
 *
 * @param[in] p_arg Priority level this probe was created at, as an integer.
 */
static void selftest_prio_probe(void *p_arg)
{
    if (s_prio_index < 2U)
    {
        s_prio_order[s_prio_index] = (UINT32_T)(UINTPTR_T)p_arg;
        s_prio_index++;
    }

    (void)osal_task_delete(osal_task_get_current_handle());
}

/**
 * @brief Create every OSAL object the cases need.
 *
 * @return true when all objects were created.
 */
static BOOL_T selftest_objects_create(void)
{
    BOOL_T ok = true;

    ok = ok && (OSAL_SUCCESS == osal_sema_init(&s_sema, 0U));
    ok = ok && (OSAL_SUCCESS == osal_mutex_init(&s_mutex));
    ok = ok && (OSAL_SUCCESS == osal_queue_create(&s_queue,
                                                  SELFTEST_QUEUE_DEPTH,
                                                  sizeof(UINT32_T)));
    ok = ok && (OSAL_SUCCESS == osal_queue_create(&s_mailbox, 1U,
                                                  sizeof(UINT32_T)));
    ok = ok && (OSAL_SUCCESS == osal_event_group_create(&s_event_group));
    ok = ok && (OSAL_SUCCESS == osal_timer_create(&s_timer, "selftest_tmr",
                                                  SELFTEST_TIMER_PERIOD,
                                                  OSAL_TRUE, NULL,
                                                  selftest_timer_cb));
    return ok;
}

/**
 * @brief Semaphore and queue: five rounds against the producer task.
 */
static void selftest_case_sema_queue(void)
{
    BOOL_T   sema_ok  = true;
    BOOL_T   queue_ok = true;
    UINT32_T i;

    for (i = 0U; i < SELFTEST_QUEUE_ROUNDS; i++)
    {
        UINT32_T got = 0U;

        if (OSAL_SUCCESS != osal_sema_take(s_sema, SELFTEST_WAIT_TICKS))
        {
            sema_ok = false;
            break;
        }

        if ((OSAL_SUCCESS != osal_queue_receive(s_queue, &got,
                                                SELFTEST_WAIT_TICKS)) ||
            (got != (i + 1U)))
        {
            queue_ok = false;
            break;
        }
    }

    selftest_record("sema      take/give x5", sema_ok);
    selftest_record("queue     send/receive x5, FIFO order", queue_ok);
}

/**
 * @brief Mutex: plain lock and unlock from task context.
 */
static void selftest_case_mutex(void)
{
    BOOL_T ok;

    ok = (OSAL_SUCCESS == osal_mutex_take(s_mutex, SELFTEST_WAIT_TICKS));
    ok = ok && (OSAL_SUCCESS == osal_mutex_give(s_mutex));

    selftest_record("mutex     take/give", ok);
}

/**
 * @brief Event group: wait for a bit the producer sets, clearing on exit.
 */
static void selftest_case_event_group(void)
{
    UINT32_T bits = 0U;
    BOOL_T   ok;

    ok = (OSAL_SUCCESS == osal_event_group_wait_bits(s_event_group,
                                                     SELFTEST_EVENT_BIT_A,
                                                     true,  /* clear on exit */
                                                     false, /* any bit       */
                                                     SELFTEST_WAIT_TICKS,
                                                     &bits));
    ok = ok && (0U != (bits & SELFTEST_EVENT_BIT_A));

    /* Cleared on exit, so a second wait must time out rather than pass. */
    ok = ok && (OSAL_SUCCESS != osal_event_group_wait_bits(s_event_group,
                                                           SELFTEST_EVENT_BIT_A,
                                                           true, false,
                                                           osal_ms_to_ticks(20U),
                                                           &bits));

    selftest_record("event     wait OR + clear-on-exit", ok);
}

/**
 * @brief Task notification: the wait side, with nothing pending.
 *
 * @note OSAL only exposes osal_notify_send_from_isr(), so a task cannot
 *       notify another task and the send side cannot be driven from here.
 *       The real send path is exercised on hardware by the CST816T touch and
 *       MPU6050 motion ISRs, which are the only notify users in the project.
 *
 *       What this case does cover is that the wait side is wired up at all:
 *       under RT-Thread the notification slot is an rt_event embedded in the
 *       task block, resolved through the thread's user_data, and a task
 *       created from static storage must find its slot. A missing slot
 *       returns OSAL_ERROR immediately; a working one blocks and then times
 *       out, which is what is asserted.
 */
static void selftest_case_notify(void)
{
    UINT32_T value = 0U;
    BOOL_T   ok;

    ok = (OSAL_NOTIFY_TIMEOUT == osal_notify_wait(0U, SELFTEST_NOTIFY_BIT,
                                                  &value,
                                                  osal_ms_to_ticks(50U)));

    selftest_record("notify    wait slot resolves, times out empty", ok);
}

/**
 * @brief Mailbox overwrite: a full single-slot mailbox must be replaced.
 *
 * @note The interesting half is the second send. A plain queue send would
 *       block or fail on a full queue; overwrite has to replace the pending
 *       item, and the reader must then see only the newer value.
 */
static void selftest_case_mailbox(void)
{
    UINT32_T first  = 0xAAAAAAAAUL;
    UINT32_T second = 0x55555555UL;
    UINT32_T got    = 0U;
    BOOL_T   ok;

    ok = (OSAL_SUCCESS == osal_mailbox_overwrite(s_mailbox, &first));
    ok = ok && (OSAL_SUCCESS == osal_mailbox_peek(&s_mailbox));
    ok = ok && (OSAL_SUCCESS == osal_mailbox_overwrite(s_mailbox, &second));
    ok = ok && (OSAL_SUCCESS == osal_queue_receive(s_mailbox, &got,
                                                   SELFTEST_WAIT_TICKS));
    ok = ok && (second == got);
    /* Drained by the receive, so peek must now report empty. */
    ok = ok && (OSAL_SUCCESS != osal_mailbox_peek(&s_mailbox));

    selftest_record("mailbox   overwrite replaces pending item", ok);
}

/**
 * @brief Software timer: a periodic timer must fire repeatedly, then stop.
 */
static void selftest_case_timer(void)
{
    UINT32_T fired_after_stop;
    BOOL_T   ok;

    s_timer_fired = 0U;

    ok = (OSAL_SUCCESS == osal_timer_start(s_timer, SELFTEST_WAIT_TICKS));
    osal_task_delay(osal_ms_to_ticks(SELFTEST_TIMER_SETTLE));

    /* SELFTEST_TIMER_SETTLE / SELFTEST_TIMER_PERIOD is 5; require most of
       them so a slow tick does not read as a failure. */
    ok = ok && (s_timer_fired >= 3U);

    ok = ok && (OSAL_SUCCESS == osal_timer_stop(s_timer, SELFTEST_WAIT_TICKS));
    fired_after_stop = s_timer_fired;
    osal_task_delay(osal_ms_to_ticks(SELFTEST_TIMER_SETTLE));
    ok = ok && (fired_after_stop == s_timer_fired);

    selftest_record("timer     periodic fires, stop halts it", ok);
}

/**
 * @brief Tick: the OSAL tick must advance at roughly the configured rate.
 */
static void selftest_case_tick(void)
{
    osal_tick_type_t start;
    osal_tick_type_t delta;
    BOOL_T           ok;

    start = osal_task_get_tick_count();
    osal_task_delay(osal_ms_to_ticks(200U));
    delta = osal_task_get_tick_count() - start;

    ok = ((delta >= 195U) && (delta <= 215U));

    selftest_record("tick      ~200 ticks per 200 ms", ok);
}

/**
 * @brief Heap: allocate, write, free.
 */
static void selftest_case_heap(void)
{
    void  *p_block;
    BOOL_T ok;

    p_block = osal_heap_malloc(512U);
    ok      = (NULL != p_block);
    if (ok)
    {
        (void)memset(p_block, 0xA5, 512U);
        osal_heap_free(p_block);
    }

    selftest_record("heap      malloc/free 512 B", ok);
}

/**
 * @brief Priority ordering: the higher OSAL priority must run first.
 *
 * @note This is the case that catches an inverted priority mapping. Two
 *       probes are created lowest-first while this task holds a priority
 *       above both, so neither can run until this task blocks; when it does,
 *       the scheduler picks the higher-priority probe. If the mapping were
 *       reflected the wrong way the order would come out reversed here
 *       instead of quietly misbehaving at runtime.
 */
static void selftest_case_priority(void)
{
    osal_task_handle_t low_handle  = NULL;
    osal_task_handle_t high_handle = NULL;
    UINT32_T           low_prio    = (UINT32_T)PRI_NORMAL;
    UINT32_T           high_prio   = (UINT32_T)PRI_NORMAL + 2U;
    BOOL_T             ok;

    s_prio_index = 0U;

    ok = (OSAL_SUCCESS == osal_task_create(&low_handle, "sel_prio_lo",
                                           (void *)(UINTPTR_T)low_prio,
                                           selftest_prio_probe,
                                           SELFTEST_HELPER_STACK, low_prio));
    ok = ok && (OSAL_SUCCESS == osal_task_create(&high_handle, "sel_prio_hi",
                                                 (void *)(UINTPTR_T)high_prio,
                                                 selftest_prio_probe,
                                                 SELFTEST_HELPER_STACK,
                                                 high_prio));

    /* Blocks this task so both probes get to run. */
    osal_task_delay(osal_ms_to_ticks(100U));

    ok = ok && (2U == s_prio_index);
    ok = ok && (high_prio == s_prio_order[0]);
    ok = ok && (low_prio == s_prio_order[1]);

    selftest_record("priority  higher OSAL priority runs first", ok);
}

/**
 * @brief Callee-saved registers must survive a context switch.
 *
 * @note The most basic contract an RTOS has with compiled code: r4-r11 belong
 *       to the caller, so a thread that blocks and resumes must find them
 *       exactly as it left them. Break it and nothing faults -- the compiler
 *       simply keeps long-lived locals in those registers and they silently
 *       read back as whatever the switch restored, which on RT-Thread is the
 *       0xdeadbeef that rt_hw_stack_init() paints into a fresh stack frame.
 *
 *       Eight longer-lived locals plus a blocking delay is the shape that
 *       pushes the compiler into using r4-r11 and then forces a switch across
 *       them. The values are distinct so a report can name which one moved.
 */
static void selftest_case_ctx_regs(void)
{
    volatile UINT32_T seed = 0xA0000000UL;
    UINT32_T v1 = seed + 1UL;
    UINT32_T v2 = seed + 2UL;
    UINT32_T v3 = seed + 3UL;
    UINT32_T v4 = seed + 4UL;
    UINT32_T v5 = seed + 5UL;
    UINT32_T v6 = seed + 6UL;
    UINT32_T v7 = seed + 7UL;
    UINT32_T v8 = seed + 8UL;
    BOOL_T   ok;

    /* Block long enough that the idle and timer threads certainly run. */
    osal_task_delay(osal_ms_to_ticks(60U));

    ok = (v1 == 0xA0000001UL) && (v2 == 0xA0000002UL) &&
         (v3 == 0xA0000003UL) && (v4 == 0xA0000004UL) &&
         (v5 == 0xA0000005UL) && (v6 == 0xA0000006UL) &&
         (v7 == 0xA0000007UL) && (v8 == 0xA0000008UL);

    if (!ok)
    {
        DEBUG_OUT(e, OSAL_SELFTEST_LOG_TAG,
                  "    ctx regs: %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX",
                  (unsigned long)v1, (unsigned long)v2, (unsigned long)v3,
                  (unsigned long)v4, (unsigned long)v5, (unsigned long)v6,
                  (unsigned long)v7, (unsigned long)v8);
    }

    selftest_record("ctxregs   r4-r11 survive a context switch", ok);
}

/**
 * @brief Body for the thread-exit register case: just deletes itself.
 *
 * @param[in] p_arg Unused.
 */
static void selftest_exit_probe(void *p_arg)
{
    PLATFORM_UNUSED(p_arg);
    (void)osal_task_delete(osal_task_get_current_handle());
}

/**
 * @brief Callee-saved registers must survive another thread's self-deletion.
 *
 * @note Narrower than the plain context-switch case: it spans a thread being
 *       created, running, and removing itself through osal_task_delete() on
 *       its own handle. That path ends in rt_thread_exit(), which reschedules
 *       from inside a thread the kernel has already closed -- a different
 *       shape of switch from an ordinary block-and-resume, and the one this
 *       task's own long-lived registers appear not to survive.
 */
static void selftest_case_ctx_thread_exit(void)
{
    volatile UINT32_T  seed   = 0xB0000000UL;
    UINT32_T           v1     = seed + 1UL;
    UINT32_T           v2     = seed + 2UL;
    UINT32_T           v3     = seed + 3UL;
    UINT32_T           v4     = seed + 4UL;
    UINT32_T           v5     = seed + 5UL;
    UINT32_T           v6     = seed + 6UL;
    UINT32_T           v7     = seed + 7UL;
    UINT32_T           v8     = seed + 8UL;
    osal_task_handle_t handle = NULL;
    BOOL_T             ok;

    (void)osal_task_create(&handle, "sel_exit", NULL, selftest_exit_probe,
                           SELFTEST_HELPER_STACK, (UINT32_T)PRI_NORMAL);

    /* Long enough for the probe to run and delete itself. */
    osal_task_delay(osal_ms_to_ticks(80U));

    ok = (v1 == 0xB0000001UL) && (v2 == 0xB0000002UL) &&
         (v3 == 0xB0000003UL) && (v4 == 0xB0000004UL) &&
         (v5 == 0xB0000005UL) && (v6 == 0xB0000006UL) &&
         (v7 == 0xB0000007UL) && (v8 == 0xB0000008UL);

    if (!ok)
    {
        DEBUG_OUT(e, OSAL_SELFTEST_LOG_TAG,
                  "    exit regs: %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX",
                  (unsigned long)v1, (unsigned long)v2, (unsigned long)v3,
                  (unsigned long)v4, (unsigned long)v5, (unsigned long)v6,
                  (unsigned long)v7, (unsigned long)v8);
    }

    selftest_record("ctxexit   r4-r11 survive a thread self-delete", ok);
}

/**
 * @brief Static task allocation: the production task table uses it for all
 *        14 tasks, so it has to work before anything else is believable.
 *
 * @note Implicitly already proven -- this very task was created that way --
 *       so the case just confirms the handle is live.
 */
static void selftest_case_static_task(void)
{
    selftest_record("task      static allocation scheduled",
                    (NULL != osal_task_get_current_handle()));
}
//***************************** Local Functions *****************************//

//******************************* Functions *********************************//
/**
 * @brief OSAL conformance test task entry.
 *
 * @param[in] p_arg Unused.
 */
void osal_selftest_thread(void *p_arg)
{
    UINT32_T passed = 0U;
    UINT32_T i;

    PLATFORM_UNUSED(p_arg);

    DEBUG_OUT(i, OSAL_SELFTEST_LOG_TAG, "=== OSAL self-test ===");
    DEBUG_OUT(i, OSAL_SELFTEST_LOG_TAG, "backend   : %s",
              (RTTHREAD_SUPPORT == OSAL_RTOS_SUPPORT) ? "RT-Thread"
                                                      : "FreeRTOS");
    DEBUG_OUT(i, OSAL_SELFTEST_LOG_TAG, "prio scale: 0..%lu (larger = higher)",
              (unsigned long)OSAL_PRIORITY_MAX - 1UL);

    if (!selftest_objects_create())
    {
        DEBUG_OUT(e, OSAL_SELFTEST_LOG_TAG,
                  "object creation failed - aborting");
        for (;;)
        {
            osal_task_delay(osal_ms_to_ticks(1000U));
        }
    }

    /* The producer drives the sema, queue, event and notify cases. */
    {
        osal_task_handle_t producer = NULL;

        (void)osal_task_create(&producer, "sel_producer", NULL,
                               selftest_producer, SELFTEST_HELPER_STACK,
                               (UINT32_T)PRI_NORMAL);
    }

    selftest_case_sema_queue();
    selftest_case_mutex();
    selftest_case_event_group();
    selftest_case_notify();
    selftest_case_mailbox();
    selftest_case_timer();
    selftest_case_tick();
    selftest_case_heap();
    selftest_case_priority();
    selftest_case_ctx_regs();
    selftest_case_ctx_thread_exit();
    selftest_case_static_task();

    for (i = 0U; i < s_case_count; i++)
    {
        if (s_case_pass[i])
        {
            passed++;
        }
    }

    DEBUG_OUT(i, OSAL_SELFTEST_LOG_TAG, "=== %lu/%lu passed ===",
              (unsigned long)passed, (unsigned long)s_case_count);

    if (passed == s_case_count)
    {
        DEBUG_OUT(i, OSAL_SELFTEST_LOG_TAG, "RESULT: ALL PASS");
    }
    else
    {
        DEBUG_OUT(e, OSAL_SELFTEST_LOG_TAG, "RESULT: FAILURES PRESENT");
        for (i = 0U; i < s_case_count; i++)
        {
            if (!s_case_pass[i])
            {
                DEBUG_OUT(e, OSAL_SELFTEST_LOG_TAG, "  failed: %s",
                          s_case_name[i]);
            }
        }
    }

    /* Idle from here; the summary above is the deliverable. */
    for (;;)
    {
        osal_task_delay(osal_ms_to_ticks(1000U));
    }
}

//******************************* Functions *********************************//

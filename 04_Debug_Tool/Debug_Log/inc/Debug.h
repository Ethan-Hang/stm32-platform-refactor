/******************************************************************************
 * @file Debug.h
 *
 * @par dependencies
 * - "elog.h"
 * - "itm_trace.h"
 *
 * @author Ethan-Hang
 *
 * @brief Provide centralized debug tag filtering, RTT channel routing,
 *        ITM/SWO tag routing, and logging output macros.
 *
 * Processing flow:
 * Define log tags, filter policy, RTT terminal routing, and DEBUG_OUT
 * wrapper for EasyLogger.  Tags listed in debug_is_itm_tag() are routed
 * to printf() → ITM/SWO instead of the RTT path.
 *
 * @version V1.0 2025-11-20
 * @version V2.0 2026-04-01
 * @version V3.0 2026-04-13
 * @version V4.0 2026-04-23
 * @upgrade 2.0:
 * Per-module dual tags (MODULE/MODULE_ERR) for centralized log output
 * @upgrade 3.0:
 * RTT multi-terminal routing: debug_tag_to_rtt_channel() maps each tag
 * to a dedicated RTT up-channel so J-Link RTT Viewer can display module
 * groups on separate terminals without changing any call sites.
 * @upgrade 4.0:
 * ITM/SWO output path: tags listed in debug_is_itm_tag() bypass EasyLogger
 * and are printed via printf() → __io_putchar() → ITM stimulus port 0.
 * Both paths (RTT and ITM) are active simultaneously; select per tag.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/
#ifndef __DEBUG_H__
#define __DEBUG_H__

//******************************** Includes ********************************//
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "elog.h"
#include "itm_trace.h"
//******************************** Includes ********************************//

//******************************** Defines *********************************//

#define DEBUG                 (1) /* Enable centralized debug output       */

/* Keep existing feature flags to avoid behavior changes in other modules. */
#define USED_PWM_CONTROL      (1) /* Enable PWM control for LED operations */

/*
 * Project-level tags: one normal/error pair per module.
 */
#define WT588_HANDLER_LOG_TAG               "WT588_HANDLER"
#define MPUXXXX_ERR_LOG_TAG                   "MPUXXXX_ERR"
#define WT588_LOG_TAG                               "WT588"
#define AHT21_LOG_TAG                               "AHT21"
#define TEMP_HUMI_LOG_TAG                       "TEMP_HUMI"
#define MPUXXXX_LOG_TAG                           "MPUXXXX"
#define UNPACK_LOG_TAG                             "UNPACK"
#define WT588_ERR_LOG_TAG                       "WT588_ERR"
#define TEMP_HUMI_ERR_LOG_TAG               "TEMP_HUMI_ERR"
#define AHT21_ERR_LOG_TAG                       "AHT21_ERR"
#define WT588_HANDLER_ERR_LOG_TAG       "WT588_HANDLER_ERR"
#define CORE_LOG_TAG                                 "CORE"
#define USER_INIT_LOG_TAG                       "USER_INIT"
#define HAL_IIC_ERR_LOG_TAG                   "HAL_IIC_ERR"
#define UNPACK_ERR_LOG_TAG                     "UNPACK_ERR"
#define USER_INIT_ERR_LOG_TAG               "USER_INIT_ERR"
#define CORE_ERR_LOG_TAG                         "CORE_ERR"
#define TEMP_HUMI_TEST_LOG_TAG             "TEMP_HUMI_TEST"
#define TEMP_HUMI_TEST_ERR_LOG_TAG     "TEMP_HUMI_TEST_ERR"
#define STACK_MONITOR_LOG_TAG               "STACK_MONITOR"
#define STACK_MONITOR_ERR_LOG_TAG       "STACK_MONITOR_ERR"
#define WT588_HAL_PORT_LOG_TAG             "WT588_HAL_PORT"
#define WT588_HAL_PORT_ERR_LOG_TAG     "WT588_HAL_PORT_ERR"
#define LIST_LOG_TAG                                 "LIST"
#define LIST_ERR_LOG_TAG                         "LIST_ERR"
#define WT588_TEST_LOG_TAG                     "WT588_TEST"
#define WT588_TEST_ERR_LOG_TAG             "WT588_TEST_ERR"
#define RTOS_TRACE_TASK_OUT_TAG           "RTOS_TRACE_TASK"

/*
 * ──────────────────────── ITM/SWO Tag Assignments ───────────────────────── *
 *                                                                            *
 *  Tags defined here are routed through printf() → __io_putchar() → ITM     *
 *  stimulus port 0 instead of through EasyLogger/RTT.  The output appears   *
 *  in JLink SWO Viewer or Ozone's SWO Terminal at the configured baud rate. *
 *                                                                            *
 *  To add a new ITM-routed tag:                                              *
 *    1. Define a new *_ITM_LOG_TAG constant below.                           *
 *    2. Add it to debug_is_itm_tag().                                        *
 *    No changes to elog_port.c or RTT configuration required.                *
 */
#define CORE_ITM_LOG_TAG                         "CORE_ITM"
#define USER_INIT_ITM_LOG_TAG                    "INIT_ITM"

static inline int debug_is_itm_tag(const char *tag)
{
    if ((tag == NULL) || (tag[0] == '\0'))
    {
        return 0;
    }

    return (strcmp(    CORE_ITM_LOG_TAG, tag) == 0) ||
           (strcmp(USER_INIT_ITM_LOG_TAG, tag) == 0);
}

/*
 * ──────────────────── RTT Virtual Terminal Assignments ──────────────────── *
 *                                                                            *
 *  All log data travels through physical RTT channel 0.                      *
 *  elog_port_output() calls SEGGER_RTT_SetTerminal() to prefix each          *
 *  message with a 2-byte escape sequence, which J-Link RTT Viewer uses to    *
 *  route the message to the corresponding Terminal tab (0-9).                *
 *                                                                            *
 *  Terminal 0 : default — all tags not explicitly routed below               *
 *  Terminal 2+: custom groups visible as tabs in J-Link RTT Viewer           *
 *                                                                            *
 *  To add a new terminal group:                                              *
 *    1. Define a new DEBUG_RTT_CH_* constant below (value 0-9).              *
 *    2. Add the relevant tags to debug_tag_to_rtt_channel().                 *
 *    No buffer registration or SEGGER_RTT_Conf.h change required.            *
 */
#define DEBUG_RTT_CH_DEFAULT        (0u)    /* catch-all terminal            */
#define DEBUG_RTT_CH_SENSOR0        (1u)    /* AHT21 / temperature-humidity  */
#define DEBUG_RTT_CH_SENSOR1        (2u)    /* WT588 handler / test          */
#define DEBUG_RTT_CH_SENSOR2        (3u)    /* reserved for future use       */
#define DEBUG_RTT_CH_STACK          (4u)    /* stack high-water monitor      */
/*
 * g_debug_rtt_channel is written by DEBUG_OUT() immediately before the
 * elog_* call and read by elog_port_output() to select the RTT channel.
 * EasyLogger's output lock (portENTER_CRITICAL) serialises the
 * format+write phase so log messages are never interleaved.
 */
extern volatile uint8_t g_debug_rtt_channel;

static inline int debug_is_tag_allowed(const char *tag)
{
    if ((tag == NULL) || (tag[0] == '\0'))
    {
        return 0;
    }

    return\
            // (strcmp(   RTOS_TRACE_TASK_OUT_TAG, tag) == 0)                   ||
            (strcmp(            UNPACK_LOG_TAG, tag) == 0)                   ||
            (strcmp(     WT588_HANDLER_LOG_TAG, tag) == 0)                   ||
            (strcmp(       MPUXXXX_ERR_LOG_TAG, tag) == 0)                   ||
            (strcmp(             WT588_LOG_TAG, tag) == 0)                   ||
            // (strcmp(             AHT21_LOG_TAG, tag) == 0)                   ||
            // (strcmp(         TEMP_HUMI_LOG_TAG, tag) == 0)                   ||
            (strcmp(           MPUXXXX_LOG_TAG, tag) == 0)                   ||
            (strcmp(         WT588_ERR_LOG_TAG, tag) == 0)                   ||
            (strcmp(     TEMP_HUMI_ERR_LOG_TAG, tag) == 0)                   ||
            (strcmp(         AHT21_ERR_LOG_TAG, tag) == 0)                   ||
            (strcmp( WT588_HANDLER_ERR_LOG_TAG, tag) == 0)                   ||
            (strcmp(         USER_INIT_LOG_TAG, tag) == 0)                   ||
            (strcmp(       HAL_IIC_ERR_LOG_TAG, tag) == 0)                   ||
            (strcmp(        UNPACK_ERR_LOG_TAG, tag) == 0)                   ||
            (strcmp(     USER_INIT_ERR_LOG_TAG, tag) == 0)                   ||
            (strcmp(          CORE_ERR_LOG_TAG, tag) == 0)                   ||
            (strcmp(    TEMP_HUMI_TEST_LOG_TAG, tag) == 0)                   ||
            (strcmp(TEMP_HUMI_TEST_ERR_LOG_TAG, tag) == 0)                   ||
            (strcmp(     STACK_MONITOR_LOG_TAG, tag) == 0)                   ||
            (strcmp( STACK_MONITOR_ERR_LOG_TAG, tag) == 0)                   ||
            (strcmp(    WT588_HAL_PORT_LOG_TAG, tag) == 0)                   ||
            (strcmp(WT588_HAL_PORT_ERR_LOG_TAG, tag) == 0)                   ||
            (strcmp(             LIST_LOG_TAG,  tag) == 0)                   ||
            (strcmp(         LIST_ERR_LOG_TAG,  tag) == 0)                   ||
            (strcmp(       WT588_TEST_LOG_TAG,  tag) == 0)                   ||
            (strcmp(   WT588_TEST_ERR_LOG_TAG,  tag) == 0);
}

/**
 * @brief Map a log tag string to its designated RTT up-channel index.
 *
 * @param[in] tag : log tag string (one of the *_LOG_TAG macros above)
 *
 * @return RTT channel index (0 = default, 2+ = custom terminals)
 *
 * */
static inline uint8_t debug_tag_to_rtt_channel(const char *tag)
{
    /* === Terminal 1 : stack high-water monitor === */
    if (
        (strcmp(    STACK_MONITOR_LOG_TAG, tag) == 0)                        ||
        (strcmp(STACK_MONITOR_ERR_LOG_TAG, tag) == 0)                        ||
        (strcmp(  RTOS_TRACE_TASK_OUT_TAG, tag) == 0)
       )
    {
        return DEBUG_RTT_CH_STACK;
    }

    /* === Terminal 2 : AHT21 and temperature/humidity modules === */
    if (
        (strcmp(              AHT21_LOG_TAG, tag) == 0)                      ||
        (strcmp(          AHT21_ERR_LOG_TAG, tag) == 0)                      ||
        (strcmp(          TEMP_HUMI_LOG_TAG, tag) == 0)                      ||
        (strcmp(      TEMP_HUMI_ERR_LOG_TAG, tag) == 0)                      ||
        (strcmp(     TEMP_HUMI_TEST_LOG_TAG, tag) == 0)                      ||
        (strcmp( TEMP_HUMI_TEST_ERR_LOG_TAG, tag) == 0)                      ||
        (strcmp(               CORE_LOG_TAG, tag) == 0)
        )
    {
        return DEBUG_RTT_CH_SENSOR0;
    }

    if (
        (strcmp(     WT588_HANDLER_LOG_TAG, tag) == 0)                      ||
        (strcmp( WT588_HANDLER_ERR_LOG_TAG, tag) == 0)                      ||
        (strcmp(             WT588_LOG_TAG, tag) == 0)                      ||
        (strcmp(         WT588_ERR_LOG_TAG, tag) == 0)                      ||
        (strcmp(        WT588_TEST_LOG_TAG, tag) == 0)                      ||
        (strcmp(    WT588_TEST_ERR_LOG_TAG, tag) == 0)
        )
    {
        return DEBUG_RTT_CH_SENSOR1;
    }

    if (
        (strcmp(       MPUXXXX_ERR_LOG_TAG, tag) == 0)                      ||
        (strcmp(           MPUXXXX_LOG_TAG, tag) == 0)                      ||
        (strcmp(            UNPACK_LOG_TAG, tag) == 0)                      ||
        (strcmp(        UNPACK_ERR_LOG_TAG, tag) == 0)                      ||
        (strcmp(              LIST_LOG_TAG, tag) == 0)                      ||
        (strcmp(          LIST_ERR_LOG_TAG, tag) == 0)
        )
    {
        return DEBUG_RTT_CH_SENSOR2;
    }

    return DEBUG_RTT_CH_DEFAULT;
}

/*
 * Usage:
 * DEBUG_OUT(i, AHT21_LOG_TAG,    "AHT21 init ok");
 * DEBUG_OUT(e, AHT21_ERR_LOG_TAG,"AHT21 read timeout %d", err);
 * DEBUG_OUT(i, CORE_ITM_LOG_TAG, "boot complete");   <- routed to ITM/SWO
 *
 * Tags listed in debug_is_itm_tag() → printf() → ITM stimulus port 0.
 * All other allowed tags           → EasyLogger → RTT (unchanged).
 * If ITM is not connected the itm_putchar() call is a silent no-op.
 */
#define DEBUG_OUT(LEVEL, TAG, fmt, ...)                                       \
    do                                                                        \
    {                                                                         \
        if (DEBUG)                                                            \
        {                                                                     \
            const char *debug_tag__ = (TAG);                                  \
            if (debug_is_tag_allowed(debug_tag__))                            \
            {                                                                 \
                g_debug_rtt_channel = debug_tag_to_rtt_channel(debug_tag__);  \
                elog_##LEVEL(debug_tag__, fmt, ##__VA_ARGS__);                \
            }                                                                 \
            else if (debug_is_itm_tag(debug_tag__))                           \
            {                                                                 \
                printf("[" #LEVEL "][%s] " fmt "\r\n",                        \
                       debug_tag__, ##__VA_ARGS__);                           \
            }                                                                 \
        }                                                                     \
    } while (0)

void debug_init(void);

//********************************* Defines *********************************//

//******************************** Declaring ********************************//

//******************************** Declaring ********************************//


#endif // End of __DEBUG_H__

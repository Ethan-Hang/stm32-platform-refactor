/******************************************************************************
 * @file Debug.c
 *
 * @par dependencies
 * - "Debug.h"
 *
 * @author Ethan-Hang
 *
 * @brief Debug subsystem initialisation: EasyLogger setup and RTT
 *        virtual-terminal routing initialisation.
 *
 * @version V1.0 2025-11-20
 * @version V2.0 2026-04-13
 * @upgrade 2.0:
 * Virtual-terminal routing via SEGGER_RTT_SetTerminal(): all log data
 * travels through physical RTT channel 0, and elog_port_output() inserts
 * SetTerminal escape sequences so J-Link RTT Viewer sorts messages into
 * the correct Terminal tab.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "Debug.h"
//******************************** Includes *********************************//

//******************************* Variables *********************************//

/*
 * Virtual terminal selector: written by DEBUG_OUT() before every elog
 * call, read by elog_port_output() to call SEGGER_RTT_SetTerminal().
 * All log data is sent through physical RTT channel 0; this index (0-9)
 * determines which Terminal tab in J-Link RTT Viewer the message appears
 * under.
 */
volatile uint8_t g_debug_rtt_channel = 0u;

//******************************* Variables *********************************//

//******************************* Functions *********************************//

/**
 * @brief Initialise the debug subsystem.
 *
 * Configures EasyLogger output format and starts the logger.
 * Must be called once from the main initialisation sequence before any
 * DEBUG_OUT() call.
 *
 * @param[in] : none
 *
 * @param[out] : none
 *
 * @return void
 *
 * */
void debug_init(void)
{
#if DEBUG
    /** Configure EasyLogger output format for every severity level. */
    elog_init();
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_ERROR,  ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_WARN,   ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_INFO,   ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_DEBUG,  ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_start();
#endif /* DEBUG */
}

//******************************* Functions *********************************//

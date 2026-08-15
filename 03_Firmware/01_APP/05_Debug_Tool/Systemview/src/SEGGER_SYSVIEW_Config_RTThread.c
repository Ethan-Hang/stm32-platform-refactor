/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2015 - 2021  SEGGER Microcontroller GmbH                  *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------
File    : SEGGER_SYSVIEW_Config_RTThread.c
Purpose : SystemView configuration for the RT-Thread backend.

Notes   : Counterpart of SEGGER_SYSVIEW_Config_FreeRTOS.c, deliberately kept
          in the same shape so the two backends can be compared line by line.

          Written for this project rather than taken from the upstream
          RT-Thread SystemView package, whose configuration file is mostly
          RT-Thread package-manager (PKG_*) Kconfig branches this build has no
          use for, plus DWT cycle-counter enabling that core_dwt_init() in
          Core/Src/main.c already performs before SEGGER_SYSVIEW_Conf() runs.

          The timestamp source itself is not configured here: this project's
          SEGGER_SYSVIEW_Conf.h overrides only SEGGER_SYSVIEW_SECTION, so
          SEGGER_SYSVIEW_GET_TIMESTAMP falls through to the Cortex-M default
          in SEGGER_SYSVIEW_ConfDefaults.h, the DWT cycle counter.
*/

#include "SEGGER_SYSVIEW.h"
#include "stm32f4xx.h"   /* SystemCoreClock */

extern const SEGGER_SYSVIEW_OS_API SYSVIEW_X_OS_TraceAPI;

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
// The application name to be displayed in SystemViewer
#define SYSVIEW_APP_NAME        "RT-Thread Application"

// The target device name
#define SYSVIEW_DEVICE_NAME     "Cortex-M4"

// Frequency of the timestamp. Must match SEGGER_SYSVIEW_GET_TIMESTAMP in SEGGER_SYSVIEW_Conf.h
#define SYSVIEW_TIMESTAMP_FREQ  (SystemCoreClock)

// System Frequency. SystemcoreClock is used in most CMSIS compatible projects.
#define SYSVIEW_CPU_FREQ        (SystemCoreClock)

// The lowest RAM address used for IDs (pointers)
#define SYSVIEW_RAM_BASE        (0x20000000)

/*********************************************************************
*
*       _cbSendSystemDesc()
*
*  Function description
*    Sends SystemView description strings.
*
*    O= must read RT-Thread for SystemViewer to load the matching description
*    file (05_Debug_Tool/Systemview/description/SYSVIEW_RT-Thread.txt, which
*    has to be copied into the SystemView installation's Description folder).
*/
static void _cbSendSystemDesc(void) {
  SEGGER_SYSVIEW_SendSysDesc("N="SYSVIEW_APP_NAME",D="SYSVIEW_DEVICE_NAME",O=RT-Thread");
  SEGGER_SYSVIEW_SendSysDesc("I#15=SysTick");
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
void SEGGER_SYSVIEW_Conf(void) {
  SEGGER_SYSVIEW_Init(SYSVIEW_TIMESTAMP_FREQ, SYSVIEW_CPU_FREQ,
                      &SYSVIEW_X_OS_TraceAPI, _cbSendSystemDesc);
  SEGGER_SYSVIEW_SetRAMBase(SYSVIEW_RAM_BASE);
}

/*************************** End of file ****************************/

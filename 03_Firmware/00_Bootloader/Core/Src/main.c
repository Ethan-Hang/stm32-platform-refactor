/******************************************************************************
 * @file main.c
 *
 * @par dependencies
 * - main.h
 * - gpio.h
 * - spi.h
 * - uart.h
 * - tim.h
 * - bootmanager.h
 * - at24cxx_driver.h
 * - w25qxx_Handler.h
 * - elog.h
 * - Debug.h
 *
 * @author Ethan-Hang
 *
 * @brief
 * Bootloader main entry and OTA state polling loop.
 *
 * Processing flow:
 * 1. Initialize clock and basic peripherals.
 * 2. Initialize log system and storage devices.
 * 3. Poll OTA state machine and handle APP jump logic.
 *
 * @version V1.0 2026-4-2
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "uart.h"
#include "tim.h"

#include "bootmanager.h"

#include "at24cxx_driver.h"
#include "w25qxx_Handler.h"

#include "elog.h"

#include "Debug.h"


/* Private typedef -----------------------------------------------------------*/
typedef void (*pFunction)(void);
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t uwTimingDelay;
RCC_ClocksTypeDef    RCC_Clocks;
uint8_t              tab_1024[1024];
volatile bool        elog_init_flag = false;

/* Jump flag retained for OTA path in bootmanager.c (unused on cold boot). */
uint32_t g_jumpinit;

/* Note: g_buf is no longer needed, OTA writes directly to flash. */
/* Private function prototypes -----------------------------------------------*/


uint8_t key_scan(void)
{
    if (Bit_RESET == GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
    {
        delay_ms(10);
        if (Bit_RESET == GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
        {
            return 1;
        }
    }
    return 0;
}

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Main program.
 * @param  None.
 * @retval None.
 */
int main(void)
{
    /* Anchor vector table at Bootloader image base. */
    SCB->VTOR = 0x08000000UL;

    /* Enable Clock Security System(CSS), it generates NMI on HSE failure. */
    RCC_ClockSecuritySystemCmd(ENABLE);

    /* SysTick end of count event each 1ms */
    SystemCoreClockUpdate();
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

    /* Wait 50 ms before peripheral initialization. */
    delay_ms(50);

    GPIO_Config();
    usart1_init();

    elog_init();
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_start();
    elog_init_flag = true;

    DEBUG_OUT(i, MAIN_LOG_TAG,
              "bootloader: jumping to APP @ 0x%08lX",
              (unsigned long)AppAddress);

    /* Pause long enough for J-Link RTT Viewer to poll the up-buffer before
       APP's elog_port_init() re-initialises the RTT control block and clears
       WrOff/RdOff — otherwise this last log line gets overwritten before the
       host has a chance to read it. 200 ms is comfortably above the viewer's
       polling interval. */
    delay_ms(200);

    jump_to_app();

    /* Reach here only when APP vector table is invalid (uninitialised flash).
       Park here so the user can flash the APP. */
    DEBUG_OUT(e, MAIN_LOG_TAG,
              "bootloader: APP slot invalid, waiting for firmware");
    while (1)
    {
        delay_ms(1000);
    }
}


void delay_ms(__IO uint32_t nTime)
{
    uwTimingDelay = nTime;

    while (uwTimingDelay != 0)
        ;
}

void TimingDelay_Decrement(void)
{
    if (uwTimingDelay != 0x00)
    {
        uwTimingDelay--;
    }
}

#ifdef USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to source file name.
 * @param  line: assert_param error source line number.
 * @retval None.
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line
       number, ex: printf("Wrong parameters value: file %s on line %d\r\n",
       file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

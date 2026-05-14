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
 * 1. Initialize clock, GPIO, UART, SPI2, log.
 * 2. Bring up the external W25Q64 flash (SFUD) used as OTA staging area.
 * 3. Poll OTA state from internal-flash Sector 2 flag and dispatch
 *    OTA_StateManager(); state handlers call jump_to_app() themselves.
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

    DEBUG_OUT(d, MAIN_LOG_TAG, "this is bootloader!");

    /**
    * Bring up SPI2 + W25Q64 so the OTA state machine can stage / read
    * encrypted images. SFUD's sfud_init() runs inside W25Q64_Init().
    **/
    SPI2_Init();
    W25Q64_Init();

    DEBUG_OUT(i, MAIN_LOG_TAG,
              "bootloader: OTA loop starting, APP slot @ 0x%08lX",
              (unsigned long)AppAddress);

    /* Pause long enough for J-Link RTT Viewer to poll the up-buffer before
       APP's elog_port_init() re-initialises the RTT control block and clears
       WrOff/RdOff — otherwise this last log line gets overwritten before the
       host has a chance to read it. 200 ms is comfortably above the viewer's
       polling interval. */
    delay_ms(200);

    /**
    * OTA state-driven main loop. Most state handlers call jump_to_app()
    * themselves on the "no upgrade pending" path, so under normal boot
    * we exit this loop almost immediately via the in-APP context. The
    * loop only keeps running when we're actively waiting on a download
    * (EE_INIT_NO_APP path) or in the post-OTA verification states.
    **/
    for (;;)
    {
        OTA_StateManager();
        delay_ms(500);
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

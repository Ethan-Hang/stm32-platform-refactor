/******************************************************************************
 * @file dwt_port.c
 *
 * @par dependencies
 * - dwt_port.h
 * - stm32f4xx.h (DWT / CoreDebug, SystemCoreClock)
 *
 * @author Ethan-Hang
 *
 * @brief Cortex-M4 DWT cycle-counter implementation of the MCU-port us delay.
 *        Keeps Cortex CMSIS register touches contained inside the MCU port
 *        layer — porting to a non-Cortex MCU only requires replacing this
 *        file.
 *
 * @version V1.0 2026-04-26
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "board_types.h"
#include "dwt_port.h"
#include "stm32f4xx.h"
//******************************** Includes *********************************//

//******************************* Functions *********************************//

void core_dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT       = 0U;
    DWT->CTRL        |= DWT_CTRL_CYCCNTENA_Msk;
}

void core_dwt_delay_us(UINT32_t us)
{
    if (0U == us)
    {
        return;
    }

    /* Lazy-init: if the cycle counter is disabled, enable it now. */
    if (0U == (DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk))
    {
        core_dwt_init();
    }

    UINT32_t const start  = DWT->CYCCNT;
    UINT32_t const cycles = us * (SystemCoreClock / 1000000U);

    while ((UINT32_t)(DWT->CYCCNT - start) < cycles)
    {
        /* spin */
    }
}

UINT32_t core_dwt_get_cycles(void)
{
    /* Same lazy-init as the delay path: a profiling call must not silently
     * return a frozen 0 just because nobody ran core_dwt_init() yet. */
    if (0U == (DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk))
    {
        core_dwt_init();
    }

    return DWT->CYCCNT;
}

UINT32_t core_dwt_cycles_per_us(void)
{
    UINT32_t const per_us = SystemCoreClock / 1000000U;

    /* Guard the divide-by-zero that a caller doing cycles/per_us would hit
     * if SystemCoreClock were ever left unset. */
    return (0U == per_us) ? 1U : per_us;
}

//******************************* Functions *********************************//

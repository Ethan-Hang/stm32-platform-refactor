/******************************************************************************
 * @file user_isr_handlers.c
 *
 * @par dependencies
 *
 * @author Ethan-Hang
 *
 * @brief Bridge HAL IRQ callbacks to MPU driver interrupt handlers.
 *
 * Processing flow:
 * Forward EXTI and I2C DMA callbacks through registered function pointers.
 * @version V1.0 2026--
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "main.h"

#include "i2c.h"
#include "bsp_mpuxxxx_handler.h"


//******************************** Includes *********************************//


//******************************** Defines **********************************//

//******************************** Defines **********************************//

//******************************* Declaring *********************************//
void (*pf_pin_interrupt_callback)(void *, void *) = NULL;
void (*pf_dma_interrupt_callback)(void *, void *) = NULL;
extern bsp_mpuxxxx_hanlder_t g_mpuxxxx_handler_instance;

//******************************* Declaring *********************************//

//******************************* Functions *********************************//
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (NULL != pf_pin_interrupt_callback)
    {
        pf_pin_interrupt_callback(g_mpuxxxx_handler_instance.p_driver, NULL);
    }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == &hi2c3)
    {
        pf_dma_interrupt_callback(g_mpuxxxx_handler_instance.p_driver, NULL);
    }
}

//******************************* Functions *********************************//
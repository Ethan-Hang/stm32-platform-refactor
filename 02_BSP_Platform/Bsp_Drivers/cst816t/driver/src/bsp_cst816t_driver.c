/******************************************************************************
 * @file
 *
 * @par dependencies
 *
 * @author Ethan-Hang
 *
 * @brief
 *
 * Processing flow:
 *
 *
 * @version V1.0 2026--
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "bsp_cst816t_driver.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//

//******************************** Defines **********************************//

//******************************* Declaring *********************************//

//******************************* Declaring *********************************//

//******************************* Functions *********************************//

cst816t_status_t bsp_cst816t_inst(
    bsp_cst816t_driver_t                      * const               p_instance,
    cst816t_iic_driver_interface_t      const * const    p_iic_driver_instance,
    cst816t_timebase_interface_t        const * const      p_timebase_instance,
    cst816t_delay_interface_t           const * const         p_delay_instance,
    cst816t_os_delay_interface_t        const * const            p_os_instance,
    void (**pp_int_callback)(void *, void*)
)
{

}
//******************************* Functions *********************************//
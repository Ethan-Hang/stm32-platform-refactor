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

#pragma once
#ifndef __BSP_WRAPPER_TEMP_HUMI_H__
#define __BSP_WRAPPER_TEMP_HUMI_H__

//******************************** Includes *********************************//
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

//******************************** Includes *********************************//

//******************************** Defines **********************************//

//******************************** Defines **********************************//

//******************************* Declaring *********************************//
typedef struct _temp_humi_drv_t
{
    uint32_t                       idx;
    uint32_t                    dev_id;
    void  *                  uesr_data;

    void (*pf_temp_humi_drv_init      )(struct _temp_humi_drv_t * const   dev);

    void (*pf_temp_humi_drv_deinit    )(struct _temp_humi_drv_t * const   dev);

    void (*pf_temp_humi_read_temp     )(struct _temp_humi_drv_t * const   dev, 
                                                          float * const  temp);

    void (*pf_temp_humi_read_humi     )(struct _temp_humi_drv_t * const   dev, 
                                                          float * const  humi);

    void (*pf_temp_humi_read_temp_humi)(struct _temp_humi_drv_t * const   dev, 
                                                          float * const  temp, 
                                                          float * const  humi);
} temp_humi_drv_t;

//******************************* Declaring *********************************//

//******************************* Functions *********************************//
bool drv_adapter_temp_humi_mount(uint32_t idx, temp_humi_drv_t * const drv);

void (temp_humi_drv_init      )(void);

void (temp_humi_drv_deinit    )(void);

void (temp_humi_read_temp     )(float * const temp);

void (temp_humi_read_humi     )(float * const humi);

void (temp_humi_read_temp_humi)(float * const temp, 
                                float * const humi);

//******************************* Functions *********************************//

#endif /* __BSP_WRAPPER_TEMP_HUMI_H__ */

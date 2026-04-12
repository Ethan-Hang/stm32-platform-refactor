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
#include "bsp_wrapper_temp_humi.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define MAX_TEMP_HUMI_DRV_NUM 2
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
static temp_humi_drv_t   s_temp_humi_drv[MAX_TEMP_HUMI_DRV_NUM];
static uint32_t          s_cur_temp_humi_drv_idx = 0;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
bool drv_adapter_temp_humi_mount(uint32_t idx, temp_humi_drv_t * const drv)
{
    if (idx >= MAX_TEMP_HUMI_DRV_NUM || drv == NULL)
    {
        return false;
    }

    s_temp_humi_drv[idx].idx = \
                          idx;
    s_temp_humi_drv[idx].dev_id = \
                          drv->dev_id;
    s_temp_humi_drv[idx].uesr_data = \
                          drv->uesr_data;
    s_temp_humi_drv[idx].pf_temp_humi_drv_init = \
                          drv->pf_temp_humi_drv_init;
    s_temp_humi_drv[idx].pf_temp_humi_drv_deinit = \
                          drv->pf_temp_humi_drv_deinit;
    s_temp_humi_drv[idx].pf_temp_humi_read_temp = \
                          drv->pf_temp_humi_read_temp;
    s_temp_humi_drv[idx].pf_temp_humi_read_humi = \
                          drv->pf_temp_humi_read_humi;
    s_temp_humi_drv[idx].pf_temp_humi_read_all = \
                          drv->pf_temp_humi_read_all;

    s_cur_temp_humi_drv_idx = idx;

    return true;
}

void (temp_humi_drv_init      )(void)
{
    temp_humi_drv_t *temp_drv = &s_temp_humi_drv[s_cur_temp_humi_drv_idx];

    if (temp_drv->pf_temp_humi_drv_init)
    {
        temp_drv->pf_temp_humi_drv_init(temp_drv);
    }

    return;
}

void (temp_humi_drv_deinit    )(void)
{
    temp_humi_drv_t *temp_drv = &s_temp_humi_drv[s_cur_temp_humi_drv_idx];

    if (temp_drv->pf_temp_humi_drv_deinit)
    {
        temp_drv->pf_temp_humi_drv_deinit(temp_drv);
    }

    return;
}


void (temp_humi_read_temp     )(float * const temp)
{
    temp_humi_drv_t *temp_drv = &s_temp_humi_drv[s_cur_temp_humi_drv_idx];

    if (temp_drv->pf_temp_humi_read_temp)
    {
        temp_drv->pf_temp_humi_read_temp(temp_drv, temp);
    }

    return;
}

void (temp_humi_read_humi     )(float * const humi)
{
    temp_humi_drv_t *temp_drv = &s_temp_humi_drv[s_cur_temp_humi_drv_idx];

    if (temp_drv->pf_temp_humi_read_humi)
    {
        temp_drv->pf_temp_humi_read_humi(temp_drv, humi);
    }

    return;
}

void (temp_humi_read_all      )(float * const temp, 
                                float * const humi)
{
    temp_humi_drv_t *temp_drv = &s_temp_humi_drv[s_cur_temp_humi_drv_idx];

    if (temp_drv->pf_temp_humi_read_all)
    {
        temp_drv->pf_temp_humi_read_all(temp_drv, temp, humi);
    }

    return;
}
//******************************* Functions *********************************//

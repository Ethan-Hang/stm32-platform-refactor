/******************************************************************************
 * @file bsp_wrapper_temp_humi.c
 *
 * @par dependencies
 * - bsp_wrapper_temp_humi.h
 *
 * @author Ethan-Hang
 *
 * @brief Abstract interface for temperature/humidity sensor access.
 *
 * @version V1.0 2026--
 * @version V2.0 2026-04-12
 * @upgrade 2.0: Added _sync / _async API variants.
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
static temp_humi_drv_t s_temp_humi_drv[MAX_TEMP_HUMI_DRV_NUM];
static uint32_t        s_cur_temp_humi_drv_idx = 0;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//

bool drv_adapter_temp_humi_mount(uint32_t idx, temp_humi_drv_t *const drv)
{
    if (idx >= MAX_TEMP_HUMI_DRV_NUM || drv == NULL)
    {
        return false;
    }

    s_temp_humi_drv[idx].idx =\
                         idx;
    s_temp_humi_drv[idx].dev_id =\
                         drv->dev_id;
    s_temp_humi_drv[idx].uesr_data =\
                         drv->uesr_data;
    s_temp_humi_drv[idx].pf_temp_humi_drv_init =\
                         drv->pf_temp_humi_drv_init;
    s_temp_humi_drv[idx].pf_temp_humi_drv_deinit =\
                         drv->pf_temp_humi_drv_deinit;
    s_temp_humi_drv[idx].pf_temp_humi_read_temp_sync =\
                         drv->pf_temp_humi_read_temp_sync;
    s_temp_humi_drv[idx].pf_temp_humi_read_humi_sync =\
                         drv->pf_temp_humi_read_humi_sync;
    s_temp_humi_drv[idx].pf_temp_humi_read_all_sync =\
                         drv->pf_temp_humi_read_all_sync;
    s_temp_humi_drv[idx].pf_temp_humi_read_temp_async =\
                         drv->pf_temp_humi_read_temp_async;
    s_temp_humi_drv[idx].pf_temp_humi_read_humi_async =\
                         drv->pf_temp_humi_read_humi_async;
    s_temp_humi_drv[idx].pf_temp_humi_read_all_async =\
                         drv->pf_temp_humi_read_all_async;

    s_cur_temp_humi_drv_idx = idx;

    return true;
}

void temp_humi_drv_init(void)
{
    temp_humi_drv_t *drv = &s_temp_humi_drv[s_cur_temp_humi_drv_idx];
    if (drv->pf_temp_humi_drv_init)
    {
        drv->pf_temp_humi_drv_init(drv);
    }
}

void temp_humi_drv_deinit(void)
{
    temp_humi_drv_t *drv = &s_temp_humi_drv[s_cur_temp_humi_drv_idx];
    if (drv->pf_temp_humi_drv_deinit)
    {
        drv->pf_temp_humi_drv_deinit(drv);
    }
}

/* ---------------------------- Synchronous API ---------------------------- */

void temp_humi_read_temp_sync(float *const temp)
{
    temp_humi_drv_t *drv = &s_temp_humi_drv[s_cur_temp_humi_drv_idx];
    if (drv->pf_temp_humi_read_temp_sync)
    {
        drv->pf_temp_humi_read_temp_sync(drv, temp);
    }
}

void temp_humi_read_humi_sync(float *const humi)
{
    temp_humi_drv_t *drv = &s_temp_humi_drv[s_cur_temp_humi_drv_idx];
    if (drv->pf_temp_humi_read_humi_sync)
    {
        drv->pf_temp_humi_read_humi_sync(drv, humi);
    }
}

void temp_humi_read_all_sync(float *const temp, float *const humi)
{
    temp_humi_drv_t *drv = &s_temp_humi_drv[s_cur_temp_humi_drv_idx];
    if (drv->pf_temp_humi_read_all_sync)
    {
        drv->pf_temp_humi_read_all_sync(drv, temp, humi);
    }
}

/* ---------------------------- Synchronous API ---------------------------- */


/* ---------------------------- ASynchronous API --------------------------- */
void temp_humi_read_temp_async(temp_humi_cb_t callback)
{
    temp_humi_drv_t *drv = &s_temp_humi_drv[s_cur_temp_humi_drv_idx];
    if (drv->pf_temp_humi_read_temp_async)
    {
        drv->pf_temp_humi_read_temp_async(drv, callback);
    }
}

void temp_humi_read_humi_async(temp_humi_cb_t callback)
{
    temp_humi_drv_t *drv = &s_temp_humi_drv[s_cur_temp_humi_drv_idx];
    if (drv->pf_temp_humi_read_humi_async)
    {
        drv->pf_temp_humi_read_humi_async(drv, callback);
    }
}

void temp_humi_read_all_async(temp_humi_cb_t callback)
{
    temp_humi_drv_t *drv = &s_temp_humi_drv[s_cur_temp_humi_drv_idx];
    if (drv->pf_temp_humi_read_all_async)
    {
        drv->pf_temp_humi_read_all_async(drv, callback);
    }
}
/* ---------------------------- ASynchronous API --------------------------- */

//******************************* Functions *********************************//

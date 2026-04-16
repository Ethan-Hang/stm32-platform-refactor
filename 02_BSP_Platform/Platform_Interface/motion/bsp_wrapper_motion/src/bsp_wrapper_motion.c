/******************************************************************************
 * @file bsp_wrapper_motion.c
 *
 * @par dependencies
 * - bsp_wrapper_motion.h
 *
 * @author Ethan-Hang
 *
 * @brief Abstract wrapper implementation for motion sensor access.
 *        Maintains a static array of registered driver vtables and
 *        dispatches all public API calls to the currently active slot.
 *
 * Processing flow:
 *   1. drv_adapter_motion_mount() copies the adapter vtable into s_motion_drv[].
 *   2. All public API functions resolve the active driver via
 *      s_cur_motion_drv_idx and forward through the corresponding function ptr.
 *
 * @version V1.0 2026-04-16
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "bsp_wrapper_motion.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define MOTION_DRV_MAX_NUM 1
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
static motion_drv_t s_motion_drv[MOTION_DRV_MAX_NUM]  = {0};
static uint32_t     s_cur_motion_drv_idx              =  0;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//

bool drv_adapter_motion_mount(uint8_t idx, motion_drv_t *const drv)
{
    if (idx >= MOTION_DRV_MAX_NUM || drv == NULL)
    {
        return false;
    }

    s_motion_drv[idx].idx = \
                      idx;
    s_motion_drv[idx].dev_id = \
                      drv->dev_id;
    s_motion_drv[idx].user_data = \
                      drv->user_data;
    s_motion_drv[idx].pf_motion_drv_init = \
                      drv->pf_motion_drv_init;
    s_motion_drv[idx].pf_motion_drv_deinit = \
                      drv->pf_motion_drv_deinit;
    s_motion_drv[idx].pf_motion_drv_get_req = \
                      drv->pf_motion_drv_get_req;
    s_motion_drv[idx].pf_motion_get_data_addr = \
                      drv->pf_motion_get_data_addr;
    s_motion_drv[idx].pf_motion_read_data_done = \
                      drv->pf_motion_read_data_done;

    s_cur_motion_drv_idx = idx;

    return true;
}

void motion_drv_init(void)
{
    motion_drv_t *p_drv = &s_motion_drv[s_cur_motion_drv_idx];
    if (p_drv->pf_motion_drv_init)
    {
        p_drv->pf_motion_drv_init(p_drv);
    }
}

void motion_drv_deinit(void)
{
    motion_drv_t *p_drv = &s_motion_drv[s_cur_motion_drv_idx];
    if (p_drv->pf_motion_drv_deinit)
    {
        p_drv->pf_motion_drv_deinit(p_drv);
    }
}

wp_motion_status_t motion_drv_get_req(void)
{
    motion_drv_t *p_drv = &s_motion_drv[s_cur_motion_drv_idx];
    if (p_drv->pf_motion_drv_get_req)
    {
        return p_drv->pf_motion_drv_get_req(p_drv);
    }
    return WP_MOTION_ERRORRESOURCE;
}

uint8_t * motion_get_data_addr(void)
{
    motion_drv_t *p_drv = &s_motion_drv[s_cur_motion_drv_idx];
    if (p_drv->pf_motion_get_data_addr)
    {
        return p_drv->pf_motion_get_data_addr(p_drv);
    }
    return NULL;
}

void motion_read_data_done(void)
{
    motion_drv_t *p_drv = &s_motion_drv[s_cur_motion_drv_idx];
    if (p_drv->pf_motion_read_data_done)
    {
        p_drv->pf_motion_read_data_done(p_drv);
    }
}

//******************************* Functions *********************************//

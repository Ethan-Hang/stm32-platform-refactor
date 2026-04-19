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
#include "bsp_wrapper_audio.h"
#include "wt588_cmd.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define AUDIO_DRV_MAX_NUM 1

static drv_audio_t s_audio_drv[AUDIO_DRV_MAX_NUM] = {0};
static uint8_t     s_cur_audio_drv_idx            =   0;

/* Level 1..16 maps linearly onto WT588 volume codes 0xE0..0xEF (1:1). */
static uint8_t prv_volume_level_to_cmd(uint8_t level)
{
    return (uint8_t)(WT588_MIN_VOLUME_CODE + (level - 1U));
}

//******************************** Defines **********************************//

//******************************* Declaring *********************************//

//******************************* Declaring *********************************//

//******************************* Functions *********************************//

bool audio_drv_mount(uint8_t idx, drv_audio_t *const drv)
{
    if (NULL == drv || idx >= AUDIO_DRV_MAX_NUM)
    {
        return false;
    }

    s_audio_drv[idx].idx = \
                          idx;
    s_audio_drv[idx].dev_id = \
                          drv->dev_id;
    s_audio_drv[idx].user_data = \
                          drv->user_data;
    s_audio_drv[idx].pf_audio_drv_init = \
                        drv->pf_audio_drv_init;
    s_audio_drv[idx].pf_audio_drv_deinit = \
                          drv->pf_audio_drv_deinit;
    s_audio_drv[idx].pf_audio_drv_play = \
                          drv->pf_audio_drv_play;
    s_audio_drv[idx].pf_audio_drv_stop = \
                          drv->pf_audio_drv_stop;

    s_cur_audio_drv_idx = idx;

    return true;
}


void audio_drv_init  (void)
{
    drv_audio_t *p_drv = &s_audio_drv[s_cur_audio_drv_idx];
    if (p_drv->pf_audio_drv_init)
    {
        p_drv->pf_audio_drv_init(p_drv);
    }
}

void audio_drv_deinit(void)
{
    drv_audio_t *p_drv = &s_audio_drv[s_cur_audio_drv_idx];
    if (p_drv->pf_audio_drv_deinit)
    {
        p_drv->pf_audio_drv_deinit(p_drv);
    }
}

wp_audio_status_t audio_drv_play  (uint8_t    priority,
                                   uint8_t      volume,
                                   uint8_t  voice_addr)
{
    if (volume < AUDIO_VOLUME_LEVEL_MIN || volume > AUDIO_VOLUME_LEVEL_MAX)
    {
        return WP_AUDIO_ERRORPARAMETER;
    }

    uint8_t volume_cmd = prv_volume_level_to_cmd(volume);

    drv_audio_t *p_drv = &s_audio_drv[s_cur_audio_drv_idx];
    if (p_drv->pf_audio_drv_play)
    {
        return p_drv->pf_audio_drv_play(p_drv, priority, volume_cmd, voice_addr);
    }
    return WP_AUDIO_ERROR;
}

wp_audio_status_t audio_drv_stop  (void)
{
    drv_audio_t *p_drv = &s_audio_drv[s_cur_audio_drv_idx];
    if (p_drv->pf_audio_drv_stop)
    {
        return p_drv->pf_audio_drv_stop(p_drv);
    }
    return WP_AUDIO_ERROR;
}

//******************************* Functions *********************************//

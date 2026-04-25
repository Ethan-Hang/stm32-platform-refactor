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
#include "bsp_wt588_handler.h"
#include "bsp_adapter_port_audio.h"


//******************************** Includes *********************************//

//******************************** Defines **********************************//

//******************************** Defines **********************************//

//******************************* Declaring *********************************//

//******************************* Declaring *********************************//

//******************************* Functions *********************************//
void (wt588_drv_init  )(struct _drv_audio_t * const dev)
{
    (void)dev;    
}

void (wt588_drv_deinit)(struct _drv_audio_t * const dev)

{
    (void)dev;
}

wp_audio_status_t (wt588_drv_play  )(struct _drv_audio_t * const dev,
                                                 uint8_t    priority, 
                                                 uint8_t      volume, 
                                                 uint8_t  voice_addr)
{
    (void)dev;
    wt_handler_status_t ret = WT_HANDLER_OK;

    ret = wt588_handler_play_request(voice_addr, volume, priority);
    if (WT_HANDLER_OK != ret)
    {
        return WP_AUDIO_ERROR;
    }

    return WP_AUDIO_OK;
}   

wp_audio_status_t (wt588_drv_stop  )(struct _drv_audio_t * const dev)
{
    (void)dev;
    wt_handler_status_t ret = WT_HANDLER_OK;
    ret = wt588_handler_stop();
    if (WT_HANDLER_OK != ret)
    {
        return WP_AUDIO_ERROR;
    }

    return WP_AUDIO_OK;
}


bool drv_adapter_audio_register(void)
{
    drv_audio_t audio_drv = {
        .dev_id              =                0,
        .idx                 =                0,
        .user_data           =             NULL,
        .pf_audio_drv_init   =   wt588_drv_init,
        .pf_audio_drv_deinit = wt588_drv_deinit,
        .pf_audio_drv_play   =   wt588_drv_play,
        .pf_audio_drv_stop   =   wt588_drv_stop,
    };

    return audio_drv_mount(0U, &audio_drv);
}

//******************************* Functions *********************************//

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
#ifndef __BSP_WRAPPER_AUDIO_H__
#define __BSP_WRAPPER_AUDIO_H__

//******************************** Includes *********************************//
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define AUDIO_VOLUME_LEVEL_MIN  (1U)   /* Minimum volume level exposed to app */
#define AUDIO_VOLUME_LEVEL_MAX  (16U)  /* Maximum volume level exposed to app */
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
typedef enum
{
    WP_AUDIO_OK               = 0,         /* Operation successful       */
    WP_AUDIO_ERROR            = 1,         /* General error              */
    WP_AUDIO_ERRORTIMEOUT     = 2,         /* Timeout error              */
    WP_AUDIO_ERRORRESOURCE    = 3,         /* Resource unavailable       */
    WP_AUDIO_ERRORPARAMETER   = 4,         /* Invalid parameter          */
    WP_AUDIO_ERRORNOMEMORY    = 5,         /* Out of memory              */
    WP_AUDIO_ERRORUNSUPPORTED = 6,         /* Unsupported feature        */
    WP_AUDIO_ERRORISR         = 7,         /* ISR context error          */
    WP_AUDIO_RESERVED         = 0xFF,      /* Reserved                   */
} wp_audio_status_t;


typedef struct _drv_audio_t
{
    uint32_t                       idx;
    uint32_t                    dev_id;
    void            *        user_data;

    void              (*pf_audio_drv_init   )(struct _drv_audio_t * const dev);
    void              (*pf_audio_drv_deinit )(struct _drv_audio_t * const dev);

    wp_audio_status_t (*pf_audio_drv_play   )(struct _drv_audio_t * const dev, 
                                                          uint8_t    priority, 
                                                          uint8_t      volume, 
                                                          uint8_t  voice_addr);
    wp_audio_status_t (*pf_audio_drv_stop   )(struct _drv_audio_t * const dev);

} drv_audio_t;

//******************************* Declaring *********************************//

//******************************* Functions *********************************//
bool audio_drv_mount (uint8_t idx, drv_audio_t * const drv);

void              audio_drv_init  (void);
void              audio_drv_deinit(void);
wp_audio_status_t audio_drv_play  (uint8_t    priority,
                                   uint8_t      volume,
                                   uint8_t  voice_addr);
wp_audio_status_t audio_drv_stop  (void);

//******************************* Functions *********************************//

#endif /* __BSP_WRAPPER_AUDIO_H__ */

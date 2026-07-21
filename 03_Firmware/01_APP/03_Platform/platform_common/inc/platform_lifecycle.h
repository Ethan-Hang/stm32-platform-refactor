/******************************************************************************
 * @file platform_lifecycle.h
 *
 * @par dependencies
 * - platform_error.h
 *
 * @author Ethan-Hang
 *
 * @brief Provide the platform object lifecycle callback table.
 *
 * Processing flow:
 *
 * 1. The object owner fills this callback table.
 * 2. Managers drive the lifecycle uniformly through this interface.
 * 3. These callbacks describe object lifetime including sleep/wakeup.
 *
 * @version V1.0 2026-07-16
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __PLATFORM_LIFECYCLE_H__
#define __PLATFORM_LIFECYCLE_H__

//******************************** Includes *********************************//
#include "platform_error.h"

//******************************** Includes *********************************//

//****************************** Typedefines ********************************//
typedef struct
{
    platform_err_t (*    init)(void *p_self);
    platform_err_t (*   start)(void *p_self);
    platform_err_t (* process)(void *p_self);
    platform_err_t (*    stop)(void *p_self);
    platform_err_t (*   sleep)(void *p_self);
    platform_err_t (*  wakeup)(void *p_self);
    platform_err_t (*  deinit)(void *p_self);
} platform_lifecycle_ops_t;

typedef enum
{
    PLATFORM_LIFECYCLE_STAGE_INIT     = 0,
    PLATFORM_LIFECYCLE_STAGE_START    = 1,
    PLATFORM_LIFECYCLE_STAGE_PROCESS  = 2,
    PLATFORM_LIFECYCLE_STAGE_STOP     = 3,
    PLATFORM_LIFECYCLE_STAGE_SLEEP    = 4,
    PLATFORM_LIFECYCLE_STAGE_WAKEUP   = 5,
    PLATFORM_LIFECYCLE_STAGE_DEINIT   = 6,
    PLATFORM_LIFECYCLE_STAGE_RESERVED = 0x7FFFFFFF
} platform_lifecycle_stage_t;

//****************************** Typedefines ********************************//

#endif /* __PLATFORM_LIFECYCLE_H__ */ 

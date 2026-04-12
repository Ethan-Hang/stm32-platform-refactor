/******************************************************************************
 * @file Debug.h
 *
 * @par dependencies
 * - "elog.h"
 *
 * @author Ethan-Hang
 *
 * @brief Provide centralized debug tag filtering and logging output macros.
 *
 * Processing flow:
 * Define log tags, filter policy, and DEBUG_OUT wrapper for EasyLogger.
 * @version V1.0 2025-11-20
 * @version V2.0 2026-04-01
 * @upgrade 2.0: 
 * Per-module dual tags (MODULE/MODULE_ERR) for centralized log output
 *  
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/
#ifndef __DEBUG_H__
#define __DEBUG_H__

//******************************** Includes ********************************//
#include <string.h>

#include "elog.h"
//******************************** Includes ********************************//

//******************************** Defines *********************************//

#define DEBUG                 (1) /* Enable centralized debug output       */

/* Keep existing feature flags to avoid behavior changes in other modules. */
#define USED_PWM_CONTROL      (1) /* Enable PWM control for LED operations */

/*
 * Project-level tags: one normal/error pair per module.
 *
 * Ordered by a composite trigger-likelihood score:
 * score = I*1.0 + W*0.7 + D*0.5 + V*0.3 + sqrt(E)*0.3 + Ref*0.03
 */
#define WT588_HANDLER_LOG_TAG               "WT588_HANDLER"
#define MPUXXXX_ERR_LOG_TAG                   "MPUXXXX_ERR"
#define WT588_LOG_TAG                               "WT588"
#define AHT21_LOG_TAG                               "AHT21"
#define TEMP_HUMI_LOG_TAG                       "TEMP_HUMI"
#define MPUXXXX_LOG_TAG                           "MPUXXXX"
#define UNPACK_LOG_TAG                             "UNPACK"
#define WT588_ERR_LOG_TAG                       "WT588_ERR"
#define TEMP_HUMI_ERR_LOG_TAG               "TEMP_HUMI_ERR"
#define AHT21_ERR_LOG_TAG                       "AHT21_ERR"
#define WT588_HANDLER_ERR_LOG_TAG       "WT588_HANDLER_ERR"
#define CORE_LOG_TAG                                 "CORE"
#define USER_INIT_LOG_TAG                       "USER_INIT"
#define HAL_IIC_ERR_LOG_TAG                   "HAL_IIC_ERR"
#define UNPACK_ERR_LOG_TAG                     "UNPACK_ERR"
#define USER_INIT_ERR_LOG_TAG               "USER_INIT_ERR"
#define CORE_ERR_LOG_TAG                         "CORE_ERR"
#define TEMP_HUMI_TEST_LOG_TAG             "TEMP_HUMI_TEST"
#define TEMP_HUMI_TEST_ERR_LOG_TAG     "TEMP_HUMI_TEST_ERR"

static inline int debug_is_tag_allowed(const char *tag)
{
    if ((tag == NULL) || (tag[0] == '\0'))
    {
        return 0;
    }

    return\
            // (strcmp(           UNPACK_LOG_TAG, tag) == 0)                    ||
            (strcmp(    WT588_HANDLER_LOG_TAG, tag) == 0)                    ||
            (strcmp(      MPUXXXX_ERR_LOG_TAG, tag) == 0)                    ||
            (strcmp(            WT588_LOG_TAG, tag) == 0)                    ||
            (strcmp(            AHT21_LOG_TAG, tag) == 0)                    ||
            (strcmp(        TEMP_HUMI_LOG_TAG, tag) == 0)                    ||
            (strcmp(          MPUXXXX_LOG_TAG, tag) == 0)                    ||
            (strcmp(        WT588_ERR_LOG_TAG, tag) == 0)                    ||
            (strcmp(    TEMP_HUMI_ERR_LOG_TAG, tag) == 0)                    ||
            (strcmp(        AHT21_ERR_LOG_TAG, tag) == 0)                    ||
            (strcmp(WT588_HANDLER_ERR_LOG_TAG, tag) == 0)                    ||
            (strcmp(             CORE_LOG_TAG, tag) == 0)                    ||
            (strcmp(        USER_INIT_LOG_TAG, tag) == 0)                    ||
            (strcmp(      HAL_IIC_ERR_LOG_TAG, tag) == 0)                    ||
            (strcmp(       UNPACK_ERR_LOG_TAG, tag) == 0)                    ||
            (strcmp(    USER_INIT_ERR_LOG_TAG, tag) == 0)                    ||
            (strcmp(         CORE_ERR_LOG_TAG, tag) == 0)                    ||
            (strcmp(   TEMP_HUMI_TEST_LOG_TAG, tag) == 0)                    ||
            (strcmp(TEMP_HUMI_TEST_ERR_LOG_TAG, tag) == 0);
}

/*
 * Usage:
 * DEBUG_OUT(i, AHT21_LOG_TAG, "AHT21 init ok");
 * DEBUG_OUT(e, AHT21_ERR_LOG_TAG, "AHT21 read timeout");
 */
#define DEBUG_OUT(LEVEL, TAG, ...)                                            \
    do                                                                        \
    {                                                                         \
        if (DEBUG)                                                            \
        {                                                                     \
            const char *debug_tag__ = (TAG);                                  \
            if (DEBUG && debug_is_tag_allowed(debug_tag__))                   \
            {                                                                 \
                elog_##LEVEL(debug_tag__, __VA_ARGS__);                       \
            }                                                                 \
        }                                                                     \
    } while (0)

void debug_init(void);

//********************************* Defines *********************************//

//******************************** Declaring ********************************//

//******************************** Declaring ********************************//


#endif // End of __DEBUG_H__

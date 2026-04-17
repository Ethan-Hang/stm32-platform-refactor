/******************************************************************************
 * @file wt588_test_task.c
 *
 * @par dependencies
 * - user_task_reso_config.h
 * - bsp_wt588_handler.h
 *
 * @author Ethan-Hang
 *
 * @brief WT588 voice playback test cases.
 *
 * Processing flow:
 * Wait for handler to initialise, then submit play requests for voice 0x00
 * and 0x01 with volume 0xE1.  Each request is followed by a delay so the
 * previous playback can finish before the next one starts.
 *
 * @version V1.0 2026-04-16
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "user_task_reso_config.h"
#include "bsp_wt588_handler.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define WT588_TEST_VOLUME       (0xE5U)
#define WT588_TEST_PRIORITY     (1U)
#define WT588_TEST_PLAY_GAP_MS  (1000U)
#define WT588_TEST_INIT_WAIT_MS (3000U)
//******************************** Defines **********************************//

//******************************* Functions *********************************//
void wt588_test_task(void *argument)
{
    (void)argument;

    DEBUG_OUT(i, WT588_HANDLER_LOG_TAG, "wt588_test_task started");

    /* Wait for handler thread to finish initialisation */
    osal_task_delay(WT588_TEST_INIT_WAIT_MS);

    wt_handler_status_t ret;

    DEBUG_OUT(i, WT588_HANDLER_LOG_TAG,
              "[TEST1] play 0x00, volume 0xE1");
    ret = wt588_handler_play_request(0x00U, WT588_TEST_VOLUME,
                                     WT588_TEST_PRIORITY);
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "[TEST1] play_request failed, ret=%d", ret);
    }

    /* ---- Test 1: play voice 0x00, volume 0xE1 ---- */
    DEBUG_OUT(i, WT588_HANDLER_LOG_TAG,
              "[TEST1] play 0x00, volume 0xE1");
    ret = wt588_handler_play_request(0x01U, WT588_TEST_VOLUME,
                                     WT588_TEST_PRIORITY);
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "[TEST1] play_request failed, ret=%d", ret);
    }

    osal_task_delay(WT588_TEST_PLAY_GAP_MS);

    /* ---- Test 2: play voice 0x01, volume 0xE1 ---- */
    DEBUG_OUT(i, WT588_HANDLER_LOG_TAG,
              "[TEST2] play 0x01, volume 0xE1");
    ret = wt588_handler_play_request(0x02U, WT588_TEST_VOLUME,
                                     WT588_TEST_PRIORITY);
    if (WT_HANDLER_OK != ret)
    {
        DEBUG_OUT(e, WT588_HANDLER_ERR_LOG_TAG,
                  "[TEST2] play_request failed, ret=%d", ret);
    }

    DEBUG_OUT(i, WT588_HANDLER_LOG_TAG, "wt588_test_task all tests done");
    
    for (;;)
    {
        osal_task_delay(1000U);
    }
}
//******************************* Functions *********************************//

/******************************************************************************
 * @file bsp_adapter_port_temp_humi.c
 *
 * @par dependencies
 * - bsp_temp_humi_xxx_handler.h
 * - bsp_adapter_port_temp_humi.h
 * - bsp_wrapper_temp_humi.h
 * - osal_wrapper_adapter.h
 * - osal_error.h
 * - Debug.h
 *
 * @author Ethan-Hang
 *
 * @brief Adapter port for AHT21 temperature/humidity sensor.
 *
 * Provides concrete implementations for bsp_wrapper_temp_humi's vtable.
 *
 * Synchronous reads (_sync) are made blocking from the caller's perspective
 * by posting an event to the handler queue and then waiting on an internal
 * event group until aht21_data_ready_cb() signals completion:
 *
 *   caller ─► temp_humi_read_all_sync()
 *                ├─ bsp_temp_humi_xxx_read()   ── posts event to handler queue
 *                └─ osal_event_group_wait_bits() ── blocks caller
 *                        ↑
 *               handler reads AHT21 → aht21_data_ready_cb()
 *                 → stores result in s_temp_result / s_humi_result
 *                 → sets event group bits  →  caller unblocks
 *   caller ◄── *temp / *humi filled with fresh values
 *
 * Asynchronous reads (_async) post the user-supplied callback directly to
 * the handler queue and return immediately.  The callback fires in the
 * handler thread context when data is ready.
 *
 * Hardware porting (IIC, OS queue, timebase, yield) lives in
 * aht21_integration.c — this file contains only synchronisation logic.
 *
 * @version V1.0 2026-04-12
 * @version V2.0 2026-04-12
 * @upgrade 2.0: Event-group sync added; sync/async API split.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "bsp_temp_humi_xxx_handler.h"
#include "bsp_adapter_port_temp_humi.h"
#include "bsp_wrapper_temp_humi.h"
#include "osal_wrapper_adapter.h"
#include "osal_error.h"
#include "Debug.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/** Wait up to 5 s for the handler to complete a read before giving up. */
#define ADAPTER_EG_READ_TIMEOUT_TICKS  (5000U)

/** Event group bit set by aht21_data_ready_cb when temperature is ready. */
#define ADAPTER_EG_BIT_TEMP  (1U << 0)
/** Event group bit set by aht21_data_ready_cb when humidity is ready. */
#define ADAPTER_EG_BIT_HUMI  (1U << 1)
/** Combined mask for both bits. */
#define ADAPTER_EG_BIT_BOTH  (ADAPTER_EG_BIT_TEMP | ADAPTER_EG_BIT_HUMI)
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
static void aht21_drv_init      (temp_humi_drv_t *const dev);
static void aht21_drv_deinit    (temp_humi_drv_t *const dev);
static void aht21_read_temp_sync(temp_humi_drv_t *const dev, float *const temp);
static void aht21_read_humi_sync(temp_humi_drv_t *const dev, float *const humi);
static void aht21_read_all_sync (temp_humi_drv_t *const dev,
                                  float *const temp, float *const humi);
static void aht21_read_temp_async(temp_humi_drv_t *const dev, temp_humi_cb_t cb);
static void aht21_read_humi_async(temp_humi_drv_t *const dev, temp_humi_cb_t cb);
static void aht21_read_all_async (temp_humi_drv_t *const dev, temp_humi_cb_t cb);
static void aht21_data_ready_cb  (float *temp, float *humi);
//******************************* Declaring *********************************//

//******************************** Variables ********************************//
/**
 * Result storage written by aht21_data_ready_cb() (handler thread context)
 * and read back by the _sync functions after the event group unblocks them.
 */
static volatile float              s_temp_result  = 0.0f;
static volatile float              s_humi_result  = 0.0f;

/** Event group created lazily on the first _sync call. */
static osal_event_group_handle_t   s_eg_handle    = NULL;

/** Type of the in-flight _sync request; used by the callback to set bits. */
static temp_humi_data_type_event_t s_pending_type = TEMP_HUMI_EVENT_BOTH;
//******************************** Variables ********************************//

//******************************* Functions *********************************//

/* ---------- Internal helpers ---------------------------------------------- */

/**
 * @brief Ensure the event group has been created (lazy init).
 */
static bool adapter_eg_ensure_init(void)
{
    if (NULL != s_eg_handle)
    {
        return true;
    }

    int32_t ret = osal_event_group_create(&s_eg_handle);
    if (OSAL_SUCCESS != ret)
    {
        DEBUG_OUT(e, TEMP_HUMI_ERR_LOG_TAG,
                  "adapter: event group create failed (%d)", (int)ret);
        return false;
    }

    return true;
}

/**
 * @brief Submit a read event and block until the callback signals completion.
 */
static void adapter_sync_read(temp_humi_data_type_event_t event_type,
                               uint32_t                    bits)
{
    if (!adapter_eg_ensure_init())
    {
        return;
    }

    s_pending_type = event_type;

    temp_humi_xxx_event_t event = {
        .event_type  = event_type,
        .lifetime    = 0,
        .pf_callback = aht21_data_ready_cb,
    };

    temp_humi_status_t ret = bsp_temp_humi_xxx_read(&event);
    if (TEMP_HUMI_OK != ret)
    {
        DEBUG_OUT(e, TEMP_HUMI_ERR_LOG_TAG,
                  "adapter_sync_read: post failed (%d)", (int)ret);
        return;
    }

    int32_t eg_ret = osal_event_group_wait_bits(
                        s_eg_handle,
                        bits,
                        true,
                        false,
                        ADAPTER_EG_READ_TIMEOUT_TICKS,
                        NULL);

    if (OSAL_SUCCESS != eg_ret)
    {
        DEBUG_OUT(e, TEMP_HUMI_ERR_LOG_TAG,
                  "adapter_sync_read: wait timeout");
    }
}

/**
 * @brief Callback used by _sync reads.
 *
 * Runs in the handler task context.  Stores the sensor values and sets
 * the event group bits so the waiting caller task is unblocked.
 */
static void aht21_data_ready_cb(float *temp, float *humi)
{
    uint32_t bits = 0U;

    if (temp != NULL) { s_temp_result = *temp; }
    if (humi != NULL) { s_humi_result = *humi; }

    switch (s_pending_type)
    {
    case TEMP_HUMI_EVENT_TEMP:
        bits = ADAPTER_EG_BIT_TEMP;
        break;
    case TEMP_HUMI_EVENT_HUMI:
        bits = ADAPTER_EG_BIT_HUMI;
        break;
    case TEMP_HUMI_EVENT_BOTH:
    default:
        bits = ADAPTER_EG_BIT_BOTH;
        break;
    }

    if (NULL != s_eg_handle)
    {
        (void)osal_event_group_set_bits(s_eg_handle, bits);
    }
}

/* ---------- Wrapper vtable implementations -------------------------------- */

bool drv_adapter_temp_humi_register(void)
{
    temp_humi_drv_t temp_humi_drv = {
        .idx                          = 0,
        .dev_id                       = 0,
        .pf_temp_humi_drv_init        = aht21_drv_init,
        .pf_temp_humi_drv_deinit      = aht21_drv_deinit,
        .pf_temp_humi_read_temp_sync  = aht21_read_temp_sync,
        .pf_temp_humi_read_humi_sync  = aht21_read_humi_sync,
        .pf_temp_humi_read_all_sync   = aht21_read_all_sync,
        .pf_temp_humi_read_temp_async = aht21_read_temp_async,
        .pf_temp_humi_read_humi_async = aht21_read_humi_async,
        .pf_temp_humi_read_all_async  = aht21_read_all_async,
    };

    return drv_adapter_temp_humi_mount(0, &temp_humi_drv);
}

static void aht21_drv_init(temp_humi_drv_t *const dev)
{
    (void)dev;
}

static void aht21_drv_deinit(temp_humi_drv_t *const dev)
{
    (void)dev;
}

/* --- Synchronous vtable slots --- */

static void aht21_read_temp_sync(temp_humi_drv_t *const dev, float *const temp)
{
    (void)dev;
    adapter_sync_read(TEMP_HUMI_EVENT_TEMP, ADAPTER_EG_BIT_TEMP);
    if (temp != NULL) { *temp = s_temp_result; }
}

static void aht21_read_humi_sync(temp_humi_drv_t *const dev, float *const humi)
{
    (void)dev;
    adapter_sync_read(TEMP_HUMI_EVENT_HUMI, ADAPTER_EG_BIT_HUMI);
    if (humi != NULL) { *humi = s_humi_result; }
}

static void aht21_read_all_sync(temp_humi_drv_t *const dev,
                                 float *const temp, float *const humi)
{
    (void)dev;
    adapter_sync_read(TEMP_HUMI_EVENT_BOTH, ADAPTER_EG_BIT_BOTH);
    if (temp != NULL) { *temp = s_temp_result; }
    if (humi != NULL) { *humi = s_humi_result; }
}

/* --- Asynchronous vtable slots --- */

static void aht21_read_temp_async(temp_humi_drv_t *const dev, temp_humi_cb_t cb)
{
    (void)dev;
    if (NULL == cb) { return; }
    temp_humi_xxx_event_t event = {
        .event_type  = TEMP_HUMI_EVENT_TEMP,
        .lifetime    = 0,
        .pf_callback = cb,
    };
    (void)bsp_temp_humi_xxx_read(&event);
}

static void aht21_read_humi_async(temp_humi_drv_t *const dev, temp_humi_cb_t cb)
{
    (void)dev;
    if (NULL == cb) { return; }
    temp_humi_xxx_event_t event = {
        .event_type  = TEMP_HUMI_EVENT_HUMI,
        .lifetime    = 0,
        .pf_callback = cb,
    };
    (void)bsp_temp_humi_xxx_read(&event);
}

static void aht21_read_all_async(temp_humi_drv_t *const dev, temp_humi_cb_t cb)
{
    (void)dev;
    if (NULL == cb) { return; }
    temp_humi_xxx_event_t event = {
        .event_type  = TEMP_HUMI_EVENT_BOTH,
        .lifetime    = 0,
        .pf_callback = cb,
    };
    (void)bsp_temp_humi_xxx_read(&event);
}

//******************************* Functions *********************************//

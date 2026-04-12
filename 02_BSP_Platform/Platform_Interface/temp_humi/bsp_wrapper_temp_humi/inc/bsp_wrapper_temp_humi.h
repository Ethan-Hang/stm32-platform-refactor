/******************************************************************************
 * @file bsp_wrapper_temp_humi.h
 *
 * @par dependencies
 *
 * @author Ethan-Hang
 *
 * @brief Abstract interface for temperature/humidity sensor access.
 *
 * Exposes two read modes to the application layer:
 *
 *  _sync  — blocks the caller until fresh data is available.
 *           Returns values directly via float * out-parameters.
 *
 *  _async — posts a read request and returns immediately.
 *           The supplied callback is invoked from the handler thread
 *           context once data is ready.
 *
 * A concrete driver is registered at startup via drv_adapter_temp_humi_mount().
 *
 * @version V1.0 2026-04-12
 * @version V2.0 2026-04-12
 * @upgrade 2.0: Added _sync / _async API variants.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __BSP_WRAPPER_TEMP_HUMI_H__
#define __BSP_WRAPPER_TEMP_HUMI_H__

//******************************** Includes *********************************//
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
//******************************** Includes *********************************//

//******************************* Declaring *********************************//

/**
 * @brief Callback type for asynchronous reads.
 *
 * Both pointers are always provided; for single-axis reads the unused
 * pointer carries 0.0f.
 */
typedef void (*temp_humi_cb_t)(float *temp, float *humi);

/**
 * @brief Driver vtable — populated at startup by drv_adapter_temp_humi_mount()
 */
typedef struct _temp_humi_drv_t
{
    uint32_t                       idx;
    uint32_t                    dev_id;
    void    *                 uesr_data;

    void (*pf_temp_humi_drv_init           )(struct _temp_humi_drv_t *const dev);
    void (*pf_temp_humi_drv_deinit         )(struct _temp_humi_drv_t *const dev);

    /* Synchronous — block caller until data is ready */
    void (*pf_temp_humi_read_temp_sync     )(struct _temp_humi_drv_t *const dev,
                                                               float *const temp);
    void (*pf_temp_humi_read_humi_sync     )(struct _temp_humi_drv_t *const dev,
                                                               float *const humi);
    void (*pf_temp_humi_read_all_sync      )(struct _temp_humi_drv_t *const dev,
                                                               float *const temp,
                                                               float *const humi);

    /* Asynchronous — return immediately, result delivered via callback */
    void (*pf_temp_humi_read_temp_async    )(struct _temp_humi_drv_t *const dev,
                                                           temp_humi_cb_t    cb);
    void (*pf_temp_humi_read_humi_async    )(struct _temp_humi_drv_t *const dev,
                                                           temp_humi_cb_t    cb);
    void (*pf_temp_humi_read_all_async     )(struct _temp_humi_drv_t *const dev,
                                                           temp_humi_cb_t    cb);
} temp_humi_drv_t;

//******************************* Declaring *********************************//

//******************************* Functions *********************************//

/** Register a concrete driver into the wrapper vtable slot @p idx. */
bool drv_adapter_temp_humi_mount(uint32_t idx, temp_humi_drv_t *const drv);

void temp_humi_drv_init  (void);
void temp_humi_drv_deinit(void);

/** Synchronous API — caller blocks until sensor data is ready. */
void temp_humi_read_temp_sync(float *const temp);
void temp_humi_read_humi_sync(float *const humi);
void temp_humi_read_all_sync (float *const temp, float *const humi);

/** Asynchronous API — returns immediately; 
 * @p callback fires in handler context. */
void temp_humi_read_temp_async(temp_humi_cb_t callback);
void temp_humi_read_humi_async(temp_humi_cb_t callback);
void temp_humi_read_all_async (temp_humi_cb_t callback);

//******************************* Functions *********************************//

#endif /* __BSP_WRAPPER_TEMP_HUMI_H__ */

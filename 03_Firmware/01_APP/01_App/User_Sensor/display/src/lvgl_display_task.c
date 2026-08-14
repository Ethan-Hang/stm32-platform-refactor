/******************************************************************************
 * @file lvgl_display_task.c
 *
 * @par dependencies
 * - bsp_wrapper_display.h
 * - bsp_wrapper_touch.h
 * - lvgl.h
 * - lv_port_disp.h
 * - lv_port_indev.h
 * - gui_guider.h
 *
 * @author Ethan-Hang
 *
 * @brief LVGL bring-up task.  Drives the display + touch entirely through
 *        the platform wrapper APIs (no direct driver coupling), wires them
 *        into LVGL's display + pointer indev, then hands control to the
 *        gui_guider-generated UI.
 *
 * Processing flow:
 *   display_drv_init -> display_fill_color(BLACK) ->
 *   touch_drv_init  -> touch_get_chip_id (probe) ->
 *   lv_init -> lv_log_register_print_cb (bridge to DEBUG_OUT) ->
 *   lv_port_disp_init -> lv_port_indev_init ->
 *   setup_ui(&guider_ui) ->
 *   loop { lv_timer_handler; delay 5 ms }
 *
 * @note  Vtable mounting happens pre-kernel via drv_adapter_*_register()
 *        in platform_io_register().  Actual driver instantiation
 *        (bsp_st7789_driver_inst / bsp_cst816t_inst) is deferred to this
 *        task — it depends on OSAL primitives (bus mutex, os delay) that
 *        are only valid after osKernelStart() — and is reached purely
 *        through the wrapper API (display_drv_inst / touch_drv_inst).
 *
 * @version V1.0 2026-04-25
 * @version V2.0 2026-04-26
 * @version V3.0 2026-04-28
 * @version V4.0 2026-04-28
 * @version V5.0 2026-06-04
 * @upgrade 2.0: Removed the local ST7789 / CST816T driver bind code; the
 *               task now drives both peripherals through bsp_wrapper_display
 *               and bsp_wrapper_touch.
 * @upgrade 3.0: Task now triggers OSAL-dependent driver instantiation
 *               before the wrapper init step (was previously running
 *               pre-kernel inside drv_adapter_*_register()).
 * @upgrade 4.0: Inst now goes through display_drv_inst / touch_drv_inst on
 *               the wrapper layer instead of bsp_adapter_port_*; the task
 *               no longer depends on the concrete adapter port headers.
 * @upgrade 5.0: lvgl_log_output_cb now deep-matches LVGL's level prefix
 *               ([Trace]/[Info]/[Warn]/[Error]/[User]) and forwards through
 *               the matching EasyLogger level (d/i/w/e) instead of flattening
 *               everything to info; LVGL's duplicate "[Level]\t(time)\t"
 *               header is stripped so EasyLogger's own level + timestamp win.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "platform_type.h"
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "user_task_reso_config.h"
#include "bsp_wrapper_display.h"
#include "bsp_wrapper_touch.h"
#include "service_storage_facade.h"
#include "platform_def.h"
#include "Debug.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_port_extflash.h"
#include "gui_guider.h"
#include "ui_hr_view.h"
#include "ui_temp_humi_view.h"

#include "touch_calibration_boot.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define LV_TASK_BOOT_WAIT_MS      2000U
#define LV_TASK_TIMER_PERIOD_MS   5U

/* RGB565 black, used to clear the panel before LVGL takes over. */
#define LV_TASK_BG_COLOR_BLACK    0x0000U

/* LVGL pool snapshot period; screen changes report immediately regardless. */
#define LV_MEM_REPORT_PERIOD_MS   100U
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
/**
 * gui_guider.h declares `extern lv_ui guider_ui` but the generated tree
 * does not provide a definition; supply it here so setup_ui() and the
 * generated screen handlers link.  Keeping it file-private to this task
 * gives one owner.
 **/
lv_ui guider_ui;
//******************************* Declaring *********************************//

//******************************* Functions *********************************//

/**
 * LVGL log levels, mirrored from lv_log.h's textual prefixes.  This LVGL
 * build hands the print callback only a pre-formatted string (the callback
 * signature is `void(const char *)`, no level enum), so the level has to be
 * recovered from the leading "[Trace]/[Info]/[Warn]/[Error]/[User]" token.
 */
typedef enum
{
    LVGL_LOG_LVL_TRACE = 0,
    LVGL_LOG_LVL_INFO,
    LVGL_LOG_LVL_WARN,
    LVGL_LOG_LVL_ERROR,
    LVGL_LOG_LVL_USER,
} lvgl_log_level_t;

/**
 * @brief  Recover the LVGL log level from the buffer's "[Level]" prefix and
 *         locate the human-readable message body.
 *
 *         LVGL formats every _lv_log_add() line as:
 *             "[Warn]\t(t.ms, +dt)\t func: message \t(in file line #n)\n"
 *         The leading "[Level]\t(time)\t " header duplicates the level tag +
 *         timestamp that EasyLogger already prints, so we skip past it (the
 *         body starts after the second tab) and keep only
 *         "func: message (in file line #n)".  Plain lv_log() output has no
 *         "[Level]" prefix nor tabs — it falls through to INFO + whole buffer.
 *
 * @param[in]  buf  : NUL-terminated log string from LVGL (non-NULL).
 * @param[out] body : receives a pointer into @p buf at the message body.
 *
 * @return     The decoded level (defaults to LVGL_LOG_LVL_INFO).
 */
static lvgl_log_level_t lvgl_log_parse_level(const char *buf, const char **body)
{
    static const struct
    {
        const char       *name;
        lvgl_log_level_t   level;
    } k_level_map[] = {
        { "[Trace]", LVGL_LOG_LVL_TRACE },
        { "[Info]",  LVGL_LOG_LVL_INFO  },
        { "[Warn]",  LVGL_LOG_LVL_WARN  },
        { "[Error]", LVGL_LOG_LVL_ERROR },
        { "[User]",  LVGL_LOG_LVL_USER  },
    };

    lvgl_log_level_t level = LVGL_LOG_LVL_INFO;

    /* Default: unknown format / plain lv_log() — keep the whole buffer. */
    *body = buf;

    if ('[' != buf[0])
    {
        return level;
    }

    for (SIZE_T i = 0U; i < ARRAY_SIZE(k_level_map); i++)
    {
        if (0 == strncmp(buf, k_level_map[i].name,
                         strlen(k_level_map[i].name)))
        {
            level = k_level_map[i].level;
            break;
        }
    }

    /* Advance the body pointer past the "[Level]\t(time)\t " header — i.e. to
     * just after the second tab.  Leave *body at the whole buffer if the
     * format is unexpected (fewer than two tabs). */
    int tabs = 0;
    for (const char *p = buf; '\0' != *p; p++)
    {
        if ('\t' == *p)
        {
            if (2 == ++tabs)
            {
                const char *msg = p + 1;
                while (' ' == *msg)
                {
                    msg++;
                }
                *body = msg;
                break;
            }
        }
    }

    return level;
}

/**
 * @brief LVGL log bridge: receives pre-formatted log buffers from LVGL's
 *        internal _lv_log_add() and routes them through the project's
 *        centralized DEBUG_OUT → EasyLogger → RTT pipeline, mapping each
 *        LVGL level onto the matching EasyLogger level so warnings/errors
 *        surface as W/E (not flattened to info).
 *
 *        DEBUG_OUT()'s level is a compile-time token (elog_##LEVEL), hence
 *        the per-level switch instead of a single parameterized call.
 *
 *        LVGL's buffer ends with '\n'; we strip it because DEBUG_OUT
 *        (EasyLogger) appends its own line terminator.
 *
 * @param[in] buf : NUL-terminated log string from LVGL.
 */
static void lvgl_log_output_cb(const char *buf)
{
    if (NULL == buf)
    {
        return;
    }

    const char            *body  = buf;
    const lvgl_log_level_t  level = lvgl_log_parse_level(buf, &body);

    /* Temporarily strip the trailing '\n'.  LVGL's buffer is a stack-local in
     * _lv_log_add(); writing is safe and we restore it before returning.
     * Truncating the whole buffer also terminates the body substring. */
    SIZE_T      blen     = strlen(buf);
    BOOL_T        stripped = (blen > 0U) && ('\n' == buf[blen - 1U]);
    char *const mut_buf  = (char *const)buf;
    if (stripped)
    {
        mut_buf[blen - 1U] = '\0';
    }

    switch (level)
    {
        case LVGL_LOG_LVL_TRACE:
            DEBUG_OUT(d, LVGL_LOG_TAG, "%s", body);
            break;
        case LVGL_LOG_LVL_WARN:
            DEBUG_OUT(w, LVGL_LOG_TAG, "%s", body);
            break;
        case LVGL_LOG_LVL_ERROR:
            DEBUG_OUT(e, LVGL_LOG_TAG, "%s", body);
            break;
        case LVGL_LOG_LVL_USER:
        case LVGL_LOG_LVL_INFO:
        default:
            DEBUG_OUT(i, LVGL_LOG_TAG, "%s", body);
            break;
    }

    if (stripped)
    {
        mut_buf[blen - 1U] = '\n';
    }
}

/**
 * Maps an active-screen pointer back to its gui_guider name so a pool
 * snapshot can be attributed to the screen that was on-screen when it was
 * taken.  `&guider_ui.<field>` is a link-time constant (guider_ui is a
 * global), so the table lives in .rodata; only the pointed-to lv_obj_t*
 * changes at runtime.
 */
typedef struct
{
    lv_obj_t *const *scr;
    const char      *name;
} lvgl_scr_name_t;

static const lvgl_scr_name_t s_scr_names[] =
{
    { &guider_ui.Clock_1,             "Clock_1"    },
    { &guider_ui.Clock_2,             "Clock_2"    },
    { &guider_ui.Clock_3,             "Clock_3"    },
    { &guider_ui.top_lap,             "top_lap"    },
    { &guider_ui.under_up,            "under_up"   },
    { &guider_ui.List_1,              "List_1"     },
    { &guider_ui.List_2,              "List_2"     },
    { &guider_ui.List_3,              "List_3"     },
    { &guider_ui.Heart,               "Heart"      },
    { &guider_ui.Map,                 "Map"        },
    { &guider_ui.NFC,                 "NFC"        },
    { &guider_ui.QRcode,              "QRcode"     },
    { &guider_ui.Systeamupdate,       "SysUpd"     },
    { &guider_ui.Systeamupdate_cheak, "SysUpdChk"  },
    { &guider_ui.Set,                 "Set"        },
    { &guider_ui.Error,               "Error"      },
};

/**
 * @brief      Resolve the currently loaded screen to a printable name.
 *
 * @param[in]  act : Result of lv_scr_act().
 *
 * @return     Static string; "?" when the screen is not a gui_guider one
 *             (e.g. the touch-calibration screen created outside the UI).
 */
static const char *lvgl_scr_name(const lv_obj_t *act)
{
    for (SIZE_T i = 0u; i < ARRAY_SIZE(s_scr_names); i++)
    {
        if (act == *(s_scr_names[i].scr))
        {
            return s_scr_names[i].name;
        }
    }
    return "?";
}

/**
 * @brief      Emit one LVGL-pool snapshot to RTT terminal 4.
 *
 * @param[in]  why : Short reason string, so a periodic sample can be told
 *                   apart from a screen-transition sample in the log.
 *
 * @return     None.
 *
 * @note       Reports free_biggest_size alongside free_size on purpose:
 *             lv_mem_realloc failing while free_size is still large means
 *             fragmentation, not exhaustion, and only the gap between
 *             those two numbers shows it.
 *
 * @warning    Deliberately does NOT report mon.max_used.  LVGL only bumps
 *             its `max_used` from lv_mem_alloc() (lv_mem.c:158), while
 *             lv_mem_realloc() bypasses the `cur_used` bookkeeping
 *             entirely -- and realloc is what grows every style property
 *             array.  `max_used` therefore reads far below the truth
 *             (observed 6981 against a real 20504).  The peak tracked
 *             here is derived from total_size - free_size, which comes
 *             straight off the TLSF walk and cannot drift.
 */
static void lvgl_mem_report(const char *why)
{
    static UINT32_T s_peak_used = 0u;

    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);

    UINT32_T used = (UINT32_T)(mon.total_size - mon.free_size);
    if (used > s_peak_used)
    {
        s_peak_used = used;
    }

    DEBUG_OUT(i, LVGL_MEM_LOG_TAG,
              "%-9s scr=%-9s used=%lu/%lu (%u%%) peak=%lu free=%lu "
              "big=%lu frag=%u%% ucnt=%lu fcnt=%lu parked=%lu",
              why,
              lvgl_scr_name(lv_scr_act()),
              (unsigned long)used,
              (unsigned long)mon.total_size,
              (unsigned int)mon.used_pct,
              (unsigned long)s_peak_used,
              (unsigned long)mon.free_size,
              (unsigned long)mon.free_biggest_size,
              (unsigned int)mon.frag_pct,
              (unsigned long)mon.used_cnt,
              (unsigned long)mon.free_cnt,
              (unsigned long)lv_mem_buf_get_parked());
}

/**
 * @brief      Periodic + event-driven LVGL pool sampling, called once per
 *             service-loop iteration.
 *
 * Samples on a fixed period, and additionally the moment the active screen
 * changes — screen teardown/build is where the big allocations happen, so
 * the periodic sample alone would usually miss the peak.
 *
 * @return     None.
 */
static void lvgl_mem_monitor_poll(void)
{
    static lv_obj_t *s_last_scr  = NULL;
    static UINT32_T  s_last_tick = 0u;

    lv_obj_t *act = lv_scr_act();
    if (act != s_last_scr)
    {
        s_last_scr = act;
        lvgl_mem_report("scr-load");
    }

    UINT32_T now = (UINT32_T)osal_task_get_tick_count();
    if ((now - s_last_tick) >= LV_MEM_REPORT_PERIOD_MS)
    {
        s_last_tick = now;
        lvgl_mem_report("periodic");
    }
}

/**
 * @brief      LVGL + gui_guider task entry: brings up display and touch
 *             through the platform wrapper APIs, then hands control to
 *             the gui_guider-generated UI.
 *
 * @param[in]  argument : Unused.
 *
 * @return     None.
 * */
void lvgl_display_task(void *argument)
{
    (void)argument;

    /**
     * Boot wait gives lower-priority init paths (e.g. logging, queues) time
     * to come up before we start hammering the display.
     **/
    osal_task_delay(LV_TASK_BOOT_WAIT_MS);

    DEBUG_OUT(i, ST7789_LOG_TAG, "lvgl_display_task start (gui_guider)");

    /**
     * Driver instantiation lives here (not in platform_io_register) because
     * the underlying bsp_*_inst() touches OSAL primitives that are only
     * valid post-kernel.  Reached through the wrapper API so the task does
     * not need to know which concrete driver is mounted; failures are
     * already logged by the adapter, and the existing fill_color / chip_id
     * checks below act as a second-line guard.
     */
    (void)display_drv_inst();
    (void)touch_drv_inst();

    /* 1. Display bring-up via wrapper. */
    display_drv_init();
    platform_err_t dret = display_fill_color(LV_TASK_BG_COLOR_BLACK);
    if (PLATFORM_OK != dret)
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG,
                  "display_fill_color failed = %d", (int)dret);
        for (;;)
        {
            osal_task_delay(1000U);
        }
    }

    /* 2. Touch bring-up via wrapper.  Display still works without touch,
     *    so a probe failure is logged but does not block the task. */
    touch_drv_init();
    UINT8_T chip_id = 0u;
    platform_err_t tret = touch_get_chip_id(&chip_id);
    BOOL_T touch_ok = (PLATFORM_OK == tret);
    if (touch_ok)
    {
        DEBUG_OUT(i, CST816T_LOG_TAG, "touch chip_id = 0x%02X",
                  (unsigned)chip_id);
    }
    else
    {
        DEBUG_OUT(e, CST816T_ERR_LOG_TAG,
                  "touch chip_id probe failed = %d", (int)tret);
    }

    /* 3. LVGL core + display port. */
    lv_init();
    lv_log_register_print_cb(lvgl_log_output_cb);
    if (!lv_port_disp_init())
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG, "lvgl disp port init failed");
        for (;;)
        {
            osal_task_delay(1000U);
        }
    }

    /* 4. LVGL pointer indev (skip if touch probe failed). */
    if (touch_ok)
    {
        if (!lv_port_indev_init())
        {
            DEBUG_OUT(e, CST816T_ERR_LOG_TAG, "lvgl indev port init failed");
        }
    }

    /* 5. Provision LVGL pointer-sprite assets on the external W25Q64 flash
     *    and load them into RAM mirrors before setup_ui() runs.  Bootstrap
     *    is idempotent: if the magic matches it just re-reads the data. */
    ext_flash_status_t boot_st = storage_assets_bootstrap();
    if (EXT_FLASH_OK != boot_st)
    {
        DEBUG_OUT(e, W25Q64_ERR_LOG_TAG,
                  "storage_assets_bootstrap failed st=%d", (int)boot_st);
        /* Fall through: setup_ui will still draw, but the *_ext needle
         * descriptors will reference uninitialised RAM. */
    }

    /* 5b. Register the external-flash line-streaming decoder.  Must run
     *     after lv_init() but before setup_ui(): once gui_guider creates
     *     the image widget, LVGL's first render walks the decoder list to
     *     identify the image source. */
    lv_port_extflash_init();

    /* 5c. Touch-panel calibration.  Loads saved coefficients from W25Q64
     *     if valid; otherwise drives the 9-point on-screen UI synchronously
     *     before the main UI is shown.  Skipped when the touch probe failed
     *     above (calibration requires a working panel). */
    if (touch_ok)
    {
        const calibration_status_t calib_st = touch_calibration_boot_apply();
        if (CALIBRATION_SUCCESS != calib_st)
        {
            DEBUG_OUT(w, TOUCH_CALIB_LOG_TAG,
                      "calib boot: skipped/failed st=%d — raw indev",
                      (int)calib_st);
        }
    }

    /* 6. Hand off to gui_guider's generated UI. */
    setup_ui(&guider_ui);
    DEBUG_OUT(i, ST7789_LOG_TAG, "lvgl_display_task: gui_guider UI loaded");

    /* 6b. Bind live heart-rate data to the under_up screen's BPM label.
     *     Runs in this (LVGL) thread via an lv_timer — LVGL is not
     *     thread-safe, so the EM7028 task must not touch widgets directly. */
    ui_hr_view_register(&guider_ui);
    ui_temp_humi_view_register(&guider_ui);

    lvgl_mem_report("ui-loaded");

    /* 7. LVGL service loop. */
    for (;;)
    {
        lv_timer_handler();
        lvgl_mem_monitor_poll();
        osal_task_delay(LV_TASK_TIMER_PERIOD_MS);
    }
}
//******************************* Functions *********************************//

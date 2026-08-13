/******************************************************************************
 * @file log_sink_rtt.c
 *
 * @par dependencies
 * - log_sink_rtt.h
 * - log_sink.h
 * - SEGGER_RTT.h
 *
 * @author Ethan-Hang
 *
 * @brief SEGGER RTT log transport adapter.
 *
 *        Implements the log_sink_t vtable over SEGGER RTT. All log data
 *        travels through physical RTT channel 0; SetTerminal() prefixes each
 *        fragment with a 2-byte escape sequence so J-Link RTT Viewer sorts it
 *        into the Terminal tab named by the channel argument. This preserves
 *        the exact sequence the elog port used before the abstraction.
 *
 * @version V1.0 2026-6-5
 *
 * @note 1 tab == 4 spaces!
 *****************************************************************************/

//******************************** Includes *********************************//
#include "board_types.h"
#include "log_sink_rtt.h"
#include "log_sink.h"
#include "SEGGER_RTT.h"
//******************************** Includes *********************************//

//****************************** Declarations *******************************//

static platform_err_t rtt_sink_init(log_sink_t *self);
static platform_err_t rtt_sink_write(log_sink_t *self,
                                     UINT8_t     channel,
                                     const char *data,
                                     SIZE_t      size);

//****************************** Declarations *******************************//

//******************************* Variables *********************************//

/*
 * The RTT sink vtable. File-scope static so its lifetime spans every log
 * call after registration. pf_deinit is NULL: RTT has no teardown.
 */
static log_sink_t s_rttSink =
{
    .user_data = NULL,
    .pf_init   = rtt_sink_init,
    .pf_write  = rtt_sink_write,
    .pf_deinit = NULL,
};

//******************************* Variables *********************************//

//******************************* Functions *********************************//

/**
 * @brief Bring SEGGER RTT up.
 *
 * @param[in] self : owning sink (unused; RTT keeps no per-instance state)
 *
 * @param[out] : none
 *
 * @return PLATFORM_OK (SEGGER_RTT_Init() cannot fail).
 *
 * @note _SEGGER_RTT lives in the RTT_RAM linker section (NOLOAD, pinned to
 *       0x2001E400) so Bootloader and APP can share one physical control
 *       block for continuous J-Link RTT Viewer logging. That section is
 *       outside _sbss.._ebss, so the startup code never zeroes it: buffer
 *       slots beyond index 0 (which SEGGER_RTT_Init()'s lazy _DoInit() sets
 *       up explicitly) start out holding whatever was left in SRAM at
 *       power-on. SEGGER_RTT_AllocUpBuffer()/AllocDownBuffer() treat a
 *       non-NULL pBuffer as "slot already taken", so leftover garbage in
 *       slots 1..N-1 makes every slot look occupied and allocation fails.
 *       Explicitly clear them here so SEGGER_SYSVIEW_Init()'s dynamic
 *       channel allocation (SEGGER_SYSVIEW_Conf() in main.c) sees them as
 *       free, as intended.
 * */
static platform_err_t rtt_sink_init(log_sink_t *self)
{
    unsigned i;

    (void)self;

    SEGGER_RTT_Init();

    for (i = 1u; i < SEGGER_RTT_MAX_NUM_UP_BUFFERS; i++)
    {
        SEGGER_RTT_ConfigUpBuffer(i, NULL, NULL, 0u, 0u);
    }
    for (i = 1u; i < SEGGER_RTT_MAX_NUM_DOWN_BUFFERS; i++)
    {
        SEGGER_RTT_ConfigDownBuffer(i, NULL, NULL, 0u, 0u);
    }

    return PLATFORM_OK;
}

/**
 * @brief Write one formatted log fragment to RTT, routed by channel.
 *
 *        SetTerminal(channel) -> Write(0, ...) -> SetTerminal(0) reproduces
 *        the former elog_port_output() behaviour byte for byte. RTT silently
 *        drops the fragment when its up-buffer is full, so there is no error
 *        to surface here.
 *
 * @param[in] self    : owning sink (unused)
 * @param[in] channel : RTT virtual terminal index (0-9)
 * @param[in] data    : formatted bytes to emit
 * @param[in] size    : number of bytes at data
 *
 * @param[out] : none
 *
 * @return PLATFORM_OK.
 *
 * */
static platform_err_t rtt_sink_write(log_sink_t *self,
                                     UINT8_t     channel,
                                     const char *data,
                                     SIZE_t      size)
{
    (void)self;

    SEGGER_RTT_SetTerminal((unsigned char)channel);
    SEGGER_RTT_Write(0, data, size);
    SEGGER_RTT_SetTerminal(0u);

    return PLATFORM_OK;
}

/**
 * @brief Register the SEGGER RTT sink as the active log transport.
 *
 * @param[in] : none
 *
 * @param[out] : none
 *
 * @return PLATFORM_OK on success, or the log_sink_register() error.
 *
 * */
platform_err_t log_sink_rtt_register(void)
{
    return log_sink_register(&s_rttSink);
}

//******************************* Functions *********************************//

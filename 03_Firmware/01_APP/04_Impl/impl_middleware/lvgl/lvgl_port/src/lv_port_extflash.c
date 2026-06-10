/******************************************************************************
 * @file lv_port_extflash.c
 *
 * @par dependencies
 * - lv_port_extflash.h
 * - service_storage_facade.h
 * - lvgl.h
 *
 * @author Ethan-Hang
 *
 * @brief Implementation of the W25Q64-backed LVGL image decoder.
 *
 *        Every Read_LvglData() round-trips through two task hops
 *        (storage_manager_task -> w25q64 handler), so per-call overhead
 *        dominates small transfers.  The decoder therefore minimises call
 *        count instead of byte count:
 *
 *          - Small images (<= LV_EXTFLASH_WHOLE_READ_MAX_BYTES) are read
 *            whole into an LVGL-pool buffer at open time and handed to
 *            LVGL via img_data.  Combined with LV_IMG_CACHE_DEF_SIZE > 0
 *            the pixels survive across redraws: icons cost one flash read
 *            for as long as they stay cached.
 *
 *          - Large images stream line-by-line, but through a multi-line
 *            prefetch buffer (LV_EXTFLASH_PREFETCH_LINES rows per flash
 *            read) so a 280-line background costs ~56 round-trips instead
 *            of 280.  The buffer lives as long as the decode session, so
 *            dirty-region redraws hitting the same rows are served from
 *            RAM.
 *
 *        Every pool allocation can fail gracefully: whole-read falls back
 *        to streaming, prefetch falls back to direct per-line reads.
 *
 * Processing flow (per LVGL render of an image whose data points at an
 * lv_extflash_meta_t):
 *
 *   info_cb       -> magic check, hand back width/height/cf
 *   open_cb       -> whole-read into img_data, or set up a streaming
 *                    session (meta + optional prefetch buffer)
 *   read_line_cb  -> serve from prefetch buffer, refilling per
 *                    LV_EXTFLASH_PREFETCH_LINES rows; direct read fallback
 *   close_cb      -> free session + buffers
 *
 * @version V1.0 2026-05-08
 * @version V2.0 2026-06-10
 * @upgrade 2.0: Whole-read small images + multi-line prefetch streaming;
 *               pairs with LV_IMG_CACHE_DEF_SIZE for cross-redraw reuse.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "lv_port_extflash.h"

#include "service_storage_facade.h"
#include "Debug.h"

#include "lvgl.h"

#include "board_types.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/**
 * Images up to this many bytes are read whole at open time (32x32 alpha
 * icons = 3072 B; the threshold covers everything up to that size, which
 * is 30+ of the 41 packed assets).
 **/
#define LV_EXTFLASH_WHOLE_READ_MAX_BYTES (3200U)

/** Rows fetched per flash read on the streaming path. */
#define LV_EXTFLASH_PREFETCH_LINES       (5U)

/**
 * Per-session decode state, allocated from the LVGL pool at open time.
 * `whole_buf` is set on the whole-read path; `prefetch_buf` on the
 * streaming path (NULL when the pool could not supply it -- direct
 * per-line reads then service the session).
 **/
typedef struct
{
    const lv_extflash_meta_t *meta;
    UINT8_t                  *whole_buf;
    UINT8_t                  *prefetch_buf;
    UINT32_t                  prefetch_lines;   /* buffer capacity (rows)  */
    UINT32_t                  first_line;       /* first cached row        */
    UINT32_t                  line_count;       /* cached rows, 0 = empty  */
} lv_extflash_session_t;
//******************************** Defines **********************************//

//******************************* Functions *********************************//
/**
 * @brief Try to recognise an `lv_img_dsc_t` whose `.data` carries an
 *        `lv_extflash_meta_t`.  Returns LV_RES_INV (not LV_RES_OK) for any
 *        descriptor that doesn't carry our magic so LVGL falls through to
 *        the next decoder in the chain.
 */
static lv_res_t lv_extflash_info_cb(lv_img_decoder_t *decoder,
                                    const void       *src,
                                    lv_img_header_t  *header)
{
    LV_UNUSED(decoder);

    if (LV_IMG_SRC_VARIABLE != lv_img_src_get_type(src))
    {
        return LV_RES_INV;
    }

    const lv_img_dsc_t *dsc = (const lv_img_dsc_t *)src;
    if (NULL == dsc->data)
    {
        return LV_RES_INV;
    }

    const lv_extflash_meta_t *meta = (const lv_extflash_meta_t *)dsc->data;
    if (LV_EXTFLASH_DECODER_MAGIC != meta->magic)
    {
        return LV_RES_INV;
    }

    header->w           = meta->width;
    header->h           = meta->height;
    header->cf          = dsc->header.cf;
    header->always_zero = 0;
    header->reserved    = 0;
    return LV_RES_OK;
}

/**
 * @brief Set up a decode session.
 *
 *        Small images: one blocking read into a pool buffer published via
 *        img_data (LVGL then never calls read_line_cb, and the image cache
 *        keeps the buffer across redraws).  Large images / pool pressure:
 *        img_data = NULL switches LVGL to per-line streaming, served by
 *        read_line_cb through an optional prefetch buffer.
 */
static lv_res_t lv_extflash_open_cb(lv_img_decoder_t      *decoder,
                                    lv_img_decoder_dsc_t  *dsc)
{
    LV_UNUSED(decoder);

    const lv_img_dsc_t       *img_dsc = (const lv_img_dsc_t *)dsc->src;
    const lv_extflash_meta_t *meta    =
        (const lv_extflash_meta_t *)img_dsc->data;
    if (LV_EXTFLASH_DECODER_MAGIC != meta->magic)
    {
        return LV_RES_INV;
    }

    lv_extflash_session_t *session =
        (lv_extflash_session_t *)lv_mem_alloc(sizeof(*session));
    if (NULL == session)
    {
        DEBUG_OUT(e, W25Q64_ERR_LOG_TAG, "extflash open: session alloc failed");
        return LV_RES_INV;
    }
    lv_memset_00(session, sizeof(*session));
    session->meta = meta;

    const UINT32_t bytes_per_line = (UINT32_t)meta->width * meta->px_size;
    const UINT32_t total_bytes    = bytes_per_line * meta->height;

    /* ---- Whole-read path for small images ------------------------------ */
    if (total_bytes <= LV_EXTFLASH_WHOLE_READ_MAX_BYTES)
    {
        session->whole_buf = (UINT8_t *)lv_mem_alloc(total_bytes);
        if (NULL != session->whole_buf)
        {
            ext_flash_status_t st = Read_LvglData(meta->ext_offset,
                                                  total_bytes,
                                                  session->whole_buf);
            if (EXT_FLASH_OK == st)
            {
                dsc->user_data = session;
                dsc->img_data  = session->whole_buf;
                return LV_RES_OK;
            }

            DEBUG_OUT(e, W25Q64_ERR_LOG_TAG,
                      "extflash whole read failed st=%d, streaming", (int)st);
            lv_mem_free(session->whole_buf);
            session->whole_buf = NULL;
        }
        /* Pool pressure or read failure: fall through to streaming. */
    }

    /* ---- Streaming path with best-effort prefetch ----------------------- */
    UINT32_t lines = LV_EXTFLASH_PREFETCH_LINES;
    if (lines > meta->height)
    {
        lines = meta->height;
    }
    if (lines > 1U)
    {
        session->prefetch_buf = (UINT8_t *)lv_mem_alloc(bytes_per_line * lines);
        if (NULL != session->prefetch_buf)
        {
            session->prefetch_lines = lines;
        }
        /* NULL prefetch_buf -> direct per-line reads; still functional. */
    }

    dsc->user_data = session;
    dsc->img_data  = NULL;
    return LV_RES_OK;
}

/**
 * @brief Service a per-line read.  All offsets here are LVGL-sub-region
 *        relative; storage_manager_task adds MEMORY_LVGL_START_ADDRESS
 *        when issuing the actual driver call.
 */
static lv_res_t lv_extflash_read_line_cb(lv_img_decoder_t      *decoder,
                                         lv_img_decoder_dsc_t  *dsc,
                                         lv_coord_t             x,
                                         lv_coord_t             y,
                                         lv_coord_t             len,
                                         UINT8_t               *buf)
{
    LV_UNUSED(decoder);

    const lv_extflash_session_t *constSession =
        (const lv_extflash_session_t *)dsc->user_data;
    lv_extflash_session_t *session = (lv_extflash_session_t *)constSession;
    if ((NULL == session) || (NULL == session->meta))
    {
        return LV_RES_INV;
    }

    const lv_extflash_meta_t *meta           = session->meta;
    const UINT32_t            bytes_per_line =
        (UINT32_t)meta->width * meta->px_size;
    const UINT32_t            seg_offset     = (UINT32_t)x * meta->px_size;
    const UINT32_t            seg_bytes      = (UINT32_t)len * meta->px_size;

    /* ---- Direct read fallback (no prefetch buffer available) ----------- */
    if (NULL == session->prefetch_buf)
    {
        UINT32_t row_offset = (UINT32_t)y * bytes_per_line + seg_offset;

        ext_flash_status_t st = Read_LvglData(meta->ext_offset + row_offset,
                                              seg_bytes, buf);
        if (EXT_FLASH_OK != st)
        {
            DEBUG_OUT(e, W25Q64_ERR_LOG_TAG,
                      "extflash read_line failed st=%d y=%d", (int)st, (int)y);
            return LV_RES_INV;
        }
        return LV_RES_OK;
    }

    /* ---- Prefetch-buffered path ----------------------------------------- */
    const UINT32_t row = (UINT32_t)y;
    if ((0U == session->line_count)          ||
        (row <  session->first_line)         ||
        (row >= session->first_line + session->line_count))
    {
        UINT32_t fill_lines = session->prefetch_lines;
        if ((row + fill_lines) > meta->height)
        {
            fill_lines = meta->height - row;
        }

        ext_flash_status_t st = Read_LvglData(
            meta->ext_offset + row * bytes_per_line,
            fill_lines * bytes_per_line,
            session->prefetch_buf);
        if (EXT_FLASH_OK != st)
        {
            DEBUG_OUT(e, W25Q64_ERR_LOG_TAG,
                      "extflash prefetch failed st=%d y=%d", (int)st, (int)y);
            session->line_count = 0U;
            return LV_RES_INV;
        }

        session->first_line = row;
        session->line_count = fill_lines;
    }

    lv_memcpy(buf,
              session->prefetch_buf +
                  (row - session->first_line) * bytes_per_line + seg_offset,
              seg_bytes);
    return LV_RES_OK;
}

/**
 * @brief Tear down a decode session: release the pool buffers.  With the
 *        image cache enabled this runs on cache eviction, not per draw,
 *        so cached entries keep their pixels between redraws.
 */
static void lv_extflash_close_cb(lv_img_decoder_t     *decoder,
                                 lv_img_decoder_dsc_t *dsc)
{
    LV_UNUSED(decoder);

    lv_extflash_session_t *session = (lv_extflash_session_t *)dsc->user_data;
    if (NULL == session)
    {
        return;
    }

    if (NULL != session->whole_buf)
    {
        lv_mem_free(session->whole_buf);
    }
    if (NULL != session->prefetch_buf)
    {
        lv_mem_free(session->prefetch_buf);
    }
    lv_mem_free(session);
    dsc->user_data = NULL;
    dsc->img_data  = NULL;
}

void lv_port_extflash_init(void)
{
    lv_img_decoder_t *d = lv_img_decoder_create();
    if (NULL == d)
    {
        DEBUG_OUT(e, W25Q64_ERR_LOG_TAG, "lv_img_decoder_create failed");
        return;
    }
    lv_img_decoder_set_info_cb     (d, lv_extflash_info_cb);
    lv_img_decoder_set_open_cb     (d, lv_extflash_open_cb);
    lv_img_decoder_set_read_line_cb(d, lv_extflash_read_line_cb);
    lv_img_decoder_set_close_cb    (d, lv_extflash_close_cb);
}
//******************************* Functions *********************************//

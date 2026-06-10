/******************************************************************************
 * @file lv_port_extfont.c
 *
 * @par dependencies
 * - lv_port_extfont.h
 * - cfg_storage.h
 * - service_storage_facade.h
 * - lv_font_fmt_txt.h
 *
 * @author Ethan-Hang
 *
 * @brief W25Q64-backed LVGL font bitmap callbacks.
 *
 * @version V1.0 2026-6-3
 *
 * @note 1 tab == 4 spaces!
 *****************************************************************************/

//******************************** Includes *********************************//
#include "lv_port_extfont.h"

#include "cfg_storage.h"
#include "service_storage_facade.h"
#include "Debug.h"

#include "lvgl.h"
#include "src/font/lv_font_fmt_txt.h"

#include "board_types.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/**
 * Small-glyph cache: every glyph render otherwise costs one blocking
 * Read_LvglData round-trip (two task hops), and UI text redraws the same
 * glyphs every frame.  Cached bitmaps live in a static FIFO arena; the
 * entry table is invalidated when the arena wraps over older bytes.
 *
 * Sized against the 4 KB freed from configTOTAL_HEAP_SIZE: glyphs above
 * EXTFONT_CACHE_MAX_GLYPH bytes (the 82 px clock digits, several KB each
 * but redrawn only once a second) bypass the cache through the shared
 * scratch buffer.
 **/
#define EXTFONT_CACHE_ARENA_SIZE   (4096U)
#define EXTFONT_CACHE_ENTRIES      (48U)
#define EXTFONT_CACHE_MAX_GLYPH    (512U)

typedef struct
{
    const lv_font_t *font;      /* NULL = slot free / invalidated        */
    UINT32_t         glyphId;
    UINT16_t         offset;    /* into s_au8CacheArena                  */
    UINT16_t         size;
} extfont_cache_entry_t;
//******************************** Defines **********************************//

//******************************* Variables *********************************//
static UINT8_t s_au8GlyphBuf[CFG_LVGL_FONT_GLYPH_BUFFER_SIZE];

/* Single-task access only: the callbacks run in the LVGL draw context. */
static UINT8_t               s_au8CacheArena[EXTFONT_CACHE_ARENA_SIZE];
static extfont_cache_entry_t s_atCacheEntry[EXTFONT_CACHE_ENTRIES];
static UINT32_t              s_u32ArenaHead;
static UINT32_t              s_u32EntryHead;
//******************************* Variables *********************************//

//******************************* Functions *********************************//
/**
 * @brief Locate a value in a sorted UINT16_t list.
 *
 * @param[in] : list Sorted UINT16_t list.
 * @param[in] : listLen Number of entries in list.
 * @param[in] : value Value to find.
 * @param[out] : indexOut Matching index when the function returns true.
 *
 * @return true when found, otherwise false.
 * */
static BOOL extfont_find_u16(const UINT16_t *list,
                             UINT16_t       listLen,
                             UINT16_t       value,
                             UINT16_t      *indexOut)
{
    UINT16_t index;

    if ((NULL == list) || (NULL == indexOut))
    {
        return false;
    }

    for (index = 0U; index < listLen; index++)
    {
        if (value == list[index])
        {
            *indexOut = index;
            return true;
        }
    }

    return false;
}

/**
 * @brief Resolve a Unicode code point to an LVGL glyph descriptor index.
 *
 * @param[in] : font Pointer to the LVGL font descriptor.
 * @param[in] : letter Unicode code point requested by LVGL.
 *
 * @return LVGL glyph ID, or 0 when not found.
 * */
static UINT32_t extfont_get_glyph_id(const lv_font_t *font, UINT32_t letter)
{
    const lv_font_fmt_txt_dsc_t *fdsc;
    UINT16_t cmapIndex;

    if ((NULL == font) || (NULL == font->dsc) || ('\0' == letter))
    {
        return 0U;
    }

    if ('\t' == letter)
    {
        letter = ' ';
    }

    fdsc = (const lv_font_fmt_txt_dsc_t *)font->dsc;
    for (cmapIndex = 0U; cmapIndex < fdsc->cmap_num; cmapIndex++)
    {
        const lv_font_fmt_txt_cmap_t *cmap = &fdsc->cmaps[cmapIndex];
        UINT32_t rcp;
        UINT32_t glyphId = 0U;

        if (letter < cmap->range_start)
        {
            continue;
        }

        rcp = letter - cmap->range_start;
        if (rcp >= cmap->range_length)
        {
            continue;
        }

        if (LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY == cmap->type)
        {
            glyphId = (UINT32_t)cmap->glyph_id_start + rcp;
        }
        else if (LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL == cmap->type)
        {
            const UINT8_t *glyphOfs = (const UINT8_t *)cmap->glyph_id_ofs_list;

            if (NULL != glyphOfs)
            {
                glyphId = (UINT32_t)cmap->glyph_id_start + glyphOfs[rcp];
            }
        }
        else
        {
            UINT16_t listIndex = 0U;

            if ((rcp <= UINT16_MAX) &&
                extfont_find_u16(cmap->unicode_list,
                                 cmap->list_length,
                                 (UINT16_t)rcp,
                                 &listIndex))
            {
                if (LV_FONT_FMT_TXT_CMAP_SPARSE_TINY == cmap->type)
                {
                    glyphId = (UINT32_t)cmap->glyph_id_start + listIndex;
                }
                else if (LV_FONT_FMT_TXT_CMAP_SPARSE_FULL == cmap->type)
                {
                    const UINT16_t *glyphOfs =
                        (const UINT16_t *)cmap->glyph_id_ofs_list;

                    if (NULL != glyphOfs)
                    {
                        glyphId = (UINT32_t)cmap->glyph_id_start +
                                  glyphOfs[listIndex];
                    }
                }
            }
        }

        if ((NULL != fdsc->cache) && (0U != glyphId))
        {
            fdsc->cache->last_letter = letter;
            fdsc->cache->last_glyph_id = glyphId;
        }

        return glyphId;
    }

    return 0U;
}

/**
 * @brief Compute one uncompressed LVGL glyph bitmap byte count.
 *
 * @param[in] : gdsc Pointer to LVGL glyph descriptor.
 * @param[in] : bpp Font bits per pixel.
 *
 * @return Number of bytes needed for this glyph bitmap.
 * */
static UINT32_t extfont_get_glyph_size(
    const lv_font_fmt_txt_glyph_dsc_t *gdsc,
    UINT8_t                            bpp)
{
    UINT32_t bitCount;

    if ((NULL == gdsc) || (0U == gdsc->box_w) || (0U == gdsc->box_h))
    {
        return 0U;
    }

    bitCount = (UINT32_t)gdsc->box_w * gdsc->box_h * bpp;
    return (bitCount + 7U) / 8U;
}

/**
 * @brief Look up one glyph bitmap in the small-glyph cache.
 *
 * @param[in] : font Font owning the glyph (part of the cache key).
 * @param[in] : glyphId Resolved LVGL glyph ID (part of the cache key).
 *
 * @return Pointer into the cache arena on a hit, otherwise NULL.
 * */
static const UINT8_t *extfont_cache_find(const lv_font_t *font,
                                         UINT32_t         glyphId)
{
    UINT32_t index;

    for (index = 0U; index < EXTFONT_CACHE_ENTRIES; index++)
    {
        const extfont_cache_entry_t *entry = &s_atCacheEntry[index];

        if ((font == entry->font) && (glyphId == entry->glyphId))
        {
            return &s_au8CacheArena[entry->offset];
        }
    }
    return NULL;
}

/**
 * @brief Reserve arena space for one glyph and register its cache entry.
 *
 *        FIFO arena: wraps to the start when the tail cannot fit the
 *        request, then invalidates every entry overlapping the reserved
 *        byte range.  The entry table itself recycles slots round-robin.
 *
 * @param[in] : font Font owning the glyph.
 * @param[in] : glyphId Resolved LVGL glyph ID.
 * @param[in] : size Glyph bitmap byte count (<= EXTFONT_CACHE_MAX_GLYPH).
 *
 * @return Writable arena pointer for the caller to fill.
 * */
static UINT8_t *extfont_cache_insert(const lv_font_t *font,
                                     UINT32_t         glyphId,
                                     UINT32_t         size)
{
    UINT32_t               index;
    extfont_cache_entry_t *slot;

    if ((s_u32ArenaHead + size) > EXTFONT_CACHE_ARENA_SIZE)
    {
        s_u32ArenaHead = 0U;
    }

    /**
     * Invalidate entries whose bytes are about to be overwritten.  Any
     * pointer returned for them earlier has already been consumed: LVGL
     * finishes blending a glyph before requesting the next one.
     **/
    for (index = 0U; index < EXTFONT_CACHE_ENTRIES; index++)
    {
        extfont_cache_entry_t *entry = &s_atCacheEntry[index];

        if ((NULL != entry->font)                                  &&
            ((UINT32_t)entry->offset < (s_u32ArenaHead + size))    &&
            (((UINT32_t)entry->offset + entry->size) > s_u32ArenaHead))
        {
            entry->font = NULL;
        }
    }

    slot = &s_atCacheEntry[s_u32EntryHead];
    s_u32EntryHead = (s_u32EntryHead + 1U) % EXTFONT_CACHE_ENTRIES;

    slot->font    = font;
    slot->glyphId = glyphId;
    slot->offset  = (UINT16_t)s_u32ArenaHead;
    slot->size    = (UINT16_t)size;

    s_u32ArenaHead += size;
    return &s_au8CacheArena[slot->offset];
}

/**
 * @brief Common W25Q64 glyph bitmap reader.
 *
 * @param[in] : font Pointer to the LVGL font descriptor.
 * @param[in] : letter Unicode code point requested by LVGL.
 * @param[in] : bitmapOffset Font bitmap base offset within LVGL partition.
 * @param[in] : bitmapSize Total external bitmap payload size.
 *
 * @return Pointer to a transient glyph bitmap buffer, or NULL on failure.
 * */
static const UINT8_t *extfont_get_bitmap(const lv_font_t *font,
                                         UINT32_t         letter,
                                         UINT32_t         bitmapOffset,
                                         UINT32_t         bitmapSize)
{
    const lv_font_fmt_txt_dsc_t       *fdsc;
    const lv_font_fmt_txt_glyph_dsc_t *gdsc;
    UINT32_t                           glyphId;
    UINT32_t                           glyphSize;
    ext_flash_status_t                 st;

    if ((NULL == font) || (NULL == font->dsc))
    {
        return NULL;
    }

    fdsc = (const lv_font_fmt_txt_dsc_t *)font->dsc;
    if (LV_FONT_FMT_TXT_PLAIN != fdsc->bitmap_format)
    {
        DEBUG_OUT(e, W25Q64_ERR_LOG_TAG, "extfont compressed font unsupported");
        return NULL;
    }

    glyphId = extfont_get_glyph_id(font, letter);
    if (0U == glyphId)
    {
        return NULL;
    }

    gdsc = &fdsc->glyph_dsc[glyphId];
    glyphSize = extfont_get_glyph_size(gdsc, (UINT8_t)fdsc->bpp);
    if (0U == glyphSize)
    {
        return NULL;
    }

    if ((glyphSize > CFG_LVGL_FONT_GLYPH_BUFFER_SIZE) ||
        (gdsc->bitmap_index > bitmapSize) ||
        (glyphSize > (bitmapSize - gdsc->bitmap_index)))
    {
        DEBUG_OUT(e, W25Q64_ERR_LOG_TAG,
                  "extfont glyph out of range idx=%lu size=%lu",
                  (unsigned long)gdsc->bitmap_index,
                  (unsigned long)glyphSize);
        return NULL;
    }

    /**
     * Small glyphs go through the cache: text redraws request the same
     * glyphs every frame, and each miss costs a two-task-hop blocking
     * read.  Large glyphs (clock digits) bypass via the scratch buffer.
     **/
    if (glyphSize <= EXTFONT_CACHE_MAX_GLYPH)
    {
        const UINT8_t *cached = extfont_cache_find(font, glyphId);
        if (NULL != cached)
        {
            return cached;
        }

        UINT8_t *slotBuf = extfont_cache_insert(font, glyphId, glyphSize);

        st = Read_LvglData(bitmapOffset + gdsc->bitmap_index,
                           glyphSize,
                           slotBuf);
        if (EXT_FLASH_OK != st)
        {
            DEBUG_OUT(e, W25Q64_ERR_LOG_TAG,
                      "extfont read failed st=%d", (int)st);
            /* Drop the just-inserted entry so the bad bytes never hit. */
            s_atCacheEntry[(s_u32EntryHead + EXTFONT_CACHE_ENTRIES - 1U) %
                           EXTFONT_CACHE_ENTRIES].font = NULL;
            return NULL;
        }
        return slotBuf;
    }

    st = Read_LvglData(bitmapOffset + gdsc->bitmap_index,
                       glyphSize,
                       s_au8GlyphBuf);
    if (EXT_FLASH_OK != st)
    {
        DEBUG_OUT(e, W25Q64_ERR_LOG_TAG,
                  "extfont read failed st=%d", (int)st);
        return NULL;
    }

    return s_au8GlyphBuf;
}

#define LV_EXTFONT_DEFINE(fontName, offsetMacro, sizeMacro)                  \
    const UINT8_t *lv_port_extfont_get_bitmap_##fontName(                   \
        const lv_font_t *font,                                               \
        UINT32_t         letter)                                             \
    {                                                                        \
        return extfont_get_bitmap(font, letter, offsetMacro, sizeMacro);     \
    }

LV_EXTFONT_DEFINE(lv_font_interttf_24,
                  CFG_LVGL_FONT_INTERTTF_24_BITMAP_OFFSET,
                  CFG_LVGL_FONT_INTERTTF_24_BITMAP_SIZE)
LV_EXTFONT_DEFINE(lv_font_interttf_10,
                  CFG_LVGL_FONT_INTERTTF_10_BITMAP_OFFSET,
                  CFG_LVGL_FONT_INTERTTF_10_BITMAP_SIZE)
LV_EXTFONT_DEFINE(lv_font_interttf_82,
                  CFG_LVGL_FONT_INTERTTF_82_BITMAP_OFFSET,
                  CFG_LVGL_FONT_INTERTTF_82_BITMAP_SIZE)
LV_EXTFONT_DEFINE(lv_font_alimama_16,
                  CFG_LVGL_FONT_ALIMAMA_16_BITMAP_OFFSET,
                  CFG_LVGL_FONT_ALIMAMA_16_BITMAP_SIZE)
LV_EXTFONT_DEFINE(lv_font_alimama_36,
                  CFG_LVGL_FONT_ALIMAMA_36_BITMAP_OFFSET,
                  CFG_LVGL_FONT_ALIMAMA_36_BITMAP_SIZE)
LV_EXTFONT_DEFINE(lv_font_digitaldreamfatnarrow_36,
                  CFG_LVGL_FONT_DIGITALDREAMFATNARROW_36_BITMAP_OFFSET,
                  CFG_LVGL_FONT_DIGITALDREAMFATNARROW_36_BITMAP_SIZE)
LV_EXTFONT_DEFINE(lv_font_alimama_12,
                  CFG_LVGL_FONT_ALIMAMA_12_BITMAP_OFFSET,
                  CFG_LVGL_FONT_ALIMAMA_12_BITMAP_SIZE)
LV_EXTFONT_DEFINE(lv_font_alimama_10,
                  CFG_LVGL_FONT_ALIMAMA_10_BITMAP_OFFSET,
                  CFG_LVGL_FONT_ALIMAMA_10_BITMAP_SIZE)
LV_EXTFONT_DEFINE(lv_font_interttf_16,
                  CFG_LVGL_FONT_INTERTTF_16_BITMAP_OFFSET,
                  CFG_LVGL_FONT_INTERTTF_16_BITMAP_SIZE)
//******************************* Functions *********************************//

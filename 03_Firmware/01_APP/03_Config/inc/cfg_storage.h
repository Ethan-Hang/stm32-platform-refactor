/******************************************************************************
 * @file cfg_storage.h
 *
 * @par dependencies
 * - stdint.h
 *
 * @author Ethan-Hang
 *
 * @brief Project-level configuration for the external flash storage layer.
 *
 *        Holds:
 *          - User-tunable runtime sizes (LVGL transient buffer cap)
 *          - Layout of the LVGL sub-region of W25Q64 (asset offsets / sizes)
 *          - Bootstrap magic used to detect a freshly-erased chip
 *
 *        Addresses here are byte offsets RELATIVE to the start of the LVGL
 *        sub-region (MEMORY_LVGL_START_ADDRESS); callers add the absolute
 *        sub-region base when invoking driver-level APIs.
 *
 * @version V1.0 2026-05-08
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __CFG_STORAGE_H__
#define __CFG_STORAGE_H__

//******************************** Includes *********************************//
#include <stdint.h>
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/**
 * @brief Default cap for the transient LVGL data buffer used by callers that
 *        want a one-shot blocking read.  Sized to fit the largest pointer
 *        sprite (1050 B for fen/miao 70x5 alpha).  Large background images
 *        (240x240) must NOT use this path -- they need a custom LVGL image
 *        decoder with line-level streaming.
 */
#ifndef CFG_LVGL_DATA_MAX_SIZE
#define CFG_LVGL_DATA_MAX_SIZE       (1100U)
#endif

/**
 * @brief Erase granularity of the W25Q64 (4-KiB sector).  Used by the
 *        bootstrap path to size sector-aligned writes.
 */
#define CFG_W25Q64_SECTOR_SIZE       (4096U)

/**
 * @brief Magic written to the first 4 bytes of the LVGL sub-region after a
 *        successful bootstrap.  Mismatch on boot triggers a re-flash of the
 *        LVGL pointer-asset images.
 */
#define CFG_LVGL_ASSET_MAGIC         (0xA55A5AA5UL)

/* ── LVGL sub-region asset layout (offsets within the LVGL sub-region) ────
 *
 *   0x000000  magic         (4  B)
 *   0x000100  fen_70x5      (1050 B)   -- 256 B page-aligned
 *   0x000600  miao_70x5     (1050 B)
 *   0x000B00  time_40x5     ( 600 B)
 *   ...       (free)                                                       */
#define CFG_LVGL_ASSET_MAGIC_OFFSET  (0x000000UL)
#define CFG_LVGL_ASSET_MAGIC_SIZE    (4U)

#define CFG_LVGL_ASSET_FEN_OFFSET    (0x000100UL)
#define CFG_LVGL_ASSET_FEN_SIZE      (1050U)

#define CFG_LVGL_ASSET_MIAO_OFFSET   (0x000600UL)
#define CFG_LVGL_ASSET_MIAO_SIZE     (1050U)

#define CFG_LVGL_ASSET_TIME_OFFSET   (0x000B00UL)
#define CFG_LVGL_ASSET_TIME_SIZE     (600U)

/**
 * @brief One-past-last byte used by LVGL assets.  Bootstrap erases sectors
 *        up to here on a magic mismatch.
 */
#define CFG_LVGL_ASSET_FOOTPRINT     (CFG_LVGL_ASSET_TIME_OFFSET + \
                                       CFG_LVGL_ASSET_TIME_SIZE)
//******************************** Defines **********************************//

#endif /* __CFG_STORAGE_H__ */

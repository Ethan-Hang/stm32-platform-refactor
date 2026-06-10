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
 *          - User-tunable runtime sizes (LVGL transient buffer cap,
 *            LVGL font glyph scratch buffer)
 *          - Layout of the LVGL sub-region of W25Q64 (asset offsets / sizes)
 *          - Bootstrap magic used to detect a freshly-erased chip
 *
 *        Addresses here are byte offsets RELATIVE to the start of the LVGL
 *        sub-region (MEMORY_LVGL_START_ADDRESS); callers add the absolute
 *        sub-region base when invoking driver-level APIs.
 *
 * @version V1.2 2026-06-03  Move refreshed LVGL images/fonts to W25Q64.
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
 *        sprite (1920 B for fen 80x8 alpha).  Large background images must
 *        NOT use this path -- they need a custom LVGL image decoder with
 *        line-level streaming.
 */
#ifndef CFG_LVGL_DATA_MAX_SIZE
#define CFG_LVGL_DATA_MAX_SIZE       (2048U)
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
 *
 *        Bumped on every asset-layout change so chips holding an older map
 *        are forced to re-provision: 0xA55A5AA5 → 0xA55A5AA6 (new SquareLine
 *        UI assets) → 0xA55A5AA7 (3 watchdight 60×60 moved off .rodata onto
 *        W25Q64) → 0xA55A5AA8 (refreshed UI images + font bitmaps moved
 *        to W25Q64) → 0xA55A5AA9 (full GUI-Guider watch UI: 41 images +
 *        9 fonts, every glyph bitmap and image served from W25Q64) →
 *        0xA55A5AAA (LV_COLOR_16_SWAP=1: packed image bytes are now in
 *        panel byte order; layout unchanged, content re-ordered).
 */
#define CFG_LVGL_ASSET_MAGIC         (0xA55A5AAAUL)

/**
 * @brief Scratch buffer used by the W25Q64-backed LVGL font callback.
 *        Largest glyph bitmap in the current font set measures 4275 B
 *        (interttf_82, 4 bpp; scanned across all 9 packed fonts), so
 *        4608 fits it with margin while freeing 3.5 KB of RAM for the
 *        double-buffered LVGL flush.  The callback rejects (and logs)
 *        any glyph larger than this instead of overflowing.
 */
#ifndef CFG_LVGL_FONT_GLYPH_BUFFER_SIZE
#define CFG_LVGL_FONT_GLYPH_BUFFER_SIZE (4608U)
#endif

/* ── LVGL sub-region asset layout (offsets within the LVGL sub-region) ────
 *
 *   0x000000  magic                (4 B)
 *   0x001000  41 UI images         (one per 4 KB-sector-aligned slot)
 *   0x096000  9 custom font glyph-bitmap payloads (sector-aligned)
 *   0x110000  end of pack          (~1.06 MB of the 3 MB partition)
 *
 * Every image and font glyph bitmap lives ONLY on W25Q64; firmware carries
 * no pixel data.  `make flash-assets` provisions the chip; on boot the
 * firmware checks the magic and the two needle sprites (fen / time) are
 * mirrored into RAM for cheap rotation redraws.  Everything else is served
 * by the lv_port_extflash line-streaming decoder or the lv_port_extfont
 * glyph callback.
 *
 * Geometry macros are written as literals (not aliases) so the
 * pack_assets.py macro parser, which only resolves literal `#define`
 * pairs, can synthesise *_SIZE from W*H*PX.
 */
#define CFG_LVGL_ASSET_MAGIC_OFFSET     (0x000000UL)
#define CFG_LVGL_ASSET_MAGIC_SIZE       (4U)

/* fen minute-hand sprite (RAM-mirrored): 80x8 RGB565 + 8-bit alpha = 3 B/pixel = 1920 B. */
#define CFG_LVGL_ASSET_FEN_OFFSET     (0x001000UL)
#define CFG_LVGL_ASSET_FEN_W          (80U)
#define CFG_LVGL_ASSET_FEN_H          (8U)
#define CFG_LVGL_ASSET_FEN_PX_SIZE     (3U)
#define CFG_LVGL_ASSET_FEN_SIZE       (CFG_LVGL_ASSET_FEN_W * \
                                         CFG_LVGL_ASSET_FEN_H * \
                                         CFG_LVGL_ASSET_FEN_PX_SIZE)

/* time hour-hand sprite (RAM-mirrored): 50x8 RGB565 + 8-bit alpha = 3 B/pixel = 1200 B. */
#define CFG_LVGL_ASSET_TIME_OFFSET    (0x002000UL)
#define CFG_LVGL_ASSET_TIME_W         (50U)
#define CFG_LVGL_ASSET_TIME_H         (8U)
#define CFG_LVGL_ASSET_TIME_PX_SIZE    (3U)
#define CFG_LVGL_ASSET_TIME_SIZE      (CFG_LVGL_ASSET_TIME_W * \
                                         CFG_LVGL_ASSET_TIME_H * \
                                         CFG_LVGL_ASSET_TIME_PX_SIZE)

/* Clock_1/Clock_2 full-screen background: 240x280 RGB565 + 8-bit alpha = 3 B/pixel = 201600 B. */
#define CFG_LVGL_ASSET_MDLBG_OFFSET   (0x003000UL)
#define CFG_LVGL_ASSET_MDLBG_W        (240U)
#define CFG_LVGL_ASSET_MDLBG_H        (280U)
#define CFG_LVGL_ASSET_MDLBG_PX_SIZE   (3U)
#define CFG_LVGL_ASSET_MDLBG_SIZE     (CFG_LVGL_ASSET_MDLBG_W * \
                                         CFG_LVGL_ASSET_MDLBG_H * \
                                         CFG_LVGL_ASSET_MDLBG_PX_SIZE)

/* Clock_3 analog dial: 200x200 RGB565 + 8-bit alpha = 3 B/pixel = 120000 B. */
#define CFG_LVGL_ASSET_BIAOPAN1_OFFSET (0x035000UL)
#define CFG_LVGL_ASSET_BIAOPAN1_W      (200U)
#define CFG_LVGL_ASSET_BIAOPAN1_H      (200U)
#define CFG_LVGL_ASSET_BIAOPAN1_PX_SIZE (3U)
#define CFG_LVGL_ASSET_BIAOPAN1_SIZE   (CFG_LVGL_ASSET_BIAOPAN1_W * \
                                         CFG_LVGL_ASSET_BIAOPAN1_H * \
                                         CFG_LVGL_ASSET_BIAOPAN1_PX_SIZE)

/* Clock_3 step icon frame 1: 60x60 RGB565 + 8-bit alpha = 3 B/pixel = 10800 B. */
#define CFG_LVGL_ASSET_WATCHDIGHT1_OFFSET (0x053000UL)
#define CFG_LVGL_ASSET_WATCHDIGHT1_W      (60U)
#define CFG_LVGL_ASSET_WATCHDIGHT1_H      (60U)
#define CFG_LVGL_ASSET_WATCHDIGHT1_PX_SIZE (3U)
#define CFG_LVGL_ASSET_WATCHDIGHT1_SIZE   (CFG_LVGL_ASSET_WATCHDIGHT1_W * \
                                         CFG_LVGL_ASSET_WATCHDIGHT1_H * \
                                         CFG_LVGL_ASSET_WATCHDIGHT1_PX_SIZE)

/* Clock_3 step icon frame 2: 60x60 RGB565 + 8-bit alpha = 3 B/pixel = 10800 B. */
#define CFG_LVGL_ASSET_WATCHDIGHT2_OFFSET (0x056000UL)
#define CFG_LVGL_ASSET_WATCHDIGHT2_W      (60U)
#define CFG_LVGL_ASSET_WATCHDIGHT2_H      (60U)
#define CFG_LVGL_ASSET_WATCHDIGHT2_PX_SIZE (3U)
#define CFG_LVGL_ASSET_WATCHDIGHT2_SIZE   (CFG_LVGL_ASSET_WATCHDIGHT2_W * \
                                         CFG_LVGL_ASSET_WATCHDIGHT2_H * \
                                         CFG_LVGL_ASSET_WATCHDIGHT2_PX_SIZE)

/* Clock_3 step icon frame 3: 60x60 RGB565 + 8-bit alpha = 3 B/pixel = 10800 B. */
#define CFG_LVGL_ASSET_WATCHDIGHT3_OFFSET (0x059000UL)
#define CFG_LVGL_ASSET_WATCHDIGHT3_W      (60U)
#define CFG_LVGL_ASSET_WATCHDIGHT3_H      (60U)
#define CFG_LVGL_ASSET_WATCHDIGHT3_PX_SIZE (3U)
#define CFG_LVGL_ASSET_WATCHDIGHT3_SIZE   (CFG_LVGL_ASSET_WATCHDIGHT3_W * \
                                         CFG_LVGL_ASSET_WATCHDIGHT3_H * \
                                         CFG_LVGL_ASSET_WATCHDIGHT3_PX_SIZE)

/* degree-celsius icon: 10x10 RGB565 + 8-bit alpha = 3 B/pixel = 300 B. */
#define CFG_LVGL_ASSET_SHESHIDU_OFFSET (0x05C000UL)
#define CFG_LVGL_ASSET_SHESHIDU_W      (10U)
#define CFG_LVGL_ASSET_SHESHIDU_H      (10U)
#define CFG_LVGL_ASSET_SHESHIDU_PX_SIZE (3U)
#define CFG_LVGL_ASSET_SHESHIDU_SIZE   (CFG_LVGL_ASSET_SHESHIDU_W * \
                                         CFG_LVGL_ASSET_SHESHIDU_H * \
                                         CFG_LVGL_ASSET_SHESHIDU_PX_SIZE)

/* weather icon 16: 16x16 RGB565 + 8-bit alpha = 3 B/pixel = 768 B. */
#define CFG_LVGL_ASSET_WATHER16X16_OFFSET (0x05D000UL)
#define CFG_LVGL_ASSET_WATHER16X16_W      (16U)
#define CFG_LVGL_ASSET_WATHER16X16_H      (16U)
#define CFG_LVGL_ASSET_WATHER16X16_PX_SIZE (3U)
#define CFG_LVGL_ASSET_WATHER16X16_SIZE   (CFG_LVGL_ASSET_WATHER16X16_W * \
                                         CFG_LVGL_ASSET_WATHER16X16_H * \
                                         CFG_LVGL_ASSET_WATHER16X16_PX_SIZE)

/* heart icon 16: 16x16 RGB565 + 8-bit alpha = 3 B/pixel = 768 B. */
#define CFG_LVGL_ASSET_HEART16X16_OFFSET (0x05E000UL)
#define CFG_LVGL_ASSET_HEART16X16_W      (16U)
#define CFG_LVGL_ASSET_HEART16X16_H      (16U)
#define CFG_LVGL_ASSET_HEART16X16_PX_SIZE (3U)
#define CFG_LVGL_ASSET_HEART16X16_SIZE   (CFG_LVGL_ASSET_HEART16X16_W * \
                                         CFG_LVGL_ASSET_HEART16X16_H * \
                                         CFG_LVGL_ASSET_HEART16X16_PX_SIZE)

/* calorie icon 16: 16x16 RGB565 + 8-bit alpha = 3 B/pixel = 768 B. */
#define CFG_LVGL_ASSET_KLL16X16_OFFSET (0x05F000UL)
#define CFG_LVGL_ASSET_KLL16X16_W      (16U)
#define CFG_LVGL_ASSET_KLL16X16_H      (16U)
#define CFG_LVGL_ASSET_KLL16X16_PX_SIZE (3U)
#define CFG_LVGL_ASSET_KLL16X16_SIZE   (CFG_LVGL_ASSET_KLL16X16_W * \
                                         CFG_LVGL_ASSET_KLL16X16_H * \
                                         CFG_LVGL_ASSET_KLL16X16_PX_SIZE)

/* step icon 16: 16x16 RGB565 + 8-bit alpha = 3 B/pixel = 768 B. */
#define CFG_LVGL_ASSET_FOOT16X16_OFFSET (0x060000UL)
#define CFG_LVGL_ASSET_FOOT16X16_W      (16U)
#define CFG_LVGL_ASSET_FOOT16X16_H      (16U)
#define CFG_LVGL_ASSET_FOOT16X16_PX_SIZE (3U)
#define CFG_LVGL_ASSET_FOOT16X16_SIZE   (CFG_LVGL_ASSET_FOOT16X16_W * \
                                         CFG_LVGL_ASSET_FOOT16X16_H * \
                                         CFG_LVGL_ASSET_FOOT16X16_PX_SIZE)

/* bluetooth icon: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_BT32_OFFSET    (0x061000UL)
#define CFG_LVGL_ASSET_BT32_W         (32U)
#define CFG_LVGL_ASSET_BT32_H         (32U)
#define CFG_LVGL_ASSET_BT32_PX_SIZE    (3U)
#define CFG_LVGL_ASSET_BT32_SIZE      (CFG_LVGL_ASSET_BT32_W * \
                                         CFG_LVGL_ASSET_BT32_H * \
                                         CFG_LVGL_ASSET_BT32_PX_SIZE)

/* mianti icon: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_MIANTI_0_OFFSET (0x062000UL)
#define CFG_LVGL_ASSET_MIANTI_0_W      (32U)
#define CFG_LVGL_ASSET_MIANTI_0_H      (32U)
#define CFG_LVGL_ASSET_MIANTI_0_PX_SIZE (3U)
#define CFG_LVGL_ASSET_MIANTI_0_SIZE   (CFG_LVGL_ASSET_MIANTI_0_W * \
                                         CFG_LVGL_ASSET_MIANTI_0_H * \
                                         CFG_LVGL_ASSET_MIANTI_0_PX_SIZE)

/* vibration icon: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_ZHENGDONG_0_OFFSET (0x063000UL)
#define CFG_LVGL_ASSET_ZHENGDONG_0_W      (32U)
#define CFG_LVGL_ASSET_ZHENGDONG_0_H      (32U)
#define CFG_LVGL_ASSET_ZHENGDONG_0_PX_SIZE (3U)
#define CFG_LVGL_ASSET_ZHENGDONG_0_SIZE   (CFG_LVGL_ASSET_ZHENGDONG_0_W * \
                                         CFG_LVGL_ASSET_ZHENGDONG_0_H * \
                                         CFG_LVGL_ASSET_ZHENGDONG_0_PX_SIZE)

/* compass icon: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_COPESSS_OFFSET (0x064000UL)
#define CFG_LVGL_ASSET_COPESSS_W      (32U)
#define CFG_LVGL_ASSET_COPESSS_H      (32U)
#define CFG_LVGL_ASSET_COPESSS_PX_SIZE (3U)
#define CFG_LVGL_ASSET_COPESSS_SIZE   (CFG_LVGL_ASSET_COPESSS_W * \
                                         CFG_LVGL_ASSET_COPESSS_H * \
                                         CFG_LVGL_ASSET_COPESSS_PX_SIZE)

/* weather icon 32: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_WEATER32X32_OFFSET (0x065000UL)
#define CFG_LVGL_ASSET_WEATER32X32_W      (32U)
#define CFG_LVGL_ASSET_WEATER32X32_H      (32U)
#define CFG_LVGL_ASSET_WEATER32X32_PX_SIZE (3U)
#define CFG_LVGL_ASSET_WEATER32X32_SIZE   (CFG_LVGL_ASSET_WEATER32X32_W * \
                                         CFG_LVGL_ASSET_WEATER32X32_H * \
                                         CFG_LVGL_ASSET_WEATER32X32_PX_SIZE)

/* ellipse widget: 40x40 RGB565 + 8-bit alpha = 3 B/pixel = 4800 B. */
#define CFG_LVGL_ASSET_ELLIPSE_OFFSET (0x066000UL)
#define CFG_LVGL_ASSET_ELLIPSE_W      (40U)
#define CFG_LVGL_ASSET_ELLIPSE_H      (40U)
#define CFG_LVGL_ASSET_ELLIPSE_PX_SIZE (3U)
#define CFG_LVGL_ASSET_ELLIPSE_SIZE   (CFG_LVGL_ASSET_ELLIPSE_W * \
                                         CFG_LVGL_ASSET_ELLIPSE_H * \
                                         CFG_LVGL_ASSET_ELLIPSE_PX_SIZE)

/* small hour hand: 16x8 RGB565 + 8-bit alpha = 3 B/pixel = 384 B. */
#define CFG_LVGL_ASSET_STIME_OFFSET   (0x068000UL)
#define CFG_LVGL_ASSET_STIME_W        (16U)
#define CFG_LVGL_ASSET_STIME_H        (8U)
#define CFG_LVGL_ASSET_STIME_PX_SIZE   (3U)
#define CFG_LVGL_ASSET_STIME_SIZE     (CFG_LVGL_ASSET_STIME_W * \
                                         CFG_LVGL_ASSET_STIME_H * \
                                         CFG_LVGL_ASSET_STIME_PX_SIZE)

/* small minute hand: 21x6 RGB565 + 8-bit alpha = 3 B/pixel = 378 B. */
#define CFG_LVGL_ASSET_SFEN_OFFSET    (0x069000UL)
#define CFG_LVGL_ASSET_SFEN_W         (21U)
#define CFG_LVGL_ASSET_SFEN_H         (6U)
#define CFG_LVGL_ASSET_SFEN_PX_SIZE    (3U)
#define CFG_LVGL_ASSET_SFEN_SIZE      (CFG_LVGL_ASSET_SFEN_W * \
                                         CFG_LVGL_ASSET_SFEN_H * \
                                         CFG_LVGL_ASSET_SFEN_PX_SIZE)

/* battery icon: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_POWER_HIGHT_OFFSET (0x06A000UL)
#define CFG_LVGL_ASSET_POWER_HIGHT_W      (32U)
#define CFG_LVGL_ASSET_POWER_HIGHT_H      (32U)
#define CFG_LVGL_ASSET_POWER_HIGHT_PX_SIZE (3U)
#define CFG_LVGL_ASSET_POWER_HIGHT_SIZE   (CFG_LVGL_ASSET_POWER_HIGHT_W * \
                                         CFG_LVGL_ASSET_POWER_HIGHT_H * \
                                         CFG_LVGL_ASSET_POWER_HIGHT_PX_SIZE)

/* location icon: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_LOCATION_OFFSET (0x06B000UL)
#define CFG_LVGL_ASSET_LOCATION_W      (32U)
#define CFG_LVGL_ASSET_LOCATION_H      (32U)
#define CFG_LVGL_ASSET_LOCATION_PX_SIZE (3U)
#define CFG_LVGL_ASSET_LOCATION_SIZE   (CFG_LVGL_ASSET_LOCATION_W * \
                                         CFG_LVGL_ASSET_LOCATION_H * \
                                         CFG_LVGL_ASSET_LOCATION_PX_SIZE)

/* map region icon: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_TAIWAN_OFFSET  (0x06C000UL)
#define CFG_LVGL_ASSET_TAIWAN_W       (32U)
#define CFG_LVGL_ASSET_TAIWAN_H       (32U)
#define CFG_LVGL_ASSET_TAIWAN_PX_SIZE  (3U)
#define CFG_LVGL_ASSET_TAIWAN_SIZE    (CFG_LVGL_ASSET_TAIWAN_W * \
                                         CFG_LVGL_ASSET_TAIWAN_H * \
                                         CFG_LVGL_ASSET_TAIWAN_PX_SIZE)

/* nfc icon (list): 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_NFC_OFFSET     (0x06D000UL)
#define CFG_LVGL_ASSET_NFC_W          (32U)
#define CFG_LVGL_ASSET_NFC_H          (32U)
#define CFG_LVGL_ASSET_NFC_PX_SIZE     (3U)
#define CFG_LVGL_ASSET_NFC_SIZE       (CFG_LVGL_ASSET_NFC_W * \
                                         CFG_LVGL_ASSET_NFC_H * \
                                         CFG_LVGL_ASSET_NFC_PX_SIZE)

/* brightness icon: 47x47 RGB565 + 8-bit alpha = 3 B/pixel = 6627 B. */
#define CFG_LVGL_ASSET_LIANGDU_OFFSET (0x06E000UL)
#define CFG_LVGL_ASSET_LIANGDU_W      (47U)
#define CFG_LVGL_ASSET_LIANGDU_H      (47U)
#define CFG_LVGL_ASSET_LIANGDU_PX_SIZE (3U)
#define CFG_LVGL_ASSET_LIANGDU_SIZE   (CFG_LVGL_ASSET_LIANGDU_W * \
                                         CFG_LVGL_ASSET_LIANGDU_H * \
                                         CFG_LVGL_ASSET_LIANGDU_PX_SIZE)

/* compass background: 100x100 RGB565 + 8-bit alpha = 3 B/pixel = 30000 B. */
#define CFG_LVGL_ASSET_ZNZBG_OFFSET   (0x070000UL)
#define CFG_LVGL_ASSET_ZNZBG_W        (100U)
#define CFG_LVGL_ASSET_ZNZBG_H        (100U)
#define CFG_LVGL_ASSET_ZNZBG_PX_SIZE   (3U)
#define CFG_LVGL_ASSET_ZNZBG_SIZE     (CFG_LVGL_ASSET_ZNZBG_W * \
                                         CFG_LVGL_ASSET_ZNZBG_H * \
                                         CFG_LVGL_ASSET_ZNZBG_PX_SIZE)

/* compass arrow: 50x40 RGB565 + 8-bit alpha = 3 B/pixel = 6000 B. */
#define CFG_LVGL_ASSET_ARW_OFFSET     (0x078000UL)
#define CFG_LVGL_ASSET_ARW_W          (50U)
#define CFG_LVGL_ASSET_ARW_H          (40U)
#define CFG_LVGL_ASSET_ARW_PX_SIZE     (3U)
#define CFG_LVGL_ASSET_ARW_SIZE       (CFG_LVGL_ASSET_ARW_W * \
                                         CFG_LVGL_ASSET_ARW_H * \
                                         CFG_LVGL_ASSET_ARW_PX_SIZE)

/* compass needle: 50x50 RGB565 + 8-bit alpha = 3 B/pixel = 7500 B. */
#define CFG_LVGL_ASSET_ZNZ_OFFSET     (0x07A000UL)
#define CFG_LVGL_ASSET_ZNZ_W          (50U)
#define CFG_LVGL_ASSET_ZNZ_H          (50U)
#define CFG_LVGL_ASSET_ZNZ_PX_SIZE     (3U)
#define CFG_LVGL_ASSET_ZNZ_SIZE       (CFG_LVGL_ASSET_ZNZ_W * \
                                         CFG_LVGL_ASSET_ZNZ_H * \
                                         CFG_LVGL_ASSET_ZNZ_PX_SIZE)

/* heart icon 32: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_HEART32X32_OFFSET (0x07C000UL)
#define CFG_LVGL_ASSET_HEART32X32_W      (32U)
#define CFG_LVGL_ASSET_HEART32X32_H      (32U)
#define CFG_LVGL_ASSET_HEART32X32_PX_SIZE (3U)
#define CFG_LVGL_ASSET_HEART32X32_SIZE   (CFG_LVGL_ASSET_HEART32X32_W * \
                                         CFG_LVGL_ASSET_HEART32X32_H * \
                                         CFG_LVGL_ASSET_HEART32X32_PX_SIZE)

/* body-temperature icon: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_TIWEN_OFFSET   (0x07D000UL)
#define CFG_LVGL_ASSET_TIWEN_W        (32U)
#define CFG_LVGL_ASSET_TIWEN_H        (32U)
#define CFG_LVGL_ASSET_TIWEN_PX_SIZE   (3U)
#define CFG_LVGL_ASSET_TIWEN_SIZE     (CFG_LVGL_ASSET_TIWEN_W * \
                                         CFG_LVGL_ASSET_TIWEN_H * \
                                         CFG_LVGL_ASSET_TIWEN_PX_SIZE)

/* climb icon: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_PA_OFFSET      (0x07E000UL)
#define CFG_LVGL_ASSET_PA_W           (32U)
#define CFG_LVGL_ASSET_PA_H           (32U)
#define CFG_LVGL_ASSET_PA_PX_SIZE      (3U)
#define CFG_LVGL_ASSET_PA_SIZE        (CFG_LVGL_ASSET_PA_W * \
                                         CFG_LVGL_ASSET_PA_H * \
                                         CFG_LVGL_ASSET_PA_PX_SIZE)

/* location icon 32: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_LOCATION32X32_OFFSET (0x07F000UL)
#define CFG_LVGL_ASSET_LOCATION32X32_W      (32U)
#define CFG_LVGL_ASSET_LOCATION32X32_H      (32U)
#define CFG_LVGL_ASSET_LOCATION32X32_PX_SIZE (3U)
#define CFG_LVGL_ASSET_LOCATION32X32_SIZE   (CFG_LVGL_ASSET_LOCATION32X32_W * \
                                         CFG_LVGL_ASSET_LOCATION32X32_H * \
                                         CFG_LVGL_ASSET_LOCATION32X32_PX_SIZE)

/* Heart screen big heart: 93x85 RGB565 + 8-bit alpha = 3 B/pixel = 23715 B. */
#define CFG_LVGL_ASSET_BIGHEART_OFFSET (0x080000UL)
#define CFG_LVGL_ASSET_BIGHEART_W      (93U)
#define CFG_LVGL_ASSET_BIGHEART_H      (85U)
#define CFG_LVGL_ASSET_BIGHEART_PX_SIZE (3U)
#define CFG_LVGL_ASSET_BIGHEART_SIZE   (CFG_LVGL_ASSET_BIGHEART_W * \
                                         CFG_LVGL_ASSET_BIGHEART_H * \
                                         CFG_LVGL_ASSET_BIGHEART_PX_SIZE)

/* NFC screen icon: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_NFC32X32_OFFSET (0x086000UL)
#define CFG_LVGL_ASSET_NFC32X32_W      (32U)
#define CFG_LVGL_ASSET_NFC32X32_H      (32U)
#define CFG_LVGL_ASSET_NFC32X32_PX_SIZE (3U)
#define CFG_LVGL_ASSET_NFC32X32_SIZE   (CFG_LVGL_ASSET_NFC32X32_W * \
                                         CFG_LVGL_ASSET_NFC32X32_H * \
                                         CFG_LVGL_ASSET_NFC32X32_PX_SIZE)

/* Error screen icon: 48x48 RGB565 + 8-bit alpha = 3 B/pixel = 6912 B. */
#define CFG_LVGL_ASSET_ERROR48_OFFSET (0x087000UL)
#define CFG_LVGL_ASSET_ERROR48_W      (48U)
#define CFG_LVGL_ASSET_ERROR48_H      (48U)
#define CFG_LVGL_ASSET_ERROR48_PX_SIZE (3U)
#define CFG_LVGL_ASSET_ERROR48_SIZE   (CFG_LVGL_ASSET_ERROR48_W * \
                                         CFG_LVGL_ASSET_ERROR48_H * \
                                         CFG_LVGL_ASSET_ERROR48_PX_SIZE)

/* heart icon 37x32: 37x32 RGB565 + 8-bit alpha = 3 B/pixel = 3552 B. */
#define CFG_LVGL_ASSET_HEART37X32_OFFSET (0x089000UL)
#define CFG_LVGL_ASSET_HEART37X32_W      (37U)
#define CFG_LVGL_ASSET_HEART37X32_H      (32U)
#define CFG_LVGL_ASSET_HEART37X32_PX_SIZE (3U)
#define CFG_LVGL_ASSET_HEART37X32_SIZE   (CFG_LVGL_ASSET_HEART37X32_W * \
                                         CFG_LVGL_ASSET_HEART37X32_H * \
                                         CFG_LVGL_ASSET_HEART37X32_PX_SIZE)

/* location icon 20: 20x20 RGB565 + 8-bit alpha = 3 B/pixel = 1200 B. */
#define CFG_LVGL_ASSET_LOCATION20X20_OFFSET (0x08A000UL)
#define CFG_LVGL_ASSET_LOCATION20X20_W      (20U)
#define CFG_LVGL_ASSET_LOCATION20X20_H      (20U)
#define CFG_LVGL_ASSET_LOCATION20X20_PX_SIZE (3U)
#define CFG_LVGL_ASSET_LOCATION20X20_SIZE   (CFG_LVGL_ASSET_LOCATION20X20_W * \
                                         CFG_LVGL_ASSET_LOCATION20X20_H * \
                                         CFG_LVGL_ASSET_LOCATION20X20_PX_SIZE)

/* boot/under_up logo: 100x100 RGB565 + 8-bit alpha = 3 B/pixel = 30000 B. */
#define CFG_LVGL_ASSET_LOGO100_OFFSET (0x08B000UL)
#define CFG_LVGL_ASSET_LOGO100_W      (100U)
#define CFG_LVGL_ASSET_LOGO100_H      (100U)
#define CFG_LVGL_ASSET_LOGO100_PX_SIZE (3U)
#define CFG_LVGL_ASSET_LOGO100_SIZE   (CFG_LVGL_ASSET_LOGO100_W * \
                                         CFG_LVGL_ASSET_LOGO100_H * \
                                         CFG_LVGL_ASSET_LOGO100_PX_SIZE)

/* QR-code icon: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_QRCODE32_OFFSET (0x093000UL)
#define CFG_LVGL_ASSET_QRCODE32_W      (32U)
#define CFG_LVGL_ASSET_QRCODE32_H      (32U)
#define CFG_LVGL_ASSET_QRCODE32_PX_SIZE (3U)
#define CFG_LVGL_ASSET_QRCODE32_SIZE   (CFG_LVGL_ASSET_QRCODE32_W * \
                                         CFG_LVGL_ASSET_QRCODE32_H * \
                                         CFG_LVGL_ASSET_QRCODE32_PX_SIZE)

/* settings icon: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_SET32_OFFSET   (0x094000UL)
#define CFG_LVGL_ASSET_SET32_W        (32U)
#define CFG_LVGL_ASSET_SET32_H        (32U)
#define CFG_LVGL_ASSET_SET32_PX_SIZE   (3U)
#define CFG_LVGL_ASSET_SET32_SIZE     (CFG_LVGL_ASSET_SET32_W * \
                                         CFG_LVGL_ASSET_SET32_H * \
                                         CFG_LVGL_ASSET_SET32_PX_SIZE)

/* system-update icon: 32x32 RGB565 + 8-bit alpha = 3 B/pixel = 3072 B. */
#define CFG_LVGL_ASSET_SYTEAM32_OFFSET (0x095000UL)
#define CFG_LVGL_ASSET_SYTEAM32_W      (32U)
#define CFG_LVGL_ASSET_SYTEAM32_H      (32U)
#define CFG_LVGL_ASSET_SYTEAM32_PX_SIZE (3U)
#define CFG_LVGL_ASSET_SYTEAM32_SIZE   (CFG_LVGL_ASSET_SYTEAM32_W * \
                                         CFG_LVGL_ASSET_SYTEAM32_H * \
                                         CFG_LVGL_ASSET_SYTEAM32_PX_SIZE)

/* Custom font glyph-bitmap payloads (sizes measured by Tools/pack_assets.py). */
#define CFG_LVGL_FONT_INTERTTF_24_BITMAP_OFFSET (0x096000UL)
#define CFG_LVGL_FONT_INTERTTF_24_BITMAP_SIZE   (25753U)

#define CFG_LVGL_FONT_INTERTTF_10_BITMAP_OFFSET (0x09D000UL)
#define CFG_LVGL_FONT_INTERTTF_10_BITMAP_SIZE   (5051U)

#define CFG_LVGL_FONT_INTERTTF_82_BITMAP_OFFSET (0x09F000UL)
#define CFG_LVGL_FONT_INTERTTF_82_BITMAP_SIZE   (288357U)

#define CFG_LVGL_FONT_ALIMAMA_16_BITMAP_OFFSET (0x0E6000UL)
#define CFG_LVGL_FONT_ALIMAMA_16_BITMAP_SIZE   (17256U)

#define CFG_LVGL_FONT_ALIMAMA_36_BITMAP_OFFSET (0x0EB000UL)
#define CFG_LVGL_FONT_ALIMAMA_36_BITMAP_SIZE   (53612U)

#define CFG_LVGL_FONT_DIGITALDREAMFATNARROW_36_BITMAP_OFFSET (0x0F9000UL)
#define CFG_LVGL_FONT_DIGITALDREAMFATNARROW_36_BITMAP_SIZE   (57756U)

#define CFG_LVGL_FONT_ALIMAMA_12_BITMAP_OFFSET (0x108000UL)
#define CFG_LVGL_FONT_ALIMAMA_12_BITMAP_SIZE   (8745U)

#define CFG_LVGL_FONT_ALIMAMA_10_BITMAP_OFFSET (0x10B000UL)
#define CFG_LVGL_FONT_ALIMAMA_10_BITMAP_SIZE   (5152U)

#define CFG_LVGL_FONT_INTERTTF_16_BITMAP_OFFSET (0x10D000UL)
#define CFG_LVGL_FONT_INTERTTF_16_BITMAP_SIZE   (11603U)

/**
 * @brief One-past-last byte used by LVGL assets.  Bootstrap erases sectors
 *        up to here on a magic mismatch.
 */
#define CFG_LVGL_ASSET_FOOTPRINT        (CFG_LVGL_FONT_INTERTTF_16_BITMAP_OFFSET + \
                                         CFG_LVGL_FONT_INTERTTF_16_BITMAP_SIZE)
//******************************** Defines **********************************//

#endif /* __CFG_STORAGE_H__ */

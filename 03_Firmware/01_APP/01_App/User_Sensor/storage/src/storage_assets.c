/******************************************************************************
 * @file storage_assets.c
 *
 * @par dependencies
 * - service_storage_facade.h
 * - cfg_storage.h
 * - lvgl.h
 *
 * @author Ethan-Hang
 *
 * @brief LVGL assets that live on the external W25Q64 flash.
 *
 *        Two classes of asset:
 *
 *          1. Needle sprites (fen 80x8, time 50x8).  Small enough to
 *             mirror into MCU RAM at boot so the analog-clock rotation
 *             redraws stay cheap.  The mirror is filled by reading from
 *             the external flash, which is provisioned by
 *             Tools/pack_assets.py.
 *
 *          2. Everything else (39 images, from 16x8 icons up to the
 *             240x280 MDLBG background).  Kept out of firmware .rodata
 *             and served line-by-line straight off W25Q64 by the
 *             lv_port_extflash decoder.  The lv_img_dsc_t .data pointer
 *             is overloaded to carry an lv_extflash_meta_t the decoder
 *             recognises by its magic field.  data_size = 0 tells LVGL
 *             it must call read_line_cb.
 *
 *        The full external asset pack, including backgrounds, icons and
 *        font bitmaps, must be written by `make flash-assets`.
 *
 * @version V1.2 2026-06-10  Full GUI-Guider watch UI (16 screens).
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "service_storage_facade.h"
#include "cfg_storage.h"
#include "lv_port_extflash.h"

#include "lvgl.h"
#include "Debug.h"

#include "platform_type.h"
//******************************** Includes *********************************//

//******************************* Variables *********************************//
/* RAM mirrors filled by Read_LvglData() during bootstrap. */
static UINT8_T s_au8_fen_ram [CFG_LVGL_ASSET_FEN_SIZE];
static UINT8_T s_au8_time_ram[CFG_LVGL_ASSET_TIME_SIZE];

/**
 * Externally-visible LVGL descriptors that point at the RAM mirrors.
 * Exposed via gui_guider.h's LV_IMG_DECLARE so setup_scr_Clock_*.c can
 * reference them without including this translation unit's header.
 */
const lv_img_dsc_t _fen_alpha_80x8_ext = {
    .header.cf          = LV_IMG_CF_TRUE_COLOR_ALPHA,
    .header.always_zero = 0,
    .header.reserved    = 0,
    .header.w           = CFG_LVGL_ASSET_FEN_W,
    .header.h           = CFG_LVGL_ASSET_FEN_H,
    .data_size          = CFG_LVGL_ASSET_FEN_SIZE,
    .data               = s_au8_fen_ram,
};

const lv_img_dsc_t _time_alpha_50x8_ext = {
    .header.cf          = LV_IMG_CF_TRUE_COLOR_ALPHA,
    .header.always_zero = 0,
    .header.reserved    = 0,
    .header.w           = CFG_LVGL_ASSET_TIME_W,
    .header.h           = CFG_LVGL_ASSET_TIME_H,
    .data_size          = CFG_LVGL_ASSET_TIME_SIZE,
    .data               = s_au8_time_ram,
};

#define DEFINE_EXTFLASH_IMAGE(symbol, cfgPrefix)                         \
    static const lv_extflash_meta_t s##symbol##_meta = {                 \
        .magic      = LV_EXTFLASH_DECODER_MAGIC,                         \
        .ext_offset = cfgPrefix##_OFFSET,                                \
        .width      = cfgPrefix##_W,                                     \
        .height     = cfgPrefix##_H,                                     \
        .px_size    = cfgPrefix##_PX_SIZE,                               \
    };                                                                   \
                                                                         \
    const lv_img_dsc_t symbol##_ext = {                                  \
        .header.cf          = LV_IMG_CF_TRUE_COLOR_ALPHA,                 \
        .header.always_zero = 0,                                         \
        .header.reserved    = 0,                                         \
        .header.w           = cfgPrefix##_W,                             \
        .header.h           = cfgPrefix##_H,                             \
        .data_size          = 0,                                         \
        .data               = (const UINT8_T *)&s##symbol##_meta,        \
    }

/* All streamed images, from the 240x280 MDLBG background down to 16x8
 * icons.  Streaming trades a ~1 ms/line first-draw cost for keeping every
 * pixel out of .rodata; dirty-region redraws only re-stream covered lines. */
DEFINE_EXTFLASH_IMAGE(_MDLBG_alpha_240x280, CFG_LVGL_ASSET_MDLBG);
DEFINE_EXTFLASH_IMAGE(_biaopan1_200x200, CFG_LVGL_ASSET_BIAOPAN1);
DEFINE_EXTFLASH_IMAGE(_watchdight1_alpha_60x60, CFG_LVGL_ASSET_WATCHDIGHT1);
DEFINE_EXTFLASH_IMAGE(_watchdight2_alpha_60x60, CFG_LVGL_ASSET_WATCHDIGHT2);
DEFINE_EXTFLASH_IMAGE(_watchdight3_alpha_60x60, CFG_LVGL_ASSET_WATCHDIGHT3);
DEFINE_EXTFLASH_IMAGE(_sheshidu_alpha_10x10, CFG_LVGL_ASSET_SHESHIDU);
DEFINE_EXTFLASH_IMAGE(_wather16x16_alpha_16x16, CFG_LVGL_ASSET_WATHER16X16);
DEFINE_EXTFLASH_IMAGE(_heart16x16_alpha_16x16, CFG_LVGL_ASSET_HEART16X16);
DEFINE_EXTFLASH_IMAGE(_KLL16x16_alpha_16x16, CFG_LVGL_ASSET_KLL16X16);
DEFINE_EXTFLASH_IMAGE(_foot16x16_alpha_16x16, CFG_LVGL_ASSET_FOOT16X16);
DEFINE_EXTFLASH_IMAGE(_BT32_alpha_32x32, CFG_LVGL_ASSET_BT32);
DEFINE_EXTFLASH_IMAGE(_mianti_0_alpha_32x32, CFG_LVGL_ASSET_MIANTI_0);
DEFINE_EXTFLASH_IMAGE(_zhengdong_0_alpha_32x32, CFG_LVGL_ASSET_ZHENGDONG_0);
DEFINE_EXTFLASH_IMAGE(_copesss_alpha_32x32, CFG_LVGL_ASSET_COPESSS);
DEFINE_EXTFLASH_IMAGE(_weater32x32_alpha_32x32, CFG_LVGL_ASSET_WEATER32X32);
DEFINE_EXTFLASH_IMAGE(_Ellipse_alpha_40x40, CFG_LVGL_ASSET_ELLIPSE);
DEFINE_EXTFLASH_IMAGE(_Stime_alpha_16x8, CFG_LVGL_ASSET_STIME);
DEFINE_EXTFLASH_IMAGE(_Sfen_alpha_21x6, CFG_LVGL_ASSET_SFEN);
DEFINE_EXTFLASH_IMAGE(_power_hight_alpha_32x32, CFG_LVGL_ASSET_POWER_HIGHT);
DEFINE_EXTFLASH_IMAGE(_location_alpha_32x32, CFG_LVGL_ASSET_LOCATION);
DEFINE_EXTFLASH_IMAGE(_taiwan_alpha_32x32, CFG_LVGL_ASSET_TAIWAN);
DEFINE_EXTFLASH_IMAGE(_nfc_alpha_32x32, CFG_LVGL_ASSET_NFC);
DEFINE_EXTFLASH_IMAGE(_liangdu_47x47, CFG_LVGL_ASSET_LIANGDU);
DEFINE_EXTFLASH_IMAGE(_ZNZBG_alpha_100x100, CFG_LVGL_ASSET_ZNZBG);
DEFINE_EXTFLASH_IMAGE(_arw_alpha_50x40, CFG_LVGL_ASSET_ARW);
DEFINE_EXTFLASH_IMAGE(_ZNZ_alpha_50x50, CFG_LVGL_ASSET_ZNZ);
DEFINE_EXTFLASH_IMAGE(_heart32x32_alpha_32x32, CFG_LVGL_ASSET_HEART32X32);
DEFINE_EXTFLASH_IMAGE(_tiwen_alpha_32x32, CFG_LVGL_ASSET_TIWEN);
DEFINE_EXTFLASH_IMAGE(_pa_alpha_32x32, CFG_LVGL_ASSET_PA);
DEFINE_EXTFLASH_IMAGE(_location32x32_alpha_32x32,
                      CFG_LVGL_ASSET_LOCATION32X32);
DEFINE_EXTFLASH_IMAGE(_BIGHeart_alpha_93x85, CFG_LVGL_ASSET_BIGHEART);
DEFINE_EXTFLASH_IMAGE(_NFC32x32_alpha_32x32, CFG_LVGL_ASSET_NFC32X32);
DEFINE_EXTFLASH_IMAGE(_error_alpha_48x48, CFG_LVGL_ASSET_ERROR48);
DEFINE_EXTFLASH_IMAGE(_heart32x32_alpha_37x32, CFG_LVGL_ASSET_HEART37X32);
DEFINE_EXTFLASH_IMAGE(_location20x20_alpha_20x20,
                      CFG_LVGL_ASSET_LOCATION20X20);
DEFINE_EXTFLASH_IMAGE(_logo_100x100, CFG_LVGL_ASSET_LOGO100);
DEFINE_EXTFLASH_IMAGE(_qrcode32x32_alpha_32x32, CFG_LVGL_ASSET_QRCODE32);
DEFINE_EXTFLASH_IMAGE(_set32x32_alpha_32x32, CFG_LVGL_ASSET_SET32);
DEFINE_EXTFLASH_IMAGE(_syteam32x32_alpha_32x32, CFG_LVGL_ASSET_SYTEAM32);
//******************************* Variables *********************************//

//******************************* Functions *********************************//
/**
 * @brief Sequentially read the two needle sprites from external flash
 *        into their RAM mirrors.
 */
static ext_flash_status_t lvgl_assets_load_to_ram(void)
{
    ext_flash_status_t st;

    st = Read_LvglData(CFG_LVGL_ASSET_FEN_OFFSET,
                       CFG_LVGL_ASSET_FEN_SIZE,
                       s_au8_fen_ram);
    if (EXT_FLASH_OK != st)
    {
        DEBUG_OUT(e, W25Q64_ERR_LOG_TAG, "load fen failed st=%d", (int)st);
        return st;
    }

    st = Read_LvglData(CFG_LVGL_ASSET_TIME_OFFSET,
                       CFG_LVGL_ASSET_TIME_SIZE,
                       s_au8_time_ram);
    if (EXT_FLASH_OK != st)
    {
        DEBUG_OUT(e, W25Q64_ERR_LOG_TAG, "load time failed st=%d", (int)st);
        return st;
    }

    return EXT_FLASH_OK;
}

ext_flash_status_t storage_assets_bootstrap(void)
{
    UINT32_T            magic = 0U;
    ext_flash_status_t  st;

    DEBUG_OUT(i, W25Q64_LOG_TAG, "assets bootstrap: probing magic");

    st = Read_LvglData(CFG_LVGL_ASSET_MAGIC_OFFSET,
                       CFG_LVGL_ASSET_MAGIC_SIZE,
                       (UINT8_T *)&magic);
    if (EXT_FLASH_OK != st)
    {
        DEBUG_OUT(e, W25Q64_ERR_LOG_TAG,
                  "assets bootstrap: read magic failed st=%d", (int)st);
        return st;
    }

    if (CFG_LVGL_ASSET_MAGIC == magic)
    {
        DEBUG_OUT(i, W25Q64_LOG_TAG,
                  "assets bootstrap: magic ok (0x%08X), skipping write",
                  (unsigned int)magic);
    }
    else
    {
        DEBUG_OUT(e, W25Q64_ERR_LOG_TAG,
                  "assets bootstrap: magic mismatch (0x%08X), flash assets.bin",
                  (unsigned int)magic);
        return EXT_FLASH_ERRORRESOURCE;
    }

    st = lvgl_assets_load_to_ram();
    if (EXT_FLASH_OK != st)
    {
        return st;
    }

    DEBUG_OUT(i, W25Q64_LOG_TAG, "assets bootstrap: ram mirrors filled");
    return EXT_FLASH_OK;
}
//******************************* Functions *********************************//

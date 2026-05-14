/******************************************************************************
 * @file ota_flag.h
 *
 * @par dependencies
 * - stdint.h
 * - flash.h
 *
 * @author Ethan-Hang
 *
 * @brief Internal-Flash backed OTA flag store. Replaces the legacy AT24Cxx
 *        EEPROM as the cross-reset signalling channel between APP and
 *        Bootloader. Lives in the reserved Sector 2 region (16 KB at
 *        0x08008000), uses a 16-byte struct at the base of the sector;
 *        the remaining ~16 KB is left blank for future growth.
 *
 * @note  STRUCT LAYOUT MUST STAY BYTE-COMPATIBLE WITH
 *        01_APP/03_Config/inc/cfg_ota.h. Both projects compile independently
 *        and have no shared include path; sync changes by hand.
 *
 * @version V1.0 2026-05-14
 *
 * @note 1 tab == 4 spaces!
 *****************************************************************************/
#ifndef __OTA_FLAG_H
#define __OTA_FLAG_H

#include <stdint.h>

#include "flash.h"

/* Magic value at struct head — distinguishes a real flag from blank flash
   (which reads back as 0xFFFFFFFF after erase). */
#define OTA_FLAG_MAGIC 0xA55A5AA5UL

/**
 * @brief OTA flag struct, stored at FlagAddress (0x08008000).
 *
 * Layout is frozen at 16 bytes — keep new fields aligned and append-only so
 * old/new bootloader-vs-APP combinations stay forward-compatible while we
 * iterate.
 */
struct ota_flag_t {
    uint32_t magic;             /* OTA_FLAG_MAGIC; mismatch ⇒ treat as EE_INIT_NO_APP */
    uint32_t state;             /* EE_OTA_* widened to 32 bits */
    uint32_t image_size;        /* staged image bytes in W25Q64 BLOCK_1 (encrypted size) */
    uint32_t current_app_size;  /* size of the APP currently programmed in internal Flash */
};

/**
* @brief Read OTA flag struct from internal Flash 0x08008000.
*
* @param[out] out : receives the four 32-bit fields on success.
*
* @return  0 on success (magic valid)
*         -1 if magic mismatched (caller should treat as uninitialised)
* */
int8_t ota_flag_read(struct ota_flag_t *out);

/**
* @brief Erase Sector 2 and program a fresh OTA flag struct.
*
* The full 16 KB sector is erased even though only 16 bytes of payload are
* written — STM32F4 has no smaller granularity. Single-thread / no-OS
* bootloader context only; APP-side mirror in upgrade_service.c must wrap
* the same sequence with __disable_irq().
*
* @param[in] in : struct to program; caller fills magic = OTA_FLAG_MAGIC.
*
* @return  0 on success
*         -1 on erase or program failure
* */
int8_t ota_flag_write(const struct ota_flag_t *in);

#endif /* __OTA_FLAG_H */

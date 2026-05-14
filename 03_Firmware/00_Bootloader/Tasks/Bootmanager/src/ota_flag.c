/******************************************************************************
 * @file ota_flag.c
 *
 * @par dependencies
 * - ota_flag.h
 * - flash.h
 * - bootmanager.h (FlagAddress)
 * - string.h
 *
 * @author Ethan-Hang
 *
 * @brief Read/write the OTA flag struct in internal Flash Sector 2.
 *
 * @version V1.0 2026-05-14
 *
 * @note 1 tab == 4 spaces!
 *****************************************************************************/
#include <string.h>

#include "ota_flag.h"
#include "bootmanager.h"

/**
* @brief Read the OTA flag struct from internal Flash.
*
* @param[out] out : caller-supplied buffer.
*
* @return  0 success, -1 invalid magic.
* */
int8_t ota_flag_read(struct ota_flag_t *out)
{
    if (out == NULL)
    {
        return -1;
    }

    /**
    * Flash is memory-mapped, so a plain memcpy from FlagAddress is safe and
    * cheap. No need for any unlock dance on read.
    **/
    memcpy(out, (const void *)FlagAddress, sizeof(*out));

    if (out->magic != OTA_FLAG_MAGIC)
    {
        return -1;
    }
    return 0;
}

/**
* @brief Erase Sector 2 then program the four 32-bit fields word-by-word.
*
* @param[in] in : struct to write.
*
* @return  0 success, -1 erase/program failure.
* */
int8_t ota_flag_write(const struct ota_flag_t *in)
{
    if (in == NULL)
    {
        return -1;
    }

    /**
    * Erase first — single 16 KB sector is the smallest erase unit on F411.
    * EreaseAppSector wraps Flash_Unlock + FLASH_EraseSector + Flash_Lock.
    **/
    if (EreaseAppSector(FLASH_Sector_2) != FLASH_COMPLETE)
    {
        return -1;
    }

    /**
    * Program 4×32-bit words at FlagAddress. Flash_Write internally unlocks
    * and locks around each word — wasteful but cheap relative to the erase
    * cost we just paid. Re-reading after the write would catch a stuck-bit
    * failure, but Flash_Write already verifies FLASH_COMPLETE status.
    **/
    const uint32_t *src = (const uint32_t *)in;
    for (uint32_t i = 0; i < (sizeof(*in) / sizeof(uint32_t)); i++)
    {
        Flash_Write(FlagAddress + (i * sizeof(uint32_t)), src[i]);
    }

    /**
    * Read-back verification: catch the case where the sector erase silently
    * left a word at non-0xFFFFFFFF (write-protect, voltage glitch) and the
    * subsequent program quietly failed.
    **/
    struct ota_flag_t verify;
    memcpy(&verify, (const void *)FlagAddress, sizeof(verify));
    if (memcmp(&verify, in, sizeof(verify)) != 0)
    {
        return -1;
    }

    return 0;
}

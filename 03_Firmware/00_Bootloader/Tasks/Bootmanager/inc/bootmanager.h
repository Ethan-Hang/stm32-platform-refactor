/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BOOT_MANAGER_H
#define __BOOT_MANAGER_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>

#include "flash.h"

#include "aes.h"

/* Exported types ------------------------------------------------------------*/
typedef void (*pFunction)(void);
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define ApplicationAddress AppAddress /* Use Flash defined address */
#define NVIC_VectTab_FLASH ((uint32_t)0x08000000)
/* Internal Flash layout:
 *   0x08000000 - 0x08007FFF  ( 32K)  Bootloader (this image)
 *   0x08008000 - 0x0800BFFF  ( 16K)  OTA Flag (sector 2)
 *   0x0800C000 - 0x0807FFFF  (464K)  APP (sectors 3-7)
 * OTA staging now lives on the external W25Q64, so no internal backup partition. */
#define FlagAddress    0x08008000UL /* OTA flag sector (Sector 2) */
#define AppAddress     0x0800C000UL /* APP start (Sector 3) */
#define BackAppAddress 0UL          /* deprecated — OTA backup is on external W25Q64 */

/* Exported functions ------------------------------------------------------- */
void   jump_to_app(void);
int8_t back_to_app(int32_t buf_size);
int8_t aes_decrypt_data(uint8_t *input, uint32_t input_len, uint8_t *output,
                        uint32_t *output_len);
int8_t exA_to_exB_AES(int32_t fl_size);
int8_t exB_to_app(void);
int8_t app_to_exA(uint32_t fl_size);
int8_t exA_to_app(void);
void   OTA_StateManager(void);
void ota_apply_update(int32_t file_size, bool first_boot);


#define EE_OTA_NO_APP_UPDATE     0x00
#define EE_OTA_DOWNLOADING       0x11
#define EE_OTA_DOWNLOAD_FINISHED 0x22
#define EE_OTA_APP_CHECK_START   0x33
#define EE_OTA_APP_CHECKING      0x44
#define EE_INIT_NO_APP           0xFF

/* Maximum app image size that fits in the internal-flash APP slot
   (0x0800C000..0x0807FFFF = 464KB = 0x74000). Used by bootmanager to reject
   oversized incoming OTA images before erasing the slot. */
#define OTA_APP_MAX_SIZE         0x74000UL

#endif /* __BOOT_MANAGER_H */

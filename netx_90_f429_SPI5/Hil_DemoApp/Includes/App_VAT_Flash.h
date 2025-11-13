#ifndef APP_VAT_FLASH_H_
#define APP_VAT_FLASH_H_

#include <stdint.h>
#include "App_VAT_Parameters.h"

/******************************************************************************
 * FLASH STORAGE CONFIGURATION
 ******************************************************************************/

/* STM32 F429 Flash Sector 11 for parameter storage */
#define VAT_FLASH_SECTOR        11
#define VAT_FLASH_BASE_ADDR     0x080E0000
#define VAT_FLASH_SIZE          (128 * 1024)  /* 128 KB */

/* Magic number for validity check */
#define VAT_FLASH_MAGIC         0x56415430    /* "VAT0" */

/* Storage format version */
#define VAT_FLASH_VERSION       1

/******************************************************************************
 * FLASH STORAGE STRUCTURE
 ******************************************************************************/

typedef struct VAT_PARAM_STORAGE_Ttag {
    uint32_t magic_number;          /* 0x56415430 ("VAT0") */
    uint32_t version;               /* Storage format version */
    uint32_t crc32;                 /* CRC32 checksum */
    uint32_t timestamp;             /* Last save timestamp */

    uint8_t param_data[99][32];     /* 99 parameters, max 32 bytes each */
    uint8_t param_valid[13];        /* Valid flags (99 bits = 13 bytes) */

    uint8_t assembly_config[16];    /* Assembly configuration */
    uint8_t reserved[448];          /* Reserved for future use */
} VAT_PARAM_STORAGE_T;

/* Total size: 4+4+4+4 + 3168 + 13 + 16 + 448 = 3661 bytes */

/******************************************************************************
 * FUNCTION PROTOTYPES
 ******************************************************************************/

/* Flash operations */
int32_t VAT_Flash_EraseSector(uint8_t sector);
int32_t VAT_Flash_Write(uint32_t address, const uint8_t* pData, uint32_t size);
int32_t VAT_Flash_Read(uint32_t address, uint8_t* pData, uint32_t size);

/* Parameter storage operations */
int32_t VAT_Param_SaveToFlash(VAT_PARAM_MANAGER_T* ptManager);
int32_t VAT_Param_LoadFromFlash(VAT_PARAM_MANAGER_T* ptManager);

/* CRC32 calculation */
uint32_t VAT_CRC32_Calculate(const uint8_t* pData, uint32_t size);

#endif /* APP_VAT_FLASH_H_ */

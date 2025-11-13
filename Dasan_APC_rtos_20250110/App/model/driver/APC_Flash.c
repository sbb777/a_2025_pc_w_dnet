/*
 * APC_Flash.c
 *
 *  Created on: 2023. 11. 17.
 *      Author: Yongseok
 */

#include <APC_Flash.h>

#define FLASH_SECTOR_MAX    12
#define FLASH_WRP_SECTORS   (OB_WRP_SECTOR_8 | OB_WRP_SECTOR_9) /* sectors 8 and 9  */

typedef struct {
    uint32_t addr;
    uint32_t length;
} FlashDataType;


static FlashDataType flash_data[FLASH_SECTOR_MAX] =
{
	{0x08000000,  16*1024},  // 0x08000000 - 0x08003FFF
	{0x08004000,  16*1024},  // 0x08004000 - 0x08007FFF
	{0x08008000,  16*1024},  // 0x08008000 - 0x0800BFFF
	{0x0800C000,  16*1024},  // 0x0800C000 - 0x0800FFFF
	{0x08010000,  64*1024},  // 0x08010000 - 0x0801FFFF
	{0x08020000, 128*1024},  // 0x08020000 - 0x0803FFFF
	{0x08040000, 128*1024},  // 0x08040000 - 0x0805FFFF
	{0x08060000, 128*1024},  // 0x08060000 - 0x0807FFFF

	{0x08080000, 128*1024},  // 0x08080000 - 0x0809FFFF
	{0x080A0000, 128*1024},  // 0x080A0000 - 0x080BFFFF
	{0x080C0000, 128*1024},  // 0x080C0000 - 0x080DFFFF
	{0x080E0000, 128*1024},  // 0x080E0000 - 0x080FFFFF
};

extern NOR_HandleTypeDef hnor1;

bool isInSector(uint16_t sector_num, uint32_t addr, uint32_t length);
uint32_t getWriteProtect(uint32_t WRPSectors);
uint8_t enableWriteProtect(uint32_t WRPSectors);
uint8_t disableWriteProtect(uint32_t WRPSectors);

void initFlash(void)
{
#if 0
    // not use NOR flash
    NOR_IDTypeDef norID;
    HAL_NOR_Read_ID(&hnor1, &norID);
    HAL_NOR_ReturnToReadMode(&hnor1);
#endif
}

bool eraseFlash(uint32_t addr, uint32_t length)
{
    bool ret = false;

    int16_t  start_sector_num = -1;
    uint32_t sector_count = 0;

    for (int i=0; i < FLASH_SECTOR_MAX; i++)
    {
        if (isInSector(i, addr, length) == true) {
            if (start_sector_num < 0) {
                start_sector_num = i;
            }
            sector_count++;
        }
    }

    if (sector_count > 0)
    {
        FLASH_EraseInitTypeDef eraseInitStruct;
        uint32_t page_error;

        HAL_FLASH_Unlock();
/*
 * https://community.st.com/t5/stm32-mcus-embedded-software/why-would-flash-flag-pgperr-and-flash-flag-pgserr-be-set-after-a/m-p/351906
 */
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

        eraseInitStruct.TypeErase   = FLASH_TYPEERASE_SECTORS;
        eraseInitStruct.Banks       = FLASH_BANK_1;
        eraseInitStruct.Sector      = start_sector_num;
        eraseInitStruct.NbSectors   = sector_count;
        eraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

        HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&eraseInitStruct, &page_error);
        if (status == HAL_OK) {
            ret = true;
        }

        HAL_FLASH_Lock();
    }

    return ret;
}

bool writeFlash(uint32_t addr, uint8_t *p_data, uint32_t length)
{
    bool ret = true;
    HAL_FLASH_Unlock();

    for (int i = 0; i < length; i++)
    {
        uint16_t data = p_data[i+0] << 0;

        HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr + i, (uint64_t)data);
        if (status != HAL_OK)
        {
            ret = false;
            break;
        }
    }

    HAL_FLASH_Lock();
    return ret;
}

bool writeByteToFlash(uint32_t addr, uint8_t data)
{
    bool ret = true;
    HAL_FLASH_Unlock();

    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr, (uint64_t)data);
    if (status != HAL_OK)
    {
        ret = false;
    }

    HAL_FLASH_Lock();
    return ret;
}

bool writeUint32ToFlash(uint32_t addr, uint32_t data)
{
    bool ret = true;
    HAL_FLASH_Unlock();

    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *(uint32_t *)&data);
    if (status != HAL_OK)
    {
        ret = false;
    }

    HAL_FLASH_Lock();
    return ret;
}

bool writeUint16ToFlash(uint32_t addr, uint16_t data)
{
    bool ret = true;
    HAL_FLASH_Unlock();

    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, *(uint16_t *)&data);
    if (status != HAL_OK)
    {
        ret = false;
    }

    HAL_FLASH_Lock();
    return ret;
}

bool writeIntToFlash(uint32_t addr, int data)
{
    bool ret = true;
    HAL_FLASH_Unlock();

    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *(int *)&data);
    if (status != HAL_OK)
    {
        ret = false;
    }

    HAL_FLASH_Lock();
    return ret;
}

bool writeFloatToFlash(uint32_t addr, float data)
{
    bool ret = true;
    HAL_FLASH_Unlock();

    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *(float *)&data);
    if (status != HAL_OK)
    {
        ret = false;
    }

    HAL_FLASH_Lock();
    return ret;
}

bool writeShortToFlash(uint32_t addr, short data)
{
    bool ret = true;
    HAL_FLASH_Unlock();

    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, *(short *)&data);
    if (status != HAL_OK)
    {
        ret = false;
    }

    HAL_FLASH_Lock();
    return ret;
}

bool readFlash(uint32_t addr, uint8_t *p_data, uint32_t length)
{
    bool ret = true;
    uint8_t *p_byte = (uint8_t *)addr;

    for (int i=0; i < length; i++)
    {
        p_data[i] = p_byte[i];
    }

    return ret;
}

uint8_t readByteFromFlash(uint32_t addr)
{
    //__disable_irq();

    // Read one byte from flash
    uint8_t data = *(__IO uint8_t *)addr;
    //uint8_t data = *( uint8_t *)addr;

    //__enable_irq();

    return data;
}

uint32_t readUint32FromFlash(uint32_t addr)
{
    uint32_t rawData = * (__IO uint32_t *)addr;
    return rawData;
}

uint16_t readUint16FromFlash(uint32_t addr)
{
    uint16_t rawData = *(__IO uint16_t *)addr;
    return rawData;
}

int readIntFromFlash(uint32_t addr)
{
    uint32_t rawData = *(__IO uint32_t *)addr;
    int data = *((int *)&rawData);
    return data;
}

float readFloatFromFlash(uint32_t addr)
{
    uint32_t rawData = *(__IO uint32_t *)addr;
    float data = *((float *)&rawData);
    return data;
}

short readShortFromFlash(uint32_t addr)
{
    uint16_t rawData = *(__IO uint16_t *)addr;
    short data = *((short *)&rawData);

    return data;
}

bool isInSector(uint16_t sector_num, uint32_t addr, uint32_t length)
{
    bool ret = false;

    uint32_t sector_start = flash_data[sector_num].addr;
    uint32_t sector_end   = flash_data[sector_num].addr + flash_data[sector_num].length - 1;
    uint32_t flash_start  = addr;
    uint32_t flash_end    = addr + length - 1;

    if (sector_start >= flash_start && sector_start <= flash_end) {
        ret = true;
    }

    if (sector_end >= flash_start && sector_end <= flash_end) {
        ret = true;
    }

    if (flash_start >= sector_start && flash_start <= sector_end) {
        ret = true;
    }

    if (flash_end >= sector_start && flash_end <= sector_end) {
        ret = true;
    }

    return ret;
}

uint32_t getWriteProtect(uint32_t WRPSectors)
{
    FLASH_OBProgramInitTypeDef OBInit;

    HAL_FLASHEx_OBGetConfig(&OBInit);
    uint32_t SectorsWRPStatus = OBInit.WRPSector & WRPSectors; // FLASH_WRP_SECTORS

    return SectorsWRPStatus;
}

uint8_t enableWriteProtect(uint32_t WRPSectors)
{
    FLASH_OBProgramInitTypeDef OBInit;

    HAL_FLASH_OB_Unlock();
    HAL_FLASH_Unlock();

    OBInit.OptionType = OPTIONBYTE_WRP;
    OBInit.WRPState   = OB_WRPSTATE_ENABLE;
    OBInit.Banks      = FLASH_BANK_1;
    OBInit.WRPSector  = WRPSectors; // FLASH_WRP_SECTORS
    HAL_FLASHEx_OBProgram(&OBInit);

    if (HAL_FLASH_OB_Launch() != HAL_OK)
    {
        return HAL_ERROR;
    }

    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();

    return HAL_OK;
}

uint8_t disableWriteProtect(uint32_t WRPSectors)
{
    FLASH_OBProgramInitTypeDef OBInit;

    HAL_FLASH_OB_Unlock();
    HAL_FLASH_Unlock();

    OBInit.OptionType = OPTIONBYTE_WRP;
    OBInit.WRPState   = OB_WRPSTATE_DISABLE;
    OBInit.Banks      = FLASH_BANK_1;
    OBInit.WRPSector  = WRPSectors; // FLASH_WRP_SECTORS
    HAL_FLASHEx_OBProgram(&OBInit);

    if (HAL_FLASH_OB_Launch() != HAL_OK)
    {
        return HAL_ERROR;
    }

    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();

    return HAL_OK;
}

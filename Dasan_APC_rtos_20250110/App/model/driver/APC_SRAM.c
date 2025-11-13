/*
 * APC_SRAM.c
 *
 *  Created on: Mar 25, 2024
 *      Author: Yongseok
 */
#include "APC_SRAM.h"

#include "APC_Utils.h"

#include <APC_Define.h>

#ifdef __REV_MTECH__
extern SRAM_HandleTypeDef hsram2;
extern SRAM_HandleTypeDef hsram4;
#else
extern SRAM_HandleTypeDef hsram3;
#endif

static bool _sram_intialized = false;

uint32_t convertAddress(uint8_t dir, uint32_t addr, uint32_t base_addr);

void initSRAM(void)
{
    _sram_intialized = true;
}

bool readSRAM2(uint32_t addr, uint16_t data)
{
#ifdef __REV_MTECH__
  uint32_t conv_addr = convertAddress(0, NOR_MEMORY_ADRESS2 + addr, NOR_MEMORY_ADRESS2);
  return HAL_SRAM_Read_16b(&hsram2, (uint32_t*) conv_addr, &data, 2) == HAL_OK ? true : false;
#else
    uint32_t conv_addr = convertAddress(0, NOR_MEMORY_ADRESS2 + addr, NOR_MEMORY_ADRESS2);
    return HAL_SRAM_Read_16b(&hsram2, (uint32_t*) conv_addr, &data, 2) == HAL_OK ? true : false;
#endif
}

bool writeSRAM2(uint32_t addr, uint16_t data)
{
#ifdef __REV_MTECH__
  uint32_t conv_addr = convertAddress(1, NOR_MEMORY_ADRESS2 + addr, NOR_MEMORY_ADRESS2);
  return HAL_SRAM_Write_16b(&hsram2, (uint32_t*) conv_addr, &data, 2) == HAL_OK ? true : false;
#else
    uint32_t conv_addr = convertAddress(1, NOR_MEMORY_ADRESS2 + addr, NOR_MEMORY_ADRESS2);
    return HAL_SRAM_Write_16b(&hsram2, (uint32_t*) conv_addr, &data, 2) == HAL_OK ? true : false;
#endif
}

bool readSRAM3(uint32_t addr, uint8_t data)
{
#ifdef __REV_MTECH__
#else
    return HAL_SRAM_Read_8b(&hsram3, (uint32_t*)(NOR_MEMORY_ADRESS3 + addr), &data, 1) == HAL_OK ? true : false;
#endif
}

bool writeSRAM3(uint32_t addr, uint8_t data)
{
#ifdef __REV_MTECH__
#else
    return HAL_SRAM_Write_8b(&hsram3, (uint32_t*)(NOR_MEMORY_ADRESS3 + addr), &data, 1) == HAL_OK ? true : false;
#endif
}

uint8_t readSRAM4(uint32_t addr)
{
#ifdef __REV_MTECH__
	uint8_t data;
  HAL_SRAM_Read_8b(&hsram4, (uint32_t*)(NOR_MEMORY_ADRESS4 + addr), &data, 1);
  return GPIOC->IDR & 0xFF;
#else
	uint8_t data = 0x00;
	HAL_SRAM_Read_8b(&hsram4, (uint32_t*)(NOR_MEMORY_ADRESS4 + addr), &data, 1);
	return data;
#endif
}

bool writeSRAM4(uint32_t addr, uint8_t data)
{
#ifdef __REV_MTECH__
	//GPIOC->ODR = (GPIOC->IDR & 0xFF00) | data;
  return HAL_SRAM_Write_8b(&hsram4, (uint32_t*)(NOR_MEMORY_ADRESS4 + addr), &data, 1) == HAL_OK ? true : false;
#else
	return HAL_SRAM_Write_8b(&hsram4, (uint32_t*)(NOR_MEMORY_ADRESS4 + addr), &data, 1) == HAL_OK ? true : false;
#endif
}

/*
 * - dir: 1(write), 0(read)
 */
uint32_t convertAddress(uint8_t dir, uint32_t addr, uint32_t base_addr)
{
    uint32_t new_addr = (addr - base_addr);

    if (dir == 1) // Write
        new_addr <<= 1;
    else // Read
        new_addr >>= 1;

    new_addr += base_addr;
    return new_addr;
}

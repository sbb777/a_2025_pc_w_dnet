/*
 * APC_SRAM.h
 *
 *  Created on: Mar 25, 2024
 *      Author: Yongseok
 */

#ifndef _APC_SRAM_H_
#define _APC_SRAM_H_

#include "APC_Define.h"

#define ADDR_CS_IF1_     (1 << 17)
#define ADDR_CS_IF2_     (1 << 18)
#define ADDR_CS_IF3_     ((1 << 18) | (1 << 17))
#define ADDR_CS_PS_      (1 << 19)


void initSRAM(void);

bool readSRAM2(uint32_t addr, uint16_t data);
bool writeSRAM2(uint32_t addr, uint16_t data);

bool readSRAM3(uint32_t addr, uint8_t data);
bool writeSRAM3(uint32_t addr, uint8_t data);

uint8_t readSRAM4(uint32_t addr);
bool writeSRAM4(uint32_t addr, uint8_t data);

#endif /* _APC_SRAM_H_ */

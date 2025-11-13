/*
 * APC_Flash.h
 *
 *  Created on: 2023. 11. 17.
 *      Author: Yongseok
 */

#ifndef _APC_FLASH_H_
#define _APC_FLASH_H_

#include <APC_Define.h>


void initFlash(void);

bool eraseFlash(uint32_t addr, uint32_t length);

bool writeFlash(uint32_t addr, uint8_t *p_data, uint32_t length);
bool writeByteToFlash(uint32_t addr, uint8_t data);
bool writeUint32ToFlash(uint32_t addr, uint32_t data);
bool writeUint16ToFlash(uint32_t addr, uint16_t data);

bool writeIntToFlash(uint32_t addr, int data);
bool writeFloatToFlash(uint32_t addr, float data);
bool writeShortToFlash(uint32_t addr, short data);

bool readFlash(uint32_t addr, uint8_t *p_data, uint32_t length);
uint8_t readByteFromFlash(uint32_t addr);
uint32_t readUint32FromFlash(uint32_t addr);
uint16_t readUint16FromFlash(uint32_t addr);

int   readIntFromFlash(uint32_t addr);
float readFloatFromFlash(uint32_t addr);
short readShortFromFlash(uint32_t addr);


#endif /* _APC_FLASH_H_ */

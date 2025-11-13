/*
 * APC_DAC7612.h
 *
 *  Created on: Mar 24, 2024
 *      Author: Yongseok
 */

#ifndef _APC_DAC7612_H_
#define _APC_DAC7612_H_

#include <APC_Define.h>

#define DAC7612_SCALE   4095


void initDAC7612(void);

void getDAC7612Config(uint8_t *pData, uint8_t len);
bool setDAC7612Config(uint8_t *pData, uint8_t len);

void setDAC7612Value(uint32_t addr, uint16_t value);

#endif /* _APC_DAC7612_H_ */

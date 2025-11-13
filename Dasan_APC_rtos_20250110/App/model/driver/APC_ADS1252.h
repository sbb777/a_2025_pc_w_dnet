/*
 * APC_ADS1252.h
 *
 *  Created on: 2023. 11. 16.
 *      Author: Yongseok
 */

#ifndef _APC_ADS1252_H_
#define _APC_ADS1252_H_

#include <APC_Define.h>


void initADS1252(void);

int getADS1252(uint8_t ch);

void getADS1252Config(uint8_t *pData, uint8_t len);
bool setADS1252Config(uint8_t *pData, uint8_t len);

#endif /* _APC_ADS1252_H_ */

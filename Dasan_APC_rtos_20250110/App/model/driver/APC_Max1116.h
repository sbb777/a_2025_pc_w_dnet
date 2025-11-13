/*
 * APC_Max1116.h
 *
 *  Created on: Apr 9, 2024
 *      Author: Yongseok
 */

#ifndef _APC_MAX1116_H_
#define _APC_MAX1116_H_

#include <APC_Define.h>

#define MAX1116_SCALE   4096


void initMax1116(void);

void getMax1116Config(uint8_t *pData, uint8_t len);
bool setMax1116Config(uint8_t *pData, uint8_t len);

uint8_t getMax1116Value(void);

#endif /* _APC_MAX1116_H_ */

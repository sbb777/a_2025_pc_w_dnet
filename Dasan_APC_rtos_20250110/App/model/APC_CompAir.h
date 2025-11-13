/*
 * APC_CompAir.h
 *
 *  Created on: Oct 24, 2023
 *      Author: Yongseok
 */

#ifndef _APC_COMPAIR_H_
#define _APC_COMPAIR_H_

#include <APC_Define.h>

bool initCompAir(void);

uint8_t checkCompAirStatus(uint8_t ch, bool option);
uint8_t getCompAirStatus();

uint8_t getCompAir(uint8_t ch);
uint32_t getCompAirValue(void);

static uint8_t _comp_air_val;

#endif /* _APC_COMPAIR_H_ */

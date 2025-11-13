/*
 * APC_Power.h
 *
 *  Created on: Oct 24, 2023
 *      Author: Yongseok
 */

#ifndef _APC_POWER_H_
#define _APC_POWER_H_

#include <APC_Define.h>

bool initPower(void);

int  getPower(uint8_t ch);
bool setPower(uint8_t status);

void refreshPower();

static int _cur_volt;

#endif /* _APC_POWER_H_ */

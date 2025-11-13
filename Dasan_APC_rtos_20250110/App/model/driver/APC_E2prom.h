/*
 * APC_E2prom.h
 *
 *  Created on: Oct 18, 2023
 *      Author: Yongseok
 */

#ifndef _APC_E2PROM_H_
#define _APC_E2PROM_H_

#include <APC_Define.h>

void initE2prom(void);

bool getE2promData(uint16_t addr, uint8_t data);

bool setE2promData(uint16_t addr, uint8_t data);

#endif /* _APC_E2PROM_H_ */

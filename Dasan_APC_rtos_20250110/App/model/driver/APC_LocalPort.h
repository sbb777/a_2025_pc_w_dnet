/*
 * APC_LocalPort.h
 *
 *  Created on: 2023. 8. 16.
 *      Author: Yongseok
 */

#ifndef _APC_LOCALPORT_H_
#define _APC_LOCALPORT_H_

#include <APC_Define.h>

void initLocalPort(void);

uint32_t availableLocalRS232(uint8_t ch);
uint32_t writeLocalRS232Data(uint8_t ch, uint8_t *p_data, uint32_t length);
uint8_t  readLocalRS232Data(uint8_t ch);
uint32_t printLocalRS232(uint8_t ch, const char *fmt, ...);

#endif /* _APC_LOCALPORT_H_ */

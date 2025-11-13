/*
 * APC_CmdFilter.h
 *
 *  Created on: Apr 18, 2024
 *      Author: Yongseok
 */

#ifndef _APC_CMDFILTER_H_
#define _APC_CMDFILTER_H_

#include <APC_Define.h>


uint8_t filterAccessMode(uint8_t ch);
uint8_t filterControlMode(int cmdKey);

#endif /* _APC_CMDFILTER_H_ */

/*
 * APC_Main.h
 *
 *  Created on: Aug 15, 2023
 *      Author: Yongseok
 */

#ifndef _APC_MAIN_H_
#define _APC_MAIN_H_

#include <APC_Utils.h>


bool initMain(void);
void procMain(uint32_t count);

bool getInitStatus(void);

void setTickFlag(bool option);
bool getTickFlag(void);

#endif /* _APC_MAIN_H_ */

/*
 * APC_Display.h
 *
 *  Created on: Oct 18, 2023
 *      Author: Yongseok
 */

#ifndef _APC_DISPLAY_H_
#define _APC_DISPLAY_H_

#include <APC_Define.h>

#define DISPLAY_SIZE    (4)


void initDisplay(void);

char* getDisplayData(void);
bool setDisplayData(char* data);
void setDisplayCodeNumber(uint8_t mode, int number);

bool setDisplayOff(void);

void setLed(bool option);

#endif /* _APC_DISPLAY_H_ */

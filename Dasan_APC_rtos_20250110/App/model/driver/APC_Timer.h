/*
 * APC_Timer.h
 *
 *  Created on: Oct 28, 2023
 *      Author: Yongseok
 */

#ifndef _APC_TIMER_H_
#define _APC_TIMER_H_

#include "APC_Define.h"

typedef enum
{
  APCTIMER_LOOP,
  APCTIMER_ONETIME,
} EN_ApcTimerMode;

typedef enum {
    ApcTimer_Power      = 0,
    ApcTimer_Sensor     = 1,
    ApcTimer_Valve      = 2,
    ApcTimer_CompAir    = 3
} EN_ApcTimerType;


void initTimer(void);
bool setTimer(uint8_t type, void (*func)(void *arg), void *func_arg, EN_ApcTimerMode mode, uint32_t period);
bool startTimer(uint8_t type);
bool stopTimer(uint8_t type);

void APC_Delay_us(uint32_t usec);

//void setTickFlag(bool option);
//bool getTickFlag(void);

#endif /* _APC_TIMER_H_ */

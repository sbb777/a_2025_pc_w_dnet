/*
 * APC_Timer.c
 *
 *  Created on: Oct 28, 2023
 *      Author: Yongseok
 */

#include "APC_Timer.h"
#include <APC_Utils.h>

#define APC_TIMER_SIZE    4


typedef struct
{
	bool enable;
	EN_ApcTimerMode mode;
	uint32_t counter;
	uint32_t reload;
	void (*func)(void *arg);
	void *func_arg;
} ApcTimerType;

static ApcTimerType apc_timer[APC_TIMER_SIZE];

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;


void initTimer(void)
{
	for (int i=0; i < APC_TIMER_SIZE; i++)
	{
		apc_timer[i].enable = false;
		apc_timer[i].counter = 0;
		apc_timer[i].func = NULL;
		apc_timer[i].func_arg = NULL;
	}

    HAL_TIM_Base_Start(&htim3);
    HAL_TIM_Base_Start_IT(&htim6);
}

bool setTimer(uint8_t ch, void (*func)(void *arg), void *func_arg, EN_ApcTimerMode mode, uint32_t period)
{
	if (ch >= APC_TIMER_SIZE)
		return false;

	//apc_timer[ch].enable = false;
	//apc_timer[ch].counter = 0;
	apc_timer[ch].func = func;
	apc_timer[ch].func_arg = func_arg;
	apc_timer[ch].mode = mode;
	apc_timer[ch].reload = period;

	return true;
}

bool startTimer(uint8_t ch)
{
	if (ch >= APC_TIMER_SIZE)
		return false;

	apc_timer[ch].counter = 0;
	apc_timer[ch].enable = true;

	return true;
}

bool stopTimer(uint8_t ch)
{
	if (ch >= APC_TIMER_SIZE)
		return false;

	apc_timer[ch].enable = false;

	return true;
}

void timerISR(void)
{
	for (int i=0; i<APC_TIMER_SIZE; i++)
	{
		if (apc_timer[i].enable == true)
		{
			apc_timer[i].counter++;
			if (apc_timer[i].counter >= apc_timer[i].reload)
			{
				if (apc_timer[i].func != NULL)
				{
					apc_timer[i].func(apc_timer[i].func_arg);
				}

				apc_timer[i].counter = 0;
				if (apc_timer[i].mode == APCTIMER_ONETIME)
				{
					apc_timer[i].enable = false;
				}
			}
		}
	}
}

void APC_Delay_us(uint32_t counter)
{
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    while(__HAL_TIM_GET_COUNTER(&htim3) < counter);
}


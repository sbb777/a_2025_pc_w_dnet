/*
 * APC_Controller.h
 *
 *  Created on: 2023. 11. 12.
 *      Author: Yongseok
 */

#ifndef _APC_CONTROLLER_H_
#define _APC_CONTROLLER_H_

#include <APC_Define.h>
#include <APC_Model.h>


void initController(void);

bool syncControllerData(uint32_t addr);

uint8_t getControllerType(void);
bool setControllerType(uint8_t type);

int getGainFactor(void);
bool setGainFactor(int factor);

//float getGainFactorFloat(void);
//void setGainFactorFloat(float factor);

int getSensorDelay(void);
bool setSensorDelay(int delay);

//float getSensorDelayFloat(void);
//void setSensorDelayFloat(float delay);

uint8_t getRampTime(void);
bool setRampTime(uint8_t time);

uint32_t getPgain(void);
bool setPgain(uint32_t gain);

uint32_t getIgain(void);
bool setIgain(uint32_t gain);

uint32_t getDgain(void);
bool setDgain(uint32_t gain);

//void addIsolationCycleCounter(void);

//uint32_t getCurrentIsoCycleCounter(void);
//bool setCurrentIsoCycleCounter(uint32_t);

//uint32_t getTotalIsoCycleCounter(void);
//bool setTotalIsoCycleCounter(uint32_t);

//void addControlCycleCounter(void);

//uint32_t getCurrentCntlCycleCounter(void);
//bool setCurrentCntlCycleCounter(uint32_t);

//uint32_t getTotalCntlCycleCounter(void);
//bool setTotalCntlCycleCounter(uint32_t);

void setHoldPosition(void);

#endif /* _APC_CONTROLLER_H_ */

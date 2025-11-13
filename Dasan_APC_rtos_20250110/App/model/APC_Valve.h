/*
 * APC_Valve.h
 *
 *  Created on: Oct 24, 2023
 *      Author: Yongseok
 */

#ifndef _APC_VALVE_H_
#define _APC_VALVE_H_

#include <APC_Define.h>
#include <APC_Model.h>

#define VALVE_POSITION_CLOSE    (     0)
#define VALVE_POSITION_OPEN     (100000)

#define VALVE_MIN_SPEED         (     1)
#define VALVE_MAX_SPEED         (  1000)

#define VALVE_PENDULUM          ( 1)
#define VALVE_BUTTERFLY         ( 2)

#define P250_MAX_M_STEP         ( 25000)
#define B080_MAX_M_STEP         ( 40000)

typedef enum {
    ValveType_Pendulum      = 1,
    ValveType_Butterfly     = 2,
    ValveType_P250          = 1,
    ValveType_B080          = 2,
    ValveType_B100          = 3
} EN_ValveType;

typedef enum {
    PositionResolution_100000   = 0,
    PositionResolution_10000    = 1,
    PositionResolution_1000     = 2,
    PositionResolution_100      = 3,
    PositionResolution_90       = 4
} EN_PositionResolution;

typedef enum {
    PositionCommRange_1000      = 0,
    PositionCommRange_10000     = 1,
    PositionCommRange_100000    = 2
} EN_PositionCommRange;

typedef enum {
    ValvePosAfterPowerUp_Close  = 0,
    ValvePosAfterPowerUp_Open   = 1
} EN_ValvePosAfterPowerUp;

typedef enum {
    ValvePosAfterPowerFail_Close    = 0,
    ValvePosAfterPowerFail_Open     = 1
} EN_ValvePosAfterPowerFail;

typedef enum {
    ExtIsolationValveFunc_No        = 0,
    ExtIsolationValveFunc_Yes       = 1
} EN_ExtIsolationValveFunc;

typedef enum {
    CntlStrokeLimit_No      = 0,
    CntlStrokeLimit_Yes     = 1
} EN_CntlStrokeLimit;

typedef enum {
    NetwFailEndPosition_ValveClose  = 0,
    NetwFailEndPosition_ValveOpen   = 1,
    NetwFailEndPosition_ValveStay   = 2
} EN_NetwFailEndPosition;

typedef enum {
    SlaveOfflinePosition_ValveClose = 0,
    SlaveOfflinePosition_ValveOpen  = 1,
    SlaveOfflinePosition_ValveStay  = 2
} EN_SlaveOfflinePosition;

typedef enum {
    SynchronizationStart_Standard       = 0,
    SynchronizationStart_SpecialCommand = 1,
    SynchronizationStart_OpenCommand    = 2,
    SynchronizationStart_AllMoveCommand = 3,
    SynchronizationStart_Always         = 4
} EN_SynchronizationStart;

typedef enum {
    SynchronizationMode_Short   = 0,
    SynchronizationMode_Full    = 1
} EN_SynchronizationMode;


bool initValve(void);
bool syncValveData(uint32_t addr);
bool syncValveCount();

void addIsolationCycleCounter(void);
uint32_t getCurrentIsoCycleCounter(void);
bool setCurrentIsoCycleCounter(uint32_t);

uint32_t getTotalIsoCycleCounter(void);
bool setTotalIsoCycleCounter(uint32_t);
void addControlCycleCounter(void);

uint32_t getCurrentCntlCycleCounter(void);
bool setCurrentCntlCycleCounter(uint32_t);

uint32_t getTotalCntlCycleCounter(void);
bool setTotalCntlCycleCounter(uint32_t);

uint32_t getCurrentPowerUpCounter();
void setCurrentPowerUpCounter(uint32_t counter);

uint32_t getPFOCycleCounter(void);
void setPFOCycleCounter(uint32_t counter);
void addPFOCycleCounter();
void addPowerUpCounter();

uint8_t getPID_Kp();
uint8_t getPID_Ki();
uint8_t getPID_Kd();

void setValveOpen(void);
void setValveClosed(void);
//void setValveHold(void);

EN_PositionCommRange getPositionCommRange(void);
void setPositionCommRange(EN_PositionCommRange range);

uint32_t getSetPosition(void);
uint32_t getSetpointValvePosition(void);
void     setSetpointValvePosition(uint32_t absolute_pos);

int  getCurrentValvePosition(void);
int  getCurrentEncoder(void);
void _getCurrentValvePosition(void);
void setCurrentValvePosition(int relative_pos);

void setCurrentValvePosUsingAbsoluteStep(int absolute_step);
void setCurrentValvePosUsingRelativeStep(int relative_step);

uint32_t getValveMaxSpeed(void);
void     setValveMaxSpeed(uint32_t speed);

uint32_t getValveSpeed(void);
void     setValveSpeed(uint32_t speed);

uint8_t getValvePosAfterPowerUp();
void    setValvePosAfterPowerUp(uint8_t option);

uint8_t getValvePosAfterPowerFail();
void    setValvePosAfterPowerFail(uint8_t option);

uint8_t getValvePosAfterNetwFail();
void    setValvePosAfterNetwFail(uint8_t option);

uint8_t getExtIsolationValveFunc();
void    setExtIsolationValveFunc(uint8_t option);

uint8_t getCntlStrokeLimit();
void    setCntlStrokeLimit(uint8_t option);

uint8_t getValvePositionAfterSync();
void    setValvePositionAfterSync(uint8_t option);

uint8_t getNetwFailEndPosition();
void    setNetwFailEndPosition(uint8_t option);

uint8_t getSlaveOfflinePosition();
void    setSlaveOfflinePosition(uint8_t option);

uint8_t getSynchronizationStart();
void    setSynchronizationStart(uint8_t option);

uint8_t getSynchronizationMode();
void    setSynchronizationMode(uint8_t option);

void stopSynchronization();

int synchronizeClosedInit(uint32_t option);

int synchronizeOpenInit(uint32_t option);
int synchronizeOpenStepByStep();

int getSynchronizationResult(void);

uint8_t getValveMotorStatus(void);
int     getValveEncoderPosition();

uint32_t getValveFullStroke(void);
void     setValveFullStroke(uint32_t vfs);

void    setValveType(uint8_t type);
uint8_t getValveType(void);

#endif /* _APC_VALVE_H_ */

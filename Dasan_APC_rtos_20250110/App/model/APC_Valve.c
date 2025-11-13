/*
 * APC_Valve.c
 *
 *  Created on: Oct 24, 2023
 *      Author: Yongseok
 */

#include <APC_Valve.h>
#include <APC_Utils.h>
#include <APC_Flash.h>
#include <APC_LocalPort.h>

#include <APC_Board.h>
#include <APC_ControlMode.h>
#include "APC_Error.h"
#include "APC_Warning.h"
#include "APC_Display.h"
#include "APC_TMC.h"
#include "APC_Sensor.h"
#include "APC_Controller.h"

#define SPEED_COEFFICIENT   (5000)
#define ACCEL_COEFFICIENT   (  25)

/*********************************************************************************/
#define ADDR_VALVE_START            (_ADDR_MO_DATA +  128)

#define ADDR_ValveType              (ADDR_VALVE_START +  0) // 1byte
#define ADDR_PositionCommRange      (ADDR_VALVE_START +  1) // 1byte
#define ADDR_ValveSize              (ADDR_VALVE_START +  2) // 2byte
#define ADDR_ValveFullScale         (ADDR_VALVE_START +  4) // 4byte

#define ADDR_ValvePosAfterPowerUp   (ADDR_VALVE_START + 10)
#define ADDR_ValvePosAfterPowerFail (ADDR_VALVE_START + 11)
#define ADDR_ValvePosAfterNetwFail  (ADDR_VALVE_START + 12)
#define ADDR_ExtIsolationValveFunc  (ADDR_VALVE_START + 13)
#define ADDR_CntlStrokeLimit        (ADDR_VALVE_START + 14)
#define ADDR_ValvePosAfterSync      (ADDR_VALVE_START + 15)
#define ADDR_NetwFailEndPosition    (ADDR_VALVE_START + 16)
#define ADDR_SlaveOfflinePosition   (ADDR_VALVE_START + 17)
#define ADDR_SynchronizationStart   (ADDR_VALVE_START + 18)
#define ADDR_SynchronizationMode    (ADDR_VALVE_START + 19)
#define ADDR_PID_Kp                 (ADDR_VALVE_START + 20)
#define ADDR_PID_Ki                 (ADDR_VALVE_START + 21)
#define ADDR_PID_Kd                 (ADDR_VALVE_START + 22)

#define ADDR_TotalIsoCycleCounter   (0x080C0040)
#define ADDR_TotalCntlCycleCounter  (0x080C0044)
#define ADDR_PFOCycleCounter        (0x080C0048)


/*********************************************************************************/
static bool _valve_initialized  = false;
static bool _stop_sync          = false;

// -. Motor Step Angle : Motor가 1 Step 당 이동하는 각도, Step의 Pulse 를 줌으로 움직인다.
//      -> 1.8   (1회전에 200 steps 필요)
//      -> 0.72  (1회전에 500 steps 필요)
//      -> 0.144 (1회전에 2500 steps 필요)
// -. Motor resolution : 2 / 4 / 8 --> Step Angle 에 따른 steps 수에 * resolution = 1회전 총 steps
//      (ex) 0.72(500steps)   * 4 = 2000 steps
//      (ex) 0.72(500steps)   * 8 = 4000 steps
//      (ex) 0.144(2500steps) * 4 = 10000 steps
//      (ex) 0.144(2500steps) * 8 = 20000 steps , 70도에 해당되는 steps = 20000 * (70/360) = 4000 steps

static EN_ValveType                 MO_ValveType        ;
static EN_PositionCommRange         MO_PositionCommRange;        // 0, 1, 2
static uint16_t                     MO_ValveSize        ;
static uint32_t                     MO_ValveFullScale   ;  // MAX_M_STEP

static EN_ValvePosAfterPowerUp      MO_ValvePosAfterPowerUp    ;
static EN_ValvePosAfterPowerFail    MO_ValvePosAfterPowerFail  ;
static uint8_t                      MO_ValvePosAfterNetwFail   ;
static EN_ExtIsolationValveFunc     MO_ExtIsolationValveFunc   ;
static EN_CntlStrokeLimit           MO_CntlStrokeLimit         ;
static uint8_t                      MO_ValvePosAfterSync       ;
static EN_NetwFailEndPosition       MO_NetwFailEndPosition     ;
static EN_SlaveOfflinePosition      MO_SlaveOfflinePosition    ;
static EN_SynchronizationStart      MO_SynchronizationStart    ;
static EN_SynchronizationMode       MO_SynchronizationMode     ;

static uint8_t                      MO_PID_Kp;
static uint8_t                      MO_PID_Ki;
static uint8_t                      MO_PID_Kd;

//static uint32_t    MO_VALVE_RangeConfiguration = 0;
static int    _CurrentValvePosition    = 0;
static int    _CurrentEncoderPos       = 0;
static uint32_t    _ValveSetpointPosition   = 0;
static uint32_t    _ValveSpeed              = VALVE_MAX_SPEED;   // ratio

static int     tmc_CurrentMotorStep ;
static int     tmc_SetpointMotorStep;

static uint32_t     _CurrentIsoCycleCounter = 0;
static uint32_t     MO_TotalIsoCycleCounter   = 0;
static uint32_t     _CurrentCntlCycleCounter= 0;
static uint32_t     MO_TotalCntlCycleCounter  = 0;

static uint32_t     MO_PFOCycleCounter        = 0;
static uint32_t     _CurrentPowerUpCounter    = 0;

/*********************************************************************************/
//int pollCurrentValvePosition(void);
void addCycleCounter(int absolute_pos);
void checkValveCondition(void);
void syncEncoderAndMotor(int offset);

bool initValve(void)
{
    MO_ValveType              = readByteFromFlash(ADDR_ValveType);
    MO_PositionCommRange      = readByteFromFlash(ADDR_PositionCommRange);
    MO_ValveSize              = readUint16FromFlash(ADDR_ValveSize);
    MO_ValveFullScale         = readUint32FromFlash(ADDR_ValveFullScale);
    MO_ValvePosAfterPowerUp   = readByteFromFlash(ADDR_ValvePosAfterPowerUp);
    MO_ValvePosAfterPowerFail = readByteFromFlash(ADDR_ValvePosAfterPowerFail);
    MO_ValvePosAfterNetwFail  = readByteFromFlash(ADDR_ValvePosAfterNetwFail);
    MO_ExtIsolationValveFunc  = readByteFromFlash(ADDR_ExtIsolationValveFunc);
    MO_CntlStrokeLimit        = readByteFromFlash(ADDR_CntlStrokeLimit);
    MO_ValvePosAfterSync      = readByteFromFlash(ADDR_ValvePosAfterSync);
    MO_NetwFailEndPosition    = readByteFromFlash(ADDR_NetwFailEndPosition);
    MO_SlaveOfflinePosition   = readByteFromFlash(ADDR_SlaveOfflinePosition);
    MO_SynchronizationStart   = readByteFromFlash(ADDR_SynchronizationStart);
    MO_SynchronizationMode    = readByteFromFlash(ADDR_SynchronizationMode);
    MO_PID_Kp                 = readByteFromFlash(ADDR_PID_Kp);
    MO_PID_Ki                 = readByteFromFlash(ADDR_PID_Ki);
    MO_PID_Kd                 = readByteFromFlash(ADDR_PID_Kd);

    if (MO_ValveType == 0xff)               MO_ValveType = 0;
    if (MO_PositionCommRange == 0xff)       MO_PositionCommRange = PositionCommRange_1000;
    if (MO_ValveSize == 0xffff)             MO_ValveSize = 250;
    if (MO_ValveFullScale == UINT32_MAX)    MO_ValveFullScale = 0;
    if (MO_ValvePosAfterPowerUp == 0xff)    MO_ValvePosAfterPowerUp = ValvePosAfterPowerUp_Open;
    if (MO_ValvePosAfterPowerFail == 0xff)  MO_ValvePosAfterPowerFail = ValvePosAfterPowerFail_Open;
    if (MO_ValvePosAfterNetwFail == 0xff)   MO_ValvePosAfterNetwFail = 0;
    if (MO_ExtIsolationValveFunc == 0xff)   MO_ExtIsolationValveFunc = ExtIsolationValveFunc_No;
    if (MO_CntlStrokeLimit == 0xff)         MO_CntlStrokeLimit = 1;
    if (MO_ValvePosAfterSync == 0xff)       MO_ValvePosAfterSync = 0;
    if (MO_NetwFailEndPosition == 0xff)     MO_NetwFailEndPosition = NetwFailEndPosition_ValveOpen;
    if (MO_SlaveOfflinePosition == 0xff)    MO_SlaveOfflinePosition = SlaveOfflinePosition_ValveOpen;
    if (MO_SynchronizationStart == 0xff)    MO_SynchronizationStart = 1;
    if (MO_SynchronizationMode == 0xff)     MO_SynchronizationMode = 1;
    if (MO_PID_Kp == 0xff)                  MO_PID_Kp = '0';
    if (MO_PID_Ki == 0xff)                  MO_PID_Ki = '0';
    if (MO_PID_Kd == 0xff)                  MO_PID_Kd = '0';

    MO_TotalIsoCycleCounter      = readUint32FromFlash(ADDR_TotalIsoCycleCounter);
    MO_TotalCntlCycleCounter     = readUint32FromFlash(ADDR_TotalCntlCycleCounter);
    MO_PFOCycleCounter           = readUint32FromFlash(ADDR_PFOCycleCounter);

    if (MO_TotalIsoCycleCounter  == UINT32_MAX)  MO_TotalIsoCycleCounter = 0;
    if (MO_TotalCntlCycleCounter == UINT32_MAX)  MO_TotalCntlCycleCounter = 0;
    if (MO_PFOCycleCounter == UINT32_MAX)        MO_PFOCycleCounter = 0;

    if (MO_ValveType != ValveType_Pendulum && MO_ValveType != ValveType_Butterfly)      return false;

    if (MO_ValveFullScale != B080_MAX_M_STEP && MO_ValveFullScale != P250_MAX_M_STEP)   return false;

    tmc_CurrentMotorStep  = 0;
    tmc_SetpointMotorStep = 0;

    initTMC(MO_ValveType, MO_ValveSize);

    _valve_initialized = true;

    initPid();
    return _valve_initialized;
}

bool syncValveData(uint32_t addr)
{
    bool result = true;
    if (addr == _ADDR_MO_DATA) {
        result &= writeByteToFlash(ADDR_ValveType, MO_ValveType);
        result &= writeByteToFlash(ADDR_PositionCommRange, MO_PositionCommRange);
        result &= writeUint16ToFlash(ADDR_ValveSize, MO_ValveSize);
        result &= writeUint32ToFlash(ADDR_ValveFullScale, MO_ValveFullScale);
        uint8_t arr[] = {
            MO_ValvePosAfterPowerUp,
            MO_ValvePosAfterPowerFail,
            MO_ValvePosAfterNetwFail,
            MO_ExtIsolationValveFunc,
            MO_CntlStrokeLimit,
            MO_ValvePosAfterSync,
            MO_NetwFailEndPosition,
            MO_SlaveOfflinePosition,
            MO_SynchronizationStart,
            MO_SynchronizationMode,
            MO_PID_Kp,
            MO_PID_Ki,
            MO_PID_Kd
        };
        result &= writeFlash(ADDR_ValvePosAfterPowerUp, arr, (uint32_t)sizeof(arr));
    }
    return result;
}

bool syncValveCount()
{
    bool result = eraseFlash(0x080C0000, 128*1024);

    APC_Delay(10);

    if(result == true){
        result &= writeByteToFlash(ADDR_Sensor1Selection, getSensor1Selection());
        result &= writeByteToFlash(ADDR_Sensor2Selection, getSensor2Selection());
        result &= writeIntToFlash(ADDR_Sensor1Offset, getSensor1Offset());
        result &= writeIntToFlash(ADDR_Sensor2Offset, getSensor2Offset());

        result &= writeUint32ToFlash(ADDR_TotalIsoCycleCounter,  MO_TotalIsoCycleCounter);
        result &= writeUint32ToFlash(ADDR_TotalCntlCycleCounter, MO_TotalCntlCycleCounter);
        result &= writeUint32ToFlash(ADDR_PFOCycleCounter, MO_PFOCycleCounter);
    }

    return result;
}

uint32_t getCurrentIsoCycleCounter(void)
{
    return _CurrentIsoCycleCounter;
}

bool setCurrentIsoCycleCounter(uint32_t counter)
{
    if (_CurrentIsoCycleCounter != counter) {
        _CurrentIsoCycleCounter = counter;
        //setCountSync(ADDR_TotalIsoCycleCounter, 4);
    }
    return true;
}

uint32_t getTotalIsoCycleCounter(void)
{
    return MO_TotalIsoCycleCounter;
}
bool setTotalIsoCycleCounter(uint32_t counter)
{
    if (MO_TotalIsoCycleCounter != counter) {
        MO_TotalIsoCycleCounter = counter;

        // TODO
        //setCountSync(ADDR_TotalIsoCycleCounter, 4);
    }
    return true;
}

void addIsolationCycleCounter(void)
{
    _CurrentIsoCycleCounter++;
    MO_TotalIsoCycleCounter++;
    //setCountSync(ADDR_TotalIsoCycleCounter, 4);
    //bool ret = writeUint32ToFlash(ADDR_TotalIsoCycleCounter, MO_TotalIsoCycleCounter);
}

uint32_t getCurrentCntlCycleCounter(void)
{
    return _CurrentCntlCycleCounter;
}

bool setCurrentCntlCycleCounter(uint32_t counter)
{
    if (_CurrentCntlCycleCounter != counter) {
        _CurrentCntlCycleCounter = counter;
        //setCountSync(ADDR_TotalCntlCycleCounter, 4);
    }
    return true;
}

uint32_t getTotalCntlCycleCounter(void)
{
    return MO_TotalCntlCycleCounter;
}

bool setTotalCntlCycleCounter(uint32_t counter)
{
    if (MO_TotalCntlCycleCounter != counter) {
        MO_TotalCntlCycleCounter = counter;

        // TODO
        //setCountSync(ADDR_TotalCntlCycleCounter, 4);
    }
    return true;
}

void addControlCycleCounter(void)
{
    _CurrentCntlCycleCounter++;
    MO_TotalCntlCycleCounter++;
    //setCountSync(ADDR_TotalCntlCycleCounter, 4);
//    _count_flag = true;
/*    bool ret = writeUint32ToFlash(ADDR_TotalCntlCycleCounter, MO_TotalCntlCycleCounter);
    if(ret)
        return;*/
}

uint32_t getPFOCycleCounter(void)
{
    return MO_PFOCycleCounter;      // _PFOCycleCounter;
}

void setPFOCycleCounter(uint32_t counter)
{
    MO_PFOCycleCounter = counter;
    //_PFOCycleCounter = counter;
}

void addPFOCycleCounter()
{
    MO_PFOCycleCounter++;
}

void addPowerUpCounter()
{
    _CurrentPowerUpCounter++;
}

uint32_t getCurrentPowerUpCounter()
{
    return _CurrentPowerUpCounter;
}

void setCurrentPowerUpCounter(uint32_t counter)
{
    _CurrentPowerUpCounter = counter;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t getPID_Kp()
{
    return MO_PID_Kp;
}

uint8_t getPID_Ki()
{
    return MO_PID_Ki;
}

uint8_t getPID_Kd()
{
    return MO_PID_Kd;
}

/**
 * XXX  MO_ExtIsolationValveFunc 상태에 따라 setSolenoidStatus() 호출 여부
 */
void setValveOpen(void)
{
    //if (MO_ExtIsolationValveFunc == ExtIsolationValveFunc_Yes)   setSolenoidStatus(APC_HIGH);
    if (MO_ValveType == ValveType_Pendulum)     setSolenoidStatus(APC_HIGH);

    setSetpointValvePosition(VALVE_POSITION_OPEN);
    for (int i = 0; i < 10; i++) {
        APC_Delay(100);
        int current_position = getCurrentValvePosition();
        setDisplayCodeNumber(getControlMode(), (int) (current_position / PERMILL));
        //uint32_t gap = (VALVE_POSITION_OPEN - current_position) / PERMILL;
        //if (gap <= 10) break;
        if(getMotorStatus()&0x05)   break;
    }

    // TODO encoder
    //setEncoderPosition(getMotorPosition());
    checkValveCondition();
}

/**
 * XXX  MO_ExtIsolationValveFunc 상태에 따라 setSolenoidStatus() 호출 여부
 * Before called, user have to change the control mode into CLOSED/INTERLOCK_CLOSED.
 */
void setValveClosed(void)
{
    setSetpointValvePosition(VALVE_POSITION_CLOSE);

    for (int i = 0; i < 10; i++) {
        APC_Delay(200);
        int current_position = getCurrentValvePosition();
        setDisplayCodeNumber(getControlMode(), (int) (current_position / PERMILL));
        //uint32_t gap = (current_position - VALVE_POSITION_CLOSE) / PERMILL;
        //if (gap <= 10) break;
        if(getMotorStatus()&0x05)   break;
    }

    // TODO encoder
    //setEncoderPosition(getMotorPosition());
    checkValveCondition();
    if (MO_ValveType == ValveType_Pendulum && getControlMode() == ControlMode_CLOSED) {
        if (MO_ExtIsolationValveFunc == ExtIsolationValveFunc_Yes)   setSolenoidStatus(APC_LOW);
    }
}

EN_PositionCommRange getPositionCommRange(void)
{
    return MO_PositionCommRange;
}

void setPositionCommRange(EN_PositionCommRange range)
{
    MO_PositionCommRange = range;
}

uint16_t getValveSize(void)
{
    return MO_ValveSize;
}

void setValveSize(uint16_t size)
{
    MO_ValveSize = size;
}

/*
 * motor set position을 반환하지 않고 user 설정에 의한 값을 반환
 */
uint32_t getSetpointValvePosition(void)
{
    return _ValveSetpointPosition;
}

uint32_t getSetPosition(void)
{
    return tmc_SetpointMotorStep;       //_ValveSetpointPosition * (MO_ValveFullScale / PERCENT) / PERMILL;
}

/**
 * User requested position (ex: 0 ~ 100,000)
 * absolute_pos : Abs. position in % value * 1000 to move
 */
void setSetpointValvePosition(uint32_t absolute_pos)
{
    if (absolute_pos >= VALVE_POSITION_CLOSE && absolute_pos <= VALVE_POSITION_OPEN)
    {
        tmc_SetpointMotorStep = absolute_pos * (MO_ValveFullScale / PERCENT) / PERMILL;
        setMotorTargetPosition(tmc_SetpointMotorStep, APC_HIGH);

        // 2024-02-13, refresh 'tmc_CurrentMotorStep'
        //_getCurrentValvePosition();
        _ValveSetpointPosition = absolute_pos;
        addCycleCounter(absolute_pos);
        if(getControlMode() == ControlMode_Synchronization)
            return;
        for (int i = 0; i < 10; i++) {
            APC_Delay(10);
            setDisplayCodeNumber(getControlMode(), (int) (getCurrentValvePosition() / PERMILL));
            if(getMotorStatus()&0x05)   break;
        }
        setDisplayCodeNumber(getControlMode(), (int) (getCurrentValvePosition() / PERMILL));
    }
}

/**
 * called from external process
 */
int getCurrentValvePosition(void)
{
/*    int val_pos = getMotorPosition();

    if(val_pos > 20000){
        _CurrentValvePosition = val_pos * PERMILL / MO_ValveFullScale * PERCENT;       // Modify calculation order. by JWY
    }
    else{
        _CurrentValvePosition = val_pos * PERMILL * PERCENT / (int)MO_ValveFullScale;
    }*/

    return _CurrentValvePosition;

/*    int val_pos = getEncoderPosition();
    int valvepos = 0;

        if(val_pos > 20000){
            valvepos = val_pos * PERMILL / MO_ValveFullScale * PERCENT;       // Modify calculation order. by JWY
        }
        else{
            valvepos = val_pos * PERMILL * PERCENT / (int)MO_ValveFullScale;
        }

    return valvepos;*/
}

int getCurrentEncoder(void)
{
   return _CurrentEncoderPos;
}

/**
 * Return current valve position. For internal use only.
 * - normal state: 0 ~ [1000, 10000, 100000]
 * - abnormal state:  < 0
 */
void _getCurrentValvePosition(void)
{
    int val_pos = getMotorPosition();
    int enc_pos = getEncoderPosition();
    int gap_pos = val_pos > enc_pos ? val_pos - enc_pos : enc_pos - val_pos;

//    if (gap_pos > 1000) return -1000;

//    tmc_CurrentMotorStep = val_pos; // enc_pos;

    if(val_pos > 20000){
        _CurrentValvePosition = val_pos * PERMILL / (int)MO_ValveFullScale * PERCENT;       // Modify calculation order. by JWY
        _CurrentEncoderPos = enc_pos * PERMILL / (int)MO_ValveFullScale * PERCENT;
    }
    else{
        _CurrentValvePosition = val_pos * PERMILL * PERCENT / (int)MO_ValveFullScale;
        _CurrentEncoderPos = enc_pos * PERMILL * PERCENT / (int)MO_ValveFullScale;
    }

    return;
}

/**
 * PID
 * : ValvePosition 기준으로 변경
 * : 요청된 값이 임계치를 초과하는 경우, clamping 처리 (2024-01-28)
 * relative_pos : Rel. position in % value to move
 */
void setCurrentValvePosition(int relative_pos)
{
    int absolute_pos = getCurrentValvePosition() + relative_pos;

    if      (absolute_pos < (int)VALVE_POSITION_CLOSE)   absolute_pos = VALVE_POSITION_CLOSE;
    else if (absolute_pos > (int)VALVE_POSITION_OPEN)    absolute_pos = VALVE_POSITION_OPEN;

    tmc_SetpointMotorStep = absolute_pos * (MO_ValveFullScale / PERCENT) / PERMILL;
    setMotorTargetPosition(tmc_SetpointMotorStep, APC_HIGH);

    addCycleCounter(absolute_pos);
}

/**
 * PID
 * : 현재 값을 갱신하지 않고 요청된 absolute_step을 그대로 반영
 * : 요청된 값이 임계치를 초과하는 경우, clamping 처리
 */
void setCurrentValvePosUsingAbsoluteStep(int absolute_step)
{
    // XXX Be careful, when comparing signed and unsigned
    if (absolute_step > (int)MO_ValveFullScale) tmc_SetpointMotorStep = MO_ValveFullScale;
    else if (absolute_step < 0)                 tmc_SetpointMotorStep = 0;
    else                                        tmc_SetpointMotorStep = absolute_step;
#if 0
    printLocalRS232(APC_PORT_LOCAL, "[DBG] absolute_step[=%d], tmc_CurrentMotorStep[=%d], tmc_OffsetClosed[=%d]\r\n",
            absolute_step, tmc_SetpointMotorStep, tmc_OffsetClosed);
#endif
    setMotorTargetPosition(tmc_SetpointMotorStep, APC_HIGH);

    addCycleCounter(tmc_SetpointMotorStep * PERMILL / (MO_ValveFullScale / PERCENT));
}

/**
 * PID
 * : 현재 값을 갱신하지 않고 요청된 relative_step을 tmc_CurrentMotorStep과 연산하여 반영
 */
void setCurrentValvePosUsingRelativeStep(int relative_step)
{
    int absolute_step = tmc_CurrentMotorStep + relative_step;
    setCurrentValvePosUsingAbsoluteStep(absolute_step);
}

uint32_t getValveMaxSpeed(void) {
    return VALVE_MAX_SPEED;
}

/*
 * Motor:
 * - VMAX_Ratio: 0x4C4B40 / 1000
 * - AMAX_Ratio:   0x61A8 / 25
 */
void setValveMaxSpeed(uint32_t speed)
{
    setMotorPosVelocity(speed);
    setMotorPosAcceleration(speed);
}

uint32_t getValveSpeed(void)
{
    //uint32_t speed = getMotorVelocity();
    //_ValveSpeed = getValveMaxSpeed();
    return _ValveSpeed;
}

void setValveSpeed(uint32_t speed)
{
    uint32_t velocity, acc;
    if(speed >= 1000){
        velocity = getMotorMaxVelocity();
        acc = getMotorMaxAcc();
    }
    else if(speed < 10){
        velocity = getMotorMaxVelocity()/1000 *10;
        acc = getMotorMaxAcc()/3000 *10;
    }
    else{
        velocity = getMotorMaxVelocity()/1000 *speed;
        acc = getMotorMaxAcc()/3000 * speed / 2;
    }

    setMotorPosVelocity(velocity);
    setMotorPosAcceleration(acc);
    _ValveSpeed = speed;
}

uint8_t getValvePosAfterPowerUp()
{
    return MO_ValvePosAfterPowerUp;
}

void setValvePosAfterPowerUp(uint8_t option)
{
    MO_ValvePosAfterPowerUp = option;
}

uint8_t getValvePosAfterPowerFail()
{
    return MO_ValvePosAfterPowerFail;
}
void setValvePosAfterPowerFail(uint8_t option)
{
    MO_ValvePosAfterPowerFail = option;
}

uint8_t getValvePosAfterNetwFail()
{
    return MO_ValvePosAfterNetwFail;
}
void setValvePosAfterNetwFail(uint8_t option)
{
    MO_ValvePosAfterNetwFail = option;
}

uint8_t getExtIsolationValveFunc()
{
    return MO_ExtIsolationValveFunc;
}
void setExtIsolationValveFunc(uint8_t option)
{
    MO_ExtIsolationValveFunc = option;
}

uint8_t getCntlStrokeLimit()
{
    return MO_CntlStrokeLimit;
}
void setCntlStrokeLimit(uint8_t option)
{
    MO_CntlStrokeLimit = option;
}

uint8_t getValvePositionAfterSync()
{
    return MO_ValvePosAfterSync;
}
void setValvePositionAfterSync(uint8_t option)
{
    MO_ValvePosAfterSync = option;
}

uint8_t getNetwFailEndPosition()
{
    return MO_NetwFailEndPosition;
}
void setNetwFailEndPosition(uint8_t option)
{
    MO_NetwFailEndPosition = option;
}

uint8_t getSlaveOfflinePosition()
{
    return MO_SlaveOfflinePosition;
}
void setSlaveOfflinePosition(uint8_t option)
{
    MO_SlaveOfflinePosition = option;
}

uint8_t getSynchronizationStart()
{
    return MO_SynchronizationStart;
}
void setSynchronizationStart(uint8_t option)
{
    MO_SynchronizationStart = option;
}

uint8_t getSynchronizationMode()
{
    return MO_SynchronizationMode;
}
void setSynchronizationMode(uint8_t option)
{
    MO_SynchronizationMode = option;
}


void stopSynchronizatioin()
{
    _stop_sync = true;
}

/**
 * XXX  MO_ExtIsolationValveFunc 상태에 따라 setSolenoidStatus() 호출 여부
 * Power-up 이후에 첫 진행
 */
int synchronizeClosedInit(uint32_t initial_step)
{
    int m_step = (MO_ValveFullScale / PERCENT) / 10;
    int motor_offset = 0;
    int encoder_offset = 0;
    uint8_t m_fail_count = 0;
    uint8_t e_fail_count = 0;

    //if (MO_ExtIsolationValveFunc == ExtIsolationValveFunc_Yes)   setSolenoidStatus(APC_HIGH);
    if (MO_ValveType == ValveType_Pendulum)     setSolenoidStatus(APC_HIGH);

    int MinEncGap = (MO_ValveType == ValveType_Pendulum) ? (m_step / 2) : (m_step / 2);

    int i = 0;
    for ( ; ; i++) {
        while(!(getMotorStatus() & 0x05))   APC_Delay(5);       // Add waiting to stop valve. by JWY

        APC_Delay(50);
        int motor_pos = getMotorPosition();     // x_actual
        int encoder_pos = getEncoderPosition(); // enc_x

        if (i == 0) {
            motor_offset = motor_pos;
            encoder_offset = encoder_pos;

            setMotorTargetPosition(motor_offset - m_step, APC_HIGH);
            APC_Delay(50);
            //printLocalRS232(APC_PORT_LOCAL, "[DBG] Position[%d, cur=%d, new=%d]\r\n",
            //        i, motor_offset, (motor_offset - m_step));
        }
        else {
            int motor_gap = motor_offset > motor_pos ? motor_offset - motor_pos : 0;
            int encoder_gap = encoder_offset > encoder_pos ? encoder_offset - encoder_pos : 0;
            // DN250 m_step = 50
            if (motor_gap < m_step)      m_fail_count++;
            if (encoder_gap < MinEncGap) e_fail_count++;

            if (m_fail_count > 0 || e_fail_count > 0/*1*/) {
                // XXX check the encoder more than twice at least
                //motor_offset = motor_offset + m_step;   // rewind 2 step
                //syncEncoderAndMotor(motor_offset);

                //printLocalRS232(APC_PORT_LOCAL, "[DBG] cnt=%d, motor[=%d], encoder[=%d]\r\n",
                //        i, getMotorPosition(), getEncoderPosition());
                break;
            }
            else {
                motor_offset = motor_pos;
                encoder_offset = encoder_pos;

                setMotorTargetPosition(motor_offset - m_step, APC_HIGH);
                APC_Delay(50);
            }
        }

        /* protect the infinite moving */
        int mov_limit = initial_step < m_step ? MO_ValveFullScale / m_step : (initial_step / m_step + 5);
        if (i > mov_limit * 1.2) {
            return E20_HomingError;
        }

        if (_stop_sync == true) {
            return -1;
        }

        //APC_Delay(200);
    }

    //tmc_OffsetClosed = motor_offset;    // offset
    //printLocalRS232(APC_PORT_LOCAL, "[DBG] cnt=%d, tmc_OffsetClosed[=%d]\r\n", i, tmc_OffsetClosed);

    /* 영점조절
     * 1. ramp_mode 변경 -> reserved 모드 변경( +0x01)
     * 2. x_actual, enc_x, x_target = 0 로 변경
     * 3. ramp_mode 변경 -> position 모드 변경 ( -0x01)
     */
//    tmc_OffsetClosed = 0;
//
//    uint8_t org_ramp_mode = getRampMode();
//    setRampMode(TMC457_RM_RESERVED);
//    uartPrintf(APC_PORT_LOCAL, "[DBG] RampMode[org=%d, new=%d]\r\n",
//            org_ramp_mode, getRampMode());
//
//    setMotorPosition(tmc_OffsetClosed);
//    setEncoderPosition(tmc_OffsetClosed);
//    setMotorTargetPosition(tmc_OffsetClosed, APC_HIGH);
//    setRampMode(org_ramp_mode);
//
//    uartPrintf(APC_PORT_LOCAL, "[DBG] x_actual=%d, enc_x=%d, x_target=%d\r\n",
//            getMotorPosition(), getEncoderPosition(), getMotorTargetPosition());

    // XXX 2024-05-07 SOL delay 때문에 사용하지 않음.
    //if (MO_ExtIsolationValveFunc == ExtIsolationValveFunc_Yes)   setSolenoidStatus(APC_LOW);

    return 0;
}

/**
 * XXX  MO_ExtIsolationValveFunc 상태에 따라 setSolenoidStatus() 호출 여부
 */
int synchronizeOpenInit(uint32_t initial_step)
{
    int m_step = (MO_ValveFullScale / PERCENT) / 10;
    int motor_offset = 0;
    int encoder_offset = 0;
    uint8_t m_fail_count = 0;
    uint8_t e_fail_count = 0;

    //if (MO_ExtIsolationValveFunc == ExtIsolationValveFunc_Yes)   setSolenoidStatus(APC_HIGH);
    if (MO_ValveType == ValveType_Pendulum)     setSolenoidStatus(APC_HIGH);

    int MinEncGap = (MO_ValveType == ValveType_Pendulum) ? (m_step / 2) : (m_step / 2);

    int i = 0;
    for ( ; ; i++) {
        while(!(getMotorStatus() & 0x05))   APC_Delay(5);       // Add waiting to stop valve. by JWY

        APC_Delay(50);
        int motor_pos = getMotorPosition();
        int encoder_pos = getEncoderPosition();

        if (i == 0) {
            motor_offset = motor_pos;
            encoder_offset = encoder_pos;

            setMotorTargetPosition(motor_offset + m_step, APC_HIGH);
            APC_Delay(50);
            //printLocalRS232(APC_PORT_LOCAL, "[DBG] Position[%d, cur=%d, new=%d]\r\n",
            //        i, motor_offset, (motor_offset + m_step));
        }
        else {
            int motor_gap = motor_pos > motor_offset ? motor_pos - motor_offset : 0;
            int encoder_gap = encoder_pos > encoder_offset ? encoder_pos - encoder_offset : 0;

            if (motor_gap < m_step)      m_fail_count++;
            //if (i > (m_count * 5 / PERCENT) && encoder_gap < (m_step / 2))
            if (encoder_gap < MinEncGap) e_fail_count++;

            if (m_fail_count > 0 || e_fail_count > 2) {
                motor_offset = motor_offset - m_step;   // rewind 2 step
                //syncEncoderAndMotor(motor_offset);

                //printLocalRS232(APC_PORT_LOCAL, "[DBG] cnt=%d, m_fail_count[=%d], e_fail_count[=%d]\r\n",
                //        i, m_fail_count, e_fail_count);
                //printLocalRS232(APC_PORT_LOCAL, "[DBG] Position[%d, revert to %d]\r\n",
                //        i, motor_offset);
                break;
            } else {
                motor_offset = motor_pos;
                encoder_offset = encoder_pos;

                setMotorTargetPosition(motor_offset + m_step, APC_HIGH);
                APC_Delay(50);
                //uartPrintf(APC_PORT_LOCAL, "[DBG] Position[%d, cur=%d, new=%d]\r\n",
                //        i, motor_offset, (motor_offset + m_step));
            }
        }

        /*
         * XXX protect the infinite moving
         */
        int mov_limit = (MO_ValveFullScale / m_step) - (initial_step / m_step) + 1;
        if (i > mov_limit) {
            //last_count = i;
            break;
        }

        if (_stop_sync == true) {
            return -1;
        }

        //APC_Delay(200);
    }

//    tmc_OffsetOpen = motor_offset;  // offset
//    printLocalRS232(APC_PORT_LOCAL, "[DBG] cnt=%d, tmc_OffsetOpen[=%d]\r\n", i, tmc_OffsetOpen);

    //while(!(getMotorStatus() & 0x05))   APC_Delay(5);       // Add waiting to stop valve. by JWY
    //printLocalRS232(APC_PORT_LOCAL, "[DBG] x_actual=%d, enc_x=%d, x_target=%d\r\n",
    //        getMotorPosition(), getEncoderPosition(), getMotorTargetPosition());
    return 0;
}

/**
 * XXX  MO_ExtIsolationValveFunc 상태에 따라 setSolenoidStatus() 호출 여부
 */
int synchronizeOpenStepByStep()
{
    int first_step = (MO_ValveFullScale / PERCENT) * 90;
    int m_step = (MO_ValveFullScale / PERCENT) / 5;
    int motor_offset = 0;
    int encoder_offset = 0;
    uint8_t m_fail_count = 0;
    uint8_t e_fail_count = 0;

    //if (MO_ExtIsolationValveFunc == ExtIsolationValveFunc_Yes)   setSolenoidStatus(APC_HIGH);
    if (MO_ValveType == ValveType_Pendulum)     setSolenoidStatus(APC_HIGH);

    int MinEncGap = (MO_ValveType == ValveType_Pendulum) ? (m_step / 2) : (m_step / 2);

    int i = 0;
    for ( ; ; i++) {
        int motor_pos = getMotorPosition();
        int encoder_pos = getEncoderPosition();

        if (i == 0) {
            motor_offset = motor_pos;
            encoder_offset = encoder_pos;

            setMotorTargetPosition(motor_offset + first_step, APC_HIGH);
            printLocalRS232(APC_PORT_LOCAL, "[DBG] Position[%d, cur=%d, new=%d]\r\n",
                    i, motor_offset, (motor_offset + first_step));
            APC_Delay(3000);
        }
        else {
            int motor_gap = motor_pos > motor_offset ? motor_pos - motor_offset : 0;
            int encoder_gap = encoder_pos > encoder_offset ? encoder_pos - encoder_offset : 0;

            if (motor_gap < m_step)      m_fail_count++;
            //if (i > (m_count * 5 / PERCENT) && encoder_gap < (m_step / 2))
            if (encoder_gap < MinEncGap) e_fail_count++;

            if (m_fail_count > 0 || e_fail_count > 2) {
                //syncEncoderAndMotor(motor_offset);

                printLocalRS232(APC_PORT_LOCAL, "[DBG] m_fail_count[=%d], e_fail_count[=%d]\r\n",
                        m_fail_count, e_fail_count);
                printLocalRS232(APC_PORT_LOCAL, "[DBG] Position[%d, revert to %d]\r\n",
                        i, motor_offset);
                break;
            } else {
                motor_offset = motor_pos;
                encoder_offset = encoder_pos;

                setMotorTargetPosition(motor_offset + m_step, APC_HIGH);
                //uartPrintf(APC_PORT_LOCAL, "[DBG] Position[%d, cur=%d, new=%d]\r\n",
                //        i, motor_offset, (motor_offset + m_step));
            }
        }

        /*
         * XXX protect the infinite moving
         */
        if (i >= (MO_ValveFullScale/m_step - first_step/m_step)) {
            //last_count = i;
            break;
        }

        if (_stop_sync == true) {
            return -1;
        }

        APC_Delay(200);
    }

//    tmc_OffsetOpen = motor_offset;  // offset
//    printLocalRS232(APC_PORT_LOCAL, "[DBG] cnt=%d, tmc_OffsetOpen[=%d]\r\n", i, tmc_OffsetOpen);

    printLocalRS232(APC_PORT_LOCAL, "[DBG] x_actual=%d, enc_x=%d, x_target=%d\r\n",
            getMotorPosition(), getEncoderPosition(), getMotorTargetPosition());
    return 0;
}

int getSynchronizationResult(void)
{
    // XXX more than 2%
    if (MO_ValveFullScale > getEncoderPosition()) {
        return E21_RestrictedFullStroke_Synch;
    }
    return 0;
}

uint8_t getValveMotorStatus()
{
    return getMotorStatus();
}

int getValveEncoderPosition()
{
    return getEncoderPosition();
}

uint32_t getValveFullStroke(void)
{
    return MO_ValveFullScale;
}
void setValveFullStroke(uint32_t vfs)
{
    MO_ValveFullScale = vfs;
}

void setValveType(uint8_t type)
{
    MO_ValveType = type;
}

uint8_t getValveType(void)
{
    return MO_ValveType;
}

void checkValveCondition(void)
{
    int m_pos = getMotorPosition();
    int e_pos = getEncoderPosition();

    int gap = m_pos >= e_pos ? m_pos - e_pos : e_pos - m_pos;
    if (gap > MO_ValveFullScale * 0.1)
        setWarningCode(Warning_ServiceRequest);
}

void addCycleCounter(int absolute_pos)
{
    if (absolute_pos == VALVE_POSITION_CLOSE) {
        int control_mode = getControlMode();
        if (control_mode == ControlMode_CLOSED || control_mode == ControlMode_INTERLOCK_CLOSED) {
            if (MO_ExtIsolationValveFunc == ExtIsolationValveFunc_Yes)
                addIsolationCycleCounter();
            else
                addControlCycleCounter();
        }
        else {
            addControlCycleCounter();
        }
    }
}

void syncEncoderAndMotor(int offset)
{
    setMotorVelocity(0);
    APC_Delay(10);
    setRampMode(TMC457_RM_VELOCITY);
    APC_Delay(10);
    setMotorPosition(offset);
    APC_Delay(10);
    setMotorTargetPosition(offset, APC_HIGH);
    APC_Delay(10);
    setEncoderPosition(offset);
    APC_Delay(10);
    setRampMode(TMC457_RM_POSITION);
}

/*
 * APC_Synch.c
 *
 *  Created on: 2023. 10. 23.
 *      Author: Yongseok
 */

#include <APC_AccessMode.h>
#include <APC_ControlMode.h>
#include "APC_Utils.h"
#include "APC_Error.h"
#include <APC_Warning.h>
#include <APC_LocalPort.h>
#include <APC_Synch.h>
#include "APC_CompAir.h"
#include "APC_Display.h"
#include "APC_Valve.h"

#define SYNC_ERR_FORMAT "[SYNC] E:%06d\r\n"

static bool _synch_status  = false;

/**
 * Synchronization mode (1)
 */
void initSynch(void)
{
    uint8_t val;

    if (getValveType() == ValveType_Pendulum)     setSolenoidStatus(APC_HIGH);
    APC_Delay(2000);

    val = procSynch(1);
    while(val == 1){
        APC_Delay(1000);
        val = procSynch(1);
    }
}

/*
 * return value : 0(OK), 1(Air Fail), 2(Synch Error)
 */
uint8_t procSynch(uint8_t mode)
{
    _synch_status = false;

//    uint8_t airStatus = checkCompAirStatus(APC_PORT_INTERNAL, true);
    //if (getCompAirStatus() /*airStatus*/ != 0) {
    //    setWarningCode(Warning_CompressedAir);

        //initDisplay();
        //setDisplayCodeNumber(getControlMode(), getFatalErrorCode());
    //    setDisplayData("AIRx");
    //    return 1;
    //}

    setControlMode(APC_PORT_INTERNAL, ControlMode_Synchronization);
    initDisplay();
    setDisplayData("SYNC");

    //setPIDMode(false);
    /*
     * 동기화 수행: close-open-close 수행하면서 limit 측정
     */
    // XXX wait user action(=option)
    //uint32_t user_action = getSynchronizationStart();

    // XXX when first trial, MUST have a careful approach
    setValveSpeed(1000);
    //setSynchCondition(true);
    if(mode)
        printLocalRS232(APC_PORT_LOCAL, "[DBG] Set Speed and Start Synch Close\r\n");
    int sync_result = synchronizeClosedInit(0);
    if (sync_result > 0) {
        setControlMode(APC_PORT_INTERNAL, ControlMode_FatalError);
        setDisplayCodeNumber(ControlMode_FatalError, sync_result);
        if(mode)
            printLocalRS232(APC_PORT_LOCAL, SYNC_ERR_FORMAT, sync_result);
        //setSynchCondition(false);
        setValveSpeed(1000);
        //setPIDMode(true);
        return 2;
    }
    else{
        APC_Delay(100);
        while(!(getMotorStatus() & 0x05))   APC_Delay(5);
        APC_Delay(200);
        switch(getValveType()){
            case 1:         // Pendulum Valve
                syncEncoderAndMotor(-400);
                break;
            case 2:         // Butterfly Valve 80 Size
                syncEncoderAndMotor(-400);
                break;
            case 3:         // Butterfly Valve 100 Size
                syncEncoderAndMotor(-1800);
                break;
        }
        APC_Delay(100);
        if(mode)
            printLocalRS232(APC_PORT_LOCAL, "[DBG] Synch close Set Encoder : motor[=%d], encoder[=%d]\r\n",
                    getMotorPosition(), getValveEncoderPosition());
    }

    APC_Delay(300);

    sync_result = synchronizeOpen(mode);
    if (sync_result > 0) {
        setControlMode(APC_PORT_INTERNAL, ControlMode_FatalError);
        setDisplayCodeNumber(ControlMode_FatalError, sync_result);
        if(mode)
            printLocalRS232(APC_PORT_LOCAL, SYNC_ERR_FORMAT, sync_result);
        //setSynchCondition(false);
        setValveSpeed(1000);
        //setPIDMode(true);
        return 2;
    }
    else{
        APC_Delay(100);
        syncEncoderAndMotor(getValveEncoderPosition());
        APC_Delay(100);
    }

    sync_result = getSynchronizationResult();
    if (sync_result > 0) {
        setControlMode(APC_PORT_INTERNAL, ControlMode_FatalError);
        setDisplayCodeNumber(ControlMode_FatalError, sync_result);
        if(mode)
            printLocalRS232(APC_PORT_LOCAL, SYNC_ERR_FORMAT, sync_result);
        //setSynchCondition(false);
        setValveSpeed(1000);
        //setPIDMode(true);
        return 2;
    }

    //setValveSpeed(1000);
    //setSynchCondition(false);
    APC_Delay(1000);

    /*
     * 파라미터 확인: ValvePosAfterPowerUp 값 확인 (기본값: Closed)
     */
    uint32_t option = getValvePosAfterPowerUp();
    /*
     * Controller mode 천이: Closed / Open 상태로 이동
     */
    if (option == ValvePosAfterPowerUp_Open)
    {
        setControlMode(APC_PORT_INTERNAL, ControlMode_POSITION);
        setValveOpen();
    }
    else    // ValvePosAfterPowerUp_Open
    {
        setControlMode(APC_PORT_INTERNAL, ControlMode_POSITION);
        setValveClosed();
    }

    APC_Delay(100);
    while(!(getMotorStatus() & 0x05))   APC_Delay(5);

    if(mode)
        printLocalRS232(APC_PORT_LOCAL, "[DBG] Synch close : motor[=%d], encoder[=%d]\r\n",
                getMotorPosition(), getValveEncoderPosition());

    /*
     * Access mode: Local 모드, 일정 시간동안 입력이 없는 경우에 Remote 모드로 변경
     */
    //setAccessMode(AccessMode_Remote);
    // 스케줄러에서 변경

    _synch_status = true;
    //setPIDMode(true);

//    _getCurrentValvePosition();
//    writeAOUT_POSITION(getCurrentValvePosition());
    return 0;
}

bool getSynchStatus(void)
{
    return _synch_status;
}

/**
 * permitted only for LOCAL_PORT
 */
void syncronizeValve(uint8_t sync_mode)
{
    if(sync_mode == SynchMode_Full){
        procSynch(0);
    }
    else{
        if(!_synch_status && (sync_mode == SynchMode_Open)){
            return;
        }
        //setValveSpeed(800);

        if (sync_mode == SynchMode_Close) {
            int sync_result = synchronizeClosed();
            if (sync_result > 0) {
                setControlMode(APC_PORT_INTERNAL, ControlMode_FatalError);
                setDisplayCodeNumber(ControlMode_FatalError, sync_result);
                _synch_status = false;
                return;
            }
            else{
                APC_Delay(100);
                while(!(getMotorStatus() & 0x05))   APC_Delay(5);
                switch(getValveType()){
                    case 1:         // Pendulum Valve
                        syncEncoderAndMotor(-80);
                        break;
                    case 2:         // Butterfly Valve 80 Size
                        syncEncoderAndMotor(-400);
                        break;
                    case 3:         // Butterfly Valve 100 Size
                        syncEncoderAndMotor(-1800);
                        break;
                    }
                APC_Delay(100);
            }
            setValveSpeed(1000);
            setControlMode(APC_PORT_LOCAL, ControlMode_POSITION);
            setValveClosed();
        }
        else if(sync_mode == SynchMode_Open){
            int sync_result = synchronizeOpen(0);
            if(sync_result>0){
                setControlMode(APC_PORT_INTERNAL, ControlMode_FatalError);
                setDisplayCodeNumber(ControlMode_FatalError, sync_result);
                _synch_status = false;
                return;
            }
            setValveSpeed(1000);
            setControlMode(APC_PORT_LOCAL, ControlMode_POSITION);
            setValveOpen();
        }
        APC_Delay(100);
        while(!(getMotorStatus() & 0x05))   APC_Delay(5);
    }
    setValveSpeed(1000);
}

/**
 * Synchronization (runnig 상황에서 중간에 실행)
 * : Closed 지점을 확인하여 tmc_OffsetClosed 지정
 */
int synchronizeClosed()
{
    setSetpointValvePosition(VALVE_POSITION_OPEN * 0.1);
    APC_Delay(100);
    while(!(getMotorStatus()&0x05))     APC_Delay(5);
    return synchronizeClosedInit(0);
}

/**
 * Synchronization
 * : Open 지점을 확인하여 tmc_OffsetOpen 지정, Open 지점이 MAX_M_STEP에 미달한 경우에 Error
 */
int synchronizeOpen(uint8_t mode)
{
    setSetpointValvePosition(90000);

    APC_Delay(100);
    while(!(getMotorStatus()&0x05))     APC_Delay(5);
    if(mode)
        printLocalRS232(APC_PORT_LOCAL, "[DBG] SYNC-Move to Open Positon : motor[=%d], encoder[=%d]\r\n",
            getMotorPosition(), getValveEncoderPosition());
    return synchronizeOpenInit((uint32_t) (getValveFullStroke() * 0.9));
}

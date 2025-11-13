/*
 * APC_AsyncHandler.c
 *
 *  Created on: Jan 30, 2024
 *      Author: Yongseok
 */

#include <APC_AsyncHandler.h>
#include "cmsis_os2.h"

#include "APC_Utils.h"
#include "APC_CmdHandler.h"
#include "APC_Flash.h"
#include "APC_Board.h"
#include "APC_Valve.h"
#include "APC_Sensor.h"
#include "APC_Controller.h"
#include "APC_Learn.h"
#include "APC_Synch.h"
#include "APC_Test.h"

static bool     _job_flag  = false;
static uint32_t _job_id    = 0x00;
static uint64_t _job_val   = 0x00;
static uint32_t _job_timer = 0x00;

static bool     _sync_flag = false;
static uint32_t _sync_addr = 0x00;
static uint32_t _sync_option = 0x00;

static bool     _count_flag = false;
static uint32_t _count_addr = 0x00;

static bool     _restart_flag   = false;
static uint32_t _restart_option = 0x00;


extern osMessageQueueId_t jobQueueHandle;

//void registerMQ(osMessageQueueId_t *jobQueueHandle)
//{
//    _jobQueueHandle = jobQueueHandle;
//}

void initJob()
{
    _job_flag  = false;
    _job_id    = 0x00;
    _job_val   = 0x00;
    _job_timer = 0x00;
}

bool isJobExisted()
{
    return _job_flag;
}

bool setJobData(uint32_t job_id, uint64_t job_val)
{
    bool res = true;
    if (_job_flag == false) {
        _job_flag  = true;
        _job_id    = job_id;
        _job_val   = job_val;
        _job_timer = 0x00;
    } else {
        res = false;
    }
    return res;
}

int putDataIntoMQ(uint32_t data)
{
    osStatus_t os_status = osMessageQueuePut(jobQueueHandle, &data, 0U, 0U);  // osWaitForever
    return (int) os_status;
}

bool setDataSync(uint32_t addr, uint32_t option)
{
    if (_sync_flag == false) {
        _sync_flag   = true;
        _sync_addr   = addr;
        _sync_option = option;
        return true;
    }
    return false;
}

bool setCountSync(uint32_t addr, uint32_t length)
{
    if(_count_flag == false){
        _count_flag = true;
        //_count_addr = addr;
        return true;
    }
    return false;
}

bool setRestartFw(uint32_t option)
{
    if (_restart_flag == false) {
        _restart_flag   = true;
        _restart_option = option;
        return true;
    }
    return false;
}

void procAsyncHandler(uint32_t count)
{
#if 0
    uint32_t count = osMessageQueueGetCount(jobQueueHandle);

    if (count > 0 && count <= 16) {
        uint32_t data;
        osStatus_t os_status = osMessageQueueGet(jobQueueHandle, &data, NULL, 0U);
        if (os_status == osOK) {
            switch (data)
            {
                case CMD_KEY_C_CLOSE_VALVE: setValveClosed();   break;
                case CMD_KEY_O_OPEN_VALVE:  setValveOpen();     break;
                default:    break;
            }
        }
    }
#endif

    if (_job_flag == true) {
        switch (_job_id)
        {
            case CMD_KEY_C_CLOSE_VALVE:
                setValveClosed();   break;
            case CMD_KEY_O_OPEN_VALVE:
                setValveOpen();     break;
            case CMD_KEY_R_SETPOINT_VALVE_POSITION:
                setSetpointValvePosition((uint32_t) _job_val);  break;
            case CMD_KEY_r00_SETPOINT_VALVE_POSITION:
                setCurrentValvePosUsingAbsoluteStep((uint32_t) _job_val);   break;
            case CMD_KEY_r01_SETPOINT_VALVE_POSITION:
                setCurrentValvePosUsingRelativeStep((uint32_t) _job_val);   break;
            case CMD_KEY_Y_SYNC_VALVE:
                syncronizeValve((uint8_t) _job_val);    break;
            case CMD_KEY_L_START_LEARN:
                startLearn();   break;
            default:    break;
        }

        APC_Delay(50);
        while(!(getMotorStatus()&0x05))     APC_Delay(5);
        initJob();
    }

    if (_sync_flag == true) {
        bool result = eraseFlash(_sync_addr, _sync_option);
        APC_Delay(10);

        if (result == true && _sync_option > 0) {
            syncBoardData(_sync_addr);
            syncValveData(_sync_addr);
            syncSensorData(_sync_addr);
            syncControllerData(_sync_addr);
            syncLearnData(_sync_addr);
            APC_Delay(5);
        }

       _sync_flag = false;
    }

    if(_count_flag == true){
        bool result = eraseFlash(0x080C0000, 128*1024);
        APC_Delay(10);

        if(result == true){
            syncValveCount();
            APC_Delay(5);
        }

        _count_flag = false;
    }

    if (_restart_flag == true) {
        //HAL_GPIO_WritePin(RES__GPIO_Port, RES__Pin, GPIO_PIN_SET);
        NVIC_SystemReset();

        _restart_flag = false;
    }
}

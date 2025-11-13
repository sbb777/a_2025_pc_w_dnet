/*
 * APC_Test.c
 *
 *  Created on: Mar 6, 2024
 *      Author: Yongseok
 */

#include <APC_Test.h>

#include <APC_AccessMode.h>
#include <APC_ControlMode.h>
#include <APC_Utils.h>
#include <APC_Valve.h>

/******************** Test Mode ***************************/
static uint32_t _test_time = 0;

static uint16_t _old_ratio = 0;
static uint16_t _req_ratio = 0;
static uint16_t _req_msec  = 0;
static uint16_t _req_count = 0;

static uint32_t _test_counter = 0;
/***********************************************************/

void runTest();
void endTest();

void initTest(void)
{
}

/**
 * call by asynch. task
 */
void procTest(uint64_t job_val)
{
    int ratio = (job_val >> 32) & 0xff;
    int msec  = (job_val >> 16) & 0xffff;
    int count = (job_val >>  0) & 0xffff;

    setTest((uint16_t) ratio, (uint16_t) msec, (uint16_t) count);

    for (uint32_t timer = 0; ; timer++)
    {
        if (_req_ratio == 0 || _req_msec == 0 || _req_count == 0) {
            endTest();
        }

        if (timer - _test_time >= _req_msec) {
            _test_time = timer;
            runTest(timer);
        }

        APC_Delay(10);
    }
}

void setTest(uint16_t ratio, uint16_t msec, uint16_t count)
{
    if (ratio > 0 && msec > 0 && count > 0) {
        setControlMode(APC_PORT_INTERNAL, ControlMode_POSITION);
    }

    _old_ratio = _req_ratio;

    _req_ratio = ratio;
    _req_msec  = msec / 10;
    _req_count = count;

    _test_counter = 0;
}

void runTest()
{
    if (_req_ratio > 0 && _req_msec > 0 && _req_count > 0) {    // RUN
        if (_test_counter >= _req_count) {
            // end-of-test
            if (_req_count != 0) {
                setTest(0, 0, 0);
            }
        } else {
            int valve_position = getCurrentValvePosition();
            int new_pos = valve_position + (_req_ratio * 1000);

            if (new_pos < VALVE_POSITION_CLOSE) {
                setSetpointValvePosition(VALVE_POSITION_CLOSE);
            }
            else if (new_pos >= VALVE_POSITION_CLOSE && new_pos <= VALVE_POSITION_OPEN) {
                setSetpointValvePosition(new_pos);
            }
            else {
                setValveClosed();
                _test_counter++;
            }
            APC_Delay(50);
            while(!(getMotorStatus()&0x05))     APC_Delay(5);
        }
    }
}

void endTest()
{
    setControlMode(APC_PORT_INTERNAL, ControlMode_POSITION);
    setSetpointValvePosition(VALVE_POSITION_CLOSE);
    APC_Delay(50);
    while(!(getMotorStatus()&0x05))     APC_Delay(5);
}

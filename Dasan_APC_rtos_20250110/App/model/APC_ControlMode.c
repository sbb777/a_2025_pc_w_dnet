/*
 * APC_ControlMode.c
 *
 *  Created on: Apr 11, 2024
 *      Author: Yongseok
 */

#include <APC_ControlMode.h>

#include <APC_Error.h>
#include <APC_Scheduler.h>
#include <APC_Synch.h>
#include <APC_Board.h>
#include <APC_Display.h>

static EN_A_ControlMode _ControlMode;
static int              _SimulDemonMode;

void checkRemoteLock(void);

/**
 * Initialization mode (0)
 */
bool initControlMode(void)
{
    _ControlMode = ControlMode_Initialized;

    /*
     * XXX 디스플레이
     * digit2: 1(LOGIC), 2(RS232), 3(RS232+analog_output)
     * digit3: 0(basic), 1(SPS), 2(PFO), 3(SPS+PFO)
     * digit4: 1(1sensor), 2(2sensor)
     */
    //initDisplay();
    setDisplayData("INIT");

    checkRemoteLock();

    return (_ControlMode >= ControlMode_INTERLOCK_OPEN) ? false : true;
}


/**
 * If true, return 0. Otherwise, return ErrorCode.
 */
uint8_t setControlMode(uint8_t ch, uint8_t req_mode)
{
    uint8_t result = Status_WrongControlMode;

    if (req_mode < ControlMode_Initialized || req_mode > ControlMode_Service)
        return result;


    if (ch == APC_PORT_REMOTE) {
        switch (req_mode) {
            case ControlMode_POSITION:
            case ControlMode_CLOSED:
            case ControlMode_OPEN:
            case ControlMode_PRESSURE:
            case ControlMode_HOLD:
            case ControlMode_Synchronization:
            //case ControlMode_LEARN:
                _ControlMode = req_mode;
                result = 0;
                break;
            default:
                break;
        }
    }
    else {
        switch (req_mode) {
            case ControlMode_Initialized:
            case ControlMode_Synchronization:
            case ControlMode_POSITION:
            case ControlMode_CLOSED:
            case ControlMode_OPEN:
            case ControlMode_PRESSURE:
            case ControlMode_HOLD:
            case ControlMode_LEARN:

            case ControlMode_INTERLOCK_OPEN:
            case ControlMode_INTERLOCK_CLOSED:

            case ControlMode_MaintenanceOpen:
            case ControlMode_MaintenanceClose:
            case ControlMode_PowerFailure:
            case ControlMode_SafetyMode:
            case ControlMode_FatalError:
            case ControlMode_Service:
                _ControlMode = req_mode;
                result = 0;
                break;
            default:
                break;
        }
    }

    return result;
}

uint8_t getControlMode(void)
{
    return _ControlMode;
}

void setSimulDemonMode(int mode)
{
    _SimulDemonMode = mode;
}

int  getSimulDemonMode(void)
{
    return _SimulDemonMode;
}

void checkRemoteLock(void)
{
    if (getInterfaceType() == InterfaceType_RS232)  scheduleRemotePort(1000);
}

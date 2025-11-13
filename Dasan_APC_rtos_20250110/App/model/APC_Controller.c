/*
 * APC_Controller.c
 *
 *  Created on: 2023. 11. 12.
 *      Author: Yongseok
 */

#include "APC_Controller.h"

#include "APC_Flash.h"

/*********************************************************************************/
// Board(128) + Valve(64) + Sensor(64)
#define ADDR_CONTROLLER_START       (_ADDR_MO_DATA + 128 + 64 + 64)

#define ADDR_ControllerType         (ADDR_CONTROLLER_START +  0)
#define ADDR_GainFactor             (ADDR_CONTROLLER_START +  4)
#define ADDR_SensorDelay            (ADDR_CONTROLLER_START +  8)
#define ADDR_RampTime               (ADDR_CONTROLLER_START + 12)    // not used

#define ADDR_Pgain                  (ADDR_CONTROLLER_START + 16)
#define ADDR_Igain                  (ADDR_CONTROLLER_START + 20)
#define ADDR_Dgain                  (ADDR_CONTROLLER_START + 24)

/*********************************************************************************/
static bool _controller_initialized = false;

static EN_ControllerType     MO_ControllerType = ControllerType_AdaptiveDnStream;
//static EN_A_GainFactor       MO_GainFactor    = GainFactor_0_10;
//static EN_A_SensorDelay      MO_SensorDelay   = SensorDelay_0_10;
static int          MO_GainFactor    = 0;
static int          MO_SensorDelay   = 0;
static EN_RampTime  MO_RampTime      = RampTime_5_0;

static uint32_t     MO_Pgain;
static uint32_t     MO_Igain;
static uint32_t     MO_Dgain;

void initController(void)
{
    MO_ControllerType    = readByteFromFlash(ADDR_ControllerType);
    MO_GainFactor        = readIntFromFlash(ADDR_GainFactor);
    MO_SensorDelay       = readIntFromFlash(ADDR_SensorDelay);
    MO_RampTime          = readByteFromFlash(ADDR_RampTime);

    MO_Pgain             = readUint32FromFlash(ADDR_Pgain);
    MO_Igain             = readUint32FromFlash(ADDR_Igain);
    MO_Dgain             = readUint32FromFlash(ADDR_Dgain);

    MO_ControllerType   = MO_ControllerType == 0xFF ? 1 : MO_ControllerType;
    MO_GainFactor       = MO_GainFactor == UINT32_MAX ? 100 : MO_GainFactor;
    MO_SensorDelay      = MO_SensorDelay == UINT32_MAX ? 10 : MO_SensorDelay;
    MO_RampTime         = MO_RampTime == 0xFF ? 10 : MO_RampTime;
    MO_Pgain            = MO_Pgain == UINT32_MAX ? 1000 : MO_Pgain;
    MO_Igain            = MO_Igain == UINT32_MAX ? 1000 : MO_Igain;
    MO_Dgain            = MO_Dgain == UINT32_MAX ? 1000 : MO_Dgain;

    if(MO_ControllerType < 0 || MO_ControllerType > 3) MO_ControllerType = 1;

    _controller_initialized = true;
}

bool syncControllerData(uint32_t addr)
{
    bool result = true;
    if (addr == _ADDR_MO_DATA) {
        result &= writeByteToFlash(ADDR_ControllerType, MO_ControllerType);
        result &= writeByteToFlash(ADDR_RampTime,       MO_RampTime);
        result &= writeIntToFlash(ADDR_GainFactor,      MO_GainFactor);
        result &= writeIntToFlash(ADDR_SensorDelay,     MO_SensorDelay);

        result &= writeUint32ToFlash(ADDR_Pgain, MO_Pgain);
        result &= writeUint32ToFlash(ADDR_Igain, MO_Igain);
        result &= writeUint32ToFlash(ADDR_Dgain, MO_Dgain);
    }
    return result;
}

/*
 * return EN_ControllerType
 */
uint8_t getControllerType(void)
{
    return MO_ControllerType;
}

bool setControllerType(uint8_t type)
{
    if (MO_ControllerType != type) {
        MO_ControllerType = type;

        // TODO change configuration
        //return writeByteToFlash(ADDR_ControllerType, MO_ControllerType);
    }
    return true;
}

/*
 * return Real Value * 100
 */
int getGainFactor(void)
{
    return MO_GainFactor;
}
bool setGainFactor(int factor)
{
    if (MO_GainFactor != factor) {
        MO_GainFactor = factor;

        // TODO change configuration
        //return writeByteToFlash(ADDR_GainFactor, MO_GainFactor);
    }
    return true;
}

/*
 * return Real Value * 100
 */
int getSensorDelay(void)
{
    return MO_SensorDelay;
}
bool setSensorDelay(int delay)
{
    if (MO_SensorDelay != delay) {
        MO_SensorDelay = delay;

        // TODO change configuration
        //return writeByteToFlash(ADDR_SensorDelay, MO_SensorDelay);
    }
    return true;
}

/*
 * return EN_A_RampTime
 */
uint8_t getRampTime(void)
{
    return MO_RampTime;
}
bool setRampTime(uint8_t time)
{
    if (MO_RampTime != time) {
        MO_RampTime = time;

        // TODO change configuration
        //return writeByteToFlash(ADDR_RampTime, MO_RampTime);
    }
    return true;
}

uint32_t getPgain(void)
{
    return MO_Pgain;
}
bool setPgain(uint32_t gain)
{
    if (MO_Pgain != gain) {
        MO_Pgain = gain;

        // TODO change configuration
        //return writeByteToFlash(ADDR_Pgain, MO_Pgain);
    }
    return true;
}

uint32_t getIgain(void)
{
    return MO_Igain;
}
bool setIgain(uint32_t gain)
{
    if (MO_Igain != gain) {
        MO_Igain = gain;

        // TODO change configuration
        //return writeByteToFlash(ADDR_Igain, MO_Igain);
    }
    return true;
}

uint32_t getDgain(void)
{
    return MO_Dgain;
}
bool setDgain(uint32_t gain)
{
    if (MO_Dgain != gain) {
        MO_Dgain = gain;

        // TODO change configuration
        //return writeByteToFlash(ADDR_Dgain, MO_Dgain);
    }
    return true;
}

/*
 *
 */
void setHoldPosition(void)
{
    // do nothing
}

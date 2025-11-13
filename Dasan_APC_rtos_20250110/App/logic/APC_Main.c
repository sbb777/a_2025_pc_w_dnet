/*
 * app.c
 *
 *  Created on: Aug 15, 2023
 *      Author: Yongseok
 */

#include <APC_Main.h>

#include <APC_AccessMode.h>
#include <APC_ControlMode.h>
#include <APC_Board.h>
#include <APC_Controller.h>
#include <APC_Power.h>
#include <APC_Sensor.h>
#include <APC_Valve.h>
#include <APC_Error.h>
#include <APC_Learn.h>
#include <APC_Warning.h>
#include <APC_Synch.h>

#include <APC_CmdListener.h>
#include <APC_Scheduler.h>

static bool     _main_initialized   = false;
static bool     _tickFlag = false;

bool initMain(void)
{
    //HAL_GPIO_WritePin(RES__GPIO_Port, RES__Pin, GPIO_PIN_SET);
		// B.1
    HAL_GPIO_WritePin(MPU_SW_RST__GPIO_Port, MPU_SW_RST__Pin, GPIO_PIN_SET);

    APC_Delay(1000);
    /*
     * XXX 디스플레이
     * digit2: 1(LOGIC), 2(RS232), 3(RS232+analog_output)
     * digit3: 0(basic), 1(SPS), 2(PFO), 3(SPS+PFO)
     * digit4: 1(1sensor), 2(2sensor)
     */
    initDisplay();
    setDisplayData("0332");
    APC_Delay(2000);

    bool result = true;
    /*
     * Peripheral 초기화: Flash, SRAM, LCD, Clock, IO, I2C, UART, SPI, CAN, …
     * Config data 로딩: System, Service, Interface(DI/DO, RS232), Sensor, Valve, Display, LEARN, PID
     * Error 확인: 심각한 수준의 장애인 경우, Fatal Error 상태로 천이
     * Motor interlock 확인: Safety mode 상태로 천이
     */
    result &= initBoard();
    result &= initSensor();
    result &= initValve();
    result &= initPower();

    setDisplayData(" 332");
    APC_Delay(2000);

    initController();
    initLearn();
    initError();
    initWarning();

    initCmdListener();
    initScheduler();

    result &= initAccessMode();
    result &= initControlMode();

#ifdef __REV_MTECH__
    printLocalRS232(APC_PORT_LOCAL, "[LOCAL DBG] %s %s\r\n", __FILE__, __FUNCTION__);
    printfRemoteRS232("[REMOTE DBG] %s %s\r\n", __FILE__, __FUNCTION__);
#endif

#if 0 //def __REV_MTECH__
    writeAOUT_POSITION(5000);
    writeAOUT_PRESSURE(15000);	// 30000 5.12
#endif

    _main_initialized = result;
    /*uint8_t airStatus = checkCompAirStatus(APC_PORT_INTERNAL, true);
    if (airStatus != 0) {
        setWarningCode(Warning_CompressedAir);

        initDisplay();
        //setDisplayCodeNumber(getControlMode(), getFatalErrorCode());
        setDisplayData("AIRx");
        //result = false;

        while(checkCompAirStatus(APC_PORT_INTERNAL, true) != 0)
            APC_Delay(500);
    }*/

    if (result == true) {
        initSynch();
        getSynchStatus();
    }

    _main_initialized = result;
    return _main_initialized;
}

void procMain(uint32_t counter)
{
#ifdef _USE_THREAD
    procCmdListener(counter);
    // XXX 개별적인 처리로 변경 (2024-04-30)
    //if (_main_initialized == true)  procScheduler(counter);

#else
	while (1) {
	    procCmdListener(counter);

	    if (_tickFlag == true) {
	        if (_main_initialized == true)  procScheduler(counter);
	        counter++;
	        _tickFlag = false;
	    }
	}
#endif
}

bool getInitStatus(void)
{
    return _main_initialized;
}

void setTickFlag(bool option)
{
    _tickFlag = option;
}

bool getTickFlag(void)
{
    return _tickFlag;
}

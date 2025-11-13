/*
 * APC_Scheduler.c
 *
 *  Created on: 2023. 10. 6.
 *      Author: Yongseok
 */

#include <APC_Scheduler.h>
#include <APC_Utils.h>

#include <APC_Error.h>
#include <APC_Board.h>
#include <APC_Model.h>
#include <APC_AsyncHandler.h>
#include <APC_AccessMode.h>
#include <APC_ControlMode.h>
#include "APC_CompAir.h"
#include "APC_Display.h"
#include "APC_Power.h"
#include "APC_Sensor.h"
#include "APC_Valve.h"
#include "APC_Controller.h"
#include "APC_RemotePort.h"

/*********************************************************************************/
static bool _exti_md_flag;
static bool _exti_if_flag;
static bool _exti_gl_flag;
static bool _exti_pfo_flag;

static bool _scheduler_initialized = false;
static bool _PFO_worked = false;
static bool _Irq_Pfo = false;

/*********************************************************************************/
void initScheduler(void)
{
    _scheduler_initialized = true;
}

void procScheduler(uint32_t cur_time)
{
//    scheduleInterrupt(cur_time);
//    scheduleDisplay(cur_time);
//    schedulePower(cur_time);
//    scheduleSensor(cur_time);
//    scheduleValve(cur_time);
//    scheduleAir(cur_time);
//    scheduleRemotePort(cur_time);
}

void scheduleInterrupt(uint32_t cur_time)
{
    if (_exti_md_flag) {
        _exti_md_flag = false;
        // XXX error status
    }

    if (_exti_if_flag) {
        _exti_if_flag = false;
        // XXX read bytes using SPI_CS_IF/ and SPI_CLK, SPI_I
        uint8_t irq_if[] = "IRQ_IF";
        writeRemoteRS232Data(irq_if, strlen((char *) irq_if));
    }

    if (_exti_gl_flag) {
        _exti_gl_flag = false;
        // do nothing
    }

    if (_exti_pfo_flag) {
        _exti_pfo_flag = false;
        // XXX read bytes using SPI_CS_PS/ and SPI_CLK, SPI_I
    }
}

void scheduleDisplay(uint32_t cur_time)
{
    static uint32_t disp_time = 0;
    if (cur_time - disp_time >= 10) {  // counter -> 50 msec
        disp_time = cur_time;
#if 1
        setDisplayCodeNumber(getControlMode(), getCurrentEncoder()/PERMILL);        //getCurrentValvePosition()/PERMILL);
#else
        //setDisplayCodeNumber(getControlMode(), _getCurrentValvePosition() / PERMILL);
        // XXX 저장된 값을 read
        setDisplayCodeNumber(getControlMode(), getCurrentValvePosition() / PERMILL);
#endif
    }

    static bool _led_flag = APC_LOW;
    static uint32_t led_time = 0;
    if (cur_time - led_time >= 1000) {  // 1 sec
        led_time = cur_time;

        setLed(_led_flag);
        _led_flag = _led_flag == APC_HIGH ? APC_LOW : APC_HIGH;
    }
}

void schedulePower(uint32_t cur_time)
{
    //static int  prev_power_val = 0;
    static uint32_t power_time = 0;

    if (cur_time - power_time >= 100) { // 100 msec
        power_time = cur_time;
        refreshPower();
        // Power ON : SET, Power OFF : 처음에는 RESET으로 들어옴. 이때 Power ON을 해도 RESET으로 들어오다가 2분정도 지나면 SET으로 들어옴.

#ifdef __REV_MTECH__
        if(HAL_GPIO_ReadPin(MPU_IRQ_D24PFO_GPIO_Port, MPU_IRQ_D24PFO_Pin) == GPIO_PIN_RESET){
            if(_Irq_Pfo == false){
                addPFOCycleCounter();      // IRQ PFO Counter ++
                _Irq_Pfo = true;
            }
        }
        else{
            if(_Irq_Pfo == true){
                _Irq_Pfo = false;
                _PFO_worked = false;
                addPowerUpCounter();
            }
        }
#else
        if(HAL_GPIO_ReadPin(IRQ_PFO_GPIO_Port, IRQ_PFO_Pin) == GPIO_PIN_RESET){
            if(_Irq_Pfo == false){
                addPFOCycleCounter();      // IRQ PFO Counter ++
                _Irq_Pfo = true;
            }
        }
        else{
            if(_Irq_Pfo == true){
                _Irq_Pfo = false;
                _PFO_worked = false;
                addPowerUpCounter();
            }
        }
#endif
        //int power = getPower(APC_PORT_INTERNAL);
        //prev_power_val = power;
    }
}

void scheduleSensor(uint32_t cur_time)
{
    static uint32_t sensor_time = 0;
    if (cur_time - sensor_time >= 100) { // 10 :  counter -> 20 msec
        sensor_time = cur_time;
        refreshSensor();
    }
}

void scheduleValve(uint32_t cur_time)
{
    uint8_t ctrl_mode = getControlMode();
    if (ctrl_mode < ControlMode_POSITION || ctrl_mode > ControlMode_MaintenanceClose)
        return;

    static uint32_t valve_time = 0;
    if (cur_time - valve_time >= 10) { // counter -> 20msec
        valve_time = cur_time;

        // XXX refresh current position
        _getCurrentValvePosition();

        if (getSimulationStatus() == SimulationStatus_SystemSimulationRunning) {
            getCurrentSensorPressureSimulation(getCurrentValvePosition());
        }
    }

    if(_Irq_Pfo == true && _PFO_worked == false){
        _PFO_worked = true;
        //if(getValvePosAfterPowerFail() == ValvePosAfterPowerFail_Open){
        //    setValveOpen();
        //}
        //else{
        //    setValveClosed();
        //}

        syncValveCount();
        //setCountSync(0x080C0040, 8);
    }
}

void scheduleAir(uint32_t cur_time)
{
    static uint32_t air_time = 0;
    if (cur_time - air_time >= 50) { // 100msec
        air_time = cur_time;
        checkCompAirStatus(APC_PORT_INTERNAL, false);

        /*uint8_t ctrl_mode = getControlMode();
        if (ctrl_mode == ControlMode_POSITION || ctrl_mode == ControlMode_PRESSURE
             || ctrl_mode == ControlMode_OPEN || ctrl_mode == ControlMode_LEARN
             || ctrl_mode == ControlMode_HOLD || ctrl_mode == ControlMode_INTERLOCK_OPEN) {

            uint8_t airStatus = checkCompAirStatus(APC_PORT_INTERNAL, false);
            if (airStatus != 0) {
                //setControlMode(APC_PORT_INTERNAL, ControlMode_FatalError);
                //setFatalErrorCode((uint8_t) E40_MotorDriverFailure);

                //initDisplay();
                //setDisplayCodeNumber(getControlMode(), getFatalErrorCode());
                //setDisplayData("AIRx");
            }
            //refreshCompAirStatus(APC_PORT_INTERNAL, false);
        }*/
    }
}

void scheduleRemotePort(uint32_t cur_time)
{
    static bool bStart = false;

    switch(getInterfaceType()){
        case InterfaceType_RS232:
        case InterfaceType_RS232_AO:
        case InterfaceType_RS232_485:
        case InterfaceType_RS232_485_AO:
        case InterfaceType_RS232_485_volt:
        case InterfaceType_RS232_485_volt_AO:
            break;
        default:
            return;
    }
//    if (getInterfaceType() != InterfaceType_RS232)  return;

    /*
     * process INPUT
     */
    static uint32_t remote_input_time  = 0;
    static uint32_t demo_time = 0;
    static int i=0;

    if (cur_time - remote_input_time >= 100) { // 500msec
        remote_input_time = cur_time;

        uint8_t remoteInput = readRemotePort();
        uint8_t close_input = readCLOSE_INPUT(remoteInput);
        uint8_t open_input  = readOPEN_INPUT(remoteInput);
        uint8_t pw_md_on    = readPW_MD_ON(remoteInput);

        if (pw_md_on == APC_LOW) {
            /*
             * To provide power to the valve motor pins 4 and 8 must be bridged,
             * otherwise motor interlock is active and the valve enters the safety mode and is not operative
             */
            if (getControlMode() != ControlMode_SafetyMode) {
                setControlMode(APC_PORT_INTERNAL, ControlMode_SafetyMode);
                setHoldPosition();
            }
            // DO NOT any operation
        }
        else {
            if (getControlMode() == ControlMode_SafetyMode) {
                setControlMode(APC_PORT_INTERNAL, ControlMode_POSITION);
            }

            if (close_input == APC_HIGH/* && open_input == APC_LOW*/) {
                //if (getRemoteDigitalInputCloseValve() != DigitalInputCloseValve_Disabled){
                //    if (getControlMode() != ControlMode_INTERLOCK_CLOSED) {
                //        setControlMode(APC_PORT_INTERNAL, ControlMode_INTERLOCK_CLOSED);
                //        setValveClosed();
                //    }
                //}
                //setValveClosed();
                bStart = false;
            }
            else if (/*close_input == APC_LOW && */open_input == APC_HIGH) {
                //if (getRemoteDigitalInputOpenValve() != DigitalInputOpenValve_Disabled){
                //    if (getControlMode() != ControlMode_INTERLOCK_OPEN) {
                //        setControlMode(APC_PORT_INTERNAL, ControlMode_INTERLOCK_OPEN);
                //        setValveOpen();
                //    }
                //}
                //setValveOpen();
                if(bStart == true)
                    bStart = false;
                else
                    bStart = true;
            }
        }

        if(bStart){
            switch(i){
                case 0:
                    setValveOpen();     // printLocalRS232(APC_PORT_LOCAL, "[Demo] Next Step. step 2\r\n");
                    i++;
                    break;
                case 1:
                    if((getMotorStatus() & 0x05))
                        i++;
                    demo_time = cur_time;
                    break;
                case 2:
                    if(cur_time - demo_time >= 400){
                        demo_time = cur_time;
                        i++;
                    }
                    break;
                case 3:
                    setSetpointValvePosition(90000);
                    i++;
                    break;
                case 4:
                    if((getMotorStatus() & 0x05))
                        i++;
                    demo_time = cur_time;
                    break;
                case 5:
                    if(cur_time - demo_time >= 200){
                        demo_time = cur_time;
                        i++;
                    }
                    break;
                case 6:
                    setSetpointValvePosition(80000);
                    i++;
                    break;
                case 7:
                    if((getMotorStatus() & 0x05))
                        i++;
                    demo_time = cur_time;
                    break;
                case 8:
                    if(cur_time - demo_time >= 200){
                        demo_time = cur_time;
                        i++;
                    }
                    break;
                case 9:
                    setSetpointValvePosition(70000);
                    i++;
                    break;
                case 10:
                    if((getMotorStatus() & 0x05))
                        i++;
                    demo_time = cur_time;
                    break;
                case 11:
                    if(cur_time - demo_time >= 200){
                        demo_time = cur_time;
                        i++;
                    }
                    break;
                case 12:
                    setSetpointValvePosition(60000);
                    i++;
                    break;
                case 13:
                    if((getMotorStatus() & 0x05))
                        i++;
                    demo_time = cur_time;
                    break;
                case 14:
                    if(cur_time - demo_time >= 200){
                        demo_time = cur_time;
                        i++;
                    }
                    break;
                case 15:
                    setSetpointValvePosition(50000);
                    i++;
                    break;
                case 16:
                    if((getMotorStatus() & 0x05))
                        i++;
                    demo_time = cur_time;
                    break;
                case 17:
                    if(cur_time - demo_time >= 200){
                        demo_time = cur_time;
                        i++;
                    }
                    break;
                case 18:
                    setSetpointValvePosition(40000);
                    i++;
                    break;
                case 19:
                    if((getMotorStatus() & 0x05))
                        i++;
                    demo_time = cur_time;
                    break;
                case 20:
                    if(cur_time - demo_time >= 200){
                        demo_time = cur_time;
                        i++;
                    }
                    break;
                case 21:
                    setSetpointValvePosition(30000);
                    i++;
                    break;
                case 22:
                    if((getMotorStatus() & 0x05))
                        i++;
                    demo_time = cur_time;
                    break;
                case 23:
                    if(cur_time - demo_time >= 200){
                        demo_time = cur_time;
                        i++;
                    }
                    break;
                case 24:
                    setSetpointValvePosition(20000);
                    i++;
                    break;
                case 25:
                    if((getMotorStatus() & 0x05))
                        i++;
                    demo_time = cur_time;
                    break;
                case 26:
                    if(cur_time - demo_time >= 200){
                        demo_time = cur_time;
                        i++;
                    }
                    break;
                case 27:
                    setSetpointValvePosition(10000);
                    i++;
                    break;
                case 28:
                    if((getMotorStatus() & 0x05))
                        i++;
                    demo_time = cur_time;
                    break;
                case 29:
                    if(cur_time - demo_time >= 200){
                        demo_time = cur_time;
                        i++;
                    }
                    break;
                case 30:
                    setSetpointValvePosition(0);
                    i++;
                    break;
                case 31:
                    if((getMotorStatus() & 0x05))
                        i++;
                    demo_time = cur_time;
                    break;
                case 32:
                    if(cur_time - demo_time >= 700){
                        demo_time = cur_time;
                        i=0;
                    }
                    break;
            }
        }
    }

    /*
     * process OUTPUT
     */
    static uint32_t remote_output_time = 0;
    if (cur_time - remote_output_time >= 100) { // 100msec
        remote_output_time = cur_time;

        uint8_t ctrl_mode = getControlMode();
        if (ctrl_mode == ControlMode_CLOSED || ctrl_mode == ControlMode_INTERLOCK_CLOSED) {
            writeRELAY_CLOSE(APC_HIGH);
            writeRELAY_OPEN(APC_LOW);
        }
        else if (ctrl_mode == ControlMode_OPEN || ctrl_mode == ControlMode_INTERLOCK_OPEN) {
            writeRELAY_CLOSE(APC_LOW);
            writeRELAY_OPEN(APC_HIGH);
        }
        else {
            writeRELAY_CLOSE(APC_LOW);
            writeRELAY_OPEN(APC_LOW);
        }

        //getRemoteRS232Baudrate();
        //writeRemoteRS232Data((uint8_t *) "ABCDEFGH", strlen("ABCDEFGH"));

        if (getRS232IntfAnalogOutputEquip() != Dev_Equipped)    return;

        writeAOUT_POSITION(getCurrentValvePosition());
        writeAOUT_PRESSURE(getCurrentSensorVoltage());
    }
}

void scheduleSyncData(uint32_t cur_time)
{
    static uint32_t sync_time = 0;
    if (cur_time - sync_time >= 10000) {
        sync_time = cur_time;
        setDataSync(_ADDR_MO_DATA, DATA_SYNC_FULL);
    }
}


/*
 * Polling in schedulePower()
 * not working
 * INPUT
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
#ifdef __REV_MTECH__
		if (GPIO_Pin == MPU_IRQ_D24PFO_Pin) {
			_exti_pfo_flag = true;
		}
#else
    if (GPIO_Pin == IRQ_MD_Pin) {
        _exti_md_flag = true;
    } else if (GPIO_Pin == IRQ_IF_Pin) {
        _exti_if_flag = true;
    } else if (GPIO_Pin == IRQ_GL_Pin) {
        _exti_gl_flag = true;
    } else if (GPIO_Pin == IRQ_PFO_Pin) {
        _exti_pfo_flag = true;
    }
#endif
}

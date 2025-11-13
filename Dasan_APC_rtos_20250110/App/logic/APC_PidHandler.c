/*
 * APC_PidHandler.c
 *
 *  Created on: Feb 23, 2024
 *      Author: Yongseok
 */
#include <APC_ControlMode.h>
#include <APC_Learn.h>
#include "APC_PidHandler.h"

static bool _pid_initialized = false;
static bool _pid_run_state   = false;
static bool _isFirstRun      = true;        // Flag for 1st step in PID Control. Added by JWY. 20240612

static double PID_Kp         = 0.0;      // P gain. Added by JWY. 20240612        -> Move to APC_Valve.c
static double PID_Ki         = 0.0;      // I gain. Added by JWY. 20240612        -> Move to APC_Valve.c
static double PID_Kd         = 0.0;      // D gain. Added by JWY. 20240612        -> Move to APC_Valve.c
//static double MO_PID_SensorDelay= 0.0;      // Sensor Delay time in sec. Added by JWY. 20240612 -> Move to APC_Valve.c
int     PID_BaseStep         = 100;

static double MO_PID_Max_Output = 10000.0; // Max limit for Output step in PID Control. Added by JWY. 20240612 -> Move to APC_Valve.c
static double MO_PID_Min_Output = -10000.0; // Min limit for Output step in PID Control. Added by JWY. 20240612 -> Move to APC_Valve.c

static double Error             = 0.0;      // Error = PV-SV. Added by JWY. 20240612
double Prev_Error        = 0.0;      // Previous Step Error. Added by JWY. 20240612
static int PID_SV            = 0.0;      // Set Value for PID Control. Added by JWY. 20240612

static double Output            = 0.0;      // Output step to move by PID Control. Added by JWY. 20240612
static double Poutput           = 0.0;      // Output step to move by P Control. Added by JWY. 20240612
static double Ioutput           = 0.0;      // Output step to move by I Control. Added by JWY. 20240612
static double Doutput           = 0.0;      // Output step to move by D Control. Added by JWY. 20240612

static double integrator_max = 20.0 ;
static double integrator_min = -20.0 ;

static double Integral_reset = 1000;

static double dout_max = 2500.0;
static double dout_min = -2500.0;

static double d_threshold =0.0;

void initPid()
{
    _isFirstRun = true;
    Poutput = Ioutput = Doutput = Output = 0.0;

    setPid();

    _pid_initialized = true;
}

void resetPid()
{
    _isFirstRun = true;
    Poutput = Ioutput = Doutput = Output = 0.0;
    setPidRunState(false);
}

// To set PID Initialize Flag. Added by JWY. 20240612
void setPidInitState(bool bState)
{
    _pid_initialized = bState;
}

// To get PID Initialize Flag. Added by JWY. 20240612
bool getPidInitState()
{
    return _pid_initialized;
}

// To set PID Run State Flag. Added by JWY. 20240612
void setPidRunState(bool bState)
{
    _pid_run_state = bState;
}

// To get PID Run State Flag. Added by JWY. 20240612
bool getPidRunState()
{
    return _pid_run_state;
}

bool isPidRunning()
{
    return _pid_run_state;
}

void setPid()
{
    // Put code to set PID gain & parameter
    /*switch(getPID_Kp()){
        case '0': PID_Kp = 0.1;     break;
        case '1': PID_Kp = 0.13;    break;
        case '2': PID_Kp = 0.18;    break;
        case '3': PID_Kp = 0.23;    break;
        case '4': PID_Kp = 0.32;    break;
        case '5': PID_Kp = 0.42;    break;
        case '6': PID_Kp = 0.56;    break;
        case '7': PID_Kp = 0.75;    break;
        case '8': PID_Kp = 1.0;     break;
        case '9': PID_Kp = 1.33;    break;
        case 'A': PID_Kp = 1.78;    break;
        case 'B': PID_Kp = 2.37;    break;
        case 'C': PID_Kp = 3.16;    break;
        case 'D': PID_Kp = 4.22;    break;
        case 'E': PID_Kp = 5.62;    break;
        case 'F': PID_Kp = 7.5;     break;
        case 'G': PID_Kp = 0.0001;  break;
        case 'H': PID_Kp = 0.0003;  break;
        case 'I': PID_Kp = 0.001;   break;
        case 'J': PID_Kp = 0.003;   break;
        case 'K': PID_Kp = 0.01;    break;
        case 'L': PID_Kp = 0.02;    break;
        case 'M': PID_Kp = 0.05;    break;
        default:  PID_Kp = 0.1;     break;
    }

    switch(getPID_Ki()){
        case '0': PID_Ki = 0;     break;
        case '1': PID_Ki = 20;    break;
        case '2': PID_Ki = 40;    break;
        case '3': PID_Ki = 60;    break;
        case '4': PID_Ki = 80;    break;
        case '5': PID_Ki = 100;   break;
        case '6': PID_Ki = 150;   break;
        case '7': PID_Ki = 200;   break;
        case '8': PID_Ki = 250;   break;
        case '9': PID_Ki = 300;   break;
        case 'A': PID_Ki = 350;   break;
        case 'B': PID_Ki = 400;   break;
        case 'C': PID_Ki = 500;   break;
        case 'D': PID_Ki = 600;   break;
        case 'E': PID_Ki = 800;   break;
        case 'F': PID_Ki = 1000;  break;
        default:  PID_Ki = 0;     break;
    }

    switch(getPID_Kd()){
        case '0': PID_Kd = 0.0;     break;
        case '1': PID_Kd = 0.5;     break;
        case '2': PID_Kd = 1.0;     break;
        case '3': PID_Kd = 1.5;     break;
        case '4': PID_Kd = 2.0;     break;
        case '5': PID_Kd = 2.5;     break;
        case '6': PID_Kd = 3.0;     break;
        case '7': PID_Kd = 3.5;     break;
        case '8': PID_Kd = 4.0;     break;
        case '9': PID_Kd = 4.5;     break;
        case 'A': PID_Kd = 5.0;     break;
        case 'B': PID_Kd = 5.5;     break;
        case 'C': PID_Kd = 6.0;     break;
        case 'D': PID_Kd = 6.5;     break;
        case 'E': PID_Kd = 7.0;     break;
        case 'F': PID_Kd = 7.5;     break;
        case 'G': PID_Kd = 8.0;     break;
        case 'H': PID_Kd = 8.5;     break;
        case 'I': PID_Kd = 9.0;     break;
        case 'J': PID_Kd = 9.5;     break;
        case 'K': PID_Kd = 10.0;    break;
        default:  PID_Kd = 0.0;     break;
    }*/
    PID_Kp = getPgain();
    PID_Ki = getIgain();
    PID_Kd = getDgain();
}

void setSV(int _newSV)
{
    PID_SV = _newSV;
}

double Ioutput_sum = 0.0;
static int newPressure_ =0;
static bool integral_ok = false;
static int currentValvePosition =0;

bool procPid()
{
    static bool isFirstProc = true;
    static int MAX_VALVE_POSITION = 25000; // 40000; // pendulum 25000;
    if (isFirstProc == true){
        MAX_VALVE_POSITION = getValveFullStroke();
        isFirstProc = false;
    }

//    MAX_VALVE_POSITION = getValveFullStroke();
    const int MIN_VALVE_POSITION = 0;
    int newPV, output, spValvePressure;

    while (!(getMotorStatus() & 0x05)) {
        APC_Delay(5);
    }

    //APC_Delay(PID_Ki);  // This delay might need adjustment depending on your system

    newPV = getCurrentSensorVoltage();
    newPressure_ =newPV;

    _getCurrentValvePosition();
    currentValvePosition = getCurrentEncoder();     // getCurrentValvePosition(0);
    output = calculatePidOutput(newPV);

    spValvePressure = getSetpointValvePressure();


    int newValvePosition = currentValvePosition + output;
    newValvePosition =
            (newValvePosition < MIN_VALVE_POSITION) ? MIN_VALVE_POSITION :
            (newValvePosition > MAX_VALVE_POSITION) ?
                    MAX_VALVE_POSITION : newValvePosition;

    setCurrentValvePosUsingAbsoluteStep(newValvePosition);
    int offset = 0;

    Output = output;

    return true;
}

/** --------------------------------------------------------------------
 * Description : 목표 SV에 도달하는 데 필요한 PID 출력값을 계산
 *
 * @param _newPV  : the monitored new PV
 * @return Output : PID Output -> Valve가 이동할 량으로 이용. (음수:CLOSE방향, 양수:OPEN방향)
 */

//static double errorRateOfParticipateIout = 10.0;
//static double errorRateOfParticipateDout = 8.0;
//static double errorRateOfParticipateIout = 2.0;
static double errorRateOfParticipateIout = 6.0;
static double errorRateOfParticipateDout = 10.0;

double getErrorRateOfSFS(int error);

static int delta_t = 100;

double getErrorRateOfSFS(int error) {
    return fabs((double) error / (getSensorFullScale()*10 + 1)*100);    //louis,20240730
    // Consider to use 10000000 instead of getSensorFullScale()*10. How much is the error?
}

int calculatePidOutput(int _newPV) {
    if(getControlMode() != ControlMode_PRESSURE) Ioutput_sum =0.0;
      double zError = _newPV - getSetpointValvePressure()*10000;//0710, pressure number changed

    PID_Kp=getPgain();
    PID_Ki=getIgain();
    PID_Kd=getDgain();
    Poutput =0;
    // P
    Poutput = (PID_Kp/1000) * PID_BaseStep * (zError / 10000); //louis

    if (Poutput >= 20000)
        Poutput = 20000;
    else if (Poutput <= -20000)
      Poutput = -20000;

    // I
    double tempIoutput = 0.0;
    if (errorRateOfParticipateIout == 0.0) {
        tempIoutput = (PID_Ki/1000) * (zError / 10000)/ 10;
    } else {

        if (fabs(getErrorRateOfSFS(zError)) <= errorRateOfParticipateIout) {
            tempIoutput = (PID_Ki/1000) * (zError / 10000)/ 10;
        } else {
            tempIoutput = 0.0;
            Ioutput_sum = 0;
        }
    }

    Ioutput = tempIoutput;

    // I clamping
    Ioutput_sum =
            (Ioutput_sum < integrator_min) ? integrator_min :
            (Ioutput_sum > integrator_max) ? integrator_max : Ioutput_sum;

    //    if (fabs(zError) < Integral_reset ) Ioutput_sum = 0;

    // D

    double tempDoutput = 0.0;
    if (errorRateOfParticipateDout == 0.0) {
        tempDoutput = (PID_Kd/100) * (zError - Prev_Error) / 10000;
    } else {

        if (fabs(getErrorRateOfSFS(zError)) <= errorRateOfParticipateDout) {
            tempDoutput = (PID_Kd/100) * (zError - Prev_Error) / 10000;
        } else {
            tempDoutput = 0.0;
            Doutput = 0;
        }
    }

    Doutput = tempDoutput;

    Doutput = (Doutput < dout_min) ? dout_min:
                (Doutput > dout_max) ? dout_max: Doutput;
    Prev_Error = zError;

    // Output
    Output = Poutput + Ioutput_sum + Doutput;

    // Output clamping
    Output = (Output < MO_PID_Min_Output) ? MO_PID_Min_Output :
             (Output > MO_PID_Max_Output) ? MO_PID_Max_Output : Output;


    integral_ok = (zError * Output > 0) ? false : true;
    //    if (integral_ok)  Ioutput_sum += tempIoutput;  // Ioutput 누적

    if (currentValvePosition > 4) Ioutput_sum += tempIoutput;  // Ioutput 누적

    if(getControlMode() != ControlMode_PRESSURE) Ioutput_sum =0.0;


    Error = zError;

    return (int) Output;
}

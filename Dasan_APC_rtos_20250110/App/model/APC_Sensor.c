/*
 * APC_Sensor.c
 *
 *  Created on: Oct 22, 2023
 *      Author: Yongseok
 */

#include <APC_Sensor.h>
#include <APC_Utils.h>
#include <math.h>

#include <APC_AccessMode.h>
#include <APC_ControlMode.h>
#include <APC_Model.h>
#include <APC_Flash.h>
#include <APC_ADS1252.h>
#include <APC_LocalPort.h>
#include <APC_Board.h>

/*********************************************************************************/
// Board(128) + Valve(64)
//#define ADDR_SENSOR_START           (_ADDR_MO_DATA + 128 + 64)

//#define ADDR_Sensor1Equip           (ADDR_SENSOR_START +  0)
//#define ADDR_Sensor1Selection       (0x080C0000)            //(ADDR_SENSOR_START +  1)
//#define ADDR_Sensor1Unit            (ADDR_SENSOR_START +  2)
//#define ADDR_Sensor1ApplyRatio      (ADDR_SENSOR_START +  3)
//#define ADDR_Sensor1FullScale       (ADDR_SENSOR_START +  4)
//#define ADDR_Sensor1Offset          (0x080C0002)            //(ADDR_SENSOR_START +  8)

//#define ADDR_Sensor2Equip           (ADDR_SENSOR_START + 20)
//#define ADDR_Sensor2Selection       (0x080C0001)            //(ADDR_SENSOR_START + 21)
//#define ADDR_Sensor2Unit            (ADDR_SENSOR_START + 22)
//#define ADDR_Sensor2ApplyRatio      (ADDR_SENSOR_START + 23)
//#define ADDR_Sensor2FullScale       (ADDR_SENSOR_START + 24)
//#define ADDR_Sensor2Offset          (0x080C0006)            //(ADDR_SENSOR_START + 28)

//#define ADDR_SensorType             (ADDR_SENSOR_START + 40)
//#define ADDR_SensorZeroStatus       (ADDR_SENSOR_START + 41)
//#define ADDR_SensorZeroAdjusted     (ADDR_SENSOR_START + 42)

//#define ADDR_SensorFullScaleRatio   (ADDR_SENSOR_START + 44)    // 4byte
//#define ADDR_PressureCommRange      (ADDR_SENSOR_START + 48)    // 4byte

//#define ADDR_SensorScale            (ADDR_SENSOR_START + 52)
//#define ADDR_SensorUnit             (ADDR_SENSOR_START + 56)

#define CROSSOVER_RATIO_LOW         (0.9)
#define CROSSOVER_RATIO_HIGH        (1.0)

#define VOLTAGE_SCALE               (10000000)

/*********************************************************************************/
static bool             _sensor_initialized;

static EN_SensorType    MO_SensorType         ;  // 사용자 설정 [s:01]
static EN_Status        MO_SensorZeroStatus   ;  // 사용자 설정 [z:20, s:01]
static uint8_t          MO_SensorZeroAdjusted ;  // 사용자 설정 [Z:]

static uint32_t         MO_SensorFullScaleRatio;  // ratio(1000 ~ 100000)
static uint32_t         MO_PressureCommRange  ;   // 고정됨 (1000)

static int              _SensorExponent         = 0;  // 사용자 설정 [s:05]
static uint32_t         _SetpointValvePressure  = 0;    // 사용자 설정 [S:0xxxxxxx]

static int              _SensorPressureSimulated = 0;
static int              _SensorVoltage = 0;

// XXX MO_SensorEquip 의해 결정
static EN_DevEquip      MO_Sensor1Equip       ;  // 0=NotEquip, 1=Equip
static EN_SensorSelect  MO_Sensor1Selection   ;  // 0=Unselected, 1=Selected
static EN_SensorUnit    MO_Sensor1Unit        ;
static uint8_t          MO_Sensor1ApplyRatio  ;  // For Calculating pressure from voltage.
static int              MO_Sensor1FullScale   ;  //
static int              MO_Sensor1Offset      ;  // Offset voltage (-1.4V=-1400000 ~ 1.4V=1400000)
static int              _Sensor1OffsetPr    = 0; // Sensor 1 Offset Pressure
static int              _Sensor1Value       = 0; // Sensor Measured voltage. Pressure = _Sensor1MeasPr
static int              _Sensor1ResultVol   = 0; // Sensor1Value - Sensor1Offset
static int              _Sensor1MeasPr      = 0; // Sensor 1 Measured Pressure. voltage = _Sensor1Value
static int              _Sensor1ResultPr    = 0; // Sensor 1 Result Pressure. Voltage = _Sensor1ResultVol

// XXX MO_SensorEquip 의해 결정
static EN_DevEquip      MO_Sensor2Equip       ;  // 0=NotEquip, 1=Equip
static EN_SensorSelect  MO_Sensor2Selection   ;  // 0=Unselected, 1=Selected
static EN_SensorUnit    MO_Sensor2Unit        ;
static uint8_t          MO_Sensor2ApplyRatio  ;  // For Calculating pressure from voltage.
static int              MO_Sensor2FullScale   ;  // Range configuration (1000)
static int              MO_Sensor2Offset      ;  // voltage (-1.4V=-1400000 ~ 1.4V=1400000)
static int              _Sensor2OffsetPr    = 0; // Sensor 2 Offset Pressure
static int              _Sensor2Value       = 0; // Sensor 2 Measured voltage. Pressure = _Sensor2MeasPr
static int              _Sensor2ResultVol   = 0; // (Sensor2Value - Sensor2Offset) * Unit
static int              _Sensor2MeasPr      = 0; // Sensor 2 Measured Pressure. Voltage = _Sensor2Value
static int              _Sensor2ResultPr    = 0; // Sensor 2 Result Pressure. Voltage = _Sensor2ResultVol

static uint32_t         _CrossOverRangeHigh = 1; // in Voltage unit.
static uint32_t         _CrossOverRangeLow  = 1; // in Voltage Unit.

static int              MO_SensorScale      = 1;
static EN_SensorUnit    MO_SensorUnit           ;

/*********************************************************************************/
bool initSensor(void)
{
    MO_Sensor1Equip      = readByteFromFlash(ADDR_Sensor1Equip);
    MO_Sensor1Selection  = readByteFromFlash(ADDR_Sensor1Selection);
	MO_Sensor1Unit       = readByteFromFlash(ADDR_Sensor1Unit);
	MO_Sensor1ApplyRatio = readByteFromFlash(ADDR_Sensor1ApplyRatio);
	MO_Sensor1FullScale  = readUint32FromFlash(ADDR_Sensor1FullScale);
	MO_Sensor1Offset     = readIntFromFlash(ADDR_Sensor1Offset);

	MO_Sensor2Equip      = readByteFromFlash(ADDR_Sensor2Equip);
	MO_Sensor2Selection  = readByteFromFlash(ADDR_Sensor2Selection);
	MO_Sensor2Unit       = readByteFromFlash(ADDR_Sensor2Unit);
	MO_Sensor2ApplyRatio = readByteFromFlash(ADDR_Sensor2ApplyRatio);
	MO_Sensor2FullScale  = readUint32FromFlash(ADDR_Sensor2FullScale);
	MO_Sensor2Offset     = readIntFromFlash(ADDR_Sensor2Offset);

	MO_SensorType        = readByteFromFlash(ADDR_SensorType);
	MO_SensorZeroStatus  = readByteFromFlash(ADDR_SensorZeroStatus);
	MO_SensorZeroAdjusted = readByteFromFlash(ADDR_SensorZeroAdjusted);
	//MO_SensorFullScaleRatio = readUint32FromFlash(ADDR_SensorFullScaleRatio);
	MO_PressureCommRange = readUint32FromFlash(ADDR_PressureCommRange);
	MO_SensorScale       = readUint32FromFlash(ADDR_SensorScale);
	MO_SensorUnit        = readByteFromFlash(ADDR_SensorUnit);

	// fix-up
	MO_Sensor1Equip      = MO_Sensor1Equip     == 0xff ? 0 : MO_Sensor1Equip;
	MO_Sensor1Selection  = MO_Sensor1Selection == 0xff ? 0 : MO_Sensor1Selection;
	MO_Sensor1Unit       = MO_Sensor1Unit      == 0xff ? SensorUnit_Torr : MO_Sensor1Unit;
    MO_Sensor1ApplyRatio = MO_Sensor1ApplyRatio == 0xff ? 50 : MO_Sensor1ApplyRatio;
	MO_Sensor1FullScale  = MO_Sensor1FullScale == UINT32_MAX ? 10 : MO_Sensor1FullScale;
	MO_Sensor1Offset     = MO_Sensor1Offset    == UINT32_MAX ? 0 : MO_Sensor1Offset;

	MO_Sensor2Equip      = MO_Sensor2Equip     == 0xff ? 0 : MO_Sensor2Equip;
    MO_Sensor2Selection  = MO_Sensor2Selection == 0xff ? 0 : MO_Sensor2Selection;
    MO_Sensor2Unit       = MO_Sensor2Unit      == 0xff ? SensorUnit_mTorr : MO_Sensor2Unit;
    MO_Sensor2ApplyRatio = MO_Sensor2ApplyRatio == 0xff ? 50 : MO_Sensor2ApplyRatio;
    MO_Sensor2FullScale  = MO_Sensor2FullScale == UINT32_MAX ? 10 : MO_Sensor2FullScale;
    MO_Sensor2Offset     = MO_Sensor2Offset    == UINT32_MAX ? 0 : MO_Sensor2Offset;

	if (MO_SensorType == 0xff) {
	    MO_SensorType = SensorType_NoSensor;
	    MO_SensorZeroStatus = Status_Disabled;
	    MO_SensorZeroAdjusted = false;
	    MO_SensorFullScaleRatio = 0;
	    MO_PressureCommRange    = 1000; // 고정

	    _sensor_initialized = false;
	}
	else {
	    MO_SensorZeroStatus   = MO_SensorZeroStatus   == 0xff ? Status_Enabled : MO_SensorZeroStatus;
	    MO_SensorZeroAdjusted = MO_SensorZeroAdjusted == 0xff ? false : MO_SensorZeroAdjusted;
	    MO_SensorFullScaleRatio = MO_SensorFullScaleRatio == UINT32_MAX ? 0 : MO_SensorFullScaleRatio;
	    MO_PressureCommRange    = MO_PressureCommRange    == UINT32_MAX ? 1000 : MO_PressureCommRange;

	    // p_SensorUnit, p_SensorFullScale, MO_SensorFullScaleRatio
        alignSensor();

        _sensor_initialized = true;
	}

	if(MO_SensorUnit == 0xff){
	    switch(MO_SensorType){
	        case SensorType_1Operation_Sensor1:
	        case SensorType_2Operation_Sensor1High:
	            MO_SensorUnit = MO_Sensor1Unit;
	            MO_SensorScale = MO_Sensor1FullScale;
	            break;
	        case SensorType_1Operation_Sensor2:
	        case SensorType_2Operation_Sensor2High:
	            MO_SensorUnit = MO_Sensor2Unit;
	            MO_SensorScale = MO_Sensor2FullScale;
	            break;
	        default:
	            MO_SensorType = SensorType_1Operation_Sensor1;
	            MO_SensorUnit = MO_Sensor1Unit;
	            MO_SensorScale = MO_Sensor1FullScale;
	            break;
	    }
	}

	return _sensor_initialized;
}

bool syncSensorData(uint32_t addr)
{
    bool result = true;
    if (addr == _ADDR_MO_DATA) {
        result &= writeByteToFlash(ADDR_Sensor1Equip,       MO_Sensor1Equip);
        result &= writeByteToFlash(ADDR_Sensor1Selection,   MO_Sensor1Selection);
        result &= writeByteToFlash(ADDR_Sensor1Unit,        MO_Sensor1Unit);
        result &= writeByteToFlash(ADDR_Sensor1ApplyRatio,  MO_Sensor1ApplyRatio);
        result &= writeUint32ToFlash(ADDR_Sensor1FullScale, MO_Sensor1FullScale);
        result &= writeIntToFlash(ADDR_Sensor1Offset,       MO_Sensor1Offset);

        result &= writeByteToFlash(ADDR_Sensor2Equip,       MO_Sensor2Equip);
        result &= writeByteToFlash(ADDR_Sensor2Selection,   MO_Sensor2Selection);
        result &= writeByteToFlash(ADDR_Sensor2Unit,        MO_Sensor2Unit);
        result &= writeByteToFlash(ADDR_Sensor2ApplyRatio,  MO_Sensor2ApplyRatio);
        result &= writeUint32ToFlash(ADDR_Sensor2FullScale, MO_Sensor2FullScale);
        result &= writeIntToFlash(ADDR_Sensor2Offset,       MO_Sensor2Offset);

        result &= writeByteToFlash(ADDR_SensorType,         MO_SensorType);
        result &= writeByteToFlash(ADDR_SensorZeroStatus,   MO_SensorZeroStatus);
        result &= writeByteToFlash(ADDR_SensorZeroAdjusted, MO_SensorZeroAdjusted);
        //result &= writeUint32ToFlash(ADDR_SensorFullScaleRatio, MO_SensorFullScaleRatio);
        result &= writeUint32ToFlash(ADDR_PressureCommRange, MO_PressureCommRange);

        result &= writeUint32ToFlash(ADDR_SensorScale, MO_SensorScale);
        result &= writeByteToFlash(ADDR_SensorUnit, MO_SensorUnit);
    }
    return result;
}

/**
 * called by 'z:20'
 */
void alignSensor(void)
{
    int sensor1_sf, sensor2_sf;
    MO_SensorFullScaleRatio = 1000;

    if((MO_SensorType == SensorType_1Operation_Sensor1) || (MO_SensorType == SensorType_2Operation_Sensor1High))
    {
        MO_SensorUnit    = MO_Sensor1Unit;
        MO_SensorScale   = MO_Sensor1FullScale;

        switch(MO_Sensor1Unit){
            case SensorUnit_Pa:
            case SensorUnit_ubar:
            case SensorUnit_mTorr:
                sensor1_sf = MO_Sensor1FullScale;
                break;
            case SensorUnit_mbar:
            case SensorUnit_Torr:
                sensor1_sf = 1000*MO_Sensor1FullScale;
                break;
        }
        if(MO_SensorType == SensorType_2Operation_Sensor1High){
            switch(MO_Sensor2Unit){
                case SensorUnit_Pa:
                case SensorUnit_ubar:
                case SensorUnit_mTorr:
                    sensor2_sf = MO_Sensor2FullScale;
                    break;
                case SensorUnit_mbar:
                case SensorUnit_Torr:
                    sensor2_sf = 1000*MO_Sensor2FullScale;
                    break;
            }
            MO_SensorFullScaleRatio = sensor1_sf * 1000 / sensor2_sf;
            _CrossOverRangeHigh = 10000000 / MO_SensorFullScaleRatio;
            _CrossOverRangeLow  =  9000000 / MO_SensorFullScaleRatio;
        }
//        MO_SensorFullScaleRatio = PERMILL *
//              (MO_SensorType == SensorType_1Operation_Sensor1 ? 1 : MO_Sensor1FullScale/MO_Sensor2FullScale);
    }
    else if((MO_SensorType == SensorType_1Operation_Sensor2) || (MO_SensorType == SensorType_2Operation_Sensor2High))
    {
        MO_SensorUnit    = MO_Sensor2Unit;
        MO_SensorScale   = MO_Sensor2FullScale;

        switch(MO_Sensor2Unit){
            case SensorUnit_Pa:
            case SensorUnit_ubar:
            case SensorUnit_mTorr:
                sensor2_sf = MO_Sensor2FullScale;
                break;
            case SensorUnit_mbar:
            case SensorUnit_Torr:
                sensor2_sf = 1000*MO_Sensor2FullScale;
                break;
        }

        if(MO_SensorType == SensorType_2Operation_Sensor2High){
            switch(MO_Sensor1Unit){
                case SensorUnit_Pa:
                case SensorUnit_ubar:
                case SensorUnit_mTorr:
                    sensor1_sf = MO_Sensor1FullScale;
                    break;
                case SensorUnit_mbar:
                case SensorUnit_Torr:
                    sensor1_sf = 1000*MO_Sensor1FullScale;
                    break;
            }

            MO_SensorFullScaleRatio = sensor2_sf * 1000 / sensor1_sf;
            _CrossOverRangeHigh = 10000000 / MO_SensorFullScaleRatio;
            _CrossOverRangeLow  =  9000000 / MO_SensorFullScaleRatio;
        }
//        MO_SensorFullScaleRatio = PERMILL *
//                (MO_SensorType == SensorType_1Operation_Sensor2 ? 1 : MO_Sensor2FullScale/MO_Sensor1FullScale);
    }
}

void refreshSensor(void)
{
    int sensor1_value_filtered = 0;
    int sensor2_value_filtered = 0;

    switch (MO_SensorType)
    {
        case SensorType_1Operation_Sensor1:
            sensor1_value_filtered = getADS1252(SENSOR_SENSOR1);

            if(sensor1_value_filtered > 2000000)
                _Sensor1Value = sensor1_value_filtered / 1100 * 1000;
            else
                _Sensor1Value = sensor1_value_filtered * 1000 / 1100;

            _Sensor1ResultVol = (_Sensor1Value - MO_Sensor1Offset) ;
            _Sensor2ResultVol = 0;
            break;
        case SensorType_1Operation_Sensor2:
            sensor2_value_filtered = getADS1252(SENSOR_SENSOR2);

            if(sensor2_value_filtered > 2000000)
                _Sensor2Value = sensor2_value_filtered / 1100 * 1000;
            else
                _Sensor2Value = sensor2_value_filtered * 1000 / 1100;

            _Sensor2ResultVol = (_Sensor2Value - MO_Sensor2Offset) ;
            _Sensor1ResultVol = 0;
            break;
        case SensorType_2Operation_Sensor1High:
            sensor1_value_filtered = getADS1252(SENSOR_SENSOR1);
            sensor2_value_filtered = getADS1252(SENSOR_SENSOR2);

            if(sensor1_value_filtered > 2000000)
                _Sensor1Value = sensor1_value_filtered / 1100 * 1000;
            else
                _Sensor1Value = sensor1_value_filtered * 1000 / 1100;

            if(sensor2_value_filtered > 2000000)
                _Sensor2Value = sensor2_value_filtered / 1100 * 1000;
            else
                _Sensor2Value = sensor2_value_filtered * 1000 / 1100;

            _Sensor1ResultVol = (_Sensor1Value - MO_Sensor1Offset);
            _Sensor2ResultVol = (_Sensor2Value - MO_Sensor2Offset) * 1000 / (int)MO_SensorFullScaleRatio;
            break;
        case SensorType_2Operation_Sensor2High:
            sensor1_value_filtered = getADS1252(SENSOR_SENSOR1);
            sensor2_value_filtered = getADS1252(SENSOR_SENSOR2);

            if(sensor1_value_filtered > 2000000)
                _Sensor1Value = sensor1_value_filtered / 1100 * 1000;
            else
                _Sensor1Value = sensor1_value_filtered * 1000 / 1100;

            if(sensor2_value_filtered > 2000000)
                _Sensor2Value = sensor2_value_filtered / 1100 * 1000;
            else
                _Sensor2Value = sensor2_value_filtered * 1000 / 1100;

            _Sensor1ResultVol = (_Sensor1Value - MO_Sensor1Offset) * 1000 / (int)MO_SensorFullScaleRatio;
            _Sensor2ResultVol = (_Sensor2Value - MO_Sensor2Offset);
            break;
        default:
            _Sensor1ResultVol = _Sensor2ResultVol = 0;
            break;
    }
}

bool adjustSensorZero(uint8_t option)
{
    if (MO_SensorZeroStatus == Status_Disabled) return false;

    if (option == 0) {
        setSensorOffset(0,0);
        MO_SensorZeroAdjusted = true;
        return MO_SensorZeroAdjusted;
    }

    bool ret = true;
    switch (MO_SensorType)
    {
        case SensorType_1Operation_Sensor1:
            if (_Sensor1Value < SENSOR_OFFSET_MIN || _Sensor1Value > SENSOR_OFFSET_MAX) {
                ret &= false;
            } else {
                _Sensor2Value = 0;
            }
            break;
        case SensorType_1Operation_Sensor2:
            if (_Sensor2Value < SENSOR_OFFSET_MIN || _Sensor2Value > SENSOR_OFFSET_MAX) {
                ret &= false;
            } else {
                _Sensor1Value = 0;
            }
            break;
        case SensorType_2Operation_Sensor1High:
        case SensorType_2Operation_Sensor2High:
            if (_Sensor1Value < SENSOR_OFFSET_MIN || _Sensor1Value > SENSOR_OFFSET_MAX) {
                ret &= false;
            }
            if (_Sensor2Value < SENSOR_OFFSET_MIN || _Sensor2Value > SENSOR_OFFSET_MAX) {
                ret &= false;
            }
            break;
        default:
            ret = false;
            break;
    }

    if(ret){
        ret = setSensorOffset(_Sensor1Value, _Sensor2Value);
    }

    MO_SensorZeroAdjusted = ret;
    return MO_SensorZeroAdjusted;
}

void setSensorType(uint8_t sensor_type)
{
    MO_SensorType = sensor_type;
}

uint8_t getSensorType(void)
{
    return MO_SensorType;
}

void setSensorUnit(uint8_t sensor_unit)
{
    if (MO_SensorType == SensorType_1Operation_Sensor1 || MO_SensorType == SensorType_2Operation_Sensor1High)
    {
        MO_Sensor1Unit = sensor_unit;
    }
    else if (MO_SensorType == SensorType_1Operation_Sensor2 || MO_SensorType == SensorType_2Operation_Sensor2High)
    {
        MO_Sensor2Unit = sensor_unit;
    }
}

uint8_t getSensorUnit(void)
{
    return MO_SensorUnit;       // *p_SensorUnit;
}

void setSensorFullScale(uint32_t sfs)
{
    if (MO_SensorType == SensorType_1Operation_Sensor1 || MO_SensorType == SensorType_2Operation_Sensor1High)
    {
        MO_Sensor1FullScale = sfs;

        if (MO_SensorFullScaleRatio >= 1000 && MO_SensorType == SensorType_2Operation_Sensor1High) {
            MO_Sensor2FullScale = MO_Sensor1FullScale / MO_SensorFullScaleRatio;
        }
    }
    else if (MO_SensorType == SensorType_1Operation_Sensor2 || MO_SensorType == SensorType_2Operation_Sensor2High)
    {
        MO_Sensor2FullScale = sfs;

        if (MO_SensorFullScaleRatio >= 1000 && MO_SensorType == SensorType_2Operation_Sensor2High) {
            MO_Sensor1FullScale = MO_Sensor2FullScale / MO_SensorFullScaleRatio;
        }
    }
}

int getSensorFullScale(void)
{
    return MO_SensorScale;      // *p_SensorFullScale;
}

void setSensorExponent(int exponent)
{
    _SensorExponent = exponent;
}

int getSensorExponent(void)
{
    return _SensorExponent;
}

void setSensorZeroStatus(uint8_t status)
{
    MO_SensorZeroStatus = status;
}

uint8_t getSensorZeroStatus(void)
{
    return MO_SensorZeroStatus;
}

void setSensorZeroAdjusted(uint8_t status)
{
    MO_SensorZeroAdjusted = status;
}

uint8_t getSensorZeroAdjusted(void)
{
    return MO_SensorZeroAdjusted;
}

/**
 * ratio(1000 ~ 100000) = High/Low * 1000
 */
void setSensorFullScaleRatio(uint32_t ratio)
{
    MO_SensorFullScaleRatio = ratio;

/*    if (MO_SensorType == SensorType_1Operation_Sensor1 || MO_SensorType == SensorType_2Operation_Sensor1High)
    {
        if (MO_SensorFullScaleRatio >= 1000 && MO_SensorType == SensorType_2Operation_Sensor1High) {
            MO_Sensor2FullScale = MO_Sensor1FullScale / MO_SensorFullScaleRatio;
        }
    }
    else if (MO_SensorType == SensorType_1Operation_Sensor2 || MO_SensorType == SensorType_2Operation_Sensor2High)
    {
        if (MO_SensorFullScaleRatio >= 1000 && MO_SensorType == SensorType_2Operation_Sensor2High) {
            MO_Sensor1FullScale = MO_Sensor2FullScale / MO_SensorFullScaleRatio;
        }
    }*/
}

uint32_t getSensorFullScaleRatio(void)
{
    return MO_SensorFullScaleRatio;
}

void setSetpointValvePressure(uint32_t setpoint)
{
    _SetpointValvePressure = setpoint;

    // TODO calculate PID and send command to motor

}

uint32_t getSetpointValvePressure(void)
{
    return _SetpointValvePressure;
}

/*
 * MO_SensorZeroExecuted == true 상태에서만 요청 처리
 */
int getCurrentSensorVoltage(void)
{
    if (getSimulationStatus() == SimulationStatus_SystemSimulationRunning) {
        _SensorVoltage = _SensorPressureSimulated;      // return _SensorPressureSimulated;
    }
    //if (MO_SensorZeroAdjusted != true)  return -1;

    if (MO_SensorType == SensorType_1Operation_Sensor1)
        _SensorVoltage = _Sensor1ResultVol;     // return _Sensor1ResultVol;
    else if (MO_SensorType == SensorType_2Operation_Sensor1High) {
        // _Sensor1Pressure 값이 CROSSOVER_RATIO_LOW, CROSSOVER_RATIO_HIGH 사이에 있으면, 값을 mix
        if (_Sensor1ResultVol >= _CrossOverRangeHigh)
            _SensorVoltage = _Sensor1ResultVol;     // return _Sensor1ResultVol;
        else if(_Sensor1ResultVol < _CrossOverRangeHigh){
            if(_Sensor1ResultVol < _CrossOverRangeLow)
                _SensorVoltage = _Sensor2ResultVol;     // return _Sensor2ResultVol;
            else
                _SensorVoltage = (_Sensor1ResultVol + _Sensor2ResultVol)/2;     // return (_Sensor1ResultVol + _Sensor2ResultVol) / 2;
        }
    }
    else if (MO_SensorType == SensorType_1Operation_Sensor2)
        _SensorVoltage = _Sensor2ResultVol;     // return _Sensor2ResultVol;
    else if (MO_SensorType == SensorType_2Operation_Sensor2High) {
        // MO_Sensor2Pressure 값이 CROSSOVER_RATIO_LOW, CROSSOVER_RATIO_HIGH 사이에 있으면, 값을 mix
        if (_Sensor2ResultVol >= _CrossOverRangeHigh)
            _SensorVoltage = _Sensor2ResultVol;     // return _Sensor2ResultVol;
        else if(_Sensor2ResultVol < _CrossOverRangeHigh){
            if(_Sensor2ResultVol < _CrossOverRangeLow)
                _SensorVoltage = _Sensor1ResultVol;     // return _Sensor1ResultVol;
            else
                _SensorVoltage = (_Sensor2ResultVol + _Sensor1ResultVol)/2;     // return (_Sensor2ResultVol + _Sensor1ResultVol) / 2;
        }
    }
    return _SensorVoltage;
}

int getCurrentPressure(void)
{
    int volt = getCurrentSensorVoltage();
    int pre_pr = 0;
    int pr = 0;

    if(MO_SensorScale > 10000){
        pre_pr = volt *(MO_SensorScale / 10000);
        pr = pre_pr / 1000;
    }
    else{
        pre_pr = volt * (MO_SensorScale / 1000);
        pr = pre_pr / 10000;
    }
    return pr;
/*    if(MO_SensorScale > 1000)
        return getCurrentSensorVoltage() * (MO_SensorScale / 1000) / 10000;
    else
        return getCurrentSensorVoltage() * (MO_SensorScale / 100) / 100000;*/
}

/**
 * SimulationStatus 상태에서 값을 계산
 */
int getCurrentSensorPressureSimulation(int valve_position)
{
    if (valve_position >= 0) {
        int pv = (int) (calculatePV(valve_position)* MO_SensorScale) / 10;
        _SensorPressureSimulated = pv >= PRESSURE_RANGE_MAX ? PRESSURE_RANGE_MAX : pv % PRESSURE_RANGE_MAX;
    } else {
        _SensorPressureSimulated = 0; //millis() % 10000000;
    }
    return _SensorPressureSimulated;
}

void setSensor1Equip(uint8_t equip)
{
    MO_Sensor1Equip = equip;
}

void setSensor1Selection(uint8_t selection)
{
    MO_Sensor1Selection = selection;
}

uint8_t getSensor1Selection(void)
{
    return MO_Sensor1Selection;
}

void setSensor1Unit(uint8_t unit)
{
    MO_Sensor1Unit = unit;
}

uint8_t getSensor1Unit(void)
{
    return MO_Sensor1Unit;
}

void setSensor1FullScale(uint32_t sfs)
{
    MO_Sensor1FullScale = sfs;
}

uint32_t getSensor1FullScale(void)
{
    return MO_Sensor1FullScale;
}

bool setSensorOffset(int offset1, int offset2)
{
    if(syncValveCount() == true){
        MO_Sensor1Offset = offset1;
        MO_Sensor2Offset = offset2;
        return true;
    }
    return false;
}

int getSensor1Offset(void)
{
    return MO_Sensor1Offset;
}

int getSensor1OffsetPressure(void)
{
    return getSensor1Pressure(MO_Sensor1Offset);      // (MO_Sensor1Offset * (MO_Sensor1FullScale / VOLTAGE_SCALE));
}

int getSensor1MeasVoltage(void)
{
    return _Sensor1Value;
}

int getSensor1ResultVoltage(void)
{
    return _Sensor1ResultVol;
}

int getSensor1MeasPressure(void)
{
    return getSensor1Pressure(_Sensor1Value);
}

int getCurrentSensor1Pressure(void)
{
    return getSensor1Pressure(_Sensor1ResultVol);
}

uint8_t getSensor1ApplyRatio(void)
{
    return MO_Sensor1ApplyRatio;
}

/*
 * Convert the Sensor 1 Voltage Value to Pressure Value.
 */
int getSensor1Pressure(int nVoltage)
{
    if(MO_Sensor1FullScale > 1000)
        return (int)(nVoltage * (MO_Sensor1FullScale / 1000) / 10000);
    else
        return (int)(nVoltage * (MO_Sensor1FullScale / 100) / 100000);
}

void setSensor2Equip(uint8_t equip)
{
    MO_Sensor2Equip = equip;
}

void setSensor2Selection(uint8_t selection)
{
    MO_Sensor2Selection = selection;
}

uint8_t getSensor2Selection(void)
{
    return MO_Sensor2Selection;
}

void setSensor2Unit(uint8_t unit)
{
    MO_Sensor2Unit = unit;
}

uint8_t getSensor2Unit(void)
{
    return MO_Sensor2Unit;
}

void setSensor2FullScale(uint32_t sfs)
{
    MO_Sensor2FullScale = sfs;
}

uint32_t getSensor2FullScale(void)
{
    return MO_Sensor2FullScale;
}

int getSensor2Offset(void)
{
    return MO_Sensor2Offset;
}

int getSensor2OffsetPressure(void)
{
    return getSensor2Pressure(MO_Sensor2Offset);      // (MO_Sensor2Offset * (MO_Sensor2FullScale / VOLTAGE_SCALE));
}

int getSensor2MeasVoltage(void)
{
    return _Sensor2Value;
}

int getSensor2MeasPressure(void)
{
    return getSensor2Pressure(_Sensor2Value);
}

int getSensor2ResultVoltage()
{
    return _Sensor2ResultVol;
}

int getCurrentSensor2Pressure(void)
{
    return getSensor2Pressure(_Sensor2ResultVol);
}

uint8_t getSensor2ApplyRatio(void)
{
    return MO_Sensor2ApplyRatio;
}

/*
 * Convert the Sensor 2 Voltage Value to Pressure Value.
 */
int getSensor2Pressure(int nVoltage)
{
    if(MO_Sensor2FullScale > 1000)
        return (int)(nVoltage * (MO_Sensor2FullScale / 1000) / 10000);
    else
        return (int)(nVoltage * (MO_Sensor2FullScale / 100) / 100000);
}

uint32_t getPressureCommRange(void)
{   // 1,000 ~ 1,000,000
    return MO_PressureCommRange;
}

void setPressureCommRange(uint32_t range)
{
    MO_PressureCommRange = range;
}


/*
 * used in the simulation only
 */
double calculatePV(uint32_t _valvePosition)
{
    double zCalculatedPV = 0.0;
    double zValvePosPercent;
    switch(getPositionCommRange()){
        case 0:
            zValvePosPercent = (double)(_valvePosition / 1000.0);
            break;
        case 1:
            zValvePosPercent = (double)(_valvePosition / 10000.0);
            break;
        case 2:
            zValvePosPercent = (double)(_valvePosition / 100000.0);
            break;
    }

    if (getControlMode() == ControlMode_CLOSED) {
        zCalculatedPV = (double) 1000.0;
        return zCalculatedPV;
    }

    if(zValvePosPercent < 0.2) {
        zCalculatedPV = (double) 990.0;
        //printf(" (cal 2) ");
    }
    else if(zValvePosPercent >= 0.2 &&  zValvePosPercent < 0.4) {
        zCalculatedPV = (double) 980.0;
        //printf(" (cal 3) ");
    }
    else if(zValvePosPercent >= 0.4 &&  zValvePosPercent < 0.6) {
        zCalculatedPV = (double) 975.0;
        //printf(" (cal 4) ");
    }
    else if(zValvePosPercent >= 0.6 && zValvePosPercent < 13.0) {
        zCalculatedPV = (double) (1175.6*exp(-0.256*zValvePosPercent)); // exp() 방정식 적용 : 1175.6*EXP(-0.256*X)
        //printf(" (cal 5) ");
    }
    else if(zValvePosPercent >= 13.0 && zValvePosPercent < 15.0) {
        zCalculatedPV = (double) (41.10);                               // 상수 적용(41.10)
        //printf(" (cal 6) ");
    }
    else if(zValvePosPercent >= 15.0 && zValvePosPercent < 38.0) {
        zCalculatedPV = (double) (-8.53*log(zValvePosPercent)+63.20);    // ln() 방정식 적용 : -8.53*ln(X)+63.20
        //printf(" (cal 7) ");
    }
    else if(zValvePosPercent >= 38.0 && zValvePosPercent <= 100.0) {
        zCalculatedPV = (double) (32.1);      // 상수 적용(점근선:32.1)
        //printf(" (cal 8) ");
    }
    else {
        zCalculatedPV = (double) (30.1);      // 상수 적용(점근선:32.1)
        //printf(" (cal 9) ");
    }

    if (zCalculatedPV >= (double) 1000.0) {
        zCalculatedPV = (double) 990.0;
    }

    return zCalculatedPV;
}

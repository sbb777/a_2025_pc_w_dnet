/*
 * APC_Sensor.h
 *
 *  Created on: Oct 18, 2023
 *      Author: Yongseok
 */

#ifndef _APC_SENSOR_H_
#define _APC_SENSOR_H_

#include <APC_Define.h>

#define SENSOR_SENSOR1    _DEF_CH1
#define SENSOR_SENSOR2    _DEF_CH2

#define PRESSURE_RANGE_MIN      0
#define PRESSURE_RANGE_MAX      10000000

#define SENSOR_OFFSET_MIN       (-1400000)
#define SENSOR_OFFSET_MAX       ( 1400000)

/*********************************************************************************/
// Board(128) + Valve(64)
#define ADDR_SENSOR_START           (_ADDR_MO_DATA + 128 + 64)

#define ADDR_Sensor1Equip           (ADDR_SENSOR_START +  0)
#define ADDR_Sensor1Selection       (0x080C0000)            //(ADDR_SENSOR_START +  1)
#define ADDR_Sensor1Unit            (ADDR_SENSOR_START +  2)
#define ADDR_Sensor1ApplyRatio      (ADDR_SENSOR_START +  3)
#define ADDR_Sensor1FullScale       (ADDR_SENSOR_START +  4)
#define ADDR_Sensor1Offset          (0x080C0004)            //(ADDR_SENSOR_START +  8)

#define ADDR_Sensor2Equip           (ADDR_SENSOR_START + 20)
#define ADDR_Sensor2Selection       (0x080C0001)            //(ADDR_SENSOR_START + 21)
#define ADDR_Sensor2Unit            (ADDR_SENSOR_START + 22)
#define ADDR_Sensor2ApplyRatio      (ADDR_SENSOR_START + 23)
#define ADDR_Sensor2FullScale       (ADDR_SENSOR_START + 24)
#define ADDR_Sensor2Offset          (0x080C0008)            //(ADDR_SENSOR_START + 28)

#define ADDR_SensorType             (ADDR_SENSOR_START + 40)
#define ADDR_SensorZeroStatus       (ADDR_SENSOR_START + 41)
#define ADDR_SensorZeroAdjusted     (ADDR_SENSOR_START + 42)

#define ADDR_CrossRangeHigh         (ADDR_SENSOR_START + 44)
#define ADDR_CrossRangeLow          (ADDR_SENSOR_START + 45)
//#define ADDR_SensorFullScaleRatio   (ADDR_SENSOR_START + 44)    // 4byte
#define ADDR_PressureCommRange      (ADDR_SENSOR_START + 48)    // 4byte

#define ADDR_SensorScale            (ADDR_SENSOR_START + 52)
#define ADDR_SensorUnit             (ADDR_SENSOR_START + 56)
/***********************************************************************************/


typedef enum {
    SensorType_NoSensor                = 0,
    SensorType_1Operation_Sensor1      = 1,
    SensorType_2Operation_Sensor1High  = 2,
    SensorType_1Operation_Sensor2      = 3,
    SensorType_2Operation_Sensor2High  = 4
} EN_SensorType;

typedef enum {
    Sensor_Unselected   = 0,
    Sensor_Selected     = 1
} EN_SensorSelect;

typedef enum {
    SensorUnit_Pa       = 0,
    SensorUnit_bar      = 1,
    SensorUnit_mbar     = 2,
    SensorUnit_ubar     = 3,
    SensorUnit_Torr     = 4,
    SensorUnit_mTorr    = 5,
    SensorUnit_atm      = 6,
    SensorUnit_psi      = 7,
    SensorUnit_psf      = 8
} EN_SensorUnit;

typedef enum {
    PressureUnit_Pa     = 0,
    PressureUnit_bar    = 1,
    PressureUnit_mbar   = 2,
    PressureUnit_ubar   = 3,
    PressureUnit_Torr   = 4,
    PressureUnit_mTorr  = 5,
    PressureUnit_atm    = 6,
    PressureUnit_psi    = 7,
    PressureUnit_psf    = 8,
    PressureUnit_Percent = 9,
    PressureUnit_Permil = 10,   // A
    PressureUnit_Volt   = 11,   // B
    PressureUnit_None   = 12    // C
} EN_A_PressureUnit;


bool initSensor(void);
void alignSensor(void);

bool syncSensorData(uint32_t addr);

void refreshSensor(void);
bool adjustSensorZero(uint8_t option);

int getSensorValue(uint8_t ch);

void setSensorType(uint8_t sensor_type);
uint8_t getSensorType(void);

void setSensorUnit(uint8_t sensor_unit);
uint8_t getSensorUnit(void);

void setSensorFullScale(uint32_t sfs);
int getSensorFullScale(void);

void setSensorExponent(int exponent);
int getSensorExponent(void);

void setSensorZeroStatus(uint8_t status);
uint8_t getSensorZeroStatus(void);

void setSensorZeroAdjusted(uint8_t status);
uint8_t getSensorZeroAdjusted(void);

void setSensorFullScaleRatio(uint32_t ratio);
uint32_t getSensorFullScaleRatio(void);

void setSetpointValvePressure(uint32_t setpoint);
uint32_t getSetpointValvePressure(void);

int getCurrentSensorVoltage(void);
int getCurrentPressure(void);
int getCurrentSensorPressureSimulation(int valve_position);
int getSensor1Pressure(int nVoltage);
int getSensor2Pressure(int nVoltage);

void setSensor1Equip(uint8_t equip);

void setSensor1Selection(uint8_t selection);
uint8_t getSensor1Selection(void);

void setSensor1Unit(uint8_t unit);
uint8_t getSensor1Unit(void);

void setSensor1FullScale(uint32_t sfs);
uint32_t getSensor1FullScale(void);

bool setSensorOffset(int offset1, int offset2);
void setSensor1Offset(int offset);
int getSensor1Offset(void);
int getSensor1MeasVoltage(void);
int getSensor1ResultVoltage(void);

int getSensor1OffsetPressure(void);
int getSensor1MeasPressure(void);
int getCurrentSensor1Pressure(void);
uint8_t getSensor1ApplyRatio(void);

void setSensor2Equip(uint8_t equip);

void setSensor2Selection(uint8_t selection);
uint8_t getSensor2Selection(void);

void setSensor2Unit(uint8_t unit);
uint8_t getSensor2Unit(void);

void setSensor2FullScale(uint32_t sfs);
uint32_t getSensor2FullScale(void);

void setSensor2Offset(int offset);
int getSensor2Offset(void);
int getSensor2MeasVoltage(void);
int getSensor2ResultVoltage(void);

int getSensor2OffsetPressure(void);
int getSensor2MeasPressure(void);
int getCurrentSensor2Pressure(void);
uint8_t getSensor2ApplyRatio(void);

uint32_t getPressureCommRange(void);   // 1,000 ~ 1,000,000
void setPressureCommRange(uint32_t range);

double calculatePV(uint32_t _valvePosition);

#endif /* _APC_SENSOR_H_ */

/*
 * APC_Board.c
 *
 *  Created on: 2023. 11. 12.
 *      Author: Yongseok
 */

#include <APC_Board.h>
#include <APC_Utils.h>

#include <APC_ChipSelect.h>
#include <APC_Sensor.h>
#include <APC_Timer.h>
#include <APC_Flash.h>
#include <APC_Power.h>
#include <APC_CompAir.h>

/*********************************************************************************/
#define ADDR_BOARD_START            (_ADDR_MO_DATA)

#define ADDR_ValveSerialNumber      (ADDR_BOARD_START +  0)
#define ADDR_ValveModelName         (ADDR_BOARD_START + 20)
#define ADDR_FwVersion              (ADDR_BOARD_START + 40)
#define ADDR_FwReleasedDate         (ADDR_BOARD_START + 60)

#define ADDR_PFOEquip               (ADDR_BOARD_START + 80)
#define ADDR_SensorEquip            (ADDR_BOARD_START + 81)
#define ADDR_SPSEquip               (ADDR_BOARD_START + 82)
#define ADDR_ClusterEquip           (ADDR_BOARD_START + 83)
#define ADDR_InterfaceType          (ADDR_BOARD_START + 84)
#define ADDR_RS232InterfaceAOEquip  (ADDR_BOARD_START + 85)
#define ADDR_PFOStatus              (ADDR_BOARD_START + 86)

/*********************************************************************************/
static bool         _board_initialized;

static bool         _FwFactoryState   ;    // 0: Not factory state, 1: factory state
static EN_SimulationStatus  _SimulationStatus;

static uint8_t          MO_ValveSerialNumber[MO_STRING_SIZE+1]; // "00001234";
static uint8_t          MO_ValveModelName[MO_STRING_SIZE+1]   ; // "P250M01R";
static uint8_t          MO_FwVersion[MO_STRING_SIZE+1]        ; // "APC-V100-2024";
static uint8_t          MO_FwReleasedDate[MO_STRING_SIZE+1]   ; // "20240321KST142000";

static EN_SensorEquip   MO_SensorEquip ;
static EN_DevEquip      MO_PFOEquip    ;
static EN_DevEquip      MO_SPSEquip    ;
static EN_DevEquip      MO_ClusterEquip;
static EN_InterfaceType MO_InterfaceType;
static EN_DevEquip      MO_RS232_AOEquip;   // RS232 AnalogOutput
//uint32_t                MO_ExtIsolationValveFunc;   // moved to APC_Valve.c
static EN_Status        MO_PFOStatus   ;

int                 _CurrentPFOVoltage      = 0;
uint32_t            _CompressedAirPressure  = 0;    // Value가 0 이면 동작을 안하고 있다는 의미

/*********************************************************************************/
bool initBoard(void)
{
    initChipSelect();
    initTimer();
    initFlash();

    readFlash(ADDR_ValveSerialNumber, MO_ValveSerialNumber, MO_STRING_SIZE);
    readFlash(ADDR_ValveModelName,    MO_ValveModelName,    MO_STRING_SIZE);
    int errCount1 = 0, errCount2 = 0;
    for (int i = 0; i < MO_STRING_SIZE; i++) {
        if (MO_ValveSerialNumber[i] == 0xff) {
            MO_ValveSerialNumber[i] = 0x00; errCount1++;
        }
        if (MO_ValveModelName[i] == 0xff) {
            MO_ValveModelName[i] = 0x00;    errCount2++;
        }
    }
    readFlash(ADDR_FwVersion,         MO_FwVersion,         MO_STRING_SIZE);
    readFlash(ADDR_FwReleasedDate,    MO_FwReleasedDate,    MO_STRING_SIZE);

    MO_PFOEquip      = readByteFromFlash(ADDR_PFOEquip);
    MO_SensorEquip   = readByteFromFlash(ADDR_SensorEquip);
    MO_SPSEquip      = readByteFromFlash(ADDR_SPSEquip);
    MO_ClusterEquip  = readByteFromFlash(ADDR_ClusterEquip);
    MO_InterfaceType = readByteFromFlash(ADDR_InterfaceType);
    MO_RS232_AOEquip = readByteFromFlash(ADDR_RS232InterfaceAOEquip);
    MO_PFOStatus         = readByteFromFlash(ADDR_PFOStatus);

    if (MO_PFOEquip == 0xff)        MO_PFOEquip = Dev_NotEquipped;
    if (MO_SensorEquip == 0xff)     MO_SensorEquip = 0;
    if (MO_SPSEquip == 0xff)        MO_SPSEquip = Dev_NotEquipped;
    if (MO_ClusterEquip == 0xff)    MO_ClusterEquip = Dev_NotEquipped;
    if (MO_InterfaceType == 0xff)   MO_InterfaceType = InterfaceType_None;
    if (MO_RS232_AOEquip == 0xff)   MO_RS232_AOEquip = Dev_NotEquipped;

    //if (MO_PFOStatus == 0xff){
        if(MO_PFOEquip == Dev_NotEquipped)
            setPFOStatus(Status_Disabled);
        else
            setPFOStatus(Status_Enabled);
    //}

    if (errCount1 != MO_STRING_SIZE && errCount2 != MO_STRING_SIZE) {
        _board_initialized = true;
        _FwFactoryState = false;
    } else {
        _board_initialized = false;
        _FwFactoryState = true;
    }

    return _board_initialized;
}

bool syncBoardData(uint32_t addr)
{
    bool result = true;
    if (addr == _ADDR_MO_DATA) {
        result &= writeFlash(ADDR_ValveSerialNumber, MO_ValveSerialNumber, MO_STRING_SIZE);
        result &= writeFlash(ADDR_ValveModelName,    MO_ValveModelName,    MO_STRING_SIZE);

        uint8_t fwVersion[MO_STRING_SIZE + 1] = { 0 };
        uint8_t fwReleasedDate[MO_STRING_SIZE + 1] = { 0 };
        sprintf((char *) fwVersion,      "%s%04d", "APC-V100-", APC_BUILD_YEAR);
        sprintf((char *) fwReleasedDate, "%04d%02d%02d%s%02d%02d%02d",
                APC_BUILD_YEAR, APC_BUILD_MONTH, APC_BUILD_DAY, "KST", APC_BUILD_HOUR, APC_BUILD_MIN, APC_BUILD_SEC);
        result &= writeFlash(ADDR_FwVersion,         fwVersion,         MO_STRING_SIZE);
        result &= writeFlash(ADDR_FwReleasedDate,    fwReleasedDate,    MO_STRING_SIZE);

        result &= writeByteToFlash(ADDR_PFOEquip,    MO_PFOEquip);
        result &= writeByteToFlash(ADDR_SensorEquip, MO_SensorEquip);
        result &= writeByteToFlash(ADDR_SPSEquip,    MO_SPSEquip);
        result &= writeByteToFlash(ADDR_ClusterEquip, MO_ClusterEquip);
        result &= writeByteToFlash(ADDR_InterfaceType, MO_InterfaceType);
        result &= writeByteToFlash(ADDR_RS232InterfaceAOEquip, MO_RS232_AOEquip);
        result &= writeByteToFlash(ADDR_PFOStatus,   MO_PFOStatus);
    }
    return result;
}

/**
 * 0: False, 1: True
 */
bool getFwFactoryState(void)
{
    return _FwFactoryState;
}

void setFwFactoryState(bool state)
{
    _FwFactoryState = state;
}

void getValveSerialNumber(char *pData)
{
    memset( pData,                          0x00, strlen(pData));
    memcpy( pData, (char *) MO_ValveSerialNumber, MO_STRING_SIZE);
}

void setValveSerialNumber(char *pData, uint8_t length)
{
    memset((char *) MO_ValveSerialNumber,  0x00, MO_STRING_SIZE);
    memcpy((char *) MO_ValveSerialNumber, pData, length < MO_STRING_SIZE ? length : MO_STRING_SIZE);
}

void getValveModelName(char *pData)
{
    memset( pData,                       0x00, strlen(pData));
    memcpy( pData, (char *) MO_ValveModelName, MO_STRING_SIZE);
}

void setValveModelName(char *pData, uint8_t length)
{
    memset((char *) MO_ValveModelName,  0x00, MO_STRING_SIZE);
    memcpy((char *) MO_ValveModelName, pData, length < MO_STRING_SIZE ? length : MO_STRING_SIZE);
}

void getFwVersion(char *pData)
{
    memset( pData,                  0x00, strlen(pData));
    memcpy( pData, (char *) MO_FwVersion, MO_STRING_SIZE);
}

void setFwVersion(char *pData, uint8_t length)
{
    memset((char *) MO_FwVersion,  0x00, MO_STRING_SIZE);
    memcpy((char *) MO_FwVersion, pData, length < MO_STRING_SIZE ? length : MO_STRING_SIZE);
}

void getFwReleasedDate(char *pData)
{
    memset( pData,                       0x00, strlen(pData));
    memcpy( pData, (char *) MO_FwReleasedDate, MO_STRING_SIZE);
}

void setFwReleasedDate(char *pData, uint8_t length)
{
    memset((char *) MO_FwReleasedDate,  0x00, MO_STRING_SIZE);
    memcpy((char *) MO_FwReleasedDate, pData, length < MO_STRING_SIZE ? length : MO_STRING_SIZE);
}

EN_SensorEquip getSensorEquip(void)
{
    return MO_SensorEquip;
}

void setSensorEquip(EN_SensorEquip equip)
{
    MO_SensorEquip = equip;
}

EN_DevEquip getPFOEquip(void)
{
    return MO_PFOEquip;
}

void setPFOEquip(EN_DevEquip equip)
{
    MO_PFOEquip = equip;
}

EN_DevEquip getSPSEquip(void)
{
    return MO_SPSEquip;
}

void setSPSEquip(EN_DevEquip equip)
{
    MO_SPSEquip = equip;
}

EN_DevEquip getClusterEquip(void)
{
    return MO_ClusterEquip;
}

void setClusterEquip(EN_DevEquip equip)
{
    MO_ClusterEquip = equip;
}

EN_DevEquip getRS232IntfAnalogOutputEquip(void)
{
    return MO_RS232_AOEquip;
}

void setRS232IntfAnalogOutputEquip(EN_DevEquip equip)
{
    MO_RS232_AOEquip = equip;
}

EN_InterfaceType getInterfaceType(void)
{
    return MO_InterfaceType;
}

void setInterfaceType(EN_InterfaceType type)
{
    MO_InterfaceType = type;
}

EN_SimulationStatus getSimulationStatus(void)
{
    return _SimulationStatus;
}

void setSimulationStatus(EN_SimulationStatus status)
{
    _SimulationStatus = status;
}

EN_Status getPFOStatus(void)
{
    return MO_PFOStatus;
}

/**
 * If disabled,
 */
void setPFOStatus(EN_Status status)
{
    MO_PFOStatus = status;
    setPower(status);
}

int getCurrentPFOVoltage(void)
{
    _CurrentPFOVoltage = getPower(_DEF_CH1);
    return _CurrentPFOVoltage;
}

void setCurrentPFOVoltage(int voltage)
{
    _CurrentPFOVoltage = voltage;
}

uint32_t getCompressedAirPressure(void)
{
    _CompressedAirPressure = getCompAirValue();
    return _CompressedAirPressure;
}

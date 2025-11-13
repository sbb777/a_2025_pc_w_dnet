/*
 * APC_Board.h
 *
 *  Created on: 2023. 11. 12.
 *      Author: Yongseok
 */

#ifndef _APC_BOARD_H_
#define _APC_BOARD_H_

#include <APC_Define.h>
#include <APC_Model.h>

bool initBoard(void);

bool syncBoardData(uint32_t addr);

bool getFwFactoryState(void);
void setFwFactoryState(bool state);

void getValveSerialNumber(char *pData);
void setValveSerialNumber(char *pData, uint8_t length);

void getValveModelName(char *pData);
void setValveModelName(char *pData, uint8_t length);

void getFwVersion(char *pData);
void setFwVersion(char *pData, uint8_t length);

void getFwReleasedDate(char *pData);
void setFwReleasedDate(char *pData, uint8_t length);

EN_SensorEquip getSensorEquip(void);
void setSensorEquip(EN_SensorEquip equip);

EN_DevEquip getPFOEquip(void);
void setPFOEquip(EN_DevEquip equip);

EN_DevEquip getSPSEquip(void);
void setSPSEquip(EN_DevEquip equip);

EN_DevEquip getClusterEquip(void);
void setClusterEquip(EN_DevEquip equip);

EN_DevEquip getRS232IntfAnalogOutputEquip(void);
void setRS232IntfAnalogOutputEquip(EN_DevEquip equip);

//uint32_t getExtIsolationValveFunc(void);
//void setExtIsolationValveFunc(uint32_t valve_func); // XXX

EN_InterfaceType getInterfaceType(void);
void setInterfaceType(EN_InterfaceType type);

EN_SimulationStatus getSimulationStatus(void);
void setSimulationStatus(EN_SimulationStatus status);

EN_Status getPFOStatus(void);
void setPFOStatus(EN_Status status);

int  getCurrentPFOVoltage(void);
void setCurrentPFOVoltage(int voltage);

uint32_t getCompressedAirPressure(void);


#endif /* _APC_BOARD_H_ */

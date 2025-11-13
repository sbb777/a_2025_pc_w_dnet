/*
 * APC_ControlMode.h
 *
 *  Created on: Apr 11, 2024
 *      Author: Yongseok
 */

#ifndef _APC_CONTROLMODE_H_
#define _APC_CONTROLMODE_H_

#include <APC_Define.h>

typedef enum {
    ControlMode_Initialized         = 0,
    ControlMode_Synchronization     = 1,
    ControlMode_POSITION            = 2,    // V
    ControlMode_CLOSED              = 3,
    ControlMode_OPEN                = 4,
    ControlMode_PRESSURE            = 5,
    ControlMode_HOLD                = 6,
    ControlMode_LEARN               = 7,
    ControlMode_INTERLOCK_OPEN      = 8,
    ControlMode_INTERLOCK_CLOSED    = 9,
    ControlMode_MaintenanceOpen     = 10,   // A
    ControlMode_MaintenanceClose    = 11,   // B
    ControlMode_PowerFailure        = 12,   // C
    ControlMode_SafetyMode          = 13,   // D
    ControlMode_FatalError          = 14,   // E
    ControlMode_Service             = 15,   // F
    ControlMode_CLOSING             = 16,
    ControlMode_OPENING             = 17
} EN_A_ControlMode;


typedef enum {
    SimulDemonMode_Simulation     = 0,
    SimulDemonMode_Demonstration  = 1
} EN_SimulDemonMode;


bool initControlMode(void);

uint8_t setControlMode(uint8_t ch, uint8_t mode);
uint8_t getControlMode(void);

void setSimulDemonMode(int mode);
int  getSimulDemonMode(void);


#endif /* _APC_CONTROLMODE_H_ */

/*
 * APC_Warning.h
 *
 *  Created on: 2023. 11. 13.
 *      Author: Yongseok
 */

#ifndef _APC_WARNING_H_
#define _APC_WARNING_H_

#include <APC_Define.h>


typedef enum {
    WarningStatus_NoWarnings    = 0,
    WarningStatus_Warnings      = 1
} EN_WarningStatus;

typedef enum {
    WarningService_NoRequest        = 0,
    WarningService_Request          = 1
} EN_WarningServiceRequest;

typedef enum {
    WarningLearnDataset_Present     = 0,
    WarningLearnDataset_NotPresent  = 1
} EN_WarningLearnDatasetPresent;

typedef enum {
    WarningPFOBattery_Ready     = 0,
    WarningPFOBattery_NotReady  = 1
} EN_WarningPFOBattery;

typedef enum {
    WarningCompressedAir_Ok     = 0,
    WarningCompressedAir_Nok    = 1
} EN_WarningCompressedAir;

typedef enum {
    Warning_ServiceRequest      = 0,
    Warning_LearnParameter      = 1,
    Warning_CompressedAir       = 2,
    Warning_PFOnotReady         = 3,
    Warning_SensorFactor        = 4,
    Warning_IsolationValve      = 5,
    Warning_SlaveOffline        = 6,
    Warning_NetworkFailure      = 7,
    Warning_FirmwareMemory      = 8,
    Warning_UnknownInterface    = 9,
    Warning_NoSensorSignal      = 10,
    Warning_NoAnalogSignal      = 11,
    Warning_Reserved_12         = 12,
    Warning_Reserved_13         = 13,
    Warning_Reserved_14         = 14,
    Warning_Reserved_15         = 15
} EN_Warning;

void initWarning(void);
//uint8_t getWarningCode(void);
bool    setWarningCode(uint8_t warn_no);

uint8_t getWarningStatus(void);
uint8_t getWarningServiceRequest(void);
uint8_t getWarningLearnDatasetPresent(void);
uint8_t getWarningCompressedAir(void);
uint8_t getWarningPFOBattery(void);

#endif /* _APC_WARNING_H_ */

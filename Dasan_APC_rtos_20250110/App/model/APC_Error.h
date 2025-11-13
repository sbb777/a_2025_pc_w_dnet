/*
 * APC_Error.h
 *
 *  Created on: 2023. 11. 12.
 *      Author: Yongseok
 */

#ifndef _APC_ERROR_H_
#define _APC_ERROR_H_

#include <APC_Define.h>

typedef enum {
    ResetError_ServiceRequestFromWARNINGS   = 0,
    ResetError_FATAL_ERROR                  = 1
} EN_ResetError;

typedef enum {
    Transmission_ParityFailure                  = 1,
    Transmission_InputBufferTooMuchCharacters   = 2,
    Transmission_FramingError                   = 3,
    Transmission_InputBufferTooMuchCommands     = 4,
    Cluster_ChecksumFailure                     = 5,
    //TimeoutExpired          = 6,
    Format_TerminationCharactorMissing          = 10,
    Format_ColonMissing                         = 11,
    Format_WrongNumberCharacters                = 12,
    Format_InvalidCommandCharacter              = 20,
    Format_InvalidCommandType                   = 21,
    Format_InvalidNumberRange                   = 22,
    Format_CommandInterpretationFailure         = 23,
    Format_CommandOnlyGetCommand                = 24,
    Format_CommandParameterOutOfRange           = 30,

    Hardware_InvalidCommandMissingSensor        = 40,
    Hardware_InvalidCommandMissingHardware      = 41,
    Hardware_InvalidCommandMissingHardware2     = 42,

    Setting_InvalidSetupToPerformZero           = 60,
    Setting_NoLearnDataset                      = 61,

    Status_InvalidPosition                      = 70,
    Status_InvalidPressure                      = 71,
    Status_InvalidValue                         = 72,

    Status_ProcessingJobExist                   = 75,   // added by 2024-02-07

    Status_WrongAccessMode                      = 80,
    Status_WrongAccessModeLocked                = 81,
    Status_WrongControlMode                     = 82,

    Cluster_SlaveNotFreezeMode                  = 83,
    Cluster_IllegalSlaveCommand                 = 84,
    Cluster_UnknownSalveAddress                 = 85,
    Cluster_SalveCommandOnly                    = 86,
    Cluster_MasterCommandOnly                   = 87,
    Cluster_IndivisualSlaveCommandRequired      = 88,
    Status_WrongControlModeCalibration          = 89,
    Status_InvalidPressureControlAlgorithm      = 92,

    Test_InternalDebugCondition                 = 99,
    Ethernet_ErrorResponseDuringConfig          = 100,
    RSinterface_ParityFailure                   = 101,
    RSinterface_FramingError                    = 103,
    RSinterface_BufferOverrun                   = 104
} EN_ErrorCode;

typedef enum {
    E1_NoValueDefined                       = 1,
    E20_HomingError                         = 20,
    E21_RestrictedFullStroke_Synch          = 21,
    E22_RestrictedFullStroke_Oper           = 22,
    E23_FailureAxis1                        = 23,
    E24_FailureAxis2                        = 24,
    E25_FailureAxis1_2                      = 25,
    E26_MultipleAxisMisalign                = 26,
    E27_ComputationError                    = 27,
    E28_RecoverError                        = 28,
    E40_MotorDriverFailure                  = 40
} EN_FatalErrorCode;


void initError(void);

uint8_t getErrorCode(void);
bool setErrorCode(uint8_t err_num);

uint8_t getFatalErrorCode(void);
bool setFatalErrorCode(uint8_t err_num);

void resetError(uint8_t option);

#endif /* _APC_ERROR_H_ */

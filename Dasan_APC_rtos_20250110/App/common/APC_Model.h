/*
 * APC_Model.h
 *
 *  Created on: Apr 10, 2024
 *      Author: Yongseok
 */

#ifndef _APC_MODEL_H_
#define _APC_MODEL_H_


#define MO_STRING_SIZE              ( 20)

/*************************************************************************************************/
typedef enum {
    InterfaceType_None              = 0,
    InterfaceType_LogicIO           = 1,
    InterfaceType_RS232             = 2,
    InterfaceType_RS232_AO          = 3,
    InterfaceType_DeviceNet         = 4,
    InterfaceType_Unknown           = 5,
    InterfaceType_Ethernet          = 6,
    InterfaceType_Ethernet_AO       = 7,
    InterfaceType_RS232_485         = 8,
    InterfaceType_RS232_485_AO      = 9,
    InterfaceType_Busmodule         = 10,   // A
    InterfaceType_Busmodule_Unknown = 11,   // B
    InterfaceType_Profibus          = 12,   // C
    InterfaceType_Unknown2          = 13,   // D
    InterfaceType_RS232_485_volt    = 14,   // E
    InterfaceType_RS232_485_volt_AO = 15,   // F
    InterfaceType_Logic4_20mA       = 16,   // G
    InterfaceType_Logic4_20mA_SP    = 17,   // H
    InterfaceType_CC_Link           = 18,   // I
    InterfaceType_EtherCAT          = 19    // J
} EN_InterfaceType;

typedef enum {
    Status_Disabled       = 0,
    Status_Enabled        = 1
} EN_Status;

typedef enum {
    Dev_NotEquipped     = 0,
    Dev_Equipped        = 1
} EN_DevEquip;

typedef enum {
    SensorEquip_1SensorVersion      = 1,
    SensorEquip_2SensorVersion      = 2
} EN_SensorEquip;

//typedef enum {
//    RS232Interface_NotAnalogOutput  = 2,
//    RS232Interface_AnalogOutput     = 3
//} EN_RS232IntfAnalogOutputEquip;

typedef enum {
    GasFlowUnit_sccm    = 0,
    GasFlowUnit_slm     = 1,
    GasFlowUnit_mbar    = 2,
    GasFlowUnit_Pa      = 3,
    GasFlowUnit_Torr    = 4,
    GasFlowUnit_atm     = 5,
    GasFlowUnit_lusec   = 6,
    GasFlowUnit_Mol     = 7
} EN_GasFlowUnit;


typedef enum {
    PortType_Service    = 0,
    PortType_Interface  = 1
} EN_PortType;

typedef enum {
    AxisMappingMode_linear          = 0,
    AxisMappingMode_logarithmic     = 1
} EN_AxisMappingMode;

typedef enum {
    AxisScalingMode_fixed           = 0,
    AxisScalingMode_autoscale       = 1
} EN_AxisScalingMode;

typedef enum {
    AxisVisibilityMode_hide     = 0,
    AxisVisibilityMode_show     = 1
} EN_AxisVisibilityMode;

typedef enum {
    Decades_1       = 0,
    Decades_2       = 1,
    Decades_3       = 2,
    Decades_4       = 3,
    Decades_5       = 4
} EN_Decades;

typedef enum {
    TimeRange_30sec     = 0,
    TimeRange_1min      = 1,
    TimeRange_2min      = 2,
    TimeRange_5min      = 3
} EN_TimeRange;

typedef enum {
    DuplexMode_FullDuplex       = 0,
    DuplexMode_HalfDuplex       = 1
} EN_DuplexMode;

typedef enum {
    TerminationRS232_LF     = 0,
    TerminationRS232_CR     = 1
} EN_TerminationRS232;


/*************************************************************************************************/
typedef enum {
    ControllerType_AdaptiveDnStream     = 0,
    ControllerType_FixedPI_DnStream     = 1,
    ControllerType_FixedPI_UpStream     = 2,
    ControllerType_SoftPump     = 3
} EN_ControllerType;

typedef enum {
    GainFactor_0_10     = 0,
    GainFactor_0_13     = 1,
    GainFactor_0_18     = 2,
    GainFactor_0_23     = 3,
    GainFactor_0_32     = 4,
    GainFactor_0_42     = 5,
    GainFactor_0_56     = 6,
    GainFactor_0_75     = 7,
    GainFactor_1_00     = 8,
    GainFactor_1_33     = 9,
    GainFactor_1_78     = 10,   // A
    GainFactor_2_37     = 11,   // B
    GainFactor_3_16     = 12,   // C
    GainFactor_4_22     = 13,   // D
    GainFactor_5_62     = 14,   // E
    GainFactor_7_50     = 15,   // F
    GainFactor_0_0001   = 16,   // G
    GainFactor_0_0003   = 17,   // H
    GainFactor_0_001    = 18,   // I
    GainFactor_0_003    = 19,   // J
    GainFactor_0_01     = 20,   // K
    GainFactor_0_02     = 21,   // L
    GainFactor_0_05     = 22    // M
} EN_A_GainFactor;

typedef enum {
    RampTime_0_0        = 0,
    RampTime_0_5        = 1,
    RampTime_1_0        = 2,
    RampTime_1_5        = 3,
    RampTime_2_0        = 4,
    RampTime_2_5        = 5,
    RampTime_3_0        = 6,
    RampTime_3_5        = 7,
    RampTime_4_0        = 8,
    RampTime_4_5        = 9,
    RampTime_5_0        = 10,   // A
    RampTime_5_5        = 11,   // B
    RampTime_6_0        = 12,   // C
    RampTime_6_5        = 13,   // D
    RampTime_7_0        = 14,   // E
    RampTime_7_5        = 15,   // F
    RampTime_8_0        = 16,   // G
    RampTime_8_5        = 17,   // H
    RampTime_9_0        = 18,   // I
    RampTime_9_5        = 19,   // J
    RampTime_10_0       = 20    // K
} EN_RampTime;

typedef enum {
    SensorDelay_0_00        = 0,
    SensorDelay_0_02        = 1,
    SensorDelay_0_04        = 2,
    SensorDelay_0_06        = 3,
    SensorDelay_0_08        = 4,
    SensorDelay_0_10        = 5,
    SensorDelay_0_15        = 6,
    SensorDelay_0_20        = 7,
    SensorDelay_0_25        = 8,
    SensorDelay_0_30        = 9,
    SensorDelay_0_35        = 10,   // A
    SensorDelay_0_40        = 11,   // B
    SensorDelay_0_50        = 12,   // C
    SensorDelay_0_60        = 13,   // D
    SensorDelay_0_80        = 14,   // E
    SensorDelay_1_00        = 15    // F
} EN_A_SensorDelay;

typedef enum {
    Pgain_0_0010        = 0,
    Pgain_0_0013        = 1,
    Pgain_0_0018        = 2,
    Pgain_0_0024        = 3,
    Pgain_0_0032        = 4,
    Pgain_0_0042        = 5,
    Pgain_0_0056        = 6,
    Pgain_0_0075        = 7,
    Pgain_0_010     = 8,
    Pgain_0_013     = 9,
    Pgain_0_018     = 10,
    Pgain_0_024     = 11,
    Pgain_0_032     = 12,
    Pgain_0_042     = 13,
    Pgain_0_056     = 14,
    Pgain_0_075     = 15,
    Pgain_0_10      = 16,
    Pgain_0_13      = 17,
    Pgain_0_18      = 18,
    Pgain_0_24      = 19,
    Pgain_0_32      = 20,
    Pgain_0_42      = 21,
    Pgain_0_56      = 22,
    Pgain_0_75      = 23,
    Pgain_1_0       = 24,
    Pgain_1_3       = 25,
    Pgain_1_8       = 26,
    Pgain_2_4       = 27,
    Pgain_3_2       = 28,
    Pgain_4_2       = 29,
    Pgain_5_6       = 30,
    Pgain_7_5       = 31,
    Pgain_10        = 32,
    Pgain_13        = 33,
    Pgain_18        = 34,
    Pgain_24        = 35,
    Pgain_32        = 36,
    Pgain_42        = 37,
    Pgain_56        = 38,
    Pgain_75        = 39,
    Pgain_100       = 40
} EN_Pgain;

typedef enum {
    Igain_0_0010        = 0,
    Igain_0_0013        = 1,
    Igain_0_0018        = 2,
    Igain_0_0024        = 3,
    Igain_0_0032        = 4,
    Igain_0_0042        = 5,
    Igain_0_0056        = 6,
    Igain_0_0075        = 7,
    Igain_0_010     = 8,
    Igain_0_013     = 9,
    Igain_0_018     = 10,
    Igain_0_024     = 11,
    Igain_0_032     = 12,
    Igain_0_042     = 13,
    Igain_0_056     = 14,
    Igain_0_075     = 15,
    Igain_0_10      = 16,
    Igain_0_13      = 17,
    Igain_0_18      = 18,
    Igain_0_24      = 19,
    Igain_0_32      = 20,
    Igain_0_42      = 21,
    Igain_0_56      = 22,
    Igain_0_75      = 23,
    Igain_1_0       = 24,
    Igain_1_3       = 25,
    Igain_1_8       = 26,
    Igain_2_4       = 27,
    Igain_3_2       = 28,
    Igain_4_2       = 29,
    Igain_5_6       = 30,
    Igain_7_5       = 31,
    Igain_10        = 32,
    Igain_13        = 33,
    Igain_18        = 34,
    Igain_24        = 35,
    Igain_32        = 36,
    Igain_42        = 37,
    Igain_56        = 38,
    Igain_75        = 39,
    Igain_100       = 40
} EN_Igain;

typedef enum {
    RampMode_ConstantTime       = 0,
    RampMode_ConstantSlop       = 1
} EN_RampMode;

typedef enum {
    RampType_Linear             = 0,
    RampType_Logarithmic        = 1
} EN_RampType;

typedef enum {
    ControlDirection_Downstream     = 0,
    ControlDirection_Upstream       = 1
} EN_ControlDirection;


/*************************************************************************************************/
typedef enum {
    LearnPressureLogic_Fullrange        = 0,
    LearnPressureLogic_AnalogInput      = 1
} EN_LearnPressureLogic;

typedef enum {
    LearnRunningStatus_No       = 0,
    LearnRunningStatus_Yes      = 1
} EN_LearnRunningStatus;

typedef enum {
    LearnDatasetPresentStatus_Ok        = 0,
    LearnDatasetPresentStatus_No        = 1
} EN_LearnDatasetPresentStatus;

typedef enum {
    LearnAbortStatus_Completed          = 0,
    LearnAbortStatus_AbortedByUser      = 1,
    LearnAbortStatus_AbortedByControl   = 2     // ex) when not sufficient gas flow
} EN_LearnAbortStatus;

typedef enum {
    LearnOpenPressureStatus_Ok          = 0,
    LearnOpenPressureStatus_Over50      = 1,
    LearnOpenPressureStatus_NoGas       = 2
} EN_LearnOpenPressureStatus;

typedef enum {
    LearnClosePressureStatus_Ok         = 0,
    LearnClosePressureStatus_Below10    = 1
} EN_LearnClosePressureStatus;

typedef enum {
    LearnPressureRaisingStatus_Ok       = 0,
    LearnPressureRaisingStatus_NOT      = 1
} EN_LearnPressureRaisingStatus;

typedef enum {
    LearnPressureStabilityStatus_Ok         = 0,
    LearnPressureStabilityStatus_Unstable   = 1
} EN_LearnPressureStabilityStatus;


typedef enum {
    SimulationStatus_NormalOperation            = 0,
    SimulationStatus_SystemSimulationRunning    = 1
} EN_SimulationStatus;


#endif /* _APC_MODEL_H_ */

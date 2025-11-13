/**************************************************************************************
 * VAT Adaptive Pressure Controller DeviceNet Communication Test
 *
 * Based on EDS File: 476297.eds
 * Device: VAT Adaptive Pressure Controller (Product Code 650)
 * Vendor: VAT Vakuumventile AG (Vendor Code 404)
 * Protocol: DeviceNet / CIP
 *
 * This header defines data structures and function prototypes for testing
 * DeviceNet communication with VAT pressure controller
 **************************************************************************************/

#ifndef VAT_DEVICENET_TEST_H_
#define VAT_DEVICENET_TEST_H_

#include <stdint.h>
#include <stdbool.h>

/*****************************************************************************/
/*! DEVICE IDENTIFICATION                                                    */
/*****************************************************************************/

#define VAT_VENDOR_CODE             404
#define VAT_VENDOR_NAME             "VAT Vakuumventile AG"
#define VAT_PRODUCT_CODE            650
#define VAT_PRODUCT_TYPE            29      /* Process Control Valve */
#define VAT_MAJOR_REVISION          2
#define VAT_MINOR_REVISION          1
#define VAT_PRODUCT_NAME            "VAT Adaptive Pressure Controller"

/*****************************************************************************/
/*! ASSEMBLY INSTANCE DEFINITIONS                                            */
/*****************************************************************************/

/* Input Assembly Instance IDs */
#define VAT_INPUT_ASSEMBLY_1        0x01    /* 2 bytes - INT Process Variable */
#define VAT_INPUT_ASSEMBLY_2        0x02    /* 3 bytes - Exception Status, Pressure */
#define VAT_INPUT_ASSEMBLY_3        0x03    /* 5 bytes - Exception Status, Pressure, Position */
#define VAT_INPUT_ASSEMBLY_100      0x64    /* 7 bytes - Full status */
#define VAT_INPUT_ASSEMBLY_101      0x65    /* 7 bytes - Full status with discrete inputs */

/* Output Assembly Instance IDs */
#define VAT_OUTPUT_ASSEMBLY_7       0x07    /* 4 bytes - Control Setpoint, Control Instance */
#define VAT_OUTPUT_ASSEMBLY_8       0x08    /* 5 bytes - Control Mode, Setpoint, Instance */
#define VAT_OUTPUT_ASSEMBLY_102     0x66    /* 8 bytes - Full control with calibration */

/*****************************************************************************/
/*! CIP OBJECT PATHS                                                         */
/*****************************************************************************/

/* Assembly Object Class = 0x04 */
#define CIP_CLASS_ASSEMBLY          0x04
#define CIP_ATTRIBUTE_DATA          0x03

/*****************************************************************************/
/*! INPUT DATA STRUCTURES (Slave → Master)                                  */
/*****************************************************************************/

/* Input Assembly 1: Process Variable Only (2 bytes) */
typedef __attribute__((packed)) struct VAT_INPUT_ASSEMBLY_1_Ttag
{
    int16_t process_variable;       /* INT Process variable value */
} VAT_INPUT_ASSEMBLY_1_T;

/* Input Assembly 2: Exception Status + Pressure (3 bytes) */
typedef __attribute__((packed)) struct VAT_INPUT_ASSEMBLY_2_Ttag
{
    uint8_t exception_status;       /* Exception status flags */
    int16_t pressure;               /* Pressure measurement */
} VAT_INPUT_ASSEMBLY_2_T;

/* Input Assembly 3: Exception Status + Pressure + Position (5 bytes) */
typedef __attribute__((packed)) struct VAT_INPUT_ASSEMBLY_3_Ttag
{
    uint8_t exception_status;       /* Exception status flags */
    int16_t pressure;               /* Pressure measurement */
    int16_t position;               /* Valve position */
} VAT_INPUT_ASSEMBLY_3_T;

/* Input Assembly 100: Full Status (7 bytes) */
typedef __attribute__((packed)) struct VAT_INPUT_ASSEMBLY_100_Ttag
{
    uint8_t exception_status;       /* Exception status flags */
    int16_t pressure;               /* Pressure measurement */
    int16_t position;               /* Valve position */
    uint8_t device_status;          /* Device status byte */
    uint8_t access_mode;            /* Current access mode */
} VAT_INPUT_ASSEMBLY_100_T;

/* Input Assembly 101: Full Status with Discrete Inputs (7 bytes) */
typedef __attribute__((packed)) struct VAT_INPUT_ASSEMBLY_101_Ttag
{
    uint8_t exception_status;       /* Exception status flags */
    int16_t pressure;               /* Pressure measurement */
    int16_t position;               /* Valve position */
    uint8_t discrete_inputs;        /* Digital input states */
    uint8_t device_status;          /* Device status byte */
} VAT_INPUT_ASSEMBLY_101_T;

/*****************************************************************************/
/*! OUTPUT DATA STRUCTURES (Master → Slave)                                 */
/*****************************************************************************/

/* Output Assembly 7: Basic Control (4 bytes) */
typedef __attribute__((packed)) struct VAT_OUTPUT_ASSEMBLY_7_Ttag
{
    int16_t control_setpoint;       /* Control setpoint value */
    uint16_t control_instance;      /* Control instance selector */
} VAT_OUTPUT_ASSEMBLY_7_T;

/* Output Assembly 8: Control with Mode (5 bytes) */
typedef __attribute__((packed)) struct VAT_OUTPUT_ASSEMBLY_8_Ttag
{
    uint8_t control_mode;           /* Control mode selector */
    int16_t control_setpoint;       /* Control setpoint value */
    uint16_t control_instance;      /* Control instance selector */
} VAT_OUTPUT_ASSEMBLY_8_T;

/* Output Assembly 102: Full Control with Calibration (8 bytes) */
typedef __attribute__((packed)) struct VAT_OUTPUT_ASSEMBLY_102_Ttag
{
    uint8_t control_mode;           /* Control mode selector */
    int16_t control_setpoint;       /* Control setpoint value */
    uint16_t control_instance;      /* Control instance selector */
    uint8_t auto_learn;             /* Auto learning enable */
    uint8_t calibration_scale;      /* Calibration scale factor */
    uint8_t zero_adjust;            /* Zero point adjustment */
} VAT_OUTPUT_ASSEMBLY_102_T;

/*****************************************************************************/
/*! CONTROL MODE DEFINITIONS                                                 */
/*****************************************************************************/

typedef enum VAT_CONTROL_MODE_Etag
{
    VAT_CONTROL_MODE_CLOSED         = 0x00,     /* Valve closed */
    VAT_CONTROL_MODE_OPEN           = 0x01,     /* Valve fully open */
    VAT_CONTROL_MODE_PRESSURE       = 0x02,     /* Pressure control */
    VAT_CONTROL_MODE_POSITION       = 0x03,     /* Position control */
    VAT_CONTROL_MODE_MANUAL         = 0x04,     /* Manual control */
    VAT_CONTROL_MODE_THROTTLE       = 0x05      /* Throttle control */
} VAT_CONTROL_MODE_E;

/*****************************************************************************/
/*! EXCEPTION STATUS BIT DEFINITIONS                                         */
/*****************************************************************************/

#define VAT_EXCEPTION_SETPOINT_OUT_OF_RANGE     (1 << 0)
#define VAT_EXCEPTION_HARDWARE_FAULT            (1 << 1)
#define VAT_EXCEPTION_SENSOR_FAULT              (1 << 2)
#define VAT_EXCEPTION_COMMUNICATION_FAULT       (1 << 3)
#define VAT_EXCEPTION_CALIBRATION_ERROR         (1 << 4)
#define VAT_EXCEPTION_OVERTEMPERATURE           (1 << 5)
#define VAT_EXCEPTION_UNDER_VACUUM              (1 << 6)
#define VAT_EXCEPTION_OVER_PRESSURE             (1 << 7)

/*****************************************************************************/
/*! DEVICE STATUS BIT DEFINITIONS                                            */
/*****************************************************************************/

#define VAT_DEVICE_STATUS_READY                 (1 << 0)
#define VAT_DEVICE_STATUS_FAULT                 (1 << 1)
#define VAT_DEVICE_STATUS_CALIBRATING           (1 << 2)
#define VAT_DEVICE_STATUS_LEARN_MODE            (1 << 3)
#define VAT_DEVICE_STATUS_REMOTE_MODE           (1 << 4)
#define VAT_DEVICE_STATUS_LOCAL_MODE            (1 << 5)

/*****************************************************************************/
/*! TEST CONFIGURATION                                                       */
/*****************************************************************************/

typedef struct VAT_TEST_CONFIG_Ttag
{
    uint8_t  node_address;          /* DeviceNet node address (0-63) */
    uint32_t baud_rate;             /* DeviceNet baud rate (125k/250k/500k) */
    uint16_t epr_ms;                /* Expected packet rate in milliseconds */
    uint8_t  input_assembly;        /* Selected input assembly instance */
    uint8_t  output_assembly;       /* Selected output assembly instance */
    bool     enable_logging;        /* Enable data logging */
    bool     enable_validation;     /* Enable data validation */
} VAT_TEST_CONFIG_T;

/*****************************************************************************/
/*! TEST STATISTICS                                                          */
/*****************************************************************************/

typedef struct VAT_TEST_STATS_Ttag
{
    uint32_t total_read_count;      /* Total read operations */
    uint32_t total_write_count;     /* Total write operations */
    uint32_t read_error_count;      /* Read errors */
    uint32_t write_error_count;     /* Write errors */
    uint32_t exception_count;       /* Exception status occurrences */
    uint32_t timeout_count;         /* Communication timeouts */
    uint32_t last_read_time_ms;     /* Timestamp of last read */
    uint32_t last_write_time_ms;    /* Timestamp of last write */
    int32_t  last_error_code;       /* Last error code */
} VAT_TEST_STATS_T;

/*****************************************************************************/
/*! TEST CONTEXT                                                             */
/*****************************************************************************/

typedef struct VAT_TEST_CONTEXT_Ttag
{
    VAT_TEST_CONFIG_T   config;             /* Test configuration */
    VAT_TEST_STATS_T    stats;              /* Test statistics */

    /* Input data buffers */
    VAT_INPUT_ASSEMBLY_1_T    input_asm1;
    VAT_INPUT_ASSEMBLY_2_T    input_asm2;
    VAT_INPUT_ASSEMBLY_3_T    input_asm3;
    VAT_INPUT_ASSEMBLY_100_T  input_asm100;
    VAT_INPUT_ASSEMBLY_101_T  input_asm101;

    /* Output data buffers */
    VAT_OUTPUT_ASSEMBLY_7_T   output_asm7;
    VAT_OUTPUT_ASSEMBLY_8_T   output_asm8;
    VAT_OUTPUT_ASSEMBLY_102_T output_asm102;

    /* Status flags */
    bool input_data_valid;
    bool output_data_sent;
    bool device_ready;
    bool test_running;

} VAT_TEST_CONTEXT_T;

/*****************************************************************************/
/*! FUNCTION PROTOTYPES                                                      */
/*****************************************************************************/

/* Initialization and configuration */
void VAT_Test_Init(VAT_TEST_CONTEXT_T* ptContext);
void VAT_Test_Configure(VAT_TEST_CONTEXT_T* ptContext, const VAT_TEST_CONFIG_T* ptConfig);
void VAT_Test_Deinit(VAT_TEST_CONTEXT_T* ptContext);

/* Communication functions */
int32_t VAT_Test_ReadInput(VAT_TEST_CONTEXT_T* ptContext, void* hChannel);
int32_t VAT_Test_WriteOutput(VAT_TEST_CONTEXT_T* ptContext, void* hChannel);

/* Data access functions */
void VAT_Test_GetInputData(VAT_TEST_CONTEXT_T* ptContext, void* ptData, uint8_t assembly_id);
void VAT_Test_SetOutputData(VAT_TEST_CONTEXT_T* ptContext, const void* ptData, uint8_t assembly_id);

/* Control functions */
void VAT_Test_SetControlMode(VAT_TEST_CONTEXT_T* ptContext, VAT_CONTROL_MODE_E eMode);
void VAT_Test_SetPressureSetpoint(VAT_TEST_CONTEXT_T* ptContext, int16_t pressure);
void VAT_Test_SetPositionSetpoint(VAT_TEST_CONTEXT_T* ptContext, int16_t position);

/* Status functions */
bool VAT_Test_IsDeviceReady(const VAT_TEST_CONTEXT_T* ptContext);
bool VAT_Test_HasException(const VAT_TEST_CONTEXT_T* ptContext);
uint8_t VAT_Test_GetExceptionStatus(const VAT_TEST_CONTEXT_T* ptContext);
uint8_t VAT_Test_GetDeviceStatus(const VAT_TEST_CONTEXT_T* ptContext);

/* Statistics functions */
void VAT_Test_GetStats(const VAT_TEST_CONTEXT_T* ptContext, VAT_TEST_STATS_T* ptStats);
void VAT_Test_ResetStats(VAT_TEST_CONTEXT_T* ptContext);
void VAT_Test_PrintStats(const VAT_TEST_CONTEXT_T* ptContext);

/* Validation functions */
bool VAT_Test_ValidateInputData(const VAT_TEST_CONTEXT_T* ptContext, uint8_t assembly_id);
bool VAT_Test_ValidateOutputData(const VAT_TEST_CONTEXT_T* ptContext, uint8_t assembly_id);

/* Logging functions */
void VAT_Test_LogInputData(const VAT_TEST_CONTEXT_T* ptContext, uint8_t assembly_id);
void VAT_Test_LogOutputData(const VAT_TEST_CONTEXT_T* ptContext, uint8_t assembly_id);

/* Test scenarios */
void VAT_Test_BasicPressureControl(VAT_TEST_CONTEXT_T* ptContext, void* hChannel);
void VAT_Test_PositionControl(VAT_TEST_CONTEXT_T* ptContext, void* hChannel);
void VAT_Test_FullCalibration(VAT_TEST_CONTEXT_T* ptContext, void* hChannel);
void VAT_Test_StressTest(VAT_TEST_CONTEXT_T* ptContext, void* hChannel);

#endif /* VAT_DEVICENET_TEST_H_ */

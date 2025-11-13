#ifndef APP_VAT_ASSEMBLIES_H_
#define APP_VAT_ASSEMBLIES_H_

#include "Hil_Compiler.h"
#include <stdint.h>

/******************************************************************************
 * ASSEMBLY METADATA AND MANAGEMENT
 ******************************************************************************/

/* Assembly direction */
#define ASSEMBLY_DIR_INPUT      0  /* Producing (Slave → Master) */
#define ASSEMBLY_DIR_OUTPUT     1  /* Consuming (Master → Slave) */

/* I/O Type Mask Definitions */
#define IO_TYPE_POLL            0x0001  /* Poll */
#define IO_TYPE_STROBE          0x0002  /* Strobe */
#define IO_TYPE_COS             0x0004  /* Change of State */
#define IO_TYPE_CYCLIC          0x0008  /* Cyclic */
#define IO_TYPE_MULTICAST_POLL  0x0010  /* Multicast Poll */

#define IO_TYPE_ALL             0x000F  /* Poll + Strobe + COS + Cyclic */
#define IO_TYPE_NO_STROBE       0x000D  /* Poll + COS + Cyclic (No Strobe) */

/* Assembly metadata structure */
typedef struct ASSEMBLY_METADATA_Ttag {
    uint16_t instance_number;      /* Assembly Instance (1-150) */
    uint8_t  size;                 /* Data size in bytes */
    uint8_t  direction;            /* 0=Input(Producing), 1=Output(Consuming) */
    uint16_t io_type_mask;         /* Compatible I/O Type Mask */
    char     name[64];             /* Assembly name */
    char     description[128];     /* Data description */
} ASSEMBLY_METADATA_T;

/* Assembly data buffer */
typedef struct ASSEMBLY_DATA_Ttag {
    uint8_t data[34];              /* Maximum size: 34 bytes (Input111/113) */
    uint8_t valid_size;            /* Actual data size in use */
} ASSEMBLY_DATA_T;

/* Assembly manager structure */
typedef struct ASSEMBLY_MANAGER_Ttag {
    ASSEMBLY_METADATA_T metadata[35];   /* 24 Input + 11 Output */
    ASSEMBLY_DATA_T input_buffers[24];  /* Input Assembly data */
    ASSEMBLY_DATA_T output_buffers[11]; /* Output Assembly data */

    uint8_t active_input_instance;      /* Currently active Input Assembly */
    uint8_t active_output_instance;     /* Currently active Output Assembly */

    uint16_t io_connection_type;        /* Current I/O connection type */
    uint8_t  data_type;                 /* 0xC3=INT, 0xCA=REAL */
} ASSEMBLY_MANAGER_T;

/******************************************************************************
 * INPUT ASSEMBLIES (24 types)
 ******************************************************************************/

/* Input Assembly 1: INT Process Variable (2 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_1_Ttag {
    int16_t process_variable;
} __HIL_PACKED_POST INPUT_ASSEMBLY_1_T;

/* Input Assembly 2: Exception Status, Pressure (3 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_2_Ttag {
    uint8_t exception_status;
    int16_t pressure;
} __HIL_PACKED_POST INPUT_ASSEMBLY_2_T;

/* Input Assembly 3: Exception Status, Pressure, Position (5 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_3_Ttag {
    uint8_t exception_status;
    int16_t pressure;
    int16_t position;
} __HIL_PACKED_POST INPUT_ASSEMBLY_3_T;

/* Input Assembly 4: Exception Status, Pressure, Control Setpoint (5 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_4_Ttag {
    uint8_t exception_status;
    int16_t pressure;
    int16_t control_setpoint;
} __HIL_PACKED_POST INPUT_ASSEMBLY_4_T;

/* Input Assembly 5: Exception Status, Pressure, Control Setpoint, Position (7 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_5_Ttag {
    uint8_t exception_status;
    int16_t pressure;
    int16_t control_setpoint;
    int16_t position;
} __HIL_PACKED_POST INPUT_ASSEMBLY_5_T;

/* Input Assembly 6: Exception Status, Pressure, Control Setpoint, Control Mode, Position (8 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_6_Ttag {
    uint8_t exception_status;
    int16_t pressure;
    int16_t control_setpoint;
    uint8_t control_mode;
    int16_t position;
} __HIL_PACKED_POST INPUT_ASSEMBLY_6_T;

/* Input Assembly 10: Exception Status only (1 byte) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_10_Ttag {
    uint8_t exception_status;
} __HIL_PACKED_POST INPUT_ASSEMBLY_10_T;

/* Input Assembly 11: Exception Status, Pressure, Position, Discrete Inputs (6 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_11_Ttag {
    uint8_t exception_status;
    int16_t pressure;
    int16_t position;
    uint8_t discrete_inputs;
} __HIL_PACKED_POST INPUT_ASSEMBLY_11_T;

/* Input Assembly 17: FP-Pressure (4 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_17_Ttag {
    float fp_pressure;
} __HIL_PACKED_POST INPUT_ASSEMBLY_17_T;

/* Input Assembly 18: Exception Status, FP-Pressure (5 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_18_Ttag {
    uint8_t exception_status;
    float fp_pressure;
} __HIL_PACKED_POST INPUT_ASSEMBLY_18_T;

/* Input Assembly 19: Exception Status, FP-Pressure, FP-Position (9 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_19_Ttag {
    uint8_t exception_status;
    float fp_pressure;
    float fp_position;
} __HIL_PACKED_POST INPUT_ASSEMBLY_19_T;

/* Input Assembly 20: Exception Status, FP-Pressure, FP-Control Setpoint (9 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_20_Ttag {
    uint8_t exception_status;
    float fp_pressure;
    float fp_control_setpoint;
} __HIL_PACKED_POST INPUT_ASSEMBLY_20_T;

/* Input Assembly 21: Exception Status, FP-Pressure, FP-Control Setpoint, FP-Position (13 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_21_Ttag {
    uint8_t exception_status;
    float fp_pressure;
    float fp_control_setpoint;
    float fp_position;
} __HIL_PACKED_POST INPUT_ASSEMBLY_21_T;

/* Input Assembly 22: Exception Status, FP-Pressure, FP-Control Setpoint, Control Mode, FP-Position (14 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_22_Ttag {
    uint8_t exception_status;
    float fp_pressure;
    float fp_control_setpoint;
    uint8_t control_mode;
    float fp_position;
} __HIL_PACKED_POST INPUT_ASSEMBLY_22_T;

/* Input Assembly 26: Exception Status, FP-Pressure, FP-Position, Discrete Inputs (10 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_26_Ttag {
    uint8_t exception_status;
    float fp_pressure;
    float fp_position;
    uint8_t discrete_inputs;
} __HIL_PACKED_POST INPUT_ASSEMBLY_26_T;

/* Input Assembly 100: Exception Status, Pressure, Position, Device Status, Access Mode (7 bytes) */
/* DEFAULT ASSEMBLY - CURRENTLY IMPLEMENTED */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_100_Ttag {
    uint8_t exception_status;
    int16_t pressure;
    int16_t position;
    uint8_t device_status;
    uint8_t access_mode;
} __HIL_PACKED_POST INPUT_ASSEMBLY_100_T;

/* Input Assembly 101: Exception Status, Pressure, Position, Discrete Inputs, Device Status (7 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_101_Ttag {
    uint8_t exception_status;
    int16_t pressure;
    int16_t position;
    uint8_t discrete_inputs;
    uint8_t device_status;
} __HIL_PACKED_POST INPUT_ASSEMBLY_101_T;

/* Input Assembly 104: Complex 23-byte structure */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_104_Ttag {
    uint8_t exception_status;
    int16_t pressure;
    int16_t pressure_sensor_2;
    int16_t position;
    uint8_t access_mode;
    uint8_t device_status;
    uint8_t cluster_info[14];      /* Cluster information */
} __HIL_PACKED_POST INPUT_ASSEMBLY_104_T;

/* Input Assembly 105: Exception Status, FP-Pressure, FP-Position, Device Status, Access Mode (11 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_105_Ttag {
    uint8_t exception_status;
    float fp_pressure;
    float fp_position;
    uint8_t device_status;
    uint8_t access_mode;
} __HIL_PACKED_POST INPUT_ASSEMBLY_105_T;

/* Input Assembly 106: Exception Status, FP-Pressure, FP-Position, Discrete Inputs, Device Status (11 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_106_Ttag {
    uint8_t exception_status;
    float fp_pressure;
    float fp_position;
    uint8_t discrete_inputs;
    uint8_t device_status;
} __HIL_PACKED_POST INPUT_ASSEMBLY_106_T;

/* Input Assembly 109: Complex 29-byte structure */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_109_Ttag {
    uint8_t exception_status;
    float fp_pressure;
    float fp_pressure_sensor_2;
    float fp_position;
    uint8_t access_mode;
    uint8_t device_status;
    uint8_t cluster_info[14];
} __HIL_PACKED_POST INPUT_ASSEMBLY_109_T;

/* Input Assembly 111: Complex 34-byte structure (MAXIMUM SIZE) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_111_Ttag {
    uint8_t exception_status;
    float fp_pressure;
    float fp_position;
    float fp_pressure_sensor_1;
    float fp_pressure_sensor_2;
    uint8_t discrete_inputs;
    uint8_t device_status;
    uint8_t access_mode;
    uint8_t cluster_info[14];
} __HIL_PACKED_POST INPUT_ASSEMBLY_111_T;

/* Input Assembly 113: Same as 111 (34 bytes) */
typedef INPUT_ASSEMBLY_111_T INPUT_ASSEMBLY_113_T;

/* Input Assembly 150: Exception Status, Pressure Sensor #1, Pressure Sensor #2, Position, Sensor Select, Discrete Inputs (9 bytes) */
typedef __HIL_PACKED_PRE struct INPUT_ASSEMBLY_150_Ttag {
    uint8_t exception_status;
    int16_t pressure_sensor_1;
    int16_t pressure_sensor_2;
    int16_t position;
    uint8_t sensor_select;
    uint8_t discrete_inputs;
} __HIL_PACKED_POST INPUT_ASSEMBLY_150_T;

/* Union for all Input Assemblies */
typedef union INPUT_ASSEMBLY_UNION_Ttag {
    INPUT_ASSEMBLY_1_T   input1;
    INPUT_ASSEMBLY_2_T   input2;
    INPUT_ASSEMBLY_3_T   input3;
    INPUT_ASSEMBLY_4_T   input4;
    INPUT_ASSEMBLY_5_T   input5;
    INPUT_ASSEMBLY_6_T   input6;
    INPUT_ASSEMBLY_10_T  input10;
    INPUT_ASSEMBLY_11_T  input11;
    INPUT_ASSEMBLY_17_T  input17;
    INPUT_ASSEMBLY_18_T  input18;
    INPUT_ASSEMBLY_19_T  input19;
    INPUT_ASSEMBLY_20_T  input20;
    INPUT_ASSEMBLY_21_T  input21;
    INPUT_ASSEMBLY_22_T  input22;
    INPUT_ASSEMBLY_26_T  input26;
    INPUT_ASSEMBLY_100_T input100;
    INPUT_ASSEMBLY_101_T input101;
    INPUT_ASSEMBLY_104_T input104;
    INPUT_ASSEMBLY_105_T input105;
    INPUT_ASSEMBLY_106_T input106;
    INPUT_ASSEMBLY_109_T input109;
    INPUT_ASSEMBLY_111_T input111;
    INPUT_ASSEMBLY_113_T input113;
    INPUT_ASSEMBLY_150_T input150;
    uint8_t raw[34];  /* Maximum size */
} INPUT_ASSEMBLY_UNION_T;

/******************************************************************************
 * OUTPUT ASSEMBLIES (11 types)
 ******************************************************************************/

/* Output Assembly 7: Control Setpoint, Control Instance (4 bytes) */
typedef __HIL_PACKED_PRE struct OUTPUT_ASSEMBLY_7_Ttag {
    int16_t control_setpoint;
    uint8_t control_instance;
    uint8_t reserved;
} __HIL_PACKED_POST OUTPUT_ASSEMBLY_7_T;

/* Output Assembly 8: Control Mode, Control Setpoint, Control Instance (5 bytes) */
/* DEFAULT ASSEMBLY - CURRENTLY IMPLEMENTED */
typedef __HIL_PACKED_PRE struct OUTPUT_ASSEMBLY_8_Ttag {
    uint8_t control_mode;
    int16_t control_setpoint;
    uint8_t control_instance;
    uint8_t reserved;
} __HIL_PACKED_POST OUTPUT_ASSEMBLY_8_T;

/* Output Assembly 23: FP Control Setpoint, Control Instance (6 bytes) */
typedef __HIL_PACKED_PRE struct OUTPUT_ASSEMBLY_23_Ttag {
    float fp_control_setpoint;
    uint8_t control_instance;
    uint8_t reserved;
} __HIL_PACKED_POST OUTPUT_ASSEMBLY_23_T;

/* Output Assembly 24: Control Mode, FP Control Setpoint, Control Instance (7 bytes) */
typedef __HIL_PACKED_PRE struct OUTPUT_ASSEMBLY_24_Ttag {
    uint8_t control_mode;
    float fp_control_setpoint;
    uint8_t control_instance;
    uint8_t reserved;
} __HIL_PACKED_POST OUTPUT_ASSEMBLY_24_T;

/* Output Assembly 32: Control Mode, FP Control Setpoint, Kp, Ki, Kd (17 bytes) */
typedef __HIL_PACKED_PRE struct OUTPUT_ASSEMBLY_32_Ttag {
    uint8_t control_mode;
    float fp_control_setpoint;
    float kp;
    float ki;
    float kd;
} __HIL_PACKED_POST OUTPUT_ASSEMBLY_32_T;

/* Output Assembly 102: Control Mode, Control Setpoint, Control Instance, Auto Learn, Calibration Scale, Zero Adjust (8 bytes) */
typedef __HIL_PACKED_PRE struct OUTPUT_ASSEMBLY_102_Ttag {
    uint8_t control_mode;
    int16_t control_setpoint;
    uint8_t control_instance;
    uint8_t auto_learn;
    uint8_t calibration_scale;
    uint8_t zero_adjust;
    uint8_t reserved;
} __HIL_PACKED_POST OUTPUT_ASSEMBLY_102_T;

/* Output Assembly 103: Control Mode, Control Setpoint, Control Instance, Individual Valve Control (6 bytes) */
typedef __HIL_PACKED_PRE struct OUTPUT_ASSEMBLY_103_Ttag {
    uint8_t control_mode;
    int16_t control_setpoint;
    uint8_t control_instance;
    uint8_t valve_address;
    uint8_t valve_action;
} __HIL_PACKED_POST OUTPUT_ASSEMBLY_103_T;

/* Output Assembly 107: Control Mode, FP Control Setpoint, Control Instance, Auto Learn, Calibration Scale, Zero Adjust (12 bytes) */
typedef __HIL_PACKED_PRE struct OUTPUT_ASSEMBLY_107_Ttag {
    uint8_t control_mode;
    float fp_control_setpoint;
    uint8_t control_instance;
    uint8_t auto_learn;
    uint8_t calibration_scale;
    uint8_t zero_adjust;
    uint8_t reserved[2];
} __HIL_PACKED_POST OUTPUT_ASSEMBLY_107_T;

/* Output Assembly 108: Control Mode, FP-Control Setpoint, Control Instance, Individual Valve Control (8 bytes) */
typedef __HIL_PACKED_PRE struct OUTPUT_ASSEMBLY_108_Ttag {
    uint8_t control_mode;
    float fp_control_setpoint;
    uint8_t control_instance;
    uint8_t valve_address;
    uint8_t valve_action;
} __HIL_PACKED_POST OUTPUT_ASSEMBLY_108_T;

/* Output Assembly 110: Complex 17-byte structure */
typedef __HIL_PACKED_PRE struct OUTPUT_ASSEMBLY_110_Ttag {
    uint8_t control_mode;
    float fp_control_setpoint_pressure;
    float fp_setpoint_position;
    uint8_t control_instance;
    uint8_t auto_learn;
    uint8_t calibration_scale;
    uint8_t zero_adjust;
    uint8_t valve_address;
    uint8_t valve_action;
} __HIL_PACKED_POST OUTPUT_ASSEMBLY_110_T;

/* Output Assembly 112: Complex 18-byte structure (MAXIMUM SIZE) */
typedef __HIL_PACKED_PRE struct OUTPUT_ASSEMBLY_112_Ttag {
    uint8_t control_mode;
    float fp_control_setpoint_pressure;
    float fp_setpoint_position;
    uint8_t control_instance;
    uint8_t auto_learn;
    float fp_calibration_scale;
    uint8_t zero_adjust;
    uint8_t valve_address;
    uint8_t valve_action;
} __HIL_PACKED_POST OUTPUT_ASSEMBLY_112_T;

/* Union for all Output Assemblies */
typedef union OUTPUT_ASSEMBLY_UNION_Ttag {
    OUTPUT_ASSEMBLY_7_T   output7;
    OUTPUT_ASSEMBLY_8_T   output8;
    OUTPUT_ASSEMBLY_23_T  output23;
    OUTPUT_ASSEMBLY_24_T  output24;
    OUTPUT_ASSEMBLY_32_T  output32;
    OUTPUT_ASSEMBLY_102_T output102;
    OUTPUT_ASSEMBLY_103_T output103;
    OUTPUT_ASSEMBLY_107_T output107;
    OUTPUT_ASSEMBLY_108_T output108;
    OUTPUT_ASSEMBLY_110_T output110;
    OUTPUT_ASSEMBLY_112_T output112;
    uint8_t raw[18];  /* Maximum size */
} OUTPUT_ASSEMBLY_UNION_T;

/******************************************************************************
 * FUNCTION PROTOTYPES
 ******************************************************************************/

/* Assembly Manager Initialization */
void VAT_Assembly_Init(ASSEMBLY_MANAGER_T* ptManager);

/* Assembly Selection */
int32_t VAT_Assembly_SelectInput(ASSEMBLY_MANAGER_T* ptManager, uint8_t instance);
int32_t VAT_Assembly_SelectOutput(ASSEMBLY_MANAGER_T* ptManager, uint8_t instance);

/* Assembly Size Query */
uint8_t VAT_Assembly_GetInputSize(uint8_t instance);
uint8_t VAT_Assembly_GetOutputSize(uint8_t instance);

/* Assembly Validation */
uint8_t VAT_Assembly_IsValidInput(uint8_t instance);
uint8_t VAT_Assembly_IsValidOutput(uint8_t instance);

#endif /* APP_VAT_ASSEMBLIES_H_ */

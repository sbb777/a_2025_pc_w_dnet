# VAT EDS 476297 전체 적용 작업 계획서

## 📋 작업 개요
- **날짜**: 2025-11-05
- **담당자**: Firmware Development Team
- **프로젝트**: STM32 F429 + netX90 DeviceNet VAT Controller
- **작업 유형**: 설계/개발/구현
- **작업 범위**: 476297.eds 파일의 모든 사양을 펌웨어에 완전 적용

## 🎯 작업 목표

**주요 목표**: VAT Adaptive Pressure Controller EDS 파일(476297.eds)에 정의된 모든 기능을 netX90 DeviceNet 펌웨어에 완전히 구현하여 CYCON.net 및 모든 DeviceNet 마스터와 완벽하게 호환되도록 함

**세부 목표**:
1. 24개 Input Assembly 및 11개 Output Assembly 전체 구현
2. 99개 CIP 파라미터 객체 구현
3. 4가지 I/O 연결 타입 (Poll, Strobe, COS, Cyclic) 지원
4. 동적 Data Type 전환 (INT ↔ REAL)
5. 다양한 Units 지원 (Pressure: 9가지, Position: 3가지)
6. Assembly 동적 선택 기능
7. 파라미터 그룹화 및 접근 제어
8. Flash 기반 파라미터 저장/복구

## 📊 현재 상황 분석

### ✅ 기 완료 작업 (2025-10-29)
- [x] CIP Identity Object 변경
  - Vendor ID: 283 → 404 (VAT Vakuumventile AG)
  - Device Type: 12 → 29 (Process Control Valve)
  - Product Code: 34 → 650
  - Major/Minor Revision: 5.2 → 2.1
  - Product Name: "VAT Adaptive Pressure Controller"
- [x] 단일 Input Assembly 100 구현 (7 bytes)
- [x] 단일 Output Assembly 8 구현 (5 bytes)
- [x] I/O 데이터 구조체 크기 조정
- [x] 버퍼 오버플로우 방지 코드 수정

### ⚠️ 미완료 작업 (이번 작업 범위)
- [ ] 23개 추가 Input Assembly 구현
- [ ] 10개 추가 Output Assembly 구현
- [ ] 99개 CIP 파라미터 구현
- [ ] 동적 Assembly 선택 메커니즘
- [ ] Data Type 전환 기능 (INT/REAL)
- [ ] Units 설정 및 변환 기능
- [ ] I/O 연결 타입 다중 지원
- [ ] 파라미터 저장/복구 기능 (Flash)

## 🏗️ 전체 아키텍처 설계

### 시스템 구성도

```
┌─────────────────────────────────────────────────────────┐
│                  DeviceNet Master                        │
│              (CYCON.net / RSNetWorx)                     │
└──────────────────┬──────────────────────────────────────┘
                   │ DeviceNet Bus
                   │
┌──────────────────▼──────────────────────────────────────┐
│              netX90 DeviceNet Stack                      │
│  ┌────────────────────────────────────────────────┐    │
│  │        CIP Identity Object (Class 0x01)        │    │
│  │  VendorID: 404  ProductCode: 650  Rev: 2.1    │    │
│  └────────────────────────────────────────────────┘    │
│  ┌────────────────────────────────────────────────┐    │
│  │     Assembly Object Manager (Class 0x04)       │    │
│  │  • 24 Input Assemblies (1-150)                 │    │
│  │  • 11 Output Assemblies (7-112)                │    │
│  │  • Dynamic Selection (Param11/12)              │    │
│  └────────────────────────────────────────────────┘    │
│  ┌────────────────────────────────────────────────┐    │
│  │    Parameter Object Manager (Class 0x0F)       │    │
│  │  • 99 Parameters (Param1-99)                   │    │
│  │  • Class/Instance/Attribute Access             │    │
│  │  • Flash Storage Support                       │    │
│  └────────────────────────────────────────────────┘    │
│  ┌────────────────────────────────────────────────┐    │
│  │      Data Conversion Engine                    │    │
│  │  • INT ↔ REAL Conversion                       │    │
│  │  • Units Conversion (Pressure, Position)       │    │
│  │  • Range Validation                            │    │
│  └────────────────────────────────────────────────┘    │
└──────────────────┬──────────────────────────────────────┘
                   │ SPI
┌──────────────────▼──────────────────────────────────────┐
│              STM32 F429 Application                      │
│  ┌────────────────────────────────────────────────┐    │
│  │         VAT Pressure Control Logic             │    │
│  │  • Pressure Sensor Reading                     │    │
│  │  • Position Control                            │    │
│  │  • PID Controller                              │    │
│  │  • Valve Actuation                             │    │
│  └────────────────────────────────────────────────┘    │
│  ┌────────────────────────────────────────────────┐    │
│  │          Flash Parameter Storage               │    │
│  │  • Sector 11 (128KB)                           │    │
│  │  • CRC32 Checksum                              │    │
│  │  • Double Buffering                            │    │
│  └────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────┘
```

---

## Phase 1: 데이터 구조 확장 (예상: 2일)

### 1.1 Assembly 데이터 구조 설계

#### Assembly 메타데이터 구조체

**파일**: `Hil_DemoApp/Includes/App_VAT_Assemblies.h` (신규)

```c
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
```

### 1.2 파라미터 데이터 구조 설계

**파일**: `Hil_DemoApp/Includes/App_VAT_Parameters.h` (신규)

```c
#ifndef APP_VAT_PARAMETERS_H_
#define APP_VAT_PARAMETERS_H_

#include "Hil_Compiler.h"
#include <stdint.h>

/******************************************************************************
 * CIP PARAMETER DEFINITIONS
 ******************************************************************************/

/* Maximum parameters */
#define VAT_PARAM_COUNT_MAX     99

/* CIP Data Types */
#define CIP_DATA_TYPE_USINT     0xC6  /* Unsigned Short INT (1 byte) */
#define CIP_DATA_TYPE_UINT      0xC7  /* Unsigned INT (2 bytes) */
#define CIP_DATA_TYPE_INT       0xC3  /* Signed INT (2 bytes) */
#define CIP_DATA_TYPE_REAL      0xCA  /* REAL (4 bytes float) */
#define CIP_DATA_TYPE_STRING    0xDA  /* SHORT_STRING */

/* Parameter Descriptor Flags */
#define PARAM_DESC_ENUM         0x0002  /* Enumerated String */
#define PARAM_DESC_READ_ONLY    0x0010  /* Read Only */
#define PARAM_DESC_MONITORING   0x0020  /* Monitoring Attribute */

/* Pressure Units (Param9) */
#define PRESSURE_UNIT_COUNTS    0x1001  /* Raw Counts */
#define PRESSURE_UNIT_PERCENT   0x1007  /* Percent */
#define PRESSURE_UNIT_PSI       0x1300  /* psi */
#define PRESSURE_UNIT_TORR      0x1301  /* Torr */
#define PRESSURE_UNIT_MTORR     0x1302  /* mTorr */
#define PRESSURE_UNIT_BAR       0x1307  /* Bar */
#define PRESSURE_UNIT_MBAR      0x1308  /* mBar */
#define PRESSURE_UNIT_PA        0x1309  /* Pa */
#define PRESSURE_UNIT_ATM       0x130B  /* atm */

/* Position Units (Param10) */
#define POSITION_UNIT_COUNTS    0x1001  /* Raw Counts */
#define POSITION_UNIT_PERCENT   0x1007  /* Percent */
#define POSITION_UNIT_DEGREES   0x1703  /* Degrees */

/* Controller Mode Enum (Param6) */
typedef enum CONTROLLER_MODE_Etag {
    CTRL_MODE_INIT              = 0,
    CTRL_MODE_SYNC              = 1,
    CTRL_MODE_POSITION          = 2,
    CTRL_MODE_CLOSE             = 3,
    CTRL_MODE_OPEN              = 4,
    CTRL_MODE_PRESSURE          = 5,
    CTRL_MODE_HOLD              = 6,
    CTRL_MODE_AUTOLEARNING      = 7,
    CTRL_MODE_OPEN_INTERLOCK    = 8,
    CTRL_MODE_CLOSE_INTERLOCK   = 9,
    CTRL_MODE_MAINT_OPEN        = 10,
    CTRL_MODE_MAINT_CLOSE       = 11,
    CTRL_MODE_POWER_FAIL        = 12,
    CTRL_MODE_FATAL_ERROR       = 14
} CONTROLLER_MODE_E;

/* Device Status Enum (Param5) */
typedef enum DEVICE_STATUS_Etag {
    DEV_STATUS_UNDEFINED        = 0,
    DEV_STATUS_SELF_TESTING     = 1,
    DEV_STATUS_IDLE             = 2,
    DEV_STATUS_SELF_TEST_EXCEPT = 3,
    DEV_STATUS_EXECUTING        = 4,
    DEV_STATUS_ABORT            = 5,
    DEV_STATUS_CRITICAL_FAULT   = 6
} DEVICE_STATUS_E;

/* Access Mode Enum (Param7) */
typedef enum ACCESS_MODE_Etag {
    ACCESS_MODE_LOCAL           = 0,
    ACCESS_MODE_REMOTE          = 1,
    ACCESS_MODE_LOCKED          = 2
} ACCESS_MODE_E;

/* Parameter structure */
typedef struct VAT_PARAMETER_Ttag {
    uint8_t  param_id;          /* Parameter ID (1-99) */
    uint8_t  class_id;          /* CIP Class ID */
    uint8_t  instance_id;       /* CIP Instance ID */
    uint8_t  attribute_id;      /* CIP Attribute ID */

    uint16_t descriptor;        /* Parameter descriptor */
    uint8_t  data_type;         /* CIP Data Type */
    uint8_t  data_size;         /* Data size in bytes */

    char name[64];              /* Parameter name */
    char units[16];             /* Units */
    char help[128];             /* Help text */

    int32_t min_value;          /* Minimum value */
    int32_t max_value;          /* Maximum value */
    int32_t default_value;      /* Default value */

    uint8_t data[32];           /* Current value (max 32 bytes for strings) */
} VAT_PARAMETER_T;

/* Parameter manager */
typedef struct VAT_PARAM_MANAGER_Ttag {
    VAT_PARAMETER_T params[VAT_PARAM_COUNT_MAX];
    uint8_t param_count;
    uint8_t modified[13];       /* Modified flags (99/8 = 13 bytes) */
} VAT_PARAM_MANAGER_T;

/******************************************************************************
 * FUNCTION PROTOTYPES
 ******************************************************************************/

/* Parameter Manager Initialization */
void VAT_Param_Init(VAT_PARAM_MANAGER_T* ptManager);

/* Parameter Access by ID */
int32_t VAT_Param_Get(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id, void* pData, uint8_t* pSize);
int32_t VAT_Param_Set(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id, const void* pData, uint8_t size);

/* Parameter Access by CIP Path */
int32_t VAT_Param_GetByPath(VAT_PARAM_MANAGER_T* ptManager, uint8_t class_id, uint8_t instance_id, uint8_t attribute_id, void* pData, uint8_t* pSize);
int32_t VAT_Param_SetByPath(VAT_PARAM_MANAGER_T* ptManager, uint8_t class_id, uint8_t instance_id, uint8_t attribute_id, const void* pData, uint8_t size);

/* Modified Flags */
uint8_t VAT_Param_IsModified(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id);
void VAT_Param_ClearModified(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id);
void VAT_Param_SetModified(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id);

/* Flash Storage */
int32_t VAT_Param_SaveToFlash(VAT_PARAM_MANAGER_T* ptManager);
int32_t VAT_Param_LoadFromFlash(VAT_PARAM_MANAGER_T* ptManager);

#endif /* APP_VAT_PARAMETERS_H_ */
```

---

## Phase 2: Input Assembly 구현 (예상: 3일)

### 2.1 Input Assembly 전체 목록

| # | Instance | Size | Data Format | I/O Mask | Priority |
|---|----------|------|-------------|----------|----------|
| 1 | 1 | 2 | INT Process Variable | 0x000F | Low |
| 2 | 2 | 3 | Exception Status, Pressure | 0x000F | Medium |
| 3 | 3 | 5 | Exception Status, Pressure, Position | 0x000F | Medium |
| 4 | 4 | 5 | Exception Status, Pressure, Control Setpoint | 0x000F | Medium |
| 5 | 5 | 7 | Exception Status, Pressure, Control Setpoint, Position | 0x000F | Medium |
| 6 | 6 | 8 | Exception Status, Pressure, Control Setpoint, Control Mode, Position | 0x000F | High |
| 7 | 10 | 1 | Exception Status | 0x000F | Low |
| 8 | 11 | 6 | Exception Status, Pressure, Position, Discrete Inputs | 0x000F | Medium |
| 9 | 17 | 4 | FP-Pressure | 0x000F | Medium |
| 10 | 18 | 5 | Exception Status, FP-Pressure | 0x000F | Medium |
| 11 | 19 | 9 | Exception Status, FP-Pressure, FP-Position | 0x000D | Medium |
| 12 | 20 | 9 | Exception Status, FP-Pressure, FP-Control Setpoint | 0x000D | Medium |
| 13 | 21 | 13 | Exception Status, FP-Pressure, FP-Control Setpoint, FP-Position | 0x000D | High |
| 14 | 22 | 14 | Exception Status, FP-Pressure, FP-Control Setpoint, Control Mode, FP-Position | 0x000D | High |
| 15 | 26 | 10 | Exception Status, FP-Pressure, FP-Position, Discrete Inputs | 0x000D | Medium |
| **16** | **100** | **7** | **Exception Status, Pressure, Position, Device Status, Access Mode** | **0x000F** | **Critical** |
| 17 | 101 | 7 | Exception Status, Pressure, Position, Discrete Inputs, Device Status | 0x000F | High |
| 18 | 104 | 23 | Exception Status, Pressure, Pressure Sensor #2, Position, Access Mode, Device Status, Clusterinfo | 0x000D | High |
| 19 | 105 | 11 | Exception Status, FP-Pressure, FP-Position, Device Status, Access Mode | 0x000D | High |
| 20 | 106 | 11 | Exception Status, FP-Pressure, FP-Position, Discrete Inputs, Device Status | 0x000D | Medium |
| 21 | 109 | 29 | Exception Status, FP-Pressure, FP-Pressure Sensor #2, FP-Position, Access Mode, Device Status, Clusterinfo | 0x000D | High |
| 22 | 111 | 34 | Exception Status, FP-Pressure, FP-Position, FP-Pressure Sensor #1, FP-Pressure Sensor #2, Discrete Inputs, Device Status, Access Mode, Clusterinfo | 0x000D | High |
| 23 | 113 | 34 | Same as 111 | 0x000D | High |
| 24 | 150 | 9 | Exception Status, Pressure Sensor #1, Pressure Sensor #2, Position, Sensor Select, Discrete Inputs | 0x000D | Medium |

### 2.2 구현 작업 체크리스트

#### Day 1: Critical & High Priority (Input100, 101, 6, 21, 22)
- [ ] Input100 리팩토링 (현재 구조체 기반으로 변경)
- [ ] Input101 구현 및 테스트
- [ ] Input6 구현 및 테스트
- [ ] Input21 구현 (FP 타입, 13 bytes)
- [ ] Input22 구현 (FP 타입, 14 bytes)
- [ ] 단위 테스트 작성

#### Day 2: High Priority (Input104, 105, 109, 111, 113)
- [ ] Input104 구현 (23 bytes, Cluster info)
- [ ] Input105 구현 (FP 타입, 11 bytes)
- [ ] Input109 구현 (FP 타입, 29 bytes, Cluster info)
- [ ] Input111 구현 (34 bytes, MAXIMUM)
- [ ] Input113 구현 (=Input111)
- [ ] 단위 테스트 작성

#### Day 3: Medium & Low Priority (나머지)
- [ ] Input1-5 구현 (INT 타입)
- [ ] Input10-11 구현
- [ ] Input17-20 구현 (FP 타입)
- [ ] Input26 구현 (FP 타입)
- [ ] Input106 구현
- [ ] Input150 구현 (센서 선택)
- [ ] 전체 크기 검증
- [ ] 통합 테스트

---

## Phase 3: Output Assembly 구현 (예상: 2일)

### 3.1 Output Assembly 전체 목록

| # | Instance | Size | Data Format | I/O Mask | Priority |
|---|----------|------|-------------|----------|----------|
| 1 | 7 | 4 | Control Setpoint, Control Instance | 0x000D | Medium |
| **2** | **8** | **5** | **Control Mode, Control Setpoint, Control Instance** | **0x000D** | **Critical** |
| 3 | 23 | 6 | FP Control Setpoint, Control Instance | 0x000D | Medium |
| 4 | 24 | 7 | Control Mode, FP Control Setpoint, Control Instance | 0x000D | High |
| 5 | 32 | 17 | Control Mode, FP Control Setpoint, Kp, Ki, Kd | 0x000D | High |
| 6 | 102 | 8 | Control Mode, Control Setpoint, Control Instance, Auto Learn, Calibration Scale, Zero Adjust | 0x000D | High |
| 7 | 103 | 6 | Control Mode, Control Setpoint, Control Instance, Individual Valve Control (Address, Action) | 0x000D | Medium |
| 8 | 107 | 12 | Control Mode, FP Control Setpoint, Control Instance, Auto Learn, Calibration Scale, Zero Adjust | 0x000D | High |
| 9 | 108 | 8 | Control Mode, FP-Control Setpoint, Control Instance, Individual Valve Control (Address, Action) | 0x000D | Medium |
| 10 | 110 | 17 | Control Mode, FP Control Setpoint Pressure, FP-Setpoint Position, Control Instance, Auto Learn, Calibration Scale, Zero Adjust, Individual Valve Control | 0x000D | High |
| 11 | 112 | 18 | Control Mode, FP Control Setpoint Pressure, FP-Setpoint Position, Control Instance, Auto Learn, FP-Calibration Scale, Zero Adjust, Individual Valve Control | 0x000D | High |

### 3.2 구현 작업 체크리스트

#### Day 1: Critical & High Priority
- [ ] Output8 리팩토링 (현재 구조체 기반으로 변경)
- [ ] Output24 구현 (FP 타입)
- [ ] Output32 구현 (PID 파라미터 Kp, Ki, Kd)
- [ ] Output102 구현 (Auto Learn, Calibration)
- [ ] Output107 구현 (FP + Auto Learn)
- [ ] 단위 테스트

#### Day 2: High & Medium Priority
- [ ] Output110 구현 (17 bytes, 복합 제어)
- [ ] Output112 구현 (18 bytes, MAXIMUM)
- [ ] Output7 구현
- [ ] Output23 구현 (FP 타입)
- [ ] Output103 구현 (Individual Valve Control)
- [ ] Output108 구현 (FP + Valve Control)
- [ ] 전체 크기 검증
- [ ] 통합 테스트

---

## Phase 4: Assembly 동적 선택 메커니즘 (예상: 2일)

### 4.1 Assembly Selector 구현

**파일**: `Hil_DemoApp/Sources/App_VAT_AssemblySelector.c` (신규)

```c
#include "App_VAT_Assemblies.h"
#include <string.h>

/******************************************************************************
 * ASSEMBLY SIZE LOOKUP TABLES
 ******************************************************************************/

/* Input Assembly Size Map */
typedef struct ASSEMBLY_SIZE_MAP_Ttag {
    uint8_t instance;
    uint8_t size;
    uint16_t io_type_mask;
} ASSEMBLY_SIZE_MAP_T;

static const ASSEMBLY_SIZE_MAP_T g_atInputAssemblySizeMap[] = {
    {1,   2,  IO_TYPE_ALL},
    {2,   3,  IO_TYPE_ALL},
    {3,   5,  IO_TYPE_ALL},
    {4,   5,  IO_TYPE_ALL},
    {5,   7,  IO_TYPE_ALL},
    {6,   8,  IO_TYPE_ALL},
    {10,  1,  IO_TYPE_ALL},
    {11,  6,  IO_TYPE_ALL},
    {17,  4,  IO_TYPE_ALL},
    {18,  5,  IO_TYPE_ALL},
    {19,  9,  IO_TYPE_NO_STROBE},
    {20,  9,  IO_TYPE_NO_STROBE},
    {21,  13, IO_TYPE_NO_STROBE},
    {22,  14, IO_TYPE_NO_STROBE},
    {26,  10, IO_TYPE_NO_STROBE},
    {100, 7,  IO_TYPE_ALL},
    {101, 7,  IO_TYPE_ALL},
    {104, 23, IO_TYPE_NO_STROBE},
    {105, 11, IO_TYPE_NO_STROBE},
    {106, 11, IO_TYPE_NO_STROBE},
    {109, 29, IO_TYPE_NO_STROBE},
    {111, 34, IO_TYPE_NO_STROBE},
    {113, 34, IO_TYPE_NO_STROBE},
    {150, 9,  IO_TYPE_NO_STROBE}
};

static const ASSEMBLY_SIZE_MAP_T g_atOutputAssemblySizeMap[] = {
    {7,   4,  IO_TYPE_NO_STROBE},
    {8,   5,  IO_TYPE_NO_STROBE},
    {23,  6,  IO_TYPE_NO_STROBE},
    {24,  7,  IO_TYPE_NO_STROBE},
    {32,  17, IO_TYPE_NO_STROBE},
    {102, 8,  IO_TYPE_NO_STROBE},
    {103, 6,  IO_TYPE_NO_STROBE},
    {107, 12, IO_TYPE_NO_STROBE},
    {108, 8,  IO_TYPE_NO_STROBE},
    {110, 17, IO_TYPE_NO_STROBE},
    {112, 18, IO_TYPE_NO_STROBE}
};

#define INPUT_ASSEMBLY_COUNT  (sizeof(g_atInputAssemblySizeMap) / sizeof(ASSEMBLY_SIZE_MAP_T))
#define OUTPUT_ASSEMBLY_COUNT (sizeof(g_atOutputAssemblySizeMap) / sizeof(ASSEMBLY_SIZE_MAP_T))

/******************************************************************************
 * ASSEMBLY VALIDATION FUNCTIONS
 ******************************************************************************/

uint8_t VAT_Assembly_IsValidInput(uint8_t instance)
{
    for (uint8_t i = 0; i < INPUT_ASSEMBLY_COUNT; i++) {
        if (g_atInputAssemblySizeMap[i].instance == instance) {
            return 1;
        }
    }
    return 0;
}

uint8_t VAT_Assembly_IsValidOutput(uint8_t instance)
{
    for (uint8_t i = 0; i < OUTPUT_ASSEMBLY_COUNT; i++) {
        if (g_atOutputAssemblySizeMap[i].instance == instance) {
            return 1;
        }
    }
    return 0;
}

uint8_t VAT_Assembly_GetInputSize(uint8_t instance)
{
    for (uint8_t i = 0; i < INPUT_ASSEMBLY_COUNT; i++) {
        if (g_atInputAssemblySizeMap[i].instance == instance) {
            return g_atInputAssemblySizeMap[i].size;
        }
    }
    return 0;
}

uint8_t VAT_Assembly_GetOutputSize(uint8_t instance)
{
    for (uint8_t i = 0; i < OUTPUT_ASSEMBLY_COUNT; i++) {
        if (g_atOutputAssemblySizeMap[i].instance == instance) {
            return g_atOutputAssemblySizeMap[i].size;
        }
    }
    return 0;
}

/******************************************************************************
 * ASSEMBLY SELECTION FUNCTIONS
 ******************************************************************************/

int32_t VAT_Assembly_SelectInput(ASSEMBLY_MANAGER_T* ptManager, uint8_t instance)
{
    if (!VAT_Assembly_IsValidInput(instance)) {
        return -1;  /* Invalid instance */
    }

    ptManager->active_input_instance = instance;
    return 0;
}

int32_t VAT_Assembly_SelectOutput(ASSEMBLY_MANAGER_T* ptManager, uint8_t instance)
{
    if (!VAT_Assembly_IsValidOutput(instance)) {
        return -1;  /* Invalid instance */
    }

    ptManager->active_output_instance = instance;
    return 0;
}
```

### 4.2 DeviceNet 연결 협상 통합

**파일**: `Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c` 수정

```c
/* Global Assembly Manager */
extern ASSEMBLY_MANAGER_T g_tAssemblyManager;

/* Assembly configuration callback (호출: DeviceNet Stack) */
uint32_t AppDNS_HandleAssemblyConfig(uint16_t usInputInstance, uint16_t usOutputInstance,
                                     uint16_t* pusInputSize, uint16_t* pusOutputSize)
{
    uint32_t ulResult = SUCCESS;

    /* Validate and select Input Assembly */
    if (VAT_Assembly_IsValidInput((uint8_t)usInputInstance)) {
        *pusInputSize = VAT_Assembly_GetInputSize((uint8_t)usInputInstance);
        VAT_Assembly_SelectInput(&g_tAssemblyManager, (uint8_t)usInputInstance);
    } else {
        ulResult = ERROR_INVALID_PARAMETER;
    }

    /* Validate and select Output Assembly */
    if (ulResult == SUCCESS && VAT_Assembly_IsValidOutput((uint8_t)usOutputInstance)) {
        *pusOutputSize = VAT_Assembly_GetOutputSize((uint8_t)usOutputInstance);
        VAT_Assembly_SelectOutput(&g_tAssemblyManager, (uint8_t)usOutputInstance);
    } else {
        ulResult = ERROR_INVALID_PARAMETER;
    }

    return ulResult;
}
```

---

## Phase 5: CIP 파라미터 구현 (예상: 5일)

### 5.1 주요 파라미터 상세 사양 (Param1-20)

#### Param1: Vendor ID
```c
{
    .param_id = 1,
    .class_id = 0x01,      /* Identity Object */
    .instance_id = 0x01,
    .attribute_id = 0x01,
    .descriptor = PARAM_DESC_READ_ONLY | PARAM_DESC_ENUM,
    .data_type = CIP_DATA_TYPE_UINT,
    .data_size = 2,
    .name = "Vendor ID",
    .units = "",
    .help = "Identification of vendor",
    .min_value = 404,
    .max_value = 404,
    .default_value = 404,
    .data = {0x94, 0x01}  /* 404 in little-endian */
}
```

#### Param2: Manufacturer's Model Number
```c
{
    .param_id = 2,
    .class_id = 0x30,      /* S-Device Supervisor */
    .instance_id = 0x01,
    .attribute_id = 0x06,
    .descriptor = PARAM_DESC_READ_ONLY,
    .data_type = CIP_DATA_TYPE_STRING,
    .data_size = 1,
    .name = "Manufacturer's Model Number",
    .units = "",
    .help = "VAT part number",
    .min_value = 0,
    .max_value = 20,
    .default_value = 0,
    .data = ""  /* Empty string, to be filled from hardware */
}
```

#### Param3: Product Variant
```c
{
    .param_id = 3,
    .class_id = 0x01,
    .instance_id = 0x01,
    .attribute_id = 0x64,
    .descriptor = PARAM_DESC_READ_ONLY,
    .data_type = CIP_DATA_TYPE_STRING,
    .data_size = 1,
    .name = "Product Variant",
    .units = "",
    .help = "Valve Type",
    .min_value = 0,
    .max_value = 32,
    .default_value = 0,
    .data = "650"  /* Product code as string */
}
```

#### Param4: Serial Number
```c
{
    .param_id = 4,
    .class_id = 0x01,
    .instance_id = 0x01,
    .attribute_id = 0x65,
    .descriptor = PARAM_DESC_READ_ONLY,
    .data_type = CIP_DATA_TYPE_STRING,
    .data_size = 1,
    .name = "Serial Number",
    .units = "",
    .help = "DeviceNet Serial Number",
    .min_value = 8,
    .max_value = 8,
    .default_value = 0,
    .data = "00000000"  /* To be filled from EEPROM/Flash */
}
```

#### Param5: Device Status
```c
{
    .param_id = 5,
    .class_id = 0x30,
    .instance_id = 0x01,
    .attribute_id = 0x0B,
    .descriptor = PARAM_DESC_READ_ONLY | PARAM_DESC_ENUM | PARAM_DESC_MONITORING,
    .data_type = CIP_DATA_TYPE_USINT,
    .data_size = 1,
    .name = "Device Status",
    .units = "",
    .help = "State of the S-Device Supervisor Object",
    .min_value = 0,
    .max_value = 255,
    .default_value = 0,
    /* Enum values:
     * 0: Undefined
     * 1: Self Testing
     * 2: Idle
     * 3: Self-Test Exception
     * 4: Executing
     * 5: Abort
     * 6: Critical Fault
     */
}
```

#### Param6: Controller Mode
```c
{
    .param_id = 6,
    .class_id = 0x64,      /* Valve Object */
    .instance_id = 0x01,
    .attribute_id = 0x67,
    .descriptor = PARAM_DESC_READ_ONLY | PARAM_DESC_ENUM | PARAM_DESC_MONITORING,
    .data_type = CIP_DATA_TYPE_USINT,
    .data_size = 1,
    .name = "Controller Mode",
    .units = "",
    .help = "Actual operating mode of the valve",
    .min_value = 0,
    .max_value = 14,
    .default_value = 0,
    /* Enum values:
     * 0: Init, 1: Sync, 2: Position, 3: Close, 4: Open, 5: Pressure,
     * 6: Hold, 7: Autolearning, 8: Open Interlock, 9: Close Interlock,
     * 10: Maintenance Open, 11: Maintenance Close, 12: Power Fail, 14: Fatal Error
     */
}
```

#### Param7: Access Mode
```c
{
    .param_id = 7,
    .class_id = 0x64,
    .instance_id = 0x01,
    .attribute_id = 0x6B,
    .descriptor = PARAM_DESC_ENUM | PARAM_DESC_MONITORING,
    .data_type = CIP_DATA_TYPE_USINT,
    .data_size = 1,
    .name = "Access Mode",
    .units = "",
    .help = "Access state of the controller",
    .min_value = 0,
    .max_value = 2,
    .default_value = 0,
    /* Enum values:
     * 0: Local
     * 1: Remote
     * 2: Locked
     */
}
```

#### Param8: Data Type
```c
{
    .param_id = 8,
    .class_id = 0x31,      /* Assembly Configuration */
    .instance_id = 0x01,
    .attribute_id = 0x03,
    .descriptor = PARAM_DESC_ENUM,
    .data_type = CIP_DATA_TYPE_USINT,
    .data_size = 1,
    .name = "Data Type",
    .units = "",
    .help = "Data Type for pressure and position values",
    .min_value = 195,
    .max_value = 202,
    .default_value = 195,  /* INT */
    /* Enum values:
     * 0xC3 (195): INT
     * 0xCA (202): REAL
     */
}
```

#### Param9: Pressure Units
```c
{
    .param_id = 9,
    .class_id = 0x31,
    .instance_id = 0x01,
    .attribute_id = 0x04,
    .descriptor = PARAM_DESC_ENUM,
    .data_type = CIP_DATA_TYPE_UINT,
    .data_size = 2,
    .name = "Pressure Units",
    .units = "",
    .help = "Data Units for pressure values",
    .min_value = 0x1001,
    .max_value = 0x130B,
    .default_value = 0x1301,  /* Torr */
    /* Enum values:
     * 0x1001: Counts
     * 0x1007: Percent
     * 0x1300: psi
     * 0x1301: Torr (DEFAULT)
     * 0x1302: mTorr
     * 0x1307: Bar
     * 0x1308: mBar
     * 0x1309: Pa
     * 0x130B: atm
     */
}
```

#### Param10: Position Units
```c
{
    .param_id = 10,
    .class_id = 0x31,
    .instance_id = 0x03,
    .attribute_id = 0x04,
    .descriptor = PARAM_DESC_ENUM,
    .data_type = CIP_DATA_TYPE_UINT,
    .data_size = 2,
    .name = "Position Units",
    .units = "",
    .help = "Data Units for position values",
    .min_value = 0x1001,
    .max_value = 0x1703,
    .default_value = 0x1007,  /* Percent */
    /* Enum values:
     * 0x1001: Counts
     * 0x1007: Percent (DEFAULT)
     * 0x1703: Degrees
     */
}
```

#### Param11: Poll Output Assembly
```c
{
    .param_id = 11,
    .class_id = 0x05,      /* Assembly Selection */
    .instance_id = 0x00,
    .attribute_id = 0x64,
    .descriptor = PARAM_DESC_ENUM,
    .data_type = CIP_DATA_TYPE_USINT,
    .data_size = 1,
    .name = "Poll Output",
    .units = "",
    .help = "Output assembly number",
    .min_value = 7,
    .max_value = 112,
    .default_value = 8,
    /* Enum values: 7, 8, 23, 24, 32, 102, 103, 107, 108, 110, 112 */
}
```

#### Param12: Poll Input Assembly
```c
{
    .param_id = 12,
    .class_id = 0x05,
    .instance_id = 0x00,
    .attribute_id = 0x65,
    .descriptor = PARAM_DESC_ENUM,
    .data_type = CIP_DATA_TYPE_USINT,
    .data_size = 1,
    .name = "Poll Input",
    .units = "",
    .help = "Input assembly number",
    .min_value = 1,
    .max_value = 150,
    .default_value = 100,
    /* Enum values: 1,2,3,4,5,6,10,11,17,18,19,20,21,22,26,100,101,104,105,106,109,111,113,150 */
}
```

### 5.2 파라미터 초기화 구현

**파일**: `Hil_DemoApp/Sources/App_VAT_Parameters.c` (신규)

```c
#include "App_VAT_Parameters.h"
#include <string.h>

/* Global Parameter Manager */
static VAT_PARAM_MANAGER_T g_tParamManager;

void VAT_Param_Init(VAT_PARAM_MANAGER_T* ptManager)
{
    memset(ptManager, 0, sizeof(VAT_PARAM_MANAGER_T));

    /* Initialize all 99 parameters */
    /* Param1: Vendor ID */
    ptManager->params[0].param_id = 1;
    ptManager->params[0].class_id = 0x01;
    ptManager->params[0].instance_id = 0x01;
    ptManager->params[0].attribute_id = 0x01;
    ptManager->params[0].descriptor = PARAM_DESC_READ_ONLY | PARAM_DESC_ENUM;
    ptManager->params[0].data_type = CIP_DATA_TYPE_UINT;
    ptManager->params[0].data_size = 2;
    strcpy(ptManager->params[0].name, "Vendor ID");
    strcpy(ptManager->params[0].help, "Identification of vendor");
    ptManager->params[0].min_value = 404;
    ptManager->params[0].max_value = 404;
    ptManager->params[0].default_value = 404;
    *((uint16_t*)ptManager->params[0].data) = 404;

    /* Param5: Device Status */
    ptManager->params[4].param_id = 5;
    ptManager->params[4].class_id = 0x30;
    ptManager->params[4].instance_id = 0x01;
    ptManager->params[4].attribute_id = 0x0B;
    ptManager->params[4].descriptor = PARAM_DESC_READ_ONLY | PARAM_DESC_ENUM | PARAM_DESC_MONITORING;
    ptManager->params[4].data_type = CIP_DATA_TYPE_USINT;
    ptManager->params[4].data_size = 1;
    strcpy(ptManager->params[4].name, "Device Status");
    strcpy(ptManager->params[4].help, "State of the S-Device Supervisor Object");
    ptManager->params[4].min_value = 0;
    ptManager->params[4].max_value = 255;
    ptManager->params[4].default_value = 0;
    ptManager->params[4].data[0] = DEV_STATUS_IDLE;

    /* ... Initialize remaining parameters (Param2-99) ... */

    ptManager->param_count = 99;
}

int32_t VAT_Param_Get(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id, void* pData, uint8_t* pSize)
{
    if (param_id < 1 || param_id > 99) {
        return -1;
    }

    VAT_PARAMETER_T* pParam = &ptManager->params[param_id - 1];

    if (pSize) {
        *pSize = pParam->data_size;
    }

    if (pData) {
        memcpy(pData, pParam->data, pParam->data_size);
    }

    return 0;
}

int32_t VAT_Param_Set(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id, const void* pData, uint8_t size)
{
    if (param_id < 1 || param_id > 99) {
        return -1;
    }

    VAT_PARAMETER_T* pParam = &ptManager->params[param_id - 1];

    /* Check Read-Only */
    if (pParam->descriptor & PARAM_DESC_READ_ONLY) {
        return -2;  /* Read-only parameter */
    }

    /* Validate size */
    if (size != pParam->data_size) {
        return -3;  /* Invalid size */
    }

    /* Validate range (for numeric types) */
    if (pParam->data_type == CIP_DATA_TYPE_USINT ||
        pParam->data_type == CIP_DATA_TYPE_UINT ||
        pParam->data_type == CIP_DATA_TYPE_INT) {

        int32_t value = 0;
        switch (pParam->data_type) {
            case CIP_DATA_TYPE_USINT:
                value = *(uint8_t*)pData;
                break;
            case CIP_DATA_TYPE_UINT:
                value = *(uint16_t*)pData;
                break;
            case CIP_DATA_TYPE_INT:
                value = *(int16_t*)pData;
                break;
        }

        if (value < pParam->min_value || value > pParam->max_value) {
            return -4;  /* Out of range */
        }
    }

    /* Set data */
    memcpy(pParam->data, pData, size);

    /* Mark as modified */
    VAT_Param_SetModified(ptManager, param_id);

    return 0;
}
```

---

## Phase 6: Data Type 및 Units 변환 (예상: 2일)

### 6.1 Data Type 변환 함수

**파일**: `Hil_DemoApp/Sources/App_VAT_Conversion.c` (신규)

```c
#include "App_VAT_Conversion.h"
#include "App_VAT_Parameters.h"
#include <math.h>

/******************************************************************************
 * PRESSURE CONVERSION CONSTANTS
 ******************************************************************************/

/* Pressure conversion factors (all to/from Torr) */
#define PRESSURE_COUNTS_TO_TORR(counts, max_torr)  ((float)(counts) * (max_torr) / 32767.0f)
#define PRESSURE_TORR_TO_COUNTS(torr, max_torr)    ((int16_t)((torr) * 32767.0f / (max_torr)))

/* Standard conversion factors to Torr */
#define PSI_TO_TORR       51.71493f
#define BAR_TO_TORR       750.0616f
#define MBAR_TO_TORR      0.750062f
#define PA_TO_TORR        0.007501f
#define ATM_TO_TORR       760.0f
#define MTORR_TO_TORR     0.001f

/******************************************************************************
 * POSITION CONVERSION CONSTANTS
 ******************************************************************************/

#define POSITION_COUNTS_MAX     32767.0f
#define POSITION_PERCENT_MAX    100.0f
#define POSITION_DEGREES_MAX    90.0f   /* Valve rotation range */

/******************************************************************************
 * INT ↔ REAL CONVERSION FUNCTIONS
 ******************************************************************************/

/* Convert INT16 pressure to REAL (float) */
float VAT_ConvertPressureIntToReal(int16_t int_pressure, uint16_t pressure_units, float max_pressure_torr)
{
    float pressure_torr = PRESSURE_COUNTS_TO_TORR(int_pressure, max_pressure_torr);

    /* Convert from Torr to target units */
    switch (pressure_units) {
        case PRESSURE_UNIT_COUNTS:
            return (float)int_pressure;
        case PRESSURE_UNIT_PERCENT:
            return (pressure_torr / max_pressure_torr) * 100.0f;
        case PRESSURE_UNIT_TORR:
            return pressure_torr;
        case PRESSURE_UNIT_MTORR:
            return pressure_torr / MTORR_TO_TORR;
        case PRESSURE_UNIT_PSI:
            return pressure_torr / PSI_TO_TORR;
        case PRESSURE_UNIT_BAR:
            return pressure_torr / BAR_TO_TORR;
        case PRESSURE_UNIT_MBAR:
            return pressure_torr / MBAR_TO_TORR;
        case PRESSURE_UNIT_PA:
            return pressure_torr / PA_TO_TORR;
        case PRESSURE_UNIT_ATM:
            return pressure_torr / ATM_TO_TORR;
        default:
            return pressure_torr;
    }
}

/* Convert REAL (float) pressure to INT16 */
int16_t VAT_ConvertPressureRealToInt(float real_pressure, uint16_t pressure_units, float max_pressure_torr)
{
    float pressure_torr = 0.0f;

    /* Convert to Torr first */
    switch (pressure_units) {
        case PRESSURE_UNIT_COUNTS:
            return (int16_t)real_pressure;
        case PRESSURE_UNIT_PERCENT:
            pressure_torr = (real_pressure / 100.0f) * max_pressure_torr;
            break;
        case PRESSURE_UNIT_TORR:
            pressure_torr = real_pressure;
            break;
        case PRESSURE_UNIT_MTORR:
            pressure_torr = real_pressure * MTORR_TO_TORR;
            break;
        case PRESSURE_UNIT_PSI:
            pressure_torr = real_pressure * PSI_TO_TORR;
            break;
        case PRESSURE_UNIT_BAR:
            pressure_torr = real_pressure * BAR_TO_TORR;
            break;
        case PRESSURE_UNIT_MBAR:
            pressure_torr = real_pressure * MBAR_TO_TORR;
            break;
        case PRESSURE_UNIT_PA:
            pressure_torr = real_pressure * PA_TO_TORR;
            break;
        case PRESSURE_UNIT_ATM:
            pressure_torr = real_pressure * ATM_TO_TORR;
            break;
        default:
            pressure_torr = real_pressure;
    }

    return PRESSURE_TORR_TO_COUNTS(pressure_torr, max_pressure_torr);
}

/* Convert INT16 position to REAL (float) */
float VAT_ConvertPositionIntToReal(int16_t int_position, uint16_t position_units)
{
    float position_percent = ((float)int_position / POSITION_COUNTS_MAX) * 100.0f;

    switch (position_units) {
        case POSITION_UNIT_COUNTS:
            return (float)int_position;
        case POSITION_UNIT_PERCENT:
            return position_percent;
        case POSITION_UNIT_DEGREES:
            return (position_percent / 100.0f) * POSITION_DEGREES_MAX;
        default:
            return position_percent;
    }
}

/* Convert REAL (float) position to INT16 */
int16_t VAT_ConvertPositionRealToInt(float real_position, uint16_t position_units)
{
    float position_percent = 0.0f;

    switch (position_units) {
        case POSITION_UNIT_COUNTS:
            return (int16_t)real_position;
        case POSITION_UNIT_PERCENT:
            position_percent = real_position;
            break;
        case POSITION_UNIT_DEGREES:
            position_percent = (real_position / POSITION_DEGREES_MAX) * 100.0f;
            break;
        default:
            position_percent = real_position;
    }

    return (int16_t)((position_percent / 100.0f) * POSITION_COUNTS_MAX);
}

/******************************************************************************
 * UNITS CONVERSION FUNCTIONS
 ******************************************************************************/

/* Convert pressure between units (both REAL) */
float VAT_ConvertPressure(float value, uint16_t from_units, uint16_t to_units)
{
    if (from_units == to_units) {
        return value;
    }

    /* Convert to Torr first */
    float value_torr = 0.0f;
    switch (from_units) {
        case PRESSURE_UNIT_COUNTS:
            /* Cannot convert from counts without max_pressure_torr */
            return value;
        case PRESSURE_UNIT_PERCENT:
            /* Cannot convert from percent without max_pressure_torr */
            return value;
        case PRESSURE_UNIT_TORR:
            value_torr = value;
            break;
        case PRESSURE_UNIT_MTORR:
            value_torr = value * MTORR_TO_TORR;
            break;
        case PRESSURE_UNIT_PSI:
            value_torr = value * PSI_TO_TORR;
            break;
        case PRESSURE_UNIT_BAR:
            value_torr = value * BAR_TO_TORR;
            break;
        case PRESSURE_UNIT_MBAR:
            value_torr = value * MBAR_TO_TORR;
            break;
        case PRESSURE_UNIT_PA:
            value_torr = value * PA_TO_TORR;
            break;
        case PRESSURE_UNIT_ATM:
            value_torr = value * ATM_TO_TORR;
            break;
        default:
            return value;
    }

    /* Convert from Torr to target units */
    switch (to_units) {
        case PRESSURE_UNIT_TORR:
            return value_torr;
        case PRESSURE_UNIT_MTORR:
            return value_torr / MTORR_TO_TORR;
        case PRESSURE_UNIT_PSI:
            return value_torr / PSI_TO_TORR;
        case PRESSURE_UNIT_BAR:
            return value_torr / BAR_TO_TORR;
        case PRESSURE_UNIT_MBAR:
            return value_torr / MBAR_TO_TORR;
        case PRESSURE_UNIT_PA:
            return value_torr / PA_TO_TORR;
        case PRESSURE_UNIT_ATM:
            return value_torr / ATM_TO_TORR;
        default:
            return value_torr;
    }
}

/* Convert position between units (both REAL) */
float VAT_ConvertPosition(float value, uint16_t from_units, uint16_t to_units)
{
    if (from_units == to_units) {
        return value;
    }

    /* Convert to percent first */
    float value_percent = 0.0f;
    switch (from_units) {
        case POSITION_UNIT_COUNTS:
            value_percent = (value / POSITION_COUNTS_MAX) * 100.0f;
            break;
        case POSITION_UNIT_PERCENT:
            value_percent = value;
            break;
        case POSITION_UNIT_DEGREES:
            value_percent = (value / POSITION_DEGREES_MAX) * 100.0f;
            break;
        default:
            return value;
    }

    /* Convert from percent to target units */
    switch (to_units) {
        case POSITION_UNIT_COUNTS:
            return (value_percent / 100.0f) * POSITION_COUNTS_MAX;
        case POSITION_UNIT_PERCENT:
            return value_percent;
        case POSITION_UNIT_DEGREES:
            return (value_percent / 100.0f) * POSITION_DEGREES_MAX;
        default:
            return value_percent;
    }
}
```

---

## Phase 7: Flash 저장/복구 (예상: 2일)

### 7.1 Flash Storage 구조

**파일**: `Hil_DemoApp/Includes/App_VAT_Flash.h` (신규)

```c
#ifndef APP_VAT_FLASH_H_
#define APP_VAT_FLASH_H_

#include <stdint.h>

/******************************************************************************
 * FLASH STORAGE CONFIGURATION
 ******************************************************************************/

/* STM32 F429 Flash Sector 11 for parameter storage */
#define VAT_FLASH_SECTOR        11
#define VAT_FLASH_BASE_ADDR     0x080E0000
#define VAT_FLASH_SIZE          (128 * 1024)  /* 128 KB */

/* Magic number for validity check */
#define VAT_FLASH_MAGIC         0x56415430    /* "VAT0" */

/* Storage format version */
#define VAT_FLASH_VERSION       1

/******************************************************************************
 * FLASH STORAGE STRUCTURE
 ******************************************************************************/

typedef struct VAT_PARAM_STORAGE_Ttag {
    uint32_t magic_number;          /* 0x56415430 ("VAT0") */
    uint32_t version;               /* Storage format version */
    uint32_t crc32;                 /* CRC32 checksum */
    uint32_t timestamp;             /* Last save timestamp */

    uint8_t param_data[99][32];     /* 99 parameters, max 32 bytes each */
    uint8_t param_valid[13];        /* Valid flags (99 bits = 13 bytes) */

    uint8_t assembly_config[16];    /* Assembly configuration */
    uint8_t reserved[448];          /* Reserved for future use */
} VAT_PARAM_STORAGE_T;

/* Total size: 4+4+4+4 + 3168 + 13 + 16 + 448 = 3661 bytes */

/******************************************************************************
 * FUNCTION PROTOTYPES
 ******************************************************************************/

/* Flash operations */
int32_t VAT_Flash_EraseSector(uint8_t sector);
int32_t VAT_Flash_Write(uint32_t address, const uint8_t* pData, uint32_t size);
int32_t VAT_Flash_Read(uint32_t address, uint8_t* pData, uint32_t size);

/* Parameter storage operations */
int32_t VAT_Param_SaveToFlash(VAT_PARAM_MANAGER_T* ptManager);
int32_t VAT_Param_LoadFromFlash(VAT_PARAM_MANAGER_T* ptManager);

/* CRC32 calculation */
uint32_t VAT_CRC32_Calculate(const uint8_t* pData, uint32_t size);

#endif /* APP_VAT_FLASH_H_ */
```

### 7.2 Flash 구현

**파일**: `Hil_DemoApp/Sources/App_VAT_Flash.c` (신규)

```c
#include "App_VAT_Flash.h"
#include "App_VAT_Parameters.h"
#include "stm32f4xx_hal.h"
#include <string.h>

/******************************************************************************
 * CRC32 CALCULATION
 ******************************************************************************/

/* CRC32 table */
static const uint32_t crc32_table[256] = {
    /* Standard CRC32 table (generated with polynomial 0xEDB88320) */
    /* ... (256 entries) ... */
};

uint32_t VAT_CRC32_Calculate(const uint8_t* pData, uint32_t size)
{
    uint32_t crc = 0xFFFFFFFF;

    for (uint32_t i = 0; i < size; i++) {
        crc = (crc >> 8) ^ crc32_table[(crc ^ pData[i]) & 0xFF];
    }

    return ~crc;
}

/******************************************************************************
 * FLASH OPERATIONS
 ******************************************************************************/

int32_t VAT_Flash_EraseSector(uint8_t sector)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError = 0;

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector = sector;
    EraseInitStruct.NbSectors = 1;

    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);

    HAL_FLASH_Lock();

    return (status == HAL_OK) ? 0 : -1;
}

int32_t VAT_Flash_Write(uint32_t address, const uint8_t* pData, uint32_t size)
{
    HAL_FLASH_Unlock();

    HAL_StatusTypeDef status = HAL_OK;

    for (uint32_t i = 0; i < size; i++) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address + i, pData[i]);
        if (status != HAL_OK) {
            break;
        }
    }

    HAL_FLASH_Lock();

    return (status == HAL_OK) ? 0 : -1;
}

int32_t VAT_Flash_Read(uint32_t address, uint8_t* pData, uint32_t size)
{
    memcpy(pData, (const void*)address, size);
    return 0;
}

/******************************************************************************
 * PARAMETER STORAGE OPERATIONS
 ******************************************************************************/

int32_t VAT_Param_SaveToFlash(VAT_PARAM_MANAGER_T* ptManager)
{
    VAT_PARAM_STORAGE_T storage;

    /* Fill storage structure */
    storage.magic_number = VAT_FLASH_MAGIC;
    storage.version = VAT_FLASH_VERSION;
    storage.timestamp = HAL_GetTick();

    /* Copy parameter data */
    for (uint8_t i = 0; i < 99; i++) {
        memcpy(storage.param_data[i], ptManager->params[i].data, 32);
    }

    /* Copy valid flags */
    memcpy(storage.param_valid, ptManager->modified, 13);

    /* Calculate CRC32 (exclude magic, version, crc fields) */
    uint8_t* pCrcData = (uint8_t*)&storage.timestamp;
    uint32_t crcSize = sizeof(VAT_PARAM_STORAGE_T) - 12;  /* Exclude magic, version, crc */
    storage.crc32 = VAT_CRC32_Calculate(pCrcData, crcSize);

    /* Erase sector */
    if (VAT_Flash_EraseSector(VAT_FLASH_SECTOR) != 0) {
        return -1;
    }

    /* Write to flash */
    if (VAT_Flash_Write(VAT_FLASH_BASE_ADDR, (const uint8_t*)&storage, sizeof(VAT_PARAM_STORAGE_T)) != 0) {
        return -2;
    }

    return 0;
}

int32_t VAT_Param_LoadFromFlash(VAT_PARAM_MANAGER_T* ptManager)
{
    VAT_PARAM_STORAGE_T storage;

    /* Read from flash */
    VAT_Flash_Read(VAT_FLASH_BASE_ADDR, (uint8_t*)&storage, sizeof(VAT_PARAM_STORAGE_T));

    /* Verify magic number */
    if (storage.magic_number != VAT_FLASH_MAGIC) {
        return -1;  /* Invalid magic */
    }

    /* Verify version */
    if (storage.version != VAT_FLASH_VERSION) {
        return -2;  /* Version mismatch */
    }

    /* Verify CRC32 */
    uint8_t* pCrcData = (uint8_t*)&storage.timestamp;
    uint32_t crcSize = sizeof(VAT_PARAM_STORAGE_T) - 12;
    uint32_t calculated_crc = VAT_CRC32_Calculate(pCrcData, crcSize);

    if (calculated_crc != storage.crc32) {
        return -3;  /* CRC mismatch */
    }

    /* Restore parameter data */
    for (uint8_t i = 0; i < 99; i++) {
        memcpy(ptManager->params[i].data, storage.param_data[i], 32);
    }

    /* Restore valid flags */
    memcpy(ptManager->modified, storage.param_valid, 13);

    return 0;
}
```

---

## Phase 8: I/O 연결 타입 지원 (예상: 2일)

### 8.1 I/O Connection Types

**지원 연결 타입**:
1. **Cyclic (0x08)**: 주기적 자동 전송 (가장 일반적)
2. **Poll (0x01)**: Master 폴링 시 응답
3. **Change of State (0x04)**: 데이터 변경 시만 전송
4. **Strobe (0x02)**: Strobe 신호와 함께 전송

### 8.2 Cyclic Connection 구현

**파일**: `Hil_DemoAppDNS/Sources/AppDNS_IoConnection.c` (수정)

```c
/* Cyclic I/O connection handler */
void AppDNS_HandleCyclicIo(void)
{
    /* Get active assemblies */
    uint8_t input_instance = g_tAssemblyManager.active_input_instance;
    uint8_t output_instance = g_tAssemblyManager.active_output_instance;

    /* Update Input Assembly data (Slave → Master) */
    switch (input_instance) {
        case 100:
            VAT_UpdateInputAssembly100(&g_tAssemblyManager.input_buffers[0]);
            break;
        case 101:
            VAT_UpdateInputAssembly101(&g_tAssemblyManager.input_buffers[1]);
            break;
        /* ... other assemblies ... */
    }

    /* Process Output Assembly data (Master → Slave) */
    switch (output_instance) {
        case 8:
            VAT_ProcessOutputAssembly8(&g_tAssemblyManager.output_buffers[0]);
            break;
        case 24:
            VAT_ProcessOutputAssembly24(&g_tAssemblyManager.output_buffers[1]);
            break;
        /* ... other assemblies ... */
    }
}
```

---

## Phase 9: CIP Explicit Messaging (예상: 3일)

### 9.1 CIP Services 구현

**파일**: `Hil_DemoAppDNS/Sources/AppDNS_ExplicitMsg.c` (신규)

```c
#include "App_VAT_Parameters.h"

/******************************************************************************
 * CIP SERVICE CODES
 ******************************************************************************/

#define CIP_SERVICE_GET_ATTRIBUTE_SINGLE    0x0E
#define CIP_SERVICE_SET_ATTRIBUTE_SINGLE    0x10
#define CIP_SERVICE_GET_ATTRIBUTE_ALL       0x01
#define CIP_SERVICE_RESET                   0x05
#define CIP_SERVICE_SAVE                    0x16

/******************************************************************************
 * CIP ERROR CODES
 ******************************************************************************/

#define CIP_ERROR_SUCCESS                   0x00
#define CIP_ERROR_PATH_SEGMENT_ERROR        0x04
#define CIP_ERROR_INVALID_ATTRIBUTE         0x09
#define CIP_ERROR_ATTRIBUTE_NOT_SETTABLE    0x0E
#define CIP_ERROR_OBJECT_STATE_CONFLICT     0x10

/******************************************************************************
 * GET ATTRIBUTE SINGLE
 ******************************************************************************/

uint8_t CIP_HandleGetAttributeSingle(uint8_t class_id, uint8_t instance_id, uint8_t attribute_id,
                                      uint8_t* pResponse, uint8_t* pResponseSize)
{
    /* Find parameter by CIP path */
    VAT_PARAMETER_T* pParam = VAT_Param_FindByPath(class_id, instance_id, attribute_id);

    if (!pParam) {
        return CIP_ERROR_INVALID_ATTRIBUTE;
    }

    /* Copy parameter data to response */
    memcpy(pResponse, pParam->data, pParam->data_size);
    *pResponseSize = pParam->data_size;

    return CIP_ERROR_SUCCESS;
}

/******************************************************************************
 * SET ATTRIBUTE SINGLE
 ******************************************************************************/

uint8_t CIP_HandleSetAttributeSingle(uint8_t class_id, uint8_t instance_id, uint8_t attribute_id,
                                      const uint8_t* pData, uint8_t dataSize)
{
    /* Find parameter by CIP path */
    VAT_PARAMETER_T* pParam = VAT_Param_FindByPath(class_id, instance_id, attribute_id);

    if (!pParam) {
        return CIP_ERROR_INVALID_ATTRIBUTE;
    }

    /* Check if settable */
    if (pParam->descriptor & PARAM_DESC_READ_ONLY) {
        return CIP_ERROR_ATTRIBUTE_NOT_SETTABLE;
    }

    /* Validate size */
    if (dataSize != pParam->data_size) {
        return CIP_ERROR_INVALID_ATTRIBUTE;
    }

    /* Validate range */
    if (pParam->data_type == CIP_DATA_TYPE_USINT ||
        pParam->data_type == CIP_DATA_TYPE_UINT ||
        pParam->data_type == CIP_DATA_TYPE_INT) {

        int32_t value = 0;
        switch (pParam->data_type) {
            case CIP_DATA_TYPE_USINT:
                value = *(uint8_t*)pData;
                break;
            case CIP_DATA_TYPE_UINT:
                value = *(uint16_t*)pData;
                break;
            case CIP_DATA_TYPE_INT:
                value = *(int16_t*)pData;
                break;
        }

        if (value < pParam->min_value || value > pParam->max_value) {
            return CIP_ERROR_INVALID_ATTRIBUTE;
        }
    }

    /* Set parameter value */
    memcpy(pParam->data, pData, dataSize);

    /* Mark as modified */
    VAT_Param_SetModified(&g_tParamManager, pParam->param_id);

    return CIP_ERROR_SUCCESS;
}
```

---

## Phase 10: 통합 및 테스트 (예상: 5일)

### 10.1 단위 테스트 체크리스트

#### Assembly 테스트
- [ ] 모든 Assembly 구조체 크기 검증
  - Input: 1(2), 2(3), 3(5), ..., 113(34)
  - Output: 7(4), 8(5), 23(6), ..., 112(18)
- [ ] Assembly Union 크기 검증 (최대 크기와 일치)
- [ ] Byte ordering 검증 (Little-Endian)
- [ ] Struct packing 검증 (패딩 없음)

#### Parameter 테스트
- [ ] 99개 Parameter 읽기/쓰기
- [ ] 범위 검증 (min/max)
- [ ] Read-Only Parameter 쓰기 거부
- [ ] Enum 값 검증
- [ ] CIP Path 접근 검증

#### 변환 테스트
- [ ] INT ↔ REAL 변환 정확도
- [ ] Pressure Units 변환 (9가지)
- [ ] Position Units 변환 (3가지)
- [ ] 오버플로우/언더플로우 처리

#### Flash 테스트
- [ ] 저장/복구 정상 동작
- [ ] CRC32 검증
- [ ] Magic Number 검증
- [ ] 전원 차단 시나리오

### 10.2 CYCON.net 통합 테스트

#### Day 1-2: Assembly 테스트
- [ ] Device Scan → 476297.EDS 매칭 확인
- [ ] 24개 Input Assembly 선택 가능 확인
- [ ] 11개 Output Assembly 선택 가능 확인
- [ ] 각 Assembly별 I/O 데이터 송수신
- [ ] Assembly 동적 전환 테스트

#### Day 3: Parameter 테스트
- [ ] Param1-12 읽기 (Identity, Status, Configuration)
- [ ] Param8-10 쓰기 (Data Type, Units)
- [ ] Param11-12 변경으로 Assembly 전환
- [ ] 99개 Parameter 전수 테스트

#### Day 4: 성능 테스트
- [ ] I/O 스캔 주기 측정 (목표: 10ms)
- [ ] Parameter 응답 시간 측정 (목표: <50ms)
- [ ] Assembly 전환 시간 측정 (목표: <100ms)
- [ ] Flash 저장 시간 측정 (목표: <500ms)
- [ ] CPU/RAM 사용량 측정

#### Day 5: 안정성 테스트
- [ ] 24시간 연속 동작 테스트
- [ ] Assembly 반복 전환 (1000회)
- [ ] Parameter 반복 변경 (1000회)
- [ ] 전원 On/Off 반복 (100회)
- [ ] Flash 저장/복구 반복 (100회)

### 10.3 테스트 시나리오

#### 시나리오 1: 기본 동작 검증
```
1. CYCON.net Device Scan
2. VAT Adaptive Pressure Controller 인식 확인
3. 476297.EDS 매칭 확인
4. Default Assembly (Input100, Output8) 연결
5. I/O 데이터 송수신 확인
```

#### 시나리오 2: Assembly 동적 전환
```
1. Param12 변경 (Input100 → Input105)
2. 데이터 타입 확인 (INT → REAL)
3. I/O 데이터 검증
4. Param11 변경 (Output8 → Output24)
5. 제어 명령 검증
```

#### 시나리오 3: Units 변환
```
1. Param9 변경 (Torr → Bar)
2. Pressure 데이터 변환 확인
3. Param10 변경 (Percent → Degrees)
4. Position 데이터 변환 확인
```

#### 시나리오 4: Flash 저장/복구
```
1. Parameter 값 변경 (Param8-12)
2. Flash 저장 실행
3. 전원 Off/On
4. Parameter 값 복구 확인
5. CRC32 검증
```

---

## 📈 성과 지표 및 추적

### 목표 지표

| 항목 | 목표 | 측정 방법 |
|------|------|-----------|
| Assembly 구현율 | 35/35 (100%) | 코드 검토 |
| Parameter 구현율 | 99/99 (100%) | 코드 검토 |
| EDS 호환성 | 100% | CYCON.net 테스트 |
| I/O 스캔 주기 | <10ms | 오실로스코프 |
| I/O CPU 사용률 | <10% @ 100Hz | 프로파일링 |
| Parameter 응답 | <50ms | CYCON.net 로그 |
| Flash 저장 시간 | <500ms | 타이머 측정 |
| RAM 사용량 | <15KB | 링커 맵 파일 |
| Flash 사용량 | <128KB 추가 | 링커 맵 파일 |
| 코드 품질 | 0 Warning, 0 Error | 컴파일러 |

### 진행 상황 추적

| Phase | 작업 내용 | 예상 시간 | 현재 상태 | 완료율 |
|-------|----------|-----------|-----------|--------|
| Phase 1 | 데이터 구조 확장 | 2일 | 미시작 | 0% |
| Phase 2 | Input Assembly 구현 | 3일 | 진행중 (Input100만) | 4% |
| Phase 3 | Output Assembly 구현 | 2일 | 진행중 (Output8만) | 9% |
| Phase 4 | Assembly 동적 선택 | 2일 | 미시작 | 0% |
| Phase 5 | CIP 파라미터 구현 | 5일 | 미시작 | 0% |
| Phase 6 | Data Type/Units 변환 | 2일 | 미시작 | 0% |
| Phase 7 | Flash 저장/복구 | 2일 | 미시작 | 0% |
| Phase 8 | I/O 연결 타입 지원 | 2일 | 미시작 | 0% |
| Phase 9 | CIP Explicit Messaging | 3일 | 미시작 | 0% |
| Phase 10 | 통합 및 테스트 | 5일 | 미시작 | 0% |
| **전체** | **전체 작업** | **28일** | **진행중** | **~5%** |

---

## 💡 주요 결정사항

### 결정 1: Assembly 데이터 구조 방식
- **결정**: Union 타입 사용하여 모든 Assembly를 하나의 버퍼로 관리
- **이유**:
  - 메모리 효율적 (최대 크기만큼만 할당)
  - 타입 안전성 확보 (컴파일 타임 체크)
  - Assembly 전환 시 간단한 포인터 캐스팅
- **영향**:
  - RAM 사용량 약 100 bytes (Input 34 + Output 18 + 메타데이터)
  - 코드 가독성 향상
  - 버그 가능성 감소

### 결정 2: 파라미터 저장 방식
- **결정**: Flash Sector 11 사용, CRC32 체크섬 적용, Double buffering 없음
- **이유**:
  - 비휘발성 메모리 필요 (전원 차단 시에도 유지)
  - STM32 F429의 Flash 구조 활용 (Sector 11: 128KB)
  - CRC32로 데이터 무결성 보장
  - Double buffering은 복잡도 증가 대비 이득 적음
- **영향**:
  - 128KB Flash 영역 사용
  - 저장 시간 약 500ms (Erase + Write)
  - 전원 차단 시 설정 유지

### 결정 3: Data Type 전환 방식
- **결정**: Assembly를 INT/REAL 타입별로 분리, 런타임에 선택
- **이유**:
  - EDS 파일이 이미 INT/REAL Assembly를 분리 정의함
  - 런타임 변환보다 컴파일 타임 타입이 안전
  - Master가 원하는 타입의 Assembly 직접 선택 가능
  - 변환 오버헤드 없음
- **영향**:
  - Assembly 개수 증가 (24개)
  - 구현 복잡도 감소 (타입 변환 로직 불필요)
  - 타입 오류 위험 감소

### 결정 4: I/O 연결 타입 우선순위
- **결정**: Cyclic 연결 우선 구현, Poll/COS는 차순위
- **이유**:
  - Cyclic이 가장 일반적인 연결 방식 (>90% 사용)
  - 구현 복잡도 낮음
  - 대부분의 DeviceNet Master가 Cyclic 우선 지원
- **영향**:
  - Phase 1 릴리스에 Cyclic만 포함
  - Poll/COS는 Phase 2로 연기 가능
  - 초기 호환성 빠르게 확보

### 결정 5: Parameter 접근 방식
- **결정**: Class/Instance/Attribute 경로 기반 접근 (CIP 표준)
- **이유**:
  - CIP 표준 완전 준수
  - EDS 파일 구조와 1:1 매칭
  - 확장성 우수 (향후 Object 추가 용이)
  - CYCON.net의 모든 기능 활용 가능
- **영향**:
  - CIP Explicit Messaging 완전 구현 필요
  - 다른 DeviceNet Master와 호환성 100%
  - 구현 복잡도 증가하지만 표준 준수로 안정성 확보

---

## 🐛 예상 이슈 및 해결 방안

### 이슈 1: Assembly 전환 시 데이터 손실
- **증상**: Assembly 크기 변경 시 기존 데이터가 손실되거나 잘못된 값 전송
- **원인**: 버퍼 크기 불일치, 포인터 캐스팅 오류, 매핑 로직 오류
- **해결 방법**:
  1. Assembly 전환 전 현재 데이터 백업
  2. 새 Assembly로 매핑 가능한 필드 복사 (pressure, position 등)
  3. 매핑 불가능한 필드는 기본값으로 초기화
  4. Assembly 전환 완료 후 Master에 통지
- **재발 방지**:
  - Assembly 전환 시 로그 기록
  - 크기 검증 로직 추가 (assert)
  - 단위 테스트에 전환 시나리오 추가

### 이슈 2: Flash 저장 중 전원 차단
- **증상**: Parameter 저장 중 전원 차단 시 데이터 손상, 다음 부팅 실패
- **원인**: Flash Write가 완료되지 않아 CRC 불일치
- **해결 방법**:
  1. Magic number + Version + CRC32로 유효성 검증
  2. 손상된 경우 기본값으로 초기화
  3. 저장 시작 플래그 관리 (Flash 마지막 바이트 사용)
  4. 부팅 시 무결성 검사 루틴 실행
- **재발 방지**:
  - 저장 전 Battery Backup 확인
  - Critical Parameter는 우선 저장
  - 테스트에 전원 차단 시나리오 추가

### 이슈 3: Floating-point 연산 성능 저하
- **증상**: FP Assembly 사용 시 CPU 부하 증가, I/O 스캔 주기 지연
- **원인**: STM32 F429의 FPU 설정 미흡, 불필요한 FP 연산
- **해결 방법**:
  1. FPU 활성화 확인 (Cortex-M4F 하드웨어 FPU)
  2. 컴파일러 최적화 옵션 설정 (-mfpu=fpv4-sp-d16 -mfloat-abi=hard)
  3. Critical path에서 FP 연산 최소화
  4. 미리 계산 가능한 값은 테이블로 저장
- **재발 방지**:
  - 프로파일링으로 성능 모니터링
  - FP 연산 횟수 측정 및 최적화

### 이슈 4: Parameter 범위 초과 설정
- **증상**: Master가 범위 밖의 값을 Parameter에 설정
- **원인**: Master 설정 오류, EDS 파일 불일치, 사용자 실수
- **해결 방법**:
  1. Set_Attribute 시 항상 범위 검증 (min/max)
  2. 범위 초과 시 CIP 에러 코드 반환 (0x09 - Invalid Attribute Value)
  3. 에러 로그 기록 (Parameter ID, 설정 시도 값)
  4. 현재 값 유지 (변경하지 않음)
- **재발 방지**:
  - 모든 Parameter에 min/max 검증 적용
  - EDS 파일과 코드 일치 확인 (자동화 스크립트)
  - CYCON.net에서 테스트 (범위 밖 값 설정 시도)

### 이슈 5: 메모리 부족 (RAM)
- **증상**: 35개 Assembly + 99개 Parameter 구현 시 RAM 부족
- **원인**: 과도한 메모리 할당, 중복 버퍼
- **해결 방법**:
  1. Union 타입 활용 (최대 크기만 할당)
  2. Assembly 버퍼 공유 (동시 활성은 Input 1개 + Output 1개)
  3. Parameter 데이터만 RAM 유지 (메타데이터는 Flash/ROM)
  4. Stack 사용량 최적화 (지역 변수 크기 제한)
- **재발 방지**:
  - 링커 맵 파일로 메모리 사용량 모니터링
  - Static 분석 도구 사용 (Stack Analyzer)
  - 메모리 사용량 목표 설정 (<15KB 추가)

---

## 🔗 관련 리소스

### 프로젝트 파일
- `D:\git\netx_90_f429_SPI5\476297.eds` - VAT EDS 파일 (2098 lines, 99 parameters, 35 assemblies)
- `D:\git\netx_90_f429_SPI5\20251029_VAT_EDS_Integration_Complete.md` - 기존 작업 보고서 (Identity Object 변경 완료)
- `Hil_DemoAppDNS\Sources\AppDNS_DemoApplicationFunctions.c` - DeviceNet 초기화 및 설정
- `Hil_DemoApp\Includes\App_DemoApplication.h` - 현재 I/O 구조체 (Input100, Output8)
- `Hil_DemoApp\Sources\App_DemoApplication.c` - I/O 처리 로직

### 참고 문서
- **CIP Networks Library Volume 1**: CIP Common Specification (Identity Object, Assembly Object, Parameter Object)
- **CIP Networks Library Volume 3**: DeviceNet Adaptation (I/O Connection, Explicit Messaging, EDS Format)
- **ODVA DeviceNet Specification**: DeviceNet Protocol Details
- **STM32F429 Reference Manual**: Flash, GPIO, SPI, Interrupts
- **netX90 DeviceNet Stack User Manual**: API Reference, Configuration

### 개발 도구
- **STM32CubeIDE 1.x**: 통합 개발 환경
- **CYCON.net DeviceNet Scanner**: EDS 매칭 테스트, Parameter 설정
- **Hilscher netANALYZER**: DeviceNet 프로토콜 분석
- **EZ-EDS Editor**: EDS 파일 편집 및 검증

### 온라인 리소스
- ODVA Website: https://www.odva.org/
- VAT Website: https://www.vatvalve.com/
- Hilscher Community: https://community.hilscher.com/

---

## ⏭️ 다음 작업 (8주 로드맵)

### Week 1: Phase 1 (데이터 구조 확장)
- [ ] `App_VAT_Assemblies.h` 파일 생성
- [ ] 24개 Input Assembly 구조체 정의
- [ ] 11개 Output Assembly 구조체 정의
- [ ] Assembly Union 및 Manager 구조체 정의
- [ ] `App_VAT_Parameters.h` 파일 생성
- [ ] CIP 파라미터 구조체 정의
- [ ] 함수 프로토타입 선언
- [ ] 컴파일 및 크기 검증 (구조체 sizeof 확인)

### Week 2-3: Phase 2-4 (Assembly 구현 및 동적 선택)
- [ ] 24개 Input Assembly 구현 (우선순위별)
- [ ] 11개 Output Assembly 구현 (우선순위별)
- [ ] Assembly Selector 모듈 구현
- [ ] Assembly 크기 Lookup 테이블
- [ ] DeviceNet 연결 협상 콜백 통합
- [ ] CYCON.net Assembly 선택 테스트
- [ ] I/O 데이터 송수신 검증

### Week 4-5: Phase 5 (CIP 파라미터 구현)
- [ ] 99개 Parameter 초기화 함수
- [ ] Identity Parameters (Param1-4)
- [ ] Status Parameters (Param5-7)
- [ ] Configuration Parameters (Param8-12)
- [ ] Control Parameters (Param13-30)
- [ ] Sensor Parameters (Param31-50)
- [ ] Calibration Parameters (Param51-70)
- [ ] Advanced Parameters (Param71-99)
- [ ] Get/Set Attribute 서비스 구현
- [ ] 접근 권한 및 범위 검증

### Week 6: Phase 6-7 (변환 및 저장)
- [ ] Data Type 변환 함수 (INT ↔ REAL)
- [ ] Pressure Units 변환 (9가지 단위)
- [ ] Position Units 변환 (3가지 단위)
- [ ] 정밀도 테스트
- [ ] Flash Driver 구현 (Erase, Write, Read)
- [ ] CRC32 계산 함수
- [ ] Parameter Storage 구조체
- [ ] SaveToFlash/LoadFromFlash 함수
- [ ] 저장/복구 테스트

### Week 7: Phase 8-9 (I/O 연결 및 Messaging)
- [ ] Cyclic Connection 핸들러
- [ ] Poll Connection 핸들러
- [ ] Change of State 감지 로직
- [ ] I/O Type Mask 검증
- [ ] Get_Attribute_Single 서비스
- [ ] Set_Attribute_Single 서비스
- [ ] Get_Attribute_All 서비스
- [ ] Reset/Save 서비스
- [ ] CIP 에러 코드 정의 및 처리

### Week 8: Phase 10 (통합 테스트 및 문서화)
- [ ] 단위 테스트 (Assembly, Parameter, 변환, Flash)
- [ ] CYCON.net 통합 테스트
  - Device Scan 및 EDS 매칭
  - Assembly 선택 및 I/O 데이터
  - Parameter 읽기/쓰기
- [ ] 성능 테스트 (I/O 스캔, Parameter 응답, CPU/RAM)
- [ ] 안정성 테스트 (24시간 연속, 반복 테스트)
- [ ] 문서화
  - 사용자 매뉴얼
  - 코드 주석 보완
  - 테스트 결과 정리
  - Release Note 작성

---

## 💬 비고

### 작업 범위 확정
본 작업은 476297.eds 파일에 정의된 **모든 기능**을 완전히 구현하는 것을 목표로 합니다.

**포함 사항**:
- 24개 Input Assembly 전체
- 11개 Output Assembly 전체
- 99개 CIP Parameter 전체
- 4가지 I/O Connection Type
- Data Type 전환 (INT/REAL)
- 9가지 Pressure Units + 3가지 Position Units
- Flash 기반 파라미터 저장/복구
- CIP Explicit Messaging 완전 구현

**제외 사항**:
- VAT 고유의 압력 제어 알고리즘 (별도 프로젝트)
- PID 튜닝 최적화 (기본 구현만)
- 고급 진단 기능 (Phase 2)

### 개발 기간 및 인력
- **총 예상 기간**: 8주 (40일, 약 2개월)
- **인력**: 펌웨어 개발자 1명 (Full-time)
- **리스크 버퍼**: +2주 (예상치 못한 이슈 대응)
- **최종 예상**: 10주 (50일)

### 마일스톤 (Milestone)
1. **M1 (Week 2 종료)**: Assembly 데이터 구조 완료, 컴파일 성공
2. **M2 (Week 4 종료)**: 모든 Assembly 구현 완료, CYCON.net Assembly 선택 가능
3. **M3 (Week 6 종료)**: 모든 Parameter 구현 완료, Flash 저장/복구 동작
4. **M4 (Week 8 종료)**: 통합 테스트 완료, 릴리스 준비

### 단계적 릴리스 전략
시간이 부족할 경우, 다음 우선순위로 단계적 릴리스 가능:

**Phase 1 Release** (최소 기능):
- Critical Assembly만 구현 (Input100, Output8) ✅ 이미 완료
- 필수 Parameter만 구현 (Param1-12: Identity, Status, Configuration)
- Cyclic Connection만 지원
- Flash 저장 없음 (휘발성)

**Phase 2 Release** (주요 기능):
- High Priority Assembly 추가 (Input6, 21, 22, 101 + Output24, 32, 102)
- 주요 Parameter 추가 (Param13-50: Control, Sensor)
- Poll Connection 추가
- Flash 저장 구현

**Phase 3 Release** (완전 구현):
- 모든 Assembly 구현 (35개)
- 모든 Parameter 구현 (99개)
- 모든 I/O Connection Type 지원
- 완전한 CIP Explicit Messaging
- 최적화 및 성능 튜닝

### 리스크 관리
| 리스크 | 확률 | 영향 | 대응 방안 |
|-------|------|------|----------|
| 메모리 부족 (RAM) | High | High | Union 타입, 버퍼 공유, 메모리 최적화 |
| Flash 저장 불안정 | Medium | High | CRC32, Magic Number, 테스트 강화 |
| FP 연산 성능 저하 | Medium | Medium | FPU 활성화, 최적화, 프로파일링 |
| Master 호환성 문제 | Low | Medium | CIP 표준 준수, CYCON.net 테스트 |
| 일정 지연 | Medium | Medium | 단계적 릴리스, 우선순위 조정 |

### 성공 기준
프로젝트 성공 기준은 다음과 같습니다:

1. **기능 완성도**: 35개 Assembly + 99개 Parameter 100% 구현
2. **EDS 호환성**: 476297.eds와 100% 일치, CYCON.net 인식
3. **성능**: I/O 10ms, Parameter <50ms, CPU <10%
4. **안정성**: 24시간 연속 동작, Flash 저장/복구 100회 성공
5. **코드 품질**: 0 Warning, 0 Error, 주석 충실

---

**작성일**: 2025-11-05
**작성자**: Firmware Development Team
**검토자**: [검토자명]
**승인자**: [승인자명]
**문서 버전**: 1.0
**다음 리뷰**: 2025-11-12 (Phase 1 완료 후)

---

## 📎 부록

### 부록 A: Assembly 전체 목록 및 바이트 맵

[여기에 35개 Assembly의 상세한 바이트 맵 추가 가능]

### 부록 B: Parameter 전체 목록 (Param1-99)

[여기에 99개 Parameter의 상세 사양 추가 가능]

### 부록 C: Units 변환표

#### Pressure Units 변환 계수
| From/To | Counts | Percent | psi | Torr | mTorr | Bar | mBar | Pa | atm |
|---------|--------|---------|-----|------|-------|-----|------|----|----|
| Torr | - | - | 0.01934 | 1.0 | 1000 | 0.001333 | 1.33322 | 133.322 | 0.001316 |

#### Position Units 변환 계수
| From/To | Counts | Percent | Degrees |
|---------|--------|---------|---------|
| Percent | 327.67 | 1.0 | 0.9 |

### 부록 D: 테스트 체크리스트

[여기에 상세한 테스트 체크리스트 추가 가능]

---

**문서 끝**

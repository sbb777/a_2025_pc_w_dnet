# VAT EDS 기반 DeviceNet Explicit Message 통신 구현 계획서

**작성일시**: 2025-11-07 17:10
**프로젝트**: netx_90_f429_SPI5 - VAT Adaptive Pressure Controller
**목적**: VAT EDS를 기반으로 DeviceNet Explicit Message 통신 완전 구현

---

## 1. 프로젝트 개요

### 1.1 현재 상태 분석

**구현 완료 항목**:
- ✅ **I/O Data 통신**: Assembly Object 기반 (Input 100, Output 8)
- ✅ **VAT Parameter Manager**: 99개 파라미터 정의 및 관리
- ✅ **Assembly 구조**: 24개 Input + 11개 Output Assembly
- ✅ **기본 테스트**: 압력 제어, 위치 제어 테스트

**미구현 항목**:
- ❌ **Explicit Message 통신**: CIP 서비스 요청/응답 처리
- ❌ **Parameter 원격 접근**: GET/SET Attribute 서비스
- ❌ **Configuration 서비스**: Assembly 전환, 설정 변경
- ❌ **Diagnostic 서비스**: 상태 조회, 오류 진단

### 1.2 VAT EDS 구조 요약

#### 파라미터 시스템
```c
/* 파라미터 매니저 */
typedef struct VAT_PARAMETER_Ttag {
    uint8_t  param_id;          // Parameter ID (1-99)
    uint8_t  class_id;          // CIP Class ID
    uint8_t  instance_id;       // CIP Instance ID
    uint8_t  attribute_id;      // CIP Attribute ID

    uint16_t descriptor;        // Parameter descriptor
    uint8_t  data_type;         // CIP Data Type (INT, REAL, STRING)
    uint8_t  data_size;         // Data size in bytes

    char name[64];              // Parameter name
    char units[16];             // Units
    int32_t min_value;          // Min value
    int32_t max_value;          // Max value
    int32_t default_value;      // Default value

    uint8_t data[32];           // Current value
} VAT_PARAMETER_T;

/* 파라미터 매니저 */
typedef struct VAT_PARAM_MANAGER_Ttag {
    VAT_PARAMETER_T params[99];
    uint8_t param_count;
    uint8_t modified[13];       // Modified flags
} VAT_PARAM_MANAGER_T;

extern VAT_PARAM_MANAGER_T g_tParamManager;
```

#### Assembly 시스템
```c
/* 기본 Input Assembly 100 (7 bytes) - Slave → Master */
typedef struct INPUT_ASSEMBLY_100_Ttag {
    uint8_t exception_status;   // 예외 상태
    int16_t pressure;           // 압력 (INT)
    int16_t position;           // 위치 (INT)
    uint8_t device_status;      // 장치 상태
    uint8_t access_mode;        // 접근 모드
} INPUT_ASSEMBLY_100_T;

/* 기본 Output Assembly 8 (5 bytes) - Master → Slave */
typedef struct OUTPUT_ASSEMBLY_8_Ttag {
    uint8_t control_mode;       // 제어 모드
    int16_t control_setpoint;   // 제어 설정값
    uint8_t control_instance;   // 제어 인스턴스
    uint8_t reserved;           // 예약
} OUTPUT_ASSEMBLY_8_T;

/* Assembly Manager */
typedef struct ASSEMBLY_MANAGER_Ttag {
    ASSEMBLY_METADATA_T metadata[35];   // 24 Input + 11 Output
    ASSEMBLY_DATA_T input_buffers[24];
    ASSEMBLY_DATA_T output_buffers[11];

    uint8_t active_input_instance;
    uint8_t active_output_instance;
    uint16_t io_connection_type;
} ASSEMBLY_MANAGER_T;
```

### 1.3 목표

**Primary Goals**:
1. ✅ VAT 파라미터를 Explicit Message로 원격 설정/조회
2. ✅ Assembly 구성 동적 전환 (I/O 데이터 형식 변경)
3. ✅ 진단 정보 실시간 조회 (상태, 오류, 통계)
4. ✅ 설정 저장/복원 (Flash 연동)

**Secondary Goals**:
1. ✅ 교정(Calibration) 서비스 구현
2. ✅ 밸브 개별 제어 (Individual Valve Control)
3. ✅ PID 파라미터 튜닝
4. ✅ 네트워크 진단 및 통계

---

## 2. VAT CIP 객체 모델

### 2.1 CIP 객체 계층 구조

```
[Identity Object - Class 0x01]
└── Instance 1: Device Identity

[Message Router - Class 0x02]
└── Instance 1: Message Routing

[DeviceNet Object - Class 0x03]
└── Instance 1: DeviceNet Configuration

[Assembly Object - Class 0x04]
├── Instance 100: Input Assembly (7 bytes)
├── Instance 8: Output Assembly (5 bytes)
├── Instance 1-150: Other Assemblies
└── ...

[Connection Object - Class 0x05]
└── Instances: I/O Connections

[VAT Parameter Object - Class 0x64 (100)]
├── Instance 1: Param1 (Pressure Setpoint)
├── Instance 2: Param2 (Position Setpoint)
├── Instance 3: Param3 (Controller Mode)
├── ...
└── Instance 99: Param99

[VAT Diagnostic Object - Class 0x65 (101)]
├── Instance 1: System Diagnostics
├── Instance 2: Pressure Diagnostics
├── Instance 3: Position Diagnostics
└── Instance 4: Network Statistics

[VAT Calibration Object - Class 0x66 (102)]
├── Instance 1: Pressure Calibration
└── Instance 2: Position Calibration

[VAT Valve Control Object - Class 0x67 (103)]
└── Instance 1-n: Individual Valve Control
```

### 2.2 주요 CIP 서비스

| 서비스 코드 | 이름 | 용도 | VAT 적용 |
|------------|------|------|----------|
| **0x0E** | GET_ATTRIBUTE_SINGLE | 단일 속성 읽기 | 파라미터 조회 |
| **0x10** | SET_ATTRIBUTE_SINGLE | 단일 속성 쓰기 | 파라미터 설정 |
| **0x01** | GET_ATTRIBUTES_ALL | 전체 속성 읽기 | 진단 정보 일괄 조회 |
| **0x02** | SET_ATTRIBUTES_ALL | 전체 속성 쓰기 | 설정 일괄 변경 |
| **0x05** | RESET | 객체 리셋 | 장치 리셋, 오류 클리어 |
| **0x4B** | GET_ATTRIBUTE_LIST | 속성 목록 조회 | 파라미터 목록 |
| **0x32-0x64** | Custom Services | 커스텀 서비스 | VAT 특화 기능 |

### 2.3 VAT Parameter Object (Class 0x64) 정의

**Attribute 매핑**:

| Attr ID | 이름 | 타입 | 크기 | 접근 | 설명 |
|---------|------|------|------|------|------|
| **1** | Pressure Setpoint | INT | 2 | R/W | 압력 설정값 |
| **2** | Position Setpoint | INT | 2 | R/W | 위치 설정값 |
| **3** | Controller Mode | USINT | 1 | R/W | 제어 모드 (0-14) |
| **4** | Control Instance | USINT | 1 | R/W | 제어 인스턴스 |
| **5** | Device Status | USINT | 1 | R | 장치 상태 (0-6) |
| **6** | Exception Status | USINT | 1 | R | 예외 상태 비트마스크 |
| **7** | Access Mode | USINT | 1 | R/W | 접근 모드 (0=Local, 1=Remote) |
| **8** | Current Pressure | INT | 2 | R | 현재 압력 측정값 |
| **9** | Pressure Units | UINT | 2 | R/W | 압력 단위 코드 |
| **10** | Position Units | UINT | 2 | R/W | 위치 단위 코드 |
| **11** | Current Position | INT | 2 | R | 현재 위치 측정값 |
| **12** | Auto Learn Enable | USINT | 1 | R/W | 자동 학습 활성화 |
| **13** | Calibration Scale | USINT | 1 | R/W | 교정 스케일 |
| **14** | Zero Adjust | USINT | 1 | W | 제로 조정 트리거 |
| **15** | Valve Address | USINT | 1 | R/W | 밸브 주소 |
| **16** | Valve Action | USINT | 1 | W | 밸브 동작 커맨드 |
| **17-99** | Other Parameters | Various | Varies | R/W | 추가 파라미터 |

**Example - Param1 (Pressure Setpoint)**:
```c
VAT_PARAMETER_T param1 = {
    .param_id = 1,
    .class_id = 0x64,       // VAT Parameter Object
    .instance_id = 1,       // Instance 1
    .attribute_id = 1,      // Attribute 1

    .descriptor = 0x0000,   // Read/Write
    .data_type = CIP_DATA_TYPE_INT,
    .data_size = 2,

    .name = "Pressure Setpoint",
    .units = "mTorr",
    .help = "Target pressure control setpoint",

    .min_value = 0,
    .max_value = 1000,
    .default_value = 500,

    .data = {0xF4, 0x01}    // 500 (Little Endian)
};
```

### 2.4 VAT Diagnostic Object (Class 0x65) 정의

**Attribute 매핑**:

| Attr ID | 이름 | 타입 | 크기 | 접근 | 설명 |
|---------|------|------|------|------|------|
| **1** | Uptime Seconds | UDINT | 4 | R | 시스템 가동 시간 (초) |
| **2** | Total Cycles | UDINT | 4 | R | 총 제어 사이클 수 |
| **3** | Error Count | UINT | 2 | R | 총 오류 발생 횟수 |
| **4** | Last Error Code | UINT | 2 | R | 마지막 오류 코드 |
| **5** | Last Error Timestamp | UDINT | 4 | R | 마지막 오류 발생 시간 |
| **6** | Pressure Min | INT | 2 | R | 최소 압력 기록 |
| **7** | Pressure Max | INT | 2 | R | 최대 압력 기록 |
| **8** | Pressure Average | INT | 2 | R | 평균 압력 (최근 100회) |
| **9** | Position Min | INT | 2 | R | 최소 위치 기록 |
| **10** | Position Max | INT | 2 | R | 최대 위치 기록 |
| **11** | Network RX Count | UDINT | 4 | R | 수신 패킷 수 |
| **12** | Network TX Count | UDINT | 4 | R | 송신 패킷 수 |
| **13** | Network Error Count | UINT | 2 | R | 네트워크 오류 수 |
| **14** | Temperature | INT | 2 | R | 시스템 온도 (섭씨) |
| **15** | Firmware Version | UDINT | 4 | R | 펌웨어 버전 |

**데이터 구조**:
```c
typedef struct VAT_DIAGNOSTIC_DATA_Ttag {
    uint32_t ulUptimeSeconds;
    uint32_t ulTotalCycles;
    uint16_t usErrorCount;
    uint16_t usLastErrorCode;
    uint32_t ulLastErrorTimestamp;

    int16_t sPressureMin;
    int16_t sPressureMax;
    int16_t sPressureAvg;

    int16_t sPositionMin;
    int16_t sPositionMax;

    uint32_t ulNetworkRxCount;
    uint32_t ulNetworkTxCount;
    uint16_t usNetworkErrorCount;

    int16_t sTemperature;
    uint32_t ulFirmwareVersion;
} VAT_DIAGNOSTIC_DATA_T;

static VAT_DIAGNOSTIC_DATA_T g_tVatDiagnostics = {0};
```

---

## 3. 구현 아키텍처

### 3.1 전체 시스템 구조

```
┌─────────────────────────────────────────────────────────┐
│               DeviceNet Master                          │
│  (RSLogix, Studio 5000, etc.)                          │
└────────────────┬────────────────────────────────────────┘
                 │
                 │ DeviceNet Network
                 │
┌────────────────▼────────────────────────────────────────┐
│           Hilscher netX90 Device                        │
│  ┌─────────────────────────────────────────────────┐   │
│  │        DeviceNet Protocol Stack                 │   │
│  │  (Hilscher Firmware)                           │   │
│  └─────────────────┬───────────────────────────────┘   │
│                    │                                     │
│  ┌─────────────────▼───────────────────────────────┐   │
│  │     Packet Handler (Callback)                   │   │
│  │  - AppDNS_PacketHandler_callback()             │   │
│  │  - CIP Service Request Routing                  │   │
│  └─────────────────┬───────────────────────────────┘   │
│                    │                                     │
│     ┌──────────────┴──────────────┐                    │
│     │                               │                    │
│  ┌──▼───────────────┐    ┌────────▼─────────────┐     │
│  │  I/O Data Path   │    │ Explicit Message Path│     │
│  │  (Assembly)      │    │  (CIP Services)      │     │
│  └──┬───────────────┘    └────────┬─────────────┘     │
│     │                               │                    │
│  ┌──▼───────────────┐    ┌────────▼─────────────┐     │
│  │ VAT_IoHandler    │    │ VAT_ExplicitHandler  │     │
│  │  - Read Input    │    │  - Route by Class    │     │
│  │  - Write Output  │    │  - Service Dispatch  │     │
│  └──┬───────────────┘    └────────┬─────────────┘     │
│     │                               │                    │
│  ┌──▼──────────────────────────────▼─────────────┐     │
│  │         VAT Application Layer                  │     │
│  │  ┌──────────────┐  ┌──────────────┐          │     │
│  │  │ Parameter    │  │ Diagnostic   │          │     │
│  │  │ Manager      │  │ Manager      │          │     │
│  │  └──────┬───────┘  └──────┬───────┘          │     │
│  │         │                  │                   │     │
│  │  ┌──────▼──────────────────▼─────────────┐   │     │
│  │  │    VAT Control Logic                   │   │     │
│  │  │  - Pressure Control                    │   │     │
│  │  │  - Position Control                    │   │     │
│  │  │  - Calibration                         │   │     │
│  │  │  - Valve Control                       │   │     │
│  │  └────────────────────────────────────────┘   │     │
│  │                                                │     │
│  │  ┌────────────────────────────────────────┐   │     │
│  │  │    Flash Storage                       │   │     │
│  │  │  - Parameter Persistence               │   │     │
│  │  │  - Configuration Backup                │   │     │
│  │  └────────────────────────────────────────┘   │     │
│  └────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

### 3.2 파일 구조

```
Hil_DemoApp/
├── Sources/
│   ├── AppDNS_DemoApplicationFunctions.c      [수정] 패킷 핸들러
│   ├── App_VAT_ExplicitHandler.c              [신규] Explicit Message 핸들러
│   ├── App_VAT_Parameters.c                   [기존] 파라미터 매니저
│   ├── App_VAT_Diagnostic.c                   [신규] 진단 매니저
│   ├── App_VAT_Calibration.c                  [신규] 교정 서비스
│   ├── App_VAT_ValveControl.c                 [신규] 밸브 제어
│   └── App_VAT_Flash.c                        [기존] Flash 저장
│
├── Includes/
│   ├── App_VAT_ExplicitHandler.h              [신규]
│   ├── App_VAT_Parameters.h                   [기존]
│   ├── App_VAT_Diagnostic.h                   [신규]
│   ├── App_VAT_Calibration.h                  [신규]
│   ├── App_VAT_ValveControl.h                 [신규]
│   └── App_VAT_CIP_Objects.h                  [신규] CIP 객체 정의
│
└── Config/
    └── VAT_EDS.eds                            [수정] EDS 파일
```

---

## 4. 단계별 구현 계획

### Phase 1: 기본 인프라 구축 (2-3일)

#### 4.1.1 패킷 핸들러 활성화

**파일**: `AppDNS_DemoApplicationFunctions.c`

**목표**: CIP 서비스 요청을 VAT Explicit Handler로 라우팅

**수정 사항**:
```c
/* DNS_HOST_APP_REGISTRATION 활성화 */
#define DNS_HOST_APP_REGISTRATION

bool AppDNS_PacketHandler_callback(CIFX_PACKET* ptPacket, void* pvUserData)
{
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc =
        (APP_DNS_CHANNEL_HANDLER_RSC_T*)pvUserData;

    /* 1. 패킷 버퍼 검증 */
    if(ptPacket != &g_tDnsChannelRsc.tPacket) {
        printf("ERROR: Unexpected packet resource!\r\n");
        return false;
    }

    /* 2. 커맨드 분기 */
    uint32_t ulCmd = ptPacket->tHeader.ulCmd;

    switch(ulCmd) {
        case DNS_CMD_CIP_SERVICE_IND:
        {
            /* CIP 서비스 요청 → VAT Explicit Handler */
            return VAT_Explicit_HandleCipService(ptDnsRsc,
                (DNS_PACKET_CIP_SERVICE_IND_T*)ptPacket);
        }

        case DNS_CMD_EXPLICIT_READ_IND:
        {
            /* Explicit Read 요청 */
            return VAT_Explicit_HandleRead(ptDnsRsc,
                (DNS_PACKET_EXPLICIT_READ_IND_T*)ptPacket);
        }

        case DNS_CMD_EXPLICIT_WRITE_IND:
        {
            /* Explicit Write 요청 */
            return VAT_Explicit_HandleWrite(ptDnsRsc,
                (DNS_PACKET_EXPLICIT_WRITE_IND_T*)ptPacket);
        }

        default:
        {
            /* Indication 패킷 처리 */
            if((ulCmd & 0x1) == 0) {
                /* 미지원 Indication → 오류 응답 */
                ptPacket->tHeader.ulState = ERR_HIL_NO_APPLICATION_REGISTERED;
                ptPacket->tHeader.ulCmd |= 0x1;  // Confirmation으로 변환
                AppDNS_GlobalPacket_Send(ptDnsRsc);
            }
            break;
        }
    }

    return true;
}
```

#### 4.1.2 CIP 객체 정의

**파일**: `App_VAT_CIP_Objects.h`

```c
#ifndef APP_VAT_CIP_OBJECTS_H_
#define APP_VAT_CIP_OBJECTS_H_

/******************************************************************************
 * VAT CIP CLASS DEFINITIONS
 ******************************************************************************/

/* Standard CIP Classes */
#define CIP_CLASS_IDENTITY          0x01
#define CIP_CLASS_MESSAGE_ROUTER    0x02
#define CIP_CLASS_DEVICENET         0x03
#define CIP_CLASS_ASSEMBLY          0x04
#define CIP_CLASS_CONNECTION        0x05

/* VAT Custom Classes */
#define VAT_CLASS_PARAMETER         0x64  /* 100 - Parameter Object */
#define VAT_CLASS_DIAGNOSTIC        0x65  /* 101 - Diagnostic Object */
#define VAT_CLASS_CALIBRATION       0x66  /* 102 - Calibration Object */
#define VAT_CLASS_VALVE_CONTROL     0x67  /* 103 - Valve Control Object */

/* VAT Parameter Attributes */
#define VAT_PARAM_ATTR_PRESSURE_SETPOINT    1
#define VAT_PARAM_ATTR_POSITION_SETPOINT    2
#define VAT_PARAM_ATTR_CONTROLLER_MODE      3
#define VAT_PARAM_ATTR_CONTROL_INSTANCE     4
#define VAT_PARAM_ATTR_DEVICE_STATUS        5
#define VAT_PARAM_ATTR_EXCEPTION_STATUS     6
#define VAT_PARAM_ATTR_ACCESS_MODE          7
#define VAT_PARAM_ATTR_CURRENT_PRESSURE     8
#define VAT_PARAM_ATTR_PRESSURE_UNITS       9
#define VAT_PARAM_ATTR_POSITION_UNITS       10
#define VAT_PARAM_ATTR_CURRENT_POSITION     11
#define VAT_PARAM_ATTR_AUTO_LEARN           12
#define VAT_PARAM_ATTR_CALIB_SCALE          13
#define VAT_PARAM_ATTR_ZERO_ADJUST          14
#define VAT_PARAM_ATTR_VALVE_ADDRESS        15
#define VAT_PARAM_ATTR_VALVE_ACTION         16

/* VAT Diagnostic Attributes */
#define VAT_DIAG_ATTR_UPTIME                1
#define VAT_DIAG_ATTR_TOTAL_CYCLES          2
#define VAT_DIAG_ATTR_ERROR_COUNT           3
#define VAT_DIAG_ATTR_LAST_ERROR_CODE       4
#define VAT_DIAG_ATTR_LAST_ERROR_TIMESTAMP  5
#define VAT_DIAG_ATTR_PRESSURE_MIN          6
#define VAT_DIAG_ATTR_PRESSURE_MAX          7
#define VAT_DIAG_ATTR_PRESSURE_AVG          8
#define VAT_DIAG_ATTR_POSITION_MIN          9
#define VAT_DIAG_ATTR_POSITION_MAX          10
#define VAT_DIAG_ATTR_NETWORK_RX_COUNT      11
#define VAT_DIAG_ATTR_NETWORK_TX_COUNT      12
#define VAT_DIAG_ATTR_NETWORK_ERROR_COUNT   13
#define VAT_DIAG_ATTR_TEMPERATURE           14
#define VAT_DIAG_ATTR_FIRMWARE_VERSION      15

#endif /* APP_VAT_CIP_OBJECTS_H_ */
```

#### 4.1.3 Explicit Handler 프레임워크

**파일**: `App_VAT_ExplicitHandler.c`

```c
#include "App_VAT_ExplicitHandler.h"
#include "App_VAT_Parameters.h"
#include "App_VAT_Diagnostic.h"
#include "App_VAT_CIP_Objects.h"
#include <string.h>

/******************************************************************************
 * CIP SERVICE HANDLER - MAIN ENTRY POINT
 ******************************************************************************/
bool VAT_Explicit_HandleCipService(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    DNS_PACKET_CIP_SERVICE_IND_T*  ptInd)
{
    DNS_PACKET_CIP_SERVICE_RES_T* ptRes =
        (DNS_PACKET_CIP_SERVICE_RES_T*)ptInd;

    uint32_t ulResDataSize = 0;
    uint32_t ulGRC = CIP_GRC_SUCCESS;
    uint32_t ulERC = 0;

    /* CIP Path 추출 */
    uint8_t bClass = ptInd->tData.ulClass;
    uint8_t bInstance = ptInd->tData.ulInstance;
    uint8_t bAttribute = ptInd->tData.ulAttribute;
    uint8_t bService = ptInd->tData.ulService;

    printf("CIP Service: 0x%02X, Class: 0x%02X, Inst: %u, Attr: %u\r\n",
           bService, bClass, bInstance, bAttribute);

    /* Class별 라우팅 */
    switch(bClass) {
        case VAT_CLASS_PARAMETER:
            ulGRC = VAT_Parameter_HandleService(ptDnsRsc, ptInd, ptRes, &ulResDataSize);
            break;

        case VAT_CLASS_DIAGNOSTIC:
            ulGRC = VAT_Diagnostic_HandleService(ptDnsRsc, ptInd, ptRes, &ulResDataSize);
            break;

        case VAT_CLASS_CALIBRATION:
            ulGRC = VAT_Calibration_HandleService(ptDnsRsc, ptInd, ptRes, &ulResDataSize);
            break;

        case VAT_CLASS_VALVE_CONTROL:
            ulGRC = VAT_ValveControl_HandleService(ptDnsRsc, ptInd, ptRes, &ulResDataSize);
            break;

        case CIP_CLASS_ASSEMBLY:
            ulGRC = VAT_Assembly_HandleService(ptDnsRsc, ptInd, ptRes, &ulResDataSize);
            break;

        default:
            ulGRC = CIP_GRC_OBJECT_DOES_NOT_EXIST;
            break;
    }

    /* 응답 패킷 구성 */
    ptRes->tHead.ulCmd = DNS_CMD_CIP_SERVICE_RES;
    ptRes->tHead.ulLen = DNS_CIP_SERVICE_RES_SIZE + ulResDataSize;
    ptRes->tHead.ulState = CIFX_NO_ERROR;
    ptRes->tData.ulGRC = ulGRC;
    ptRes->tData.ulERC = ulERC;

    /* 응답 전송 */
    int32_t lRet = AppDNS_GlobalPacket_Send(ptDnsRsc);
    if(lRet != CIFX_NO_ERROR) {
        printf("ERROR: Failed to send CIP response: 0x%08X\r\n", (unsigned int)lRet);
        return false;
    }

    return true;
}
```

---

### Phase 2: VAT Parameter Object 구현 (3-4일)

#### 4.2.1 Parameter Handler

**파일**: `App_VAT_ExplicitHandler.c` (추가)

```c
/******************************************************************************
 * VAT PARAMETER OBJECT HANDLER (Class 0x64)
 ******************************************************************************/
uint32_t VAT_Parameter_HandleService(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    DNS_PACKET_CIP_SERVICE_IND_T*  ptInd,
    DNS_PACKET_CIP_SERVICE_RES_T*  ptRes,
    uint32_t*                      pulResDataSize)
{
    uint32_t ulGRC = CIP_GRC_SUCCESS;
    uint8_t bService = ptInd->tData.ulService;
    uint8_t bInstance = ptInd->tData.ulInstance;
    uint8_t bAttribute = ptInd->tData.ulAttribute;

    /* Instance 범위 검증 (1-99) */
    if(bInstance == 0 || bInstance > VAT_PARAM_COUNT_MAX) {
        return CIP_GRC_OBJECT_DOES_NOT_EXIST;
    }

    /* Service 분기 */
    switch(bService) {
        case CIP_SERVICE_GET_ATTRIBUTE_SINGLE:
        {
            /* 단일 속성 읽기 */
            uint8_t bSize = 0;
            uint8_t abData[32] = {0};

            int32_t lRet = VAT_Param_GetByPath(
                &g_tParamManager,
                VAT_CLASS_PARAMETER,
                bInstance,
                bAttribute,
                abData,
                &bSize);

            if(lRet == 0) {
                /* 성공 → 데이터 복사 */
                memcpy(ptRes->tData.abData, abData, bSize);
                *pulResDataSize = bSize;
                ulGRC = CIP_GRC_SUCCESS;

                printf("GET Param%u.Attr%u: ", bInstance, bAttribute);
                for(uint8_t i = 0; i < bSize; i++) {
                    printf("%02X ", abData[i]);
                }
                printf("\r\n");
            } else {
                ulGRC = CIP_GRC_ATTRIBUTE_NOT_SUPPORTED;
            }
            break;
        }

        case CIP_SERVICE_SET_ATTRIBUTE_SINGLE:
        {
            /* 단일 속성 쓰기 */
            uint32_t ulDataLength = ptInd->tHead.ulLen - DNS_CIP_SERVICE_IND_SIZE;

            if(ulDataLength == 0) {
                /* 데이터 없음 → 속성 리셋 */
                ulGRC = CIP_GRC_SUCCESS;
            } else {
                /* 데이터 쓰기 */
                int32_t lRet = VAT_Param_SetByPath(
                    &g_tParamManager,
                    VAT_CLASS_PARAMETER,
                    bInstance,
                    bAttribute,
                    ptInd->tData.abData,
                    (uint8_t)ulDataLength);

                if(lRet == 0) {
                    ulGRC = CIP_GRC_SUCCESS;

                    printf("SET Param%u.Attr%u: ", bInstance, bAttribute);
                    for(uint32_t i = 0; i < ulDataLength; i++) {
                        printf("%02X ", ptInd->tData.abData[i]);
                    }
                    printf("\r\n");
                } else if(lRet == -1) {
                    ulGRC = CIP_GRC_ATTRIBUTE_NOT_SUPPORTED;
                } else if(lRet == -2) {
                    ulGRC = CIP_GRC_TOO_MUCH_DATA;
                } else if(lRet == -3) {
                    ulGRC = CIP_GRC_INVALID_ATTRIBUTE_VALUE;
                } else {
                    ulGRC = CIP_GRC_NOT_ENOUGH_DATA;
                }
            }
            break;
        }

        case CIP_SERVICE_GET_ATTRIBUTES_ALL:
        {
            /* 전체 속성 읽기 (Param 전체 데이터) */
            ulGRC = VAT_Parameter_GetAll(bInstance, ptRes, pulResDataSize);
            break;
        }

        case CIP_SERVICE_RESET:
        {
            /* 파라미터 리셋 (기본값 복원) */
            ulGRC = VAT_Parameter_Reset(bInstance);
            break;
        }

        default:
            ulGRC = CIP_GRC_SERVICE_NOT_SUPPORTED;
            break;
    }

    return ulGRC;
}
```

#### 4.2.2 Parameter Manager 확장

**파일**: `App_VAT_Parameters.c` (수정)

```c
/******************************************************************************
 * PARAMETER ACCESS BY CIP PATH (기존 함수 활용)
 ******************************************************************************/

/* GET_ATTRIBUTES_ALL 구현 */
uint32_t VAT_Parameter_GetAll(
    uint8_t bInstance,
    DNS_PACKET_CIP_SERVICE_RES_T* ptRes,
    uint32_t* pulResDataSize)
{
    if(bInstance == 0 || bInstance > VAT_PARAM_COUNT_MAX) {
        return CIP_GRC_OBJECT_DOES_NOT_EXIST;
    }

    VAT_PARAMETER_T* ptParam = &g_tParamManager.params[bInstance - 1];

    /* 전체 파라미터 데이터 복사 */
    uint8_t* pbDst = ptRes->tData.abData;

    /* Attribute 1: Data */
    memcpy(pbDst, ptParam->data, ptParam->data_size);
    pbDst += ptParam->data_size;

    /* Attribute 2: Min Value */
    memcpy(pbDst, &ptParam->min_value, sizeof(int32_t));
    pbDst += sizeof(int32_t);

    /* Attribute 3: Max Value */
    memcpy(pbDst, &ptParam->max_value, sizeof(int32_t));
    pbDst += sizeof(int32_t);

    /* Attribute 4: Default Value */
    memcpy(pbDst, &ptParam->default_value, sizeof(int32_t));
    pbDst += sizeof(int32_t);

    /* Attribute 5: Data Type */
    *pbDst++ = ptParam->data_type;

    /* Attribute 6: Descriptor */
    memcpy(pbDst, &ptParam->descriptor, sizeof(uint16_t));
    pbDst += sizeof(uint16_t);

    *pulResDataSize = pbDst - ptRes->tData.abData;

    return CIP_GRC_SUCCESS;
}

/* RESET 구현 */
uint32_t VAT_Parameter_Reset(uint8_t bInstance)
{
    if(bInstance == 0 || bInstance > VAT_PARAM_COUNT_MAX) {
        return CIP_GRC_OBJECT_DOES_NOT_EXIST;
    }

    VAT_PARAMETER_T* ptParam = &g_tParamManager.params[bInstance - 1];

    /* 기본값으로 복원 */
    memcpy(ptParam->data, &ptParam->default_value, ptParam->data_size);

    /* Modified 플래그 설정 */
    VAT_Param_SetModified(&g_tParamManager, bInstance);

    printf("Reset Param%u to default: %ld\r\n", bInstance, ptParam->default_value);

    return CIP_GRC_SUCCESS;
}
```

#### 4.2.3 실시간 동기화

**목표**: Explicit Message로 변경된 파라미터를 I/O Data에 반영

```c
/******************************************************************************
 * PARAMETER → I/O DATA SYNCHRONIZATION
 ******************************************************************************/
void VAT_Sync_ParametersToIoData(void)
{
    /* Param1 (Pressure Setpoint) → Output Assembly */
    int16_t sPressureSetpoint = 0;
    uint8_t bSize = 0;

    VAT_Param_Get(&g_tParamManager, 1, &sPressureSetpoint, &bSize);
    g_tAppData.tInputData.output_asm8.control_setpoint = sPressureSetpoint;

    /* Param3 (Controller Mode) → Output Assembly */
    uint8_t bControlMode = 0;
    VAT_Param_Get(&g_tParamManager, 3, &bControlMode, &bSize);
    g_tAppData.tInputData.output_asm8.control_mode = bControlMode;

    /* I/O Data → Parameters (Read-Only) */
    VAT_Param_Set(&g_tParamManager, 8,
        &g_tAppData.tOutputData.input_asm100.pressure, 2);

    VAT_Param_Set(&g_tParamManager, 11,
        &g_tAppData.tOutputData.input_asm100.position, 2);

    VAT_Param_Set(&g_tParamManager, 5,
        &g_tAppData.tOutputData.input_asm100.device_status, 1);
}

/* 주기적 호출 (100ms마다) */
void VAT_CyclicTask(void)
{
    VAT_Sync_ParametersToIoData();
    VAT_Diagnostic_Update();
}
```

---

### Phase 3: VAT Diagnostic Object 구현 (2-3일)

#### 4.3.1 Diagnostic Data 구조

**파일**: `App_VAT_Diagnostic.h`

```c
#ifndef APP_VAT_DIAGNOSTIC_H_
#define APP_VAT_DIAGNOSTIC_H_

#include <stdint.h>

/* Diagnostic Data Structure */
typedef struct VAT_DIAGNOSTIC_DATA_Ttag {
    /* System Statistics */
    uint32_t ulUptimeSeconds;
    uint32_t ulTotalCycles;
    uint16_t usErrorCount;
    uint16_t usLastErrorCode;
    uint32_t ulLastErrorTimestamp;

    /* Pressure Statistics */
    int16_t sPressureMin;
    int16_t sPressureMax;
    int16_t sPressureAvg;
    int32_t lPressureSum;
    uint16_t usPressureSampleCount;

    /* Position Statistics */
    int16_t sPositionMin;
    int16_t sPositionMax;

    /* Network Statistics */
    uint32_t ulNetworkRxCount;
    uint32_t ulNetworkTxCount;
    uint16_t usNetworkErrorCount;

    /* System Status */
    int16_t sTemperature;
    uint32_t ulFirmwareVersion;
} VAT_DIAGNOSTIC_DATA_T;

/* Global Diagnostics */
extern VAT_DIAGNOSTIC_DATA_T g_tVatDiagnostics;

/* Function Prototypes */
void VAT_Diagnostic_Init(void);
void VAT_Diagnostic_Update(void);
void VAT_Diagnostic_RecordError(uint16_t usErrorCode);
void VAT_Diagnostic_Reset(void);

uint32_t VAT_Diagnostic_HandleService(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    DNS_PACKET_CIP_SERVICE_IND_T*  ptInd,
    DNS_PACKET_CIP_SERVICE_RES_T*  ptRes,
    uint32_t*                      pulResDataSize);

#endif /* APP_VAT_DIAGNOSTIC_H_ */
```

#### 4.3.2 Diagnostic Handler

**파일**: `App_VAT_Diagnostic.c`

```c
#include "App_VAT_Diagnostic.h"
#include "App_VAT_CIP_Objects.h"
#include <string.h>

VAT_DIAGNOSTIC_DATA_T g_tVatDiagnostics = {0};
static uint32_t s_ulStartTick = 0;

/******************************************************************************
 * INITIALIZATION
 ******************************************************************************/
void VAT_Diagnostic_Init(void)
{
    memset(&g_tVatDiagnostics, 0, sizeof(VAT_DIAGNOSTIC_DATA_T));

    /* 초기값 설정 */
    g_tVatDiagnostics.sPressureMin = INT16_MAX;
    g_tVatDiagnostics.sPressureMax = INT16_MIN;
    g_tVatDiagnostics.sPositionMin = INT16_MAX;
    g_tVatDiagnostics.sPositionMax = INT16_MIN;

    g_tVatDiagnostics.ulFirmwareVersion = 0x01000000;  // v1.0.0

    s_ulStartTick = HAL_GetTick();
}

/******************************************************************************
 * UPDATE (주기적 호출 - 100ms)
 ******************************************************************************/
void VAT_Diagnostic_Update(void)
{
    /* Uptime 갱신 */
    g_tVatDiagnostics.ulUptimeSeconds = (HAL_GetTick() - s_ulStartTick) / 1000;

    /* Total Cycles */
    g_tVatDiagnostics.ulTotalCycles++;

    /* 압력 통계 (최근 100 샘플 평균) */
    int16_t sPressure = g_tAppData.tOutputData.input_asm100.pressure;

    if(sPressure < g_tVatDiagnostics.sPressureMin) {
        g_tVatDiagnostics.sPressureMin = sPressure;
    }
    if(sPressure > g_tVatDiagnostics.sPressureMax) {
        g_tVatDiagnostics.sPressureMax = sPressure;
    }

    g_tVatDiagnostics.lPressureSum += sPressure;
    g_tVatDiagnostics.usPressureSampleCount++;

    if(g_tVatDiagnostics.usPressureSampleCount >= 100) {
        g_tVatDiagnostics.sPressureAvg =
            (int16_t)(g_tVatDiagnostics.lPressureSum / 100);
        g_tVatDiagnostics.lPressureSum = 0;
        g_tVatDiagnostics.usPressureSampleCount = 0;
    }

    /* 위치 통계 */
    int16_t sPosition = g_tAppData.tOutputData.input_asm100.position;

    if(sPosition < g_tVatDiagnostics.sPositionMin) {
        g_tVatDiagnostics.sPositionMin = sPosition;
    }
    if(sPosition > g_tVatDiagnostics.sPositionMax) {
        g_tVatDiagnostics.sPositionMax = sPosition;
    }

    /* 온도 (시뮬레이션) */
    g_tVatDiagnostics.sTemperature = 25;  // 25°C
}

/******************************************************************************
 * CIP SERVICE HANDLER
 ******************************************************************************/
uint32_t VAT_Diagnostic_HandleService(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    DNS_PACKET_CIP_SERVICE_IND_T*  ptInd,
    DNS_PACKET_CIP_SERVICE_RES_T*  ptRes,
    uint32_t*                      pulResDataSize)
{
    uint32_t ulGRC = CIP_GRC_SUCCESS;
    uint8_t bService = ptInd->tData.ulService;
    uint8_t bInstance = ptInd->tData.ulInstance;
    uint8_t bAttribute = ptInd->tData.ulAttribute;

    /* Instance 1만 지원 */
    if(bInstance != 1) {
        return CIP_GRC_OBJECT_DOES_NOT_EXIST;
    }

    switch(bService) {
        case CIP_SERVICE_GET_ATTRIBUTE_SINGLE:
        {
            /* 단일 속성 읽기 */
            uint8_t* pbSrc = NULL;
            uint8_t bSize = 0;

            switch(bAttribute) {
                case VAT_DIAG_ATTR_UPTIME:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.ulUptimeSeconds;
                    bSize = 4;
                    break;

                case VAT_DIAG_ATTR_TOTAL_CYCLES:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.ulTotalCycles;
                    bSize = 4;
                    break;

                case VAT_DIAG_ATTR_ERROR_COUNT:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.usErrorCount;
                    bSize = 2;
                    break;

                case VAT_DIAG_ATTR_LAST_ERROR_CODE:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.usLastErrorCode;
                    bSize = 2;
                    break;

                case VAT_DIAG_ATTR_PRESSURE_MIN:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.sPressureMin;
                    bSize = 2;
                    break;

                case VAT_DIAG_ATTR_PRESSURE_MAX:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.sPressureMax;
                    bSize = 2;
                    break;

                case VAT_DIAG_ATTR_PRESSURE_AVG:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.sPressureAvg;
                    bSize = 2;
                    break;

                case VAT_DIAG_ATTR_TEMPERATURE:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.sTemperature;
                    bSize = 2;
                    break;

                case VAT_DIAG_ATTR_FIRMWARE_VERSION:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.ulFirmwareVersion;
                    bSize = 4;
                    break;

                default:
                    ulGRC = CIP_GRC_ATTRIBUTE_NOT_SUPPORTED;
                    break;
            }

            if(pbSrc != NULL) {
                memcpy(ptRes->tData.abData, pbSrc, bSize);
                *pulResDataSize = bSize;
            }
            break;
        }

        case CIP_SERVICE_GET_ATTRIBUTES_ALL:
        {
            /* 전체 진단 데이터 읽기 */
            uint8_t* pbDst = ptRes->tData.abData;

            memcpy(pbDst, &g_tVatDiagnostics.ulUptimeSeconds, 4);
            pbDst += 4;

            memcpy(pbDst, &g_tVatDiagnostics.ulTotalCycles, 4);
            pbDst += 4;

            memcpy(pbDst, &g_tVatDiagnostics.usErrorCount, 2);
            pbDst += 2;

            memcpy(pbDst, &g_tVatDiagnostics.sPressureMin, 2);
            pbDst += 2;

            memcpy(pbDst, &g_tVatDiagnostics.sPressureMax, 2);
            pbDst += 2;

            memcpy(pbDst, &g_tVatDiagnostics.sPressureAvg, 2);
            pbDst += 2;

            memcpy(pbDst, &g_tVatDiagnostics.sTemperature, 2);
            pbDst += 2;

            memcpy(pbDst, &g_tVatDiagnostics.ulFirmwareVersion, 4);
            pbDst += 4;

            *pulResDataSize = pbDst - ptRes->tData.abData;
            break;
        }

        case CIP_SERVICE_RESET:
        {
            /* 통계 리셋 */
            VAT_Diagnostic_Reset();
            break;
        }

        default:
            ulGRC = CIP_GRC_SERVICE_NOT_SUPPORTED;
            break;
    }

    return ulGRC;
}

/******************************************************************************
 * ERROR RECORDING
 ******************************************************************************/
void VAT_Diagnostic_RecordError(uint16_t usErrorCode)
{
    g_tVatDiagnostics.usErrorCount++;
    g_tVatDiagnostics.usLastErrorCode = usErrorCode;
    g_tVatDiagnostics.ulLastErrorTimestamp = HAL_GetTick() / 1000;

    printf("ERROR: Code 0x%04X recorded (Total: %u)\r\n",
           usErrorCode, g_tVatDiagnostics.usErrorCount);
}

/******************************************************************************
 * RESET
 ******************************************************************************/
void VAT_Diagnostic_Reset(void)
{
    /* 통계만 리셋, 펌웨어 버전은 유지 */
    uint32_t ulFwVer = g_tVatDiagnostics.ulFirmwareVersion;

    VAT_Diagnostic_Init();

    g_tVatDiagnostics.ulFirmwareVersion = ulFwVer;

    printf("Diagnostic statistics reset\r\n");
}
```

---

### Phase 4: 고급 서비스 구현 (3-4일)

#### 4.4.1 Calibration Object (Class 0x66)

**목적**: 압력/위치 센서 교정, 자동 학습

**Attributes**:
- Attr 1: Auto Learn Enable (USINT)
- Attr 2: Calibration Scale (USINT)
- Attr 3: Zero Adjust (USINT, Write-Only)
- Attr 4: Pressure Calibration Status (USINT, Read-Only)
- Attr 5: Position Calibration Status (USINT, Read-Only)

**Custom Services**:
- Service 0x32: Start Auto-Learn
- Service 0x33: Stop Auto-Learn
- Service 0x34: Save Calibration

#### 4.4.2 Valve Control Object (Class 0x67)

**목적**: 개별 밸브 제어, 진단

**Attributes**:
- Attr 1: Valve Address (USINT)
- Attr 2: Valve Action (USINT)
- Attr 3: Valve Status (USINT, Read-Only)

**Custom Services**:
- Service 0x40: Open Valve
- Service 0x41: Close Valve
- Service 0x42: Test Valve

#### 4.4.3 Assembly Object 확장

**목적**: 동적 Assembly 전환

**Custom Services**:
- Service 0x50: Switch Input Assembly
- Service 0x51: Switch Output Assembly
- Service 0x52: Query Available Assemblies

---

### Phase 5: 테스트 및 검증 (4-5일)

#### 4.5.1 단위 테스트

**테스트 케이스**:

```c
/* Test 1: GET Parameter */
void Test_GetParameter_Success(void)
{
    // Master → Device: GET Param1 (Pressure Setpoint)
    DNS_PACKET_CIP_SERVICE_IND_T tInd = {0};
    tInd.tData.ulService = CIP_SERVICE_GET_ATTRIBUTE_SINGLE;
    tInd.tData.ulClass = VAT_CLASS_PARAMETER;
    tInd.tData.ulInstance = 1;
    tInd.tData.ulAttribute = 1;

    // Expected: INT 500 (0xF4 0x01)
    assert(response == 0x00F4);
}

/* Test 2: SET Parameter */
void Test_SetParameter_Success(void)
{
    // Master → Device: SET Param1 to 750
    DNS_PACKET_CIP_SERVICE_IND_T tInd = {0};
    tInd.tHead.ulLen = DNS_CIP_SERVICE_IND_SIZE + 2;
    tInd.tData.ulService = CIP_SERVICE_SET_ATTRIBUTE_SINGLE;
    tInd.tData.ulClass = VAT_CLASS_PARAMETER;
    tInd.tData.ulInstance = 1;
    tInd.tData.ulAttribute = 1;
    tInd.tData.abData[0] = 0xEE;
    tInd.tData.abData[1] = 0x02;  // 750

    // Verify: Parameter Manager updated
    int16_t sValue = 0;
    VAT_Param_Get(&g_tParamManager, 1, &sValue, NULL);
    assert(sValue == 750);
}

/* Test 3: GET Diagnostic */
void Test_GetDiagnostic_Uptime(void)
{
    // Master → Device: GET Diagnostic Uptime
    tInd.tData.ulClass = VAT_CLASS_DIAGNOSTIC;
    tInd.tData.ulInstance = 1;
    tInd.tData.ulAttribute = VAT_DIAG_ATTR_UPTIME;

    // Expected: UDINT (4 bytes)
    assert(response_size == 4);
}
```

#### 4.5.2 통합 테스트

**시나리오 1: 원격 압력 설정**
```
1. Master → GET Param1 (현재 Pressure Setpoint 조회)
2. Master → SET Param1 to 800 (새로운 Setpoint 설정)
3. Device: Parameter Manager 업데이트
4. Device: I/O Data 동기화 (Output Assembly)
5. Device → Master: I/O Data 전송 (압력 제어 시작)
6. Master → GET Param8 (현재 압력 측정값 조회)
7. Verify: 압력이 800으로 수렴
```

**시나리오 2: 진단 정보 조회**
```
1. Master → GET Diagnostic.Attr1 (Uptime)
2. Master → GET Diagnostic.Attr3 (Error Count)
3. Master → GET Diagnostic.Attr6-8 (Pressure Min/Max/Avg)
4. Master → GET_ATTRIBUTES_ALL (전체 진단 정보)
5. Verify: 모든 데이터 일관성
```

#### 4.5.3 성능 테스트

**측정 항목**:
- Explicit Message 응답 시간: < 10ms
- Parameter 변경 → I/O 반영 시간: < 100ms
- 초당 처리 가능한 Explicit Message 수: > 50 msgs/sec

#### 4.5.4 스트레스 테스트

**시나리오**:
- 100개 파라미터 연속 읽기/쓰기
- I/O Data + Explicit Message 동시 처리
- 24시간 장시간 안정성 테스트

---

## 5. EDS 파일 업데이트

### 5.1 EDS 구조

```ini
[File]
DescText = "VAT Adaptive Pressure Controller with Explicit Messaging"
CreateDate = 11-07-2025
CreateTime = 17:10:00
ModDate = 11-07-2025
ModTime = 17:10:00
Revision = 2.0

[Device]
VendID = 0x????
DevType = 0x0C
ProdCode = 1
Revision = 2.0
ProdName = "VAT DeviceNet Controller"
Catalog = "VAT-DN-100"

[Device Classification]
Class1 = Yes

[Params]
Param1 =
0x0000,               ; Descriptor
0xC3,                 ; Data type = INT
0x01,                 ; Data size = 2 bytes
"Pressure Setpoint",
"mTorr",
"Target pressure control setpoint",
0x0000, 0x03E8,       ; Min = 0, Max = 1000
0x01F4;               ; Default = 500

Param2 =
0x0000,
0xC3,
0x01,
"Position Setpoint",
"Counts",
"Target position control setpoint",
0x0000, 0x03E8,
0x01F4;

Param3 =
0x0000,
0xC6,
0x00,
"Controller Mode",
"",
"Control mode: 0=Init, 1=Sync, 2=Position, 5=Pressure",
0x00, 0x0E,
0x05;

; ... (Total 99 parameters)

[Assembly]
Object_Name = "Assembly"
Object_Class_Code = 0x04

; Input Assembly 100 (7 bytes)
Assem100 =
"Input Assembly 100",
7,                    ; Size
0x0001,               ; Descriptor
"Exception Status",1,USINT,
"Pressure",2,INT,
"Position",2,INT,
"Device Status",1,USINT,
"Access Mode",1,USINT;

; Output Assembly 8 (5 bytes)
Assem8 =
"Output Assembly 8",
5,
0x0001,
"Control Mode",1,USINT,
"Control Setpoint",2,INT,
"Control Instance",1,USINT,
"Reserved",1,USINT;

[Connection Manager]
Connection1 =
0x04, 0x44, 0x64, 0x65,   ; Trigger & Transport
20, 100,                   ; RPI = 100ms
0x0064,                    ; Input Assembly 100
0x0008,                    ; Output Assembly 8
0x0000;                    ; Config Assembly (none)

[Message Router]
Object_Name = "Message Router"
Object_Class_Code = 0x02

[VAT Parameter Object]
Object_Name = "VAT Parameter Object"
Object_Class_Code = 0x64
Max_Instance = 99
Object_Path = "20 64 24 01"

[VAT Diagnostic Object]
Object_Name = "VAT Diagnostic Object"
Object_Class_Code = 0x65
Max_Instance = 1
Object_Path = "20 65 24 01"
```

---

## 6. 사용 예시

### 6.1 RSLogix 5000에서 파라미터 설정

**MSG Instruction 설정**:
```
Message Type: CIP Generic
Service Type: Set_Attribute_Single (0x10)
Class: 0x64 (VAT Parameter)
Instance: 1 (Param1)
Attribute: 1
Source Element: Pressure_Setpoint_Tag (INT)
Destination: VAT_Device_Node_10
```

### 6.2 Python 스크립트 (테스트용)

```python
import devicenet

# DeviceNet 연결
dn = devicenet.DeviceNet('/dev/can0', baudrate=125000)
node = dn.node(10)  # VAT Device Node 10

# Param1 (Pressure Setpoint) 읽기
value = node.get_attribute(class_id=0x64, instance=1, attribute=1)
print(f"Current Pressure Setpoint: {value} mTorr")

# Param1 설정
node.set_attribute(class_id=0x64, instance=1, attribute=1, value=750)
print("Pressure Setpoint set to 750 mTorr")

# 진단 정보 읽기
uptime = node.get_attribute(class_id=0x65, instance=1, attribute=1)
print(f"Device Uptime: {uptime} seconds")

error_count = node.get_attribute(class_id=0x65, instance=1, attribute=3)
print(f"Total Errors: {error_count}")
```

---

## 7. 체크리스트

### Phase 1: 기본 인프라 (2-3일)
- [ ] `DNS_HOST_APP_REGISTRATION` 매크로 활성화
- [ ] `AppDNS_PacketHandler_callback` 수정
- [ ] `App_VAT_ExplicitHandler.c` 생성
- [ ] `App_VAT_CIP_Objects.h` 정의
- [ ] CIP 서비스 라우팅 구현
- [ ] 기본 오류 처리

### Phase 2: Parameter Object (3-4일)
- [ ] `VAT_Parameter_HandleService` 구현
- [ ] GET_ATTRIBUTE_SINGLE 처리
- [ ] SET_ATTRIBUTE_SINGLE 처리
- [ ] GET_ATTRIBUTES_ALL 구현
- [ ] RESET 서비스 구현
- [ ] Parameter ↔ I/O Data 동기화
- [ ] 단위 테스트 (10개)

### Phase 3: Diagnostic Object (2-3일)
- [ ] `App_VAT_Diagnostic.c` 생성
- [ ] 진단 데이터 구조 정의
- [ ] `VAT_Diagnostic_Update` 구현 (주기적 갱신)
- [ ] `VAT_Diagnostic_HandleService` 구현
- [ ] GET 서비스 처리 (15개 속성)
- [ ] GET_ATTRIBUTES_ALL 구현
- [ ] 오류 기록 메커니즘
- [ ] 단위 테스트 (5개)

### Phase 4: 고급 서비스 (3-4일)
- [ ] Calibration Object 구현
- [ ] Valve Control Object 구현
- [ ] Assembly 동적 전환
- [ ] 커스텀 서비스 (6개)
- [ ] 통합 테스트 (5개 시나리오)

### Phase 5: 테스트 및 검증 (4-5일)
- [ ] 단위 테스트 완료 (30개)
- [ ] 통합 테스트 완료 (10개 시나리오)
- [ ] 성능 테스트 (응답 시간, 처리량)
- [ ] 스트레스 테스트 (24시간)
- [ ] EDS 파일 업데이트 및 검증
- [ ] 사용자 문서 작성

---

## 8. 예상 일정

**총 소요 기간**: 14-19일 (약 3-4주)

| Phase | 작업 | 일정 | 누적 |
|-------|------|------|------|
| **Phase 1** | 기본 인프라 구축 | 2-3일 | 2-3일 |
| **Phase 2** | Parameter Object | 3-4일 | 5-7일 |
| **Phase 3** | Diagnostic Object | 2-3일 | 7-10일 |
| **Phase 4** | 고급 서비스 | 3-4일 | 10-14일 |
| **Phase 5** | 테스트 및 검증 | 4-5일 | 14-19일 |

---

## 9. 리스크 및 대응 방안

### 9.1 기술적 리스크

**리스크 1**: Explicit Message와 I/O Data 동시 처리 시 충돌
- **대응**: 뮤텍스 또는 세마포어 사용, 우선순위 설정

**리스크 2**: 파라미터 동기화 지연
- **대응**: 변경 플래그 기반 효율적 동기화

**리스크 3**: EDS 파일 호환성 문제
- **대응**: EDS Checker 도구로 검증, 표준 준수

### 9.2 성능 리스크

**리스크 1**: 응답 시간 초과 (> 10ms)
- **대응**: 코드 최적화, 비동기 처리

**리스크 2**: 메모리 부족
- **대응**: 동적 할당 최소화, 메모리 풀 사용

---

## 10. 참고 문서

### 10.1 내부 문서
- `20251107_1510_AppDNS_PacketHandler_Analysis.md`
- `20251107_1530_AppDNS_DemoApplication_UserObject_Analysis.md`
- `20251107_1635_ExplicitMessage_Implementation_Plan.md`
- VAT EDS 문서 (사용자 제공)

### 10.2 외부 표준
- **ODVA DeviceNet Specification Volume I**
- **CIP Common Specification Volume 1**
- **CIP Volume 8: CIP Safety**
- **Hilscher DeviceNet Protocol API Manual**

---

## 11. 결론

본 계획서는 VAT Adaptive Pressure Controller의 DeviceNet Explicit Message 통신을 완전히 구현하기 위한 상세 로드맵입니다.

**핵심 달성 목표**:
1. ✅ **99개 파라미터 원격 접근**: CIP GET/SET 서비스
2. ✅ **실시간 진단**: 15개 진단 속성 조회
3. ✅ **동적 Assembly 전환**: 24 Input + 11 Output 지원
4. ✅ **고급 제어**: 교정, 밸브 제어, 커스텀 서비스

**기대 효과**:
- 📈 **설정 편의성**: 네트워크를 통한 원격 설정
- 📊 **진단 강화**: 실시간 상태 모니터링
- 🔧 **유지보수 효율**: 원격 교정 및 진단
- 🚀 **확장성**: 추가 기능 손쉽게 통합

**다음 단계**:
1. Phase 1 착수: 패킷 핸들러 수정
2. Parameter Manager 통합 테스트
3. 마스터 소프트웨어(RSLogix) 연동 검증

---

**작성자**: Claude
**문서 버전**: 1.0
**최종 수정**: 2025-11-07 17:10

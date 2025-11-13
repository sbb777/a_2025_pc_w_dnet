# DeviceNet Master Identity Query via CIP Explicit Messaging

**작성일시**: 2025-10-28
**작업자**: Claude
**목적**: DeviceNet Master 장치의 Identity 정보를 CIP Explicit Messaging을 사용하여 조회하는 기능 구현

---

## 1. 작업 개요

### 1.1 요구사항
- DeviceNet 네트워크상의 Master 장치 정보 조회
- Master의 제품명, 펌웨어 버전, Vendor ID, Serial Number 등 Identity Object 정보 획득
- CIP (Common Industrial Protocol) Explicit Messaging 방식 적용

### 1.2 배경
이전 작업에서 `xChannelInfo()` API를 통해 획득한 정보는 **netX90 Slave 모듈 자체**의 펌웨어 정보였습니다.
사용자가 필요로 하는 것은 **DeviceNet Master (Scanner/PLC)** 장치의 정보이므로, CIP Explicit Messaging을 통해 Master의 Identity Object를 직접 조회하는 방식을 구현하였습니다.

---

## 2. DeviceNet 네트워크 구조

```
┌─────────────────────┐
│  DeviceNet Master   │  ← 조회 대상 (Scanner/PLC)
│   (Scanner/PLC)     │
└──────────┬──────────┘
           │ DeviceNet Network
           │
┌──────────┴──────────┐
│   netX90 Module     │  ← Slave (cifX API로 제어)
│   (Slave Device)    │
└──────────┬──────────┘
           │ SPI Communication
           │
┌──────────┴──────────┐
│   STM32F429         │  ← Host Processor
│   (Host Controller) │
└─────────────────────┘
```

### 통신 방식
1. **I/O Messaging**: 주기적 데이터 교환 (VAT Pressure, Setpoint 등)
2. **Explicit Messaging**: 비주기적 서비스 요청 (Identity 조회, 설정 변경 등)

---

## 3. CIP Identity Object 구조

### 3.1 CIP Object Model
- **Class**: 0x01 (Identity Object Class)
- **Instance**: Master Node ID (일반적으로 0 또는 63)
- **Attributes**:

| Attribute ID | Name | Data Type | Description |
|--------------|------|-----------|-------------|
| 1 | Vendor ID | UINT16 | 제조사 고유 번호 |
| 2 | Device Type | UINT16 | 장치 유형 코드 |
| 3 | Product Code | UINT16 | 제품 코드 |
| 4 | Revision | USINT[2] | Major.Minor 버전 |
| 5 | Status | UINT16 | 장치 상태 |
| 6 | Serial Number | UDINT | 일련번호 |
| 7 | Product Name | SHORT_STRING | 제품명 문자열 |

### 3.2 CIP Service
- **Service Code**: 0x0E (Get_Attribute_Single)
- **Request**: Class + Instance + Attribute ID
- **Response**: Attribute 데이터 반환

---

## 4. 구현 내용

### 4.1 파일 구조

```
Hil_DemoApp/
├── Includes/
│   └── devicenet_master_info.h      (새로 생성)
└── Sources/
    └── devicenet_master_info.c      (새로 생성)

Core/Src/
└── main.c                           (수정)
```

### 4.2 생성된 파일

#### 4.2.1 `devicenet_master_info.h`

**위치**: `D:\git\netx_90_f429_SPI5\Hil_DemoApp\Includes\devicenet_master_info.h`

**주요 내용**:
```c
typedef struct DEVICENET_MASTER_IDENTITY_Ttag
{
    uint16_t usVendorID;              /* Vendor ID */
    uint16_t usDeviceType;            /* Device Type */
    uint16_t usProductCode;           /* Product Code */
    uint8_t  bRevisionMajor;          /* Revision Major */
    uint8_t  bRevisionMinor;          /* Revision Minor */
    uint16_t usStatus;                /* Device Status */
    uint32_t ulSerialNumber;          /* Serial Number */
    uint8_t  bProductNameLength;      /* Product Name Length */
    char     szProductName[64];       /* Product Name String */
    bool     bDataValid;              /* Data validity flag */
} DEVICENET_MASTER_IDENTITY_T;
```

**제공 함수**:
1. `DeviceNet_GetMasterIdentity()` - Master Identity 전체 조회
2. `DeviceNet_GetMasterAttribute()` - 개별 Attribute 조회
3. `DeviceNet_PrintMasterIdentity()` - Identity 정보 출력
4. `DeviceNet_ScanForMaster()` - Master 노드 주소 자동 검색

#### 4.2.2 `devicenet_master_info.c`

**위치**: `D:\git\netx_90_f429_SPI5\Hil_DemoApp\Sources\devicenet_master_info.c`

**핵심 로직**:

```c
int32_t DeviceNet_GetMasterAttribute(CIFXHANDLE hChannel,
                                     uint8_t masterNodeId,
                                     uint8_t attributeId,
                                     void* pData,
                                     uint32_t dataSize,
                                     uint32_t* pReceivedSize)
{
    DNS_PACKET_CIP_SERVICE_REQ_T tSendPkt;
    DNS_PACKET_CIP_SERVICE_CNF_T tRecvPkt;

    /* CIP Service Request 구성 */
    tSendPkt.tData.ulService = CIP_SERVICE_GET_ATTRIBUTE_SINGLE;
    tSendPkt.tData.ulClass = CIP_CLASS_IDENTITY;
    tSendPkt.tData.ulInstance = masterNodeId;
    tSendPkt.tData.ulAttribute = attributeId;

    /* 패킷 송신 */
    lRet = xChannelPutPacket(hChannel, &tSendPkt, PACKET_TIMEOUT_MS);

    /* 응답 수신 */
    lRet = xChannelGetPacket(hChannel, &tRecvPkt, PACKET_TIMEOUT_MS);

    /* 데이터 복사 */
    memcpy(pData, tRecvPkt.tData.abData, ulDataLen);

    return CIFX_NO_ERROR;
}
```

**특징**:
- 7개의 Identity Attribute를 순차적으로 조회
- 각 Attribute별 에러 핸들링
- Vendor ID 성공 시 `bDataValid = true` 설정
- Product Name은 SHORT_STRING 형식 (길이 + 문자열) 파싱

### 4.3 수정된 파일

#### 4.3.1 `main.c` 수정 내용

**1) Include 추가** (Line 36):
```c
#include "devicenet_master_info.h"
```

**2) PrintDeviceInformation() 함수 확장** (Line 347-384):
```c
/* 4. DeviceNet Master Identity Information */
DEVICENET_MASTER_IDENTITY_T tMasterIdentity;
uint8_t masterNodeId = 0;

/* Try to scan for master on network */
lRet = DeviceNet_ScanForMaster(hChannel, &masterNodeId);

if(lRet == CIFX_NO_ERROR)
{
    DeviceNet_PrintMasterIdentity(&tMasterIdentity);
}
else
{
    /* Fallback to direct query */
    lRet = DeviceNet_GetMasterIdentity(hChannel, 0, &tMasterIdentity);

    if(lRet == CIFX_NO_ERROR && tMasterIdentity.bDataValid)
    {
        DeviceNet_PrintMasterIdentity(&tMasterIdentity);
    }
    else
    {
        printf("ERROR: Could not retrieve Master identity\r\n");
        /* 에러 가이드 출력 */
    }
}
```

---

## 5. 동작 흐름

### 5.1 Master 검색 및 조회 프로세스

```
시작
  ↓
[1] DeviceNet_ScanForMaster() 호출
  ↓
[2] 일반적인 Master 주소 시도 (0, 63, 1, 62)
  ├─ 성공 → Master Node ID 반환
  └─ 실패 → 전체 스캔 (0-63)
  ↓
[3] DeviceNet_GetMasterIdentity() 호출
  ↓
[4] 7개 Attribute 순차 조회
  ├─ Attribute 1: Vendor ID
  ├─ Attribute 2: Device Type
  ├─ Attribute 3: Product Code
  ├─ Attribute 4: Revision (Major.Minor)
  ├─ Attribute 5: Status
  ├─ Attribute 6: Serial Number
  └─ Attribute 7: Product Name
  ↓
[5] DeviceNet_PrintMasterIdentity() 출력
  ↓
완료
```

### 5.2 CIP 패킷 교환

```
STM32 → netX90 → Master
        ┌──────────────────────────────┐
        │ CIP Get_Attribute_Single Req │
        │ - Service: 0x0E              │
        │ - Class: 0x01 (Identity)     │
        │ - Instance: Master Node ID   │
        │ - Attribute: 1~7             │
        └──────────────────────────────┘
                     ↓
        ┌──────────────────────────────┐
        │ CIP Service Response         │
        │ - Status: Success/Error      │
        │ - Data: Attribute Value      │
        └──────────────────────────────┘
STM32 ← netX90 ← Master
```

---

## 6. 출력 예시

### 6.1 성공 케이스

```
========================================
 DeviceNet Master Identity
========================================

=== Querying DeviceNet Master Identity (Node 0) ===
Vendor ID: 0x0001
Device Type: 0x000C
Product Code: 0x0064
Revision: 1.5
Status: 0x0020
Serial Number: 123456789
Product Name: XYZ DeviceNet Scanner V1.5

╔════════════════════════════════════════════════════╗
║     DeviceNet Master Identity Information         ║
╠════════════════════════════════════════════════════╣
║  Vendor ID:      0x0001                            ║
║  Device Type:    0x000C                            ║
║  Product Code:   0x0064                            ║
║  Revision:       1.5                               ║
║  Status:         0x0020                            ║
║  Serial Number:  123456789                         ║
║  Product Name:   XYZ DeviceNet Scanner V1.5        ║
╚════════════════════════════════════════════════════╝

========================================
```

### 6.2 실패 케이스

```
========================================
 DeviceNet Master Identity
========================================

Scanning failed, trying direct query at Node 0...
Error: Failed to receive CIP service response (0x80000009)
ERROR: Could not retrieve Master identity

Please verify:
  1. DeviceNet Master is powered on
  2. Network cables are connected
  3. Network termination is correct
  4. Master node address configuration

========================================
```

---

## 7. 테스트 방법

### 7.1 필수 조건
1. DeviceNet Master (Scanner/PLC) 전원 ON
2. DeviceNet 네트워크 케이블 연결
3. 네트워크 종단 저항 설정 (124Ω)
4. Master와 netX90 모듈 간 통신 설정 완료

### 7.2 테스트 절차
1. STM32 프로젝트 빌드 및 다운로드
2. UART1 시리얼 터미널 연결 (115200 baud)
3. VAT 테스트 모드 선택 (기존과 동일)
4. `PrintDeviceInformation()` 함수가 자동 호출됨
5. 출력에서 "DeviceNet Master Identity" 섹션 확인

### 7.3 예상 출력 위치
```
[Driver Information]
  ...

[Board Information]
  ...

[Channel Information]
  ...

[Firmware Information]  ← netX90 Slave 모듈 정보
  ...

========================================
 DeviceNet Master Identity  ← Master 장치 정보 (NEW!)
========================================
  ...
```

---

## 8. 에러 처리

### 8.1 가능한 에러 코드

| 에러 코드 | 의미 | 원인 | 해결 방법 |
|-----------|------|------|-----------|
| 0x80000009 | CIFX_DEV_TIMEOUT | 패킷 응답 타임아웃 | Master 전원, 케이블 확인 |
| 0x8000000A | CIFX_DEV_NOT_READY | 장치 준비 안됨 | Master 부팅 대기 |
| 0x8000000B | CIFX_DEV_NOT_FOUND | 장치 없음 | Master 주소 설정 확인 |
| 0x00000001 | CIP Error Response | CIP 서비스 에러 | Master의 CIP 로그 확인 |

### 8.2 디버깅 팁
1. **타임아웃 발생 시**:
   - Master가 부팅 중인지 확인
   - 네트워크 종단 저항 확인
   - Master의 DeviceNet LED 상태 확인

2. **Master를 찾을 수 없는 경우**:
   - Master Node ID 설정 확인 (DIP 스위치 등)
   - 네트워크 설정 도구로 스캔 시도
   - Master의 DeviceNet 통신 활성화 확인

3. **일부 Attribute만 조회되는 경우**:
   - Master가 특정 Attribute를 지원하지 않을 수 있음
   - Vendor ID가 조회되면 기본 성공으로 간주
   - Product Name은 옵션일 수 있음

---

## 9. 참고 자료

### 9.1 CIP 사양
- ODVA CIP Volume 1: Common Industrial Protocol (CIP™) and the Family of CIP Networks
- CIP Identity Object Definition

### 9.2 DeviceNet 사양
- DeviceNet Specification Release 2.0
- DeviceNet Device Object Model

### 9.3 cifX API
- cifX Toolkit API Reference Manual
- Hilscher DeviceNet Stack Documentation

### 9.4 관련 헤더 파일
- `CIP_common_definitions.h` - CIP 클래스 및 서비스 정의
- `DNS_packet_cip_service.h` - DeviceNet CIP 패킷 구조
- `cifXUser.h` - cifX API 함수 프로토타입

---

## 10. 향후 개선 사항

### 10.1 기능 확장
1. **다른 CIP Object 조회**:
   - Assembly Object (I/O 데이터 구조 확인)
   - Connection Object (연결 상태 확인)
   - Parameter Object (설정 값 조회)

2. **Set Service 구현**:
   - Set_Attribute_Single로 설정 변경
   - Reset Service로 Master 리셋

3. **에러 처리 강화**:
   - CIP Error Code 상세 파싱
   - Retry 로직 추가
   - 타임아웃 설정 최적화

### 10.2 성능 최적화
1. **캐싱**:
   - 한 번 조회한 Identity 정보 캐싱
   - Master 주소 캐싱

2. **병렬 조회**:
   - Multiple Attribute Get 사용 (지원 시)
   - 비동기 패킷 처리

3. **에러 복구**:
   - 자동 재시도
   - 대체 Master 주소 시도

---

## 11. 요약

### 11.1 구현 완료 항목
✅ CIP Explicit Messaging 기반 Master Identity 조회 함수 구현
✅ 7개 Identity Attribute 조회 기능
✅ Master Node ID 자동 검색 기능
✅ 에러 처리 및 가이드 메시지
✅ main.c 통합 및 자동 호출

### 11.2 핵심 코드 위치
- **헤더 파일**: `Hil_DemoApp/Includes/devicenet_master_info.h`
- **구현 파일**: `Hil_DemoApp/Sources/devicenet_master_info.c`
- **통합 위치**: `Core/Src/main.c` Line 347-384

### 11.3 사용 방법
기존 VAT 테스트 실행 시 자동으로 Master Identity가 조회 및 출력됩니다.
별도의 설정이나 함수 호출은 필요하지 않습니다.

---

**작업 완료일시**: 2025-10-28
**문서 버전**: 1.0

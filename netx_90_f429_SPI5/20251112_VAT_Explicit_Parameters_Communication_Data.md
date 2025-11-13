# VAT 파라미터별 Explicit Message 통신 데이터

**작성일**: 2025-11-12
**프로젝트**: VAT DeviceNet Implementation
**목적**: VAT 10개 파라미터의 Explicit Message 통신 데이터 정리

---

## 📋 목차

1. [기본 패킷 구조](#기본-패킷-구조)
2. [파라미터별 통신 데이터](#파라미터별-통신-데이터)
3. [CIP Service 코드](#cip-service-코드)
4. [에러 코드](#에러-코드)
5. [주의사항](#주의사항)

---

## 기본 패킷 구조

### CIP Service Indication (Master → Device)

```c
Cmd:  0x0000308A (DNS_CMD_CIP_SERVICE_IND)
Len:  20 bytes (읽기) / 20 + data_size bytes (쓰기)

Data Structure:
┌─────────────┬─────────────┬─────────────┬─────────────┬─────────────┬──────────┐
│ ulService   │ ulClass     │ ulInstance  │ ulAttribute │ ulMember    │ abData[] │
│ (4 bytes)   │ (4 bytes)   │ (4 bytes)   │ (4 bytes)   │ (4 bytes)   │ (변동)   │
├─────────────┼─────────────┼─────────────┼─────────────┼─────────────┼──────────┤
│ 0x0E or 0x10│ 0x64        │ 1-10        │ 0x01        │ 0x01        │ 쓰기 데이터│
│ Get or Set  │ VAT Param   │ Param ID    │ (고정)      │ Device ID   │ (Set만)  │
└─────────────┴─────────────┴─────────────┴─────────────┴─────────────┴──────────┘
```

### CIP Service Response (Device → Master)

```c
Cmd:  0x0000308B (DNS_CMD_CIP_SERVICE_RES)
Len:  8 bytes (기본) + data_size bytes (읽기 성공 시)

Data Structure:
┌─────────────┬─────────────┬──────────────┐
│ ulGRC       │ ulERC       │ abData[]     │
│ (4 bytes)   │ (4 bytes)   │ (변동)       │
├─────────────┼─────────────┼──────────────┤
│ 0x00=성공   │ 0x00        │ 읽은 데이터   │
│ 기타=에러   │ 확장 에러   │ (Get만)      │
└─────────────┴─────────────┴──────────────┘
```

---

## 파라미터별 통신 데이터

### 1. Param1: Pressure Setpoint (압력 설정값) ✏️ Writable

**속성**:
- 데이터 타입: INT16 (2 bytes)
- 범위: 0 ~ 10000 (0.01 mbar 단위)
- 기본값: 5000 (50.00 mbar)
- Class: 0x64, Instance: 1

#### 읽기 (Get)

```hex
Cmd:  0x0000308A
Len:  20 (0x14)

Data (20 bytes):
0E 00 00 00  64 00 00 00  01 00 00 00  01 00 00 00  01 00 00 00
│            │            │            │            │
Service=0x0E Class=0x64   Instance=1   Attribute=1  Member=1
(Get_Single) (VAT Param)  (Param1)     (고정)       (Device ID)
```

**예상 응답** (현재값 = 5000):
```hex
Cmd:  0x0000308B
Len:  10 (0x0A)

Data:
00 00 00 00  00 00 00 00  88 13
│            │            │
GRC=0        ERC=0        Value=5000 (Little-Endian: 0x1388)
(Success)    (No Error)
```

#### 쓰기 (Set) - 7000 (70.00 mbar)로 설정

```hex
Cmd:  0x0000308A
Len:  22 (0x16)

Data (22 bytes):
10 00 00 00  64 00 00 00  01 00 00 00  01 00 00 00  01 00 00 00  58 1B
│            │            │            │            │            │
Service=0x10 Class=0x64   Instance=1   Attribute=1  Member=1     Value=7000
(Set_Single)                                                     (0x1B58)
```

**예상 응답** (성공):
```hex
Cmd:  0x0000308B
Len:  8 (0x08)

Data:
00 00 00 00  00 00 00 00
│            │
GRC=0        ERC=0
(Success)
```

---

### 2. Param2: Position Setpoint (위치 설정값) ✏️ Writable

**속성**:
- 데이터 타입: INT16 (2 bytes)
- 범위: 0 ~ 10000 (0.01% 단위)
- 기본값: 5000 (50.00%)
- Class: 0x64, Instance: 2

#### 읽기 (Get)

```hex
Data (20 bytes):
0E 00 00 00  64 00 00 00  02 00 00 00  01 00 00 00  01 00 00 00
                           ↑
                           Instance=2 (Position Setpoint)
```

#### 쓰기 (Set) - 3000 (30.00%)로 설정

```hex
Data (22 bytes):
10 00 00 00  64 00 00 00  02 00 00 00  01 00 00 00  01 00 00 00  B8 0B
                           ↑                                       ↑
                           Instance=2                              Value=3000 (0x0BB8)
```

---

### 3. Param3: Control Mode (제어 모드) ✏️ Writable

**속성**:
- 데이터 타입: USINT (1 byte)
- 범위: 0 ~ 5
- 기본값: 0 (Pressure Control)
- Class: 0x64, Instance: 3

**제어 모드 값**:
```
0 = Pressure Control (압력 제어)
1 = Position Control (위치 제어)
2 = Open (열림)
3 = Close (닫힘)
4 = Hold (유지)
5 = Learn (학습)
```

#### 읽기 (Get)

```hex
Data (20 bytes):
0E 00 00 00  64 00 00 00  03 00 00 00  01 00 00 00  01 00 00 00
                           ↑
                           Instance=3 (Control Mode)
```

**예상 응답** (Pressure Control):
```hex
Data:
00 00 00 00  00 00 00 00  00
                           ↑
                           Value=0 (Pressure Control)
```

#### 쓰기 (Set) - Position Control (1)로 변경

```hex
Data (21 bytes):
10 00 00 00  64 00 00 00  03 00 00 00  01 00 00 00  01 00 00 00  01
                           ↑                                       ↑
                           Instance=3                              Value=1
```

---

### 4. Param4: Actual Pressure (현재 압력) 📖 READ-ONLY

**속성**:
- 데이터 타입: INT16 (2 bytes)
- 범위: 0 ~ 10000 (0.01 mbar 단위)
- 읽기 전용: I/O Assembly에서 자동 업데이트
- Class: 0x64, Instance: 4

#### 읽기 (Get)

```hex
Data (20 bytes):
0E 00 00 00  64 00 00 00  04 00 00 00  01 00 00 00  01 00 00 00
                           ↑
                           Instance=4 (Actual Pressure)
```

**예상 응답** (현재 압력 = 2500 = 25.00 mbar):
```hex
Data:
00 00 00 00  00 00 00 00  C4 09
                           ↑
                           Value=2500 (0x09C4)
```

#### ⚠️ 쓰기 시도 시 에러

```hex
쓰기 시도:
10 00 00 00  64 00 00 00  04 00 00 00  01 00 00 00  01 00 00 00  64 00

에러 응답:
02 00 00 00  00 00 00 00
↑
GRC=0x02 (Too Much Data / Read-Only Error)
```

---

### 5. Param5: Actual Position (현재 위치) 📖 READ-ONLY

**속성**:
- 데이터 타입: INT16 (2 bytes)
- 범위: 0 ~ 10000 (0.01% 단위)
- 읽기 전용: I/O Assembly에서 자동 업데이트
- Class: 0x64, Instance: 5

#### 읽기 (Get)

```hex
Data (20 bytes):
0E 00 00 00  64 00 00 00  05 00 00 00  01 00 00 00  01 00 00 00
                           ↑
                           Instance=5 (Actual Position)
```

**예상 응답** (현재 위치 = 4567 = 45.67%):
```hex
Data:
00 00 00 00  00 00 00 00  D7 11
                           ↑
                           Value=4567 (0x11D7)
```

---

### 6. Param6: Device Status (장치 상태) 📖 READ-ONLY

**속성**:
- 데이터 타입: USINT (1 byte)
- 범위: 0 ~ 4
- 읽기 전용: I/O Assembly에서 자동 업데이트
- Class: 0x64, Instance: 6

**상태 값**:
```
0 = Init (초기화 중)
1 = Idle (대기)
2 = Controlling (제어 중)
3 = Error (에러)
4 = Learn (학습 중)
```

#### 읽기 (Get)

```hex
Data (20 bytes):
0E 00 00 00  64 00 00 00  06 00 00 00  01 00 00 00  01 00 00 00
                           ↑
                           Instance=6 (Device Status)
```

**예상 응답** (Idle):
```hex
Data:
00 00 00 00  00 00 00 00  01
                           ↑
                           Value=1 (Idle)
```

---

### 7. Param7: Exception Status (예외 상태) 📖 READ-ONLY

**속성**:
- 데이터 타입: USINT (1 byte)
- 범위: 0 ~ 255 (비트 플래그)
- 읽기 전용: I/O Assembly에서 자동 업데이트
- Class: 0x64, Instance: 7

**비트 정의**:
```
Bit 0: Pressure Error (압력 에러)
Bit 1: Position Error (위치 에러)
Bit 2: Communication Error (통신 에러)
Bit 3-7: Reserved (예약됨)
```

#### 읽기 (Get)

```hex
Data (20 bytes):
0E 00 00 00  64 00 00 00  07 00 00 00  01 00 00 00  01 00 00 00
                           ↑
                           Instance=7 (Exception Status)
```

**예상 응답** (에러 없음):
```hex
Data:
00 00 00 00  00 00 00 00  00
                           ↑
                           Value=0x00 (No errors)
```

**예상 응답** (압력 에러 + 통신 에러):
```hex
Data:
00 00 00 00  00 00 00 00  05
                           ↑
                           Value=0x05 (Bit0=1, Bit2=1)
```

---

### 8. Param8: Access Mode (접근 모드) 📖 READ-ONLY

**속성**:
- 데이터 타입: USINT (1 byte)
- 범위: 0 ~ 1
- 읽기 전용
- Class: 0x64, Instance: 8

**접근 모드 값**:
```
0 = Local (로컬 제어)
1 = Remote (원격 제어)
```

#### 읽기 (Get)

```hex
Data (20 bytes):
0E 00 00 00  64 00 00 00  08 00 00 00  01 00 00 00  01 00 00 00
                           ↑
                           Instance=8 (Access Mode)
```

**예상 응답** (Remote):
```hex
Data:
00 00 00 00  00 00 00 00  01
                           ↑
                           Value=1 (Remote)
```

---

### 9. Param9: Pressure Upper Limit (압력 상한) ✏️ Writable

**속성**:
- 데이터 타입: INT16 (2 bytes)
- 범위: 0 ~ 10000 (0.01 mbar 단위)
- 기본값: 10000 (100.00 mbar)
- Class: 0x64, Instance: 9

#### 읽기 (Get)

```hex
Data (20 bytes):
0E 00 00 00  64 00 00 00  09 00 00 00  01 00 00 00  01 00 00 00
                           ↑
                           Instance=9 (Pressure Upper Limit)
```

**예상 응답** (기본값 = 10000):
```hex
Data:
00 00 00 00  00 00 00 00  10 27
                           ↑
                           Value=10000 (0x2710)
```

#### 쓰기 (Set) - 9500 (95.00 mbar)로 설정

```hex
Data (22 bytes):
10 00 00 00  64 00 00 00  09 00 00 00  01 00 00 00  01 00 00 00  1C 25
                           ↑                                       ↑
                           Instance=9                              Value=9500 (0x251C)
```

---

### 10. Param10: Pressure Lower Limit (압력 하한) ✏️ Writable

**속성**:
- 데이터 타입: INT16 (2 bytes)
- 범위: 0 ~ 10000 (0.01 mbar 단위)
- 기본값: 0 (0.00 mbar)
- Class: 0x64, Instance: 10

#### 읽기 (Get)

```hex
Data (20 bytes):
0E 00 00 00  64 00 00 00  0A 00 00 00  01 00 00 00  01 00 00 00
                           ↑
                           Instance=10 (Pressure Lower Limit)
```

**예상 응답** (기본값 = 0):
```hex
Data:
00 00 00 00  00 00 00 00  00 00
                           ↑
                           Value=0 (0x0000)
```

#### 쓰기 (Set) - 500 (5.00 mbar)로 설정

```hex
Data (22 bytes):
10 00 00 00  64 00 00 00  0A 00 00 00  01 00 00 00  01 00 00 00  F4 01
                           ↑                                       ↑
                           Instance=10                             Value=500 (0x01F4)
```

---

## CIP Service 코드

### 주요 Service 코드

| Service Code | 16진수 | 이름 | 기능 | Len 계산 |
|--------------|--------|------|------|----------|
| 0x0E | 0x0E | Get_Attribute_Single | 단일 속성 읽기 | 20 bytes |
| 0x10 | 0x10 | Set_Attribute_Single | 단일 속성 쓰기 | 20 + data_size |
| 0x01 | 0x01 | Get_Attributes_All | 모든 속성 읽기 | 20 bytes |
| 0x05 | 0x05 | Reset | 기본값 리셋 | 20 bytes |

### Get_Attributes_All 예시 (Param1)

```hex
Cmd:  0x0000308A
Len:  20

Data:
01 00 00 00  64 00 00 00  01 00 00 00  00 00 00 00  01 00 00 00
↑            ↑            ↑            ↑            ↑
Service=0x01 Class=0x64   Instance=1   Attribute=0  Member=1
(Get_All)                              (무시됨)
```

**예상 응답** (모든 속성 데이터):
```hex
Data:
00 00 00 00  00 00 00 00  [2] [4] [4] [4] [1] [2]
                           ↑   ↑   ↑   ↑   ↑   ↑
                           현재 최소 최대 기본 타입 Desc
                           값  값  값  값
```

---

## 에러 코드

### CIP General Response Codes (GRC)

| GRC | 16진수 | 이름 | 의미 |
|-----|--------|------|------|
| 0x00 | 0x00 | Success | 성공 |
| 0x02 | 0x02 | Too Much Data | 데이터 크기 초과 (Read-Only 쓰기 시도) |
| 0x04 | 0x04 | Not Enough Data | 데이터 부족 |
| 0x05 | 0x05 | Object Does Not Exist | 객체 없음 (잘못된 Instance) |
| 0x08 | 0x08 | Service Not Supported | 지원하지 않는 서비스 |
| 0x09 | 0x09 | Invalid Attribute Value | 잘못된 속성 값 (범위 초과) |
| 0x14 | 0x14 | Attribute Not Supported | 지원하지 않는 속성 |
| 0x1C | 0x1C | Permission Denied | 권한 없음 |

### 에러 응답 예시

#### 1. Read-Only 파라미터 쓰기 시도 (Param4)

```hex
요청:
10 00 00 00  64 00 00 00  04 00 00 00  01 00 00 00  01 00 00 00  64 00

응답:
02 00 00 00  00 00 00 00
↑            ↑
GRC=0x02     ERC=0x00
(Too Much Data / Read-Only)
```

#### 2. 범위 초과 값 쓰기 (Param1에 10001 쓰기)

```hex
요청:
10 00 00 00  64 00 00 00  01 00 00 00  01 00 00 00  01 00 00 00  11 27
                                                                   ↑
                                                                   10001 (Max=10000)

응답:
09 00 00 00  00 00 00 00
↑            ↑
GRC=0x09     ERC=0x00
(Invalid Attribute Value)
```

#### 3. 잘못된 Instance (Param99 - 존재하지 않음)

```hex
요청:
0E 00 00 00  64 00 00 00  63 00 00 00  01 00 00 00  01 00 00 00
                           ↑
                           Instance=99 (존재하지 않음)

응답:
05 00 00 00  00 00 00 00
↑            ↑
GRC=0x05     ERC=0x00
(Object Does Not Exist)
```

---

## 주의사항

### 1. 바이트 순서 (Endianness)

**모든 다중 바이트 값은 Little-Endian 사용**:
```
값 7000 (10진수) = 0x1B58 (16진수)
→ Little-Endian: 58 1B (낮은 바이트 먼저)

값 10000 (10진수) = 0x2710 (16진수)
→ Little-Endian: 10 27
```

### 2. Read-Only 파라미터

**읽기 전용 파라미터** (쓰기 시도 시 에러):
- Param4: Actual Pressure
- Param5: Actual Position
- Param6: Device Status
- Param7: Exception Status
- Param8: Access Mode

이 파라미터들은 **I/O Assembly**에서 자동으로 업데이트됩니다.

### 3. 파라미터 범위 검증

모든 쓰기 작업은 범위 검증 수행:
```c
// App_VAT_Parameters.c:195-197
if (value < pParam->min_value || value > pParam->max_value) {
    return -4;  // Out of range → GRC=0x09
}
```

### 4. ulMember 필드

**Device ID 확인 필수**:
- 현재 구현: `ulMember = 1` (Device ID = 1)
- 시스템의 MacID (Node ID)에 따라 변경 가능
- EDS 파일의 `#MacId` 섹션 참조

### 5. I/O Assembly와의 동기화

**App_VAT_ExplicitHandler.c** 참조:

#### Parameters → I/O Data (VAT_Sync_ParametersToIoData)
```c
// Param1 (Pressure Setpoint) → Output Assembly [1:2]
// Param3 (Control Mode)      → Output Assembly [0]
```

#### I/O Data → Parameters (VAT_Sync_IoDataToParameters)
```c
// Input Assembly [1:2] → Param4 (Actual Pressure)
// Input Assembly [3:4] → Param5 (Actual Position)
// Input Assembly [5]   → Param6 (Device Status)
// Input Assembly [0]   → Param7 (Exception Status)
```

### 6. netHOST 테스트 팁

**Send Packet 입력 형식**:
```
Cmd:  0x0000308A
Len:  20
Data: 0E 00 00 00 64 00 00 00 01 00 00 00 01 00 00 00 01 00 00 00
```

또는 가독성을 위해 스페이스로 구분:
```
Data: 0E 00 00 00  64 00 00 00  01 00 00 00  01 00 00 00  01 00 00 00
```

---

## 코드 참조

### 관련 파일

| 파일 | 위치 | 기능 |
|------|------|------|
| App_VAT_Parameters.c | Hil_DemoApp/Sources/ | 파라미터 초기화 및 관리 |
| App_VAT_ExplicitHandler.c | Hil_DemoApp/Sources/ | CIP Service 처리 |
| App_VAT_Parameters.h | Hil_DemoApp/Includes/ | 파라미터 구조체 정의 |
| DNS_packet_cip_service.h | Hil_DemoAppDNS/includes/DNS_API/ | CIP 패킷 구조체 |
| CIP_common_definitions.h | Hil_DemoAppDNS/includes/DNS_API/ | CIP Service 코드 정의 |

### 핵심 함수

```c
// App_VAT_Parameters.c
void VAT_Param_Init(VAT_PARAM_MANAGER_T* ptManager);
int32_t VAT_Param_Get(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id, void* pData, uint8_t* pSize);
int32_t VAT_Param_Set(VAT_PARAM_MANAGER_T* ptManager, uint8_t param_id, const void* pData, uint8_t size);

// App_VAT_ExplicitHandler.c
uint32_t VAT_Parameter_HandleService(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    DNS_PACKET_CIP_SERVICE_IND_T*  ptInd,
    DNS_PACKET_CIP_SERVICE_RES_T*  ptRes,
    uint32_t*                      pulResDataSize);
```

---

**문서 작성 완료**: 2025-11-12
**버전**: 1.0
**작성자**: Claude Code Assistant

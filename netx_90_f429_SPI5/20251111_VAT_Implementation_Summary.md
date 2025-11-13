# VAT Explicit Message 구현 요약

**작성일**: 2025-11-11
**프로젝트**: netx_90_f429_SPI5 - VAT Adaptive Pressure Controller

---

## 구현 내용

### 1. VAT Parameter Object (Class 0x64)

**10개의 VAT Valve 특성 파라미터** 구현:

| ID | 파라미터 | 타입 | 접근 | 기본값 | 설명 |
|----|----------|------|------|--------|------|
| 1 | Pressure Setpoint | INT16 | RW | 5000 (50.00 mbar) | 목표 압력 설정값 |
| 2 | Position Setpoint | INT16 | RW | 5000 (50.00%) | 목표 위치 설정값 |
| 3 | Control Mode | UINT8 | RW | 0 (Pressure) | 제어 모드 (0-5) |
| 4 | Actual Pressure | INT16 | RO | 0 | 현재 압력 값 (I/O 연동) |
| 5 | Actual Position | INT16 | RO | 0 | 현재 위치 값 (I/O 연동) |
| 6 | Device Status | UINT8 | RO | 1 (Idle) | 장치 상태 |
| 7 | Exception Status | UINT8 | RO | 0 | 에러 비트맵 |
| 8 | Access Mode | UINT8 | RO | 1 (Remote) | 제어 모드 |
| 9 | Pressure Upper Limit | INT16 | RW | 10000 | 압력 상한 |
| 10 | Pressure Lower Limit | INT16 | RW | 0 | 압력 하한 |

**지원 서비스**:
- `0x0E`: Get_Attribute_Single - 단일 파라미터 읽기
- `0x10`: Set_Attribute_Single - 단일 파라미터 쓰기 (RW만)
- `0x01`: Get_Attributes_All - 모든 속성 읽기
- `0x05`: Reset - 기본값으로 복원

### 2. VAT Diagnostic Object (Class 0x65)

**15개의 진단 Attributes** 구현:

| Attr | 이름 | 타입 | 설명 |
|------|------|------|------|
| 1 | Uptime | UINT32 | 가동 시간 (초) |
| 2 | Total Cycles | UINT32 | 총 제어 사이클 수 |
| 3 | Error Count | UINT16 | 총 에러 발생 횟수 |
| 4 | Last Error Code | UINT16 | 마지막 에러 코드 |
| 5 | Last Error Timestamp | UINT32 | 마지막 에러 시각 |
| 6-10 | Pressure/Position Stats | INT16 | Min/Max/Avg 통계 |
| 11-13 | Network Stats | UINT32/16 | 통신 통계 |
| 14 | Temperature | INT16 | 시스템 온도 |
| 15 | Firmware Version | UINT32 | 펌웨어 버전 |

**지원 서비스**:
- `0x0E`: Get_Attribute_Single - 단일 속성 읽기
- `0x01`: Get_Attributes_All - 전체 진단 데이터 읽기
- `0x05`: Reset - 통계 초기화

### 3. Explicit Message Handler 통합

**통합 코드**:
- `AppDNS_HandleCipServiceIndication()`: Class 0x64, 0x65 감지 시 VAT 핸들러로 라우팅
- `VAT_Explicit_HandleCipService_Direct()`: APP_DATA_T 기반 처리
- 자동 응답 패킷 생성 및 전송

---

## 구현 파일

### 신규 파일
1. **`Hil_DemoApp/Sources/App_VAT_Parameters.c`**
   - VAT 파라미터 초기화 및 관리
   - Get/Set 로직 구현
   - 범위 검증 및 Read-Only 체크

2. **`Hil_DemoApp/Sources/App_VAT_Diagnostic.c`**
   - 진단 데이터 수집 및 관리
   - 통계 계산 (Min/Max/Avg)
   - 에러 기록

3. **`Hil_DemoApp/Sources/App_VAT_ExplicitHandler.c`**
   - CIP Service 요청 처리
   - Parameter/Diagnostic 핸들러 라우팅
   - 응답 패킷 생성

### 수정 파일
1. **`Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c`**
   - VAT 핸들러 통합 (Class 0x64, 0x65 라우팅)

2. **`Hil_DemoAppDNS/includes/DemoAppDNS/AppDNS_DemoApplication.h`**
   - `APP_DNS_CHANNEL_HANDLER_RSC_T` 타입 정의 추가
   - `PKT_INTERFACE_H` 타입 정의 추가

---

## Hilscher Stack과의 차이점

### 이미 Stack에 구현된 것 (테스트 불필요)
- Identity Object (Class 0x01) - Vendor ID, Product Code 등
- Assembly Object 기본 기능 (Class 0x04)
- Connection Manager (Class 0x06)
- DeviceNet Object (Class 0x03)

### 새로 구현한 것 (테스트 필요)
✅ **VAT Parameter Object (Class 0x64)**
- VAT Valve 특성 파라미터 10개
- Explicit Message로 읽기/쓰기
- I/O Assembly와 독립적

✅ **VAT Diagnostic Object (Class 0x65)**
- 실시간 진단 데이터
- 통계 수집 및 제공
- 에러 기록

---

## 테스트 방법

### netHOST Device Test Application 사용

#### 1단계: Pressure Setpoint 읽기
```
Send Packet:
Cmd:  0x0000308A
Len:  16
Data: 0E 00 00 00 64 00 00 00 01 00 00 00 01 00 00 00
      ↑           ↑           ↑           ↑
      Get         Class 0x64  Instance 1  Attr 1

예상 응답: 88 13 (5000 = 50.00 mbar)
```

#### 2단계: Pressure Setpoint 쓰기
```
Send Packet:
Cmd:  0x0000308A
Len:  20
Data: 10 00 00 00 64 00 00 00 01 00 00 00 01 00 00 00 58 1B
      ↑           ↑           ↑           ↑           ↑
      Set         Class 0x64  Instance 1  Attr 1      7000 (70.00 mbar)

예상 응답: GRC=0x00 (Success)
```

#### 3단계: 읽기로 변경 확인
```
Send Packet: (1단계와 동일)
예상 응답: 58 1B (7000 = 70.00 mbar) ← 변경 확인
```

---

## 동작 흐름

```
netHOST Master
    ↓ DNS_CMD_CIP_SERVICE_IND (0x0000308A)
DeviceNet Stack
    ↓
AppDNS_HandleCipServiceIndication()
    ↓ Class 체크
    ├─ 0x64 or 0x65? → YES
    │   ↓
    │  VAT_Explicit_HandleCipService_Direct()
    │   ↓
    │  ├─ Class 0x64 → VAT_Parameter_HandleService()
    │  │   ↓
    │  │  VAT_Param_Get() / VAT_Param_Set()
    │  │
    │  └─ Class 0x65 → VAT_Diagnostic_HandleService()
    │      ↓
    │     g_tVatDiagnostics 데이터 반환
    │   ↓
    │  응답 패킷 생성
    │   ↓
    │  AppDNS_GlobalPacket_Send()
    │
    └─ Other Classes → 기존 Stack 처리
    ↓
DNS_CMD_CIP_SERVICE_RES (0x0000308B)
    ↓
netHOST Master
```

---

## 에러 처리

### 구현된 에러 코드

| GRC | 이름 | 발생 조건 |
|-----|------|-----------|
| 0x00 | Success | 정상 처리 |
| 0x02 | Too Much Data | Read-Only 파라미터 쓰기 시도 |
| 0x08 | Service Not Supported | 지원하지 않는 서비스 |
| 0x09 | Invalid Attribute Value | 범위 초과 값 |
| 0x14 | Attribute Not Supported | 존재하지 않는 Attribute |
| 0x16 | Object Does Not Exist | 존재하지 않는 Instance |

---

## I/O Assembly 연동 (향후 작업)

### 현재 상태
- Parameter Object와 I/O Assembly는 **독립적**으로 동작
- Explicit Message로 Parameter 변경 가능
- I/O 데이터는 cyclic으로 별도 처리

### 향후 구현 필요
```c
/* Sync Parameters → I/O Output Assembly */
void VAT_Sync_ParametersToIoData(void) {
    // Param1 (Pressure Setpoint) → Output Assembly [1:2]
    // Param3 (Control Mode) → Output Assembly [0]
}

/* Sync I/O Input Assembly → Parameters */
void VAT_Sync_IoDataToParameters(void) {
    // Input Assembly [1:2] (Pressure) → Param4 (Actual Pressure)
    // Input Assembly [3:4] (Position) → Param5 (Actual Position)
}
```

---

## 디버그 메시지

### 시리얼 출력 예시
```
=== CIP Service Indication ===
Service:   0x0E (Get Attribute Single)
Class:     0x64
Instance:  0x01
Attribute: 0x01
  -> Routing to VAT Handler
[VAT] CIP Service: 0x0E, Class: 0x64, Inst: 1, Attr: 1
[VAT] GET Param1: 88 13
[VAT] Response sent: GRC=0x00, DataSize=2
```

---

## 성공 기준

### Phase 1: 기본 기능 (현재)
- [x] Parameter Object 구현
- [x] Diagnostic Object 구현
- [x] Explicit Message Handler 통합
- [x] 컴파일 성공
- [ ] **netHOST 테스트 성공**

### Phase 2: I/O 연동 (다음)
- [ ] Parameter ↔ I/O Assembly 동기화
- [ ] Cyclic I/O 업데이트
- [ ] Real-time 진단 데이터 수집

### Phase 3: 고급 기능
- [ ] Parameter 저장/복원 (EEPROM)
- [ ] 에러 로깅
- [ ] 이벤트 알림

---

**작업 완료**: 2025-11-11
**다음 작업**: netHOST를 사용한 Explicit Message 테스트

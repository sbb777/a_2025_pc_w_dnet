# DeviceNet CIP 통신 테스트 가이드

## 수정 완료된 사항

### 1. 초기화 시퀀스 수정 (가장 중요!)
**문제**: Class 등록이 채널 준비 전에 실행되어 `0xC000012D` 에러 발생

**해결**:
- `AppDNS_ConfigureStack()` 함수에 채널 준비 대기 루프 추가
- `HIL_COMM_COS_READY` 플래그 확인 후에만 Class 등록 진행

**새로운 초기화 순서**:
```
1. SetConfiguration()      → 설정 전송
2. ChannelInit()            → 설정 활성화
3. StartCommunication()     → 통신 시작
4. [Wait for Ready]         → ⭐ 채널 준비 대기 (최대 5초)
5. VAT_Param_Init()         → 파라미터 초기화
6. RegisterAllVatClasses()  → Class 0x01, 0x30 등록
```

### 2. DNS_HOST_APP_REGISTRATION 매크로 활성화
**파일**: `AppDNS_DemoApplication.c:22`
```c
#define DNS_HOST_APP_REGISTRATION
```
→ 어플리케이션이 CIP 패킷을 받을 수 있도록 등록

### 3. 전역 변수 링크 수정
**파일**: `App_VAT_Parameters.c:5`
- `static` 키워드 제거로 외부 파일 접근 가능

### 4. 함수 선언 추가
**파일**: `AppDNS_DemoApplication.h:55`
```c
void AppDNS_HandleCipServiceIndication(APP_DATA_T* ptAppData);
```

---

## 테스트 절차

### Step 1: 프로젝트 빌드
```
1. STM32CubeIDE에서 프로젝트 클린 빌드
   - Project → Clean...
   - Project → Build All (Ctrl+B)

2. 빌드 에러 확인
   - ✅ 0 errors, 0 warnings 확인
```

### Step 2: 펌웨어 다운로드
```
1. STM32F429 보드 연결
2. Run → Debug As → STM32 C/C++ Application
3. 펌웨어 다운로드 완료 대기
```

### Step 3: UART 로그 확인

#### 시리얼 터미널 설정
```
Baud Rate:   115200
Data Bits:   8
Parity:      None
Stop Bits:   1
```

#### 예상되는 성공 로그
```
========================================
 DeviceNet Stack Initialization
========================================
Step 1: Calling AppDNS_ConfigureStack()...

Waiting for channel to be ready before registering classes...
Channel ready! (waited 150 ms)
  COS Flags: 0x00000001                    ← ✅ READY 플래그만 설정됨

VAT Parameter Manager initialized with 99 parameters

=== Registering VAT CIP Classes ===
✅ Class 0x01 registered successfully      ← ✅ Identity Object 등록 성공
✅ Class 0x30 registered successfully      ← ✅ VAT Object 등록 성공
===================================

Step 1: ✅ AppDNS_ConfigureStack() completed successfully
========================================
```

#### 에러 발생 시 로그 분석

**에러 1: 채널 준비 타임아웃**
```
ERROR: Channel ready timeout after 5000 ms!
  COS Flags: 0x00000000 (expected: 0x00000001)
```
→ **원인**: netX 통신 오류 또는 하드웨어 문제
→ **조치**:
  - SPI 연결 확인
  - netX 전원 확인
  - 펌웨어 재부팅

**에러 2: Class 등록 실패**
```
❌ Failed to register Class 0x01, error: 0xC000012D
```
→ **원인**: 여전히 채널이 준비되지 않음
→ **조치**:
  - 대기 시간 늘리기 (MAX_WAIT_MS를 10000으로 증가)
  - COS 플래그 값 확인

**에러 3: xChannelInfo 실패**
```
ERROR: xChannelInfo failed: 0x????????
```
→ **원인**: CIFX 드라이버 문제
→ **조치**:
  - 채널 핸들 유효성 확인
  - 드라이버 초기화 확인

---

### Step 4: CIP 통신 테스트

#### Test 4-1: Get Attribute Single (읽기)

**마스터에서 전송**:
```
Service:    0x0E (Get Attribute Single)
Class:      0x01
Instance:   0x01
Attribute:  0x01 (Vendor ID)
```

**예상 UART 로그**:
```
=== CIP Service Indication ===
Service:   0x0E (Get Attribute Single)
Class:     0x01
Instance:  0x01
Attribute: 0x01
Data Len:  0
  -> Get: Success, Data=94 01          ← ✅ Vendor ID = 404 (0x0194)
Response:  Cmd=0x0000B105, Len=18, Error=0x00
==============================
```

**마스터에서 수신**:
```
State:  0x00000000  ← ✅ 성공
Len:    2           ← ✅ 데이터 존재
Data:   94 01       ← ✅ Vendor ID = 404
```

#### Test 4-2: Set Attribute Single (쓰기)

**마스터에서 전송**:
```
Service:    0x10 (Set Attribute Single)
Class:      0x30
Instance:   0x01
Attribute:  0x0C (Controller Mode)
Data:       03      (Mode 3)
```

**예상 UART 로그**:
```
=== CIP Service Indication ===
Service:   0x10 (Set Attribute Single)
Class:     0x30
Instance:  0x01
Attribute: 0x0C
Data Len:  1
  Data: 03
  -> Set: Success                        ← ✅ 쓰기 성공
Response:  Cmd=0x0000B105, Len=17, Error=0x00
==============================
```

**마스터에서 수신**:
```
State:  0x00000000  ← ✅ 성공
Len:    0           ← ✅ Set은 데이터 없음
```

#### Test 4-3: Get Attribute All (전체 읽기)

**마스터에서 전송**:
```
Service:    0x01 (Get Attribute All)
Class:      0x30
Instance:   0x01
```

**예상 UART 로그**:
```
=== CIP Service Indication ===
Service:   0x01 (Get Attribute All)
Class:     0x30
Instance:  0x01
  -> Get All: Success, Size=...
Response:  Cmd=0x0000B105, Len=..., Error=0x00
==============================
```

---

## 디버깅 방법 (문제 발생 시)

### CubeIDE Live Expressions 설정

**Window → Show View → Live Expressions**

추가할 변수:
```c
// 채널 상태
ptAppData->aptChannels[0]->tChannelInfo.ulDeviceCOSFlags

// 파라미터 초기화 확인
g_tParamManager.param_count                  // 99여야 함
g_tParamManager.params[0].param_id           // 1이어야 함
g_tParamManager.params[0].data[0]            // 404 (Vendor ID)

// 패킷 정보
ptAppData->aptChannels[0]->tPacket.tHeader.ulCmd
ptAppData->aptChannels[0]->tPacket.tHeader.ulState
ptAppData->aptChannels[0]->tPacket.tHeader.ulLen
```

### Breakpoint 설정

**파일**: `AppDNS_DemoApplicationFunctions.c`

```c
// BP1: 채널 준비 확인 (Line 219)
if(ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tChannelInfo.ulDeviceCOSFlags & HIL_COMM_COS_READY)
    break;  // ← 여기에 브레이크포인트

// BP2: Class 등록 시작 (Line 255)
ulRet = AppDNS_RegisterAllVatClasses(ptAppData);  // ← 여기에 브레이크포인트

// BP3: CIP 서비스 수신 (Line 300)
AppDNS_HandleCipServiceIndication(ptAppData);  // ← 여기에 브레이크포인트
```

---

## 성공 기준

### ✅ 초기화 성공 조건
1. COS Flags = `0x00000001` (READY만 설정)
2. Class 0x01, 0x30 등록 성공
3. 타임아웃 없이 완료 (보통 100-200ms)

### ✅ CIP 통신 성공 조건
1. Get 요청: State = `0x00000000`, Len > 0
2. Set 요청: State = `0x00000000`, Error = `0x00`
3. UART에 정확한 데이터 출력

---

## 문제 해결 체크리스트

### 빌드 전
- [ ] `g_tParamManager`가 `App_VAT_Parameters.c`에 정의됨
- [ ] `App_VAT_Parameters.h`에 `extern` 선언됨
- [ ] `AppDNS_ExplicitMsg.h` include path 설정됨
- [ ] 모든 함수 선언이 헤더 파일에 존재함

### 초기화 시
- [ ] `DNS_HOST_APP_REGISTRATION` 매크로 정의됨
- [ ] `VAT_Param_Init()` 호출 확인
- [ ] `AppDNS_RegisterAllVatClasses()` 호출 확인
- [ ] UART에 등록 성공 메시지 출력됨
- [ ] COS 플래그 = 0x00000001 확인

### 런타임
- [ ] 패킷 핸들러 콜백 등록됨
- [ ] `case DNS_CMD_CIP_SERVICE_IND` 존재함
- [ ] CIP 서비스 핸들러 함수 구현됨
- [ ] 응답 패킷 전송 코드 있음

---

## 관련 파일 위치

| 파일 | 경로 | 수정 내용 |
|------|------|----------|
| AppDNS_DemoApplication.c | `Hil_DemoAppDNS/Sources/` | DNS_HOST_APP_REGISTRATION 추가 |
| AppDNS_DemoApplicationFunctions.c | `Hil_DemoAppDNS/Sources/` | 초기화 시퀀스 수정 |
| AppDNS_DemoApplication.h | `Hil_DemoAppDNS/includes/DemoAppDNS/` | 함수 선언 추가 |
| App_VAT_Parameters.c | `Hil_DemoApp/Sources/` | static 제거 |
| App_VAT_Parameters.h | `Hil_DemoApp/Includes/` | extern 선언 |
| AppDNS_ExplicitMsg.c | `Hil_DemoAppDNS/Sources/` | CIP 핸들러 구현 |

---

## 추가 참고 문서

- `STM32_DeviceNet_Debug_Guide.md` - 디버깅 상세 가이드
- `FIX_SUMMARY.md` - 수정 내역 요약
- `VAT_CIP_Test_Procedure.md` - CIP 테스트 절차

---

**작성일**: 2025-11-06
**버전**: 1.1
**대상**: STM32F429 + netX90 DeviceNet Slave

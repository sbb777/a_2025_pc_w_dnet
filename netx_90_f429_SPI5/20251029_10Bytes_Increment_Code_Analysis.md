# 10개 출력 바이트 1씩 증가 코드 분석

**작성일시**: 2025-10-29
**질문**: "DeviceNet 통신으로 10개의 출력 바이트가 1씩 증가하면서 송신된다. 이 코드가 실행되는 것이 VAT Test인가 아닌가?"

---

## 🎯 명확한 답변

**❌ 아니오, VAT Test가 아닙니다!**

현재 실행 중인 **10개 바이트 증가 코드**는 **일반 DeviceNet 데모 모드**입니다.

---

## 📊 코드 위치 확인

### 10개 바이트 증가 코드

**파일**: `Hil_DemoApp\Sources\App_DemoApplication.c`
**함수**: `App_IODataHandler()`
**라인**: 399-401

```c
/** OUTPUT DATA ****************************************/
/** update output data image to be sent in this cycle */
for (int i = 0; i< 10; i++){
    ptAppData->tOutputData.output[i]++;  // ← 10개 바이트를 1씩 증가
}

lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel,
                       0, 0,
                       sizeof(ptAppData->tOutputData),
                       &ptAppData->tOutputData,
                       0);
```

---

## 🔄 실행 흐름 분석

### 현재 설정 (Core\Src\main.c:122)

```c
volatile bool g_bEnableVatTest = false;  // ← VAT 테스트 비활성화
```

### 실행 경로

```
main()
  └─ InitializeToolkit() ✅
  └─ if (g_bEnableVatTest)  ← false
      ├─ [실행 안됨] VAT 테스트 모드
      └─ ...
  └─ else  ← 이 블록이 실행됨
      └─ App_CifXApplicationDemo("cifX0") ✅
          ├─ xDriverOpen()
          ├─ xChannelOpen()
          ├─ AppDNS_ConfigureStack() (방금 추가됨)
          └─ while (g_tAppData.fRunning)  ← 통신 루프
              ├─ App_IODataHandler(&g_tAppData) ✅
              │   └─ for (i=0; i<10; i++) output[i]++  ← 여기!
              ├─ App_AllChannels_PacketHandler()
              └─ OS_Sleep(500)
```

**결과**: `App_IODataHandler()`가 **500ms 주기**로 실행되면서 10개 바이트를 1씩 증가시킴

---

## 🔍 VAT Test vs 일반 모드 비교

| 항목 | VAT Test 모드 | 일반 DeviceNet 모드 (현재) |
|------|--------------|---------------------------|
| **활성화 플래그** | `g_bEnableVatTest = true` | `g_bEnableVatTest = false` ✅ |
| **실행 함수** | `VAT_RunTest(hChannel)` | `App_CifXApplicationDemo()` ✅ |
| **데이터 송신** | VAT 압력 제어 데이터<br>(control_mode, setpoint) | 10개 바이트 증가 데이터 ✅ |
| **송신 주기** | 100ms (VAT 모드에 따라 다름) | 500ms ✅ |
| **데이터 패턴** | 고정값 또는 설정값 | 연속 증가 (0→1→2→...) ✅ |
| **코드 위치** | `vat_devicenet_test.c` | `App_DemoApplication.c` ✅ |

---

## 📝 각 모드의 데이터 송신 특징

### 일반 DeviceNet 모드 (현재 실행 중) ✅

**데이터 구조**: `APP_PROCESS_DATA_OUTPUT_T` (10 bytes)

```c
typedef struct APP_PROCESS_DATA_OUTPUT_Ttag
{
  uint8_t output[10];  // 10개 바이트
} APP_PROCESS_DATA_OUTPUT_T;
```

**동작**:
```c
// 매 500ms마다 실행
for (int i = 0; i < 10; i++) {
    ptAppData->tOutputData.output[i]++;  // 0→1→2→3→...→255→0→1...
}
xChannelIOWrite(...);  // DeviceNet으로 송신
```

**특징**:
- ✅ 10개 바이트 모두 1씩 증가
- ✅ 255 다음에 0으로 롤오버
- ✅ 500ms 주기로 반복
- ✅ **이것이 현재 실행 중인 코드!**

---

### VAT Test 모드 (현재 비활성화) ❌

**데이터 구조**: `VAT_OUTPUT_ASSEMBLY_8_T` (5 bytes)

```c
typedef struct VAT_OUTPUT_ASSEMBLY_8_Ttag
{
    uint8_t control_mode;       // 1 byte: 제어 모드
    int16_t control_setpoint;   // 2 bytes: 압력 설정값
    uint8_t control_instance;   // 1 byte: 제어 인스턴스
    uint8_t reserved;           // 1 byte: 예약
} VAT_OUTPUT_ASSEMBLY_8_T;
```

**동작**:
```c
// VAT 테스트 설정에 따라
ptContext->output_asm8.control_mode = VAT_CONTROL_MODE_PRESSURE;
ptContext->output_asm8.control_setpoint = 500;  // 고정값 또는 설정값
xChannelIOWrite(...);  // DeviceNet으로 송신
```

**특징**:
- ❌ 바이트가 증가하지 않음
- ❌ 압력 제어 데이터 (고정값 또는 설정값)
- ❌ 100ms 주기
- ❌ **현재 실행되지 않음!**

---

## 🔑 핵심 정리

### 현재 상태
```
g_bEnableVatTest = false  → 일반 DeviceNet 모드 실행
  → App_CifXApplicationDemo() 실행
    → App_IODataHandler() 실행 (500ms 주기)
      → 10개 바이트 1씩 증가 송신 ✅
```

### 관찰된 현상
- ✅ DeviceNet 통신 정상
- ✅ 10개 출력 바이트가 1씩 증가
- ✅ 500ms 주기로 송신
- ✅ Master에서 수신 가능

**결론**: 이것은 **VAT Test가 아닌 일반 DeviceNet 데모 모드**입니다!

---

## 🔄 모드 전환 방법

### VAT Test로 전환하려면

**파일**: `Core\Src\main.c:122`

**변경**:
```c
// 현재
volatile bool g_bEnableVatTest = false;

// VAT Test 활성화
volatile bool g_bEnableVatTest = true;
```

**효과**:
- VAT 압력 제어 모드 실행
- 10개 바이트 증가 코드 실행 안됨
- VAT 특화 데이터 송신 (control_mode, setpoint 등)

---

## 📊 디버깅 확인 방법

### 1. Live Expressions로 확인

STM32CubeIDE에서 다음 변수 모니터링:

```c
g_bEnableVatTest           // false = 일반 모드, true = VAT 모드
g_tAppData.tOutputData.output[0]  // 1씩 증가하는지 확인
g_tAppData.tOutputData.output[1]
...
g_tAppData.tOutputData.output[9]
```

**예상 값** (일반 모드):
```
Cycle 1: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
Cycle 2: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2]
Cycle 3: [3, 3, 3, 3, 3, 3, 3, 3, 3, 3]
...
```

### 2. 브레이크포인트로 확인

**위치**: `Hil_DemoApp\Sources\App_DemoApplication.c:399`

```c
for (int i = 0; i< 10; i++){
    ptAppData->tOutputData.output[i]++;  // ← 브레이크포인트
}
```

**확인 사항**:
- 이 코드가 실행되면 → **일반 모드**
- 이 코드가 실행 안되면 → **VAT 모드**

### 3. 시리얼 로그로 확인

**일반 모드 시작 메시지**:
```
---------- cifX Application Demo ----------
```

**VAT 모드 시작 메시지**:
```
========================================
 VAT DeviceNet Test Initialization
========================================
```

---

## 💡 추가 정보

### 일반 모드의 목적

10개 바이트 증가 코드는 **DeviceNet 통신 테스트용 데모 코드**입니다:

**용도**:
- DeviceNet 통신이 정상 작동하는지 확인
- Master에서 데이터가 변화하는 것을 시각적으로 확인
- IO 데이터 교환 주기 확인 (500ms)
- 통신 안정성 테스트

**실제 제품에서는**:
- 이 코드를 센서 데이터나 실제 제어 데이터로 교체해야 함
- 예: 압력 센서 값, 온도 값, 상태 플래그 등

---

## 📚 관련 코드 파일

### 일반 DeviceNet 모드
- **메인**: `Hil_DemoApp\Sources\App_DemoApplication.c`
  - `App_CifXApplicationDemo()` (Line 236-357)
  - `App_IODataHandler()` (Line 370-414)

### VAT Test 모드
- **메인**: `Hil_DemoApp\Sources\vat_devicenet_test.c`
  - `VAT_Test_WriteOutput()` (Line 240-277)
  - `VAT_Test_SetPressureSetpoint()` (Line 303-316)

### 설정 파일
- **모드 전환**: `Core\Src\main.c`
  - `g_bEnableVatTest` (Line 122)
  - `g_VatTestMode` (Line 115)

---

## ✅ 최종 결론

**질문**: "10개 바이트가 1씩 증가하는 것이 VAT Test인가?"

**답변**: **❌ 아니오, VAT Test가 아닙니다!**

**현재 실행 중인 것**:
- 일반 DeviceNet 데모 모드
- `App_IODataHandler()` 함수
- 500ms 주기로 10개 바이트 증가
- 통신 테스트용 데모 코드

**VAT Test로 전환하려면**:
- `g_bEnableVatTest = true`로 변경
- 재빌드 및 플래시
- VAT 압력 제어 데이터 송신으로 변경됨

---

**분석 완료**: 2025-10-29
**분석자**: Claude AI Assistant

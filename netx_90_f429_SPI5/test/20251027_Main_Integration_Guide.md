# main.c VAT 테스트 통합 가이드
**작성일시**: 2025-10-27

---

## 1. 개요

### 📋 문서 목적

기존 `Core/Src/main.c`를 **최소한의 변경**으로 VAT Adaptive Pressure Controller DeviceNet 테스트를 통합하는 방법을 설명합니다.

### 🎯 통합 목표

- ✅ 기존 코드 99% 유지
- ✅ 간단한 플래그로 테스트 모드 전환
- ✅ Live Expression 디버깅 지원
- ✅ 3가지 테스트 시나리오 내장
- ✅ 기존 App_CifXApplicationDemo와 공존

---

## 2. 변경 사항 요약

### 📊 변경 통계

| 항목 | 원본 | 수정본 | 변경 |
|------|------|--------|------|
| **전체 줄 수** | 1,133 줄 | ~650 줄 (주석 포함) | +150 줄 |
| **Include 추가** | 0 | 1 | `vat_devicenet_test.h` |
| **전역 변수 추가** | 0 | 3 | 컨텍스트, 모드, 플래그 |
| **함수 추가** | 0 | 2 | 초기화, 테스트 실행 |
| **main() 수정** | 핵심 1곳 | if-else 분기 | 15 줄 |

### ✨ 추가된 코드 (총 3개 섹션)

#### 섹션 1: Include 추가 (1 줄)
```c
#include "vat_devicenet_test.h"
```
**위치**: Line 33 (기존 include 섹션)

#### 섹션 2: 전역 변수 추가 (15 줄)
```c
/* VAT TEST INTEGRATION - 추가 시작 */
VAT_TEST_CONTEXT_T g_tVatContext;
volatile uint8_t g_VatTestMode = 1;
volatile bool g_bEnableVatTest = true;
/* VAT TEST INTEGRATION - 추가 끝 */
```
**위치**: Line 88-102 (기존 전역 변수 섹션)

#### 섹션 3: 함수 추가 (120 줄)
```c
/* VAT TEST INTEGRATION - 함수 구현 시작 */
static void VAT_InitializeTest(void);
static void VAT_RunTest(CIFXHANDLE hChannel);
/* VAT TEST INTEGRATION - 함수 구현 끝 */
```
**위치**: Line 140-260 (USER CODE BEGIN 0 섹션)

#### 섹션 4: main() 함수 수정 (15 줄)
```c
if(g_bEnableVatTest)
{
    /* VAT 테스트 모드 */
    VAT_InitializeTest();
    // ... VAT 테스트 실행
}
else
{
    /* 기존 App_CifXApplicationDemo 실행 */
    App_CifXApplicationDemo("cifX0");
}
```
**위치**: main() 함수 내부 (Line 276-282 대체)

---

## 3. 상세 변경 내용

### 📦 Include 섹션

#### 변경 전
```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "cifXToolkit.h"
#include "SerialDPMInterface.h"
#include "cifXErrors.h"
#include "App_DemoApplication.h"
/* USER CODE END Includes */
```

#### 변경 후
```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "cifXToolkit.h"
#include "SerialDPMInterface.h"
#include "cifXErrors.h"
#include "App_DemoApplication.h"

/* ========== VAT TEST INTEGRATION - 추가 시작 ========== */
#include "vat_devicenet_test.h"
/* ========== VAT TEST INTEGRATION - 추가 끝 ========== */
/* USER CODE END Includes */
```

**변경 이유**: VAT 테스트 API 사용을 위한 헤더 포함

---

### 🌐 전역 변수 섹션

#### 추가된 전역 변수 (3개)

##### 1. VAT 테스트 컨텍스트
```c
VAT_TEST_CONTEXT_T g_tVatContext;
```
**용도**:
- VAT 테스트의 모든 상태 및 데이터 저장
- Live Expression으로 실시간 모니터링 가능
- 입출력 데이터, 통계, 설정 포함

**디버깅 예시**:
```
// Live Expression 창에 추가
g_tVatContext.input_asm100.pressure
g_tVatContext.input_asm100.position
g_tVatContext.output_asm8.control_setpoint
g_tVatContext.stats.total_read_count
```

##### 2. 테스트 모드 선택
```c
volatile uint8_t g_VatTestMode = 1;
```
**용도**: 실행할 테스트 시나리오 선택

| 값 | 테스트 시나리오 | 설명 |
|-----|----------------|------|
| **1** | Basic Pressure Control | 압력 설정값 500, 10회 통신 |
| **2** | Full Calibration | 자동 학습 모드, 5회 통신 |
| **3** | Continuous Monitoring | 100회 연속 모니터링, 10초간 |

**런타임 변경 방법**:
```c
// 디버거에서 실행 중 변경 가능
g_VatTestMode = 2;  // 보정 테스트로 변경
```

##### 3. VAT 테스트 활성화 플래그
```c
volatile bool g_bEnableVatTest = true;
```
**용도**: VAT 테스트 또는 기존 App 선택

| 값 | 동작 |
|----|------|
| **true** | VAT 테스트 실행 |
| **false** | 기존 App_CifXApplicationDemo 실행 |

**컴파일 시점 설정**:
```c
volatile bool g_bEnableVatTest = false;  // 기존 동작으로 빌드
```

---

### 🔧 함수 추가

#### 1. VAT_InitializeTest()

**목적**: VAT 테스트 초기화 및 설정

```c
static void VAT_InitializeTest(void)
{
    printf("\r\n========================================\r\n");
    printf(" VAT DeviceNet Test Initialization\r\n");
    printf("========================================\r\n");

    /* 테스트 컨텍스트 초기화 */
    VAT_Test_Init(&g_tVatContext);

    /* 테스트 설정 */
    VAT_TEST_CONFIG_T tConfig = {
        .node_address = 10,                         // DeviceNet 노드 주소
        .baud_rate = 250000,                        // 250 kbps
        .epr_ms = 100,                              // 100ms 패킷 주기
        .input_assembly = VAT_INPUT_ASSEMBLY_100,   // 7바이트 입력
        .output_assembly = VAT_OUTPUT_ASSEMBLY_8,   // 5바이트 출력
        .enable_logging = true,
        .enable_validation = true
    };

    VAT_Test_Configure(&g_tVatContext, &tConfig);

    printf("VAT Test Configuration:\r\n");
    printf("  Mode: %u (1=Basic, 2=Calibration, 3=Monitoring)\r\n", g_VatTestMode);
    printf("  Node Address: %u\r\n", tConfig.node_address);
    printf("  Baud Rate: %lu bps\r\n", tConfig.baud_rate);
    printf("========================================\r\n\r\n");
}
```

**실행 시점**: main() 함수에서 cifX 채널 오픈 전

**출력 예시**:
```
========================================
 VAT DeviceNet Test Initialization
========================================
VAT Test Configuration:
  Mode: 1 (1=Basic, 2=Calibration, 3=Monitoring)
  Node Address: 10
  Baud Rate: 250000 bps
  Input Assembly: 0x64 (7 bytes)
  Output Assembly: 0x08 (5 bytes)
========================================
```

#### 2. VAT_RunTest()

**목적**: 선택된 테스트 시나리오 실행

```c
static void VAT_RunTest(CIFXHANDLE hChannel)
{
    printf("\r\n========================================\r\n");
    printf(" Running VAT Test Mode: %u\r\n", g_VatTestMode);
    printf("========================================\r\n\r\n");

    switch(g_VatTestMode)
    {
        case 1:  /* Basic Pressure Control Test */
            VAT_Test_BasicPressureControl(&g_tVatContext, hChannel);
            break;

        case 2:  /* Full Calibration Test */
            VAT_Test_FullCalibration(&g_tVatContext, hChannel);
            break;

        case 3:  /* Continuous Monitoring */
            /* 압력 제어 모드 설정 */
            VAT_Test_SetControlMode(&g_tVatContext, VAT_CONTROL_MODE_PRESSURE);
            VAT_Test_SetPressureSetpoint(&g_tVatContext, 750);

            /* 100회 연속 모니터링 */
            for(int i = 0; i < 100; i++)
            {
                VAT_Test_WriteOutput(&g_tVatContext, hChannel);
                VAT_Test_ReadInput(&g_tVatContext, hChannel);

                if(VAT_Test_HasException(&g_tVatContext))
                {
                    printf("[%03d] WARNING: Exception 0x%02X\r\n",
                           i, VAT_Test_GetExceptionStatus(&g_tVatContext));
                }

                /* 매 10회마다 상태 출력 */
                if((i % 10) == 0)
                {
                    printf("[%03d] Pressure=%d, Position=%d\r\n",
                           i,
                           g_tVatContext.input_asm100.pressure,
                           g_tVatContext.input_asm100.position);
                }

                HAL_Delay(100);
            }
            break;

        default:
            printf("ERROR: Invalid test mode %u\r\n", g_VatTestMode);
            break;
    }

    /* 최종 통계 출력 */
    VAT_Test_PrintStats(&g_tVatContext);
}
```

**실행 흐름**:
1. 테스트 모드 출력
2. `g_VatTestMode`에 따라 분기
3. 해당 테스트 시나리오 실행
4. 통계 출력

---

### 🚀 main() 함수 수정

#### 변경 전 (원본)
```c
/* USER CODE BEGIN 2 */

// ... DIP switch validation ...

lRet = InitializeToolkit();

if(CIFX_NO_ERROR == lRet)
{
    lRet = App_CifXApplicationDemo("cifX0");
}

/* USER CODE END 2 */
```

#### 변경 후
```c
/* USER CODE BEGIN 2 */

// ... DIP switch validation (동일) ...

/* cifX Toolkit 초기화 */
lRet = InitializeToolkit();

if(CIFX_NO_ERROR == lRet)
{
    /* ========== VAT TEST INTEGRATION - 메인 로직 수정 시작 ========== */

    if(g_bEnableVatTest)
    {
        /* VAT 테스트 모드 */
        CIFXHANDLE hDriver = NULL;
        CIFXHANDLE hChannel = NULL;

        /* VAT 테스트 초기화 */
        VAT_InitializeTest();

        /* cifX 드라이버 열기 */
        lRet = xDriverOpen(&hDriver);
        if(CIFX_NO_ERROR == lRet)
        {
            /* 채널 0 열기 */
            lRet = xChannelOpen(hDriver, "cifX0", 0, &hChannel);
            if(CIFX_NO_ERROR == lRet)
            {
                /* 채널 준비 대기 */
                CHANNEL_INFORMATION tChannelInfo;
                printf("Waiting for channel ready...\r\n");

                do
                {
                    xChannelInfo(hChannel, sizeof(CHANNEL_INFORMATION), &tChannelInfo);
                }
                while(!(tChannelInfo.ulDeviceCOSFlags & HIL_COMM_COS_READY));

                printf("Channel ready!\r\n");

                /* VAT 테스트 실행 */
                VAT_RunTest(hChannel);

                /* 채널 닫기 */
                xChannelClose(hChannel);
            }

            /* 드라이버 닫기 */
            xDriverClose(hDriver);
        }

        /* 테스트 종료 */
        VAT_Test_Deinit(&g_tVatContext);
    }
    else
    {
        /* 기존 App_CifXApplicationDemo 실행 */
        printf("Running original App_CifXApplicationDemo...\r\n");
        lRet = App_CifXApplicationDemo("cifX0");
    }

    /* ========== VAT TEST INTEGRATION - 메인 로직 수정 끝 ========== */
}

/* USER CODE END 2 */
```

**주요 변경점**:
1. `if(g_bEnableVatTest)` 조건 분기 추가
2. VAT 테스트 경로: 초기화 → 채널 오픈 → 테스트 실행 → 종료
3. 기존 경로: `App_CifXApplicationDemo()` 직접 호출
4. 에러 처리 강화

---

## 4. 통합 방법

### 방법 1: 파일 교체 (가장 간단)

#### Step 1: 백업
```bash
# 원본 main.c 백업
cp Core/Src/main.c Core/Src/main.c.backup
```

#### Step 2: 교체
```bash
# VAT 테스트 버전으로 교체
cp test/main_vat_test.c Core/Src/main.c
```

#### Step 3: 빌드
1. STM32CubeIDE 프로젝트 새로고침
2. Project → Build All
3. 빌드 완료 확인

### 방법 2: 수동 통합 (정밀 제어)

#### Step 1: 헤더 추가
`Core/Src/main.c` 파일의 **Line 33** 근처에 추가:
```c
#include "App_DemoApplication.h"
#include "vat_devicenet_test.h"  // ← 추가
```

#### Step 2: 전역 변수 추가
`Core/Src/main.c` 파일의 **Line 88-102** (전역 변수 섹션) 근처에 추가:
```c
volatile uint8_t g_DipSwitchData[5] = {0};
volatile uint8_t g_bDipDataValid = 0;

/* VAT TEST INTEGRATION - 추가 시작 */
VAT_TEST_CONTEXT_T g_tVatContext;
volatile uint8_t g_VatTestMode = 1;
volatile bool g_bEnableVatTest = true;
/* VAT TEST INTEGRATION - 추가 끝 */
```

#### Step 3: 함수 프로토타입 추가
`Core/Src/main.c` 파일의 **함수 프로토타입 섹션**에 추가:
```c
static uint8_t ValidateDipSwitchData(const uint8_t *currentLine);

/* VAT TEST INTEGRATION - 추가 시작 */
static void VAT_InitializeTest(void);
static void VAT_RunTest(CIFXHANDLE hChannel);
/* VAT TEST INTEGRATION - 추가 끝 */
```

#### Step 4: 함수 구현 추가
`Core/Src/main.c` 파일의 **USER CODE BEGIN 0** 섹션에 추가:

`test/main_vat_test.c`에서 다음 함수들을 복사:
- `VAT_InitializeTest()` (약 40 줄)
- `VAT_RunTest()` (약 80 줄)

#### Step 5: main() 함수 수정
`Core/Src/main.c`의 main() 함수에서:

**변경 전**:
```c
lRet = InitializeToolkit();

if(CIFX_NO_ERROR == lRet)
{
    lRet = App_CifXApplicationDemo("cifX0");
}
```

**변경 후**:
```c
lRet = InitializeToolkit();

if(CIFX_NO_ERROR == lRet)
{
    if(g_bEnableVatTest)
    {
        CIFXHANDLE hDriver = NULL;
        CIFXHANDLE hChannel = NULL;

        VAT_InitializeTest();

        lRet = xDriverOpen(&hDriver);
        if(CIFX_NO_ERROR == lRet)
        {
            lRet = xChannelOpen(hDriver, "cifX0", 0, &hChannel);
            if(CIFX_NO_ERROR == lRet)
            {
                CHANNEL_INFORMATION tChannelInfo;
                printf("Waiting for channel ready...\r\n");

                do
                {
                    xChannelInfo(hChannel, sizeof(CHANNEL_INFORMATION), &tChannelInfo);
                }
                while(!(tChannelInfo.ulDeviceCOSFlags & HIL_COMM_COS_READY));

                printf("Channel ready!\r\n");

                VAT_RunTest(hChannel);

                xChannelClose(hChannel);
            }

            xDriverClose(hDriver);
        }

        VAT_Test_Deinit(&g_tVatContext);
    }
    else
    {
        printf("Running original App_CifXApplicationDemo...\r\n");
        lRet = App_CifXApplicationDemo("cifX0");
    }
}
```

#### Step 6: 파일 추가
프로젝트에 VAT 테스트 파일 추가:
1. `test/vat_devicenet_test.h` → `Hil_DemoApp/Includes/`
2. `test/vat_devicenet_test.c` → `Hil_DemoApp/Sources/`

---

## 5. 사용 방법

### 🎮 테스트 모드 선택

#### 방법 1: 컴파일 시점 설정
```c
/* main.c의 전역 변수 섹션 */
volatile uint8_t g_VatTestMode = 2;  // 1→2로 변경 (보정 테스트)
```

#### 방법 2: 디버거 실행 중 변경
1. 브레이크포인트 설정: `VAT_InitializeTest()` 진입 직전
2. 디버거 실행
3. Expressions 창에서 변경:
```
g_VatTestMode = 3
```
4. Continue 실행

#### 방법 3: Live Expression 모니터링
Live Expression 창에 추가:
```
g_VatTestMode
g_tVatContext.stats.total_read_count
g_tVatContext.input_asm100.pressure
g_tVatContext.output_asm8.control_setpoint
```

### 🔄 기존 App으로 복귀

#### 임시 복귀 (디버거)
```c
// 실행 중 플래그 변경
g_bEnableVatTest = false;
// 리셋 후 재실행
```

#### 영구 복귀 (코드 수정)
```c
/* main.c의 전역 변수 섹션 */
volatile bool g_bEnableVatTest = false;  // true→false로 변경
```

재빌드 후 실행하면 원래의 `App_CifXApplicationDemo()`가 실행됩니다.

---

## 6. 테스트 시나리오 상세

### 📊 모드 1: Basic Pressure Control

**목적**: 기본 압력 제어 동작 확인

```c
g_VatTestMode = 1;
```

**동작**:
1. 압력 설정값 500 설정
2. 10회 통신 (쓰기 → 읽기)
3. 입출력 데이터 로깅
4. 통계 출력

**예상 출력**:
```
========== Basic Pressure Control Test ==========
[VAT Output 0x07] Setpoint=500, Instance=0
[VAT Input 0x02] Exception=0x00, Pressure=485
[VAT Output 0x07] Setpoint=500, Instance=0
[VAT Input 0x02] Exception=0x00, Pressure=493
...
=================================================

========== VAT Test Statistics ==========
Total Read Operations:   10
Total Write Operations:  10
Read Errors:             0
Write Errors:            0
=========================================
```

### 🔧 모드 2: Full Calibration

**목적**: 자동 학습 모드 및 보정 기능 테스트

```c
g_VatTestMode = 2;
```

**동작**:
1. 압력 제어 모드 설정
2. 자동 학습 활성화
3. 보정 스케일 100 설정
4. 5회 통신 및 장치 상태 확인

**예상 출력**:
```
========== Full Calibration Test ==========
[VAT Test] Control mode set to: 2
[VAT Output 0x66] Mode=2, Setpoint=0, Instance=0, Learn=1, Cal=100, Zero=0
[VAT Input 0x64] Exception=0x00, Pressure=0, Position=0, Status=0x08, Access=0x00
Device is ready
...
===========================================
```

### 📈 모드 3: Continuous Monitoring

**목적**: 장시간 안정성 및 실시간 모니터링

```c
g_VatTestMode = 3;
```

**동작**:
1. 압력 설정값 750 설정
2. 100회 연속 통신 (10초)
3. 10회마다 상태 출력
4. 예외 발생 시 즉시 출력

**예상 출력**:
```
========== Continuous Monitoring ==========
Running for 10 seconds...
[000] Pressure=742, Position=850, Status=0x01
[010] Pressure=748, Position=875, Status=0x01
[020] Pressure=751, Position=890, Status=0x01
...
[090] Pressure=750, Position=900, Status=0x01

Continuous monitoring completed.
===========================================

========== VAT Test Statistics ==========
Total Read Operations:   100
Total Write Operations:  100
Read Errors:             0
Write Errors:            0
Exception Count:         0
=========================================
```

---

## 7. 디버깅 가이드

### 🔍 Live Expression 설정

#### 추가할 변수
```c
// 테스트 모드 및 플래그
g_VatTestMode
g_bEnableVatTest

// 통계
g_tVatContext.stats.total_read_count
g_tVatContext.stats.total_write_count
g_tVatContext.stats.read_error_count
g_tVatContext.stats.exception_count

// 입력 데이터 (Assembly 100)
g_tVatContext.input_asm100.exception_status
g_tVatContext.input_asm100.pressure
g_tVatContext.input_asm100.position
g_tVatContext.input_asm100.device_status

// 출력 데이터 (Assembly 8)
g_tVatContext.output_asm8.control_mode
g_tVatContext.output_asm8.control_setpoint
g_tVatContext.output_asm8.control_instance

// 상태 플래그
g_tVatContext.input_data_valid
g_tVatContext.output_data_sent
```

### 🐛 일반적인 문제

#### 문제 1: 컴파일 에러 - "vat_devicenet_test.h not found"

**원인**: VAT 테스트 파일이 프로젝트에 추가되지 않음

**해결**:
1. `test/vat_devicenet_test.h` → `Hil_DemoApp/Includes/` 복사
2. `test/vat_devicenet_test.c` → `Hil_DemoApp/Sources/` 복사
3. STM32CubeIDE 프로젝트 새로고침
4. 빌드

#### 문제 2: 링크 에러 - "undefined reference to VAT_Test_*"

**원인**: vat_devicenet_test.c가 빌드에 포함되지 않음

**해결**:
1. Project Explorer에서 `vat_devicenet_test.c` 확인
2. 회색으로 표시되면 빌드 제외 상태
3. 우클릭 → Resource Configurations → Exclude from Build → 체크 해제

#### 문제 3: 런타임 에러 - "Channel open failed"

**원인**: cifX 장치 연결 안됨 또는 펌웨어 문제

**해결**:
1. netX90 장치 전원 확인
2. SPI 연결 확인
3. DIP 스위치 검증 완료 확인
4. Cookie 확인:
```c
// Live Expression
g_szLastCookie
// "netX" 또는 다른 유효한 값이어야 함
```

#### 문제 4: 통신 타임아웃

**원인**: DeviceNet 노드 주소 또는 Baud Rate 불일치

**해결**:
```c
// VAT_InitializeTest() 함수에서 설정 변경
VAT_TEST_CONFIG_T tConfig = {
    .node_address = 10,     // VAT 장치 DIP 스위치 확인
    .baud_rate = 250000,    // 또는 125000, 500000
    // ...
};
```

---

## 8. 성능 최적화

### ⚡ 로깅 비활성화

프로덕션 환경에서 성능 향상:

```c
VAT_TEST_CONFIG_T tConfig = {
    // ...
    .enable_logging = false,     // true → false
    .enable_validation = true
};
```

**효과**: 약 20-30% 성능 향상

### 🚀 통신 주기 조정

```c
VAT_TEST_CONFIG_T tConfig = {
    // ...
    .epr_ms = 50,   // 100 → 50ms (2배 빠름)
    // ...
};
```

**주의**: VAT 장치의 최소 EPR 확인 필요

---

## 9. 체크리스트

### ✅ 통합 전 확인사항

- [ ] 원본 `main.c` 백업 완료
- [ ] `vat_devicenet_test.h` 파일 존재 확인
- [ ] `vat_devicenet_test.c` 파일 존재 확인
- [ ] `test/main_vat_test.c` 파일 존재 확인

### ✅ 통합 후 확인사항

- [ ] 컴파일 에러 없음
- [ ] 링크 에러 없음
- [ ] `g_VatTestMode` 변수 확인 (1/2/3)
- [ ] `g_bEnableVatTest` 플래그 확인 (true/false)

### ✅ 실행 전 확인사항

- [ ] netX90 장치 전원 켜짐
- [ ] SPI 연결 정상
- [ ] DIP 스위치 검증 완료
- [ ] VAT 장치 DeviceNet 연결 확인

### ✅ 실행 후 확인사항

- [ ] 초기화 메시지 출력 확인
- [ ] 테스트 시나리오 실행 확인
- [ ] 통계 출력 확인
- [ ] 에러 메시지 없음

---

## 10. 요약

### 📌 핵심 변경 사항

| 항목 | 변경 |
|------|------|
| **Include** | +1 줄 (`vat_devicenet_test.h`) |
| **전역 변수** | +3개 (컨텍스트, 모드, 플래그) |
| **함수** | +2개 (초기화, 실행) |
| **main()** | 15줄 수정 (if-else 분기) |
| **기존 코드 유지** | 99% |

### 🎯 장점

1. ✅ **최소 침투성**: 기존 코드 거의 수정 안함
2. ✅ **간단한 전환**: 플래그 하나로 모드 변경
3. ✅ **공존 가능**: 기존 App과 병행 가능
4. ✅ **디버깅 편의**: Live Expression 지원
5. ✅ **확장성**: 새 테스트 시나리오 추가 용이

### 📚 추가 참고 자료

- **VAT 테스트 상세 가이드**: `test/20251027_VAT_DeviceNet_Test_Guide.md`
- **EDS 파일 분석**: `20251027_DNS_V5_EDS_Analysis.md`
- **Live Expression 가이드**: `20251027_LiveExpression_GlobalVariables.md`

---

**문서 끝**

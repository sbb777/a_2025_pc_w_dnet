# VAT 테스트 IO 데이터 변화 없음 문제 분석 및 해결

**작성일시**: 2025-01-30
**문제**: 무한 루프 코드 적용 후에도 NetHost IO 데이터가 변화하지 않음
**프로젝트**: STM32 F429 + netX90 DeviceNet Slave
**대상**: VAT Adaptive Pressure Controller

---

## 🔴 문제 상황

### 관찰된 증상

**시리얼 로그 출력**:
```
[VAT Output 0x08] Mode=2, Setpoint=367, Instance=0
[VAT Input 0x64] Exception=0x00, Pressure=0, Position=0, Status=0x00, Access=0x00
```

**분석**:
- ✅ **Setpoint가 증가 중** (367) → 코드는 실행되고 있음
- ❌ **NetHost IO 데이터 변화 없음** → 실제 통신이 안됨
- ❌ **Input 데이터 모두 0** → Master로부터 데이터 수신 안됨

---

## 🔍 원인 분석

### 원인 1: VAT 테스트는 내부 구조체만 업데이트 (가장 유력)

**문제**:
```c
// vat_devicenet_test.c
VAT_Test_WriteOutput(&g_tVatContext, hChannel)
{
    // ⚠️ g_tVatContext 내부 구조체만 업데이트
    pvData = &ptContext->output_asm8;

    // ✅ xChannelIOWrite 호출은 함
    lRet = xChannelIOWrite(hChannel, 0, 0, ulDataLen, pvData, 0);
}
```

**하지만**:
- `g_tVatContext.output_asm8`은 **별도 메모리 공간**
- NetHost가 읽는 것은 `g_tAppData.tOutputData` (일반 모드에서 사용하는 것)

**실제 DeviceNet 통신에 사용되는 구조체**:
```c
// App_DemoApplication.h:74-77
typedef struct APP_PROCESS_DATA_OUTPUT_Ttag
{
  uint8_t output[7];  /* VAT Input Assembly 100: 7 bytes */
} APP_PROCESS_DATA_OUTPUT_T;

// 전역 변수
APP_DATA_T g_tAppData;
  └─ tOutputData.output[7]  ← 이것이 실제 DPM에 매핑됨!
```

**VAT 테스트 구조체 (별도)**:
```c
// vat_devicenet_test.h
typedef struct VAT_OUTPUT_ASSEMBLY_8_Ttag
{
    uint8_t control_mode;
    int16_t control_setpoint;
    uint8_t control_instance;
    uint8_t reserved;
} VAT_OUTPUT_ASSEMBLY_8_T;  // 5 bytes

// 전역 변수
VAT_TEST_CONTEXT_T g_tVatContext;
  └─ output_asm8  ← 이것은 DPM에 매핑 안됨!
```

**결론**:
- `g_tVatContext`에 데이터를 쓰고 있지만
- 실제 DPM(Dual Port Memory)과 연결된 것은 `g_tAppData.tOutputData`
- **두 개가 연결되어 있지 않음!**

---

### 원인 2: Assembly 매핑 불일치

**VAT 테스트 설정**:
```c
// main.c:479에서
g_tVatContext.config.output_assembly = VAT_OUTPUT_ASSEMBLY_8;  // Assembly 8 (5 bytes)
g_tVatContext.config.input_assembly = VAT_INPUT_ASSEMBLY_100;  // Assembly 100 (7 bytes)
```

**일반 모드 설정**:
```c
// App_DemoApplication.h:74-77
APP_PROCESS_DATA_OUTPUT_T  // 7 bytes (Assembly 100?)
APP_PROCESS_DATA_INPUT_T   // 5 bytes (Assembly 8?)
```

**문제**: Assembly 번호와 데이터 크기가 교차되어 있을 가능성

---

## 🛠️ 해결 방법

### 해결책 1: 일반 모드 사용 (추천)

VAT 테스트 모드 대신 **일반 DeviceNet 모드**를 무한 루프로 수정

**장점**:
- ✅ 검증된 통신 경로 사용
- ✅ `g_tAppData`가 실제 DPM과 연결됨
- ✅ 7바이트 증가 패턴이 NetHost에서 확인 가능

**수정 방법**:

#### Step 1: VAT 테스트 비활성화

**파일**: `Core/Src/main.c:122`

```c
// 변경 전
volatile bool g_bEnableVatTest = true;

// 변경 후
volatile bool g_bEnableVatTest = false;  // ← 일반 모드로 전환
```

#### Step 2: 일반 모드 확인

**파일**: `Hil_DemoApp/Sources/App_DemoApplication.c:398-402`

```c
/* OUTPUT DATA */
for (int i = 0; i < 7; i++){
    ptAppData->tOutputData.output[i]++;  // ✅ 이미 1씩 증가 코드 있음
}
```

**결과**:
- ✅ 7바이트가 1씩 증가하며 무한 전송
- ✅ NetHost에서 실시간으로 데이터 변화 확인 가능

---

### 해결책 2: VAT 테스트 데이터를 g_tAppData에 복사

VAT 테스트를 계속 사용하고 싶다면, 데이터를 실제 DPM 영역으로 복사

**수정 위치**: `Core/Src/main.c` case 3 블록

**수정 전**:
```c
while(1)
{
    setpoint++;
    VAT_Test_SetPressureSetpoint(&g_tVatContext, setpoint);

    VAT_Test_WriteOutput(&g_tVatContext, hChannel);  // ← g_tVatContext만 업데이트
    VAT_Test_ReadInput(&g_tVatContext, hChannel);

    HAL_Delay(100);
    cycle_count++;
}
```

**수정 후**:
```c
while(1)
{
    setpoint++;
    VAT_Test_SetPressureSetpoint(&g_tVatContext, setpoint);

    VAT_Test_WriteOutput(&g_tVatContext, hChannel);
    VAT_Test_ReadInput(&g_tVatContext, hChannel);

    /* ✅ VAT 데이터를 실제 DPM 영역으로 복사 */
    // Assembly 8 (5 bytes) → g_tAppData.tInputData (Master → Slave)
    memcpy(&g_tAppData.tInputData.input[0],
           &g_tVatContext.output_asm8,
           sizeof(VAT_OUTPUT_ASSEMBLY_8_T));

    // Assembly 100 (7 bytes) → g_tAppData.tOutputData (Slave → Master)
    g_tAppData.tOutputData.output[0] = g_tVatContext.input_asm100.exception_status;
    g_tAppData.tOutputData.output[1] = (uint8_t)(g_tVatContext.input_asm100.pressure & 0xFF);
    g_tAppData.tOutputData.output[2] = (uint8_t)((g_tVatContext.input_asm100.pressure >> 8) & 0xFF);
    g_tAppData.tOutputData.output[3] = (uint8_t)(g_tVatContext.input_asm100.position & 0xFF);
    g_tAppData.tOutputData.output[4] = (uint8_t)((g_tVatContext.input_asm100.position >> 8) & 0xFF);
    g_tAppData.tOutputData.output[5] = g_tVatContext.input_asm100.device_status;
    g_tAppData.tOutputData.output[6] = g_tVatContext.input_asm100.access_mode;

    HAL_Delay(100);
    cycle_count++;
}
```

**필요한 include**:
```c
// main.c 상단에 추가
#include "App_DemoApplication.h"  // g_tAppData 사용
#include <string.h>                // memcpy 사용
```

---

### 해결책 3: xChannelIOWrite/Read 직접 호출 (가장 간단)

VAT 테스트 구조체 대신 `g_tAppData`를 직접 사용

**수정 위치**: `Core/Src/main.c` case 3 블록

**수정 전**:
```c
while(1)
{
    setpoint++;
    VAT_Test_SetPressureSetpoint(&g_tVatContext, setpoint);

    VAT_Test_WriteOutput(&g_tVatContext, hChannel);
    VAT_Test_ReadInput(&g_tVatContext, hChannel);

    HAL_Delay(100);
    cycle_count++;
}
```

**수정 후**:
```c
while(1)
{
    /* Setpoint 증가 */
    setpoint++;
    if(setpoint > 1000) setpoint = 0;

    /* ✅ g_tAppData에 직접 데이터 설정 (5 bytes, Master → Slave) */
    g_tAppData.tInputData.input[0] = 2;  // Control Mode = Pressure
    g_tAppData.tInputData.input[1] = (uint8_t)(setpoint & 0xFF);
    g_tAppData.tInputData.input[2] = (uint8_t)((setpoint >> 8) & 0xFF);
    g_tAppData.tInputData.input[3] = 0;  // Control Instance
    g_tAppData.tInputData.input[4] = 0;  // Reserved

    /* ✅ 출력 데이터 증가 (7 bytes, Slave → Master) */
    for(int i = 0; i < 7; i++) {
        g_tAppData.tOutputData.output[i]++;
    }

    /* ✅ 실제 DPM에 쓰기 */
    int32_t lRet = xChannelIOWrite(hChannel, 0, 0,
                                    sizeof(g_tAppData.tOutputData),
                                    &g_tAppData.tOutputData, 0);
    if(lRet != CIFX_NO_ERROR)
    {
        printf("[%lu] Write Error: 0x%08X\r\n", cycle_count, (unsigned int)lRet);
    }

    /* ✅ 실제 DPM에서 읽기 */
    lRet = xChannelIORead(hChannel, 0, 0,
                          sizeof(g_tAppData.tInputData),
                          &g_tAppData.tInputData, 0);
    if(lRet != CIFX_NO_ERROR)
    {
        printf("[%lu] Read Error: 0x%08X\r\n", cycle_count, (unsigned int)lRet);
    }

    /* 매 10회마다 상태 출력 */
    if((cycle_count % 10) == 0)
    {
        printf("[%lu] Setpoint=%d, Output=[%02X %02X %02X %02X %02X %02X %02X]\r\n",
               cycle_count,
               setpoint,
               g_tAppData.tOutputData.output[0],
               g_tAppData.tOutputData.output[1],
               g_tAppData.tOutputData.output[2],
               g_tAppData.tOutputData.output[3],
               g_tAppData.tOutputData.output[4],
               g_tAppData.tOutputData.output[5],
               g_tAppData.tOutputData.output[6]);
    }

    HAL_Delay(100);
    cycle_count++;
}
```

**필요한 선언**:
```c
// main.c 상단에 추가
extern APP_DATA_T g_tAppData;  // App_DemoApplication.c에서 정의됨
```

---

## 📊 비교표

| 해결책 | 난이도 | 장점 | 단점 |
|--------|--------|------|------|
| **1. 일반 모드 사용** | ⭐ 쉬움 | 검증된 방법<br>코드 수정 최소 | VAT 테스트 기능 사용 안함 |
| **2. 데이터 복사** | ⭐⭐ 보통 | VAT 테스트 유지<br>로깅 활용 | 코드 복잡<br>메모리 복사 오버헤드 |
| **3. 직접 호출** | ⭐ 쉬움 | 간단명료<br>오버헤드 없음 | VAT 테스트 로깅 사용 안함 |

---

## 🎯 추천 해결 방법

### 가장 간단한 방법: **해결책 1 (일반 모드 사용)**

**이유**:
1. ✅ 코드 1줄만 수정 (`g_bEnableVatTest = false`)
2. ✅ 이미 검증된 통신 경로
3. ✅ 7바이트 증가 패턴이 NetHost에서 확인됨
4. ✅ 무한 루프로 계속 전송

**단계**:

1. **main.c:122 수정**:
   ```c
   volatile bool g_bEnableVatTest = false;
   ```

2. **빌드 및 다운로드**

3. **NetHost 확인**:
   - Process Data Input Image
   - 7바이트가 1씩 증가하는지 확인

---

### 고급 사용자용: **해결책 3 (직접 호출)**

VAT 테스트 모드를 계속 사용하고 싶다면:

1. **main.c 상단에 추가** (Line 100 근처):
   ```c
   extern APP_DATA_T g_tAppData;
   ```

2. **case 3 블록 내부 while(1) 수정** (위의 "해결책 3" 코드 참고)

3. **빌드 및 테스트**

---

## 🔧 적용 후 검증

### STM32 Serial 출력 (예상)

**해결책 1 (일반 모드)**:
```
Running original App_CifXApplicationDemo...
Channel ready!

[매 500ms마다 자동 실행]
Output: [01] [02] [03] [04] [05] [06] [07]
Output: [02] [03] [04] [05] [06] [07] [08]
...
← 계속 증가
```

**해결책 3 (직접 호출)**:
```
Test: INFINITE Continuous Monitoring
Cycles: INFINITE (until reset)

[000] Setpoint=500, Output=[00 01 02 03 04 05 06]
[010] Setpoint=510, Output=[0A 0B 0C 0D 0E 0F 10]
[020] Setpoint=520, Output=[14 15 16 17 18 19 1A]
...
← 계속 증가
```

### NetHost Process Data Input Image (예상)

```
Area Number: 0
Offset: 0
Length: 7
Data: [01] [02] [03] [04] [05] [06] [07]
      ↓ 100ms 후
Data: [02] [03] [04] [05] [06] [07] [08]
      ↓ 100ms 후
Data: [03] [04] [05] [06] [07] [08] [09]
...
← 계속 업데이트됨!
```

---

## 🔍 추가 진단

### 현재 상태 확인

**Live Expressions에서 확인**:
```c
g_tAppData.tOutputData.output[0]  // 이 값이 증가하는가?
g_tAppData.tOutputData.output[1]
...
g_tAppData.tOutputData.output[6]

g_tVatContext.output_asm8.control_setpoint  // 367인가?
```

**예상**:
- `g_tVatContext.output_asm8.control_setpoint` = 367 ✅ (증가 중)
- `g_tAppData.tOutputData.output[0]` = 0 ❌ (변화 없음)

**결론**: 두 구조체가 연결 안됨!

---

## 📝 요약

### 문제 원인
VAT 테스트는 `g_tVatContext` 구조체만 업데이트하고 있으며, 실제 DPM과 연결된 `g_tAppData`를 업데이트하지 않음

### 해결 방법
1. **추천**: `g_bEnableVatTest = false`로 일반 모드 사용
2. **고급**: `g_tAppData`를 직접 업데이트하는 코드 추가

### 다음 단계
1. 해결책 선택
2. 코드 수정
3. 빌드 및 다운로드
4. NetHost에서 데이터 변화 확인

---

**작업 상태**: 원인 분석 완료, 해결책 제시
**작성자**: Claude AI Assistant
**문서 버전**: 1.0

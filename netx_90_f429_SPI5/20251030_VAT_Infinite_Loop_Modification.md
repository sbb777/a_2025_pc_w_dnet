# VAT 테스트 Mode 3 무한 루프 수정 작업

**작성일시**: 2025-01-30
**목적**: VAT 테스트 Mode 3을 100회 제한에서 무한 루프로 변경하여 NetHost에서 계속 poll 데이터 수신 가능하도록 수정
**프로젝트**: STM32 F429 + netX90 DeviceNet Slave
**대상**: VAT Adaptive Pressure Controller (Vendor 404, Product 650)

---

## 📋 문제 상황

### 현재 동작 (종료됨)

**증상**:
```
*** VAT Test completed - keeping channel open for continuous operation ***

========== VAT Test Statistics ==========
Total Read Operations:   100
Total Write Operations:  100
=========================================
```

- ❌ **100회 실행 후 종료**
- ❌ 더 이상 데이터 전송 안함
- ❌ NetHost에서 데이터 수신 중단

### 원인 분석

**파일**: `Core/Src/main.c:482`

```c
/* 연속 모니터링 루프 */
for(int i = 0; i < 100; i++)  // ⚠️ 100회만 실행
{
    VAT_Test_WriteOutput(&g_tVatContext, hChannel);
    VAT_Test_ReadInput(&g_tVatContext, hChannel);
    HAL_Delay(100);
}

printf("\r\nContinuous monitoring completed.\r\n");  // ← 종료
break;  // ← VAT_RunTest() 함수 종료
```

**실행 흐름**:
```
main()
  └─ VAT_RunTest(hChannel)
      └─ case 3:
          ├─ for(i=0; i<100; i++) ✓ (100회 실행)
          ├─ printf("completed") ✓
          └─ break ✓ → VAT_RunTest() 종료

  ← 여기로 돌아옴 (line 789)

  main() while(1) 루프는 계속되지만
  → 더 이상 통신 코드 없음
  → 채널은 열려있으나 데이터 송수신 안함
```

---

## 🛠️ 수정 방법

### 파일: `Core/Src/main.c`

**수정 위치**: Line 473-519 (case 3 블록 내부)

### 변경 전 코드

```c
            printf("Test: Continuous Monitoring\r\n");
            printf("Cycles: 100 (10 seconds @ 100ms)\r\n");
            printf("Setpoint: 750\r\n\r\n");

            /* 압력 제어 모드 설정 */
            VAT_Test_SetControlMode(&g_tVatContext, VAT_CONTROL_MODE_PRESSURE);
            VAT_Test_SetPressureSetpoint(&g_tVatContext, 750);

            /* 연속 모니터링 루프 */
            for(int i = 0; i < 100; i++)
            {
                /* 제어 데이터 전송 */
                int32_t lRet = VAT_Test_WriteOutput(&g_tVatContext, hChannel);
                if(lRet != CIFX_NO_ERROR)
                {
                    printf("[%03d] Write Error: 0x%08X\r\n", i, (unsigned int)lRet);
                }

                /* 센서 데이터 읽기 */
                lRet = VAT_Test_ReadInput(&g_tVatContext, hChannel);
                if(lRet != CIFX_NO_ERROR)
                {
                    printf("[%03d] Read Error: 0x%08X\r\n", i, (unsigned int)lRet);
                }

                /* 예외 상태 확인 */
                if(VAT_Test_HasException(&g_tVatContext))
                {
                    printf("[%03d] WARNING: Exception 0x%02X\r\n",
                           i, VAT_Test_GetExceptionStatus(&g_tVatContext));
                }

                /* 매 10회마다 상태 출력 */
                if((i % 10) == 0 && g_tVatContext.input_data_valid)
                {
                    printf("[%03d] Pressure=%d, Position=%d, Status=0x%02X\r\n",
                           i,
                           g_tVatContext.input_asm100.pressure,
                           g_tVatContext.input_asm100.position,
                           g_tVatContext.input_asm100.device_status);
                }

                /* 100ms 대기 */
                HAL_Delay(100);
            }

            printf("\r\nContinuous monitoring completed.\r\n");
```

### 변경 후 코드 (목표)

```c
            printf("Test: INFINITE Continuous Monitoring\r\n");
            printf("Cycles: INFINITE (until reset)\r\n");
            printf("Update Rate: 100ms\r\n");
            printf("Press RESET button to stop.\r\n\r\n");

            /* 압력 제어 모드 설정 */
            VAT_Test_SetControlMode(&g_tVatContext, VAT_CONTROL_MODE_PRESSURE);

            /* 초기 Setpoint */
            int16_t setpoint = 500;
            VAT_Test_SetPressureSetpoint(&g_tVatContext, setpoint);

            /* 무한 연속 모니터링 루프 */
            uint32_t cycle_count = 0;
            while(1)
            {
                /* Setpoint 증가 */
                setpoint++;
                if(setpoint > 1000) setpoint = 0;
                VAT_Test_SetPressureSetpoint(&g_tVatContext, setpoint);

                /* 제어 데이터 전송 */
                int32_t lRet = VAT_Test_WriteOutput(&g_tVatContext, hChannel);
                if(lRet != CIFX_NO_ERROR)
                {
                    printf("[%lu] Write Error: 0x%08X\r\n", cycle_count, (unsigned int)lRet);
                }

                /* 센서 데이터 읽기 */
                lRet = VAT_Test_ReadInput(&g_tVatContext, hChannel);
                if(lRet != CIFX_NO_ERROR)
                {
                    printf("[%lu] Read Error: 0x%08X\r\n", cycle_count, (unsigned int)lRet);
                }

                /* 예외 상태 확인 */
                if(VAT_Test_HasException(&g_tVatContext))
                {
                    printf("[%lu] WARNING: Exception 0x%02X\r\n",
                           cycle_count, VAT_Test_GetExceptionStatus(&g_tVatContext));
                }

                /* 매 10회마다 상태 출력 */
                if((cycle_count % 10) == 0 && g_tVatContext.input_data_valid)
                {
                    printf("[%lu] Setpoint=%d, Pressure=%d, Position=%d, Status=0x%02X\r\n",
                           cycle_count,
                           setpoint,
                           g_tVatContext.input_asm100.pressure,
                           g_tVatContext.input_asm100.position,
                           g_tVatContext.input_asm100.device_status);
                }

                /* 100ms 대기 */
                HAL_Delay(100);

                cycle_count++;

                /* 1000회마다 진행 상황 출력 */
                if((cycle_count % 1000) == 0)
                {
                    printf("\r\n*** Running for %lu cycles (%.1f minutes) ***\r\n",
                           cycle_count, (float)cycle_count / 600.0f);
                }
            }

            /* 이 코드는 실행되지 않음 (무한 루프) */
```

---

## 📝 주요 변경 사항

### 1. 루프 타입 변경
```c
// 변경 전
for(int i = 0; i < 100; i++)

// 변경 후
uint32_t cycle_count = 0;
while(1)
{
    // ...
    cycle_count++;
}
```

**효과**: 100회 제한 제거 → 무한 실행

### 2. 데이터 증가 패턴 추가
```c
/* Setpoint 증가 */
setpoint++;
if(setpoint > 1000) setpoint = 0;
VAT_Test_SetPressureSetpoint(&g_tVatContext, setpoint);
```

**효과**: 기존 일반 모드처럼 데이터가 1씩 증가하며 전송

### 3. printf 포맷 변경
```c
// 변경 전
printf("[%03d] ...", i, ...);

// 변경 후
printf("[%lu] ...", cycle_count, ...);
printf("[%lu] Setpoint=%d, ...", cycle_count, setpoint, ...);
```

**효과**:
- `%03d` → `%lu`: uint32_t 지원 (더 큰 범위)
- Setpoint 값도 출력

### 4. 진행 상황 출력 추가
```c
/* 1000회마다 진행 상황 출력 */
if((cycle_count % 1000) == 0)
{
    printf("\r\n*** Running for %lu cycles (%.1f minutes) ***\r\n",
           cycle_count, (float)cycle_count / 600.0f);
}
```

**효과**: 100초(1.7분)마다 실행 시간 출력

### 5. 종료 메시지 제거
```c
// 제거됨
// printf("\r\nContinuous monitoring completed.\r\n");
```

**효과**: 무한 루프이므로 종료 메시지 불필요

---

## 🔧 수정 단계

### Step 1: STM32CubeIDE에서 파일 열기

1. **STM32CubeIDE 실행**
2. **Project Explorer → Core → Src → main.c 더블클릭**

### Step 2: 수정 위치로 이동

1. **Ctrl+L 누르기** (Go to Line)
2. **473 입력** → OK

### Step 3: 코드 선택 및 삭제

1. **Line 473부터 Line 519까지 선택**
   - Line 473 클릭
   - Shift+Down 키로 Line 519까지 선택

2. **Delete 키**

### Step 4: 새 코드 붙여넣기

**아래 코드 복사 (Ctrl+C)**:

```c
            printf("Test: INFINITE Continuous Monitoring\r\n");
            printf("Cycles: INFINITE (until reset)\r\n");
            printf("Update Rate: 100ms\r\n");
            printf("Press RESET button to stop.\r\n\r\n");

            /* 압력 제어 모드 설정 */
            VAT_Test_SetControlMode(&g_tVatContext, VAT_CONTROL_MODE_PRESSURE);

            /* 초기 Setpoint */
            int16_t setpoint = 500;
            VAT_Test_SetPressureSetpoint(&g_tVatContext, setpoint);

            /* 무한 연속 모니터링 루프 */
            uint32_t cycle_count = 0;
            while(1)
            {
                /* Setpoint 증가 */
                setpoint++;
                if(setpoint > 1000) setpoint = 0;
                VAT_Test_SetPressureSetpoint(&g_tVatContext, setpoint);

                /* 제어 데이터 전송 */
                int32_t lRet = VAT_Test_WriteOutput(&g_tVatContext, hChannel);
                if(lRet != CIFX_NO_ERROR)
                {
                    printf("[%lu] Write Error: 0x%08X\r\n", cycle_count, (unsigned int)lRet);
                }

                /* 센서 데이터 읽기 */
                lRet = VAT_Test_ReadInput(&g_tVatContext, hChannel);
                if(lRet != CIFX_NO_ERROR)
                {
                    printf("[%lu] Read Error: 0x%08X\r\n", cycle_count, (unsigned int)lRet);
                }

                /* 예외 상태 확인 */
                if(VAT_Test_HasException(&g_tVatContext))
                {
                    printf("[%lu] WARNING: Exception 0x%02X\r\n",
                           cycle_count, VAT_Test_GetExceptionStatus(&g_tVatContext));
                }

                /* 매 10회마다 상태 출력 */
                if((cycle_count % 10) == 0 && g_tVatContext.input_data_valid)
                {
                    printf("[%lu] Setpoint=%d, Pressure=%d, Position=%d, Status=0x%02X\r\n",
                           cycle_count,
                           setpoint,
                           g_tVatContext.input_asm100.pressure,
                           g_tVatContext.input_asm100.position,
                           g_tVatContext.input_asm100.device_status);
                }

                /* 100ms 대기 */
                HAL_Delay(100);

                cycle_count++;

                /* 1000회마다 진행 상황 출력 */
                if((cycle_count % 1000) == 0)
                {
                    printf("\r\n*** Running for %lu cycles (%.1f minutes) ***\r\n",
                           cycle_count, (float)cycle_count / 600.0f);
                }
            }

            /* 이 코드는 실행되지 않음 (무한 루프) */
```

**STM32CubeIDE에서 붙여넣기 (Ctrl+V)**

### Step 5: 저장 및 빌드

1. **저장**: Ctrl+S
2. **빌드**: Project → Build All (또는 Ctrl+B)
3. **오류 확인**: Console 창에서 "0 errors" 확인

### Step 6: 다운로드 및 실행

1. **Run → Debug (F11)**
2. **또는 Run → Run (Ctrl+F11)**

---

## ✅ 수정 후 예상 동작

### STM32 Serial 출력

```
========================================
 Running VAT Test Mode: 3
========================================

Test: INFINITE Continuous Monitoring
Cycles: INFINITE (until reset)
Update Rate: 100ms
Press RESET button to stop.

[000] Setpoint=500, Pressure=0, Position=0, Status=0x00
[010] Setpoint=510, Pressure=0, Position=0, Status=0x00
[020] Setpoint=520, Pressure=0, Position=0, Status=0x00
[030] Setpoint=530, Pressure=0, Position=0, Status=0x00
...
[100] Setpoint=600, Pressure=0, Position=0, Status=0x00
...
[1000] Setpoint=1500 → 500, Pressure=0, Position=0, Status=0x00

*** Running for 1000 cycles (1.7 minutes) ***

[1010] Setpoint=510, Pressure=0, Position=0, Status=0x00
...

← 계속 실행됨! 종료 안됨!
```

### NetHost Process Data Input Image

**실시간 업데이트**:
```
Time: 00:00:00  Data: [00] [F4] [01] [00] [00] [00] [00]  (Setpoint=500)
Time: 00:00:01  Data: [00] [FE] [01] [00] [00] [00] [00]  (Setpoint=510)
Time: 00:00:02  Data: [00] [08] [02] [00] [00] [00] [00]  (Setpoint=520)
...
Time: 00:01:00  Data: [00] [58] [02] [00] [00] [00] [00]  (Setpoint=600)
...

→ 계속 업데이트됨! 종료 안됨!
```

**Cyclic 체크박스 활성화 시**: 자동 갱신

---

## 🔍 Live Expressions 모니터링

### STM32CubeIDE Live Expressions 창

**추가할 변수**:
```c
cycle_count                              // 실행 횟수
setpoint                                 // 현재 Setpoint 값
g_tVatContext.input_asm100.pressure      // 수신된 압력
g_tVatContext.input_asm100.position      // 수신된 위치
g_tVatContext.input_asm100.device_status // 디바이스 상태
g_tVatContext.stats.total_write_count    // 총 전송 횟수
g_tVatContext.stats.total_read_count     // 총 수신 횟수
```

**예상 값**:
```
cycle_count: 0 → 1 → 2 → 3 → ... → 계속 증가
setpoint: 500 → 501 → 502 → ... → 1000 → 0 → 1 → ...
total_write_count: 0 → 1 → 2 → ... → 계속 증가
total_read_count: 0 → 1 → 2 → ... → 계속 증가
```

---

## 📊 기존 일반 모드와 비교

### 일반 DeviceNet 모드 (g_bEnableVatTest = false)

**파일**: `Hil_DemoApp/Sources/App_DemoApplication.c:398-402`

```c
/* OUTPUT DATA */
for (int i = 0; i < 7; i++){
    ptAppData->tOutputData.output[i]++;  // ✅ 1씩 증가
}
```

**특징**:
- ✅ App_IODataHandler()가 500ms마다 무한 호출
- ✅ 7바이트 데이터 1씩 증가
- ✅ 종료 없음

### VAT 테스트 모드 (수정 후)

**파일**: `Core/Src/main.c:471-540`

```c
while(1)
{
    setpoint++;  // ✅ 1씩 증가
    if(setpoint > 1000) setpoint = 0;
    VAT_Test_SetPressureSetpoint(&g_tVatContext, setpoint);

    VAT_Test_WriteOutput(&g_tVatContext, hChannel);
    VAT_Test_ReadInput(&g_tVatContext, hChannel);

    HAL_Delay(100);
    cycle_count++;
}
```

**특징**:
- ✅ while(1) 무한 루프
- ✅ Setpoint 1씩 증가 (0~1000 범위)
- ✅ 종료 없음
- ✅ 100ms 주기 (일반 모드: 500ms)

**동일한 동작**: 종료 없이 계속 데이터 전송!

---

## 🛑 중단 방법

### 방법 1: RESET 버튼

**STM32 보드의 RESET 버튼 누르기**

### 방법 2: 디버거 종료

**STM32CubeIDE**:
- Terminate 버튼 (빨간 사각형)
- 또는 Ctrl+F2

### 방법 3: 코드 재다운로드

**다른 펌웨어 다운로드**

---

## 📚 참고 자료

- **20251029_10Bytes_Increment_Code_Analysis.md**: 기존 10바이트 증가 코드 분석
- **20251029_DeviceNet_LiveMonitoring_Guide.md**: Live Expressions 모니터링 가이드
- **20251028_DeviceNet_10min_Test_Duration_Modification.md**: 테스트 시간 수정 이력

---

**작업 상태**: 코드 준비 완료, 수동 적용 필요
**작성자**: Claude AI Assistant
**문서 버전**: 1.0
**적용 방법**: STM32CubeIDE에서 직접 복사/붙여넣기


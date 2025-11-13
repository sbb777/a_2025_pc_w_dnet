# VAT DeviceNet 간단한 I/O 테스트 예제 (netHost)

## 📋 개요

가장 간단한 DeviceNet I/O 테스트 예제입니다.
- Flash 사용 없음 (메모리만 사용)
- 기본 Assembly만 사용 (Input 100 / Output 8)
- netHost로 즉시 테스트 가능

---

## 🔄 데이터 흐름

### Master → Slave (Output Assembly 8)
```
┌─────────────────────────────────────────┐
│ Master (netHost)가 보내는 데이터         │
├─────────────────────────────────────────┤
│ Byte 0: Control Mode     (1 byte)       │  예: 5 (Pressure Control)
│ Byte 1-2: Control Setpoint (INT16)      │  예: 5000 (Little-endian: 0x88 0x13)
│ Byte 3: Control Instance (1 byte)       │  예: 1
│ Byte 4: Reserved        (1 byte)        │  예: 0
├─────────────────────────────────────────┤
│ Total: 5 bytes                          │
└─────────────────────────────────────────┘
```

### Slave → Master (Input Assembly 100)
```
┌─────────────────────────────────────────┐
│ Slave (VAT Device)가 보내는 데이터       │
├─────────────────────────────────────────┤
│ Byte 0: Exception Status (1 byte)       │  예: 0 (Normal)
│ Byte 1-2: Pressure      (INT16)         │  예: 4850 (0xF2 0x12)
│ Byte 3-4: Position      (INT16)         │  예: 16383 (0xFF 0x3F)
│ Byte 5: Device Status   (1 byte)        │  예: 2 (Idle)
│ Byte 6: Access Mode     (1 byte)        │  예: 0 (Local)
├─────────────────────────────────────────┤
│ Total: 7 bytes                          │
└─────────────────────────────────────────┘
```

---

## 💻 최소 구현 코드

### 1. 전역 변수 선언

**main.c 또는 App_DemoApplication.c**:

```c
#include "App_VAT_Assemblies.h"
#include "App_VAT_IoHandler.h"

/* Assembly Manager */
ASSEMBLY_MANAGER_T g_tAssemblyManager;

/* I/O 데이터 (전역 변수) */
// Slave → Master (Input)
int16_t  g_current_pressure = 4850;     // 현재 압력 (예: 4850 counts)
int16_t  g_current_position = 16383;    // 현재 위치 (예: 50%, 16383 = 32767 * 0.5)
uint8_t  g_exception_status = 0;        // 예외 상태 (0 = Normal)
uint8_t  g_device_status = 2;           // 디바이스 상태 (2 = Idle)
uint8_t  g_access_mode = 0;             // 액세스 모드 (0 = Local)
uint8_t  g_discrete_inputs = 0;         // 디스크리트 입력

// Master → Slave (Output)
uint8_t  g_control_mode = 0;            // 제어 모드 (Master가 설정)
int16_t  g_control_setpoint = 0;        // 제어 설정값 (Master가 설정)
uint8_t  g_control_instance = 0;        // 제어 인스턴스
```

### 2. 초기화 함수

```c
void VAT_Simple_Init(void)
{
    /* Assembly Manager 초기화 */
    VAT_Assembly_Init(&g_tAssemblyManager);

    /* 기본 Assembly 설정 */
    g_tAssemblyManager.active_input_instance = 100;   // Input Assembly 100
    g_tAssemblyManager.active_output_instance = 8;    // Output Assembly 8

    /* 초기 데이터 설정 */
    g_current_pressure = 4850;      // 압력: 4850 counts
    g_current_position = 16383;     // 위치: 50%
    g_exception_status = 0;         // 정상
    g_device_status = 2;            // Idle
    g_access_mode = 0;              // Local

    g_control_mode = 0;             // 제어 모드: Init
    g_control_setpoint = 0;         // 설정값: 0
}
```

### 3. I/O 업데이트 함수 (주기적 호출)

```c
void VAT_Simple_UpdateIO(void)
{
    /* ======================================
     * 1. Input Assembly 업데이트 (Slave → Master)
     * ====================================== */

    INPUT_ASSEMBLY_100_T* pInput = (INPUT_ASSEMBLY_100_T*)&g_tAssemblyManager.input_buffers[0].data;

    pInput->exception_status = g_exception_status;
    pInput->pressure = g_current_pressure;
    pInput->position = g_current_position;
    pInput->device_status = g_device_status;
    pInput->access_mode = g_access_mode;

    /* ======================================
     * 2. Output Assembly 처리 (Master → Slave)
     * ====================================== */

    OUTPUT_ASSEMBLY_8_T* pOutput = (OUTPUT_ASSEMBLY_8_T*)&g_tAssemblyManager.output_buffers[0].data;

    g_control_mode = pOutput->control_mode;
    g_control_setpoint = pOutput->control_setpoint;
    g_control_instance = pOutput->control_instance;

    /* ======================================
     * 3. 간단한 시뮬레이션 (선택 사항)
     * ====================================== */

    // Master가 설정한 setpoint로 압력 서서히 변경
    if (g_control_mode == 5) {  // Pressure Control Mode
        if (g_current_pressure < g_control_setpoint) {
            g_current_pressure += 10;  // 천천히 증가
        } else if (g_current_pressure > g_control_setpoint) {
            g_current_pressure -= 10;  // 천천히 감소
        }
    }
}
```

### 4. DeviceNet 스택 통합

**AppDNS_DemoApplicationFunctions.c 수정**:

```c
#include "App_VAT_Assemblies.h"

extern ASSEMBLY_MANAGER_T g_tAssemblyManager;

/* DeviceNet Cyclic I/O Callback */
void AppDNS_CyclicCallback(void)
{
    /* 이 함수는 DeviceNet 스택이 주기적으로 호출 (예: 10ms마다) */
    VAT_Simple_UpdateIO();
}
```

---

## 🧪 netHost 테스트 절차

### 1. netHost 설정

#### 1.1 프로젝트 생성
```
1. netHost 실행
2. File → New Project
3. DeviceNet 네트워크 선택
4. Master 추가
```

#### 1.2 EDS 파일 로드
```
1. Device → Add Device
2. EDS 파일 선택: 476297.eds
3. Node Address 설정: 1
4. I/O Connection 설정:
   - Input Assembly: 100 (7 bytes)
   - Output Assembly: 8 (5 bytes)
   - RPI (Requested Packet Interval): 20 ms
```

### 2. 데이터 송수신 테스트

#### 2.1 Output Data 보내기 (Master → Slave)

**netHost 화면**:
```
Output Data View:
┌─────────────────────────────────────────┐
│ Byte 0: [05]  ← Control Mode = 5       │
│ Byte 1: [88]  ← Setpoint Low = 0x88    │
│ Byte 2: [13]  ← Setpoint High = 0x13   │
│ Byte 3: [01]  ← Control Instance = 1   │
│ Byte 4: [00]  ← Reserved = 0           │
└─────────────────────────────────────────┘
```

**16진수 입력**:
```
05 88 13 01 00
```

**해석**:
- Control Mode: 5 (Pressure Control)
- Control Setpoint: 0x1388 = 5000 (Little-endian)
- Control Instance: 1

#### 2.2 Input Data 받기 (Slave → Master)

**netHost 화면**:
```
Input Data View:
┌─────────────────────────────────────────┐
│ Byte 0: [00]  ← Exception Status = 0   │
│ Byte 1: [F2]  ← Pressure Low = 0xF2    │
│ Byte 2: [12]  ← Pressure High = 0x12   │
│ Byte 3: [FF]  ← Position Low = 0xFF    │
│ Byte 4: [3F]  ← Position High = 0x3F   │
│ Byte 5: [02]  ← Device Status = 2      │
│ Byte 6: [00]  ← Access Mode = 0        │
└─────────────────────────────────────────┘
```

**16진수 출력 예**:
```
00 F2 12 FF 3F 02 00
```

**해석**:
- Exception Status: 0 (Normal)
- Pressure: 0x12F2 = 4850 counts
- Position: 0x3FFF = 16383 (50%)
- Device Status: 2 (Idle)
- Access Mode: 0 (Local)

### 3. 실시간 모니터링

**netHost 모니터 창**:
```
Time     | Output (Master→Slave)    | Input (Slave→Master)
---------|--------------------------|-------------------------
00:00.00 | 05 88 13 01 00          | 00 F2 12 FF 3F 02 00
00:00.02 | 05 88 13 01 00          | 00 FC 12 FF 3F 02 00  ← Pressure 증가
00:00.04 | 05 88 13 01 00          | 00 06 13 FF 3F 02 00  ← Pressure 계속 증가
00:00.06 | 05 88 13 01 00          | 00 10 13 FF 3F 02 00
...
00:01.00 | 05 88 13 01 00          | 00 88 13 FF 3F 02 00  ← Setpoint 도달!
```

---

## 📊 테스트 시나리오

### 시나리오 1: 압력 제어 테스트

**단계**:
1. Master가 Setpoint 5000으로 설정
   - Output: `05 88 13 01 00`
2. Slave가 현재 압력 4850 보고
   - Input: `00 F2 12 FF 3F 02 00`
3. Slave가 압력을 5000으로 증가
   - Input: `00 88 13 FF 3F 02 00` (도달!)

### 시나리오 2: 제어 모드 변경

**단계**:
1. Master가 Open 모드로 변경 (Mode = 4)
   - Output: `04 00 00 01 00`
2. Slave가 Position 증가
   - Input: `00 88 13 00 7F 02 00` (Position 증가)

### 시나리오 3: 에러 시뮬레이션

**단계**:
1. 펌웨어에서 에러 발생:
   ```c
   g_exception_status = 0x10;  // Warning
   g_device_status = 6;        // Critical Fault
   ```
2. Master가 Input Data 수신:
   - Input: `10 88 13 FF 3F 06 00`
   - Exception Status: 0x10 (Warning)
   - Device Status: 6 (Critical Fault)

---

## 🔧 디버그 출력 추가

### printf로 I/O 데이터 확인

```c
void VAT_Simple_UpdateIO(void)
{
    static uint32_t counter = 0;

    /* Input 업데이트 */
    INPUT_ASSEMBLY_100_T* pInput = (INPUT_ASSEMBLY_100_T*)&g_tAssemblyManager.input_buffers[0].data;
    pInput->exception_status = g_exception_status;
    pInput->pressure = g_current_pressure;
    pInput->position = g_current_position;
    pInput->device_status = g_device_status;
    pInput->access_mode = g_access_mode;

    /* Output 처리 */
    OUTPUT_ASSEMBLY_8_T* pOutput = (OUTPUT_ASSEMBLY_8_T*)&g_tAssemblyManager.output_buffers[0].data;
    g_control_mode = pOutput->control_mode;
    g_control_setpoint = pOutput->control_setpoint;
    g_control_instance = pOutput->control_instance;

    /* 1초마다 출력 */
    if (++counter % 100 == 0) {
        printf("\n=== I/O Data (1 sec) ===\n");

        printf("Output (Master→Slave):\n");
        printf("  Control Mode: %d\n", g_control_mode);
        printf("  Control Setpoint: %d\n", g_control_setpoint);
        printf("  Control Instance: %d\n", g_control_instance);

        printf("Input (Slave→Master):\n");
        printf("  Exception Status: 0x%02X\n", g_exception_status);
        printf("  Pressure: %d\n", g_current_pressure);
        printf("  Position: %d\n", g_current_position);
        printf("  Device Status: %d\n", g_device_status);
        printf("  Access Mode: %d\n", g_access_mode);
    }

    /* 압력 시뮬레이션 */
    if (g_control_mode == 5) {  // Pressure Control
        if (g_current_pressure < g_control_setpoint) {
            g_current_pressure += 10;
        } else if (g_current_pressure > g_control_setpoint) {
            g_current_pressure -= 10;
        }
    }
}
```

**예상 출력**:
```
=== I/O Data (1 sec) ===
Output (Master→Slave):
  Control Mode: 5
  Control Setpoint: 5000
  Control Instance: 1
Input (Slave→Master):
  Exception Status: 0x00
  Pressure: 4850
  Position: 16383
  Device Status: 2
  Access Mode: 0
```

---

## ✅ 빠른 테스트 체크리스트

### 기본 동작 확인 (5분)
- [ ] netHost 연결 성공
- [ ] Output Data 전송 확인
- [ ] Input Data 수신 확인
- [ ] LED 깜빡임 확인 (통신 중)

### 데이터 정합성 확인 (10분)
- [ ] Master가 보낸 Output Data가 전역 변수에 반영됨
- [ ] 전역 변수 값이 Input Data로 전송됨
- [ ] Little-endian byte order 확인
- [ ] 실시간 데이터 업데이트 확인

### 시뮬레이션 확인 (10분)
- [ ] Setpoint 5000 설정 → 압력이 5000으로 수렴
- [ ] Control Mode 변경 → 동작 변화 확인
- [ ] 에러 발생 → Exception Status 전송 확인

---

## 📈 데이터 값 참고표

### Control Mode (Byte 0 of Output)
```
0 = Init
1 = Sync
2 = Position Control
3 = Close
4 = Open
5 = Pressure Control  ← 일반적으로 사용
6 = Hold
```

### Device Status (Byte 5 of Input)
```
0 = Undefined
1 = Self Testing
2 = Idle             ← 정상 대기
3 = Self Test Exception
4 = Executing        ← 제어 중
5 = Abort
6 = Critical Fault   ← 심각한 오류
```

### Exception Status (Byte 0 of Input)
```
0x00 = Normal        ← 정상
0x01 = Minor Alarm
0x02 = Major Alarm
0x04 = Critical Alarm
0x08 = Warning
0x10 = Status
```

### Pressure/Position 변환
```
Pressure (INT16):
  -32768 ~ 32767 counts
  예: 0 = 최소, 32767 = 최대

Position (INT16):
  0 ~ 32767 counts
  예: 0 = 0%, 16383 = 50%, 32767 = 100%
```

---

## 🎯 요약

### 최소 필요 파일
1. ✅ `App_VAT_Assemblies.h` - 구조체 정의
2. ✅ `App_VAT_AssemblySelector.c` - 초기화
3. ✅ 전역 변수 (9개)
4. ✅ 초기화 함수 (VAT_Simple_Init)
5. ✅ I/O 업데이트 함수 (VAT_Simple_UpdateIO)

### 데이터 흐름
```
Master (netHost)
    ↓ Output (5 bytes)
전역 변수 (g_control_mode, g_control_setpoint)
    ↓ 제어 로직 (시뮬레이션)
전역 변수 (g_current_pressure, g_current_position)
    ↓ Input (7 bytes)
Master (netHost)
```

### 테스트 소요 시간
- 설정: 10분
- 기본 테스트: 5분
- 시뮬레이션: 10분
- **총**: 25분

---

**작성일**: 2025-11-05
**버전**: 1.0

# VAT EDS 실제 데이터 시뮬레이션 구현 가이드

**작성일**: 2025-10-30
**프로젝트**: netX90 + STM32F429 VAT DeviceNet Integration
**목적**: VAT Adaptive Pressure Controller의 실제 동작을 시뮬레이션하는 의미 있는 데이터 송수신 구현

---

## 1. 개요

현재 Mode 3 무한 루프는 단순히 데이터를 증가시키는 테스트 패턴을 사용합니다.
이를 VAT 압력 제어기의 실제 동작을 시뮬레이션하는 의미 있는 데이터로 변경합니다.

### VAT Pressure Controller 동작 원리

1. **Master → Slave (Output Assembly 8)**: 제어 모드 및 설정값 전송
   - `control_mode`: CLOSED/OPEN/PRESSURE/POSITION 등
   - `control_setpoint`: 목표 압력 또는 위치 값

2. **Slave → Master (Input Assembly 100)**: 센서 데이터 및 상태 전송
   - `pressure`: 현재 압력 측정값 (mbar 단위)
   - `position`: 밸브 위치 (0-1000 = 0-100%)
   - `device_status`: 밸브 상태 플래그
   - `exception_status`: 예외/경고 플래그

3. **제어 루프**:
   - Master가 설정값(setpoint) 전송
   - Slave가 현재값(pressure/position)을 설정값에 근접하도록 조정
   - 실제 물리적 지연을 시뮬레이션 (ramping)

---

## 2. VAT EDS 데이터 구조 복습

### Input Assembly 100 (Slave → Master, 7 bytes)

```c
typedef struct VAT_INPUT_ASSEMBLY_100_Ttag {
    uint8_t exception_status;  // [0] 예외 상태 플래그
    int16_t pressure;          // [1-2] 압력 (mbar, -1000 ~ +1000)
    int16_t position;          // [3-4] 밸브 위치 (0-1000 = 0-100%)
    uint8_t device_status;     // [5] 장치 상태
    uint8_t access_mode;       // [6] 액세스 모드
} VAT_INPUT_ASSEMBLY_100_T;
```

**Exception Status Bits** (offset 0):
- Bit 0: `COMM_ERROR` - 통신 오류
- Bit 1: `SENSOR_ERROR` - 센서 오류
- Bit 2: `OVERPRESSURE` - 과압
- Bit 3: `MOTOR_ERROR` - 모터 오류
- Bit 4-7: Reserved

**Device Status Bits** (offset 5):
- Bit 0: `VALVE_OPEN` - 밸브 완전 열림
- Bit 1: `VALVE_CLOSED` - 밸브 완전 닫힘
- Bit 2: `MOVING` - 밸브 이동 중
- Bit 3: `AT_SETPOINT` - 설정값 도달
- Bit 4: `MANUAL_MODE` - 수동 모드
- Bit 5-7: Reserved

### Output Assembly 8 (Master → Slave, 5 bytes)

```c
typedef struct VAT_OUTPUT_ASSEMBLY_8_Ttag {
    uint8_t control_mode;      // [0] 제어 모드
    int16_t control_setpoint;  // [1-2] 설정값
    uint16_t control_instance; // [3-4] 제어 인스턴스
} VAT_OUTPUT_ASSEMBLY_8_T;
```

**Control Modes**:
- `0x00`: CLOSED - 완전 닫힘
- `0x01`: OPEN - 완전 열림
- `0x02`: PRESSURE - 압력 제어
- `0x03`: POSITION - 위치 제어
- `0x04`: MANUAL - 수동 모드
- `0x05`: THROTTLE - 스로틀 제어

---

## 3. 실제 데이터 시뮬레이션 알고리즘

### 3.1 압력 제어 모드 (PRESSURE) 시뮬레이션

```c
/* 시뮬레이션 상태 변수 */
typedef struct {
    int16_t current_pressure;   // 현재 압력 (mbar)
    int16_t current_position;   // 현재 위치 (0-1000)
    int16_t target_pressure;    // 목표 압력
    uint8_t device_status;      // 장치 상태
    uint8_t exception_status;   // 예외 상태
    bool is_moving;             // 이동 중 플래그
} VAT_SIM_STATE_T;

/* 압력 제어 시뮬레이션 */
void VAT_Simulate_Pressure_Control(VAT_SIM_STATE_T *pState, int16_t setpoint)
{
    int16_t pressure_error = setpoint - pState->current_pressure;

    /* 압력 오차가 ±10 mbar 이내면 설정값 도달 */
    if (abs(pressure_error) <= 10) {
        pState->device_status |= 0x08;   // AT_SETPOINT
        pState->device_status &= ~0x04;  // MOVING 해제
        pState->is_moving = false;
        return;
    }

    /* 압력 오차에 비례하여 밸브 위치 조정 (P 제어) */
    pState->is_moving = true;
    pState->device_status |= 0x04;   // MOVING
    pState->device_status &= ~0x08;  // AT_SETPOINT 해제

    /* 밸브 위치 조정 (매 사이클마다 작은 단위로 이동) */
    if (pressure_error > 0) {
        pState->current_position += 5;  // 열림 방향
        if (pState->current_position > 1000) pState->current_position = 1000;
    } else {
        pState->current_position -= 5;  // 닫힘 방향
        if (pState->current_position < 0) pState->current_position = 0;
    }

    /* 밸브 위치에 따른 압력 변화 시뮬레이션 */
    /* 위치가 클수록 압력 상승 (간단한 선형 모델) */
    pState->current_pressure = (pState->current_position * setpoint) / 1000;

    /* 밸브 완전 열림/닫힘 상태 갱신 */
    if (pState->current_position >= 1000) {
        pState->device_status |= 0x01;   // VALVE_OPEN
    } else {
        pState->device_status &= ~0x01;
    }

    if (pState->current_position <= 0) {
        pState->device_status |= 0x02;   // VALVE_CLOSED
    } else {
        pState->device_status &= ~0x02;
    }
}
```

### 3.2 위치 제어 모드 (POSITION) 시뮬레이션

```c
void VAT_Simulate_Position_Control(VAT_SIM_STATE_T *pState, int16_t setpoint)
{
    int16_t position_error = setpoint - pState->current_position;

    /* 위치 오차가 ±5 이내면 설정값 도달 */
    if (abs(position_error) <= 5) {
        pState->device_status |= 0x08;   // AT_SETPOINT
        pState->device_status &= ~0x04;  // MOVING 해제
        pState->is_moving = false;
        return;
    }

    /* 위치로 이동 */
    pState->is_moving = true;
    pState->device_status |= 0x04;   // MOVING
    pState->device_status &= ~0x08;  // AT_SETPOINT 해제

    /* 매 사이클마다 10씩 이동 */
    if (position_error > 0) {
        pState->current_position += 10;
        if (pState->current_position > 1000) pState->current_position = 1000;
    } else {
        pState->current_position -= 10;
        if (pState->current_position < 0) pState->current_position = 0;
    }

    /* 위치에 따른 압력 변화 (예: 최대 압력 1000 mbar 가정) */
    pState->current_pressure = (pState->current_position * 1000) / 1000;

    /* 밸브 완전 열림/닫힘 상태 갱신 */
    if (pState->current_position >= 1000) {
        pState->device_status |= 0x01;   // VALVE_OPEN
    } else {
        pState->device_status &= ~0x01;
    }

    if (pState->current_position <= 0) {
        pState->device_status |= 0x02;   // VALVE_CLOSED
    } else {
        pState->device_status &= ~0x02;
    }
}
```

### 3.3 OPEN/CLOSED 모드 시뮬레이션

```c
void VAT_Simulate_Open_Closed(VAT_SIM_STATE_T *pState, uint8_t mode)
{
    int16_t target_position = (mode == VAT_CONTROL_MODE_OPEN) ? 1000 : 0;
    VAT_Simulate_Position_Control(pState, target_position);
}
```

---

## 4. main.c 수정 코드

### 4.1 시뮬레이션 상태 변수 추가

`Core/Src/main.c` 파일 상단 전역 변수 섹션에 추가:

```c
/* VAT 시뮬레이션 상태 */
typedef struct {
    int16_t current_pressure;
    int16_t current_position;
    uint8_t device_status;
    uint8_t exception_status;
} VAT_SIM_STATE_T;

static VAT_SIM_STATE_T g_tVatSimState = {0};
```

### 4.2 Mode 3 무한 루프 수정

`Core/Src/main.c`의 `case 3:` 블록을 다음과 같이 수정:

```c
case 3:  /* VAT Real Data Simulation - INFINITE */
{
    printf("Test: VAT Real Data Simulation (INFINITE)\r\n");
    printf("Simulating VAT Adaptive Pressure Controller\r\n");
    printf("Press RESET to stop.\r\n\r\n");

    /* 시뮬레이션 상태 초기화 */
    memset(&g_tVatSimState, 0, sizeof(g_tVatSimState));
    g_tVatSimState.current_pressure = 0;
    g_tVatSimState.current_position = 0;
    g_tVatSimState.device_status = 0x02;  // VALVE_CLOSED
    g_tVatSimState.exception_status = 0x00;

    uint32_t cycle_count = 0;

    /* 무한 제어 루프 */
    while(1)
    {
        /* === Master → Slave: 제어 명령 읽기 === */
        int32_t lRet = xChannelIORead(hChannel, 0, 0,
                                       sizeof(g_tAppData.tInputData),
                                       &g_tAppData.tInputData, 0);
        if(lRet != CIFX_NO_ERROR) {
            printf("[%lu] Read Error: 0x%08X\r\n", cycle_count, (unsigned int)lRet);
        }

        /* Master로부터 받은 제어 명령 파싱 */
        uint8_t control_mode = g_tAppData.tInputData.input[0];
        int16_t control_setpoint = (int16_t)(g_tAppData.tInputData.input[1] |
                                             (g_tAppData.tInputData.input[2] << 8));

        /* === 제어 모드에 따른 시뮬레이션 === */
        switch(control_mode)
        {
            case 0x00:  /* CLOSED */
                if(g_tVatSimState.current_position > 0) {
                    g_tVatSimState.current_position -= 10;
                    if(g_tVatSimState.current_position < 0)
                        g_tVatSimState.current_position = 0;
                    g_tVatSimState.device_status |= 0x04;  // MOVING
                } else {
                    g_tVatSimState.device_status &= ~0x04;
                    g_tVatSimState.device_status |= 0x02;  // VALVE_CLOSED
                }
                g_tVatSimState.current_pressure = 0;
                break;

            case 0x01:  /* OPEN */
                if(g_tVatSimState.current_position < 1000) {
                    g_tVatSimState.current_position += 10;
                    if(g_tVatSimState.current_position > 1000)
                        g_tVatSimState.current_position = 1000;
                    g_tVatSimState.device_status |= 0x04;  // MOVING
                } else {
                    g_tVatSimState.device_status &= ~0x04;
                    g_tVatSimState.device_status |= 0x01;  // VALVE_OPEN
                }
                g_tVatSimState.current_pressure = 1000;
                break;

            case 0x02:  /* PRESSURE */
            {
                int16_t pressure_error = control_setpoint - g_tVatSimState.current_pressure;

                if(abs(pressure_error) <= 10) {
                    /* 설정값 도달 */
                    g_tVatSimState.device_status |= 0x08;   // AT_SETPOINT
                    g_tVatSimState.device_status &= ~0x04;  // MOVING 해제
                } else {
                    /* 압력 조정 중 */
                    g_tVatSimState.device_status |= 0x04;   // MOVING
                    g_tVatSimState.device_status &= ~0x08;  // AT_SETPOINT 해제

                    /* 밸브 위치 조정 */
                    if(pressure_error > 0) {
                        g_tVatSimState.current_position += 5;
                        if(g_tVatSimState.current_position > 1000)
                            g_tVatSimState.current_position = 1000;
                    } else {
                        g_tVatSimState.current_position -= 5;
                        if(g_tVatSimState.current_position < 0)
                            g_tVatSimState.current_position = 0;
                    }

                    /* 압력 업데이트 (간단한 모델) */
                    g_tVatSimState.current_pressure =
                        (g_tVatSimState.current_position * control_setpoint) / 1000;
                }
                break;
            }

            case 0x03:  /* POSITION */
            {
                int16_t position_error = control_setpoint - g_tVatSimState.current_position;

                if(abs(position_error) <= 5) {
                    g_tVatSimState.device_status |= 0x08;
                    g_tVatSimState.device_status &= ~0x04;
                } else {
                    g_tVatSimState.device_status |= 0x04;
                    g_tVatSimState.device_status &= ~0x08;

                    if(position_error > 0) {
                        g_tVatSimState.current_position += 10;
                        if(g_tVatSimState.current_position > 1000)
                            g_tVatSimState.current_position = 1000;
                    } else {
                        g_tVatSimState.current_position -= 10;
                        if(g_tVatSimState.current_position < 0)
                            g_tVatSimState.current_position = 0;
                    }
                }

                /* 위치에 따른 압력 (최대 1000 mbar) */
                g_tVatSimState.current_pressure = g_tVatSimState.current_position;
                break;
            }

            default:
                /* 알 수 없는 모드 - 안전을 위해 닫힘 */
                g_tVatSimState.current_position = 0;
                g_tVatSimState.current_pressure = 0;
                g_tVatSimState.device_status = 0x02;  // VALVE_CLOSED
                break;
        }

        /* 밸브 완전 열림/닫힘 플래그 갱신 */
        if(g_tVatSimState.current_position >= 1000) {
            g_tVatSimState.device_status |= 0x01;   // VALVE_OPEN
            g_tVatSimState.device_status &= ~0x02;
        } else if(g_tVatSimState.current_position <= 0) {
            g_tVatSimState.device_status |= 0x02;   // VALVE_CLOSED
            g_tVatSimState.device_status &= ~0x01;
        } else {
            g_tVatSimState.device_status &= ~0x03;  // 중간 상태
        }

        /* === Slave → Master: 센서 데이터 전송 === */
        g_tAppData.tOutputData.output[0] = g_tVatSimState.exception_status;
        g_tAppData.tOutputData.output[1] = (uint8_t)(g_tVatSimState.current_pressure & 0xFF);
        g_tAppData.tOutputData.output[2] = (uint8_t)((g_tVatSimState.current_pressure >> 8) & 0xFF);
        g_tAppData.tOutputData.output[3] = (uint8_t)(g_tVatSimState.current_position & 0xFF);
        g_tAppData.tOutputData.output[4] = (uint8_t)((g_tVatSimState.current_position >> 8) & 0xFF);
        g_tAppData.tOutputData.output[5] = g_tVatSimState.device_status;
        g_tAppData.tOutputData.output[6] = 0x00;  // access_mode (reserved)

        /* DPM에 쓰기 */
        lRet = xChannelIOWrite(hChannel, 0, 0,
                               sizeof(g_tAppData.tOutputData),
                               &g_tAppData.tOutputData, 0);
        if(lRet != CIFX_NO_ERROR) {
            printf("[%lu] Write Error: 0x%08X\r\n", cycle_count, (unsigned int)lRet);
        }

        /* 매 10회마다 상태 출력 */
        if((cycle_count % 10) == 0) {
            printf("[%lu] Mode=%d, Setpoint=%d, Pressure=%d, Position=%d, Status=0x%02X\r\n",
                   cycle_count,
                   control_mode,
                   control_setpoint,
                   g_tVatSimState.current_pressure,
                   g_tVatSimState.current_position,
                   g_tVatSimState.device_status);
        }

        /* 100ms 대기 */
        HAL_Delay(100);
        cycle_count++;

        /* 1000회마다 진행 상황 */
        if((cycle_count % 1000) == 0) {
            printf("\r\n*** VAT Simulation running for %lu cycles (%.1f min) ***\r\n",
                   cycle_count, (float)cycle_count / 600.0f);
        }
    }

    break;
}
```

---

## 5. 테스트 시나리오

### 5.1 압력 제어 모드 테스트

**NetHost에서 설정**:
- Control Mode: `0x02` (PRESSURE)
- Control Setpoint: `500` (500 mbar)

**예상 결과**:
1. 초기: Position=0, Pressure=0, Status=0x02 (CLOSED)
2. 조정 중: Position 증가 (5씩), Pressure 증가, Status=0x04 (MOVING)
3. 도달: Position≈500, Pressure≈500, Status=0x08 (AT_SETPOINT)

### 5.2 위치 제어 모드 테스트

**NetHost에서 설정**:
- Control Mode: `0x03` (POSITION)
- Control Setpoint: `750` (75% 열림)

**예상 결과**:
1. 초기: Position=0
2. 조정 중: Position 증가 (10씩), Status=0x04 (MOVING)
3. 도달: Position=750, Status=0x08 (AT_SETPOINT)

### 5.3 OPEN/CLOSED 모드 테스트

**OPEN 테스트**:
- Control Mode: `0x01`
- 결과: Position → 1000, Status=0x01 (VALVE_OPEN)

**CLOSED 테스트**:
- Control Mode: `0x00`
- 결과: Position → 0, Status=0x02 (VALVE_CLOSED)

---

## 6. NetHost/CYCON.net에서 확인할 값

### Input Assembly 100 (STM32 → Master)

| Offset | 이름 | 예상 값 (PRESSURE mode, setpoint=500) |
|--------|------|---------------------------------------|
| 0 | Exception Status | 0x00 (정상) |
| 1-2 | Pressure | 500 (0x01F4) |
| 3-4 | Position | 500 (0x01F4) |
| 5 | Device Status | 0x08 (AT_SETPOINT) |
| 6 | Access Mode | 0x00 |

### Output Assembly 8 (Master → STM32)

| Offset | 이름 | 설정 값 |
|--------|------|---------|
| 0 | Control Mode | 0x02 (PRESSURE) |
| 1-2 | Control Setpoint | 500 (0x01F4) |
| 3-4 | Control Instance | 0x0000 |

---

## 7. 예외 상태 시뮬레이션 (선택 사항)

과압 시뮬레이션 예제:

```c
/* 압력이 1000 mbar 초과 시 예외 발생 */
if(g_tVatSimState.current_pressure > 1000) {
    g_tVatSimState.exception_status |= 0x04;  // OVERPRESSURE
    /* 안전을 위해 밸브 닫기 */
    g_tVatSimState.current_position = 0;
    g_tVatSimState.current_pressure = 0;
} else {
    g_tVatSimState.exception_status &= ~0x04;
}
```

---

## 8. 요약

| 항목 | 기존 (단순 증가) | 신규 (실제 시뮬레이션) |
|------|------------------|----------------------|
| Setpoint | 500→1001 증가 | Master에서 수신 |
| Pressure | 단순 증가 | Setpoint 추종 (ramping) |
| Position | 단순 증가 | Pressure/Position 모드에 따라 조정 |
| Device Status | 고정값 | MOVING/AT_SETPOINT 동적 변경 |
| 제어 로직 | 없음 | PRESSURE/POSITION/OPEN/CLOSED 모드 |

---

## 9. 적용 방법

1. **STM32CubeIDE에서 `Core/Src/main.c` 열기**
2. **전역 변수 섹션에 `VAT_SIM_STATE_T` 구조체 및 `g_tVatSimState` 추가**
3. **`case 3:` 블록을 위의 "4.2 Mode 3 무한 루프 수정" 코드로 교체**
4. **빌드 및 다운로드**
5. **NetHost에서 Control Mode 및 Setpoint 설정**
6. **Live Expressions에서 `g_tVatSimState` 관찰**
7. **NetHost Input Assembly 100에서 실시간 데이터 확인**

---

## 10. 기대 효과

- ✅ VAT EDS 사양에 맞는 실제 의미 있는 데이터 송수신
- ✅ Master의 제어 명령에 따라 동적으로 반응하는 시뮬레이션
- ✅ 압력/위치 제어 루프의 실제 동작 재현
- ✅ NetHost에서 실시간으로 밸브 상태 및 센서 값 관찰 가능
- ✅ 향후 실제 VAT 하드웨어 연동 시 동일한 인터페이스 사용 가능

---

**작성자**: Claude (SuperClaude Framework)
**문서 버전**: 1.0

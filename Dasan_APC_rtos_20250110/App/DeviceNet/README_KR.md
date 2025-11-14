# DeviceNet 통합 모듈

이 디렉토리는 Dasan APC RTOS 프로젝트를 위한 DeviceNet 프로토콜 통합을 포함합니다.

## 디렉토리 구조

```
DeviceNet/
├── includes/                        # 헤더 파일
│   ├── DNS_API/                     # DNS 패킷 API 헤더 (SDK에서)
│   │   └── README.md               # SDK 파일 복사 지침
│   ├── AppDNS_DemoApplication.h    # 메인 애플리케이션 헤더
│   └── AppDNS_DeviceNetTask.h      # FreeRTOS 태스크 헤더
│
└── Sources/                         # 소스 파일
    ├── AppDNS_DemoApplication.c           # 메인 애플리케이션 구현
    ├── AppDNS_DemoApplicationFunctions.c  # 헬퍼 함수
    ├── AppDNS_ExplicitMsg.c               # Explicit 메시징
    └── AppDNS_DeviceNetTask.c             # FreeRTOS 태스크 구현
```

## 개요

DeviceNet은 CAN (Controller Area Network) 기반의 산업용 통신 프로토콜입니다. 이 모듈은 DeviceNet Slave (DNS) 프로토콜 스택을 FreeRTOS 기반 애플리케이션과 통합합니다.

## 통합 단계

### 1. DeviceNet SDK 파일 복사

DeviceNet SDK에서 필요한 파일을 복사합니다:

1. **DNS_API 헤더**: 모든 DNS API 헤더를 `includes/DNS_API/`로 복사
   - 저수준 DeviceNet 프로토콜 인터페이스 제공
   - 자세한 내용은 `includes/DNS_API/README.md` 참조

2. **애플리케이션 템플릿**: 데모 애플리케이션 소스 파일을 `Sources/`로 복사
   - `AppDNS_DemoApplication.c`
   - `AppDNS_DemoApplicationFunctions.c`
   - `AppDNS_ExplicitMsg.c`

### 2. cifXToolkit 설정

DeviceNet 스택은 netX 90 칩에 대한 통신 인터페이스를 제공하는 cifXToolkit 위에서 실행됩니다. cifXToolkit 통합에 대해서는 `../cifXToolkit/README_KR.md`를 참조하십시오.

### 3. FreeRTOS 태스크 통합

`AppDNS_DeviceNetTask.c` 파일은 DeviceNet 스택을 위한 FreeRTOS 태스크 래퍼를 제공합니다:

```c
/* main.c 또는 freertos.c에서 DeviceNet 태스크 생성 */
#include "AppDNS_DeviceNetTask.h"

void MX_FREERTOS_Init(void)
{
    /* DeviceNet 태스크 생성 */
    if (AppDNS_DeviceNetTask_Create() != pdPASS)
    {
        Error_Handler();
    }
}
```

### 4. 설정 파라미터

DeviceNet 파라미터 설정:
- **MAC ID**: DeviceNet 네트워크의 노드 주소 (0-63)
- **Baud Rate**: 125 kbps, 250 kbps, 또는 500 kbps
- **I/O Assembly**: 입출력 데이터 설정
- **Explicit Messaging**: 명령/응답 설정

### 5. 네트워크 객체

DeviceNet 네트워크 객체는 netX 90 펌웨어에서 설정되어야 합니다. 일반적으로 다음을 통해 수행됩니다:
- 네트워크 설정 파일 (`.nxd` 파일)
- Hilscher netX Studio 도구
- 런타임 설정 패킷

## 태스크 설정

DeviceNet 태스크는 `includes/AppDNS_DeviceNetTask.h`에서 설정됩니다:

```c
#define DEVICENET_TASK_PRIORITY      (tskIDLE_PRIORITY + 3)
#define DEVICENET_TASK_STACK_SIZE    (1024 * 4)  /* 4KB 스택 */
```

애플리케이션 요구사항에 따라 이 값을 조정하십시오.

## 종속성

- **FreeRTOS**: 태스크 관리 및 동기화
- **cifXToolkit**: netX 90 칩과의 통신
- **DeviceNet SDK**: 프로토콜 스택 구현
- **STM32 HAL**: 하드웨어 추상화 (SPI, GPIO)

## 통신 흐름

```
애플리케이션 (DeviceNet 태스크)
    ↓
DeviceNet SDK (DNS API)
    ↓
cifXToolkit (DPM 액세스)
    ↓
SerialDPM (SPI 프로토콜)
    ↓
STM32 SPI5 HAL
    ↓
netX 90 칩 (DeviceNet 프로토콜)
    ↓
DeviceNet 네트워크
```

## DeviceNet 태스크 동작 흐름

### 초기화 시퀀스

```
1. 태스크 시작
   ↓
2. SPI 하드웨어 초기화
   ├─ GPIO 설정 (CS, SRDY)
   └─ SPI5 파라미터 확인
   ↓
3. cifX 드라이버 열기
   ├─ DPM 인터페이스 초기화
   └─ 통신 채널 설정
   ↓
4. DeviceNet 스택 초기화
   ├─ MAC ID 설정
   ├─ Baud Rate 설정
   └─ I/O Assembly 설정
   ↓
5. 네트워크 시작
   └─ 온라인 상태 대기
   ↓
6. 메인 루프 진입
```

### 메인 루프 (10ms 주기)

```
무한 루프 {
    │
    ├─→ I/O 데이터 처리
    │   ├─ 입력 데이터 읽기 (센서 → DeviceNet)
    │   └─ 출력 데이터 쓰기 (DeviceNet → 액추에이터)
    │
    ├─→ Explicit 메시지 처리
    │   ├─ 수신 메시지 확인
    │   ├─ 명령 처리
    │   └─ 응답 전송
    │
    ├─→ 상태 모니터링
    │   ├─ 연결 상태 확인
    │   └─ 에러 감지
    │
    └─→ vTaskDelay(10ms)
}
```

## I/O 데이터 매핑 예제

### 입력 어셈블리 (센서 → DeviceNet)

```c
typedef struct {
    uint16_t pressure_sensor_1;    // 압력 센서 1 (0.01 bar 단위)
    uint16_t pressure_sensor_2;    // 압력 센서 2 (0.01 bar 단위)
    uint16_t temperature;          // 온도 (0.1°C 단위)
    uint8_t  digital_inputs;       // 디지털 입력 (비트 필드)
    uint8_t  status_flags;         // 상태 플래그
} DeviceNet_InputData_t;

/* 애플리케이션에서 DeviceNet으로 데이터 전송 */
void UpdateDeviceNetInputs(void)
{
    DeviceNet_InputData_t inputData;

    /* 센서 값 읽기 */
    inputData.pressure_sensor_1 = ReadPressureSensor1() * 100;
    inputData.pressure_sensor_2 = ReadPressureSensor2() * 100;
    inputData.temperature = ReadTemperature() * 10;
    inputData.digital_inputs = ReadDigitalInputs();
    inputData.status_flags = GetStatusFlags();

    /* DeviceNet 입력 영역에 쓰기 */
    xChannelIOWrite(hDevice, 0, 0,
                    sizeof(inputData),
                    &inputData,
                    100);
}
```

### 출력 어셈블리 (DeviceNet → 액추에이터)

```c
typedef struct {
    uint16_t valve_control;        // 밸브 제어 (0-10000 = 0-100%)
    uint8_t  digital_outputs;      // 디지털 출력 (비트 필드)
    uint8_t  control_mode;         // 제어 모드
} DeviceNet_OutputData_t;

/* DeviceNet에서 애플리케이션으로 데이터 수신 */
void ProcessDeviceNetOutputs(void)
{
    DeviceNet_OutputData_t outputData;
    uint32_t bytesRead;

    /* DeviceNet 출력 영역 읽기 */
    if (xChannelIORead(hDevice, 0, 0,
                       sizeof(outputData),
                       &outputData,
                       &bytesRead,
                       100) == CIFX_NO_ERROR)
    {
        /* 액추에이터 제어 */
        SetValveControl(outputData.valve_control / 100.0);
        SetDigitalOutputs(outputData.digital_outputs);
        SetControlMode(outputData.control_mode);
    }
}
```

## Explicit 메시징 예제

### 파라미터 읽기 요청 처리

```c
/* Object Dictionary 읽기 요청 처리 */
int32_t HandleExplicitReadRequest(uint16_t classID,
                                   uint16_t instanceID,
                                   uint16_t attributeID,
                                   uint8_t* pData,
                                   uint16_t* pDataLen)
{
    /* 예: 디바이스 정보 읽기 */
    if (classID == 0x01 && instanceID == 0x01)
    {
        switch (attributeID)
        {
            case 1:  /* Vendor ID */
                *(uint16_t*)pData = VENDOR_ID;
                *pDataLen = 2;
                return 0;

            case 2:  /* Device Type */
                *(uint16_t*)pData = DEVICE_TYPE;
                *pDataLen = 2;
                return 0;

            case 3:  /* Product Code */
                *(uint16_t*)pData = PRODUCT_CODE;
                *pDataLen = 2;
                return 0;

            default:
                return -1;  /* Attribute not supported */
        }
    }

    return -1;  /* Class/Instance not found */
}
```

### 파라미터 쓰기 요청 처리

```c
/* Object Dictionary 쓰기 요청 처리 */
int32_t HandleExplicitWriteRequest(uint16_t classID,
                                    uint16_t instanceID,
                                    uint16_t attributeID,
                                    uint8_t* pData,
                                    uint16_t dataLen)
{
    /* 예: 설정값 쓰기 */
    if (classID == 0x64 && instanceID == 0x01)
    {
        switch (attributeID)
        {
            case 1:  /* Setpoint */
                if (dataLen == 2)
                {
                    uint16_t setpoint = *(uint16_t*)pData;
                    SetControlSetpoint(setpoint);
                    return 0;
                }
                break;

            case 2:  /* Control Mode */
                if (dataLen == 1)
                {
                    uint8_t mode = *pData;
                    SetControlMode(mode);
                    return 0;
                }
                break;
        }
    }

    return -1;  /* Write failed */
}
```

## 에러 처리

### 네트워크 에러 감지

```c
/* DeviceNet 네트워크 상태 모니터링 */
void MonitorDeviceNetStatus(void)
{
    uint32_t networkStatus;

    /* 네트워크 상태 읽기 */
    if (GetNetworkStatus(&networkStatus) == 0)
    {
        if (networkStatus & DNS_STATUS_BUS_OFF)
        {
            /* Bus-Off 상태 - 복구 시도 */
            RecoverFromBusOff();
        }

        if (networkStatus & DNS_STATUS_TIMEOUT)
        {
            /* 통신 타임아웃 */
            HandleCommunicationTimeout();
        }

        if (networkStatus & DNS_STATUS_DUPLICATE_MAC)
        {
            /* 중복 MAC ID 감지 */
            HandleDuplicateMacID();
        }
    }
}
```

### 에러 복구 절차

```c
/* Bus-Off 복구 */
void RecoverFromBusOff(void)
{
    /* 1. 출력 안전 상태로 설정 */
    SetSafeOutputState();

    /* 2. 에러 카운터 리셋 */
    ResetErrorCounters();

    /* 3. 네트워크 재시작 */
    RestartDeviceNetNetwork();

    /* 4. 온라인 상태 대기 (타임아웃 5초) */
    if (WaitForOnline(5000) == 0)
    {
        /* 복구 성공 */
        LogEvent("DeviceNet: Recovered from Bus-Off");
    }
    else
    {
        /* 복구 실패 - 에러 표시 */
        SetErrorState(ERROR_DEVICENET_OFFLINE);
    }
}
```

## 테스트

### 1. 네트워크 초기화 테스트

```c
void Test_DeviceNet_Initialization(void)
{
    BaseType_t result;

    /* DeviceNet 태스크 생성 */
    result = AppDNS_DeviceNetTask_Create();

    assert(result == pdPASS);

    /* 초기화 완료 대기 (최대 10초) */
    vTaskDelay(pdMS_TO_TICKS(10000));

    /* 네트워크 온라인 상태 확인 */
    assert(IsDeviceNetOnline() == true);
}
```

### 2. I/O 데이터 교환 테스트

```c
void Test_DeviceNet_IOExchange(void)
{
    DeviceNet_InputData_t testInput;
    DeviceNet_OutputData_t receivedOutput;

    /* 테스트 입력 데이터 설정 */
    testInput.pressure_sensor_1 = 1500;  // 15.00 bar
    testInput.pressure_sensor_2 = 2000;  // 20.00 bar
    testInput.temperature = 250;         // 25.0°C

    /* 입력 데이터 전송 */
    UpdateDeviceNetInputs(&testInput);

    /* 출력 데이터 수신 대기 */
    vTaskDelay(pdMS_TO_TICKS(50));

    /* 출력 데이터 확인 */
    ProcessDeviceNetOutputs(&receivedOutput);

    /* 출력 값 검증 */
    assert(receivedOutput.valve_control <= 10000);
}
```

### 3. Explicit 메시징 테스트

```c
void Test_DeviceNet_ExplicitMsg(void)
{
    uint16_t vendorID;
    int32_t result;

    /* Vendor ID 읽기 요청 */
    result = ReadObjectAttribute(0x01, 0x01, 0x01,
                                  (uint8_t*)&vendorID,
                                  sizeof(vendorID));

    assert(result == 0);
    assert(vendorID == EXPECTED_VENDOR_ID);

    /* Setpoint 쓰기 요청 */
    uint16_t setpoint = 5000;
    result = WriteObjectAttribute(0x64, 0x01, 0x01,
                                   (uint8_t*)&setpoint,
                                   sizeof(setpoint));

    assert(result == 0);
}
```

## 성능 최적화

### 타이밍 최적화

```c
/* DeviceNet 태스크 타이밍 설정 */
#define DEVICENET_CYCLE_TIME_MS    10    // 10ms 주기 (일반적)
// #define DEVICENET_CYCLE_TIME_MS    5   // 5ms 주기 (고속)
// #define DEVICENET_CYCLE_TIME_MS    20  // 20ms 주기 (저속)

/* 메인 루프 */
void AppDNS_DeviceNetTask(void *argument)
{
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(DEVICENET_CYCLE_TIME_MS);

    /* 초기화... */

    /* 정확한 주기 실행을 위해 vTaskDelayUntil 사용 */
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        /* DeviceNet 처리 */
        AppDNS_DemoApplication_Run();

        /* 정확한 주기 대기 */
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}
```

### 메모리 최적화

```c
/* I/O 버퍼 크기 최소화 */
#define MAX_INPUT_SIZE     32   // 입력 어셈블리 최대 크기
#define MAX_OUTPUT_SIZE    32   // 출력 어셈블리 최대 크기

/* 스택 크기 조정 */
#define DEVICENET_TASK_STACK_SIZE    (1024 * 3)  // 필요에 따라 조정
```

## 디버깅

### 디버그 출력 활성화

```c
/* AppDNS_DeviceNetTask.c에 추가 */
#define DEVICENET_DEBUG    1

#if DEVICENET_DEBUG
#define DEBUG_PRINT(...)   printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

/* 사용 예 */
DEBUG_PRINT("DeviceNet: Network online\r\n");
DEBUG_PRINT("DeviceNet: Input data sent, pressure=%d\r\n", pressure);
```

### 상태 LED 표시

```c
/* DeviceNet 상태 LED 제어 */
void UpdateDeviceNetStatusLED(void)
{
    uint32_t status = GetDeviceNetStatus();

    if (status & DNS_STATUS_ONLINE)
    {
        /* 녹색 LED: 온라인 */
        HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);
    }
    else if (status & DNS_STATUS_ERROR)
    {
        /* 빨간색 LED: 에러 */
        HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);
    }
    else
    {
        /* 깜빡임: 초기화 중 */
        HAL_GPIO_TogglePin(LED_GREEN_PORT, LED_GREEN_PIN);
    }
}
```

## 참조

- DeviceNet 사양 (ODVA)
- Hilscher DeviceNet SDK 문서
- netX 90 사용자 매뉴얼
- cifXToolkit API 레퍼런스

## 참고사항

- DeviceNet 태스크 우선순위는 애플리케이션 태스크보다 높지만 중요한 시스템 태스크보다는 낮아야 합니다
- 스택 크기는 I/O 어셈블리의 복잡도에 따라 조정이 필요할 수 있습니다
- DeviceNet 네트워크 통신을 위한 적절한 타이밍을 보장하십시오 (일반적으로 10ms 주기)
- Explicit 메시징은 비주기적이므로 I/O 데이터 교환을 방해하지 않아야 합니다

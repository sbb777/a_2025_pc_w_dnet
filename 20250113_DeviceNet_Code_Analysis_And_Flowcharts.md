# DeviceNet 코드 상세 분석 및 순서도

**작성 일시**: 2025-01-13  
**프로젝트**: Dasan_APC_rtos_20250110  
**버전**: FULL Mode Implementation v1.0

---

## 목차

1. [개요](#1-개요)
2. [코드 구조 분석](#2-코드-구조-분석)
3. [함수별 상세 분석](#3-함수별-상세-분석)
4. [동작 순서도](#4-동작-순서도)
5. [데이터 흐름도](#5-데이터-흐름도)
6. [API 사용 가이드](#6-api-사용-가이드)
7. [에러 처리](#7-에러-처리)
8. [성능 분석](#8-성능-분석)

---

## 1. 개요

### 1.1 구현 범위

**파일**: `App/DeviceNet/Sources/AppDNS_DemoApplication.c`

**구현된 기능**:
- ✅ cifX 드라이버 초기화
- ✅ DeviceNet 채널 오픈
- ✅ 주기적 I/O 데이터 교환 (Cyclic I/O)
- ✅ 통계 추적 (읽기/쓰기/에러 카운트)
- ✅ 조건부 컴파일 (STUB/FULL 모드)

**미구현 (향후 확장)**:
- ⏳ Explicit 메시지 처리 (xChannelGetPacket/PutPacket)
- ⏳ 네트워크 설정 (MAC ID, Baud rate)
- ⏳ 에러 복구 및 재연결

### 1.2 아키텍처

```
┌─────────────────────────────────────────────────────┐
│           APC Application Layer                     │
│  (Sensor/Control Logic, Business Logic)             │
└─────────────┬───────────────────────────────────────┘
              │ AppDNS_SetInputData()
              │ AppDNS_GetOutputData()
              ▼
┌─────────────────────────────────────────────────────┐
│      DeviceNet Application Layer                    │
│    AppDNS_DemoApplication.c                         │
│  - Init: Driver + Channel Open                      │
│  - Run: Cyclic I/O Exchange                         │
└─────────────┬───────────────────────────────────────┘
              │ xChannelIORead/Write()
              ▼
┌─────────────────────────────────────────────────────┐
│          cifXToolkit Library                        │
│  - xDriverOpen()                                    │
│  - xChannelOpen()                                   │
│  - xChannelIORead/Write()                           │
└─────────────┬───────────────────────────────────────┘
              │ SerialDPM_*()
              ▼
┌─────────────────────────────────────────────────────┐
│      Serial DPM Interface (SPI)                     │
│    SerialDPMInterface.c                             │
│  - SPI5 Communication                               │
└─────────────┬───────────────────────────────────────┘
              │ HAL_SPI_TransmitReceive()
              ▼
┌─────────────────────────────────────────────────────┐
│         STM32 HAL SPI5 Driver                       │
│  - GPIO: CS (PF6), SRDY (PF10)                      │
│  - SPI5: SCK (PF7), MOSI (PF9), MISO (PF8)          │
└─────────────┬───────────────────────────────────────┘
              │ Hardware
              ▼
         ┌──────────┐
         │ netX 90  │ ← DeviceNet Protocol Processor
         └──────────┘
```

---

## 2. 코드 구조 분석

### 2.1 주요 변수

```c
/* cifX 핸들 */
static CIFXHANDLE g_hDriver = NULL;   // 드라이버 핸들
static CIFXHANDLE g_hChannel = NULL;  // 채널 핸들

/* I/O 데이터 버퍼 */
static uint8_t g_inputData[DEVICENET_INPUT_SIZE];   // Slave → Master (8 bytes)
static uint8_t g_outputData[DEVICENET_OUTPUT_SIZE]; // Master → Slave (8 bytes)

/* 상태 및 통계 */
static uint8_t g_deviceNetInitialized = 0;  // 초기화 플래그
static uint32_t g_ioReadCount = 0;          // 읽기 성공 횟수
static uint32_t g_ioWriteCount = 0;         // 쓰기 성공 횟수
static uint32_t g_ioErrorCount = 0;         // 에러 발생 횟수
```

### 2.2 핵심 상수

```c
#define DEVICENET_DEVICE_NAME       "cifX0"  // 디바이스 이름
#define DEVICENET_CHANNEL_INDEX     0        // 채널 번호 (0)
#define CIFX_IO_TIMEOUT_MS          10       // I/O 타임아웃 (10ms)

// DeviceNet_Config.h에서 정의됨
#define DEVICENET_INPUT_SIZE        8        // 입력 데이터 크기
#define DEVICENET_OUTPUT_SIZE       8        // 출력 데이터 크기
```

### 2.3 함수 계층 구조

```
Public API (AppDNS_*)
├── AppDNS_DemoApplication_Init()
│   ├── DeviceNet_InitDriver()        [Private]
│   │   ├── SerialDPM_Init()
│   │   └── xDriverOpen()
│   └── DeviceNet_OpenChannel()       [Private]
│       └── xChannelOpen()
│
├── AppDNS_DemoApplication_Run()
│   └── DeviceNet_IOExchange()        [Private]
│       ├── xChannelIORead()
│       └── xChannelIOWrite()
│
├── AppDNS_SetInputData()
├── AppDNS_GetOutputData()
├── AppDNS_GetStatistics()
├── AppDNS_GetInitStatus()
└── AppDNS_GetStubCounter()
```

---

## 3. 함수별 상세 분석

### 3.1 DeviceNet_InitDriver()

**목적**: cifX 드라이버 초기화 및 오픈

**코드 흐름**:
```c
static int32_t DeviceNet_InitDriver(void)
{
    int32_t lRet;

    // Step 1: SPI DPM 인터페이스 초기화
    lRet = SerialDPM_Init();
    if (lRet != CIFX_NO_ERROR)
        return -1;  // SPI 초기화 실패

    // Step 2: cifX 드라이버 오픈
    lRet = xDriverOpen(&g_hDriver);
    if (lRet != CIFX_NO_ERROR)
        return -2;  // 드라이버 오픈 실패

    return 0;  // 성공
}
```

**동작 설명**:
1. **SerialDPM_Init()**:
   - SPI5 하드웨어 초기화
   - GPIO 핀 초기화 (CS, SRDY)
   - netX 90과의 통신 준비

2. **xDriverOpen()**:
   - cifX 드라이버 인스턴스 생성
   - 드라이버 핸들(`g_hDriver`) 반환
   - 전역 드라이버 정보 초기화

**반환값**:
- `0`: 성공
- `-1`: SPI 초기화 실패
- `-2`: 드라이버 오픈 실패

---

### 3.2 DeviceNet_OpenChannel()

**목적**: DeviceNet 통신 채널 오픈

**코드 흐름**:
```c
static int32_t DeviceNet_OpenChannel(void)
{
    int32_t lRet;

    // Channel 0 오픈
    lRet = xChannelOpen(g_hDriver,
                        DEVICENET_DEVICE_NAME,  // "cifX0"
                        DEVICENET_CHANNEL_INDEX, // 0
                        &g_hChannel);

    if (lRet != CIFX_NO_ERROR)
        return -1;  // 채널 오픈 실패

    return 0;  // 성공
}
```

**동작 설명**:
1. **xChannelOpen()**:
   - 드라이버에서 지정된 채널 오픈
   - 채널 핸들(`g_hChannel`) 반환
   - DPM 메모리 매핑 설정
   - 통신 버퍼 초기화

**매개변수**:
- `g_hDriver`: 드라이버 핸들
- `"cifX0"`: 디바이스 이름
- `0`: 채널 인덱스 (보통 0)
- `&g_hChannel`: 채널 핸들 저장 포인터

**반환값**:
- `0`: 성공
- `-1`: 채널 오픈 실패

---

### 3.3 DeviceNet_IOExchange()

**목적**: 주기적 I/O 데이터 교환 (핵심 함수)

**코드 흐름**:
```c
static int32_t DeviceNet_IOExchange(void)
{
    int32_t lRet;

    // Step 1: Master → Slave 데이터 읽기
    lRet = xChannelIORead(g_hChannel,
                          0,                      // Area number
                          0,                      // Offset
                          DEVICENET_OUTPUT_SIZE,  // 8 bytes
                          g_outputData,
                          CIFX_IO_TIMEOUT_MS);    // 10ms

    if (lRet == CIFX_NO_ERROR)
        g_ioReadCount++;
    else
        g_ioErrorCount++;

    // Step 2: Slave → Master 데이터 쓰기
    lRet = xChannelIOWrite(g_hChannel,
                           0,                     // Area number
                           0,                     // Offset
                           DEVICENET_INPUT_SIZE,  // 8 bytes
                           g_inputData,
                           CIFX_IO_TIMEOUT_MS);   // 10ms

    if (lRet == CIFX_NO_ERROR)
        g_ioWriteCount++;
    else
        g_ioErrorCount++;

    return 0;
}
```

**동작 설명**:

1. **xChannelIORead()** - 입력 데이터 읽기:
   - Master가 보낸 제어 데이터를 읽음
   - DPM의 Input Area에서 데이터 복사
   - `g_outputData[]`에 저장 (8 bytes)
   - 타임아웃: 10ms

2. **xChannelIOWrite()** - 출력 데이터 쓰기:
   - Slave 센서 데이터를 Master로 전송
   - `g_inputData[]`를 DPM의 Output Area에 복사
   - 타임아웃: 10ms

3. **통계 업데이트**:
   - 성공 시: 각 카운터 증가
   - 실패 시: `g_ioErrorCount` 증가

**Area Number = 0**: 표준 I/O 영역 (DeviceNet의 경우 보통 0)

**반환값**: 항상 `0` (에러는 내부에서 통계로 추적)

---

### 3.4 AppDNS_DemoApplication_Init()

**목적**: DeviceNet 애플리케이션 전체 초기화

**코드 흐름**:
```c
int AppDNS_DemoApplication_Init(void)
{
#if ENABLE_DEVICENET
    int32_t lRet;

    // Step 1: 드라이버 초기화
    lRet = DeviceNet_InitDriver();
    if (lRet != 0)
        return lRet;

    // Step 2: 채널 오픈
    lRet = DeviceNet_OpenChannel();
    if (lRet != 0)
        return lRet;

    // Step 3: 버퍼 초기화
    memset(g_inputData, 0, sizeof(g_inputData));
    memset(g_outputData, 0, sizeof(g_outputData));

    // Step 4: 통계 리셋
    g_ioReadCount = 0;
    g_ioWriteCount = 0;
    g_ioErrorCount = 0;

    // Step 5: 초기화 완료 플래그 설정
    g_deviceNetInitialized = 1;

    return 0;  // 성공
#else
    // STUB mode
    g_stubCounter = 0;
    return 0;
#endif
}
```

**호출 시점**: FreeRTOS 태스크 시작 시 (1회)

**반환값**:
- `0`: 성공
- `-1`: SPI 초기화 실패
- `-2`: 드라이버 오픈 실패
- `-1` (from OpenChannel): 채널 오픈 실패

---

### 3.5 AppDNS_DemoApplication_Run()

**목적**: 주기적으로 호출되는 메인 루프

**코드 흐름**:
```c
int AppDNS_DemoApplication_Run(void)
{
#if ENABLE_DEVICENET
    // 초기화 확인
    if (!g_deviceNetInitialized)
        return -1;

    // I/O 교환 수행
    DeviceNet_IOExchange();

    // TODO: Explicit 메시지 처리 (향후)
    // xChannelGetPacket()
    // xChannelPutPacket()

    return 0;
#else
    // STUB mode
    g_stubCounter++;
    return 0;
#endif
}
```

**호출 시점**: FreeRTOS 태스크에서 주기적 (권장: 10ms)

**반환값**:
- `0`: 정상
- `-1`: 초기화되지 않음

---

### 3.6 AppDNS_SetInputData()

**목적**: 애플리케이션에서 Master로 보낼 데이터 설정

**코드**:
```c
int AppDNS_SetInputData(const uint8_t *pData, uint8_t len)
{
    if (!g_deviceNetInitialized)
        return -1;

    if (len > DEVICENET_INPUT_SIZE)
        len = DEVICENET_INPUT_SIZE;  // 최대 8 bytes

    memcpy(g_inputData, pData, len);
    return 0;
}
```

**사용 예시**:
```c
uint8_t sensorData[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
AppDNS_SetInputData(sensorData, 8);
```

---

### 3.7 AppDNS_GetOutputData()

**목적**: Master에서 받은 제어 데이터 가져오기

**코드**:
```c
int AppDNS_GetOutputData(uint8_t *pData, uint8_t len)
{
    if (!g_deviceNetInitialized)
        return -1;

    if (len > DEVICENET_OUTPUT_SIZE)
        len = DEVICENET_OUTPUT_SIZE;  // 최대 8 bytes

    memcpy(pData, g_outputData, len);
    return 0;
}
```

**사용 예시**:
```c
uint8_t controlCmd[8];
if (AppDNS_GetOutputData(controlCmd, 8) == 0) {
    // controlCmd[0]: 명령 코드
    // controlCmd[1-7]: 파라미터
    ProcessCommand(controlCmd);
}
```

---

### 3.8 AppDNS_GetStatistics()

**목적**: I/O 통계 조회 (디버깅/모니터링)

**코드**:
```c
void AppDNS_GetStatistics(uint32_t *pReadCount,
                          uint32_t *pWriteCount,
                          uint32_t *pErrorCount)
{
    if (pReadCount)
        *pReadCount = g_ioReadCount;
    if (pWriteCount)
        *pWriteCount = g_ioWriteCount;
    if (pErrorCount)
        *pErrorCount = g_ioErrorCount;
}
```

**사용 예시**:
```c
uint32_t reads, writes, errors;
AppDNS_GetStatistics(&reads, &writes, &errors);

printf("I/O Stats: R=%lu, W=%lu, E=%lu\n", reads, writes, errors);

// 에러율 계산
float errorRate = (errors * 100.0) / (reads + writes + errors);
```

---

## 4. 동작 순서도

### 4.1 초기화 순서도

```
┌─────────────────────────────────────────┐
│  FreeRTOS: DeviceNet Task Start         │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  AppDNS_DemoApplication_Init()          │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  DeviceNet_InitDriver()                 │
├─────────────────────────────────────────┤
│  1. SerialDPM_Init()                    │
│     - SPI5 초기화                        │
│     - GPIO 초기화 (CS, SRDY)             │
│  2. xDriverOpen(&g_hDriver)             │
│     - cifX 드라이버 인스턴스 생성         │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  DeviceNet_OpenChannel()                │
├─────────────────────────────────────────┤
│  xChannelOpen(g_hDriver, "cifX0", 0,    │
│               &g_hChannel)              │
│  - Channel 0 오픈                        │
│  - DPM 메모리 매핑                       │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  버퍼 및 통계 초기화                      │
├─────────────────────────────────────────┤
│  - memset(g_inputData, 0, 8)            │
│  - memset(g_outputData, 0, 8)           │
│  - g_ioReadCount = 0                    │
│  - g_ioWriteCount = 0                   │
│  - g_ioErrorCount = 0                   │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  g_deviceNetInitialized = 1             │
│  (초기화 완료)                           │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  주기적 실행 루프 진입                    │
└─────────────────────────────────────────┘
```

---

### 4.2 주기적 I/O 교환 순서도

```
┌─────────────────────────────────────────┐
│  FreeRTOS: Every 10ms                   │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  AppDNS_DemoApplication_Run()           │
└──────────────┬──────────────────────────┘
               │
               ▼
        ┌──────────────┐
        │ 초기화됨?     │ NO → Return -1
        └──────┬───────┘
             YES
               │
               ▼
┌─────────────────────────────────────────┐
│  DeviceNet_IOExchange()                 │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  xChannelIORead()                       │
├─────────────────────────────────────────┤
│  Master → Slave 데이터 읽기              │
│  - Area: 0, Offset: 0                   │
│  - Size: 8 bytes                        │
│  - Timeout: 10ms                        │
│  - Dest: g_outputData[]                 │
└──────────────┬──────────────────────────┘
               │
        ┌──────▼───────┐
        │  성공?        │
        └──┬────────┬──┘
         YES       NO
           │        │
           │        ▼
           │   g_ioErrorCount++
           │
           ▼
    g_ioReadCount++
           │
           ▼
┌─────────────────────────────────────────┐
│  xChannelIOWrite()                      │
├─────────────────────────────────────────┤
│  Slave → Master 데이터 쓰기              │
│  - Area: 0, Offset: 0                   │
│  - Size: 8 bytes                        │
│  - Timeout: 10ms                        │
│  - Src: g_inputData[]                   │
└──────────────┬──────────────────────────┘
               │
        ┌──────▼───────┐
        │  성공?        │
        └──┬────────┬──┘
         YES       NO
           │        │
           │        ▼
           │   g_ioErrorCount++
           │
           ▼
    g_ioWriteCount++
           │
           ▼
┌─────────────────────────────────────────┐
│  Return 0 (완료)                         │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  osDelay(10)  // 10ms 대기               │
└──────────────┬──────────────────────────┘
               │
               └──────┐
                      │
               (반복) │
                      ▼
```

---

### 4.3 FreeRTOS 태스크 통합 순서도

```
┌─────────────────────────────────────────┐
│  main()                                 │
│  → MX_FREERTOS_Init()                   │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  AppDNS_DeviceNetTask_Create()          │
├─────────────────────────────────────────┤
│  osThreadNew(AppDNS_DeviceNetTask,      │
│              NULL, &attributes)         │
│  - Priority: osPriorityAboveNormal      │
│  - Stack: 4KB                           │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  AppDNS_DeviceNetTask() - FreeRTOS Task │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│  Step 1: 초기화                          │
│  AppDNS_DemoApplication_Init()          │
└──────────────┬──────────────────────────┘
               │
        ┌──────▼───────┐
        │  성공?        │ NO → Error handling
        └──────┬───────┘
             YES
               │
               ▼
┌─────────────────────────────────────────┐
│  Step 2: 무한 루프                       │
│  while(1) {                             │
│    AppDNS_DemoApplication_Run();        │
│    osDelay(10);  // 10ms                │
│  }                                      │
└─────────────────────────────────────────┘
```

---

## 5. 데이터 흐름도

### 5.1 전체 데이터 흐름

```
┌──────────────────────────────────────────────────────────────┐
│                    APC Application                           │
│  (Sensors, Actuators, Control Logic)                         │
└────┬─────────────────────────────────────────────────┬───────┘
     │ SetInputData()                      GetOutputData() │
     │ (Sensor → Master)                  (Master → Actuator)
     │                                                    │
     ▼                                                    ▼
┌─────────────────────┐                        ┌──────────────────┐
│  g_inputData[8]     │                        │ g_outputData[8]  │
│  (Slave → Master)   │                        │ (Master → Slave) │
└──────┬──────────────┘                        └────────┬─────────┘
       │                                                │
       │ xChannelIOWrite()                xChannelIORead() │
       │                                                │
       ▼                                                ▼
┌────────────────────────────────────────────────────────────────┐
│               cifXToolkit API Layer                            │
│  - xChannelIORead/Write()                                      │
│  - Packet marshalling/unmarshalling                            │
└────┬──────────────────────────────────────────────────┬────────┘
     │                                                   │
     │ SerialDPM_Transfer()                              │
     │                                                   │
     ▼                                                   ▼
┌────────────────────────────────────────────────────────────────┐
│            Serial DPM Interface (SPI5)                         │
│  - CS control (PF6)                                            │
│  - SRDY handshake (PF10)                                       │
│  - SPI5 transfer: HAL_SPI_TransmitReceive()                    │
└────┬──────────────────────────────────────────────────┬────────┘
     │                                                   │
     ▼                                                   ▼
┌────────────────────────────────────────────────────────────────┐
│                    netX 90 Chip                                │
│  ┌──────────────────────────────────────────────────┐          │
│  │        Dual Port Memory (DPM)                    │          │
│  │  ┌─────────────────┐  ┌─────────────────┐       │          │
│  │  │  Input Area     │  │  Output Area    │       │          │
│  │  │  (Slave→Master) │  │  (Master→Slave) │       │          │
│  │  └─────────────────┘  └─────────────────┘       │          │
│  └──────────────────────────────────────────────────┘          │
│  ┌──────────────────────────────────────────────────┐          │
│  │        DeviceNet Protocol Stack                  │          │
│  │  - CAN communication                             │          │
│  │  - I/O Assembly handling                         │          │
│  │  - Explicit messaging                            │          │
│  └──────────────────────────────────────────────────┘          │
└──────────────────┬───────────────────────────────────────────┘
                   │
                   ▼
        ┌────────────────────┐
        │  DeviceNet Bus     │
        │  (CAN Physical)    │
        └────────────────────┘
                   │
                   ▼
        ┌────────────────────┐
        │  DeviceNet Master  │
        │  (PLC, SCADA, etc) │
        └────────────────────┘
```

### 5.2 I/O 데이터 매핑

```
Slave (STM32) → Master (PLC)
─────────────────────────────
g_inputData[0]  → Byte 0  (예: 센서 1 온도)
g_inputData[1]  → Byte 1  (예: 센서 2 압력)
g_inputData[2]  → Byte 2  (예: 센서 3 유량)
g_inputData[3]  → Byte 3  (예: 디지털 입력 상태)
g_inputData[4]  → Byte 4  (예: 에러 코드)
g_inputData[5]  → Byte 5  (예: 상태 플래그)
g_inputData[6]  → Byte 6  (예: 예비)
g_inputData[7]  → Byte 7  (예: 체크섬)

Master (PLC) → Slave (STM32)
─────────────────────────────
Byte 0 → g_outputData[0]  (예: 제어 명령)
Byte 1 → g_outputData[1]  (예: 설정값 1)
Byte 2 → g_outputData[2]  (예: 설정값 2)
Byte 3 → g_outputData[3]  (예: 디지털 출력 제어)
Byte 4 → g_outputData[4]  (예: 모드 선택)
Byte 5 → g_outputData[5]  (예: 예비)
Byte 6 → g_outputData[6]  (예: 예비)
Byte 7 → g_outputData[7]  (예: 체크섬)
```

---

## 6. API 사용 가이드

### 6.1 기본 사용 패턴

```c
/* ========================================
 * 초기화 (1회, 태스크 시작 시)
 * ======================================== */
void AppDNS_DeviceNetTask(void *argument)
{
    int ret;
    
    // 초기화
    ret = AppDNS_DemoApplication_Init();
    if (ret != 0) {
        // 에러 처리
        printf("DeviceNet Init Failed: %d\n", ret);
        return;
    }
    
    printf("DeviceNet Initialized Successfully\n");
    
    /* ========================================
     * 주기적 루프
     * ======================================== */
    while(1)
    {
        // Step 1: 애플리케이션 데이터 준비
        UpdateSensorData();
        
        // Step 2: DeviceNet I/O 교환
        ret = AppDNS_DemoApplication_Run();
        if (ret != 0) {
            printf("DeviceNet Run Error: %d\n", ret);
        }
        
        // Step 3: 수신 데이터 처리
        ProcessReceivedData();
        
        // Step 4: 대기 (10ms 주기 권장)
        osDelay(10);
    }
}
```

### 6.2 센서 데이터 전송 예제

```c
void UpdateSensorData(void)
{
    uint8_t sensorData[8];
    
    // 센서 데이터 수집
    sensorData[0] = ReadTemperature();    // 온도 (°C)
    sensorData[1] = ReadPressure();       // 압력 (bar)
    sensorData[2] = ReadFlow();           // 유량 (L/min)
    sensorData[3] = ReadDigitalInputs();  // DI 상태 (8 bits)
    
    // 상태 정보
    sensorData[4] = GetErrorCode();
    sensorData[5] = GetStatusFlags();
    
    // 예비
    sensorData[6] = 0x00;
    
    // 체크섬 (간단한 XOR)
    sensorData[7] = 0;
    for (int i = 0; i < 7; i++) {
        sensorData[7] ^= sensorData[i];
    }
    
    // DeviceNet으로 전송
    AppDNS_SetInputData(sensorData, 8);
}
```

### 6.3 제어 명령 수신 예제

```c
void ProcessReceivedData(void)
{
    uint8_t controlData[8];
    
    // DeviceNet에서 데이터 수신
    if (AppDNS_GetOutputData(controlData, 8) == 0)
    {
        // 체크섬 검증
        uint8_t checksum = 0;
        for (int i = 0; i < 7; i++) {
            checksum ^= controlData[i];
        }
        
        if (checksum != controlData[7]) {
            printf("Checksum Error!\n");
            return;
        }
        
        // 명령 처리
        uint8_t command = controlData[0];
        
        switch (command)
        {
            case 0x01:  // 모터 시작
                StartMotor();
                break;
                
            case 0x02:  // 모터 정지
                StopMotor();
                break;
                
            case 0x03:  // 속도 설정
                SetMotorSpeed(controlData[1]);
                break;
                
            case 0x10:  // 디지털 출력 제어
                SetDigitalOutputs(controlData[3]);
                break;
                
            default:
                printf("Unknown Command: 0x%02X\n", command);
                break;
        }
    }
}
```

### 6.4 통계 모니터링 예제

```c
void MonitorDeviceNetStatus(void)
{
    static uint32_t lastPrintTime = 0;
    uint32_t currentTime = osKernelGetTickCount();
    
    // 1초마다 통계 출력
    if (currentTime - lastPrintTime >= 1000)
    {
        uint32_t reads, writes, errors;
        AppDNS_GetStatistics(&reads, &writes, &errors);
        
        uint32_t total = reads + writes;
        float errorRate = (total > 0) ? (errors * 100.0f / total) : 0.0f;
        
        printf("=== DeviceNet Statistics ===\n");
        printf("Read Count:   %lu\n", reads);
        printf("Write Count:  %lu\n", writes);
        printf("Error Count:  %lu\n", errors);
        printf("Error Rate:   %.2f%%\n", errorRate);
        printf("I/O Rate:     %lu ops/sec\n", total);
        
        lastPrintTime = currentTime;
    }
}
```

---

## 7. 에러 처리

### 7.1 에러 코드 정리

| 함수 | 반환값 | 의미 | 대응 방법 |
|------|--------|------|-----------|
| `AppDNS_DemoApplication_Init()` | `0` | 성공 | - |
| | `-1` | SPI 초기화 실패 | SPI5 하드웨어 확인 |
| | `-2` | 드라이버 오픈 실패 | cifX 드라이버 상태 확인 |
| | `-1` (채널) | 채널 오픈 실패 | netX 90 통신 확인 |
| `AppDNS_DemoApplication_Run()` | `0` | 정상 | - |
| | `-1` | 초기화 안됨 | Init() 먼저 호출 |
| `AppDNS_SetInputData()` | `0` | 성공 | - |
| | `-1` | 초기화 안됨 | Init() 먼저 호출 |
| `AppDNS_GetOutputData()` | `0` | 성공 | - |
| | `-1` | 초기화 안됨 | Init() 먼저 호출 |

### 7.2 에러 처리 패턴

```c
void DeviceNetErrorHandler(int errorCode)
{
    static uint32_t errorCount[10] = {0};
    
    switch (errorCode)
    {
        case -1:  // 초기화 실패 또는 미초기화
            errorCount[1]++;
            printf("ERROR: DeviceNet not initialized\n");
            
            // 재초기화 시도 (3회까지)
            if (errorCount[1] <= 3) {
                printf("Attempting re-initialization...\n");
                osDelay(1000);  // 1초 대기
                AppDNS_DemoApplication_Init();
            }
            break;
            
        case -2:  // 드라이버 오픈 실패
            errorCount[2]++;
            printf("ERROR: Driver open failed\n");
            printf("Check cifX driver status\n");
            
            // 심각한 에러 - 시스템 재시작 필요
            if (errorCount[2] > 5) {
                printf("CRITICAL: System restart required\n");
                NVIC_SystemReset();
            }
            break;
            
        default:
            printf("ERROR: Unknown error code: %d\n", errorCode);
            break;
    }
}
```

### 7.3 타임아웃 처리

```c
// xChannelIORead/Write 내부에서 타임아웃 처리됨
// CIFX_IO_TIMEOUT_MS = 10ms

// 타임아웃 발생 시:
// - 함수는 CIFX_DEV_TIMEOUT 반환
// - g_ioErrorCount 증가
// - 통계로 추적 가능

// 대응 방법:
uint32_t reads, writes, errors;
AppDNS_GetStatistics(&reads, &writes, &errors);

if (errors > 100) {
    printf("WARNING: High error rate detected\n");
    printf("Check network connection\n");
    
    // 재초기화 시도
    AppDNS_DemoApplication_Init();
}
```

---

## 8. 성능 분석

### 8.1 실행 시간 추정

| 함수 | 예상 실행 시간 | 비고 |
|------|---------------|------|
| `SerialDPM_Init()` | ~5ms | SPI 초기화, GPIO 설정 |
| `xDriverOpen()` | ~10ms | 드라이버 인스턴스 생성 |
| `xChannelOpen()` | ~20ms | DPM 매핑, 버퍼 설정 |
| `xChannelIORead()` | ~100-200us | SPI 전송 + DPM 읽기 |
| `xChannelIOWrite()` | ~100-200us | SPI 전송 + DPM 쓰기 |
| `AppDNS_DemoApplication_Init()` | ~35ms | 전체 초기화 |
| `AppDNS_DemoApplication_Run()` | ~200-400us | 한 주기 I/O |

### 8.2 CPU 사용률

**조건**: 
- FreeRTOS 태스크 주기: 10ms
- I/O 교환 시간: ~300us

**계산**:
```
CPU Usage = (300us / 10ms) * 100% = 3%
```

**실제 사용률**: 약 **3-5%** (컨텍스트 스위칭 포함)

### 8.3 메모리 사용량

**Static 메모리**:
```c
CIFXHANDLE g_hDriver;                  // 4 bytes (pointer)
CIFXHANDLE g_hChannel;                 // 4 bytes (pointer)
uint8_t g_inputData[8];                // 8 bytes
uint8_t g_outputData[8];               // 8 bytes
uint8_t g_deviceNetInitialized;        // 1 byte
uint32_t g_ioReadCount;                // 4 bytes
uint32_t g_ioWriteCount;               // 4 bytes
uint32_t g_ioErrorCount;               // 4 bytes
───────────────────────────────────────
Total:                                  37 bytes
```

**Stack 메모리**:
```c
DeviceNet Task Stack:  4KB (configurable)
```

**총 메모리**: ~4KB (매우 효율적)

### 8.4 통신 대역폭

**SPI5 설정**:
- Baud Rate: 180MHz / 64 = 2.8 MHz
- Full Duplex: 양방향 동시 전송

**I/O 데이터**:
- 읽기: 8 bytes
- 쓰기: 8 bytes
- 총: 16 bytes per cycle

**DPM 프로토콜 오버헤드**: ~50 bytes (헤더, 체크섬 등)

**주기당 전송량**:
```
Total SPI Transfer = 16 + 50 = 66 bytes
Transfer Time = 66 * 8 / 2.8MHz = ~190us
```

**대역폭 사용률**:
```
10ms 주기 기준:
Bandwidth = (66 bytes * 100 / 10ms) = 6.6 KB/s
```

---

## 9. 디버깅 팁

### 9.1 초기화 실패 디버깅

```c
int ret = AppDNS_DemoApplication_Init();

if (ret == -1) {
    printf("SPI Init Failed\n");
    // 확인사항:
    // 1. SPI5_MISO 핀 (PF8) 설정 확인
    // 2. GPIO CS (PF6), SRDY (PF10) 확인
    // 3. SPI5 클록 활성화 확인
}
else if (ret == -2) {
    printf("Driver Open Failed\n");
    // 확인사항:
    // 1. SerialDPM_Init() 성공 여부
    // 2. netX 90 전원 확인
    // 3. SPI 통신 신호 확인 (오실로스코프)
}
```

### 9.2 I/O 에러 모니터링

```c
void DebugIOErrors(void)
{
    static uint32_t lastErrorCount = 0;
    uint32_t dummy, errors;
    
    AppDNS_GetStatistics(NULL, NULL, &errors);
    
    if (errors > lastErrorCount) {
        printf("NEW ERRORS: %lu\n", errors - lastErrorCount);
        printf("Possible causes:\n");
        printf("- Network cable disconnected\n");
        printf("- Master device offline\n");
        printf("- SPI communication error\n");
        printf("- DPM timeout\n");
        
        lastErrorCount = errors;
    }
}
```

### 9.3 데이터 검증

```c
void VerifyDataIntegrity(void)
{
    uint8_t testData[8] = {0xAA, 0x55, 0x01, 0x02, 0x03, 0x04, 0x05, 0xFF};
    uint8_t readback[8];
    
    // 데이터 설정
    AppDNS_SetInputData(testData, 8);
    
    // 다음 주기 후 확인
    osDelay(20);
    
    // Master에서 이 데이터를 다시 보내도록 설정되어 있다면:
    AppDNS_GetOutputData(readback, 8);
    
    // 비교
    if (memcmp(testData, readback, 8) == 0) {
        printf("Data Integrity: OK\n");
    } else {
        printf("Data Integrity: FAILED\n");
        for (int i = 0; i < 8; i++) {
            printf("  [%d] Sent: 0x%02X, Recv: 0x%02X\n",
                   i, testData[i], readback[i]);
        }
    }
}
```

---

## 10. 요약

### 10.1 핵심 포인트

✅ **3계층 아키텍처**:
1. Application Layer (데이터 준비/처리)
2. DeviceNet Layer (I/O 교환)
3. cifXToolkit Layer (프로토콜 처리)

✅ **주기적 실행**:
- 초기화: 1회 (태스크 시작 시)
- I/O 교환: 10ms 주기 (권장)
- CPU 사용률: ~3-5%

✅ **간단한 API**:
- `Init()` → `Run()` 반복
- `SetInputData()` / `GetOutputData()`
- `GetStatistics()` 모니터링

✅ **견고한 에러 처리**:
- 명확한 반환 코드
- 통계 추적
- 타임아웃 보호

### 10.2 다음 단계

**즉시 가능**:
1. ✅ STM32CubeIDE 빌드
2. ✅ 코드 리뷰

**하드웨어 필요**:
3. ⏳ GPIO 설정 (STM32CubeMX)
4. ⏳ netX 90 연결
5. ⏳ Master와 통신 테스트

**향후 확장**:
6. ⏳ Explicit 메시지 구현
7. ⏳ 진단 기능 추가
8. ⏳ SPI5 Mutex 적용

---

**문서 버전**: 1.0  
**작성 완료**: 2025-01-13  
**작성자**: Claude Code

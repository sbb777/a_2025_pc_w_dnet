# DeviceNet 연동 통합 계획서

**작성일:** 2025-11-13
**프로젝트:** netx_90_f429_SPI5 DeviceNet 코드를 Dasan_APC_rtos_20250110에 통합

---

## 1. 개요

### 1.1 목적
- netx_90_f429_SPI5 폴더에 있는 DeviceNet 연동 코드를 Dasan_APC_rtos_20250110 프로젝트에 통합
- STM32F429 호스트에서 netX90을 통한 DeviceNet Slave 기능 구현
- 기존 APC 애플리케이션과 DeviceNet 통신 기능의 공존

### 1.2 시스템 아키텍처
```
┌─────────────────────────────────────────────────────────────┐
│              Dasan_APC_rtos_20250110 (Target)               │
│  ┌───────────────────────────────────────────────────────┐  │
│  │ STM32F429ZG (FreeRTOS)                                │  │
│  │  ┌────────────────┐     ┌──────────────────────────┐ │  │
│  │  │  APC App       │     │ DeviceNet Integration    │ │  │
│  │  │  - PID Control │     │  - cifXToolkit           │ │  │
│  │  │  - Scheduler   │     │  - SerialDPM (SPI)       │ │  │
│  │  │  - Async Tasks │     │  - AppDNS Demo           │ │  │
│  │  └────────────────┘     └──────────────────────────┘ │  │
│  │           │                        │                  │  │
│  │           └────────┬───────────────┘                  │  │
│  │                    │ SPI5 (New DeviceNet Task)        │  │
│  └────────────────────┼──────────────────────────────────┘  │
│                       │                                      │
└───────────────────────┼──────────────────────────────────────┘
                        │ SPI5
                        ▼
           ┌────────────────────────┐
           │   netX90 (DeviceNet)   │
           │   - DeviceNet Slave FW │
           │   - CAN Controller     │
           └────────────────────────┘
                        │
                        ▼
                  DeviceNet Network
```

---

## 2. 소스 분석

### 2.1 netx_90_f429_SPI5 프로젝트 구조

```
netx_90_f429_SPI5/
├── Hil_DemoAppDNS/                      # DeviceNet Simple Config Demo
│   ├── includes/
│   │   ├── DNS_API/                     # DeviceNet 패킷 API
│   │   │   ├── DNS_packet_commands.h
│   │   │   ├── DNS_packet_set_configuration.h
│   │   │   ├── DNS_packet_cip_service.h
│   │   │   ├── DNS_packet_register_class.h
│   │   │   └── DNS_includes.h
│   │   ├── DemoAppDNS/
│   │   │   └── AppDNS_DemoApplication.h
│   │   └── AppDNS_ExplicitMsg.h         # Explicit Message 처리
│   └── Sources/
│       ├── AppDNS_DemoApplication.c     # DeviceNet 메인 애플리케이션
│       ├── AppDNS_DemoApplicationFunctions.c
│       └── AppDNS_ExplicitMsg.c
│
├── netx_tk/                             # cifXToolkit (netX 통신 라이브러리)
│   ├── Source/                          # cifXToolkit 핵심 소스
│   │   ├── cifXToolkit.h
│   │   ├── cifXInit.c
│   │   ├── cifXFunctions.c
│   │   ├── cifXHWFunctions.c
│   │   └── ...
│   ├── SerialDPM/                       # SPI 기반 DPM 인터페이스
│   │   ├── SerialDPMInterface.h
│   │   ├── SerialDPMInterface.c         # SPI Read/Write 구현
│   │   └── OS_Spi.h
│   └── OSAbstraction/                   # OS 종속적 함수 추상화
│       ├── OS_Custom.c                  # OS 함수 (Mutex, Semaphore 등)
│       ├── OS_SPICustom.c               # SPI 함수 (HAL_SPI_Transmit/Receive)
│       └── OS_Includes.h
│
├── stm32f429_main.c                     # STM32F429 메인 예제 (참조용)
└── hilscher_explicit/                   # DeviceNet Extended Config (추가 참조)
```

### 2.2 Dasan_APC_rtos_20250110 프로젝트 구조

```
Dasan_APC_rtos_20250110/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── FreeRTOSConfig.h
│   │   ├── spi.h                        # SPI1, SPI3, SPI4, SPI5 설정
│   │   └── ...
│   └── Src/
│       ├── main.c                       # STM32 메인 진입점
│       ├── freertos.c                   # FreeRTOS 태스크 정의
│       ├── spi.c
│       └── ...
│
├── App/
│   ├── logic/                           # 비즈니스 로직
│   │   ├── APC_Main.c/h                 # APC 메인 로직
│   │   ├── APC_Scheduler.c/h            # 스케줄러
│   │   ├── APC_PidHandler.c/h           # PID 제어
│   │   ├── APC_AsyncHandler.c/h         # 비동기 처리
│   │   └── cmd/                         # 명령 처리
│   └── model/
│       └── driver/                      # 하드웨어 드라이버
│           ├── APC_Spi.c/h              # SPI 드라이버
│           └── ...
│
├── Drivers/                             # STM32 HAL 드라이버
└── Middlewares/                         # FreeRTOS
```

---

## 3. 통합 계획

### 3.1 디렉토리 구조 설계

Dasan_APC_rtos_20250110 프로젝트에 다음 디렉토리를 추가:

```
Dasan_APC_rtos_20250110/
├── App/
│   ├── DeviceNet/                       # 🆕 DeviceNet 통합 모듈
│   │   ├── includes/                    # 헤더 파일
│   │   │   ├── DNS_API/                 # DNS 패킷 API (복사)
│   │   │   ├── AppDNS_DemoApplication.h
│   │   │   └── AppDNS_DeviceNetTask.h   # 🆕 FreeRTOS 태스크 헤더
│   │   └── Sources/                     # 소스 파일
│   │       ├── AppDNS_DemoApplication.c
│   │       ├── AppDNS_DemoApplicationFunctions.c
│   │       ├── AppDNS_ExplicitMsg.c
│   │       └── AppDNS_DeviceNetTask.c   # 🆕 FreeRTOS 태스크 구현
│   │
│   └── cifXToolkit/                     # 🆕 cifXToolkit 라이브러리
│       ├── Source/                      # cifX 핵심 소스 (복사)
│       ├── SerialDPM/                   # SPI DPM 인터페이스 (복사)
│       └── OSAbstraction/               # OS 추상화 레이어 (수정 필요)
│           ├── OS_Custom.c              # FreeRTOS 적용 (Mutex, Semaphore)
│           └── OS_SPICustom.c           # SPI5 HAL 적용
│
├── Core/
│   ├── Inc/
│   │   └── spi.h                        # SPI5 설정 확인
│   └── Src/
│       ├── freertos.c                   # deviceNetTask 추가 🔧
│       └── spi.c                        # SPI5 초기화 확인
```

### 3.2 주요 통합 컴포넌트

#### 3.2.1 cifXToolkit 라이브러리
**역할:** netX90 디바이스 초기화, 패킷 송수신, DPM 관리

**필요 파일:**
- `netx_tk/Source/*.c` (약 20개 파일)
- `netx_tk/Source/*.h`
- `netx_tk/SerialDPM/SerialDPMInterface.c/h`
- `netx_tk/OSAbstraction/OS_Custom.c` (FreeRTOS 수정 필요)
- `netx_tk/OSAbstraction/OS_SPICustom.c` (SPI5 HAL 적용)

**수정 사항:**
- `OS_Custom.c`: FreeRTOS Mutex, Semaphore, Timer 함수 구현
- `OS_SPICustom.c`: HAL_SPI_Transmit/Receive를 SPI5 핸들로 변경

#### 3.2.2 AppDNS DeviceNet 애플리케이션
**역할:** DeviceNet Slave 프로토콜 처리, CIP 서비스 핸들링

**필요 파일:**
- `Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.c`
- `Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c`
- `Hil_DemoAppDNS/Sources/AppDNS_ExplicitMsg.c`
- `Hil_DemoAppDNS/includes/DNS_API/*.h` (모든 헤더)
- `Hil_DemoAppDNS/includes/DemoAppDNS/AppDNS_DemoApplication.h`

**수정 사항:**
- I/O 데이터 크기 조정 (현재 Input 6 bytes, Output 10 bytes)
- APC 센서/액추에이터 데이터와 매핑

#### 3.2.3 FreeRTOS 태스크 통합
**새로운 태스크:** `deviceNetTask`

**역할:**
- cifXToolkit 초기화
- DeviceNet 스택 설정
- 주기적인 I/O 데이터 갱신
- Explicit 메시지 처리

**우선순위:** `osPriorityNormal` (defaultTask와 동일)
**스택 크기:** 512 * 4 bytes (2KB)

---

## 4. 단계별 마이그레이션 전략

### Phase 1: 기초 환경 구축 (1-2일)

#### Step 1.1: 디렉토리 생성 및 파일 복사
```bash
# Dasan_APC_rtos_20250110 프로젝트에서 실행

# 1. DeviceNet 애플리케이션 복사
mkdir -p App/DeviceNet/includes/DNS_API
mkdir -p App/DeviceNet/includes/DemoAppDNS
mkdir -p App/DeviceNet/Sources

cp -r ../netx_90_f429_SPI5/Hil_DemoAppDNS/includes/DNS_API/* \
      App/DeviceNet/includes/DNS_API/

cp ../netx_90_f429_SPI5/Hil_DemoAppDNS/includes/DemoAppDNS/* \
   App/DeviceNet/includes/DemoAppDNS/

cp ../netx_90_f429_SPI5/Hil_DemoAppDNS/Sources/* \
   App/DeviceNet/Sources/

# 2. cifXToolkit 라이브러리 복사
mkdir -p App/cifXToolkit/Source
mkdir -p App/cifXToolkit/SerialDPM
mkdir -p App/cifXToolkit/OSAbstraction

cp -r ../netx_90_f429_SPI5/netx_tk/Source/* \
      App/cifXToolkit/Source/

cp -r ../netx_90_f429_SPI5/netx_tk/SerialDPM/* \
      App/cifXToolkit/SerialDPM/

cp -r ../netx_90_f429_SPI5/netx_tk/OSAbstraction/* \
      App/cifXToolkit/OSAbstraction/
```

**검증:**
- [ ] 모든 파일이 정상 복사되었는지 확인
- [ ] 약 30개의 .c 파일, 50개의 .h 파일 복사 완료

#### Step 1.2: 프로젝트 설정 수정
**STM32CubeIDE/.cproject 파일 수정:**

1. Include Paths 추가:
   ```xml
   <option name="com.st.stm32cube.ide.mcu.gnu.managedbuild.tool.c.compiler.option.includepaths">
     <!-- 기존 include paths... -->
     <listOptionValue value="../App/DeviceNet/includes"/>
     <listOptionValue value="../App/DeviceNet/includes/DNS_API"/>
     <listOptionValue value="../App/DeviceNet/includes/DemoAppDNS"/>
     <listOptionValue value="../App/cifXToolkit/Source"/>
     <listOptionValue value="../App/cifXToolkit/SerialDPM"/>
     <listOptionValue value="../App/cifXToolkit/OSAbstraction"/>
   </option>
   ```

2. Source Locations 추가:
   ```xml
   <sourceEntries>
     <!-- 기존 source paths... -->
     <entry name="App/DeviceNet/Sources"/>
     <entry name="App/cifXToolkit/Source"/>
     <entry name="App/cifXToolkit/SerialDPM"/>
     <entry name="App/cifXToolkit/OSAbstraction"/>
   </sourceEntries>
   ```

**또는 STM32CubeIDE GUI에서:**
- Project Properties → C/C++ Build → Settings → Include paths 추가
- Project Properties → C/C++ General → Paths and Symbols 에서 Source Location 추가

**검증:**
- [ ] 프로젝트 리빌드 시 include 에러 없이 빌드 시작

---

### Phase 2: OS 추상화 레이어 구현 (2-3일)

#### Step 2.1: OS_Custom.c FreeRTOS 적용

`App/cifXToolkit/OSAbstraction/OS_Custom.c` 파일 수정:

**주요 수정 함수:**

1. **Mutex 함수:**
```c
void* OS_CreateMutex(void)
{
  return (void*)xSemaphoreCreateMutex();
}

void OS_DeleteMutex(void* pvMutex)
{
  vSemaphoreDelete((SemaphoreHandle_t)pvMutex);
}

void OS_WaitMutex(void* pvMutex, uint32_t ulTimeout)
{
  if(ulTimeout == 0xFFFFFFFF)
    xSemaphoreTake((SemaphoreHandle_t)pvMutex, portMAX_DELAY);
  else
    xSemaphoreTake((SemaphoreHandle_t)pvMutex, pdMS_TO_TICKS(ulTimeout));
}

void OS_ReleaseMutex(void* pvMutex)
{
  xSemaphoreGive((SemaphoreHandle_t)pvMutex);
}
```

2. **타이머 함수:**
```c
uint32_t OS_GetMilliSecCounter(void)
{
  return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void OS_Sleep(uint32_t ulTimeout)
{
  vTaskDelay(pdMS_TO_TICKS(ulTimeout));
}
```

3. **메모리 함수:**
```c
void* OS_Memalloc(uint32_t ulSize)
{
  return pvPortMalloc(ulSize);
}

void OS_Memfree(void* pvMem)
{
  vPortFree(pvMem);
}

void OS_Memset(void* pvMem, uint8_t bFill, uint32_t ulSize)
{
  memset(pvMem, bFill, ulSize);
}

void OS_Memcpy(void* pvDst, void* pvSrc, uint32_t ulSize)
{
  memcpy(pvDst, pvSrc, ulSize);
}
```

**검증:**
- [ ] 빌드 에러 없이 컴파일 성공
- [ ] Mutex, Timer, Memory 함수 정상 동작 확인

#### Step 2.2: OS_SPICustom.c SPI5 HAL 적용

`App/cifXToolkit/OSAbstraction/OS_SPICustom.c` 파일 수정:

```c
#include "spi.h"  // SPI5 핸들 포함
extern SPI_HandleTypeDef hspi5;  // Core/Src/spi.c에서 정의

int32_t OS_SpiTransfer(uint8_t* pabSendData, uint8_t* pabRecvData, uint32_t ulLen)
{
  HAL_StatusTypeDef status;

  if(pabSendData && pabRecvData)
  {
    // Full-duplex
    status = HAL_SPI_TransmitReceive(&hspi5, pabSendData, pabRecvData, ulLen, HAL_MAX_DELAY);
  }
  else if(pabSendData)
  {
    // Transmit only
    status = HAL_SPI_Transmit(&hspi5, pabSendData, ulLen, HAL_MAX_DELAY);
  }
  else if(pabRecvData)
  {
    // Receive only
    status = HAL_SPI_Receive(&hspi5, pabRecvData, ulLen, HAL_MAX_DELAY);
  }

  return (status == HAL_OK) ? 0 : -1;
}
```

**SPI5 CS (Chip Select) 제어:**
```c
// GPIO 핀 정의 (실제 핀 번호는 하드웨어에 맞게 수정)
#define SPI5_CS_PIN       GPIO_PIN_6
#define SPI5_CS_PORT      GPIOF

void OS_SpiCSAssert(void)
{
  HAL_GPIO_WritePin(SPI5_CS_PORT, SPI5_CS_PIN, GPIO_PIN_RESET);  // CS Low
}

void OS_SpiCSDeassert(void)
{
  HAL_GPIO_WritePin(SPI5_CS_PORT, SPI5_CS_PIN, GPIO_PIN_SET);    // CS High
}
```

**검증:**
- [ ] SPI5 전송/수신 함수 정상 동작
- [ ] CS 신호 제어 확인 (Logic Analyzer)

#### Step 2.3: SPI5 핀 설정 확인

**STM32CubeMX 또는 Core/Src/spi.c 확인:**

```c
void MX_SPI5_Init(void)
{
  hspi5.Instance = SPI5;
  hspi5.Init.Mode = SPI_MODE_MASTER;
  hspi5.Init.Direction = SPI_DIRECTION_2LINES;
  hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;   // netX90: CPOL=0
  hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;       // netX90: CPHA=0
  hspi5.Init.NSS = SPI_NSS_SOFT;               // Software CS control
  hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;  // 적절한 속도 조정
  hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi5.Init.CRCPolynomial = 10;

  if (HAL_SPI_Init(&hspi5) != HAL_OK)
  {
    Error_Handler();
  }
}
```

**핀 배치 예시 (실제 하드웨어에 맞게 수정):**
- SPI5_SCK:  PF7
- SPI5_MISO: PF8
- SPI5_MOSI: PF9
- SPI5_CS:   PF6 (GPIO Output)

**검증:**
- [ ] SPI5 클럭 속도 확인 (1-10 MHz 권장)
- [ ] CPOL=0, CPHA=0 설정 확인
- [ ] CS 핀 GPIO Output으로 설정 확인

---

### Phase 3: cifXToolkit 초기화 (1-2일)

#### Step 3.1: DeviceNet 태스크 생성

`Core/Src/freertos.c` 수정:

```c
/* USER CODE BEGIN Includes */
#include "AppDNS_DeviceNetTask.h"
/* USER CODE END Includes */

/* Definitions for deviceNetTask */
osThreadId_t deviceNetTaskHandle;
const osThreadAttr_t deviceNetTask_attributes = {
  .name = "deviceNetTask",
  .stack_size = 512 * 4,  // 2KB
  .priority = (osPriority_t) osPriorityNormal,
};

void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN RTOS_THREADS */
  deviceNetTaskHandle = osThreadNew(StartDeviceNetTask, NULL, &deviceNetTask_attributes);
  /* USER CODE END RTOS_THREADS */
}
```

#### Step 3.2: DeviceNet 태스크 구현

`App/DeviceNet/Sources/AppDNS_DeviceNetTask.c` 생성:

```c
#include "AppDNS_DeviceNetTask.h"
#include "cifXToolkit.h"
#include "SerialDPMInterface.h"
#include "AppDNS_DemoApplication.h"
#include "cmsis_os.h"

void StartDeviceNetTask(void *argument)
{
  int32_t lRet;

  /* cifXToolkit 초기화 */
  lRet = InitializeCifXToolkit();
  if(lRet != CIFX_NO_ERROR)
  {
    printf("ERROR: cifXToolkit initialization failed! Error: 0x%08X\r\n", (unsigned int)lRet);
    vTaskDelete(NULL);
    return;
  }

  /* DeviceNet 스택 초기화 */
  lRet = InitializeDeviceNetStack();
  if(lRet != CIFX_NO_ERROR)
  {
    printf("ERROR: DeviceNet stack initialization failed! Error: 0x%08X\r\n", (unsigned int)lRet);
    vTaskDelete(NULL);
    return;
  }

  printf("DeviceNet Task started successfully.\r\n");

  /* 메인 루프 */
  for(;;)
  {
    /* DeviceNet 메시지 처리 */
    ProcessDeviceNetMessages();

    /* I/O 데이터 갱신 */
    UpdateIOData();

    /* 100ms 주기 */
    osDelay(100);
  }
}

static int32_t InitializeCifXToolkit(void)
{
  int32_t lRet;

  lRet = cifXTKitInit();
  if(CIFX_NO_ERROR != lRet)
    return lRet;

  /* 디바이스 인스턴스 생성 */
  PDEVICEINSTANCE ptDevInstance = (PDEVICEINSTANCE) OS_Memalloc(sizeof(*ptDevInstance));
  OS_Memset(ptDevInstance, 0, sizeof(*ptDevInstance));

  ptDevInstance->fPCICard = 0;
  ptDevInstance->pvOSDependent = ptDevInstance;
  ptDevInstance->ulDPMSize = 0x8000;  // 32KB
  OS_Strncpy(ptDevInstance->szName, "cifX0", sizeof(ptDevInstance->szName));

  /* SerialDPM 초기화 (SPI5) */
  SerialDPM_Init(ptDevInstance);

  /* Cookie 확인 (netX90 부팅 대기) */
  if(!WaitForCookie(ptDevInstance, 5000))
  {
    printf("ERROR: netX90 not responding!\r\n");
    return CIFX_DEV_NOT_READY;
  }

  /* 디바이스 추가 */
  lRet = cifXTKitAddDevice(ptDevInstance);

  return lRet;
}

static int32_t InitializeDeviceNetStack(void)
{
  // AppDNS_DemoApplication.c의 초기화 함수 호출
  // - DeviceNet 설정
  // - CIP 클래스 등록
  // - 통신 시작

  return AppDNS_ConfigureStack(...);
}

static void ProcessDeviceNetMessages(void)
{
  // Explicit 메시지 처리
  AppDNS_HandleCipServiceIndication(...);
}

static void UpdateIOData(void)
{
  // Input Data: Master → Slave (제어 데이터)
  // Output Data: Slave → Master (센서 데이터)

  uint8_t abInputData[6];
  uint8_t abOutputData[10];

  // 채널에서 Input 데이터 읽기
  xChannelIORead(..., abInputData, sizeof(abInputData));

  // APC 애플리케이션에 데이터 전달 (예: LED 제어)
  ProcessInputData(abInputData);

  // APC 센서 데이터 읽기
  GetSensorData(abOutputData);

  // 채널에 Output 데이터 쓰기
  xChannelIOWrite(..., abOutputData, sizeof(abOutputData));
}
```

**검증:**
- [ ] deviceNetTask 정상 생성
- [ ] cifXToolkit 초기화 성공
- [ ] netX90 Cookie 확인 완료

---

### Phase 4: DeviceNet 스택 설정 (2-3일)

#### Step 4.1: DeviceNet 설정 구조체

`AppDNS_DemoApplication.c`의 설정 함수 수정:

```c
uint32_t AppDNS_ConfigureStack(APP_DATA_T* ptAppData, uint32_t ulBusOnDelay)
{
  DNS_CMD_SET_CONFIGURATION_REQ_T tSetConfigReq = {0};

  tSetConfigReq.tData.bMacId = 63;  // DeviceNet MAC ID (DIP 스위치로 변경 가능)
  tSetConfigReq.tData.bBaudrate = DNS_BAUDRATE_125KBAUD;  // 125 kbps

  tSetConfigReq.tData.ulVendorId = 0x0123;      // Vendor ID (실제 값으로 변경)
  tSetConfigReq.tData.ulProductCode = 0x4567;   // Product Code
  tSetConfigReq.tData.bMajorRevision = 1;
  tSetConfigReq.tData.bMinorRevision = 0;
  tSetConfigReq.tData.ulSerialNumber = 0x00000001;

  tSetConfigReq.tData.usProducingConnSize = 10;  // Output Data Size
  tSetConfigReq.tData.usConsumingConnSize = 6;   // Input Data Size

  tSetConfigReq.tData.ulBusOnDelay = ulBusOnDelay;

  // 설정 패킷 전송
  return SendConfigPacket(&tSetConfigReq);
}
```

#### Step 4.2: CIP 클래스 등록

VAT (Vendor Application Task)를 위한 커스텀 CIP 클래스 등록:

```c
uint32_t AppDNS_RegisterAllVatClasses(APP_DATA_T* ptAppData)
{
  uint32_t ulRet;

  // Identity Object (Class 0x01)
  ulRet = RegisterIdentityClass();
  if(ulRet != CIFX_NO_ERROR)
    return ulRet;

  // Assembly Object (Class 0x04)
  ulRet = RegisterAssemblyClass();
  if(ulRet != CIFX_NO_ERROR)
    return ulRet;

  // Connection Manager (Class 0x06)
  ulRet = RegisterConnectionManagerClass();
  if(ulRet != CIFX_NO_ERROR)
    return ulRet;

  // User Object (Custom Class)
  ulRet = RegisterUserObjectClass();
  if(ulRet != CIFX_NO_ERROR)
    return ulRet;

  return CIFX_NO_ERROR;
}
```

**검증:**
- [ ] DeviceNet 설정 패킷 전송 성공
- [ ] CIP 클래스 등록 완료
- [ ] Master에서 Slave 검색 가능

#### Step 4.3: EDS 파일 준비

`Hil_DemoAppDNS/DeviceDescription/DNS_V5_SIMPLE_CONFIG_DEMO.EDS` 파일을 기반으로 커스텀 EDS 생성:

**주요 수정 사항:**
```ini
[Device]
VendorID=0x0123
ProductCode=0x4567
MajorRevision=1
MinorRevision=0

[Params]
Param1=6   ; Input Data Size (bytes)
Param2=10  ; Output Data Size (bytes)

[Assembly]
Assem100=6  ; Input Assembly (Consuming)
Assem101=10 ; Output Assembly (Producing)
```

**검증:**
- [ ] EDS 파일을 DeviceNet Master 설정 도구에 임포트 성공

---

### Phase 5: I/O 데이터 매핑 (2-3일)

#### Step 5.1: 데이터 구조 정의

`App/DeviceNet/includes/AppDNS_IOMapping.h` 생성:

```c
#ifndef APPDNS_IOMAPPING_H
#define APPDNS_IOMAPPING_H

#include <stdint.h>

/* Input Data: Master → Slave (6 bytes) */
typedef struct __attribute__((packed))
{
  uint8_t  bControlMode;       // 0: Manual, 1: Auto
  uint8_t  bTargetSetpoint;    // Target setpoint (0-100%)
  uint16_t usReserved1;
  uint16_t usControlFlags;     // Bit flags for control
} DEVICENET_INPUT_DATA_T;

/* Output Data: Slave → Master (10 bytes) */
typedef struct __attribute__((packed))
{
  uint16_t usSensorValue1;     // Sensor 1 (e.g., Pressure)
  uint16_t usSensorValue2;     // Sensor 2 (e.g., Temperature)
  uint8_t  bStatusFlags;       // Status flags
  uint8_t  bErrorCode;         // Error code
  uint16_t usActualSetpoint;   // Actual setpoint
  uint16_t usReserved2;
} DEVICENET_OUTPUT_DATA_T;

#endif
```

#### Step 5.2: APC 애플리케이션 연동

`App/DeviceNet/Sources/AppDNS_IOMapping.c` 생성:

```c
#include "AppDNS_IOMapping.h"
#include "APC_Main.h"
#include "APC_Sensors.h"
#include "APC_Actuators.h"

static DEVICENET_INPUT_DATA_T  g_tInputData;
static DEVICENET_OUTPUT_DATA_T g_tOutputData;

void ProcessInputData(uint8_t* pabInputData)
{
  memcpy(&g_tInputData, pabInputData, sizeof(g_tInputData));

  // APC 애플리케이션에 제어 데이터 전달
  APC_SetControlMode(g_tInputData.bControlMode);
  APC_SetTargetSetpoint(g_tInputData.bTargetSetpoint);
  APC_SetControlFlags(g_tInputData.usControlFlags);
}

void GetSensorData(uint8_t* pabOutputData)
{
  // APC 센서 데이터 읽기
  g_tOutputData.usSensorValue1 = APC_GetPressure();
  g_tOutputData.usSensorValue2 = APC_GetTemperature();
  g_tOutputData.bStatusFlags = APC_GetStatusFlags();
  g_tOutputData.bErrorCode = APC_GetErrorCode();
  g_tOutputData.usActualSetpoint = APC_GetActualSetpoint();

  memcpy(pabOutputData, &g_tOutputData, sizeof(g_tOutputData));
}
```

**APC_Main.c 수정:**

```c
// DeviceNet에서 제어 명령 수신 시 호출
void APC_SetControlMode(uint8_t mode)
{
  // 기존 APC 제어 로직 수정
  if(mode == 0)
  {
    // Manual mode
    setControlMode(MANUAL_MODE);
  }
  else if(mode == 1)
  {
    // Auto mode
    setControlMode(AUTO_MODE);
  }
}

void APC_SetTargetSetpoint(uint8_t setpoint)
{
  // PID 컨트롤러에 목표값 설정
  setPIDTarget(setpoint);
}
```

**검증:**
- [ ] Master에서 제어 데이터 전송 시 APC 동작 변경 확인
- [ ] Slave 센서 데이터가 Master에 정상 수신

---

### Phase 6: Explicit 메시지 처리 (1-2일)

#### Step 6.1: CIP Service 핸들러 구현

`AppDNS_ExplicitMsg.c` 수정:

```c
void AppDNS_HandleCipServiceIndication(APP_DATA_T* ptAppData)
{
  CIFX_PACKET tPacket;

  // CIP Service Indication 수신
  if(ReceiveCIPServicePacket(&tPacket) == CIFX_NO_ERROR)
  {
    DNS_CMD_CIP_SERVICE_IND_T* ptInd = (DNS_CMD_CIP_SERVICE_IND_T*)&tPacket;

    switch(ptInd->tData.bService)
    {
      case CIP_SERVICE_GET_ATTRIBUTE_SINGLE:
        HandleGetAttributeSingle(ptInd);
        break;

      case CIP_SERVICE_SET_ATTRIBUTE_SINGLE:
        HandleSetAttributeSingle(ptInd);
        break;

      case CIP_SERVICE_RESET:
        HandleReset(ptInd);
        break;

      default:
        SendCIPServiceError(ptInd, CIP_GSC_SERVICE_NOT_SUPPORTED);
        break;
    }
  }
}
```

**검증:**
- [ ] Master에서 Get Attribute 명령 전송 시 응답 확인
- [ ] Set Attribute로 파라미터 변경 가능

---

### Phase 7: 테스트 및 디버깅 (3-5일)

#### Step 7.1: 유닛 테스트

**테스트 항목:**
1. cifXToolkit 초기화
   - [ ] SerialDPM SPI5 통신 정상
   - [ ] netX90 Cookie 확인
   - [ ] 디바이스 등록 성공

2. DeviceNet 스택
   - [ ] 설정 패킷 전송/응답
   - [ ] CIP 클래스 등록 완료
   - [ ] 통신 시작 성공

3. I/O 데이터
   - [ ] Input 데이터 수신 정상
   - [ ] Output 데이터 송신 정상
   - [ ] 데이터 포맷 일치

4. Explicit 메시지
   - [ ] Get Attribute 응답
   - [ ] Set Attribute 동작
   - [ ] Error 처리

#### Step 7.2: 통합 테스트

**시나리오 1: Master 스캔**
1. Master에서 DeviceNet 스캔 실행
2. Slave (MAC ID 63) 발견 확인
3. Identity 정보 읽기 성공

**시나리오 2: I/O 데이터 교환**
1. Master에서 제어 데이터 송신
2. Slave에서 APC 제어 동작 확인
3. Slave 센서 데이터를 Master에서 수신
4. 실시간 데이터 모니터링

**시나리오 3: 파라미터 변경**
1. Master에서 Set Attribute 전송
2. Slave 파라미터 변경 확인
3. Get Attribute로 변경 값 확인

#### Step 7.3: 디버깅 도구

**UART 디버그 출력:**
```c
// Core/Src/syscalls.c 또는 별도 파일
int _write(int file, char *ptr, int len)
{
  HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
  return len;
}
```

**디버그 메시지 출력:**
```c
printf("[DeviceNet] cifXToolkit initialized.\r\n");
printf("[DeviceNet] Input Data: %02X %02X %02X %02X %02X %02X\r\n", ...);
printf("[DeviceNet] Output Data: %04X %04X %02X %02X\r\n", ...);
```

**Live Expressions (STM32CubeIDE):**
- `g_tInputData`
- `g_tOutputData`
- `g_ulTraceLevel`

**검증:**
- [ ] UART 디버그 출력 정상
- [ ] Live Expression으로 실시간 데이터 확인

---

## 5. 리소스 및 메모리 관리

### 5.1 메모리 사용량 예상

| 컴포넌트                  | Flash (KB) | RAM (KB) |
|---------------------------|------------|----------|
| cifXToolkit 라이브러리    | 80         | 10       |
| AppDNS 애플리케이션       | 40         | 8        |
| DeviceNet Task 스택       | -          | 2        |
| DPM 버퍼                  | -          | 32       |
| 총계                      | ~120       | ~52      |

**기존 프로젝트 여유 공간 확인 필요**

### 5.2 FreeRTOS Heap 설정

`Core/Inc/FreeRTOSConfig.h` 확인:

```c
#define configTOTAL_HEAP_SIZE    ((size_t)(64 * 1024))  // 64KB 이상 권장
```

필요 시 Heap 크기 증가:
```c
#define configTOTAL_HEAP_SIZE    ((size_t)(96 * 1024))  // 96KB로 증가
```

### 5.3 스택 크기 조정

각 태스크 스택 크기 점검:
- defaultTask: 256 * 4 = 1024 bytes
- asyncTask: 128 * 4 = 512 bytes
- pidTask: 128 * 4 = 512 bytes
- pollTask: 128 * 4 = 512 bytes
- **deviceNetTask: 512 * 4 = 2048 bytes** (새로 추가)

**총 스택 사용:** ~4.5 KB

---

## 6. 위험 요소 및 대응 방안

### 6.1 SPI 통신 불안정
**증상:** DPM 읽기/쓰기 실패, Cookie 확인 실패

**원인:**
- SPI 클럭 속도 과다
- 타이밍 이슈
- 하드웨어 노이즈

**대응:**
1. SPI 클럭 속도 낮추기 (예: 1 MHz)
2. CS Assert/Deassert 타이밍 조정
3. Pull-up/Pull-down 저항 추가
4. 로직 분석기로 파형 확인

### 6.2 FreeRTOS 스택 오버플로
**증상:** Hard Fault, 시스템 리셋

**원인:**
- deviceNetTask 스택 크기 부족
- 깊은 함수 호출 스택
- 큰 로컬 변수 사용

**대응:**
1. 스택 크기 증가 (512*4 → 1024*4)
2. configCHECK_FOR_STACK_OVERFLOW 활성화
3. 로컬 변수를 static 또는 동적 할당으로 변경

### 6.3 타이밍 충돌
**증상:** PID 제어 성능 저하, 센서 읽기 누락

**원인:**
- deviceNetTask가 CPU 점유
- 기존 태스크 우선순위 충돌

**대응:**
1. deviceNetTask 우선순위 조정 (Normal → Below Normal)
2. osDelay() 주기 조정 (100ms → 50ms)
3. Mutex를 사용한 공유 데이터 보호

### 6.4 netX90 펌웨어 미부팅
**증상:** Cookie 확인 실패, 타임아웃

**원인:**
- netX90 전원 미공급
- JTAG 연결 문제
- 펌웨어 미다운로드

**대응:**
1. netX90 전원 확인
2. JTAG로 펌웨어 다운로드
3. `doio_chaselight_with_dnetFW_spi0` 프로젝트 빌드 및 다운로드

---

## 7. 참고 문서

### 7.1 Hilscher 문서
- cifXToolkit User Manual
- netX DPM Interface Manual (netX_Dual-Port_Memory_Interface_DPM_17_EN.pdf)
- DeviceNet Slave Protocol API Manual

### 7.2 기존 프로젝트 문서
- `20251027_VAT_Integration_Work_History.md`: VAT 통합 작업 이력
- `20251111_VAT_ExplicitMessage_TestGuide.md`: Explicit Message 테스트 가이드
- `20251027_DNS_V5_EDS_Analysis.md`: EDS 파일 분석
- `STM32_DeviceNet_Debug_Guide.md`: DeviceNet 디버깅 가이드

### 7.3 코드 예제
- `netx_90_f429_SPI5/stm32f429_main.c`: STM32 메인 예제
- `netx_90_f429_SPI5/Hil_DemoAppDNS/`: DeviceNet 애플리케이션 예제

---

## 8. 일정 계획

| Phase | 작업 내용                           | 예상 기간 | 담당자 |
|-------|-------------------------------------|-----------|--------|
| 1     | 기초 환경 구축                      | 1-2일     |        |
| 2     | OS 추상화 레이어 구현               | 2-3일     |        |
| 3     | cifXToolkit 초기화                  | 1-2일     |        |
| 4     | DeviceNet 스택 설정                 | 2-3일     |        |
| 5     | I/O 데이터 매핑                     | 2-3일     |        |
| 6     | Explicit 메시지 처리                | 1-2일     |        |
| 7     | 테스트 및 디버깅                    | 3-5일     |        |
| **총계** |                                  | **12-20일** |        |

---

## 9. 체크리스트

### 환경 구축
- [ ] App/DeviceNet 디렉토리 생성 및 파일 복사
- [ ] App/cifXToolkit 디렉토리 생성 및 파일 복사
- [ ] Include paths 추가
- [ ] Source locations 추가
- [ ] 빌드 에러 없이 컴파일 성공

### OS 추상화
- [ ] OS_Custom.c FreeRTOS 함수 구현
- [ ] OS_SPICustom.c SPI5 HAL 적용
- [ ] SPI5 핀 설정 확인
- [ ] Mutex, Timer, Memory 함수 정상 동작

### cifXToolkit
- [ ] deviceNetTask 생성
- [ ] cifXTKitInit() 성공
- [ ] SerialDPM_Init() 성공
- [ ] Cookie 확인 완료
- [ ] cifXTKitAddDevice() 성공

### DeviceNet 스택
- [ ] SET_CONFIGURATION 패킷 전송 성공
- [ ] CIP 클래스 등록 완료
- [ ] START_STOP_COMM 성공
- [ ] Master에서 Slave 검색 가능

### I/O 데이터
- [ ] Input 데이터 수신 정상
- [ ] Output 데이터 송신 정상
- [ ] APC 애플리케이션 연동 성공
- [ ] 실시간 데이터 모니터링

### Explicit 메시지
- [ ] Get Attribute 응답
- [ ] Set Attribute 동작
- [ ] CIP Service 핸들러 정상

### 테스트
- [ ] 유닛 테스트 완료
- [ ] 통합 테스트 완료
- [ ] 장시간 안정성 테스트

---

## 10. 결론

본 통합 계획서는 netx_90_f429_SPI5의 DeviceNet 연동 코드를 Dasan_APC_rtos_20250110 프로젝트에 체계적으로 통합하기 위한 로드맵을 제시합니다.

핵심 성공 요소:
1. **OS 추상화 레이어의 정확한 구현** (FreeRTOS 적용)
2. **SPI5 통신의 안정성 확보** (타이밍, 클럭 속도)
3. **기존 APC 애플리케이션과의 원활한 연동** (데이터 매핑)
4. **충분한 테스트 및 디버깅 시간 확보**

단계별로 검증하며 진행하면 안정적인 DeviceNet 통신 기능을 구현할 수 있습니다.

---

**문서 버전:** 1.0
**최종 수정:** 2025-11-13

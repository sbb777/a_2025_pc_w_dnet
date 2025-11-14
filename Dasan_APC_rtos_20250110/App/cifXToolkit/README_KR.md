# cifXToolkit 라이브러리

이 디렉토리는 netX 90 칩과 인터페이스하기 위한 Hilscher cifXToolkit 라이브러리를 포함합니다.

## 디렉토리 구조

```
cifXToolkit/
├── Source/                          # cifX 툴킷 핵심 소스
│   └── README.md                   # 툴킷 파일 복사 지침
│
├── SerialDPM/                       # Serial DPM (SPI) 인터페이스
│   └── README.md                   # SPI DPM 파일 지침
│
└── OSAbstraction/                   # OS 및 하드웨어 추상화
    ├── README_KR.md                # 통합 가이드라인
    ├── OS_Custom.c                 # FreeRTOS 적용
    └── OS_SPICustom.c              # STM32 HAL SPI5 적용
```

## 개요

cifXToolkit은 Hilscher netX 칩에 액세스하기 위한 표준화된 API를 제공합니다. 이 프로젝트에서는 DeviceNet 프로토콜 처리를 위해 SPI5 인터페이스를 통한 netX 90 칩과의 통신을 가능하게 합니다.

## 통합 단계

### 1. cifXToolkit 핵심 파일 복사

Hilscher cifXToolkit 라이브러리에서 핵심 cifX 파일을 `Source/`로 복사합니다:

복사할 파일의 전체 목록은 `Source/README.md`를 참조하십시오.

주요 파일:
- `cifXToolkit.h` - 메인 API 헤더
- `cifXFunctions.c` - 핵심 API 구현
- `cifXHWFunctions.c` - 하드웨어 추상화
- `NetX_RegDefs.h` - netX 레지스터 정의
- DPM 구조 정의

### 2. SerialDPM 파일 복사

SPI DPM 인터페이스 파일을 `SerialDPM/`로 복사합니다:

자세한 내용은 `SerialDPM/README.md`를 참조하십시오.

**중요**: **netX 90** 전용 구현을 사용하십시오:
- `NetX90_SPIDPMInterface.h`
- `NetX90_SPIDPMInterface.c`

### 3. OS 추상화 레이어

OS 추상화 레이어는 `OSAbstraction/`에 구현되어 있습니다:

#### OS_Custom.c (FreeRTOS 통합)
FreeRTOS 전용 구현을 제공합니다:
- **Mutex 작업**: FreeRTOS 세마포어 사용
- **메모리 관리**: FreeRTOS 힙 사용
- **시간 함수**: FreeRTOS 틱 카운터 사용
- **문자열 작업**: 표준 C 라이브러리 사용

#### OS_SPICustom.c (STM32 HAL SPI5 통합)
STM32 하드웨어 전용 구현을 제공합니다:
- **SPI 전송**: HAL_SPI_TransmitReceive() 사용
- **GPIO 제어**: CS (Chip Select) 신호
- **핸드셰이킹**: SRDY (Service Ready) 모니터링

### 4. STM32CubeMX SPI5 설정

STM32CubeMX에서 다음 파라미터로 SPI5를 설정합니다:

```
모드: Master
Hardware NSS: Disable (소프트웨어 제어)
방향: Full Duplex Master
데이터 크기: 8 Bits
First Bit: MSB First

클럭 파라미터:
  Prescaler: (최대 20 MHz SPI 클럭을 달성하도록 조정)
  Clock Polarity (CPOL): Low
  Clock Phase (CPHA): 1 Edge
  CRC Calculation: Disabled
  NSS Signal Type: Software

GPIO 설정:
  SPI5_SCK:  클럭 출력
  SPI5_MISO: 데이터 입력
  SPI5_MOSI: 데이터 출력
  CS:        GPIO 출력 (수동 제어)
  SRDY:      GPIO 입력 (풀업 포함)
```

### 5. 하드웨어 연결

STM32F429와 netX 90 간의 다음 연결을 확인하십시오:

| STM32F429 | netX 90 | 신호 | 방향 |
|-----------|---------|--------|-----------|
| SPI5_SCK  | SPI_CLK | 클럭  | 출력       |
| SPI5_MOSI | SPI_MOSI| 데이터   | 출력       |
| SPI5_MISO | SPI_MISO| 데이터   | 입력        |
| GPIO (CS) | SPI_CS  | 칩 선택 | 출력  |
| GPIO      | SRDY    | 서비스 준비 | 입력  |
| GND       | GND     | 그라운드 | -         |

**중요**: 전압 레벨이 호환되는지 확인하십시오 (3.3V 로직).

### 6. GPIO 핀 할당

`OSAbstraction/OS_SPICustom.c`의 GPIO 핀 정의를 업데이트하십시오:

```c
/* TODO: SPI5 핸드셰이킹용 GPIO 핀 정의 */
#define NETX_CS_PIN         GPIO_PIN_6    /* 예: SPI5_CS on PF6 */
#define NETX_CS_PORT        GPIOF

#define NETX_SRDY_PIN       GPIO_PIN_7    /* 예: SRDY on PF7 */
#define NETX_SRDY_PORT      GPIOF
```

하드웨어 설계의 실제 핀 할당으로 교체하십시오.

## API 사용법

### cifX 디바이스 초기화

```c
#include "cifXToolkit.h"

int32_t lRet;
CIFXHANDLE hDriver = NULL;
CIFXHANDLE hDevice = NULL;

/* 드라이버 초기화 */
lRet = xDriverOpen(&hDriver);
if (lRet != CIFX_NO_ERROR)
{
    /* 에러 처리 */
}

/* 디바이스 열기 */
lRet = xChannelOpen(hDriver, "cifX0", 0, &hDevice);
if (lRet != CIFX_NO_ERROR)
{
    /* 에러 처리 */
}
```

### 패킷 송수신

```c
CIFX_PACKET tSendPacket;
CIFX_PACKET tRecvPacket;
uint32_t ulRecvLen;

/* 전송 패킷 준비 */
/* ... tSendPacket 채우기 ... */

/* 패킷 전송 및 응답 대기 */
lRet = xChannelPutPacket(hDevice, &tSendPacket, 1000);
if (lRet == CIFX_NO_ERROR)
{
    lRet = xChannelGetPacket(hDevice, sizeof(tRecvPacket),
                             &tRecvPacket, &ulRecvLen, 1000);
}
```

### I/O 데이터 액세스

```c
uint8_t abInputData[32];
uint8_t abOutputData[32];
uint32_t ulBytesRead;

/* DPM에서 입력 데이터 읽기 */
lRet = xChannelIORead(hDevice, 0, 0, sizeof(abInputData),
                      abInputData, &ulBytesRead, 100);

/* DPM에 출력 데이터 쓰기 */
lRet = xChannelIOWrite(hDevice, 0, 0, sizeof(abOutputData),
                       abOutputData, 100);
```

## SPI DPM 액세스 프로토콜

### DPM 읽기 프로토콜

```
1. Mutex 획득
   └─ OS_WaitMutex()

2. CS 활성화 (LOW)
   └─ HAL_GPIO_WritePin(CS_PIN, LOW)

3. SRDY 대기 (HIGH)
   └─ while(SRDY != HIGH) { delay(10us); }

4. 읽기 명령 전송
   ├─ 명령 바이트: 0x00
   ├─ 주소 High byte
   └─ 주소 Low byte

5. 데이터 수신
   └─ N 바이트 읽기

6. CS 비활성화 (HIGH)
   └─ HAL_GPIO_WritePin(CS_PIN, HIGH)

7. SRDY 해제 대기 (LOW)
   └─ while(SRDY != LOW) { delay(10us); }

8. Mutex 해제
   └─ OS_ReleaseMutex()
```

### DPM 쓰기 프로토콜

```
1. Mutex 획득
   └─ OS_WaitMutex()

2. CS 활성화 (LOW)
   └─ HAL_GPIO_WritePin(CS_PIN, LOW)

3. SRDY 대기 (HIGH)
   └─ while(SRDY != HIGH) { delay(10us); }

4. 쓰기 명령 전송
   ├─ 명령 바이트: 0x80
   ├─ 주소 High byte
   └─ 주소 Low byte

5. 데이터 전송
   └─ N 바이트 쓰기

6. CS 비활성화 (HIGH)
   └─ HAL_GPIO_WritePin(CS_PIN, HIGH)

7. SRDY 해제 대기 (LOW)
   └─ while(SRDY != LOW) { delay(10us); }

8. Mutex 해제
   └─ OS_ReleaseMutex()
```

## 타이밍 요구사항

### SPI 통신
- **SPI 클럭**: 최대 20 MHz (케이블 길이 및 노이즈에 따라 조정)
- **CS 셋업 타임**: 첫 번째 클럭 엣지 전 최소 10ns
- **CS 홀드 타임**: 마지막 클럭 엣지 후 최소 10ns
- **SRDY 폴링**: 각 DPM 액세스 전 확인
- **타임아웃**: 합리적인 타임아웃 구현 (일반적으로 100-1000ms)

### DPM 액세스
- **핸드셰이크 프로토콜**:
  1. CS를 LOW로 설정
  2. SRDY가 HIGH가 될 때까지 대기
  3. SPI 전송 수행
  4. CS를 HIGH로 설정
  5. SRDY가 LOW가 될 때까지 대기

### 타이밍 다이어그램

```
CS    ────┐        ┌────────
          └────────┘

SRDY  ────────┐    ┌────────
              └────┘

SPI         ┌─┬─┬─┬─┐
CLK   ──────┘ └─┘ └─┘ └─────

         CMD  AH  AL  D0  D1
MOSI  ───┤  ├──├──├──├──├───
         └──┘  └──┘  └──┘

         XX   XX  XX  D0  D1
MISO  ───┤  ├──├──├──├──├───
         └──┘  └──┘  └──┘

타이밍:
- CS setup: 10ns 최소
- SRDY response: 1us 전형적
- 바이트 간 간격: 없음 (연속 전송)
- CS hold: 10ns 최소
```

## 에러 처리

일반적인 에러 및 해결 방법:

| 에러 | 원인 | 해결 방법 |
|-------|-------|----------|
| `CIFX_DEV_NOT_READY` | netX가 초기화되지 않음 | netX 펌웨어 확인, SRDY 신호 확인 |
| `CIFX_DEV_NO_COM_FLAG` | DPM 통신 플래그 누락 | DPM 구조 확인, SPI 타이밍 확인 |
| `CIFX_DEV_EXCHANGE_FAILED` | 데이터 교환 타임아웃 | SRDY 핸드셰이킹 확인, SPI 전송 확인 |
| `CIFX_INVALID_PARAMETER` | 잘못된 API 파라미터 | 버퍼 크기 및 포인터 확인 |

### 에러 복구 예제

```c
/* cifX 디바이스 초기화 (재시도 포함) */
int32_t InitCifXDeviceWithRetry(CIFXHANDLE* phDevice)
{
    int32_t lRet;
    int retryCount = 0;
    const int MAX_RETRIES = 3;

    while (retryCount < MAX_RETRIES)
    {
        lRet = xChannelOpen(hDriver, "cifX0", 0, phDevice);

        if (lRet == CIFX_NO_ERROR)
        {
            return 0;  /* 성공 */
        }

        /* 에러 로깅 */
        LogError("cifX init failed, error=0x%08X, retry=%d",
                 lRet, retryCount);

        /* 재시도 전 대기 */
        vTaskDelay(pdMS_TO_TICKS(1000));
        retryCount++;
    }

    return -1;  /* 실패 */
}
```

## 메모리 사용량

일반적인 메모리 요구사항:
- **RAM**: DPM 버퍼 및 내부 구조를 위한 약 16KB
- **Flash**: cifXToolkit 라이브러리 코드를 위한 약 64KB
- **스택**: cifX API를 사용하는 스레드당 2-4KB

`FreeRTOSConfig.h`에서 FreeRTOS 힙 크기를 적절히 조정하십시오:
```c
#define configTOTAL_HEAP_SIZE    ((size_t)(64 * 1024))  /* 64KB 힙 */
```

## 스레드 안전성

cifXToolkit은 mutex를 사용하여 스레드 안전성을 보장합니다:
- 여러 태스크가 동일한 디바이스에 안전하게 액세스할 수 있습니다
- Mutex 타임아웃은 API 호출마다 설정할 수 있습니다
- ISR에서 cifX 함수를 호출하지 마십시오

### 멀티태스크 사용 예제

```c
/* 태스크 1: I/O 데이터 읽기 */
void Task_ReadIO(void *argument)
{
    uint8_t inputData[32];
    uint32_t bytesRead;

    for (;;)
    {
        /* Mutex로 보호된 I/O 읽기 */
        xChannelIORead(hDevice, 0, 0, sizeof(inputData),
                       inputData, &bytesRead, 100);

        /* 데이터 처리 */
        ProcessInputData(inputData, bytesRead);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* 태스크 2: I/O 데이터 쓰기 */
void Task_WriteIO(void *argument)
{
    uint8_t outputData[32];

    for (;;)
    {
        /* 출력 데이터 준비 */
        PrepareOutputData(outputData);

        /* Mutex로 보호된 I/O 쓰기 */
        xChannelIOWrite(hDevice, 0, 0, sizeof(outputData),
                        outputData, 100);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

## 디버깅

디버그 출력을 활성화하려면 다음을 정의하십시오:
```c
#define CIFX_TOOLKIT_DEBUG  1
```

이렇게 하면 설정된 디버그 인터페이스를 통해 진단 메시지가 출력됩니다.

### 디버그 출력 예제

```c
#if CIFX_TOOLKIT_DEBUG
#include <stdio.h>
#define CIFX_DEBUG_PRINT(...)  printf(__VA_ARGS__)
#else
#define CIFX_DEBUG_PRINT(...)
#endif

/* SPI 전송 디버깅 */
int32_t SPI_Transfer_Debug(uint8_t* pbSend, uint8_t* pbRecv, uint32_t ulLen)
{
    CIFX_DEBUG_PRINT("SPI Transfer: len=%d\r\n", ulLen);

    /* 전송 데이터 출력 */
    for (uint32_t i = 0; i < ulLen && i < 16; i++)
    {
        CIFX_DEBUG_PRINT("  TX[%d]=0x%02X\r\n", i, pbSend[i]);
    }

    /* 전송 수행 */
    int32_t ret = SPI_Transfer(pbSend, pbRecv, ulLen);

    /* 수신 데이터 출력 */
    if (ret == 0)
    {
        for (uint32_t i = 0; i < ulLen && i < 16; i++)
        {
            CIFX_DEBUG_PRINT("  RX[%d]=0x%02X\r\n", i, pbRecv[i]);
        }
    }

    return ret;
}
```

### Logic Analyzer 사용

SPI 통신 디버깅을 위해 로직 분석기를 사용하십시오:

모니터링할 신호:
- SPI_CLK: SPI 클럭
- SPI_MOSI: Master → Slave 데이터
- SPI_MISO: Slave → Master 데이터
- CS: 칩 선택
- SRDY: 서비스 준비

확인할 사항:
- CS 활성화 전에 SRDY가 HIGH인지
- SPI 클럭 주파수가 20 MHz 이하인지
- 데이터가 MSB first로 전송되는지
- CS 비활성화 후 SRDY가 LOW로 돌아오는지

## 성능 최적화

### DPM 액세스 최적화

```c
/* 잘못된 예: 여러 번 작은 읽기 수행 */
uint8_t byte1, byte2, byte3;
xChannelIORead(hDevice, 0, 0, 1, &byte1, &bytesRead, 100);
xChannelIORead(hDevice, 0, 1, 1, &byte2, &bytesRead, 100);
xChannelIORead(hDevice, 0, 2, 1, &byte3, &bytesRead, 100);

/* 올바른 예: 한 번에 읽기 수행 */
uint8_t data[3];
xChannelIORead(hDevice, 0, 0, 3, data, &bytesRead, 100);
```

### 버퍼 정렬

```c
/* DPM 버퍼는 4바이트 정렬 권장 */
__attribute__((aligned(4))) uint8_t inputBuffer[32];
__attribute__((aligned(4))) uint8_t outputBuffer[32];
```

## 참조

- cifXToolkit API 레퍼런스 매뉴얼
- netX 90 Dual-Port Memory 인터페이스 매뉴얼
- netX 90 SPI DPM 애플리케이션 노트
- Hilscher 지식 베이스: https://kb.hilscher.com

## 참고사항

- SerialDPM 레이어는 DPM 액세스를 위한 저수준 SPI 프로토콜을 처리합니다
- SRDY 신호는 적절한 핸드셰이킹에 중요합니다 - 올바르게 연결되었는지 확인하십시오
- SPI 클럭 속도는 PCB 레이아웃 및 케이블 길이에 따라 조정되어야 합니다
- 내부 풀업을 사용하지 않는 경우 SRDY에 외부 풀업 저항을 사용하십시오
- 노이즈가 많은 산업 환경에서는 SPI 라인에 EMI 필터링을 추가하는 것을 고려하십시오

## 문제 해결 체크리스트

### 1. 하드웨어 확인
- [ ] SPI5 핀이 올바르게 연결됨
- [ ] CS 신호가 작동함
- [ ] SRDY 신호가 올바르게 읽힘
- [ ] 전압 레벨이 호환됨 (3.3V)
- [ ] 그라운드가 연결됨

### 2. 소프트웨어 확인
- [ ] SPI5가 STM32CubeMX에서 설정됨
- [ ] GPIO 핀이 OS_SPICustom.c에서 올바르게 정의됨
- [ ] cifXToolkit 파일이 모두 복사됨
- [ ] Include 경로가 빌드 시스템에 추가됨
- [ ] FreeRTOS 힙 크기가 충분함

### 3. 통신 확인
- [ ] xDriverOpen()이 성공함
- [ ] xChannelOpen()이 성공함
- [ ] DPM 매직 넘버가 확인됨
- [ ] 통신 플래그가 설정됨
- [ ] SRDY 핸드셰이킹이 작동함

### 4. netX 확인
- [ ] netX 90 펌웨어가 로드됨
- [ ] 펌웨어 버전이 호환됨
- [ ] SPI DPM 모드가 활성화됨
- [ ] 네트워크 설정이 올바름

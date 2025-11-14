# OS 추상화 레이어

이 디렉토리는 cifXToolkit을 위한 운영 체제 추상화 레이어를 포함합니다.

## 파일 생성/수정

### 1. OS_Custom.c (FreeRTOS 적용)
FreeRTOS 전용 구현을 제공하기 위해 이 파일을 생성해야 합니다:

#### 필수 함수:
```c
/* Mutex 함수 */
void* OS_CreateMutex(void);
int   OS_WaitMutex(void* pvMutex, uint32_t ulTimeout);
void  OS_ReleaseMutex(void* pvMutex);
void  OS_DeleteMutex(void* pvMutex);

/* 메모리 함수 */
void* OS_Memalloc(uint32_t ulSize);
void  OS_Memfree(void* pvMem);
void  OS_Memcpy(void* pvDest, const void* pvSrc, uint32_t ulSize);
void  OS_Memset(void* pvMem, uint8_t bFill, uint32_t ulSize);
int   OS_Memcmp(const void* pvBuf1, const void* pvBuf2, uint32_t ulSize);

/* 시간 함수 */
void     OS_Sleep(uint32_t ulSleepTimeMs);
uint32_t OS_GetMilliSecCounter(void);

/* 문자열 함수 */
int      OS_Strcmp(const char* pszBuf1, const char* pszBuf2);
int      OS_Strnicmp(const char* pszBuf1, const char* pszBuf2, uint32_t ulLen);
uint32_t OS_Strlen(const char* szText);

/* 파일 함수 (파일 시스템을 사용하지 않는 경우 스텁 가능) */
void* OS_FileOpen(const char* szFile, ...);
void  OS_FileClose(void* pvFile);
// ... 기타 파일 함수
```

### 2. OS_SPICustom.c (STM32 HAL SPI5 적용)
STM32 HAL SPI5 전용 구현을 제공하기 위해 이 파일을 생성해야 합니다:

#### 필수 함수:
```c
/* SPI 하드웨어 추상화 */
int32_t SPI_Init(void);
int32_t SPI_Deinit(void);
int32_t SPI_Transfer(uint8_t* pbSend, uint8_t* pbRecv, uint32_t ulLen);
int32_t SPI_SetCS(uint8_t bActive);
int32_t SPI_GetSRDY(void);

/* 핸드셰이킹을 위한 GPIO */
void GPIO_InitHandshake(void);
void GPIO_SetCS(GPIO_PinState state);
GPIO_PinState GPIO_GetSRDY(void);
```

## 통합 요구사항

### FreeRTOS 통합 (OS_Custom.c)
- Mutex 생성을 위해 `xSemaphoreCreateMutex()` 사용
- Mutex 작업을 위해 `xSemaphoreTake()` / `xSemaphoreGive()` 사용
- Sleep 함수를 위해 `vTaskDelay()` 사용
- 시간 함수를 위해 `xTaskGetTickCount()` 사용

#### Mutex 구현 예제

```c
#include "FreeRTOS.h"
#include "semphr.h"

/* Mutex 생성 */
void* OS_CreateMutex(void)
{
    SemaphoreHandle_t hMutex;

    hMutex = xSemaphoreCreateMutex();
    if (hMutex == NULL)
    {
        /* 생성 실패 - 메모리 부족 */
        return NULL;
    }

    return (void*)hMutex;
}

/* Mutex 대기 */
int OS_WaitMutex(void* pvMutex, uint32_t ulTimeout)
{
    SemaphoreHandle_t hMutex = (SemaphoreHandle_t)pvMutex;
    TickType_t xTicksToWait;

    if (hMutex == NULL)
        return -1;

    /* 타임아웃 변환 */
    if (ulTimeout == 0xFFFFFFFF)
        xTicksToWait = portMAX_DELAY;  /* 무한 대기 */
    else
        xTicksToWait = pdMS_TO_TICKS(ulTimeout);

    /* Mutex 획득 시도 */
    if (xSemaphoreTake(hMutex, xTicksToWait) == pdTRUE)
        return 0;  /* 성공 */
    else
        return -1;  /* 타임아웃 */
}

/* Mutex 해제 */
void OS_ReleaseMutex(void* pvMutex)
{
    SemaphoreHandle_t hMutex = (SemaphoreHandle_t)pvMutex;

    if (hMutex != NULL)
        xSemaphoreGive(hMutex);
}

/* Mutex 삭제 */
void OS_DeleteMutex(void* pvMutex)
{
    SemaphoreHandle_t hMutex = (SemaphoreHandle_t)pvMutex;

    if (hMutex != NULL)
        vSemaphoreDelete(hMutex);
}
```

#### 메모리 관리 구현

```c
#include "FreeRTOS.h"

/* 메모리 할당 */
void* OS_Memalloc(uint32_t ulSize)
{
    return pvPortMalloc(ulSize);
}

/* 메모리 해제 */
void OS_Memfree(void* pvMem)
{
    if (pvMem != NULL)
        vPortFree(pvMem);
}

/* 메모리 복사 */
void OS_Memcpy(void* pvDest, const void* pvSrc, uint32_t ulSize)
{
    memcpy(pvDest, pvSrc, ulSize);
}

/* 메모리 설정 */
void OS_Memset(void* pvMem, uint8_t bFill, uint32_t ulSize)
{
    memset(pvMem, bFill, ulSize);
}

/* 메모리 비교 */
int OS_Memcmp(const void* pvBuf1, const void* pvBuf2, uint32_t ulSize)
{
    return memcmp(pvBuf1, pvBuf2, ulSize);
}
```

#### 시간 함수 구현

```c
/* Sleep (밀리초) */
void OS_Sleep(uint32_t ulSleepTimeMs)
{
    vTaskDelay(pdMS_TO_TICKS(ulSleepTimeMs));
}

/* 밀리초 카운터 가져오기 */
uint32_t OS_GetMilliSecCounter(void)
{
    /* FreeRTOS 틱을 밀리초로 변환 */
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}
```

### STM32 HAL SPI5 통합 (OS_SPICustom.c)
- SPI 전송을 위해 `HAL_SPI_TransmitReceive()` 사용
- SPI5를 다음과 같이 설정:
  - Master 모드
  - Full duplex
  - 8비트 데이터 크기
  - netX 90 요구사항에 따른 클럭 극성/위상
- GPIO 제어 구현:
  - CS (Chip Select) 신호
  - SRDY (Service Ready) 입력 모니터링

#### SPI 초기화 구현

```c
#include "stm32f4xx_hal.h"

extern SPI_HandleTypeDef hspi5;  /* main.c 또는 spi.c에서 정의 */

/* GPIO 핀 정의 - 실제 하드웨어에 맞게 조정 */
#define NETX_CS_PIN         GPIO_PIN_6
#define NETX_CS_PORT        GPIOF
#define NETX_SRDY_PIN       GPIO_PIN_7
#define NETX_SRDY_PORT      GPIOF

/* SPI 초기화 */
int32_t SPI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO 클럭 활성화 */
    __HAL_RCC_GPIOF_CLK_ENABLE();

    /* CS 핀을 출력으로 설정 */
    GPIO_InitStruct.Pin = NETX_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(NETX_CS_PORT, &GPIO_InitStruct);

    /* CS를 HIGH로 설정 (비활성) */
    HAL_GPIO_WritePin(NETX_CS_PORT, NETX_CS_PIN, GPIO_PIN_SET);

    /* SRDY 핀을 입력으로 설정 */
    GPIO_InitStruct.Pin = NETX_SRDY_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(NETX_SRDY_PORT, &GPIO_InitStruct);

    return 0;
}
```

#### SPI 전송 구현

```c
#define SPI_TIMEOUT_MS      1000

/* SPI 전송 (송수신 동시) */
int32_t SPI_Transfer(uint8_t* pbSend, uint8_t* pbRecv, uint32_t ulLen)
{
    HAL_StatusTypeDef status;

    if (pbSend == NULL || pbRecv == NULL || ulLen == 0)
        return -1;

    /* Full-duplex 전송 수행 */
    status = HAL_SPI_TransmitReceive(&hspi5, pbSend, pbRecv,
                                      ulLen, SPI_TIMEOUT_MS);

    if (status != HAL_OK)
        return -1;

    return 0;
}

/* Chip Select 설정 */
int32_t SPI_SetCS(uint8_t bActive)
{
    if (bActive)
    {
        /* CS 활성 - LOW로 설정 */
        HAL_GPIO_WritePin(NETX_CS_PORT, NETX_CS_PIN, GPIO_PIN_RESET);
    }
    else
    {
        /* CS 비활성 - HIGH로 설정 */
        HAL_GPIO_WritePin(NETX_CS_PORT, NETX_CS_PIN, GPIO_PIN_SET);
    }

    return 0;
}

/* SRDY 신호 상태 가져오기 */
int32_t SPI_GetSRDY(void)
{
    GPIO_PinState state;

    state = HAL_GPIO_ReadPin(NETX_SRDY_PORT, NETX_SRDY_PIN);

    /* SRDY는 active high */
    return (state == GPIO_PIN_SET) ? 1 : 0;
}

/* 타임아웃과 함께 SRDY 대기 */
int32_t SPI_WaitSRDY(uint32_t ulTimeoutMs)
{
    uint32_t startTime = HAL_GetTick();

    while ((HAL_GetTick() - startTime) < ulTimeoutMs)
    {
        if (SPI_GetSRDY() == 1)
            return 0;  /* SRDY 활성 */

        /* 짧은 지연 */
        HAL_Delay(1);
    }

    return -1;  /* 타임아웃 */
}
```

## 타이밍 요구사항

netX 90 사양에 따름:
- SPI 클럭: 최대 20 MHz
- CS 셋업 타임: 최소 10ns
- SRDY 폴링: 각 전송 전 확인
- 바이트 간 지연: 연속 전송에는 불필요

### 핸드셰이크 시퀀스

```
1. CS 활성화 (LOW)
   ↓
2. SRDY 대기 (HIGH)
   - 타임아웃: 1000ms
   - 폴링 간격: 10us
   ↓
3. SPI 전송 수행
   - 명령 바이트
   - 주소 바이트(들)
   - 데이터 바이트(들)
   ↓
4. CS 비활성화 (HIGH)
   ↓
5. SRDY 해제 대기 (LOW)
   - 타임아웃: 100ms
   - 폴링 간격: 10us
```

## DPM 액세스 예제

### 완전한 DPM 읽기 함수

```c
int32_t DPM_Read(uint16_t usAddress, uint8_t* pbData, uint32_t ulLen)
{
    uint8_t abCmd[3];
    uint8_t abDummy[3];
    int32_t ret;

    /* 1. Mutex 획득 */
    if (OS_WaitMutex(g_hSpiMutex, 1000) != 0)
        return -1;  /* Mutex 타임아웃 */

    /* 2. SRDY 대기 */
    if (SPI_WaitSRDY(1000) != 0)
    {
        OS_ReleaseMutex(g_hSpiMutex);
        return -2;  /* SRDY 타임아웃 */
    }

    /* 3. CS 활성화 */
    SPI_SetCS(1);

    /* 4. 읽기 명령 전송 */
    abCmd[0] = 0x00;  /* 읽기 명령 */
    abCmd[1] = (uint8_t)(usAddress >> 8);    /* 주소 High */
    abCmd[2] = (uint8_t)(usAddress & 0xFF);  /* 주소 Low */

    ret = SPI_Transfer(abCmd, abDummy, 3);
    if (ret != 0)
    {
        SPI_SetCS(0);
        OS_ReleaseMutex(g_hSpiMutex);
        return -3;  /* 명령 전송 실패 */
    }

    /* 5. 데이터 읽기 */
    memset(abCmd, 0xFF, ulLen);  /* 더미 데이터 (읽기용) */
    ret = SPI_Transfer(abCmd, pbData, ulLen);

    /* 6. CS 비활성화 */
    SPI_SetCS(0);

    /* 7. Mutex 해제 */
    OS_ReleaseMutex(g_hSpiMutex);

    return ret;
}
```

### 완전한 DPM 쓰기 함수

```c
int32_t DPM_Write(uint16_t usAddress, uint8_t* pbData, uint32_t ulLen)
{
    uint8_t abCmd[3];
    uint8_t abDummy[3];
    int32_t ret;

    /* 1. Mutex 획득 */
    if (OS_WaitMutex(g_hSpiMutex, 1000) != 0)
        return -1;  /* Mutex 타임아웃 */

    /* 2. SRDY 대기 */
    if (SPI_WaitSRDY(1000) != 0)
    {
        OS_ReleaseMutex(g_hSpiMutex);
        return -2;  /* SRDY 타임아웃 */
    }

    /* 3. CS 활성화 */
    SPI_SetCS(1);

    /* 4. 쓰기 명령 전송 */
    abCmd[0] = 0x80;  /* 쓰기 명령 */
    abCmd[1] = (uint8_t)(usAddress >> 8);    /* 주소 High */
    abCmd[2] = (uint8_t)(usAddress & 0xFF);  /* 주소 Low */

    ret = SPI_Transfer(abCmd, abDummy, 3);
    if (ret != 0)
    {
        SPI_SetCS(0);
        OS_ReleaseMutex(g_hSpiMutex);
        return -3;  /* 명령 전송 실패 */
    }

    /* 5. 데이터 쓰기 */
    ret = SPI_Transfer(pbData, abDummy, ulLen);

    /* 6. CS 비활성화 */
    SPI_SetCS(0);

    /* 7. Mutex 해제 */
    OS_ReleaseMutex(g_hSpiMutex);

    return ret;
}
```

## 디버깅 지원

### SRDY 타이밍 측정

```c
/* SRDY 응답 시간 측정 */
uint32_t Measure_SRDY_Response(void)
{
    uint32_t startTime, endTime;

    /* CS 활성화 */
    SPI_SetCS(1);

    /* 시작 시간 기록 */
    startTime = OS_GetMilliSecCounter();

    /* SRDY 대기 */
    while (SPI_GetSRDY() == 0)
    {
        /* 타임아웃 방지 */
        if ((OS_GetMilliSecCounter() - startTime) > 1000)
            break;
    }

    /* 종료 시간 기록 */
    endTime = OS_GetMilliSecCounter();

    /* CS 비활성화 */
    SPI_SetCS(0);

    /* 응답 시간 반환 (밀리초) */
    return (endTime - startTime);
}
```

### SPI 전송 통계

```c
typedef struct {
    uint32_t totalTransfers;
    uint32_t successfulTransfers;
    uint32_t failedTransfers;
    uint32_t timeouts;
    uint32_t maxTransferTime;
    uint32_t minTransferTime;
    uint32_t avgTransferTime;
} SPI_Statistics_t;

SPI_Statistics_t g_spiStats = {0};

/* 통계를 포함한 SPI 전송 */
int32_t SPI_Transfer_WithStats(uint8_t* pbSend, uint8_t* pbRecv, uint32_t ulLen)
{
    uint32_t startTime, elapsedTime;
    int32_t ret;

    g_spiStats.totalTransfers++;

    startTime = OS_GetMilliSecCounter();
    ret = SPI_Transfer(pbSend, pbRecv, ulLen);
    elapsedTime = OS_GetMilliSecCounter() - startTime;

    if (ret == 0)
    {
        g_spiStats.successfulTransfers++;

        /* 전송 시간 통계 업데이트 */
        if (elapsedTime > g_spiStats.maxTransferTime)
            g_spiStats.maxTransferTime = elapsedTime;

        if (g_spiStats.minTransferTime == 0 ||
            elapsedTime < g_spiStats.minTransferTime)
            g_spiStats.minTransferTime = elapsedTime;

        /* 평균 계산 */
        g_spiStats.avgTransferTime =
            (g_spiStats.avgTransferTime + elapsedTime) / 2;
    }
    else
    {
        g_spiStats.failedTransfers++;
    }

    return ret;
}

/* 통계 출력 */
void PrintSPIStatistics(void)
{
    printf("SPI Statistics:\r\n");
    printf("  Total Transfers: %u\r\n", g_spiStats.totalTransfers);
    printf("  Successful: %u\r\n", g_spiStats.successfulTransfers);
    printf("  Failed: %u\r\n", g_spiStats.failedTransfers);
    printf("  Transfer Time (ms):\r\n");
    printf("    Min: %u\r\n", g_spiStats.minTransferTime);
    printf("    Max: %u\r\n", g_spiStats.maxTransferTime);
    printf("    Avg: %u\r\n", g_spiStats.avgTransferTime);
}
```

## 참조 구현 소스

참조 구현은 다음에서 찾을 수 있습니다:
- cifXToolkit 예제 프로젝트
- FreeRTOS 통합을 위한 애플리케이션 노트
- netX 90 SPI DPM 문서

## GPIO 핀 설정 가이드

### STM32CubeMX에서 GPIO 설정

1. **CS 핀 (Chip Select)**:
   - 모드: GPIO_Output
   - Pull: No pull-up and no pull-down
   - Speed: High
   - 초기 레벨: High (비활성)
   - 레이블: NETX_CS

2. **SRDY 핀 (Service Ready)**:
   - 모드: GPIO_Input
   - Pull: Pull-up
   - Speed: High
   - 레이블: NETX_SRDY

### 권장 핀 할당 (STM32F429)

| 신호 | 핀 | AF | 참고 |
|------|-----|-----|------|
| SPI5_SCK | PF7 | AF5 | SPI 클럭 |
| SPI5_MISO | PF8 | AF5 | SPI 데이터 입력 |
| SPI5_MOSI | PF9 | AF5 | SPI 데이터 출력 |
| CS | PF6 | GPIO | 수동 제어 |
| SRDY | PC1 | GPIO | 입력 모니터링 |

**참고**: 실제 핀 할당은 하드웨어 설계에 따라 다를 수 있습니다.

## 성능 고려사항

### SPI 클럭 속도 선택

```c
/* SPI 클럭 계산 (APB2 = 90 MHz) */
/* Prescaler = 8 → SPI_CLK = 90/8 = 11.25 MHz (안전) */
/* Prescaler = 4 → SPI_CLK = 90/4 = 22.5 MHz (너무 빠름!) */

/* STM32CubeMX에서 설정:
 * SPI5 Clock = APB2 / Prescaler
 * 목표: 10-20 MHz
 * 권장: Prescaler = 8 (11.25 MHz)
 */
```

### DMA 사용 (선택 사항)

대용량 전송의 경우 DMA를 사용하여 성능을 향상시킬 수 있습니다:

```c
/* DMA로 SPI 전송 (예제) */
int32_t SPI_Transfer_DMA(uint8_t* pbSend, uint8_t* pbRecv, uint32_t ulLen)
{
    HAL_StatusTypeDef status;

    /* DMA 전송 시작 */
    status = HAL_SPI_TransmitReceive_DMA(&hspi5, pbSend, pbRecv, ulLen);

    if (status != HAL_OK)
        return -1;

    /* 완료 대기 (세마포어 사용) */
    if (xSemaphoreTake(g_hSpiDmaSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE)
        return 0;  /* 성공 */
    else
        return -1;  /* 타임아웃 */
}

/* DMA 완료 콜백 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (hspi == &hspi5)
    {
        /* 세마포어 제공 (ISR에서) */
        xSemaphoreGiveFromISR(g_hSpiDmaSemaphore, &xHigherPriorityTaskWoken);

        /* 필요시 컨텍스트 전환 */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
```

## 문제 해결

### 일반적인 문제

1. **SRDY 타임아웃**
   - 원인: netX가 준비되지 않음
   - 해결: netX 펌웨어 확인, 리셋 핀 확인

2. **SPI 전송 실패**
   - 원인: 잘못된 SPI 설정
   - 해결: 클럭 극성/위상 확인, 속도 확인

3. **데이터 손상**
   - 원인: 타이밍 문제
   - 해결: SPI 클럭 속도 낮춤, 케이블 길이 확인

### 진단 도구

```c
/* 하드웨어 연결 테스트 */
void Test_Hardware_Connection(void)
{
    /* CS 토글 테스트 */
    for (int i = 0; i < 10; i++)
    {
        HAL_GPIO_WritePin(NETX_CS_PORT, NETX_CS_PIN, GPIO_PIN_SET);
        HAL_Delay(100);
        HAL_GPIO_WritePin(NETX_CS_PORT, NETX_CS_PIN, GPIO_PIN_RESET);
        HAL_Delay(100);
    }

    /* SRDY 읽기 테스트 */
    for (int i = 0; i < 10; i++)
    {
        GPIO_PinState srdy = HAL_GPIO_ReadPin(NETX_SRDY_PORT, NETX_SRDY_PIN);
        printf("SRDY = %d\r\n", srdy);
        HAL_Delay(100);
    }

    /* SPI 루프백 테스트 (MISO와 MOSI 연결) */
    uint8_t txData[] = {0x55, 0xAA, 0x12, 0x34};
    uint8_t rxData[4] = {0};

    HAL_SPI_TransmitReceive(&hspi5, txData, rxData, 4, 1000);

    printf("TX: %02X %02X %02X %02X\r\n",
           txData[0], txData[1], txData[2], txData[3]);
    printf("RX: %02X %02X %02X %02X\r\n",
           rxData[0], rxData[1], rxData[2], rxData[3]);
}
```

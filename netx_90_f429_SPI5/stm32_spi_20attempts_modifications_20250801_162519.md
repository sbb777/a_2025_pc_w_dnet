# STM32 F429 SPI 통신 20회 시도 수정사항

**작성일시**: 2025년 8월 1일 16:25:19  
**목적**: STM32 F429의 PreCommunicateWithSPI0 함수를 1회 통신에서 20회 연속 통신으로 변경

## 현재 코드 분석

### 기존 PreCommunicateWithSPI0 함수 (Core/Src/main.c:676-724)
```c
static int32_t PreCommunicateWithSPI0()
{
    int32_t lRet = CIFX_NO_ERROR;
    uint8_t tx_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    uint8_t rx_data[10] = {0};

    printf("Starting SPI0 pre-communication with netX90...\r\n");

    // 단일 SPI 통신
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // CS Low
    if(HAL_SPI_TransmitReceive(&hspi4, tx_data, rx_data, 10, 1000) != HAL_OK)
    {
        lRet = CIFX_FUNCTION_FAILED;
        printf("SPI0 communication failed\r\n");
    }
    else
    {
        printf("SPI0 communication successful\r\n");
        // 수신된 데이터 처리
        for(int i = 0; i < 10; i++)
        {
            printf("RX[%d]: 0x%02X\r\n", i, rx_data[i]);
        }
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); // CS High

    return lRet;
}
```

## 수정 요구사항

1. **통신 횟수**: 1회 → 20회 연속 통신
2. **성공/실패 카운팅**: 각 시도별 결과 추적
3. **데이터 변화**: 매 시도마다 다른 데이터 전송 (선택사항)
4. **통신 간격**: 시도 간 적절한 딜레이
5. **상세한 로깅**: 각 시도별 결과 출력

## 수정된 코드

### 수정 방안 1: 기본적인 20회 반복

**파일**: `Core/Src/main.c`  
**함수**: `PreCommunicateWithSPI0()` (라인 676-724)

```c
static int32_t PreCommunicateWithSPI0()
{
    int32_t lRet = CIFX_NO_ERROR;
    uint8_t tx_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    uint8_t rx_data[10] = {0};
    uint32_t successful_count = 0;
    uint32_t failed_count = 0;
    
    printf("Starting SPI0 pre-communication with netX90 (20 attempts)...\r\n");

    // 20회 반복 통신
    for(uint32_t attempt = 1; attempt <= 20; attempt++)
    {
        printf("--- Attempt %lu/20 ---\r\n", attempt);
        
        // CS Low
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
        
        // 짧은 딜레이 (CS 안정화)
        HAL_Delay(1);
        
        // SPI 통신 수행
        if(HAL_SPI_TransmitReceive(&hspi4, tx_data, rx_data, 10, 1000) != HAL_OK)
        {
            failed_count++;
            printf("Attempt %lu: FAILED\r\n", attempt);
            lRet = CIFX_FUNCTION_FAILED; // 최종 결과를 실패로 설정
        }
        else
        {
            successful_count++;
            printf("Attempt %lu: SUCCESS\r\n", attempt);
            
            // 수신된 데이터 출력 (간략하게)
            printf("RX: ");
            for(int i = 0; i < 10; i++)
            {
                printf("0x%02X ", rx_data[i]);
            }
            printf("\r\n");
        }
        
        // CS High
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
        
        // 시도 간 딜레이 (100ms)
        if(attempt < 20) // 마지막 시도가 아닌 경우에만
        {
            HAL_Delay(100);
        }
    }
    
    // 최종 결과 요약
    printf("\r\n=== SPI Communication Summary ===\r\n");
    printf("Total attempts: 20\r\n");
    printf("Successful: %lu\r\n", successful_count);
    printf("Failed: %lu\r\n", failed_count);
    printf("Success rate: %.1f%%\r\n", (float)successful_count / 20.0f * 100.0f);
    
    // 모든 통신이 성공한 경우에만 CIFX_NO_ERROR 반환
    if(successful_count == 20)
    {
        lRet = CIFX_NO_ERROR;
        printf("All communications successful!\r\n");
    }
    else
    {
        printf("Some communications failed!\r\n");
    }

    return lRet;
}
```

### 수정 방안 2: 향상된 버전 (데이터 변화 포함)

```c
static int32_t PreCommunicateWithSPI0()
{
    int32_t lRet = CIFX_NO_ERROR;
    uint8_t base_tx_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    uint8_t tx_data[10];
    uint8_t rx_data[10] = {0};
    uint32_t successful_count = 0;
    uint32_t failed_count = 0;
    
    printf("Starting SPI0 pre-communication with netX90 (20 attempts)...\r\n");

    // 20회 반복 통신
    for(uint32_t attempt = 1; attempt <= 20; attempt++)
    {
        // 매 시도마다 데이터 변경 (시도 번호 추가)
        for(int i = 0; i < 10; i++)
        {
            tx_data[i] = base_tx_data[i] + (uint8_t)(attempt - 1);
        }
        
        printf("--- Attempt %lu/20 ---\r\n", attempt);
        printf("TX: ");
        for(int i = 0; i < 10; i++)
        {
            printf("0x%02X ", tx_data[i]);
        }
        printf("\r\n");
        
        // CS Low
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
        
        // CS 안정화 딜레이
        HAL_Delay(1);
        
        // SPI 통신 수행
        HAL_StatusTypeDef spi_result = HAL_SPI_TransmitReceive(&hspi4, tx_data, rx_data, 10, 1000);
        
        if(spi_result != HAL_OK)
        {
            failed_count++;
            printf("Attempt %lu: FAILED (HAL Status: %d)\r\n", attempt, spi_result);
            lRet = CIFX_FUNCTION_FAILED; // 마지막 실패를 기록
        }
        else
        {
            successful_count++;
            printf("Attempt %lu: SUCCESS\r\n", attempt);
            
            // 수신된 데이터 출력
            printf("RX: ");
            for(int i = 0; i < 10; i++)
            {
                printf("0x%02X ", rx_data[i]);
            }
            printf("\r\n");
            
            // 데이터 검증 (옵션)
            bool data_valid = true;
            for(int i = 0; i < 10; i++)
            {
                if(rx_data[i] == 0x00) // 모든 데이터가 0이면 통신 문제 가능성
                {
                    data_valid = false;
                    break;
                }
            }
            
            if(!data_valid)
            {
                printf("Warning: Received data might be invalid (all zeros)\r\n");
            }
        }
        
        // CS High
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
        
        // 시도 간 딜레이 (netX90 처리시간 고려)
        if(attempt < 20)
        {
            HAL_Delay(150); // 150ms 딜레이로 증가
        }
        
        // 배열 초기화 (다음 수신을 위해)
        memset(rx_data, 0, sizeof(rx_data));
    }
    
    // 최종 결과 요약
    printf("\r\n=== SPI Communication Summary ===\r\n");
    printf("Total attempts: 20\r\n");
    printf("Successful: %lu\r\n", successful_count);
    printf("Failed: %lu\r\n", failed_count);
    printf("Success rate: %.1f%%\r\n", (float)successful_count / 20.0f * 100.0f);
    
    // 성공률에 따른 결과 판정
    if(successful_count >= 18) // 90% 이상 성공
    {
        lRet = CIFX_NO_ERROR;
        printf("Communication quality: EXCELLENT\r\n");
    }
    else if(successful_count >= 15) // 75% 이상 성공
    {
        lRet = CIFX_NO_ERROR;
        printf("Communication quality: GOOD\r\n");
    }
    else if(successful_count >= 10) // 50% 이상 성공
    {
        lRet = CIFX_FUNCTION_FAILED;
        printf("Communication quality: POOR\r\n");
    }
    else
    {
        lRet = CIFX_FUNCTION_FAILED;
        printf("Communication quality: CRITICAL\r\n");
    }

    return lRet;
}
```

### 수정 방안 3: 간단한 버전 (최소한의 변경)

```c
static int32_t PreCommunicateWithSPI0()
{
    int32_t lRet = CIFX_NO_ERROR;
    uint8_t tx_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    uint8_t rx_data[10] = {0};
    uint32_t success_count = 0;

    printf("Starting SPI0 pre-communication with netX90 (20 attempts)...\r\n");

    // 20회 반복
    for(int attempt = 1; attempt <= 20; attempt++)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // CS Low
        
        if(HAL_SPI_TransmitReceive(&hspi4, tx_data, rx_data, 10, 1000) != HAL_OK)
        {
            printf("Attempt %d: FAILED\r\n", attempt);
        }
        else
        {
            printf("Attempt %d: SUCCESS\r\n", attempt);
            success_count++;
        }
        
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); // CS High
        HAL_Delay(100); // 100ms 대기
    }

    printf("Success: %lu/20 attempts\r\n", success_count);
    
    if(success_count > 0)
        lRet = CIFX_NO_ERROR;
    else
        lRet = CIFX_FUNCTION_FAILED;

    return lRet;
}
```

## 추가 고려사항

### 1. 포함해야 할 헤더 (필요시)
```c
#include <string.h>  // memset 사용시
```

### 2. 타이밍 최적화
- **CS 안정화**: 1ms 딜레이
- **시도 간격**: 100-150ms (netX90 처리시간 고려)
- **전체 소요시간**: 약 3-4초 (20회 * 150ms)

### 3. 에러 처리 강화
- HAL SPI 상태 코드 체크
- 타임아웃 값 조정 가능
- 부분 성공 허용 (예: 18/20 성공시 OK)

### 4. 메모리 고려사항
- 지역 변수 사용으로 스택 사용량 최소화
- 큰 배열이나 전역 변수 불필요

### 5. 디버깅 옵션
- 각 시도별 상세 로그
- 수신 데이터 검증
- 성공률 계산 및 출력

## 권장 적용 순서

1. **1단계**: 수정 방안 3 (간단한 버전) 적용 및 테스트
2. **2단계**: 정상 동작 확인 후 수정 방안 1 (기본 버전) 적용
3. **3단계**: 필요시 수정 방안 2 (향상된 버전) 적용

각 단계별로 netX90와의 통신 상태를 확인하며 점진적으로 기능을 확장하는 것을 권장합니다.
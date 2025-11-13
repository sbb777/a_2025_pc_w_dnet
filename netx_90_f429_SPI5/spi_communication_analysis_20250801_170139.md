# STM32-netX90 SPI 통신 문제 분석

**작성일시**: 2025년 8월 1일 17:01:39  
**문제**: STM32에서 rx_data가 항상 255(0xFF) 값을 수신하는 현상

## 질문에 대한 답변

### 1️⃣ netX90 SPI 통신 횟수 제한 여부

**답변: netX90는 제한 없이 무한히 SPI 통신을 계속합니다.**

**근거**:
```c
// doio_chaselight_with_dnetFW_spi0/Targets/NXHX90-JTAG/Source/main.c (라인 165-233)
while(1)  // 무한 루프
{
    bTransactionComplete = false;
    
    // SPI 통신 설정
    eRet = DRV_SPI_TransmitReceive(&tSPISlaveHandle, ...);
    
    if(DRV_OK != eRet)
    {
        continue;  // 실패시 재시도
    }
    
    // 통신 완료 대기
    while(!bTransactionComplete) { }
    
    // 트랜잭션 카운터 증가
    ulTransactionCount++;
    
    // 데이터 처리 및 다음 통신 준비
    for(uint32_t i = 0; i < BUFFER_SIZE; i++)
    {
        aSlaveTxBuffer[i] = aSlaveRxBuffer[i] + ulTransactionCount;
    }
    
    // 무한 반복...
}
```

**특징**:
- 무한 루프(`while(1)`)로 계속 동작
- 각 통신 완료 후 자동으로 다음 통신 준비
- 통신 실패시 `continue`로 재시도
- 트랜잭션 카운터가 계속 증가

### 2️⃣ STM32에서 rx_data가 255(0xFF) 값을 받는 원인 분석

## 주요 원인 후보들

### 🔴 **1. 데이터 크기 불일치 (가장 가능성 높음)**

**STM32 설정**:
```c
// Core/Src/main.c
hspi4.Init.DataSize = SPI_DATASIZE_8BIT;        // 8비트
uint8_t tx_data[10] = {0x01, 0x02, ...};       // 8비트 배열
uint8_t rx_data[10] = {0};                      // 8비트 배열
```

**netX90 설정**:
```c
// netx90 main.c
eDataSize = DRV_SPI_DATA_SIZE_SELECT_16b;       // 16비트 ⚠️
static uint16_t aSlaveRxBuffer[BUFFER_SIZE];    // 16비트 배열
static uint16_t aSlaveTxBuffer[BUFFER_SIZE];    // 16비트 배열
```

**문제점**:
- STM32는 8비트씩 전송하지만 netX90는 16비트 단위로 처리
- 데이터 정렬 및 해석 오류 발생
- 16비트 상위 바이트가 0xFF로 채워질 가능성

### 🔴 **2. 버퍼 크기 및 전송 길이 불일치**

**STM32**:
```c
HAL_SPI_TransmitReceive(&hspi4, tx_data, rx_data, 10, 1000);  // 10바이트
```

**netX90**:
```c
DRV_SPI_TransmitReceive(&tSPISlaveHandle,
                       (uint8_t*)aSlaveTxBuffer,    // 16비트 배열을 8비트로 캐스팅
                       (uint8_t*)aSlaveRxBuffer,    // 16비트 배열을 8비트로 캐스팅
                       BUFFER_SIZE * 2);            // 64 * 2 = 128바이트
```

**문제점**:
- STM32: 10바이트 전송/수신 기대
- netX90: 128바이트 전송/수신 준비
- 길이 불일치로 인한 통신 오류

### 🔴 **3. SPI 모드 및 타이밍 문제**

**공통 설정**:
- STM32: `SPI_POLARITY_HIGH`, `SPI_PHASE_2EDGE` (Mode 3) ✓
- netX90: `DRV_SPI_MODE_3` ✓

**잠재적 문제**:
- 클럭 속도 차이
- CS 신호 타이밍
- 데이터 준비 시간 부족

### 🔴 **4. netX90 초기 상태 문제**

**초기 전송 버퍼**:
```c
static uint16_t aSlaveTxBuffer[BUFFER_SIZE] = {
    0x1000, 0x1001, 0x1002, 0x1003, ...  // 16비트 값들
};
```

**문제점**:
- 16비트 값 `0x1000`의 상위 바이트는 `0x10`
- 8비트로 해석시 `0x00, 0x10` 또는 `0x10, 0x00`
- 엔디안 차이로 인한 데이터 오해석

### 🔴 **5. 통신 동기화 문제**

**netX90 동작**:
```c
while(!bTransactionComplete) { }  // 콜백 대기
```

**문제점**:
- netX90가 준비되기 전에 STM32가 통신 시작
- 첫 번째 통신에서 netX90가 응답하지 못함
- MISO 라인이 풀업되어 0xFF 값 수신

## 🔧 해결 방안

### 1. **즉시 적용 가능한 해결책**

```c
// netX90 main.c 수정
// 기존
tSPISlaveHandle.tConfiguration.eDataSize = DRV_SPI_DATA_SIZE_SELECT_16b;

// 수정
tSPISlaveHandle.tConfiguration.eDataSize = DRV_SPI_DATA_SIZE_SELECT_8b;

// 버퍼도 8비트로 변경
static uint8_t aSlaveRxBuffer[16] = {0};
static uint8_t aSlaveTxBuffer[16] = {0x10, 0x11, 0x12, 0x13, ...};

// 전송 길이 수정
eRet = DRV_SPI_TransmitReceive(&tSPISlaveHandle,
                              aSlaveTxBuffer,      // 캐스팅 불필요
                              aSlaveRxBuffer,      // 캐스팅 불필요
                              16);                 // 16바이트로 고정
```

### 2. **STM32 코드 개선**

```c
// STM32 main.c - PreCommunicateWithSPI0 함수 수정
static int32_t PreCommunicateWithSPI0()
{
    // ... (기존 코드)
    
    for(int attempt = 1; attempt <= 20; attempt++)
    {
        // netX90 준비 시간 확보
        HAL_Delay(10);  // 추가: 10ms 대기
        
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // CS Low
        HAL_Delay(1);   // CS 안정화
        
        if(HAL_SPI_TransmitReceive(&hspi4, tx_data, rx_data, 10, 1000) != HAL_OK)
        {
            printf("Attempt %d: FAILED\r\n", attempt);
        }
        else
        {
            printf("Attempt %d: SUCCESS\r\n", attempt);
            
            // 수신 데이터 검증
            bool all_ff = true;
            for(int i = 0; i < 10; i++)
            {
                if(rx_data[i] != 0xFF) {
                    all_ff = false;
                    break;
                }
            }
            
            if(all_ff) {
                printf("Warning: All received data is 0xFF - possible communication issue\r\n");
            }
            
            success_count++;
        }
        
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); // CS High
        HAL_Delay(150); // netX90 처리 시간 확보
    }
    
    // ... (나머지 코드)
}
```

### 3. **디버깅 체크리스트**

1. **하드웨어 연결 확인**:
   - MISO 라인이 올바르게 연결되었는지
   - 풀업/풀다운 저항 상태
   - 전압 레벨 호환성 (3.3V)

2. **시그널 확인**:
   - 오실로스코프로 CS, CLK, MOSI, MISO 신호 확인
   - netX90가 MISO에 데이터를 출력하는지 확인

3. **타이밍 확인**:
   - netX90 초기화 완료 후 통신 시작
   - LED3 깜빡임으로 netX90 상태 확인

## 📊 0xFF 값이 나오는 시나리오 분석

| 원인 | 현상 | 확률 |
|------|------|------|
| netX90 응답 없음 (MISO 풀업) | 모든 바이트 0xFF | 높음 |
| 데이터 크기 불일치 (16bit vs 8bit) | 패턴있는 0xFF | 높음 |
| 통신 동기화 실패 | 첫 몇 번만 0xFF | 중간 |
| 하드웨어 연결 문제 | 항상 0xFF | 중간 |
| 클럭/타이밍 문제 | 불규칙한 0xFF | 낮음 |

## 🎯 권장 조치 순서

1. **1단계**: netX90 데이터 크기를 8비트로 변경
2. **2단계**: 버퍼 크기와 전송 길이 일치시키기
3. **3단계**: STM32에 통신 전 대기시간 추가
4. **4단계**: 하드웨어 연결 및 신호 확인
5. **5단계**: 오실로스코프로 실제 신호 분석

가장 가능성이 높은 원인은 **데이터 크기 불일치 (16bit vs 8bit)**이므로, 이 부분을 먼저 수정하는 것을 강력히 권장합니다.
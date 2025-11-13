# netX90 SPI0 Slave 통신 수정사항

**작성일시**: 2025년 8월 1일 16:11:28  
**목적**: STM32 F429 PreCommunicateWithSPI0 함수와 netX90 SPI0 slave 간 통신 호환성 확보

## 분석 결과

### STM32 F429 SPI 설정 (Core/Src/main.c:676-724)
```c
// SPI4를 마스터로 사용
hspi4.Init.Mode = SPI_MODE_MASTER;
hspi4.Init.DataSize = SPI_DATASIZE_8BIT;        // 8비트 데이터
hspi4.Init.CLKPolarity = SPI_POLARITY_HIGH;     // CPOL = 1
hspi4.Init.CLKPhase = SPI_PHASE_2EDGE;          // CPHA = 1 (Mode 3)
hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;

// 통신 데이터
uint8_t tx_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
uint8_t rx_data[10] = {0};
```

### netX90 SPI0 설정 (main.c:107-132)  
```c
// SPI0를 슬레이브로 사용
tSPISlaveHandle.tConfiguration.uMode.value = DRV_SPI_MODE_3;          // SPI Mode 3 ✓
tSPISlaveHandle.tConfiguration.eDataSize = DRV_SPI_DATA_SIZE_SELECT_16b; // 16비트 데이터 ✗
tSPISlaveHandle.tConfiguration.eBehaviour = DRV_SPI_BEHAVIOUR_SLAVE;

// 통신 버퍼
static uint16_t aSlaveRxBuffer[BUFFER_SIZE] = {0};     // BUFFER_SIZE = 64
static uint16_t aSlaveTxBuffer[BUFFER_SIZE] = {...};   // 16비트 배열
```

## 주요 호환성 문제

1. **데이터 크기 불일치**
   - STM32: 8비트 데이터
   - netX90: 16비트 데이터

2. **전송 길이 불일치**  
   - STM32: 10바이트 전송
   - netX90: 64개 16비트 요소 (128바이트) 처리 준비

3. **버퍼 크기 차이**
   - STM32: 10바이트 고정
   - netX90: 64개 16비트 요소

## netX90 수정사항

### 1. 데이터 크기를 8비트로 변경

**파일**: `doio_chaselight_with_dnetFW_spi0/Targets/NXHX90-JTAG/Source/main.c`

```c
// 기존 (라인 121)
tSPISlaveHandle.tConfiguration.eDataSize = DRV_SPI_DATA_SIZE_SELECT_16b;

// 수정 후
tSPISlaveHandle.tConfiguration.eDataSize = DRV_SPI_DATA_SIZE_SELECT_8b;
```

### 2. 버퍼를 8비트로 변경하고 크기 조정

**파일**: `doio_chaselight_with_dnetFW_spi0/Targets/NXHX90-JTAG/Source/main.c`

```c
// 기존 (라인 47-69)
static uint16_t aSlaveRxBuffer[BUFFER_SIZE] = {0};
static uint16_t aSlaveTxBuffer[BUFFER_SIZE] = {
    0x1000, 0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006, 0x1007,
    // ... 64개 16비트 값들
};

// 수정 후
static uint8_t aSlaveRxBuffer[16] = {0};  // 충분한 크기로 설정
static uint8_t aSlaveTxBuffer[16] = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
};
```

### 3. 전송 길이 수정

**파일**: `doio_chaselight_with_dnetFW_spi0/Targets/NXHX90-JTAG/Source/main.c`

```c
// 기존 (라인 177-183)
eRet = DRV_SPI_TransmitReceive(&tSPISlaveHandle,
                (uint8_t*)aSlaveTxBuffer,
                (uint8_t*)aSlaveRxBuffer,
                BUFFER_SIZE * 2);

// 수정 후
eRet = DRV_SPI_TransmitReceive(&tSPISlaveHandle,
                aSlaveTxBuffer,
                aSlaveRxBuffer,
                16);  // 16바이트로 고정
```

### 4. 데이터 처리 루프 수정

**파일**: `doio_chaselight_with_dnetFW_spi0/Targets/NXHX90-JTAG/Source/main.c`

```c
// 기존 (라인 225-231)
for(uint32_t i = 0; i < BUFFER_SIZE; i++)
{
    aSlaveTxBuffer[i] = aSlaveRxBuffer[i] + ulTransactionCount;
}

// 수정 후
for(uint32_t i = 0; i < 16; i++)
{
    aSlaveTxBuffer[i] = aSlaveRxBuffer[i] + (uint8_t)(ulTransactionCount & 0xFF);
}
```

### 5. 헤더 파일 수정 (선택사항)

**파일**: `doio_chaselight_with_dnetFW_spi0/Targets/NXHX90-JTAG/Include/main.h`

```c
// 기존 (라인 28)
#define BUFFER_SIZE              64

// 수정 후 (선택사항 - 위에서 직접 16을 사용하므로 필요시에만)
#define BUFFER_SIZE              16
```

## 추가 고려사항

1. **핀 매핑 확인**: STM32의 SPI4와 netX90의 SPI0가 올바르게 연결되어 있는지 확인
2. **클럭 속도 호환성**: STM32의 BAUDRATEPRESCALER_8 설정과 netX90의 1.56MHz 설정이 호환되는지 확인
3. **CS 신호**: STM32에서 PA6을 CS로 사용하고 netX90에서 FSS0 하드웨어 제어 사용
4. **전원 레벨**: 두 시스템 간 전압 레벨 호환성 확인 (3.3V)

## 테스트 시나리오

1. STM32에서 10바이트 전송 시 netX90가 정상 수신하는지 확인
2. netX90에서 응답 데이터가 STM32로 올바르게 전송되는지 확인  
3. 연속 통신시 데이터 무결성 유지 확인
4. LED3 깜빡임으로 통신 성공 여부 시각적 확인

## 우선순위

1. **필수**: 데이터 크기 8비트 변경
2. **필수**: 버퍼 타입과 크기 수정
3. **필수**: 전송 길이 수정
4. **권장**: 데이터 처리 로직 수정
5. **선택**: 헤더 상수 정의 수정
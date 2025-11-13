# STM32-netX90 버퍼 크기 불일치 문제 및 수정안

**작성일시**: 2025년 8월 1일 17:09:13  
**문제**: STM32(10바이트) vs netX90(16바이트) 버퍼 크기 불일치

## 🚨 사용자 지적사항이 정확합니다!

**STM32 설정**:
```c
uint8_t tx_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
uint8_t rx_data[10] = {0};
HAL_SPI_TransmitReceive(&hspi4, tx_data, rx_data, 10, 1000);  // 10바이트
```

**이전 권장안 (잘못된 것)**:
```c
// netX90 - 잘못된 수정안
static uint8_t aSlaveRxBuffer[16] = {0};     // 16바이트 버퍼
eRet = DRV_SPI_TransmitReceive(&tSPISlaveHandle,
                              aSlaveTxBuffer,
                              aSlaveRxBuffer,
                              16);                 // 16바이트 전송 ❌
```

## ⚠️ 문제점 분석

### 1. **SPI 통신의 동기화 특성**
- SPI는 **마스터가 전송 길이를 결정**
- STM32(마스터)가 10바이트만 전송하면, netX90(슬레이브)도 정확히 10바이트만 처리해야 함
- 슬레이브가 16바이트를 기대하면 **통신이 완료되지 않거나 타임아웃** 발생

### 2. **DRV_SPI_TransmitReceive 동작**
```c
// netX90가 16바이트를 기대하는 경우
DRV_SPI_TransmitReceive(&tSPISlaveHandle, tx_buf, rx_buf, 16);
```
- netX90는 16바이트 완료까지 대기
- STM32는 10바이트만 전송 후 CS를 HIGH로 변경
- netX90는 6바이트가 부족한 상태로 **통신 미완료 상태**
- `bTransactionComplete` 플래그가 설정되지 않음

### 3. **실제 발생할 수 있는 현상**
- netX90가 무한 대기 상태에 빠짐
- LED3가 깜빡이지 않음 (통신 완료 표시 없음)
- STM32는 타임아웃으로 실패 처리
- 다음 통신 시도시에도 계속 실패

## ✅ 올바른 해결 방안

### 방안 1: netX90를 STM32에 맞춤 (권장)

```c
// netX90 main.c 수정
// 1. 데이터 크기를 8비트로 변경
tSPISlaveHandle.tConfiguration.eDataSize = DRV_SPI_DATA_SIZE_SELECT_8b;

// 2. 버퍼 크기를 10바이트로 맞춤
static uint8_t aSlaveRxBuffer[10] = {0};
static uint8_t aSlaveTxBuffer[10] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};

// 3. 전송 길이를 10바이트로 설정
eRet = DRV_SPI_TransmitReceive(&tSPISlaveHandle,
                              aSlaveTxBuffer,
                              aSlaveRxBuffer,
                              10);                 // ✅ 10바이트로 맞춤

// 4. 데이터 처리 루프도 10바이트로 수정
for(uint32_t i = 0; i < 10; i++)  // BUFFER_SIZE 대신 10 사용
{
    aSlaveTxBuffer[i] = aSlaveRxBuffer[i] + (uint8_t)(ulTransactionCount & 0xFF);
}
```

### 방안 2: STM32를 netX90에 맞춤 (비권장)

```c
// STM32 main.c 수정 (비권장 - 기존 구조 변경 필요)
static int32_t PreCommunicateWithSPI0()
{
    uint8_t tx_data[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 
                          0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    uint8_t rx_data[16] = {0};

    // 16바이트 전송으로 변경
    if(HAL_SPI_TransmitReceive(&hspi4, tx_data, rx_data, 16, 1000) != HAL_OK)
    {
        // 처리...
    }
}
```

## 🔍 권장 수정 코드 (완전한 netX90 수정)

```c
// doio_chaselight_with_dnetFW_spi0/Targets/NXHX90-JTAG/Source/main.c

// 전역 버퍼 수정 (라인 47-69 대체)
static uint8_t aSlaveRxBuffer[10] = {0};
static uint8_t aSlaveTxBuffer[10] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};

// main 함수 내 SPI 설정 수정 (라인 121)
tSPISlaveHandle.tConfiguration.eDataSize = DRV_SPI_DATA_SIZE_SELECT_8b;

// 전송 설정 수정 (라인 177-183)
eRet = DRV_SPI_TransmitReceive(&tSPISlaveHandle,
                              aSlaveTxBuffer,
                              aSlaveRxBuffer,
                              10);                 // 정확히 10바이트

// 데이터 처리 수정 (라인 225-231)
for(uint32_t i = 0; i < 10; i++)                // 10바이트로 고정
{
    aSlaveTxBuffer[i] = aSlaveRxBuffer[i] + (uint8_t)(ulTransactionCount & 0xFF);
}
```

### 헤더 파일도 수정 (선택사항)

```c
// doio_chaselight_with_dnetFW_spi0/Targets/NXHX90-JTAG/Include/main.h
// #define BUFFER_SIZE              64  // 기존
#define BUFFER_SIZE              10     // STM32와 일치
```

## 🧪 테스트 시나리오

### 수정 전 예상 동작
```
STM32: 10바이트 전송 → CS HIGH
netX90: 16바이트 대기 중... (무한 대기)
결과: 통신 실패, LED3 깜빡임 없음
```

### 수정 후 예상 동작
```
STM32: 10바이트 전송 → CS HIGH
netX90: 10바이트 수신 완료 → 콜백 호출 → LED3 깜빡임
결과: 통신 성공, 정상적인 데이터 교환
```

## 📋 수정 체크리스트

- [ ] netX90 데이터 크기: 16bit → 8bit
- [ ] netX90 버퍼 크기: 64개 → 10개
- [ ] netX90 버퍼 타입: uint16_t → uint8_t
- [ ] netX90 전송 길이: 128바이트 → 10바이트
- [ ] netX90 처리 루프: BUFFER_SIZE → 10
- [ ] 헤더 파일 BUFFER_SIZE: 64 → 10 (선택)

## 🎯 우선순위

1. **최우선**: 전송 길이를 10바이트로 맞추기
2. **필수**: 데이터 크기를 8비트로 변경
3. **필수**: 버퍼 타입과 크기 수정
4. **권장**: 처리 루프 수정
5. **선택**: 헤더 상수 수정

**결론**: 사용자의 지적이 정확합니다. STM32가 10바이트를 전송하므로 netX90도 정확히 10바이트로 설정해야 합니다. 16바이트로 설정하면 통신이 완료되지 않습니다.
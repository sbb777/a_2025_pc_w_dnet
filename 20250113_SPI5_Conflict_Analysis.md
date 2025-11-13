# SPI5 통신 중복 문제 분석 보고서

**작성 일시**: 2025-01-13  
**문제**: SPI5 하드웨어를 여러 디바이스가 공유하여 충돌 가능성

---

## 1. SPI5 사용 현황

### 기존 사용처 (APC 애플리케이션)

#### 1.1 25C080 EEPROM
**파일**: `App/model/driver/APC_E2prom.c`  
**CS 핀**: PB14 (`MPU_SPI5_CFGCS__Pin`)  
**용도**: 설정 데이터 저장/읽기  
**함수**:
```c
- getE2promData()   // EEPROM 읽기
- setE2promData()   // EEPROM 쓰기
```

#### 1.2 VDRUCK (압력 센서)
**파일**: `App/model/driver/APC_Spi.c:228`  
**함수**: `transfer_VDRUCK_Spi()`  
**용도**: 압력 센서 데이터 통신

#### 1.3 DSP 통신
**파일**: `App/model/driver/APC_Spi.c:250`  
**함수**: `transferDspSpi()`  
**용도**: DSP 프로세서와 통신

### 새로 추가할 사용처 (DeviceNet)

#### 1.4 netX 90 (DeviceNet 통신 프로세서)
**파일**: `App/cifXToolkit/OSAbstraction/OS_SPICustom.c`  
**CS 핀**: PF6 (`NETX_CS_PIN`)  
**용도**: DeviceNet 산업용 필드버스 통신  
**함수**:
```c
- OS_SpiTransfer()  // netX 90과 SPI DPM 통신
```

---

## 2. 문제점: Race Condition (경쟁 상태)

### 2.1 시나리오 예시

```
시간 |  태스크 A (EEPROM)           |  태스크 B (DeviceNet)
-----|------------------------------|-----------------------------
t1   | CS_EEPROM = LOW             |
t2   | SPI5->DR = 0x03 (READ CMD)  |
t3   |                              | CS_NETX = LOW
t4   | SPI5->DR = addr_high        | SPI5->DR = 0xA5 (충돌!)
t5   | 잘못된 데이터 수신 ❌        | 잘못된 데이터 송신 ❌
```

### 2.2 발생 가능한 문제

1. **데이터 손상**
   - SPI5 하드웨어 레지스터(DR, SR)에 동시 접근
   - 한 디바이스의 데이터가 다른 디바이스로 전송됨

2. **통신 실패**
   - HAL_SPI_Transmit/Receive가 HAL_BUSY 반환
   - 타임아웃 발생

3. **시스템 불안정**
   - 잘못된 EEPROM 데이터 읽기 → 설정 오류
   - DeviceNet 통신 실패 → 필드버스 에러

---

## 3. 왜 Mutex가 필요한가?

### 3.1 하드웨어 공유 문제

STM32F429의 SPI5는 **하나의 하드웨어 블록**입니다:
- **하나의 데이터 레지스터** (SPI5->DR)
- **하나의 상태 레지스터** (SPI5->SR)
- **하나의 클록 생성기**
- **하나의 송수신 버퍼**

**4개의 디바이스가 동일한 하드웨어를 공유** → **Mutex로 순차 접근 보장 필수**

### 3.2 CS 핀만으로 충분하지 않은 이유

각 디바이스는 서로 다른 CS 핀을 사용하지만:

```
  CS_EEPROM (PB14)  ───┐
  CS_VDRUCK (???)   ───┼──→ [별도 디바이스들]
  CS_DSP (???)      ───┤
  CS_NETX (PF6)     ───┘
                        
  SPI5_SCK (PF7)    ───┐
  SPI5_MOSI (PF9)   ───┼──→ [공유 SPI5 버스]
  SPI5_MISO (PF8)   ───┘    (하나의 하드웨어!)
```

**CS 핀은 디바이스 선택만** 담당, **SPI5 하드웨어 접근은 보호하지 못함**

---

## 4. 해결 방법: SPI5 Mutex 구현

### 4.1 구현 위치

**파일**: `App/common/APC_SpiMutex.c` (NEW)

### 4.2 구현 개념

```c
// 전역 mutex
static osMutexId_t spi5_mutex = NULL;

// 초기화 (main에서 호출)
void APC_SPI5_MutexInit(void) {
    const osMutexAttr_t mutex_attr = {
        .name = "SPI5_Mutex",
        .attr_bits = osMutexPrioInherit,  // 우선순위 역전 방지
    };
    spi5_mutex = osMutexNew(&mutex_attr);
}

// SPI5 사용 전 Lock
bool APC_SPI5_Lock(uint32_t timeout_ms) {
    return (osMutexAcquire(spi5_mutex, timeout_ms) == osOK);
}

// SPI5 사용 후 Unlock
void APC_SPI5_Unlock(void) {
    osMutexRelease(spi5_mutex);
}
```

### 4.3 사용 예시

#### Before (위험):
```c
void getE2promData(uint16_t addr, uint8_t data) {
    HAL_GPIO_WritePin(MPU_SPI5_CFGCS__GPIO_Port, MPU_SPI5_CFGCS__Pin, GPIO_PIN_RESET);
    transmit_SPI5_Spi(send, 3);  // ← 다른 태스크와 충돌 가능!
    HAL_GPIO_WritePin(MPU_SPI5_CFGCS__GPIO_Port, MPU_SPI5_CFGCS__Pin, GPIO_PIN_SET);
}
```

#### After (안전):
```c
void getE2promData(uint16_t addr, uint8_t data) {
    if (!APC_SPI5_Lock(100)) return false;  // 100ms 대기
    
    HAL_GPIO_WritePin(MPU_SPI5_CFGCS__GPIO_Port, MPU_SPI5_CFGCS__Pin, GPIO_PIN_RESET);
    transmit_SPI5_Spi(send, 3);
    HAL_GPIO_WritePin(MPU_SPI5_CFGCS__GPIO_Port, MPU_SPI5_CFGCS__Pin, GPIO_PIN_SET);
    
    APC_SPI5_Unlock();
}
```

---

## 5. 적용 범위

### 5.1 수정 필요 파일

| 파일 | 함수 | 작업 |
|------|------|------|
| `APC_E2prom.c` | `getE2promData()` | Mutex 추가 |
| `APC_E2prom.c` | `setE2promData()` | Mutex 추가 |
| `APC_Spi.c` | `transfer_VDRUCK_Spi()` | Mutex 추가 |
| `APC_Spi.c` | `transferDspSpi()` | Mutex 추가 |
| `APC_Spi.c` | `receive_SPI5_Spi()` | Mutex 추가 |
| `APC_Spi.c` | `transmit_SPI5_Spi()` | Mutex 추가 |
| `APC_Spi.c` | `set_SPI5_SpiDataMode()` | Mutex 추가 |
| `OS_SPICustom.c` | `OS_SpiTransfer()` | Mutex 추가 |

### 5.2 Mutex 사용 패턴

모든 SPI5 접근은 다음 패턴을 따라야 합니다:

```c
if (!APC_SPI5_Lock(timeout)) {
    // 타임아웃 처리 (에러 로그, 재시도 등)
    return ERROR;
}

// === Critical Section 시작 ===
HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
HAL_SPI_Transmit(&hspi5, data, len, timeout);
HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
// === Critical Section 끝 ===

APC_SPI5_Unlock();
```

---

## 6. 성능 영향 분석

### 6.1 Mutex 오버헤드

- **Lock/Unlock**: 약 1-5 us (CPU 180MHz 기준)
- **컨텍스트 스위칭**: 태스크 대기 시 발생 (약 10-20 us)

### 6.2 통신 지연 예상

각 디바이스의 평균 통신 시간:

| 디바이스 | 통신 시간 | 빈도 |
|----------|-----------|------|
| EEPROM   | ~1ms      | 낮음 (설정 변경 시) |
| VDRUCK   | ~100us    | 중간 (10Hz?) |
| DSP      | ~500us    | 중간 |
| DeviceNet| ~100us    | 높음 (10-100Hz) |

**최악의 경우**: DeviceNet이 EEPROM 쓰기 대기 (1ms 지연)  
**일반적인 경우**: <100us 추가 지연 (거의 무시 가능)

### 6.3 권장 Timeout

```c
// EEPROM (블로킹 허용)
APC_SPI5_Lock(100);  // 100ms

// 실시간 센서 (빠른 실패)
APC_SPI5_Lock(10);   // 10ms

// DeviceNet (중간)
APC_SPI5_Lock(50);   // 50ms
```

---

## 7. 대안: SPI 분리 (하드웨어 변경 필요)

만약 성능이 중요하다면:

### 옵션 1: DeviceNet을 다른 SPI로 이동
- SPI3 또는 SPI4가 여유 있는지 확인
- 핀 재배치 필요 (STM32CubeMX)

### 옵션 2: 전용 SPI 추가 (하드웨어 제한)
- STM32F429는 SPI6 없음
- 불가능

**결론**: **Mutex 사용이 가장 현실적이고 안전한 방법**

---

## 8. 구현 우선순위

### Phase 1 (즉시)
1. ✅ 문제 분석 완료
2. ⏳ `APC_SpiMutex.c/h` 생성
3. ⏳ Mutex 초기화 코드 추가

### Phase 2 (기존 코드 보호)
4. ⏳ `APC_E2prom.c` Mutex 적용
5. ⏳ `APC_Spi.c` SPI5 관련 함수 Mutex 적용

### Phase 3 (DeviceNet 통합)
6. ⏳ `OS_SPICustom.c` Mutex 적용
7. ⏳ 통합 테스트

---

## 9. 테스트 계획

### 9.1 단위 테스트

1. **Mutex 기본 동작**
   - Lock → Unlock 정상 동작
   - 이중 Lock 방지 (데드락 검출)

2. **타임아웃 테스트**
   - 긴 EEPROM 쓰기 중 DeviceNet 접근
   - 타임아웃 발생 확인

### 9.2 통합 테스트

1. **동시 접근 스트레스 테스트**
   - 모든 디바이스 동시 사용
   - 데이터 무결성 검증

2. **실시간 성능 테스트**
   - DeviceNet 통신 주기 측정
   - 지터(jitter) 분석

---

## 10. 결론

### 핵심 요약

1. ✅ **SPI5가 4개 디바이스에서 공유됨**
2. ✅ **Mutex 없이는 데이터 손상 및 통신 실패 발생**
3. ✅ **Mutex 구현은 필수이며 성능 영향은 미미함**

### 다음 단계

사용자가 승인하면 즉시 `APC_SpiMutex.c/h` 구현 시작 가능합니다.

---

**작성자**: Claude Code  
**검토**: 필요

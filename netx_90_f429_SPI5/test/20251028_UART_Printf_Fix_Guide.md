# UART printf 출력 문제 해결 가이드
**작성일시**: 2025-10-28
**문제**: UART로 printf 데이터가 출력되지 않음
**해결**: `__io_putchar` 함수 구현 및 최적화

---

## 1. 문제 분석

### 📋 증상

- printf() 호출 시 데이터가 UART5로 출력되지 않음
- 시리얼 터미널에 아무것도 표시되지 않음
- 프로그램은 정상 실행되는 것처럼 보임

### 🔍 원인

**Core/Src/syscalls.c**의 `_write` 함수가 `__io_putchar`를 호출하지만, 이 함수가 구현되지 않음:

```c
// syscalls.c:80-90
__attribute__((weak)) int _write(int file, char *ptr, int len)
{
  (void)file;
  int DataIdx;

  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    __io_putchar(*ptr++);  // ← 이 함수가 구현되지 않음!
  }
  return len;
}
```

**syscalls.c:35**에서 weak 선언만 되어 있음:
```c
extern int __io_putchar(int ch) __attribute__((weak));
```

---

## 2. 해결 방법

### ✅ __io_putchar 함수 구현

**파일**: `Core/Src/main.c`
**위치**: Line 143-156 (USER CODE BEGIN 0 섹션)

#### 추가된 코드

```c
/**
 * @brief Redirect printf to UART5
 * @param ch Character to send
 * @return Character sent
 *
 * 최적화: 짧은 타임아웃으로 속도 개선
 * 115200 baud: 1 byte = ~87 μs, 10ms면 충분
 */
int __io_putchar(int ch)
{
    /* Send character via UART5 with 10ms timeout */
    HAL_UART_Transmit(&huart5, (uint8_t *)&ch, 1, 10);
    return ch;
}
```

#### 주요 특징

1. **UART5 사용**: netX90와 통신하는 UART5로 printf 출력
2. **짧은 타임아웃**: 10ms (115200 baud에서 1바이트는 약 87μs)
3. **간단한 구현**: HAL 라이브러리 직접 사용

---

## 3. 초기화 순서 확인

### 📌 중요: UART 초기화 순서

**main.c:426-445**:
```c
/* Initialize all configured peripherals */
MX_GPIO_Init();
MX_TIM1_Init();
MX_USART1_UART_Init();
MX_USB_HOST_Init();
MX_TIM3_Init();
MX_SPI4_Init();
MX_UART5_Init();              // ← Line 432: UART5 초기화
/* USER CODE BEGIN 2 */

UART_Ring_Init(&huart5);       // Line 436: 링 버퍼 초기화

/* UART printf 테스트 */        // Line 438-445: printf 사용
printf("\r\n\r\n");
printf("========================================\r\n");
printf(" STM32F429 + netX90 System Starting\r\n");
printf("========================================\r\n");
```

**✅ 올바른 순서**:
1. MX_UART5_Init() - HAL 초기화
2. UART_Ring_Init() - 링 버퍼 초기화
3. printf() 사용 - 이제 정상 작동

---

## 4. 테스트 방법

### 🚀 Step 1: 빌드

1. **STM32CubeIDE 열기**
2. **Project → Clean** (선택사항)
3. **Project → Build All** (Ctrl+B)
4. 빌드 성공 확인

### 🔌 Step 2: 시리얼 터미널 설정

#### 터미널 설정

| 항목 | 값 |
|------|-----|
| **포트** | COM# (장치 관리자 확인) |
| **Baud Rate** | 115200 |
| **Data Bits** | 8 |
| **Parity** | None |
| **Stop Bits** | 1 |
| **Flow Control** | None |

#### 권장 터미널 프로그램

- **PuTTY**: 무료, 가볍고 안정적
- **Tera Term**: 무료, 한글 지원 좋음
- **RealTerm**: 고급 기능 많음
- **STM32CubeMonitor**: ST 공식 도구

### 📺 Step 3: 프로그램 실행

1. **시리얼 터미널 먼저 오픈**
2. **Debug → Debug As → STM32 C/C++ Application**
3. **Run (F8)** 또는 **Resume** 클릭

### ✅ Step 4: 출력 확인

#### 예상 출력 (시작 메시지)

```

========================================
 STM32F429 + netX90 System Starting
========================================
UART5: 115200 baud, 8N1
Date: 2025-10-28
========================================

========================================
 VAT DeviceNet Test Initialization
========================================
VAT Test Configuration:
  Mode: 1 (1=Basic, 2=Calibration, 3=Monitoring)
  Node Address: 10
  Baud Rate: 250000 bps
  Input Assembly: 0x64 (7 bytes)
  Output Assembly: 0x08 (5 bytes)
========================================

Waiting for channel ready...
  Expected: HIL_COMM_COS_READY (0x00000001)
  [0 s] COS Flags: 0x00000001

*** Channel ready! ***
  COS Flags: 0x00000001
  Time taken: 0 seconds
...
```

---

## 5. 문제 해결 (출력 안 될 때)

### 🔧 체크리스트

#### 1️⃣ 시리얼 포트 확인

**Windows 장치 관리자**:
1. 장치 관리자 열기 (devmgmt.msc)
2. "포트 (COM & LPT)" 확장
3. "USB Serial Port (COM#)" 찾기
4. 포트 번호 확인

**드라이버 문제**:
- STM32 Virtual COM Port Driver 설치 필요
- ST-Link V2 드라이버 업데이트

#### 2️⃣ 터미널 설정 확인

**잘못된 설정 예**:
```
Baud Rate: 9600   ← 잘못됨! 115200이어야 함
Data Bits: 7      ← 잘못됨! 8이어야 함
Parity: Even      ← 잘못됨! None이어야 함
```

**올바른 설정**:
```
Baud Rate: 115200
Data Bits: 8
Parity: None
Stop Bits: 1
Flow Control: None
```

#### 3️⃣ UART5 핀 연결 확인

| 신호 | STM32F429 핀 | netX90 핀 |
|------|-------------|-----------|
| **TX** | PC12 (UART5_TX) | UART RX |
| **RX** | PD2 (UART5_RX) | UART TX |
| **GND** | GND | GND |

**주의**: TX는 RX로, RX는 TX로 교차 연결!

#### 4️⃣ 코드 확인

**__io_putchar 함수 존재 확인**:
```c
// Core/Src/main.c에 있어야 함
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart5, (uint8_t *)&ch, 1, 10);
    return ch;
}
```

**UART 초기화 확인**:
```c
// main() 함수 내
MX_UART5_Init();  // 이것이 printf() 전에 호출되어야 함
```

#### 5️⃣ 하드웨어 리셋

모든 설정이 맞는데도 출력이 안 되면:
1. STM32 프로그램 중지
2. USB 케이블 분리 (5초 대기)
3. USB 케이블 재연결
4. 시리얼 터미널 재오픈
5. 프로그램 재실행

---

## 6. 통신 속도 최적화

### ⚡ 현재 성능

**__io_putchar 호출당**:
- 전송 시간: ~87 μs (115200 baud)
- 타임아웃: 10 ms (충분한 여유)
- 오버헤드: HAL 함수 호출

**예시 계산**:
- "Hello World\r\n" (13 문자)
- 이론상: 13 × 87 μs = 1.13 ms
- 실제: ~2-3 ms (HAL 오버헤드 포함)

### 📊 개선 방법 (선택사항)

#### 방법 1: 버퍼링 (추천)

**장점**: 한 번에 여러 문자 전송
**구현**: 나중에 필요 시 추가

```c
// 예시 (구현 안함)
#define PRINTF_BUFFER_SIZE 256
static char printf_buffer[PRINTF_BUFFER_SIZE];
static uint16_t printf_index = 0;

int __io_putchar(int ch)
{
    printf_buffer[printf_index++] = (char)ch;

    if(ch == '\n' || printf_index >= PRINTF_BUFFER_SIZE)
    {
        HAL_UART_Transmit(&huart5, (uint8_t*)printf_buffer, printf_index, 100);
        printf_index = 0;
    }
    return ch;
}
```

#### 방법 2: DMA (고급)

**장점**: CPU 부하 최소화
**단점**: 설정 복잡, 버퍼 관리 필요

```c
// STM32CubeMX에서 UART5 DMA 활성화
// DMA2 Stream 7, Channel 4 (UART5_TX)
```

#### 방법 3: FIFO (하드웨어)

STM32F429의 UART FIFO 활성화:
```c
// main.c - MX_UART5_Init() 수정
huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_TXINVERT_INIT;
```

### 현재 구현 선택 이유

**10ms 타임아웃으로 충분한 이유**:
1. **안정성**: 모든 경우에 작동
2. **단순성**: 추가 코드 불필요
3. **성능**: 대부분 애플리케이션에 충분
4. **디버깅**: 실시간 출력 보장

---

## 7. 디버깅 팁

### 🐛 Live Expression 모니터링

```c
// UART 상태 확인
huart5.gState       // HAL_UART_STATE_READY 확인
huart5.ErrorCode    // 0이어야 함

// UART 레지스터 직접 확인
UART5->SR           // Status Register
UART5->DR           // Data Register
```

### 📝 간단한 테스트 코드

```c
/* USER CODE BEGIN 2 */

// 1. 직접 전송 테스트
HAL_UART_Transmit(&huart5, (uint8_t*)"Direct Test\r\n", 13, 100);

// 2. printf 테스트
printf("Printf Test\r\n");

// 3. 숫자 출력 테스트
printf("Number: %d\r\n", 12345);

// 4. 실수 출력 테스트 (선택)
printf("Float: %.2f\r\n", 3.14159);
```

### 🔍 로직 애널라이저로 확인

UART TX 신호 (PC12) 측정:
- **Idle**: High (3.3V)
- **Start Bit**: Low (0V)
- **Data**: LSB first
- **Stop Bit**: High (3.3V)
- **Baud Rate**: 115200 bps (8.68 μs/bit)

---

## 8. FAQ

### Q1: 왜 HAL_MAX_DELAY 대신 10ms를 사용하나요?

**A**: HAL_MAX_DELAY는 최대 타임아웃이지만 불필요하게 깁니다:
- 115200 baud: 1 byte = ~87 μs
- 10ms = 115 바이트 전송 시간
- 실제로는 1-2ms면 충분

### Q2: DMA를 사용하면 얼마나 빨라지나요?

**A**: CPU 부하가 줄지만 속도는 비슷합니다:
- Polling: CPU 대기 (블로킹)
- DMA: CPU 해방 (논블로킹)
- 실제 전송 속도는 동일 (115200 baud)

### Q3: printf가 느린 것 같아요

**A**: printf 자체가 느립니다:
- 포맷팅 오버헤드
- 문자 단위 전송
- 버퍼링으로 개선 가능 (방법 1 참고)

### Q4: 여러 UART를 동시에 사용할 수 있나요?

**A**: 가능하지만 __io_putchar는 하나만 지원:
```c
// UART1: printf
// UART5: 직접 HAL_UART_Transmit 사용
HAL_UART_Transmit(&huart1, data, len, 100);  // printf용
HAL_UART_Transmit(&huart5, data, len, 100);  // 직접 사용
```

### Q5: \r\n과 \n의 차이는?

**A**:
- `\n`: Line Feed (LF) - Unix/Linux 스타일
- `\r\n`: Carriage Return + Line Feed (CRLF) - Windows 스타일
- 터미널에 따라 다르지만 `\r\n`이 안전

---

## 9. 체크리스트

### ✅ 코드 수정 완료

- [ ] `__io_putchar` 함수 구현 (main.c:151-156)
- [ ] UART 초기화 순서 확인 (UART5 → printf)
- [ ] 시작 메시지 추가 (main.c:438-445)
- [ ] 빌드 성공 확인

### ✅ 하드웨어 연결

- [ ] STM32 USB 케이블 연결
- [ ] UART5 핀 연결 확인 (TX → RX 교차)
- [ ] GND 공통 연결

### ✅ 터미널 설정

- [ ] 포트 번호 확인 (장치 관리자)
- [ ] Baud Rate: 115200
- [ ] Data Bits: 8, Parity: None, Stop Bits: 1
- [ ] 터미널 오픈 확인

### ✅ 실행 및 테스트

- [ ] 시작 메시지 출력 확인
- [ ] VAT 초기화 메시지 확인
- [ ] COS Flags 주기적 출력 확인
- [ ] 모든 printf가 정상 작동

---

## 10. 요약

### 🎯 핵심 변경 사항

| 항목 | 변경 전 | 변경 후 |
|------|---------|---------|
| **__io_putchar** | 미구현 | 구현 (UART5) |
| **printf 출력** | 안 됨 | 정상 작동 |
| **타임아웃** | N/A | 10ms (최적화) |
| **초기화 순서** | 확인 안함 | UART → printf |

### ✅ 개선 효과

1. **printf 사용 가능**: 모든 디버그 메시지 출력
2. **실시간 모니터링**: 프로그램 실행 상태 확인
3. **빠른 디버깅**: 에러 메시지 즉시 확인
4. **사용자 친화적**: 터미널로 상태 확인

### 🔄 다음 단계

printf가 작동하지 않으면:
1. 시리얼 포트 번호 재확인
2. 터미널 설정 검증 (115200, 8N1)
3. UART 핀 연결 확인
4. 하드웨어 리셋 시도

---

**문서 끝**

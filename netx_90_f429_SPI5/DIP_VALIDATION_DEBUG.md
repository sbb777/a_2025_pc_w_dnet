# DIP Switch Validation 디버깅 가이드

**문제**: `g_bDipDataValid`가 0으로 유지되어 무한 루프

---

## 🔍 원인 분석

### 검증 로직 흐름

```
main() Line 430-455:
  ↓
while (!g_bDipDataValid)
  ↓
UART_Ring_ReadLine() → 0x0D로 끝나는 라인 읽기
  ↓
ValidateDipSwitchData(line) → 검증 시작
  ↓
ExtractDipSwitchData() → "D:[xxxx]" 추출
  ↓
g_bDipDataValid = 1 → 성공
```

### 실패 가능성

| 단계 | 조건 | 실패 시 출력 |
|------|------|-------------|
| **UART 수신** | netX90 → STM32 데이터 전송 | 출력 없음 |
| **링버퍼 동작** | `HAL_UART_Receive_IT()` 성공 | 출력 없음 |
| **라인 완성** | 0x0D (CR) 수신 | "RX: (데이터)" 출력 안됨 |
| **패턴 검색** | "D:[" 존재 | "No D pattern found" |
| **숫자 검증** | '0' 또는 '1' 4개 | "Invalid D digit at pos X" |
| **괄호 확인** | ']' 존재 | "Missing closing bracket" |
| **성공** | 모든 조건 통과 | "D validation SUCCESS: xxxx" |

---

## 🎯 디버깅 단계

### Step 1: UART 수신 확인

#### 예상 출력 (정상)
```
Starting D validation loop (waiting for D:[xxxx] + 0x0D)...
RX: D:[1010]
Validating line: D:[1010]
D extracted: 1010
D validation SUCCESS: 1010
D data validated!
```

#### 실제 상황별 대응

**Case 1: 아무 출력 없음**
```
Starting D validation loop (waiting for D:[xxxx] + 0x0D)...
(무한 대기)
```

**원인**: UART 데이터 미수신

**해결**:
1. netX90 전원 확인
2. UART5 연결 확인 (PC12=TX, PD2=RX)
3. 케이블 교차 확인 (netX90 TX → STM32 RX)

---

**Case 2: "RX: " 출력은 되나 검증 실패**
```
Starting D validation loop (waiting for D:[xxxx] + 0x0D)...
RX: hello world
Validating line: hello world
No D pattern found
D extraction failed
Validation FAILED, waiting for new data.
```

**원인**: 데이터 포맷 불일치

**netX90에서 전송해야 할 포맷**:
```
D:[1010]\r    (ASCII: 44 3A 5B 31 30 31 30 5D 0D)
      ^^^ CR (0x0D) 필수
```

---

**Case 3: 패턴은 있으나 숫자 오류**
```
RX: D:[1A10]
Validating line: D:[1A10]
Invalid D digit at pos 1
D extraction failed
Validation FAILED, waiting for new data.
```

**원인**: '0', '1' 외 문자 포함

**수정**: netX90에서 바이너리 문자열만 전송
```c
// 잘못된 예
sprintf(buffer, "D:[%04X]", value);  // 16진수 A-F 포함

// 올바른 예
sprintf(buffer, "D:[%d%d%d%d]\r",
        (value>>3)&1, (value>>2)&1, (value>>1)&1, value&1);
```

---

**Case 4: 괄호 누락**
```
RX: D:[1010
Validating line: D:[1010
Missing closing bracket
D extraction failed
Validation FAILED, waiting for new data.
```

**원인**: ']' 누락

**수정**: netX90 전송 코드에 ']' 추가

---

### Step 2: Live Expression 모니터링

**디버거에서 확인**:
```c
// 기본 상태
g_bDipDataValid        // 0 (실패), 1 (성공)
g_DipSwitchData        // "1010" (성공 시)

// 링버퍼 상태
rx_ring.head           // 쓰기 위치
rx_ring.tail           // 읽기 위치
rx_ring.buffer         // 수신 데이터

// UART 상태
huart5.gState          // HAL_UART_STATE_READY (정상)
huart5.RxState         // HAL_UART_STATE_BUSY_RX (수신 중)
```

**정상 동작 시**:
```
rx_ring.head = 9       // "D:[1010]\r" 수신 완료
rx_ring.tail = 9       // 모두 읽음
g_bDipDataValid = 1    // 검증 성공
g_DipSwitchData = "1010"
```

**수신 안될 때**:
```
rx_ring.head = 0       // 데이터 없음
rx_ring.tail = 0
g_bDipDataValid = 0    // 계속 0
```

---

### Step 3: 코드 수정 (임시 우회)

#### 방법 1: 검증 강제 통과

`main.c` Line 430 수정:
```c
// 원본
while (!g_bDipDataValid)
{
    int len = UART_Ring_ReadLine(line, sizeof(line));
    // ...
}

// 임시 우회 (테스트용)
g_bDipDataValid = 1;  // 강제 설정
strcpy((char*)g_DipSwitchData, "1010");  // 기본값 설정
HAL_UART_Transmit(&huart5, (uint8_t *)"DIP validation BYPASSED\r\n", 25, HAL_MAX_DELAY);

// while 루프 주석 처리
/*
while (!g_bDipDataValid)
{
    // ...
}
*/
```

**주의**: VAT 테스트만 하고 DIP 스위치가 필요 없는 경우에만 사용

---

#### 방법 2: 타임아웃 추가

`main.c` Line 430 수정:
```c
// 타임아웃 추가 (30초)
uint32_t timeout_start = HAL_GetTick();
uint32_t timeout_ms = 30000;  // 30초

while (!g_bDipDataValid)
{
    // 타임아웃 체크
    if ((HAL_GetTick() - timeout_start) > timeout_ms)
    {
        HAL_UART_Transmit(&huart5,
            (uint8_t *)"DIP validation TIMEOUT, using default\r\n",
            39, HAL_MAX_DELAY);

        // 기본값 설정
        g_bDipDataValid = 1;
        strcpy((char*)g_DipSwitchData, "0000");
        break;
    }

    int len = UART_Ring_ReadLine(line, sizeof(line));
    // ... 나머지 코드 동일
}
```

---

### Step 4: netX90 전송 코드 확인

**netX90에서 전송해야 할 데이터**:

```c
// netX90 코드 예시
void SendDipSwitchData(uint8_t dip_value)
{
    char buffer[12];

    // 바이너리 문자열 생성
    sprintf(buffer, "D:[%d%d%d%d]\r",
            (dip_value >> 3) & 1,
            (dip_value >> 2) & 1,
            (dip_value >> 1) & 1,
            (dip_value >> 0) & 1);

    // UART 전송 (예: UART0)
    HAL_UART_Transmit(&huart0, (uint8_t*)buffer, strlen(buffer), 100);
}

// 사용 예
SendDipSwitchData(0x0A);  // 1010 전송 → "D:[1010]\r"
SendDipSwitchData(0x05);  // 0101 전송 → "D:[0101]\r"
```

**전송 주기**: 초기 부팅 후 1회, 또는 1초마다 반복

---

## 🔧 해결 방법 우선순위

### 1순위: netX90 데이터 전송 확인
- **점검**: netX90 펌웨어에서 UART 전송 로직 확인
- **테스트**: PC 시리얼 모니터로 netX90 UART 출력 직접 확인
- **포맷**: `D:[xxxx]\r` (CR 0x0D 필수)

### 2순위: UART 연결 확인
- **물리 연결**: TX/RX 교차 연결 확인
- **전압 레벨**: 3.3V 호환 확인
- **GND 연결**: 공통 접지 확인

### 3순위: STM32 UART 설정
- **보드레이트**: 115200 일치 확인
- **인터럽트**: UART5 인터럽트 활성화 확인
- **클럭**: UART5 클럭 활성화 확인

### 4순위: 코드 우회 (임시)
- **강제 통과**: `g_bDipDataValid = 1` 설정
- **타임아웃**: 30초 후 기본값 사용
- **VAT 테스트 진행**: DIP 검증 없이 VAT 테스트만 실행

---

## 📋 체크리스트

### 하드웨어 점검
- [ ] netX90 전원 ON 확인
- [ ] UART5 연결 확인 (PC12/PD2)
- [ ] TX/RX 교차 연결 확인
- [ ] GND 공통 접지 확인
- [ ] 케이블 단선 확인

### 소프트웨어 점검
- [ ] netX90 펌웨어 UART 전송 로직 확인
- [ ] 전송 포맷 "D:[xxxx]\r" 확인
- [ ] STM32 UART5 초기화 확인
- [ ] 인터럽트 활성화 확인
- [ ] 시리얼 모니터 출력 확인

### 디버깅 완료
- [ ] "RX: " 출력 확인
- [ ] "D validation SUCCESS" 출력 확인
- [ ] `g_bDipDataValid = 1` 확인
- [ ] VAT 테스트 진행 확인

---

## 🚀 빠른 해결 (임시)

**VAT 테스트만 하고 싶은 경우**:

`main.c` Line 424 다음에 추가:
```c
UART_Ring_Init(&huart5);

// ===== 임시: DIP 검증 우회 =====
g_bDipDataValid = 1;
strcpy((char*)g_DipSwitchData, "1010");
HAL_UART_Transmit(&huart5, (uint8_t *)"DIP validation BYPASSED for VAT test\r\n", 38, HAL_MAX_DELAY);
// ===== 임시 코드 끝 =====

// 원래 코드 주석 처리
/*
HAL_UART_Transmit(&huart5, (uint8_t *)"Starting D validation loop...\r\n", ...);
while (!g_bDipDataValid)
{
    // ...
}
*/

lRet = InitializeToolkit();
```

재빌드 후 VAT 테스트 바로 실행됨.

---

**작성일**: 2025-10-27
**문서 버전**: 1.0

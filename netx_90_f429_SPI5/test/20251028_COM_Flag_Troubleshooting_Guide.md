# COM-flag 문제 해결 및 netHost 테스트 가이드
**작성일시**: 2025-10-28
**문제**: "COM-flag not set" 에러 발생
**해결**: 타임아웃 및 디버깅 로직 추가

---

## 1. 문제 분석

### 📋 증상

netHost VAT 테스트 실행 시 다음과 같이 무한 대기 발생:

```
Waiting for channel ready...
(무한 대기 - 프로그램 멈춤)
```

### 🔍 원인

**Core/Src/main.c:490** 라인에서 `HIL_COMM_COS_READY` 플래그 대기 중 무한 루프:

```c
while(!(tChannelInfo.ulDeviceCOSFlags & HIL_COMM_COS_READY));
```

**ulDeviceCOSFlags**: netX90 채널의 Change-of-State 플래그
- **HIL_COMM_COS_READY (0x00000001)**: 채널이 통신 준비 완료 상태

### ⚠️ 주요 원인들

| 원인 | 설명 | 확인 방법 |
|------|------|-----------|
| **1. 펌웨어 미로드** | DeviceNet 펌웨어가 netX90에 로드되지 않음 | Cookie 확인 (g_szLastCookie) |
| **2. 채널 초기화 실패** | cifX 채널 초기화가 완료되지 않음 | xChannelOpen 반환값 확인 |
| **3. SPI 통신 문제** | STM32 ↔ netX90 SPI 통신 오류 | SerialDPM_Init 상태 확인 |
| **4. 설정 누락** | DeviceNet 설정 파일 누락 또는 손상 | 펌웨어 파일 존재 확인 |

---

## 2. 적용된 수정 사항

### ✅ 타임아웃 로직 추가

**파일**: `Core/Src/main.c`
**위치**: Line 482-537

#### 주요 변경점

1. **타임아웃 카운터 추가** (5초):
   ```c
   uint32_t timeout_count = 0;
   const uint32_t MAX_TIMEOUT = 50;  /* 5초 = 50 x 100ms */
   ```

2. **주기적 상태 출력** (1초마다):
   ```c
   if((timeout_count % 10) == 0)
   {
       printf("  [%lu s] COS Flags: 0x%08X\r\n",
              timeout_count / 10,
              (unsigned int)tChannelInfo.ulDeviceCOSFlags);
   }
   ```

3. **타임아웃 시 에러 처리**:
   ```c
   if(timeout_count >= MAX_TIMEOUT)
   {
       printf("\r\n*** ERROR: Channel ready timeout! ***\r\n");
       printf("  Final COS Flags: 0x%08X\r\n", ...);
       printf("  Expected Flag:   0x%08X (HIL_COMM_COS_READY)\r\n", ...);
       lRet = CIFX_DEV_NOT_READY;
       break;
   }
   ```

4. **VAT 테스트 조건부 실행**:
   ```c
   if(lRet == CIFX_NO_ERROR)
   {
       /* 채널 준비 완료 - VAT 테스트 실행 */
       VAT_RunTest(hChannel);
   }
   else
   {
       /* 채널 준비 실패 - 테스트 스킵 */
       printf("\r\n*** Skipping VAT test due to channel not ready ***\r\n");
   }
   ```

---

## 3. 테스트 방법

### 🚀 Step 1: 빌드

1. **STM32CubeIDE 열기**
2. **Project → Build All** (Ctrl+B)
3. 빌드 성공 확인:
   ```
   Finished building: main.c
   Build Finished. 0 errors, 0 warnings.
   ```

### 🔌 Step 2: 하드웨어 연결 확인

#### 필수 연결

| 항목 | 연결 상태 | 확인 |
|------|-----------|------|
| **STM32F429 전원** | USB 케이블 연결 | LED 켜짐 |
| **netX90 전원** | 5V 전원 공급 | 전원 LED 켜짐 |
| **SPI 연결** | STM32 ↔ netX90 | 케이블 확인 |
| **DeviceNet** | VAT 장치 연결 (선택) | 터미네이터 확인 |

#### SPI 핀맵 확인

| 신호 | STM32F429 | netX90 |
|------|-----------|--------|
| **MOSI** | PE6 (SPI4_MOSI) | SPI0_MOSI |
| **MISO** | PE5 (SPI4_MISO) | SPI0_MISO |
| **SCK** | PE2 (SPI4_SCK) | SPI0_CLK |
| **CS** | PE4 | SPI0_CS |

### 📺 Step 3: 프로그램 실행

1. **Debug → Debug As → STM32 C/C++ Application**
2. **시리얼 터미널 열기** (UART5, 115200 baud)
3. **Run (F8)** 또는 **Resume** 클릭

### 🔍 Step 4: 출력 분석

#### ✅ 정상 시나리오 (채널 준비 성공)

```
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
  [0 s] COS Flags: 0x00000000
  [1 s] COS Flags: 0x00000001

*** Channel ready! ***
  COS Flags: 0x00000001
  Time taken: 1 seconds

========================================
 Running VAT Test Mode: 1
========================================
Test: Basic Pressure Control
Cycles: 10
Setpoint: 500

...
```

#### ❌ 에러 시나리오 (타임아웃)

```
Waiting for channel ready...
  Expected: HIL_COMM_COS_READY (0x00000001)
  [0 s] COS Flags: 0x00000000
  [1 s] COS Flags: 0x00000000
  [2 s] COS Flags: 0x00000000
  [3 s] COS Flags: 0x00000000
  [4 s] COS Flags: 0x00000000

*** ERROR: Channel ready timeout! ***
  Final COS Flags: 0x00000000
  Expected Flag:   0x00000001 (HIL_COMM_COS_READY)

Possible causes:
  1. DeviceNet firmware not loaded
  2. Channel initialization failed
  3. SPI communication issue
  4. netX90 not responding

*** Skipping VAT test due to channel not ready ***
```

---

## 4. 문제 해결 (타임아웃 발생 시)

### 🔧 해결 순서

#### 1️⃣ Cookie 확인

**Live Expression 창에 추가**:
```c
g_szLastCookie
```

**예상 값**: `"netX"` 또는 `"BOOT"`

| 값 | 의미 | 조치 |
|----|------|------|
| **"netX"** | 펌웨어 정상 실행 중 | 다음 단계 진행 |
| **"BOOT"** | 부트로더 모드 | 펌웨어 다운로드 필요 |
| **0x00000000** | DPM 접근 불가 | SPI 연결 확인 |

#### 2️⃣ SPI 통신 확인

**디버거 설정**:
1. **Breakpoint**: `SerialDPM_Init()` 함수 진입 시
2. **Step Through** (F6)로 SPI 초기화 과정 확인
3. **Watch**:
   ```c
   hspi4.State         // HAL_SPI_STATE_READY 확인
   ptDevInstance->pbDPM  // DPM 베이스 주소 확인
   ```

#### 3️⃣ 펌웨어 파일 확인

**필수 파일 존재 확인**:
```bash
# Windows 명령 프롬프트
dir Hil_DemoAppDNS\EthernetDevices\*.nxf
dir Hil_DemoAppDNS\EthernetDevices\*.nxd

# 예상 출력
#  DeviceNet_Master.nxf
#  DeviceNet_Master.nxd
```

**파일 누락 시**:
- Hilscher 홈페이지에서 DeviceNet 펌웨어 다운로드
- 또는 기존 작동하는 프로젝트에서 복사

#### 4️⃣ 채널 정보 확인

**Live Expression**:
```c
tChannelInfo.ulDeviceCOSFlags
tChannelInfo.ulOpenCnt
tChannelInfo.ulNetxFlags
tChannelInfo.ulHostFlags
```

**정상 값**:
```
ulDeviceCOSFlags = 0x00000001 (HIL_COMM_COS_READY)
ulOpenCnt        = 1 (채널 열림)
ulNetxFlags      = 0x00000001 (READY)
ulHostFlags      = 0x00000000
```

#### 5️⃣ 타임아웃 시간 조정

펌웨어 로딩이 느린 경우 타임아웃 증가:

**Core/Src/main.c:485**:
```c
const uint32_t MAX_TIMEOUT = 100;  /* 10초로 증가 */
```

---

## 5. Live Expression 모니터링

### 📊 추가할 변수들

#### 시스템 상태
```c
g_szLastCookie              // DPM Cookie: "netX" 확인
g_VatTestMode               // 테스트 모드: 1, 2, 3
g_bEnableVatTest            // VAT 테스트 활성화: true/false
```

#### 채널 상태
```c
tChannelInfo.ulDeviceCOSFlags   // COS 플래그: 0x00000001 대기
timeout_count                    // 타임아웃 카운터
lRet                            // 마지막 반환 값
```

#### VAT 테스트 상태 (테스트 실행 시)
```c
g_tVatContext.stats.total_read_count
g_tVatContext.stats.total_write_count
g_tVatContext.stats.read_error_count
g_tVatContext.input_asm100.pressure
g_tVatContext.output_asm8.control_setpoint
```

---

## 6. 디버깅 팁

### 🐛 Breakpoint 설정 위치

| 위치 | 목적 | 확인 사항 |
|------|------|-----------|
| **main.c:472** | VAT 초기화 시작 | `g_bEnableVatTest == true` |
| **main.c:475** | 드라이버 오픈 | `lRet == CIFX_NO_ERROR` |
| **main.c:479** | 채널 오픈 | `lRet == CIFX_NO_ERROR` |
| **main.c:490** | 채널 준비 대기 시작 | `tChannelInfo` 초기값 |
| **main.c:492** | COS 플래그 읽기 | `ulDeviceCOSFlags` 값 변화 |
| **main.c:532** | VAT 테스트 시작 | 채널 준비 완료 확인 |

### 📝 로그 분석

#### 성공적인 초기화 로그
```
cifX Toolkit initialized
Device added: cifX0
Cookie found: "netX"
Driver opened successfully
Channel opened successfully
Waiting for channel ready...
  [0 s] COS Flags: 0x00000001
*** Channel ready! ***
```

#### 실패한 초기화 로그
```
cifX Toolkit initialized
Device added: cifX0
Cookie timeout: 100ms elapsed
Driver open failed: 0x80000001
ERROR: xDriverOpen failed: 0x80000001
```

---

## 7. 고급 문제 해결

### 🔬 펌웨어 강제 다운로드

채널이 계속 준비되지 않는 경우:

1. **기존 VAT 테스트 비활성화**:
   ```c
   volatile bool g_bEnableVatTest = false;  // main.c:114
   ```

2. **기존 App 실행**:
   - 재빌드 및 실행
   - `App_CifXApplicationDemo`가 펌웨어 다운로드 수행

3. **VAT 테스트 재활성화**:
   ```c
   volatile bool g_bEnableVatTest = true;
   ```

### ⚙️ SPI 설정 확인

**hspi4 파라미터 확인** (main.c:600-618):
```c
hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
hspi4.Init.CLKPolarity = SPI_POLARITY_HIGH;
hspi4.Init.CLKPhase = SPI_PHASE_2EDGE;
```

netX90 SPI0 요구사항:
- **Polarity**: CPOL=1 (Idle High)
- **Phase**: CPHA=1 (Second Edge)
- **Speed**: ≤ 18 MHz

### 🔄 netX90 리셋

하드웨어 리셋으로 문제 해결:

1. **STM32 프로그램 중지**
2. **netX90 전원 OFF** (5초 대기)
3. **netX90 전원 ON**
4. **STM32 프로그램 재시작**

---

## 8. 예상 테스트 결과

### ✅ 시나리오 1: 정상 동작 (DeviceNet 연결됨)

```
*** Channel ready! ***
  COS Flags: 0x00000001
  Time taken: 2 seconds

========================================
 Running VAT Test Mode: 1
========================================
Test: Basic Pressure Control
Cycles: 10
Setpoint: 500

[VAT Output 0x08] Mode=2, Setpoint=500, Instance=0
[VAT Input 0x64] Exception=0x00, Pressure=498, Position=850, Status=0x01
[VAT Output 0x08] Mode=2, Setpoint=500, Instance=0
[VAT Input 0x64] Exception=0x00, Pressure=501, Position=855, Status=0x01
...

========== VAT Test Statistics ==========
Total Read Operations:   10
Total Write Operations:  10
Read Errors:             0
Write Errors:            0
=========================================
```

### ⚠️ 시나리오 2: 채널 준비됨 (DeviceNet 미연결)

```
*** Channel ready! ***
  COS Flags: 0x00000001
  Time taken: 2 seconds

========================================
 Running VAT Test Mode: 1
========================================
Test: Basic Pressure Control
Cycles: 10
Setpoint: 500

[VAT Output 0x08] Mode=2, Setpoint=500, Instance=0
[VAT Test] Read error: 0x80000003 (CIFX_DEV_NO_COM_FLAG)
[VAT Output 0x08] Mode=2, Setpoint=500, Instance=0
[VAT Test] Read error: 0x80000003 (CIFX_DEV_NO_COM_FLAG)
...

========== VAT Test Statistics ==========
Total Read Operations:   10
Total Write Operations:  10
Read Errors:             10
Write Errors:            0
=========================================
```

**해석**: 채널은 준비되었지만 DeviceNet 버스에 통신 상대가 없음

### ❌ 시나리오 3: 타임아웃

앞서 설명한 타임아웃 메시지 출력 후 프로그램 정상 종료

---

## 9. 체크리스트

### ✅ 빌드 전

- [ ] `vat_devicenet_test.h` 파일 존재 (`Hil_DemoAppDNS/Includes/`)
- [ ] `vat_devicenet_test.c` 파일 존재 (`Hil_DemoAppDNS/Sources/`)
- [ ] main.c 수정 적용 완료 (타임아웃 로직)
- [ ] `g_bEnableVatTest = true` 확인

### ✅ 실행 전

- [ ] STM32F429 전원 공급 (USB 연결)
- [ ] netX90 전원 공급 및 LED 확인
- [ ] SPI 케이블 연결 확인 (MOSI, MISO, SCK, CS)
- [ ] 시리얼 터미널 오픈 (115200 baud, UART5)

### ✅ 실행 중

- [ ] "Waiting for channel ready..." 메시지 확인
- [ ] COS Flags 주기적 출력 확인
- [ ] 5초 내 채널 준비 완료 확인

### ✅ 문제 발생 시

- [ ] Cookie 값 확인 (`g_szLastCookie`)
- [ ] COS Flags 마지막 값 확인
- [ ] 펌웨어 파일 존재 확인
- [ ] SPI 통신 상태 확인
- [ ] netX90 리셋 시도

---

## 10. 추가 참고 자료

### 📚 관련 문서

- **VAT 테스트 가이드**: `test/20251027_VAT_DeviceNet_Test_Guide.md`
- **메인 통합 가이드**: `test/20251027_Main_Integration_Guide.md`
- **Live Expression 설정**: `20251027_LiveExpression_GlobalVariables.md`
- **EDS 분석**: `20251027_DNS_V5_EDS_Analysis.md`

### 🔗 Hilscher 문서

- **cifX API 레퍼런스**: cifX Toolkit Documentation
- **DPM 구조**: Dual Port Memory Layout Specification
- **DeviceNet 프로토콜**: DeviceNet netX Protocol API Manual

### 💡 도움말

**문제가 계속 발생하는 경우**:
1. 기존 작동하는 `App_CifXApplicationDemo` 실행 테스트
2. netX90 펌웨어 버전 확인
3. SPI 로직 분석기로 신호 확인
4. Hilscher 기술 지원 문의

---

## 11. 요약

### 🎯 핵심 변경 사항

| 항목 | 이전 | 이후 |
|------|------|------|
| **대기 방식** | 무한 루프 | 5초 타임아웃 |
| **디버깅** | 없음 | 1초마다 COS 플래그 출력 |
| **에러 처리** | 없음 | 타임아웃 시 원인 분석 출력 |
| **테스트 실행** | 무조건 실행 | 조건부 실행 (채널 준비 시만) |

### ✅ 개선 효과

1. **무한 대기 방지**: 5초 후 자동 타임아웃
2. **실시간 모니터링**: COS 플래그 변화 추적 가능
3. **명확한 에러 메시지**: 문제 원인 파악 용이
4. **안전한 실행**: 채널 미준비 시 테스트 스킵

### 🔄 다음 단계

타임아웃 발생 시:
1. Cookie 및 COS Flags 확인
2. SPI 통신 검증
3. 펌웨어 파일 확인
4. 하드웨어 연결 재점검

---

**문서 끝**

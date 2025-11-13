# DeviceNet 통신 및 Live Expressions 모니터링 가이드

**작성일시**: 2025-10-29
**목적**: NetHost/CYCON.net DeviceNet Master와 통신하면서 STM32CubeIDE Live Expressions로 실시간 데이터 모니터링
**프로젝트**: STM32 F429 + netX90 DeviceNet Slave
**대상**: VAT Adaptive Pressure Controller (Vendor 404, Product 650)

---

## 📋 개요

DeviceNet Master(NetHost 또는 CYCON.net)와 netX90 Slave 간 통신을 설정하고, STM32CubeIDE의 Live Expressions 기능으로 실시간 I/O 데이터를 모니터링하는 방법을 설명합니다.

**통신 구조**:
```
[DeviceNet Master]          [netX90 Slave]          [STM32 F429]
(NetHost/CYCON.net)  <-->  (DeviceNet Stack)  <-->  (Host Application)
                                  ↓                         ↓
                            [DPM Memory]  <-------------> [Live Expressions]
```

---

## 🔧 1단계: 하드웨어 및 소프트웨어 준비

### 필요 장비

1. **DeviceNet Master**
   - NetHost (Hilscher DeviceNet Scanner) 또는
   - CYCON.net (Softing DeviceNet Configuration Tool)
   - DeviceNet USB/PCI 인터페이스 카드

2. **DeviceNet Slave**
   - STM32 F429 + netX90 보드
   - DeviceNet 케이블 연결
   - 24V 전원 공급

3. **개발 환경**
   - STM32CubeIDE (1.13.0 이상)
   - ST-Link Debugger

### DeviceNet 네트워크 설정

**물리적 연결**:
```
DeviceNet Master Interface
    |
    | DeviceNet Cable (CAN_H, CAN_L, V+, V-)
    |
    └── netX90 Slave (Node Address: 1)
        - CAN_H: DeviceNet High
        - CAN_L: DeviceNet Low
        - V+: 24V Power
        - V-: Ground
        - 종단 저항: 121Ω (필요 시)
```

**네트워크 파라미터**:
- **Baud Rate**: 125 kbps (기본값)
- **Node Address**: 1 (코드에서 설정 가능)
- **MAC ID**: Unique per device

---

## 🌐 2단계: DeviceNet Master 설정

### Option A: NetHost 사용

#### NetHost 설치 및 실행

1. **NetHost 다운로드**
   - Hilscher 웹사이트에서 NetHost 다운로드
   - Windows에 설치

2. **NetHost 실행**
   ```
   NetHost.exe 실행
   → Select Protocol: DeviceNet
   → Select Interface: [DeviceNet USB/PCI 카드 선택]
   ```

3. **Network Configuration**
   ```
   Baud Rate: 125 kbps
   MAC ID: 0 (Master)
   Auto-Allocation: Enabled (선택사항)
   ```

#### NetHost 네트워크 스캔

1. **Online 모드 진입**
   ```
   Toolbar → Online 버튼 클릭
   또는 F5 키
   ```

2. **Device Scan 실행**
   ```
   Network → Scan Network
   → 스캔 범위: 0-63 (전체) 또는 1 (특정 노드)
   → Scan 버튼 클릭
   ```

3. **스캔 결과 확인**
   ```
   Node 1 발견 시:
   - Node Address: 1
   - Vendor: VAT Vakuumventile AG (404)
   - Product: VAT Adaptive Pressure Controller (650)
   - Device Type: Process Control Valve (29)
   - State: Operational
   ```

#### NetHost I/O Connection 설정

1. **디바이스 더블클릭**
   ```
   스캔된 Node 1 더블클릭
   → Device Properties 창 열림
   ```

2. **I/O Connection 설정**
   ```
   Connections 탭 선택
   → Add Connection 버튼

   Connection Parameters:
   - Connection Type: I/O (Polled)
   - Consuming Path: Class 0x04, Instance 0x08 (Output Assembly 8)
   - Producing Path: Class 0x04, Instance 0x64 (Input Assembly 100)
   - RPI (Request Packet Interval): 100 ms (또는 원하는 주기)

   Apply → OK
   ```

3. **I/O Monitoring 시작**
   ```
   I/O Monitor 탭 선택
   → Start I/O 버튼 클릭

   Consuming Data (Master → Slave, 5 bytes):
   Byte 0: Control Mode [수동 입력 가능]
   Byte 1-2: Control Setpoint [수동 입력 가능]
   Byte 3: Control Instance [수동 입력 가능]
   Byte 4: Reserved [수동 입력 가능]

   Producing Data (Slave → Master, 7 bytes):
   Byte 0: Exception Status [실시간 표시]
   Byte 1-2: Pressure [실시간 표시]
   Byte 3-4: Position [실시간 표시]
   Byte 5: Device Status [실시간 표시]
   Byte 6: Access Mode [실시간 표시]
   ```

---

### Option B: CYCON.net 사용

#### CYCON.net 설치 및 실행

1. **CYCON.net 다운로드**
   - Softing 웹사이트에서 CYCON.net 다운로드
   - Windows에 설치

2. **CYCON.net 실행**
   ```
   CYCON.net 실행
   → New Project 생성
   → Protocol: DeviceNet
   → Interface: [DeviceNet 인터페이스 선택]
   ```

3. **Network Settings**
   ```
   Network → Network Settings
   - Baud Rate: 125 kbps
   - Master MAC ID: 0
   - Auto-Start: Enabled
   ```

#### CYCON.net 디바이스 스캔

1. **Scanner 모드**
   ```
   Tools → Scanner
   → Scan Range: 1-63
   → Start Scan 버튼 클릭
   ```

2. **스캔 결과**
   ```
   Node 1이 발견되면:
   - Hardware Device: VAT Adaptive Pressure Controller
   - DTM Device: 476297.EDS (자동 매칭)
   - Vendor: VAT Vakuumventile AG
   - Product Code: 650
   - Device Type: 29
   ```

3. **EDS 자동 매칭 확인**
   ```
   스캔된 디바이스에 체크 표시 (✓)
   → DTM Device 컬럼에 "476297.EDS" 표시되는지 확인

   ✅ 정상: 476297.EDS
   ❌ 오류: DNS_V5_SIMPLE_CONFIG_DEMO.EDS
   ```

#### CYCON.net I/O Connection 설정

1. **디바이스를 프로젝트에 추가**
   ```
   스캔 결과에서 Node 1 선택
   → Add to Project 버튼
   → Device 아이콘이 프로젝트 트리에 추가됨
   ```

2. **Connection Configuration**
   ```
   프로젝트 트리에서 Node 1 더블클릭
   → Parameters 탭 선택

   I/O Connection:
   - Produced Connection Path: 0x04 0x64 0x2C 0x64 (Input Assembly 100)
   - Consumed Connection Path: 0x04 0x08 0x2C 0x08 (Output Assembly 8)
   - Expected Packet Rate: 100 ms
   - Connection Type: Polled

   Save Configuration
   ```

3. **Download to Master**
   ```
   Network → Download Configuration
   → Master 인터페이스로 설정 다운로드
   → Download 완료 후 "Go Online" 클릭
   ```

4. **I/O Data Monitor**
   ```
   프로젝트 트리에서 Node 1 선택
   → I/O Data 탭

   Output Assembly 8 (Master → Slave):
   - Byte 0: Control Mode (수동 입력 가능)
   - Byte 1-2: Control Setpoint (INT16, 수동 입력 가능)
   - Byte 3: Control Instance (수동 입력 가능)
   - Byte 4: Reserved (0x00)

   Input Assembly 100 (Slave → Master):
   - Byte 0: Exception Status (자동 업데이트)
   - Byte 1-2: Pressure (INT16, 자동 업데이트)
   - Byte 3-4: Position (INT16, 자동 업데이트)
   - Byte 5: Device Status (자동 업데이트)
   - Byte 6: Access Mode (자동 업데이트)

   → Start I/O 버튼으로 통신 시작
   ```

---

## 🖥️ 3단계: STM32CubeIDE 디버깅 설정

### Debug Configuration 생성

1. **Debug 설정 열기**
   ```
   Run → Debug Configurations...
   → STM32 Cortex-M C/C++ Application 선택
   → New Configuration 버튼 (또는 기존 선택)
   ```

2. **Main 탭 설정**
   ```
   Project: netx_90_test_f429
   C/C++ Application: Debug/netx_90_test_f429.elf
   Build Configuration: Debug

   ✅ Enable auto build 체크
   ```

3. **Debugger 탭 설정**
   ```
   Debug Probe: ST-Link (ST-Link GDB server)
   Interface: SWD

   Serial Wire Viewer (SWV):
   ✅ Enable 체크
   Core Clock: 180 MHz (STM32 F429 기본값)
   ```

4. **Startup 탭 설정**
   ```
   Initialization Commands:
   monitor reset halt

   Load Symbols:
   ✅ Load executable 체크

   Run Commands:
   ✅ Set breakpoint at: main
   ❌ Resume 체크 해제 (수동 실행)
   ```

5. **Apply → Debug 클릭**

### 디버깅 시작

1. **Debug Perspective 전환**
   ```
   디버깅 시작 시 자동으로 Debug Perspective로 전환
   (또는 Window → Perspective → Open Perspective → Debug)
   ```

2. **초기 브레이크포인트**
   ```
   main() 함수 시작점에서 자동 정지
   → F8 키 또는 Resume 버튼으로 실행 계속
   ```

3. **실행 모드 선택**
   - **Run (F8)**: 프로그램 계속 실행 (브레이크포인트까지)
   - **Step Over (F6)**: 다음 줄 실행
   - **Step Into (F5)**: 함수 내부로 진입
   - **Step Return (F7)**: 현재 함수 종료 시까지 실행

---

## 📊 4단계: Live Expressions 설정

### Live Expressions 창 열기

1. **View 메뉴에서 열기**
   ```
   Window → Show View → Live Expressions

   또는

   Debug Perspective에서:
   Window → Show View → Debugger → Live Expressions
   ```

2. **Live Expressions 도킹**
   ```
   Live Expressions 탭을 Variables 탭 옆에 드래그 앤 드롭
   (편의상 Variables와 함께 배치)
   ```

### 모니터링 변수 추가

#### 방법 1: 직접 입력

1. **Add New Expression 버튼 클릭** (녹색 + 아이콘)

2. **변수명 입력** (아래 리스트 참고)

3. **Enter 키로 추가 완료**

#### 방법 2: 코드에서 드래그

1. **소스 코드 에디터에서 변수명 선택**
   ```c
   g_tAppData.tOutputData.output[0]  // 변수명 더블클릭 또는 드래그
   ```

2. **Live Expressions 창으로 드래그 앤 드롭**

---

## 📝 5단계: 모니터링할 변수 목록

### 핵심 I/O 데이터 변수

#### Output Data (Slave → Master, 7 bytes)

**현재 동작**: 500ms마다 1씩 증가하는 테스트 패턴

```c
// Live Expressions에 추가할 변수 (복사해서 사용)
g_tAppData.tOutputData.output[0]    // Exception Status (0→1→2→...→255→0)
g_tAppData.tOutputData.output[1]    // Pressure Low Byte
g_tAppData.tOutputData.output[2]    // Pressure High Byte
g_tAppData.tOutputData.output[3]    // Position Low Byte
g_tAppData.tOutputData.output[4]    // Position High Byte
g_tAppData.tOutputData.output[5]    // Device Status
g_tAppData.tOutputData.output[6]    // Access Mode
```

**예상 값**:
```
초기 상태: [0, 0, 0, 0, 0, 0, 0]
500ms 후: [1, 1, 1, 1, 1, 1, 1]
1초 후:   [2, 2, 2, 2, 2, 2, 2]
2초 후:   [4, 4, 4, 4, 4, 4, 4]
...
127.5초 후: [255, 255, 255, 255, 255, 255, 255]
128초 후:   [0, 0, 0, 0, 0, 0, 0]  (롤오버)
```

#### Input Data (Master → Slave, 5 bytes)

**NetHost/CYCON.net에서 송신한 데이터**

```c
// Live Expressions에 추가할 변수
g_tAppData.tInputData.input[0]      // Control Mode
g_tAppData.tInputData.input[1]      // Control Setpoint Low Byte
g_tAppData.tInputData.input[2]      // Control Setpoint High Byte
g_tAppData.tInputData.input[3]      // Control Instance
g_tAppData.tInputData.input[4]      // Reserved
```

**예상 값** (NetHost에서 "0x01, 0xF4, 0x01, 0x00, 0x00" 송신 시):
```
input[0] = 0x01  (Control Mode = Pressure Control)
input[1] = 0xF4  (Setpoint Low = 500 & 0xFF = 244)
input[2] = 0x01  (Setpoint High = 500 >> 8 = 1)
input[3] = 0x00  (Control Instance = 0)
input[4] = 0x00  (Reserved)
```

**INT16 복합 값 계산**:
```c
// Setpoint = input[2] << 8 | input[1]
// 예: 0x01F4 = (1 << 8) | 244 = 256 + 244 = 500
```

### 시스템 상태 변수

```c
// 통신 실행 상태
g_tAppData.fRunning               // true: 실행 중, false: 정지

// 입력 데이터 유효성
g_tAppData.fInputDataValid        // true: 유효한 데이터 수신, false: 미수신

// 채널 핸들 (NULL이 아니면 정상)
g_tAppData.aptChannels[0]->hChannel

// 채널 정보
g_tAppData.aptChannels[0]->tChannelInfo.bDeviceNumber
g_tAppData.aptChannels[0]->tChannelInfo.bChannelNumber
```

### 고급 모니터링 변수

#### 전체 구조체 보기

```c
// I/O 데이터 전체 구조체 (배열로 표시됨)
g_tAppData.tInputData
g_tAppData.tOutputData

// 전체 애플리케이션 데이터 (드릴다운 가능)
g_tAppData
```

#### 메모리 뷰 주소

```c
// 메모리 주소로 직접 모니터링 (Memory Browser 사용)
&g_tAppData.tInputData.input       // 입력 데이터 베이스 주소
&g_tAppData.tOutputData.output     // 출력 데이터 베이스 주소
```

---

## 🎯 6단계: 실시간 모니터링 실행

### 자동 업데이트 설정

1. **Live Expressions 자동 새로고침**
   ```
   Live Expressions 창 우클릭
   → Continuous Refresh 체크 (✓)

   또는

   Refresh Interval 설정: 100ms ~ 1000ms
   ```

2. **Variables 창 자동 새로고침**
   ```
   Variables 창 우클릭
   → Continuous Refresh 체크 (✓)
   ```

### 모니터링 시나리오 1: Output 데이터 증가 확인

**목적**: Slave가 500ms마다 7바이트를 1씩 증가시키는지 확인

1. **프로그램 실행**
   ```
   Resume (F8) 버튼으로 프로그램 계속 실행
   ```

2. **Live Expressions 관찰**
   ```
   g_tAppData.tOutputData.output[0]: 0 → 1 → 2 → 3 → ...
   g_tAppData.tOutputData.output[1]: 0 → 1 → 2 → 3 → ...
   ...
   g_tAppData.tOutputData.output[6]: 0 → 1 → 2 → 3 → ...

   주기: 약 500ms (OS_Sleep(500) 설정값)
   ```

3. **NetHost/CYCON.net 확인**
   ```
   Input Assembly 100 데이터가 동일하게 증가하는지 확인:
   Byte 0: 0 → 1 → 2 → 3 → ...
   Byte 1: 0 → 1 → 2 → 3 → ...
   ...
   Byte 6: 0 → 1 → 2 → 3 → ...
   ```

### 모니터링 시나리오 2: Input 데이터 송수신 확인

**목적**: Master가 보낸 데이터를 Slave가 정상 수신하는지 확인

1. **NetHost/CYCON.net에서 데이터 송신**
   ```
   Output Assembly 8 (Master → Slave) 설정:
   Byte 0: 0x02  (Control Mode)
   Byte 1: 0xE8  (Setpoint Low = 1000 & 0xFF = 232)
   Byte 2: 0x03  (Setpoint High = 1000 >> 8 = 3)
   Byte 3: 0x01  (Control Instance)
   Byte 4: 0x00  (Reserved)

   → Send 또는 Write 버튼 클릭
   ```

2. **Live Expressions 확인**
   ```
   g_tAppData.tInputData.input[0]: 0x02
   g_tAppData.tInputData.input[1]: 0xE8 (232)
   g_tAppData.tInputData.input[2]: 0x03
   g_tAppData.tInputData.input[3]: 0x01
   g_tAppData.tInputData.input[4]: 0x00

   g_tAppData.fInputDataValid: true (1)
   ```

3. **복합 값 계산 검증**
   ```
   Setpoint = input[2] << 8 | input[1]
            = 0x03 << 8 | 0xE8
            = 0x03E8
            = 1000 (10진수) ✓
   ```

### 모니터링 시나리오 3: 통신 상태 확인

**목적**: DeviceNet 통신 상태 및 채널 활성화 확인

1. **시스템 상태 변수 확인**
   ```
   g_tAppData.fRunning: true (1)
   g_tAppData.fInputDataValid: true (1) - 데이터 수신 중
   ```

2. **채널 핸들 확인**
   ```
   g_tAppData.aptChannels[0]: 0x20001234 (NULL이 아닌 유효한 주소)
   g_tAppData.aptChannels[0]->hChannel: 0x20001ABC (유효한 핸들)
   ```

3. **오류 발생 시**
   ```
   fRunning = false → 통신 루프 정지
   fInputDataValid = false → 입력 데이터 미수신
   aptChannels[0] = NULL → 채널 미오픈
   ```

---

## 🔍 7단계: 고급 디버깅 기법

### Breakpoint를 이용한 I/O 데이터 검증

1. **I/O Read 브레이크포인트**
   ```
   파일: Hil_DemoApp\Sources\App_DemoApplication.c
   라인: 378 (xChannelIORead 호출 직후)

   브레이크포인트 설정 → F8 실행
   → 브레이크 시 g_tAppData.tInputData 값 확인
   ```

2. **I/O Write 브레이크포인트**
   ```
   파일: Hil_DemoApp\Sources\App_DemoApplication.c
   라인: 405 (xChannelIOWrite 호출 직후)

   브레이크포인트 설정 → F8 실행
   → 브레이크 시 g_tAppData.tOutputData 값 확인
   ```

### Conditional Breakpoint (조건부 브레이크포인트)

**특정 값일 때만 정지**:

1. **브레이크포인트 우클릭 → Breakpoint Properties**

2. **Conditional 체크 후 조건 입력**
   ```c
   // 예: Output 데이터가 100일 때만 정지
   g_tAppData.tOutputData.output[0] == 100

   // 예: Input 데이터가 유효할 때만 정지
   g_tAppData.fInputDataValid == true
   ```

### Memory Browser로 직접 메모리 확인

1. **Memory Browser 열기**
   ```
   Window → Show View → Memory Browser
   ```

2. **메모리 주소 입력**
   ```
   Add Monitor 버튼 클릭
   → 주소 입력: &g_tAppData.tOutputData.output
   → Length: 7 bytes
   → Format: Hex (1 byte columns)
   ```

3. **실시간 메모리 뷰**
   ```
   주소: 0x20001000 (예시)
   +0x0: 00 01 02 03 04 05 06  (7 bytes)
         ↑  ↑  ↑  ↑  ↑  ↑  ↑
         0  1  2  3  4  5  6  (인덱스)
   ```

---

## 📈 8단계: 데이터 시각화 및 분석

### SWV ITM Data Console (Serial Wire Viewer)

**printf 디버깅 활성화**:

1. **SWV ITM Data Console 열기**
   ```
   Window → Show View → SWV → SWV ITM Data Console
   ```

2. **ITM Stimulus Port 설정**
   ```
   Configure 버튼 클릭
   → Port 0 체크 (✓)
   → Enable 클릭
   ```

3. **코드에 printf 추가** (선택사항)
   ```c
   // App_DemoApplication.c에 추가 (Line 405 근처)
   printf("Output: %d %d %d %d %d %d %d\n",
          ptAppData->tOutputData.output[0],
          ptAppData->tOutputData.output[1],
          ptAppData->tOutputData.output[2],
          ptAppData->tOutputData.output[3],
          ptAppData->tOutputData.output[4],
          ptAppData->tOutputData.output[5],
          ptAppData->tOutputData.output[6]);
   ```

4. **ITM Console 출력 확인**
   ```
   Output: 0 0 0 0 0 0 0
   Output: 1 1 1 1 1 1 1
   Output: 2 2 2 2 2 2 2
   ...
   ```

### Data Breakpoint (데이터 변경 감지)

**특정 변수가 변경될 때 자동 정지**:

1. **Variables 창에서 변수 우클릭**
   ```
   g_tAppData.tInputData.input[0] 우클릭
   → Add Data Breakpoint
   ```

2. **Data Breakpoint 설정**
   ```
   Type: Write (쓰기 시)
   Range: 1 byte

   → 이제 input[0] 값이 변경될 때마다 자동 정지
   ```

---

## 🛠️ 트러블슈팅

### 문제 1: Live Expressions가 업데이트되지 않음

**증상**:
- Live Expressions 값이 고정되어 있음
- Variables는 업데이트되는데 Live Expressions만 안됨

**해결 방법**:

1. **Continuous Refresh 활성화**
   ```
   Live Expressions 창 우클릭
   → Continuous Refresh 체크 (✓)
   ```

2. **수동 새로고침**
   ```
   Live Expressions 창의 Refresh 버튼 (순환 화살표) 클릭
   ```

3. **Debugger 재연결**
   ```
   Terminate 버튼으로 디버깅 종료
   → 다시 Debug 시작
   ```

---

### 문제 2: Output 데이터가 증가하지 않음

**증상**:
- `g_tAppData.tOutputData.output[0]` 값이 계속 0

**원인 및 해결**:

**원인 1: 프로그램이 정지되어 있음**
```
해결: Resume (F8) 버튼으로 프로그램 실행
```

**원인 2: VAT Test 모드가 활성화됨**
```
확인: main.c:122의 g_bEnableVatTest 값 확인
해결: g_bEnableVatTest = false; 설정 후 재빌드
```

**원인 3: 통신 루프가 실행되지 않음**
```
확인: g_tAppData.fRunning 값 확인
해결: fRunning이 false면 초기화 실패, 로그 확인
```

**원인 4: 브레이크포인트에 걸림**
```
해결: 모든 브레이크포인트 해제 또는 Continue 실행
```

---

### 문제 3: Input 데이터가 수신되지 않음

**증상**:
- NetHost에서 데이터를 보내도 `tInputData`가 변경 안됨
- `fInputDataValid`가 계속 false

**원인 및 해결**:

**원인 1: NetHost I/O Connection이 시작 안됨**
```
NetHost 확인:
→ Connection 상태가 "Established"인지 확인
→ "Start I/O" 버튼 클릭했는지 확인
```

**원인 2: Assembly Instance 불일치**
```
확인:
NetHost Consuming Path: 0x04, 0x08 (Assembly 8) ✓
Slave 설정: DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE = 0x08 ✓

불일치 시 재설정 필요
```

**원인 3: DeviceNet 케이블 문제**
```
확인:
→ CAN_H, CAN_L 연결 확인
→ 24V 전원 공급 확인
→ 종단 저항 확인 (네트워크 양 끝에 121Ω)
```

**원인 4: Baud Rate 불일치**
```
확인:
Master Baud Rate: 125 kbps
Slave Baud Rate: DNS_BAUDRATE_125kB (코드 설정)

→ 일치해야 통신 가능
```

---

### 문제 4: CYCON.net이 VAT EDS를 인식 못함

**증상**:
- DTM Device가 "DNS_V5_SIMPLE_CONFIG_DEMO"로 표시
- 476297.EDS로 매칭되지 않음

**원인 및 해결**:

**원인: CIP Identity가 VAT로 설정 안됨**
```
확인:
AppDNS_DemoApplicationFunctions.c:49-55

VENDOR_ID = 404 (VAT) ✓
PRODUCT_CODE = 650 (VAT) ✓

283/34로 설정되어 있으면 변경 필요
```

**해결: 20251029_VAT_EDS_Integration_Complete.md 참고**

---

### 문제 5: STM32가 응답 없음 (hang)

**증상**:
- 디버거 연결 안됨
- 프로그램 실행 안됨

**해결 방법**:

1. **하드웨어 리셋**
   ```
   STM32 보드의 Reset 버튼 누름
   또는
   전원 OFF → ON
   ```

2. **ST-Link 리셋**
   ```
   ST-Link USB 케이블 분리 → 재연결
   ```

3. **Connect Under Reset**
   ```
   Debug Configuration → Debugger 탭
   → Connection Setup: "Connect under reset" 체크
   → Apply → Debug
   ```

4. **Flash 메모리 삭제**
   ```
   STM32CubeProgrammer 실행
   → Connect
   → Full Chip Erase
   → 재빌드 후 다시 다운로드
   ```

---

## 📊 Live Expressions 설정 템플릿

### 복사해서 사용할 수 있는 변수 목록

**기본 I/O 모니터링**:
```c
g_tAppData.tOutputData.output[0]
g_tAppData.tOutputData.output[1]
g_tAppData.tOutputData.output[2]
g_tAppData.tOutputData.output[3]
g_tAppData.tOutputData.output[4]
g_tAppData.tOutputData.output[5]
g_tAppData.tOutputData.output[6]
g_tAppData.tInputData.input[0]
g_tAppData.tInputData.input[1]
g_tAppData.tInputData.input[2]
g_tAppData.tInputData.input[3]
g_tAppData.tInputData.input[4]
g_tAppData.fRunning
g_tAppData.fInputDataValid
```

**고급 모니터링**:
```c
g_tAppData.aptChannels[0]->hChannel
g_tAppData.tOutputData
g_tAppData.tInputData
&g_tAppData.tOutputData.output
```

---

## 🎯 통신 검증 체크리스트

### STM32 측 확인

- [ ] 프로젝트 빌드 성공 (0 errors)
- [ ] 플래시 다운로드 완료
- [ ] 디버거 연결 정상
- [ ] `g_bEnableVatTest = false` (일반 DeviceNet 모드)
- [ ] `g_tAppData.fRunning = true` (통신 실행 중)
- [ ] `g_tAppData.aptChannels[0] != NULL` (채널 오픈됨)
- [ ] Live Expressions 업데이트 확인
- [ ] Output 데이터 증가 확인 (0→1→2→...)

### NetHost/CYCON.net 측 확인

- [ ] DeviceNet 인터페이스 연결
- [ ] Baud Rate 125 kbps 설정
- [ ] Device Scan 성공 (Node 1 발견)
- [ ] VAT EDS 매칭 확인 (476297.EDS)
- [ ] I/O Connection 설정 완료
- [ ] Connection State: "Established"
- [ ] I/O Communication 시작 (Start I/O)
- [ ] Input Assembly 100 데이터 수신 확인 (증가 패턴)
- [ ] Output Assembly 8 데이터 송신 가능

### 하드웨어 확인

- [ ] DeviceNet 케이블 연결 (CAN_H, CAN_L)
- [ ] 24V 전원 공급
- [ ] 종단 저항 설치 (121Ω)
- [ ] LED 상태 확인 (통신 LED 깜빡임)
- [ ] ST-Link USB 연결

---

## 💡 활용 예제

### 예제 1: Pressure Setpoint 송신 테스트

**목표**: Master에서 압력 설정값 500 mbar를 Slave로 송신

1. **NetHost/CYCON.net 설정**
   ```
   Output Assembly 8:
   Byte 0: 0x01  (Control Mode = Pressure Control)
   Byte 1: 0xF4  (500 & 0xFF = 244)
   Byte 2: 0x01  (500 >> 8 = 1)
   Byte 3: 0x00  (Control Instance = 0)
   Byte 4: 0x00  (Reserved)

   → Send
   ```

2. **Live Expressions 확인**
   ```
   g_tAppData.tInputData.input[0]: 0x01 ✓
   g_tAppData.tInputData.input[1]: 0xF4 (244) ✓
   g_tAppData.tInputData.input[2]: 0x01 ✓
   g_tAppData.fInputDataValid: true ✓
   ```

3. **복합 값 검증**
   ```c
   // Watches 창에 추가:
   (g_tAppData.tInputData.input[2] << 8) | g_tAppData.tInputData.input[1]

   → 결과: 500 ✓
   ```

---

### 예제 2: 실시간 압력 모니터링

**목표**: Slave가 보내는 압력 데이터를 Master와 STM32CubeIDE에서 동시 확인

1. **Slave 코드 수정** (실제 압력 값 송신)
   ```c
   // App_DemoApplication.c:400 수정 (예시)
   int16_t pressure = 1234;  // 실제 센서 값
   ptAppData->tOutputData.output[1] = pressure & 0xFF;
   ptAppData->tOutputData.output[2] = (pressure >> 8) & 0xFF;
   ```

2. **Live Expressions 확인**
   ```
   g_tAppData.tOutputData.output[1]: 0xD2 (210)
   g_tAppData.tOutputData.output[2]: 0x04 (4)
   ```

3. **NetHost Input Assembly 100 확인**
   ```
   Byte 1: 0xD2
   Byte 2: 0x04

   Pressure (INT16): (0x04 << 8) | 0xD2 = 1234 ✓
   ```

4. **Watches로 복합 값 표시**
   ```c
   // Watches 창에 추가:
   (g_tAppData.tOutputData.output[2] << 8) | g_tAppData.tOutputData.output[1]

   → 결과: 1234 mbar ✓
   ```

---

## 📚 참고 자료

### 관련 문서

- **20251029_VAT_EDS_Integration_Complete.md**: VAT EDS 통합 작업 내용
- **20251029_CYCON_EDS_Mismatch_Analysis.md**: CYCON EDS 매칭 분석
- **20251029_DeviceNet_Master_Scan_Issue_Analysis.md**: Master 스캔 문제 분석

### DeviceNet 프로토콜

- **CIP Volume 1**: Common Industrial Protocol 사양
- **DeviceNet Specification**: DeviceNet 프로토콜 상세
- **VAT 476297.eds**: VAT Adaptive Pressure Controller EDS 파일

### STM32CubeIDE

- **User Manual**: STM32CubeIDE 사용자 매뉴얼
- **Debugging Guide**: STM32 디버깅 가이드
- **Live Expressions**: 실시간 변수 모니터링 매뉴얼

---

**작업 완료**: 2025-10-29
**작성자**: Claude AI Assistant
**문서 버전**: 1.0


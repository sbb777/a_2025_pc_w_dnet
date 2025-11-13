# VAT Adaptive Pressure Controller DeviceNet 통신 테스트 가이드
**작성일시**: 2025-10-27

---

## 1. 개요

### 📋 프로젝트 정보

이 테스트 프로그램은 **VAT Adaptive Pressure Controller**와 DeviceNet 통신을 테스트하기 위한 완전한 솔루션입니다.

| 항목 | 값 |
|------|-----|
| **EDS 파일** | 476297.eds |
| **제조사** | VAT Vakuumventile AG |
| **Vendor Code** | 404 |
| **Product Code** | 650 |
| **제품명** | VAT Adaptive Pressure Controller |
| **장치 타입** | Process Control Valve (Type 29) |
| **프로토콜** | DeviceNet / CIP (Common Industrial Protocol) |
| **펌웨어 버전** | Major: 2, Minor: 1 |

### 🎯 테스트 목적

1. **DeviceNet 통신 검증**: VAT 압력 컨트롤러와의 실시간 데이터 교환
2. **Assembly Instance 테스트**: 다양한 입출력 Assembly 조합 검증
3. **제어 모드 검증**: 압력/위치/수동 제어 등 6가지 모드 테스트
4. **에러 처리**: 예외 상태 및 통신 에러 핸들링 검증
5. **통계 분석**: 통신 품질 및 성능 측정

---

## 2. EDS 파일 분석 요약

### 🔄 지원 통신 모드

| 모드 | 설명 | 지원 여부 |
|------|------|----------|
| **Poll** | 마스터 주기적 폴링 | ✅ 지원 (기본) |
| **Strobe** | 마스터 트리거 응답 | ✅ 지원 |
| **COS** | 데이터 변경 시 전송 | ✅ 지원 |
| **Cyclic** | 고정 주기 자동 전송 | ✅ 지원 |

### 📥 입력 Assembly (슬레이브 → 마스터)

EDS 파일에 **24개의 입력 Assembly** 정의됨. 주요 Assembly:

#### Input Assembly 1 (0x01) - 2 바이트
```c
typedef struct {
    int16_t process_variable;  // 프로세스 변수값
} VAT_INPUT_ASSEMBLY_1_T;
```
- **용도**: 기본 프로세스 변수 모니터링
- **크기**: 2 바이트

#### Input Assembly 2 (0x02) - 3 바이트
```c
typedef struct {
    uint8_t exception_status;  // 예외 상태 플래그
    int16_t pressure;          // 압력 측정값
} VAT_INPUT_ASSEMBLY_2_T;
```
- **용도**: 압력 모니터링 + 예외 상태
- **크기**: 3 바이트

#### Input Assembly 3 (0x03) - 5 바이트
```c
typedef struct {
    uint8_t exception_status;  // 예외 상태 플래그
    int16_t pressure;          // 압력 측정값
    int16_t position;          // 밸브 위치
} VAT_INPUT_ASSEMBLY_3_T;
```
- **용도**: 압력 + 위치 모니터링
- **크기**: 5 바이트

#### Input Assembly 100 (0x64) - 7 바이트 ⭐ 권장
```c
typedef struct {
    uint8_t exception_status;  // 예외 상태 플래그
    int16_t pressure;          // 압력 측정값
    int16_t position;          // 밸브 위치
    uint8_t device_status;     // 장치 상태
    uint8_t access_mode;       // 액세스 모드
} VAT_INPUT_ASSEMBLY_100_T;
```
- **용도**: 전체 상태 모니터링 (권장)
- **크기**: 7 바이트
- **CIP Path**: 20 04 24 64 30 03

#### Input Assembly 101 (0x65) - 7 바이트
```c
typedef struct {
    uint8_t exception_status;  // 예외 상태 플래그
    int16_t pressure;          // 압력 측정값
    int16_t position;          // 밸브 위치
    uint8_t discrete_inputs;   // 디지털 입력 상태
    uint8_t device_status;     // 장치 상태
} VAT_INPUT_ASSEMBLY_101_T;
```
- **용도**: 전체 상태 + 디지털 입력
- **크기**: 7 바이트

### 📤 출력 Assembly (마스터 → 슬레이브)

EDS 파일에 **11개의 출력 Assembly** 정의됨. 주요 Assembly:

#### Output Assembly 7 (0x07) - 4 바이트
```c
typedef struct {
    int16_t control_setpoint;   // 제어 설정값
    uint16_t control_instance;  // 제어 인스턴스
} VAT_OUTPUT_ASSEMBLY_7_T;
```
- **용도**: 기본 제어 (설정값만)
- **크기**: 4 바이트
- **CIP Path**: 20 04 24 07 30 03

#### Output Assembly 8 (0x08) - 5 바이트 ⭐ 권장
```c
typedef struct {
    uint8_t control_mode;       // 제어 모드
    int16_t control_setpoint;   // 제어 설정값
    uint16_t control_instance;  // 제어 인스턴스
} VAT_OUTPUT_ASSEMBLY_8_T;
```
- **용도**: 모드 선택 + 제어 (권장)
- **크기**: 5 바이트
- **CIP Path**: 20 04 24 08 30 03

#### Output Assembly 102 (0x66) - 8 바이트
```c
typedef struct {
    uint8_t control_mode;           // 제어 모드
    int16_t control_setpoint;       // 제어 설정값
    uint16_t control_instance;      // 제어 인스턴스
    uint8_t auto_learn;             // 자동 학습 활성화
    uint8_t calibration_scale;      // 교정 스케일
    uint8_t zero_adjust;            // 영점 조정
} VAT_OUTPUT_ASSEMBLY_102_T;
```
- **용도**: 전체 제어 + 교정
- **크기**: 8 바이트
- **CIP Path**: 20 04 24 66 30 03

---

## 3. 제어 모드 정의

### 🎛️ 6가지 제어 모드

```c
typedef enum {
    VAT_CONTROL_MODE_CLOSED         = 0x00,  // 밸브 완전 닫힘
    VAT_CONTROL_MODE_OPEN           = 0x01,  // 밸브 완전 열림
    VAT_CONTROL_MODE_PRESSURE       = 0x02,  // 압력 제어 모드
    VAT_CONTROL_MODE_POSITION       = 0x03,  // 위치 제어 모드
    VAT_CONTROL_MODE_MANUAL         = 0x04,  // 수동 제어 모드
    VAT_CONTROL_MODE_THROTTLE       = 0x05   // 쓰로틀 제어 모드
} VAT_CONTROL_MODE_E;
```

### 📊 모드별 특성

| 모드 | 코드 | 설명 | 사용 시나리오 |
|------|------|------|--------------|
| **CLOSED** | 0x00 | 밸브 완전 닫힘 | 시스템 정지, 안전 상태 |
| **OPEN** | 0x01 | 밸브 완전 열림 | 최대 유량, 펌프다운 |
| **PRESSURE** | 0x02 | 압력 제어 | 일정 압력 유지 (PID 제어) |
| **POSITION** | 0x03 | 위치 제어 | 특정 밸브 개도율 유지 |
| **MANUAL** | 0x04 | 수동 제어 | 직접 제어, 테스트 |
| **THROTTLE** | 0x05 | 쓰로틀 제어 | 유량 제어, 미세 조정 |

---

## 4. 예외 상태 및 장치 상태 비트

### ⚠️ Exception Status Bits (예외 상태)

```c
#define VAT_EXCEPTION_SETPOINT_OUT_OF_RANGE     (1 << 0)  // 설정값 범위 초과
#define VAT_EXCEPTION_HARDWARE_FAULT            (1 << 1)  // 하드웨어 고장
#define VAT_EXCEPTION_SENSOR_FAULT              (1 << 2)  // 센서 고장
#define VAT_EXCEPTION_COMMUNICATION_FAULT       (1 << 3)  // 통신 오류
#define VAT_EXCEPTION_CALIBRATION_ERROR         (1 << 4)  // 교정 오류
#define VAT_EXCEPTION_OVERTEMPERATURE           (1 << 5)  // 과열
#define VAT_EXCEPTION_UNDER_VACUUM              (1 << 6)  // 진공 부족
#define VAT_EXCEPTION_OVER_PRESSURE             (1 << 7)  // 과압
```

### ✅ Device Status Bits (장치 상태)

```c
#define VAT_DEVICE_STATUS_READY                 (1 << 0)  // 준비 완료
#define VAT_DEVICE_STATUS_FAULT                 (1 << 1)  // 고장 발생
#define VAT_DEVICE_STATUS_CALIBRATING           (1 << 2)  // 교정 중
#define VAT_DEVICE_STATUS_LEARN_MODE            (1 << 3)  // 학습 모드
#define VAT_DEVICE_STATUS_REMOTE_MODE           (1 << 4)  // 원격 제어 모드
#define VAT_DEVICE_STATUS_LOCAL_MODE            (1 << 5)  // 로컬 제어 모드
```

---

## 5. 파일 구조

### 📂 생성된 파일 목록

```
test/
├── vat_devicenet_test.h           # 헤더 파일 (구조체, 함수 프로토타입)
├── vat_devicenet_test.c           # 구현 파일 (테스트 함수)
├── vat_test_main.c                # 메인 애플리케이션
├── Makefile                       # 빌드 스크립트
└── 20251027_VAT_DeviceNet_Test_Guide.md  # 이 문서
```

### 📄 파일별 설명

#### `vat_devicenet_test.h`
- **목적**: 데이터 구조 정의 및 함수 선언
- **포함 내용**:
  - 장치 식별 정보 (Vendor/Product Code)
  - Assembly Instance 정의
  - 입출력 데이터 구조체
  - 제어 모드 열거형
  - 테스트 설정 및 통계 구조체
  - 함수 프로토타입

#### `vat_devicenet_test.c`
- **목적**: 테스트 함수 구현
- **주요 함수**:
  - `VAT_Test_Init()`: 초기화
  - `VAT_Test_ReadInput()`: 입력 데이터 읽기
  - `VAT_Test_WriteOutput()`: 출력 데이터 쓰기
  - `VAT_Test_SetControlMode()`: 제어 모드 설정
  - `VAT_Test_SetPressureSetpoint()`: 압력 설정값 설정
  - `VAT_Test_GetStats()`: 통계 조회
  - `VAT_Test_LogInputData()`: 입력 데이터 로깅

#### `vat_test_main.c`
- **목적**: 메인 애플리케이션
- **2가지 빌드 모드**:
  - **Hardware Mode**: 실제 cifX 하드웨어 사용
  - **Standalone Mode**: 하드웨어 없이 테스트 (Mock)

#### `Makefile`
- **목적**: 자동 빌드
- **타겟**:
  - `make`: 기본 빌드 (standalone)
  - `make standalone`: 하드웨어 없이 빌드
  - `make hardware`: cifX 하드웨어용 빌드
  - `make clean`: 정리

---

## 6. 컴파일 및 실행

### 🔨 빌드 방법

#### 방법 1: Standalone 모드 (하드웨어 불필요)

```bash
cd test
make standalone
./vat_test_standalone
```

**특징**:
- cifX 하드웨어 없이 실행 가능
- Mock 함수 사용
- 테스트 로직 검증용
- 개발 및 디버깅에 적합

#### 방법 2: Hardware 모드 (실제 하드웨어)

```bash
cd test
make hardware
sudo ./vat_test_hardware
```

**특징**:
- 실제 cifX 하드웨어 필요
- cifX Toolkit 라이브러리 필요
- 실제 VAT 장치와 통신
- 프로덕션 테스트용

#### 방법 3: Windows 빌드 (MinGW)

```bash
cd test
make windows
vat_test.exe
```

**특징**:
- Windows 실행 파일 생성
- MinGW 컴파일러 필요
- Standalone 모드로 빌드

### 🛠️ 빌드 옵션

#### cifX Toolkit 경로 지정

```bash
make hardware CIFX_TOOLKIT_PATH=/path/to/netx_tk
```

#### 디버그 빌드

```bash
make standalone CFLAGS="-Wall -Wextra -g -DENABLE_LOGGING"
```

#### 최적화 빌드

```bash
make standalone CFLAGS="-Wall -O3 -DENABLE_LOGGING"
```

---

## 7. 사용 예시

### 📝 기본 사용 (Standalone 모드)

```bash
$ cd test
$ make standalone
$ ./vat_test_standalone

========================================
 VAT Pressure Controller Test v1.0
 Standalone Test Mode (No Hardware)
========================================

[VAT Test] Initialized
[VAT Test] Configured:
  Node Address: 10
  Baud Rate: 250000 bps
  EPR: 100 ms
  Input Assembly: 0x64
  Output Assembly: 0x08

========================================
 Running Test Scenarios (Mock)
========================================

========== Basic Pressure Control Test ==========
[VAT Output 0x07] Setpoint=500, Instance=0
[VAT Input 0x02] Exception=0x00, Pressure=0
...
=================================================

========== Full Calibration Test ==========
[VAT Test] Control mode set to: 2
[VAT Output 0x66] Mode=2, Setpoint=0, Instance=0, Learn=1, Cal=100, Zero=0
[VAT Input 0x64] Exception=0x00, Pressure=0, Position=0, Status=0x00, Access=0x00
Device is ready
===========================================

========== VAT Test Statistics ==========
Total Read Operations:   18
Total Write Operations:  18
Read Errors:             0
Write Errors:            0
Exception Count:         0
Timeout Count:           0
Last Error Code:         0x00000000
=========================================
```

### 📝 실제 하드웨어 테스트

```c
// 사용자 코드 예시
#include "vat_devicenet_test.h"
#include "cifXUser.h"

int main(void)
{
    CIFXHANDLE hDriver, hChannel;
    VAT_TEST_CONTEXT_T tContext;

    // 1. 초기화
    VAT_Test_Init(&tContext);

    // 2. 설정
    VAT_TEST_CONFIG_T tConfig = {
        .node_address = 10,
        .baud_rate = 250000,
        .epr_ms = 100,
        .input_assembly = VAT_INPUT_ASSEMBLY_100,
        .output_assembly = VAT_OUTPUT_ASSEMBLY_8,
        .enable_logging = true,
        .enable_validation = true
    };
    VAT_Test_Configure(&tContext, &tConfig);

    // 3. cifX 드라이버/채널 열기
    xDriverOpen(&hDriver);
    xChannelOpen(hDriver, "cifX0", 0, &hChannel);

    // 4. 압력 제어 모드 설정
    VAT_Test_SetControlMode(&tContext, VAT_CONTROL_MODE_PRESSURE);
    VAT_Test_SetPressureSetpoint(&tContext, 1000);

    // 5. 통신 루프
    for (int i = 0; i < 100; i++)
    {
        // 제어 명령 전송
        VAT_Test_WriteOutput(&tContext, hChannel);

        // 피드백 읽기
        VAT_Test_ReadInput(&tContext, hChannel);

        // 예외 확인
        if (VAT_Test_HasException(&tContext))
        {
            printf("Exception: 0x%02X\n",
                   VAT_Test_GetExceptionStatus(&tContext));
        }

        // 100ms 대기
        usleep(100000);
    }

    // 6. 통계 출력
    VAT_Test_PrintStats(&tContext);

    // 7. 종료
    xChannelClose(hChannel);
    xDriverClose(hDriver);
    VAT_Test_Deinit(&tContext);

    return 0;
}
```

---

## 8. API 레퍼런스

### 🔧 초기화 및 설정

#### `void VAT_Test_Init(VAT_TEST_CONTEXT_T* ptContext)`
- **기능**: 테스트 컨텍스트 초기화
- **파라미터**:
  - `ptContext`: 초기화할 컨텍스트
- **반환값**: 없음

#### `void VAT_Test_Configure(VAT_TEST_CONTEXT_T* ptContext, const VAT_TEST_CONFIG_T* ptConfig)`
- **기능**: 테스트 파라미터 설정
- **파라미터**:
  - `ptContext`: 컨텍스트
  - `ptConfig`: 설정 구조체
- **반환값**: 없음

#### `void VAT_Test_Deinit(VAT_TEST_CONTEXT_T* ptContext)`
- **기능**: 테스트 종료 및 통계 출력
- **파라미터**:
  - `ptContext`: 컨텍스트
- **반환값**: 없음

### 📡 통신 함수

#### `int32_t VAT_Test_ReadInput(VAT_TEST_CONTEXT_T* ptContext, void* hChannel)`
- **기능**: 입력 데이터 읽기 (슬레이브 → 마스터)
- **파라미터**:
  - `ptContext`: 컨텍스트
  - `hChannel`: cifX 채널 핸들
- **반환값**:
  - `CIFX_NO_ERROR` (0): 성공
  - 기타: 에러 코드

#### `int32_t VAT_Test_WriteOutput(VAT_TEST_CONTEXT_T* ptContext, void* hChannel)`
- **기능**: 출력 데이터 쓰기 (마스터 → 슬레이브)
- **파라미터**:
  - `ptContext`: 컨텍스트
  - `hChannel`: cifX 채널 핸들
- **반환값**:
  - `CIFX_NO_ERROR` (0): 성공
  - 기타: 에러 코드

### 🎛️ 제어 함수

#### `void VAT_Test_SetControlMode(VAT_TEST_CONTEXT_T* ptContext, VAT_CONTROL_MODE_E eMode)`
- **기능**: 제어 모드 설정
- **파라미터**:
  - `ptContext`: 컨텍스트
  - `eMode`: 제어 모드 (CLOSED/OPEN/PRESSURE/POSITION/MANUAL/THROTTLE)
- **반환값**: 없음

#### `void VAT_Test_SetPressureSetpoint(VAT_TEST_CONTEXT_T* ptContext, int16_t pressure)`
- **기능**: 압력 설정값 설정
- **파라미터**:
  - `ptContext`: 컨텍스트
  - `pressure`: 압력 설정값 (스케일된 정수값)
- **반환값**: 없음

### 📊 상태 함수

#### `bool VAT_Test_IsDeviceReady(const VAT_TEST_CONTEXT_T* ptContext)`
- **기능**: 장치 준비 상태 확인
- **파라미터**:
  - `ptContext`: 컨텍스트
- **반환값**:
  - `true`: 장치 준비 완료
  - `false`: 장치 준비 안됨

#### `bool VAT_Test_HasException(const VAT_TEST_CONTEXT_T* ptContext)`
- **기능**: 예외 상태 확인
- **파라미터**:
  - `ptContext`: 컨텍스트
- **반환값**:
  - `true`: 예외 발생
  - `false`: 정상

#### `uint8_t VAT_Test_GetExceptionStatus(const VAT_TEST_CONTEXT_T* ptContext)`
- **기능**: 예외 상태 바이트 조회
- **파라미터**:
  - `ptContext`: 컨텍스트
- **반환값**: 예외 상태 플래그 (비트마스크)

#### `uint8_t VAT_Test_GetDeviceStatus(const VAT_TEST_CONTEXT_T* ptContext)`
- **기능**: 장치 상태 바이트 조회
- **파라미터**:
  - `ptContext`: 컨텍스트
- **반환값**: 장치 상태 플래그 (비트마스크)

### 📈 통계 함수

#### `void VAT_Test_GetStats(const VAT_TEST_CONTEXT_T* ptContext, VAT_TEST_STATS_T* ptStats)`
- **기능**: 테스트 통계 조회
- **파라미터**:
  - `ptContext`: 컨텍스트
  - `ptStats`: 통계 구조체 (출력)
- **반환값**: 없음

#### `void VAT_Test_ResetStats(VAT_TEST_CONTEXT_T* ptContext)`
- **기능**: 통계 초기화
- **파라미터**:
  - `ptContext`: 컨텍스트
- **반환값**: 없음

#### `void VAT_Test_PrintStats(const VAT_TEST_CONTEXT_T* ptContext)`
- **기능**: 통계 출력
- **파라미터**:
  - `ptContext`: 컨텍스트
- **반환값**: 없음

### 🧪 테스트 시나리오

#### `void VAT_Test_BasicPressureControl(VAT_TEST_CONTEXT_T* ptContext, void* hChannel)`
- **기능**: 기본 압력 제어 테스트
- **동작**: 설정값 500으로 10회 통신
- **사용 Assembly**: Input2 + Output7

#### `void VAT_Test_FullCalibration(VAT_TEST_CONTEXT_T* ptContext, void* hChannel)`
- **기능**: 전체 교정 테스트
- **동작**: 자동 학습 모드 활성화 및 5회 통신
- **사용 Assembly**: Input100 + Output102

---

## 9. 트러블슈팅

### ⚠️ 일반적인 문제

#### 문제 1: 컴파일 에러 - cifX 헤더 파일 없음

**증상**:
```
fatal error: cifXUser.h: No such file or directory
```

**원인**: cifX Toolkit이 설치되지 않았거나 경로가 잘못됨

**해결**:
```bash
# cifX Toolkit 경로 확인
ls ../netx_tk/Includes/cifXUser.h

# 경로 지정하여 빌드
make hardware CIFX_TOOLKIT_PATH=/correct/path/to/netx_tk

# 또는 standalone 모드로 빌드 (하드웨어 불필요)
make standalone
```

#### 문제 2: 실행 시 "Permission denied"

**증상**:
```
./vat_test_hardware: Permission denied
```

**원인**: 실행 권한 없음 또는 cifX 장치 접근 권한 부족

**해결**:
```bash
# 실행 권한 추가
chmod +x vat_test_hardware

# root 권한으로 실행 (cifX 장치 접근)
sudo ./vat_test_hardware
```

#### 문제 3: 통신 타임아웃

**증상**:
```
[VAT Test] Read error: 0x80000001
[VAT Test] Write error: 0x80000001
Timeout Count:           50
```

**원인**:
- DeviceNet 노드 주소 불일치
- 잘못된 Baud Rate
- 케이블 연결 불량
- VAT 장치 전원 꺼짐

**해결**:
1. 노드 주소 확인:
```c
tConfig.node_address = 10;  // VAT 장치 DIP 스위치 설정값과 일치해야 함
```

2. Baud Rate 확인:
```c
tConfig.baud_rate = 250000;  // 250 kbps (일반적)
// 다른 옵션: 125000, 500000
```

3. 물리적 연결 확인:
   - DeviceNet 케이블 연결 상태
   - 종단 저항 (120Ω) 설치 확인
   - LED 상태 확인

#### 문제 4: 예외 상태 발생

**증상**:
```
[VAT Test] Exception detected: 0x01
Exception Count:         25
```

**원인**: VAT 장치에서 예외 발생

**해결**:
```c
// 예외 상태 확인
uint8_t exception = VAT_Test_GetExceptionStatus(&tContext);

if (exception & VAT_EXCEPTION_SETPOINT_OUT_OF_RANGE)
    printf("Setpoint out of range!\n");

if (exception & VAT_EXCEPTION_HARDWARE_FAULT)
    printf("Hardware fault!\n");

if (exception & VAT_EXCEPTION_SENSOR_FAULT)
    printf("Sensor fault!\n");

// 설정값 범위 조정
VAT_Test_SetPressureSetpoint(&tContext, 500);  // 안전한 범위로 설정
```

#### 문제 5: Assembly Instance 불일치

**증상**:
```
[VAT Test] Read error: 0x80000002
```

**원인**: 선택한 Assembly Instance가 VAT 장치에서 지원되지 않음

**해결**:
```c
// EDS 파일에 정의된 Assembly 사용
tConfig.input_assembly = VAT_INPUT_ASSEMBLY_100;   // 0x64
tConfig.output_assembly = VAT_OUTPUT_ASSEMBLY_8;    // 0x08

// 지원되지 않는 Assembly는 사용하지 않음
// 예: VAT_INPUT_ASSEMBLY_1 (0x01)은 일부 펌웨어 버전에서 미지원
```

---

## 10. 고급 사용

### 🔬 커스텀 테스트 시나리오 작성

```c
void Custom_RampTest(VAT_TEST_CONTEXT_T* ptContext, void* hChannel)
{
    printf("\n========== Custom Ramp Test ==========\n");

    // 압력 제어 모드 설정
    VAT_Test_SetControlMode(ptContext, VAT_CONTROL_MODE_PRESSURE);

    // 0에서 1000까지 100 단계로 증가
    for (int16_t setpoint = 0; setpoint <= 1000; setpoint += 100)
    {
        printf("Setting pressure to: %d\n", setpoint);

        // 설정값 설정
        VAT_Test_SetPressureSetpoint(ptContext, setpoint);

        // 각 설정값에서 10회 통신
        for (int i = 0; i < 10; i++)
        {
            VAT_Test_WriteOutput(ptContext, hChannel);
            VAT_Test_ReadInput(ptContext, hChannel);

            // 현재 압력 확인
            if (ptContext->input_data_valid)
            {
                int16_t current_pressure = ptContext->input_asm100.pressure;
                printf("  Current pressure: %d\n", current_pressure);
            }

            usleep(100000);  // 100ms 대기
        }
    }

    printf("======================================\n\n");
}
```

### 📊 실시간 데이터 로깅

```c
void RealTimeLogger(VAT_TEST_CONTEXT_T* ptContext, void* hChannel, int duration_sec)
{
    FILE* fp = fopen("vat_log.csv", "w");
    fprintf(fp, "Timestamp,Setpoint,Pressure,Position,Exception,DeviceStatus\n");

    int cycles = duration_sec * 10;  // 100ms 주기

    for (int i = 0; i < cycles; i++)
    {
        uint32_t timestamp = VAT_GetTimestampMs();

        VAT_Test_WriteOutput(ptContext, hChannel);
        VAT_Test_ReadInput(ptContext, hChannel);

        if (ptContext->input_data_valid)
        {
            fprintf(fp, "%u,%d,%d,%d,0x%02X,0x%02X\n",
                    timestamp,
                    ptContext->output_asm8.control_setpoint,
                    ptContext->input_asm100.pressure,
                    ptContext->input_asm100.position,
                    ptContext->input_asm100.exception_status,
                    ptContext->input_asm100.device_status);
        }

        usleep(100000);
    }

    fclose(fp);
    printf("Log saved to vat_log.csv\n");
}
```

### 🔍 PID 제어 응답 분석

```c
void AnalyzePIDResponse(VAT_TEST_CONTEXT_T* ptContext, void* hChannel)
{
    int16_t target_pressure = 800;
    int samples = 100;
    int16_t pressures[100];

    // 목표 압력 설정
    VAT_Test_SetControlMode(ptContext, VAT_CONTROL_MODE_PRESSURE);
    VAT_Test_SetPressureSetpoint(ptContext, target_pressure);

    // 데이터 수집
    for (int i = 0; i < samples; i++)
    {
        VAT_Test_WriteOutput(ptContext, hChannel);
        VAT_Test_ReadInput(ptContext, hChannel);

        if (ptContext->input_data_valid)
        {
            pressures[i] = ptContext->input_asm100.pressure;
        }

        usleep(100000);
    }

    // 응답 분석
    int16_t max_pressure = pressures[0];
    int16_t min_pressure = pressures[0];
    int16_t final_pressure = pressures[samples - 1];

    for (int i = 1; i < samples; i++)
    {
        if (pressures[i] > max_pressure) max_pressure = pressures[i];
        if (pressures[i] < min_pressure) min_pressure = pressures[i];
    }

    int16_t overshoot = max_pressure - target_pressure;
    int16_t steady_state_error = final_pressure - target_pressure;

    printf("PID Response Analysis:\n");
    printf("  Target Pressure: %d\n", target_pressure);
    printf("  Final Pressure: %d\n", final_pressure);
    printf("  Max Pressure: %d\n", max_pressure);
    printf("  Min Pressure: %d\n", min_pressure);
    printf("  Overshoot: %d (%.1f%%)\n", overshoot,
           (float)overshoot / target_pressure * 100);
    printf("  Steady State Error: %d (%.1f%%)\n", steady_state_error,
           (float)abs(steady_state_error) / target_pressure * 100);
}
```

---

## 11. 통합 가이드

### 🔗 기존 프로젝트에 통합

#### Step 1: 파일 복사

```bash
# 헤더 파일을 프로젝트 Include 디렉토리로 복사
cp test/vat_devicenet_test.h Hil_DemoApp/Includes/

# 소스 파일을 프로젝트 Sources 디렉토리로 복사
cp test/vat_devicenet_test.c Hil_DemoApp/Sources/
```

#### Step 2: 프로젝트에 추가

STM32CubeIDE 또는 빌드 시스템에 파일 추가:
- `vat_devicenet_test.h`
- `vat_devicenet_test.c`

#### Step 3: App_IODataHandler 수정

```c
// App_DemoApplication.c
#include "vat_devicenet_test.h"

// 전역 변수 추가
VAT_TEST_CONTEXT_T g_tVatContext;

void App_IODataHandler(void* ptAppResources)
{
    APP_DATA_T* ptAppData = (APP_DATA_T*)ptAppResources;

    if(ptAppData->aptChannels[0]->hChannel != NULL)
    {
        // VAT 테스트 함수 사용
        VAT_Test_ReadInput(&g_tVatContext, ptAppData->aptChannels[0]->hChannel);

        // 압력 제어
        VAT_Test_SetControlMode(&g_tVatContext, VAT_CONTROL_MODE_PRESSURE);
        VAT_Test_SetPressureSetpoint(&g_tVatContext, 500);

        VAT_Test_WriteOutput(&g_tVatContext, ptAppData->aptChannels[0]->hChannel);
    }
}
```

#### Step 4: 초기화

```c
// App_CifXApplicationDemo() 함수에서
int App_CifXApplicationDemo(char *szDeviceName)
{
    // ... 기존 코드 ...

    // VAT 테스트 초기화
    VAT_Test_Init(&g_tVatContext);

    VAT_TEST_CONFIG_T tConfig = {
        .node_address = 10,
        .baud_rate = 250000,
        .epr_ms = 100,
        .input_assembly = VAT_INPUT_ASSEMBLY_100,
        .output_assembly = VAT_OUTPUT_ASSEMBLY_8,
        .enable_logging = false,  // 임베디드에서는 로깅 비활성화
        .enable_validation = true
    };
    VAT_Test_Configure(&g_tVatContext, &tConfig);

    // ... 나머지 코드 ...
}
```

---

## 12. 성능 최적화

### ⚡ 최적화 팁

#### 1. 로깅 비활성화 (프로덕션)

```c
// 컴파일 시 로깅 비활성화
// Makefile: CFLAGS에서 -DENABLE_LOGGING 제거

// 또는 런타임에 비활성화
tConfig.enable_logging = false;
```

#### 2. 통신 주기 조정

```c
// 높은 응답성 필요 시 (50ms)
tConfig.epr_ms = 50;

// 일반적인 경우 (100ms)
tConfig.epr_ms = 100;

// 낮은 대역폭 환경 (200ms)
tConfig.epr_ms = 200;
```

#### 3. Assembly 크기 최소화

```c
// 필요한 데이터만 포함된 작은 Assembly 사용
// 2바이트 (가장 빠름)
tConfig.input_assembly = VAT_INPUT_ASSEMBLY_1;

// 3바이트
tConfig.input_assembly = VAT_INPUT_ASSEMBLY_2;

// 7바이트 (전체 상태, 권장)
tConfig.input_assembly = VAT_INPUT_ASSEMBLY_100;
```

#### 4. 에러 처리 최적화

```c
// 빠른 에러 확인 (예외만 체크)
if (VAT_Test_HasException(&tContext))
{
    // 예외 처리
}

// 상세 분석 (필요 시에만)
if (tContext.config.enable_validation)
{
    uint8_t exception = VAT_Test_GetExceptionStatus(&tContext);
    // 비트별 분석...
}
```

---

## 13. 요약 및 참고자료

### 📋 요약

| 항목 | 값 |
|------|-----|
| **제품** | VAT Adaptive Pressure Controller |
| **EDS 파일** | 476297.eds |
| **입력 Assembly** | 24개 (2~34 바이트) |
| **출력 Assembly** | 11개 (4~18 바이트) |
| **권장 조합** | Input 100 (7B) + Output 8 (5B) |
| **제어 모드** | 6가지 (CLOSED~THROTTLE) |
| **빌드 모드** | Standalone / Hardware |
| **테스트 시나리오** | 3개 (Basic/Calibration/Monitoring) |

### 🎯 다음 단계

1. **Standalone 테스트**: 하드웨어 없이 로직 검증
2. **Hardware 통합**: 실제 VAT 장치와 통신
3. **커스텀 시나리오**: 애플리케이션별 테스트 추가
4. **프로덕션 통합**: 기존 프로젝트에 통합

### 📚 참고 자료

- **EDS 파일**: `476297.eds` (프로젝트 루트)
- **DeviceNet 규격**: ODVA DeviceNet Specification
- **CIP 규격**: Common Industrial Protocol (ODVA)
- **VAT 매뉴얼**: VAT Adaptive Pressure Controller User Manual
- **cifX API**: Hilscher cifX Toolkit Documentation

### 🔗 추가 문서

- [DeviceNet 통신 기초](./20251027_DNS_V5_EDS_Analysis.md)
- [App_IODataHandler 분석](./20251027_App_IODataHandler_Analysis.md)
- [Live Expression 설정](./20251027_LiveExpression_GlobalVariables.md)

---

**문서 끝**

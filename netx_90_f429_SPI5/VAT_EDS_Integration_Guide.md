# VAT EDS 476297 통합 가이드

## 📋 프로젝트 개요

**프로젝트명**: VAT Adaptive Pressure Controller - DeviceNet Implementation
**EDS 파일**: 476297.eds
**작업 날짜**: 2025-11-05
**펌웨어 플랫폼**: STM32 F429 + netX90 DeviceNet

## ✅ 완료된 작업 (Phase 1-9)

### 생성된 파일 목록 (총 12개)

#### 1. 핵심 데이터 구조 (Phase 1)
- `Hil_DemoApp/Includes/App_VAT_Assemblies.h` (16KB)
  - 24개 Input Assembly 구조체 정의
  - 11개 Output Assembly 구조체 정의
  - Assembly Manager 구조체

- `Hil_DemoApp/Includes/App_VAT_Parameters.h` (5.2KB)
  - 99개 CIP 파라미터 정의
  - 파라미터 관리 구조체
  - Units 및 Data Type 상수

#### 2. Assembly 동적 선택 (Phase 4)
- `Hil_DemoApp/Sources/App_VAT_AssemblySelector.c` (5.5KB)
  - 24개 Input Assembly 크기 맵
  - 11개 Output Assembly 크기 맵
  - 유효성 검증 함수
  - 동적 선택 함수

#### 3. 파라미터 관리 (Phase 5)
- `Hil_DemoApp/Sources/App_VAT_Parameters.c` (6.2KB)
  - 99개 파라미터 초기화
  - Get/Set by ID
  - Get/Set by CIP Path
  - Modified 플래그 관리

#### 4. 데이터 변환 (Phase 6)
- `Hil_DemoApp/Includes/App_VAT_Conversion.h` (1.4KB)
- `Hil_DemoApp/Sources/App_VAT_Conversion.c` (8.3KB)
  - INT ↔ REAL 양방향 변환
  - Pressure Units 변환 (9가지)
  - Position Units 변환 (3가지)

#### 5. Flash 저장/복구 (Phase 7)
- `Hil_DemoApp/Includes/App_VAT_Flash.h` (2.2KB)
- `Hil_DemoApp/Sources/App_VAT_Flash.c` (7.3KB)
  - STM32 F429 Flash Sector 11 사용
  - CRC32 체크섬 검증
  - 파라미터 저장/복구 (3661 bytes)

#### 6. I/O 처리 (Phase 8)
- `Hil_DemoApp/Includes/App_VAT_IoHandler.h`
- `Hil_DemoApp/Sources/App_VAT_IoHandler.c`
  - Input Assembly 업데이트 함수
  - Output Assembly 처리 함수
  - Cyclic/Poll/COS I/O 핸들러

#### 7. CIP Explicit Messaging (Phase 9)
- `Hil_DemoAppDNS/Includes/AppDNS_ExplicitMsg.h`
- `Hil_DemoAppDNS/Sources/AppDNS_ExplicitMsg.c`
  - Get/Set Attribute Single
  - Get Attribute All
  - Save/Reset 서비스

---

## 🔧 통합 방법

### 1. 프로젝트에 파일 추가

**STM32CubeIDE / IAR EWARM / Keil MDK 기준:**

1. 모든 `.h` 파일을 프로젝트 Include Path에 추가
2. 모든 `.c` 파일을 빌드 대상에 추가
3. 컴파일러 Include 경로 설정:
   ```
   Hil_DemoApp/Includes
   Hil_DemoAppDNS/Includes
   ```

### 2. 전역 변수 선언

**App_DemoApplication.c** 또는 메인 파일에 추가:

```c
#include "App_VAT_Assemblies.h"
#include "App_VAT_Parameters.h"

/* Global Assembly Manager */
ASSEMBLY_MANAGER_T g_tAssemblyManager;

/* Global Parameter Manager */
VAT_PARAM_MANAGER_T g_tParamManager;

/* I/O Variables - External references for IoHandler */
int16_t g_current_pressure = 0;
int16_t g_current_position = 0;
uint8_t g_device_status = 2;      /* DEV_STATUS_IDLE */
uint8_t g_access_mode = 0;        /* ACCESS_MODE_LOCAL */
uint8_t g_exception_status = 0;
uint8_t g_discrete_inputs = 0;

int16_t g_control_setpoint = 0;
uint8_t g_control_mode = 0;
uint8_t g_control_instance = 0;
```

### 3. 초기화 코드

**main() 또는 초기화 함수에 추가:**

```c
#include "App_VAT_AssemblySelector.h"
#include "App_VAT_Parameters.h"

void VAT_System_Init(void)
{
    /* Initialize Assembly Manager */
    VAT_Assembly_Init(&g_tAssemblyManager);

    /* Initialize Parameter Manager */
    VAT_Param_Init(&g_tParamManager);

    /* Try to load parameters from Flash */
    if (VAT_Param_LoadFromFlash(&g_tParamManager) != 0) {
        /* Flash empty or corrupted - use defaults */
    }

    /* Set default assemblies */
    g_tAssemblyManager.active_input_instance = 100;   /* Input Assembly 100 */
    g_tAssemblyManager.active_output_instance = 8;    /* Output Assembly 8 */
}
```

### 4. DeviceNet 스택 통합

**AppDNS_DemoApplicationFunctions.c 수정:**

```c
#include "App_VAT_Assemblies.h"
#include "App_VAT_IoHandler.h"
#include "AppDNS_ExplicitMsg.h"

extern ASSEMBLY_MANAGER_T g_tAssemblyManager;

/* Assembly 구성 콜백 */
uint32_t AppDNS_HandleAssemblyConfig(uint16_t usInputInstance, uint16_t usOutputInstance,
                                     uint16_t* pusInputSize, uint16_t* pusOutputSize)
{
    /* Validate and select Input Assembly */
    if (VAT_Assembly_IsValidInput((uint8_t)usInputInstance)) {
        *pusInputSize = VAT_Assembly_GetInputSize((uint8_t)usInputInstance);
        VAT_Assembly_SelectInput(&g_tAssemblyManager, (uint8_t)usInputInstance);
    } else {
        return ERROR_INVALID_PARAMETER;
    }

    /* Validate and select Output Assembly */
    if (VAT_Assembly_IsValidOutput((uint8_t)usOutputInstance)) {
        *pusOutputSize = VAT_Assembly_GetOutputSize((uint8_t)usOutputInstance);
        VAT_Assembly_SelectOutput(&g_tAssemblyManager, (uint8_t)usOutputInstance);
    } else {
        return ERROR_INVALID_PARAMETER;
    }

    return SUCCESS;
}

/* Cyclic I/O 처리 */
void AppDNS_CyclicIoCallback(void)
{
    VAT_HandleCyclicIo(&g_tAssemblyManager);
}
```

### 5. CIP Explicit Messaging 통합

**DeviceNet 스택의 Explicit Message 핸들러에 추가:**

```c
uint32_t AppDNS_HandleExplicitRequest(DNS_EXPLICIT_MSG_T* ptMsg)
{
    uint8_t error_code;
    uint16_t response_size = 0;

    error_code = CIP_HandleExplicitMessage(
        ptMsg->service_code,
        ptMsg->class_id,
        ptMsg->instance_id,
        ptMsg->attribute_id,
        ptMsg->request_data,
        ptMsg->request_size,
        ptMsg->response_data,
        &response_size
    );

    ptMsg->response_size = response_size;
    ptMsg->error_code = error_code;

    return (error_code == 0) ? SUCCESS : ERROR;
}
```

---

## 📊 지원 기능 요약

### Input Assemblies (24개)
| Instance | Size | Data Format | I/O Types |
|----------|------|-------------|-----------|
| 1 | 2 | INT Process Variable | All |
| 2 | 3 | Exception Status, Pressure | All |
| 3 | 5 | Exception Status, Pressure, Position | All |
| ... | ... | ... | ... |
| **100** | **7** | **Exception Status, Pressure, Position, Device Status, Access Mode** | **All** (Default) |
| 111 | 34 | Full data (MAXIMUM SIZE) | No Strobe |

### Output Assemblies (11개)
| Instance | Size | Data Format | I/O Types |
|----------|------|-------------|-----------|
| 7 | 4 | Control Setpoint, Control Instance | No Strobe |
| **8** | **5** | **Control Mode, Control Setpoint, Control Instance** | **No Strobe** (Default) |
| 24 | 7 | Control Mode, FP Control Setpoint, Control Instance | No Strobe |
| 112 | 18 | Complex control (MAXIMUM SIZE) | No Strobe |

### CIP Parameters (99개)
- **Param1**: Vendor ID (404)
- **Param5**: Device Status (R/O, Enum)
- **Param6**: Controller Mode (Enum)
- **Param7**: Access Mode (Enum)
- **Param9**: Pressure Units (9가지)
- **Param10**: Position Units (3가지)
- **Param11/12**: Input/Output Assembly Selection
- ... (총 99개 파라미터)

### Data Types
- **INT (0xC3)**: 16-bit signed integer (-32768 ~ 32767)
- **REAL (0xCA)**: 32-bit IEEE-754 float

### Pressure Units (9가지)
1. Counts (0x1001)
2. Percent (0x1007)
3. psi (0x1300)
4. Torr (0x1301)
5. mTorr (0x1302)
6. Bar (0x1307)
7. mBar (0x1308)
8. Pa (0x1309)
9. atm (0x130B)

### Position Units (3가지)
1. Counts (0x1001)
2. Percent (0x1007)
3. Degrees (0x1703)

### I/O Connection Types
1. **Poll (0x01)**: Master 폴링 응답
2. **Strobe (0x02)**: Strobe 신호 동기화
3. **Change of State (0x04)**: 데이터 변경 시만 전송
4. **Cyclic (0x08)**: 주기적 자동 전송 (권장)

---

## 🧪 테스트 체크리스트

### Assembly 테스트
- [ ] Input Assembly 100 크기 검증 (7 bytes)
- [ ] Output Assembly 8 크기 검증 (5 bytes)
- [ ] Assembly 111 최대 크기 검증 (34 bytes)
- [ ] Assembly 112 최대 크기 검증 (18 bytes)
- [ ] Little-Endian byte ordering 확인
- [ ] Struct packing 검증 (패딩 없음)

### Parameter 테스트
- [ ] Param1 (Vendor ID) 읽기 = 404
- [ ] Param5 (Device Status) Read-Only 확인
- [ ] 범위 검증 (min/max 초과 시 거부)
- [ ] CIP Path 접근 (Class/Instance/Attribute)
- [ ] Modified 플래그 동작 확인

### 변환 테스트
- [ ] INT to REAL 변환 (Pressure: 16383 → 500.0 Torr)
- [ ] REAL to INT 변환 (500.0 Torr → 16383)
- [ ] Units 변환 (1000 mTorr → 1 Torr)
- [ ] Position 변환 (32767 counts → 100%)

### Flash 테스트
- [ ] 파라미터 저장 (VAT_Param_SaveToFlash)
- [ ] 파라미터 복구 (VAT_Param_LoadFromFlash)
- [ ] CRC32 검증
- [ ] Magic Number 검증 (0x56415430)

### I/O 테스트
- [ ] Input Assembly 100 업데이트
- [ ] Output Assembly 8 처리
- [ ] Cyclic I/O 동작 확인
- [ ] Poll I/O 동작 확인

### CIP Explicit Messaging 테스트
- [ ] Get Attribute Single (Param1 읽기)
- [ ] Set Attribute Single (Param6 쓰기)
- [ ] Get Attribute All
- [ ] Save Service (Flash 저장)
- [ ] Reset Service (Flash 복구)

---

## 🔍 디버깅 가이드

### 1. Assembly 크기 불일치
**증상**: CYCON.net에서 "Invalid Assembly Size" 오류

**확인사항**:
```c
uint8_t size = VAT_Assembly_GetInputSize(100);
// Expected: 7 bytes
printf("Input Assembly 100 size: %d\n", size);
```

**해결방법**:
- `sizeof(INPUT_ASSEMBLY_100_T)` 확인
- `__HIL_PACKED_PRE` / `__HIL_PACKED_POST` 매크로 확인
- 컴파일러 struct packing 옵션 확인

### 2. 파라미터 읽기/쓰기 실패
**증상**: Get/Set Attribute 실패

**확인사항**:
```c
uint8_t data[4];
uint8_t size;
int32_t result = VAT_Param_Get(&g_tParamManager, 1, data, &size);
// result should be 0, size should be 2
```

**해결방법**:
- 파라미터 초기화 확인 (VAT_Param_Init 호출)
- CIP Path (Class/Instance/Attribute) 일치 확인
- Read-Only 파라미터 쓰기 시도 확인

### 3. Flash 저장/복구 실패
**증상**: 파라미터가 저장되지 않음

**확인사항**:
```c
int32_t result = VAT_Param_SaveToFlash(&g_tParamManager);
// result should be 0
```

**해결방법**:
- Flash Sector 11 주소 확인 (0x080E0000)
- HAL_FLASH_Unlock/Lock 호출 확인
- Flash 쓰기 권한 확인
- CRC32 계산 확인

### 4. I/O 데이터 업데이트 안됨
**증상**: Input Assembly 데이터가 변하지 않음

**확인사항**:
```c
VAT_HandleCyclicIo(&g_tAssemblyManager);
// Check if g_current_pressure is being updated
```

**해결방법**:
- Cyclic I/O 콜백 등록 확인
- 전역 변수 (g_current_pressure 등) 업데이트 확인
- Active Assembly Instance 확인

---

## 📝 추가 구현 필요 사항

### 1. 나머지 Assembly 핸들러 구현
현재 주요 Assembly만 구현됨:
- Input: 1, 2, 3, 100, 101, 105
- Output: 7, 8, 24, 102

**추가 구현 필요**:
- Input: 4, 5, 6, 10, 11, 17~22, 26, 104, 106, 109, 111, 113, 150
- Output: 23, 32, 103, 107, 108, 110, 112

**구현 위치**: `App_VAT_IoHandler.c`의 switch 문에 추가

### 2. 나머지 파라미터 초기화
현재 Param1, Param5만 초기화됨

**추가 구현 필요**:
- Param2~99 초기화

**구현 위치**: `App_VAT_Parameters.c`의 `VAT_Param_Init()` 함수

### 3. Change of State (COS) 구현
현재 COS는 Cyclic과 동일하게 동작

**추가 구현 필요**:
- 이전 값 저장
- 변경 감지 로직
- 변경 시에만 전송

**구현 위치**: `App_VAT_IoHandler.c`의 `VAT_HandleCosIo()`

---

## 📚 참고 자료

### EDS 파일
- 파일명: `476297.eds`
- Vendor: VAT Vakuumventile AG (404)
- Device Type: Process Control Valve (29)
- Product Code: 650

### DeviceNet 표준
- ODVA DeviceNet Specification Volume I & II
- CIP Common Industrial Protocol Specification

### STM32 참고
- STM32F4xx HAL Driver User Manual
- STM32F429 Reference Manual (RM0090)
- Flash Memory Programming (Section 3)

---

## ✅ 통합 완료 확인

### Phase 1-7: 핵심 파일 생성 ✅
- Assembly 구조체 (8개 파일)
- Parameter 관리 (2개 파일)
- Conversion (2개 파일)
- Flash 저장/복구 (2개 파일)

### Phase 8-9: DeviceNet 통합 ✅
- I/O Handler (2개 파일)
- CIP Explicit Messaging (2개 파일)

### Phase 10: 문서화 ✅
- 통합 가이드 (본 문서)
- 테스트 체크리스트
- 디버깅 가이드

---

## 🎯 다음 단계

1. **빌드 및 컴파일**: 모든 파일을 프로젝트에 추가하고 컴파일 오류 해결
2. **초기화 코드 통합**: main() 함수에 초기화 코드 추가
3. **DeviceNet 스택 연동**: 콜백 함수 등록 및 테스트
4. **CYCON.net 테스트**: 실제 DeviceNet 마스터와 연결 테스트
5. **파라미터 저장/복구 검증**: Flash 동작 확인
6. **단위 테스트**: 각 모듈별 기능 검증
7. **통합 테스트**: 전체 시스템 동작 확인

---

**작업 완료일**: 2025-11-05
**버전**: 1.0
**작성자**: Claude Code Assistant

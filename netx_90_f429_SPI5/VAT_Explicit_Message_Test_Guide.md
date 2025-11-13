# VAT CIP Explicit Message 테스트 가이드

## 📋 개요

CIP Explicit Messaging은 DeviceNet 마스터가 슬레이브 디바이스의 파라미터를 읽고 쓰는 데 사용하는 프로토콜입니다.

---

## 🛠️ 테스트 환경 준비

### 필요한 장비/소프트웨어
1. **DeviceNet 마스터**:
   - CYCON.net (Hilscher 권장)
   - RSNetWorx for DeviceNet (Rockwell)
   - 기타 DeviceNet 마스터

2. **하드웨어**:
   - STM32 F429 + netX90 보드
   - DeviceNet 케이블
   - 24V 전원

3. **EDS 파일**:
   - `476297.eds` (VAT Adaptive Pressure Controller)

---

## 🔧 1. CYCON.net을 이용한 테스트

### 1.1 초기 설정

1. **CYCON.net 실행**
2. **새 프로젝트 생성**
   - File → New Project
   - DeviceNet 네트워크 선택

3. **VAT 디바이스 추가**
   - EDS 파일 import: `476297.eds`
   - 디바이스를 네트워크에 추가
   - Node Address 설정 (기본값: 1)

4. **네트워크 다운로드**
   - Download configuration to Master

### 1.2 Parameter 읽기 테스트 (Get Attribute Single)

#### Param1: Vendor ID 읽기
```
CIP Path: Class 0x01, Instance 0x01, Attribute 0x01
Expected Value: 404 (0x0194)
Data Type: UINT (2 bytes)
```

**CYCON.net 조작**:
1. Device View → Parameters 탭
2. "Param1: Vendor ID" 선택
3. "Read" 버튼 클릭
4. **예상 결과**: 404 표시

**원시 메시지 확인** (Tools → Message Log):
```
Request:  [Service: 0x0E, Class: 0x01, Instance: 0x01, Attribute: 0x01]
Response: [Status: 0x00, Data: 0x94 0x01]  // 404 in little-endian
```

#### Param5: Device Status 읽기
```
CIP Path: Class 0x30, Instance 0x01, Attribute 0x0B
Expected Value: 2 (DEV_STATUS_IDLE)
Data Type: USINT (1 byte)
```

**CYCON.net 조작**:
1. "Param5: Device Status" 선택
2. "Read" 버튼 클릭
3. **예상 결과**: "Idle" 또는 2 표시

### 1.3 Parameter 쓰기 테스트 (Set Attribute Single)

#### Param6: Controller Mode 쓰기
```
CIP Path: Class 0x30, Instance 0x01, Attribute 0x0C
Test Value: 5 (CTRL_MODE_PRESSURE)
Data Type: USINT (1 byte)
```

**CYCON.net 조작**:
1. "Param6: Controller Mode" 선택
2. 값 입력: 5 (Pressure Control)
3. "Write" 버튼 클릭
4. **예상 결과**: Write Success

**검증**:
1. 다시 "Read" 버튼 클릭
2. 값이 5로 변경되었는지 확인

**원시 메시지 확인**:
```
Request:  [Service: 0x10, Class: 0x30, Instance: 0x01, Attribute: 0x0C, Data: 0x05]
Response: [Status: 0x00]  // Success
```

#### Read-Only Parameter 쓰기 테스트 (에러 확인)
```
Parameter: Param1 (Vendor ID) - Read Only
Expected Error: 0x0E (Attribute Not Settable)
```

**CYCON.net 조작**:
1. "Param1: Vendor ID" 선택
2. 값 변경 시도
3. **예상 결과**: Error 0x0E (Attribute Not Settable)

### 1.4 Get Attribute All 테스트

**CYCON.net 조작**:
1. Device View → "Get All Attributes" 메뉴
2. Class/Instance 선택: Class 0x30, Instance 0x01
3. **예상 결과**: 해당 인스턴스의 모든 Attribute 데이터 반환

### 1.5 Save/Reset 서비스 테스트

#### Save to Flash 테스트
```
Service Code: 0x16 (Save)
```

**CYCON.net 조작**:
1. Parameter 값 변경 (예: Param6 = 5)
2. "Save to Device" 버튼 클릭
3. 전원 OFF/ON
4. 다시 읽기 → 저장된 값 확인

**원시 메시지**:
```
Request:  [Service: 0x16]
Response: [Status: 0x00]  // Success
```

#### Reset (Load from Flash) 테스트
```
Service Code: 0x05 (Reset)
```

**CYCON.net 조작**:
1. Parameter 값 변경 (저장하지 않음)
2. "Reset" 버튼 클릭
3. **예상 결과**: Flash에서 저장된 값으로 복구

---

## 🔍 2. 디버그 모니터링

### 2.1 printf 디버그 추가

**AppDNS_ExplicitMsg.c 수정**:

```c
#include <stdio.h>

uint8_t CIP_HandleGetAttributeSingle(uint8_t class_id, uint8_t instance_id, uint8_t attribute_id,
                                      uint8_t* pResponse, uint8_t* pResponseSize)
{
    printf("Get Attr: Class=0x%02X, Inst=0x%02X, Attr=0x%02X\n",
           class_id, instance_id, attribute_id);

    VAT_PARAMETER_T* pParam = VAT_Param_FindByPath(class_id, instance_id, attribute_id);

    if (!pParam) {
        printf("  -> Not Found!\n");
        return CIP_ERROR_INVALID_ATTRIBUTE;
    }

    memcpy(pResponse, pParam->data, pParam->data_size);
    *pResponseSize = pParam->data_size;

    printf("  -> Success, Size=%d\n", pParam->data_size);
    return CIP_ERROR_SUCCESS;
}

uint8_t CIP_HandleSetAttributeSingle(uint8_t class_id, uint8_t instance_id, uint8_t attribute_id,
                                      const uint8_t* pData, uint8_t dataSize)
{
    printf("Set Attr: Class=0x%02X, Inst=0x%02X, Attr=0x%02X, Size=%d\n",
           class_id, instance_id, attribute_id, dataSize);

    VAT_PARAMETER_T* pParam = VAT_Param_FindByPath(class_id, instance_id, attribute_id);

    if (!pParam) {
        printf("  -> Not Found!\n");
        return CIP_ERROR_INVALID_ATTRIBUTE;
    }

    if (pParam->descriptor & PARAM_DESC_READ_ONLY) {
        printf("  -> Read Only!\n");
        return CIP_ERROR_ATTRIBUTE_NOT_SETTABLE;
    }

    memcpy(pParam->data, pData, dataSize);
    VAT_Param_SetModified(&g_tParamManager, pParam->param_id);

    printf("  -> Success\n");
    return CIP_ERROR_SUCCESS;
}
```

### 2.2 예상 디버그 출력

**Param1 읽기 시**:
```
Get Attr: Class=0x01, Inst=0x01, Attr=0x01
  -> Success, Size=2
```

**Param6 쓰기 시**:
```
Set Attr: Class=0x30, Inst=0x01, Attr=0x0C, Size=1
  -> Success
```

**Read-Only 쓰기 시도**:
```
Set Attr: Class=0x01, Inst=0x01, Attr=0x01, Size=2
  -> Read Only!
```

---

## 🧪 3. 단위 테스트 코드

### 3.1 테스트 함수 작성

**test_explicit_msg.c** (새 파일):

```c
#include "App_VAT_Parameters.h"
#include "AppDNS_ExplicitMsg.h"
#include <stdio.h>
#include <string.h>

extern VAT_PARAM_MANAGER_T g_tParamManager;

void Test_GetAttributeSingle(void)
{
    uint8_t response[32];
    uint8_t size;
    uint8_t error;

    printf("\n=== Test Get Attribute Single ===\n");

    // Test 1: Vendor ID (Param1)
    printf("Test 1: Get Vendor ID (Param1)\n");
    error = CIP_HandleGetAttributeSingle(0x01, 0x01, 0x01, response, &size);

    if (error == CIP_ERROR_SUCCESS && size == 2) {
        uint16_t vendor_id = *(uint16_t*)response;
        printf("  Result: SUCCESS, Vendor ID = %d\n", vendor_id);
        printf("  Expected: 404, Actual: %d, %s\n",
               vendor_id, (vendor_id == 404) ? "PASS" : "FAIL");
    } else {
        printf("  Result: FAIL, Error = 0x%02X\n", error);
    }

    // Test 2: Device Status (Param5)
    printf("\nTest 2: Get Device Status (Param5)\n");
    error = CIP_HandleGetAttributeSingle(0x30, 0x01, 0x0B, response, &size);

    if (error == CIP_ERROR_SUCCESS && size == 1) {
        uint8_t status = response[0];
        printf("  Result: SUCCESS, Device Status = %d\n", status);
        printf("  Expected: 2 (Idle), Actual: %d, %s\n",
               status, (status == 2) ? "PASS" : "FAIL");
    } else {
        printf("  Result: FAIL, Error = 0x%02X\n", error);
    }

    // Test 3: Invalid Parameter
    printf("\nTest 3: Get Invalid Parameter\n");
    error = CIP_HandleGetAttributeSingle(0xFF, 0xFF, 0xFF, response, &size);
    printf("  Result: Error = 0x%02X\n", error);
    printf("  Expected: 0x09 (Invalid Attribute), Actual: 0x%02X, %s\n",
           error, (error == CIP_ERROR_INVALID_ATTRIBUTE) ? "PASS" : "FAIL");
}

void Test_SetAttributeSingle(void)
{
    uint8_t error;
    uint8_t response[32];
    uint8_t size;

    printf("\n=== Test Set Attribute Single ===\n");

    // Test 1: Set Controller Mode (Param6)
    printf("Test 1: Set Controller Mode to 5 (Pressure)\n");
    uint8_t mode = 5;
    error = CIP_HandleSetAttributeSingle(0x30, 0x01, 0x0C, &mode, 1);

    if (error == CIP_ERROR_SUCCESS) {
        printf("  Write: SUCCESS\n");

        // Read back to verify
        error = CIP_HandleGetAttributeSingle(0x30, 0x01, 0x0C, response, &size);
        uint8_t read_mode = response[0];
        printf("  Read back: %d, %s\n",
               read_mode, (read_mode == 5) ? "PASS" : "FAIL");
    } else {
        printf("  Write: FAIL, Error = 0x%02X\n", error);
    }

    // Test 2: Try to write Read-Only parameter (Vendor ID)
    printf("\nTest 2: Try to write Read-Only parameter (Vendor ID)\n");
    uint16_t vendor = 999;
    error = CIP_HandleSetAttributeSingle(0x01, 0x01, 0x01, (uint8_t*)&vendor, 2);

    printf("  Result: Error = 0x%02X\n", error);
    printf("  Expected: 0x0E (Attribute Not Settable), Actual: 0x%02X, %s\n",
           error, (error == CIP_ERROR_ATTRIBUTE_NOT_SETTABLE) ? "PASS" : "FAIL");
}

void Test_SaveLoad(void)
{
    printf("\n=== Test Save/Load ===\n");

    // Test 1: Save
    printf("Test 1: Save to Flash\n");
    uint8_t error = CIP_HandleSave();
    printf("  Result: %s\n", (error == CIP_ERROR_SUCCESS) ? "PASS" : "FAIL");

    // Test 2: Modify parameter
    printf("\nTest 2: Modify parameter\n");
    uint8_t mode = 3;
    CIP_HandleSetAttributeSingle(0x30, 0x01, 0x0C, &mode, 1);

    uint8_t response[32];
    uint8_t size;
    CIP_HandleGetAttributeSingle(0x30, 0x01, 0x0C, response, &size);
    printf("  Modified value: %d\n", response[0]);

    // Test 3: Reset (Load from Flash)
    printf("\nTest 3: Reset (Load from Flash)\n");
    error = CIP_HandleReset();
    printf("  Reset result: %s\n", (error == CIP_ERROR_SUCCESS) ? "PASS" : "FAIL");

    // Verify restored value
    CIP_HandleGetAttributeSingle(0x30, 0x01, 0x0C, response, &size);
    printf("  Restored value: %d (should be 5)\n", response[0]);
}

void Run_All_Explicit_Tests(void)
{
    printf("\n");
    printf("========================================\n");
    printf(" VAT CIP Explicit Message Tests\n");
    printf("========================================\n");

    Test_GetAttributeSingle();
    Test_SetAttributeSingle();
    Test_SaveLoad();

    printf("\n========================================\n");
    printf(" Tests Complete\n");
    printf("========================================\n\n");
}
```

### 3.2 main()에서 호출

```c
int main(void)
{
    /* System initialization */
    HAL_Init();
    SystemClock_Config();

    /* VAT System initialization */
    VAT_Assembly_Init(&g_tAssemblyManager);
    VAT_Param_Init(&g_tParamManager);

    /* Run unit tests */
    #ifdef RUN_UNIT_TESTS
    Run_All_Explicit_Tests();
    #endif

    /* Main loop */
    while(1) {
        // ...
    }
}
```

---

## 📊 4. 테스트 체크리스트

### 기본 테스트
- [ ] **Get Attribute Single**
  - [ ] Param1 (Vendor ID) 읽기 → 404
  - [ ] Param5 (Device Status) 읽기 → 2
  - [ ] 존재하지 않는 파라미터 → Error 0x09

- [ ] **Set Attribute Single**
  - [ ] Param6 (Controller Mode) 쓰기 → Success
  - [ ] Read-Only 파라미터 쓰기 → Error 0x0E
  - [ ] 잘못된 크기로 쓰기 → Error 0x15
  - [ ] 범위 밖 값 쓰기 → Error 0x15

- [ ] **Get Attribute All**
  - [ ] Class 0x30, Instance 0x01 전체 읽기
  - [ ] 여러 Attribute 데이터 확인

### 고급 테스트
- [ ] **Save Service**
  - [ ] 파라미터 변경 후 Save
  - [ ] 전원 재시작 후 값 유지 확인
  - [ ] CRC32 검증

- [ ] **Reset Service**
  - [ ] 파라미터 변경 (저장 안함)
  - [ ] Reset 실행
  - [ ] Flash 값으로 복구 확인

- [ ] **성능 테스트**
  - [ ] 연속 100회 읽기/쓰기
  - [ ] 응답 시간 측정 (< 50μs)

---

## 🐛 5. 문제 해결

### 5.1 "Invalid Attribute" 오류

**증상**: Get/Set Attribute 시 Error 0x09

**원인**:
1. CIP Path 불일치 (Class/Instance/Attribute)
2. 파라미터 초기화 안됨
3. 잘못된 파라미터 매핑

**해결**:
```c
// 디버그 출력 추가
printf("Looking for: Class=0x%02X, Inst=0x%02X, Attr=0x%02X\n",
       class_id, instance_id, attribute_id);

for (uint8_t i = 0; i < 99; i++) {
    VAT_PARAMETER_T* p = &g_tParamManager.params[i];
    if (p->class_id != 0) {  // Initialized
        printf("  Param%d: Class=0x%02X, Inst=0x%02X, Attr=0x%02X\n",
               i+1, p->class_id, p->instance_id, p->attribute_id);
    }
}
```

### 5.2 "Attribute Not Settable" 오류

**증상**: Set Attribute 시 Error 0x0E

**원인**: Read-Only 파라미터 쓰기 시도

**확인**:
```c
printf("Param descriptor: 0x%04X\n", pParam->descriptor);
printf("Is Read-Only: %d\n", !!(pParam->descriptor & PARAM_DESC_READ_ONLY));
```

### 5.3 Flash Save 실패

**증상**: Save 후 데이터가 저장되지 않음

**원인**:
1. Flash unlock 실패
2. Sector erase 실패
3. Write 권한 없음

**확인**:
```c
int32_t result = VAT_Param_SaveToFlash(&g_tParamManager);
if (result != 0) {
    printf("Save failed: %d\n", result);
    // -1: Erase failed
    // -2: Write failed
}
```

---

## 📈 6. 예상 결과

### 정상 동작 시
```
========================================
 VAT CIP Explicit Message Tests
========================================

=== Test Get Attribute Single ===
Test 1: Get Vendor ID (Param1)
  Result: SUCCESS, Vendor ID = 404
  Expected: 404, Actual: 404, PASS

Test 2: Get Device Status (Param5)
  Result: SUCCESS, Device Status = 2
  Expected: 2 (Idle), Actual: 2, PASS

Test 3: Get Invalid Parameter
  Result: Error = 0x09
  Expected: 0x09 (Invalid Attribute), Actual: 0x09, PASS

=== Test Set Attribute Single ===
Test 1: Set Controller Mode to 5 (Pressure)
  Write: SUCCESS
  Read back: 5, PASS

Test 2: Try to write Read-Only parameter (Vendor ID)
  Result: Error = 0x0E
  Expected: 0x0E (Attribute Not Settable), Actual: 0x0E, PASS

=== Test Save/Load ===
Test 1: Save to Flash
  Result: PASS

Test 2: Modify parameter
  Modified value: 3

Test 3: Reset (Load from Flash)
  Reset result: PASS
  Restored value: 5 (should be 5)

========================================
 Tests Complete
========================================
```

---

## 🎯 요약

### 테스트 순서
1. **CYCON.net 기본 테스트** (30분)
   - EDS 로드 및 디바이스 인식
   - Parameter 읽기/쓰기

2. **단위 테스트 실행** (1시간)
   - 테스트 코드 작성 및 실행
   - 모든 CIP 서비스 검증

3. **통합 테스트** (2시간)
   - 실제 DeviceNet 네트워크에서 테스트
   - 장시간 안정성 테스트

4. **성능 측정** (1시간)
   - 응답 시간 측정
   - 연속 동작 테스트

**총 예상 시간**: 4-5시간

---

**작성일**: 2025-11-05
**버전**: 1.0

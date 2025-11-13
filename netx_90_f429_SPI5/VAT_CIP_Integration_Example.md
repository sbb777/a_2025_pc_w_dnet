# VAT CIP Explicit Message 통합 예제

## 📋 개요

netHost에서 CIP Explicit Message를 수신하고 응답을 보내는 **DeviceNet 스택 통합 예제**입니다.

---

## 🔧 1. AppDNS_DemoApplicationFunctions.c 수정

### 1.1 헤더 추가

```c
#include "AppDNS_ExplicitMsg.h"
#include <stdio.h>
```

### 1.2 Explicit Message 핸들러 구현

DeviceNet 스택이 Explicit Message를 수신하면 호출하는 콜백 함수입니다.

```c
/******************************************************************************
 * Explicit Message Handler (DeviceNet Stack Callback)
 ******************************************************************************/

void AppDNS_HandleExplicitMessage(RCX_PACKET* pRequest, RCX_PACKET* pResponse)
{
    /* CIP 메시지 데이터 포인터 */
    const uint8_t* pReqData = pRequest->abData;
    uint32_t reqLen = pRequest->tHeader.ulLen;

    uint8_t* pRespData = pResponse->abData;
    uint32_t respLen = 0;

    /* 디버그 출력 */
    printf("\n=== Explicit Message Received ===\n");
    printf("Request Length: %d bytes\n", reqLen);
    printf("Request Data: ");
    for (uint32_t i = 0; i < reqLen; i++) {
        printf("%02X ", pReqData[i]);
    }
    printf("\n");

    /* CIP 메시지 처리 */
    uint8_t error = CIP_ProcessExplicitMessage(pReqData, reqLen, pRespData, &respLen);

    /* 응답 길이 설정 */
    pResponse->tHeader.ulLen = respLen;

    /* 디버그 출력 */
    printf("Response Length: %d bytes\n", respLen);
    printf("Response Data: ");
    for (uint32_t i = 0; i < respLen; i++) {
        printf("%02X ", pRespData[i]);
    }
    printf("\n");
    printf("CIP Error Code: 0x%02X\n", error);
    printf("===================================\n\n");
}
```

---

## 📊 2. 예상 동작 시나리오

### 시나리오 1: Get Attribute Single (Param1 읽기)

#### netHost → Slave
```
Request Length: 8 bytes
Request Data: 0E 03 20 01 24 01 30 01
```

#### 처리 과정
```
Service: 0x0E (Get Attribute Single)
Path Size: 3 words
Class: 0x01, Instance: 0x01, Attribute: 0x01
→ CIP_HandleGetAttributeSingle(0x01, 0x01, 0x01, ...)
→ Found: Param1 (Vendor ID) = 404
```

#### Slave → netHost
```
Response Length: 6 bytes
Response Data: 8E 00 00 00 94 01
CIP Error Code: 0x00 (Success)
```

**응답 해석**:
- `8E`: Reply Service (0x0E | 0x80)
- `00`: Reserved
- `00`: Success
- `00`: Additional Status Size
- `94 01`: Vendor ID = 404 (Little-endian)

---

### 시나리오 2: Set Attribute Single (Param6 쓰기)

#### netHost → Slave
```
Request Length: 9 bytes
Request Data: 10 03 20 30 24 01 30 0C 05
```

#### 처리 과정
```
Service: 0x10 (Set Attribute Single)
Path Size: 3 words
Class: 0x30, Instance: 0x01, Attribute: 0x0C
Data: 05 (Controller Mode = 5)
→ CIP_HandleSetAttributeSingle(0x30, 0x01, 0x0C, [05], 1)
→ Param6 found, writable, range OK
→ Value set to 5, marked as modified
```

#### Slave → netHost
```
Response Length: 4 bytes
Response Data: 90 00 00 00
CIP Error Code: 0x00 (Success)
```

**응답 해석**:
- `90`: Reply Service (0x10 | 0x80)
- `00`: Reserved
- `00`: Success
- `00`: Additional Status Size

---

### 시나리오 3: Set Read-Only Parameter (에러)

#### netHost → Slave
```
Request Length: 10 bytes
Request Data: 10 03 20 01 24 01 30 01 99 99
```

#### 처리 과정
```
Service: 0x10 (Set Attribute Single)
Path Size: 3 words
Class: 0x01, Instance: 0x01, Attribute: 0x01
Data: 99 99
→ CIP_HandleSetAttributeSingle(0x01, 0x01, 0x01, [99 99], 2)
→ Param1 found, but READ-ONLY!
→ Return CIP_ERROR_ATTRIBUTE_NOT_SETTABLE (0x0E)
```

#### Slave → netHost
```
Response Length: 4 bytes
Response Data: 90 00 0E 00
CIP Error Code: 0x0E (Attribute Not Settable)
```

**응답 해석**:
- `90`: Reply Service
- `00`: Reserved
- `0E`: **Error: Attribute Not Settable**
- `00`: Additional Status Size

---

## 🧪 3. 실제 테스트 절차

### 단계 1: 컴파일 및 다운로드
1. 프로젝트 빌드
2. netX90 보드에 다운로드
3. UART 연결 (Printf 출력 확인용)

### 단계 2: netHost 테스트

#### Test 1: Param1 읽기
**netHost 전송**:
```
Cmd: 0x00008020 (RCX_SEND_PACKET_REQ)
Len: 0x00000008
Data: 0E 03 20 01 24 01 30 01
```

**예상 UART 출력**:
```
=== Explicit Message Received ===
Request Length: 8 bytes
Request Data: 0E 03 20 01 24 01 30 01
Response Length: 6 bytes
Response Data: 8E 00 00 00 94 01
CIP Error Code: 0x00
===================================
```

**예상 netHost 수신**:
```
Len: 0x00000006
Data: 8E 00 00 00 94 01
```

#### Test 2: Param6 쓰기
**netHost 전송**:
```
Cmd: 0x00008020
Len: 0x00000009
Data: 10 03 20 30 24 01 30 0C 05
```

**예상 UART 출력**:
```
=== Explicit Message Received ===
Request Length: 9 bytes
Request Data: 10 03 20 30 24 01 30 0C 05
Response Length: 4 bytes
Response Data: 90 00 00 00
CIP Error Code: 0x00
===================================
```

**예상 netHost 수신**:
```
Len: 0x00000004
Data: 90 00 00 00
```

#### Test 3: Param6 읽기 (검증)
**netHost 전송**:
```
Cmd: 0x00008020
Len: 0x00000008
Data: 0E 03 20 30 24 01 30 0C
```

**예상 netHost 수신**:
```
Len: 0x00000005
Data: 8E 00 00 00 05
```
→ 값이 5로 변경되었음을 확인!

---

## 🔍 4. 문제 해결

### 문제 1: 응답이 없음 (Len=0)

**원인**:
- `AppDNS_HandleExplicitMessage()` 함수가 호출되지 않음
- DeviceNet 스택에 콜백이 등록되지 않음

**해결**:
```c
/* DeviceNet 스택 초기화 시 콜백 등록 */
DNS_SetExplicitMessageCallback(AppDNS_HandleExplicitMessage);
```

### 문제 2: 잘못된 응답

**원인**:
- `pResponse->tHeader.ulLen`이 설정되지 않음

**해결**:
```c
/* 반드시 응답 길이 설정 */
pResponse->tHeader.ulLen = respLen;
```

### 문제 3: 파라미터를 찾을 수 없음

**원인**:
- 파라미터가 초기화되지 않음
- CIP Path가 일치하지 않음

**해결**:
```c
/* App_VAT_Parameters.c에서 파라미터 초기화 확인 */
VAT_Param_Init(&g_tParamManager);

/* CIP Path 확인 (Printf 디버그) */
printf("Looking for: Class=0x%02X, Inst=0x%02X, Attr=0x%02X\n",
       class_id, instance_id, attribute_id);
```

---

## 📊 5. 전체 테스트 데이터표

| No | 테스트 | 전송 Hex | 예상 수신 Hex | 수신 Len | 의미 |
|----|--------|----------|---------------|----------|------|
| 1 | Get Param1 (Vendor ID) | `0E 03 20 01 24 01 30 01` | `8E 00 00 00 94 01` | 6 | 404 |
| 2 | Get Param5 (Device Status) | `0E 03 20 30 24 01 30 0B` | `8E 00 00 00 02` | 5 | 2 (Idle) |
| 3 | Set Param6 (Mode=5) | `10 03 20 30 24 01 30 0C 05` | `90 00 00 00` | 4 | Success |
| 4 | Get Param6 (검증) | `0E 03 20 30 24 01 30 0C` | `8E 00 00 00 05` | 5 | 5 |
| 5 | Set Param1 (Read-Only) | `10 03 20 01 24 01 30 01 99 99` | `90 00 0E 00` | 4 | Error 0x0E |
| 6 | Get Invalid Param | `0E 03 20 FF 24 FF 30 FF` | `8E 00 09 00` | 4 | Error 0x09 |

---

## ✅ 체크리스트

### 코드 구현
- [x] `CIP_ProcessExplicitMessage()` 구현 완료
- [x] `AppDNS_HandleExplicitMessage()` 구현 완료
- [ ] DeviceNet 스택에 콜백 등록
- [x] Param1, Param5, Param6 초기화 완료

### 테스트 준비
- [ ] 펌웨어 빌드 성공
- [ ] netX90 보드 다운로드
- [ ] UART 연결 (Printf 확인)
- [ ] netHost 실행 및 연결

### 실제 테스트
- [ ] Test 1: Get Param1 → 응답 6바이트
- [ ] Test 2: Get Param5 → 응답 5바이트
- [ ] Test 3: Set Param6 → 응답 4바이트
- [ ] Test 4: Get Param6 → 응답 5바이트 (값=5)
- [ ] Test 5: Set Param1 → 에러 0x0E
- [ ] Test 6: Printf 출력 확인

---

## 🎯 요약

### 핵심 함수
```
netHost → RCX_PACKET
    ↓
AppDNS_HandleExplicitMessage()  (콜백)
    ↓
CIP_ProcessExplicitMessage()    (파싱 + 응답 생성)
    ↓
CIP_HandleGetAttributeSingle()
CIP_HandleSetAttributeSingle()
    ↓
VAT_Param_Get() / VAT_Param_Set()
    ↓
g_tParamManager.params[]
```

### 성공 조건
1. ✅ `CIP_ProcessExplicitMessage()` 구현
2. ✅ `AppDNS_HandleExplicitMessage()` 구현
3. ✅ 파라미터 초기화 (Param1, 5, 6)
4. ⏳ DeviceNet 스택 콜백 등록
5. ⏳ netHost 테스트 및 검증

---

**작성일**: 2025-11-05
**버전**: 1.0

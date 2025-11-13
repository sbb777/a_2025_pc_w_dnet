# VAT CIP Message 패킷 분석 및 테스트 데이터

## 📊 현재 문제 분석

### 전송된 패킷 (Master → Slave)
```
Cmd=0x00008020      # RCX_SEND_PACKET_REQ
Len=0x00000009      # 9 bytes
Data=10 03 20 30 24 01 30 0C 05
```

### CIP 메시지 상세 분석
```
Byte  Value  의미
----  -----  --------------------------------------------------
0     0x10   Service Code: Set Attribute Single
1     0x03   Path Size: 3 words (6 bytes)
2     0x20   Logical Class ID (8-bit)
3     0x30   Class = 0x30
4     0x24   Logical Instance ID (8-bit)
5     0x01   Instance = 0x01
6     0x30   Logical Attribute ID (8-bit)
7     0x0C   Attribute = 0x0C
8     0x05   Data: Value = 5 (Controller Mode = Pressure Control)
```

**요청 내용**: Param6 (Controller Mode)를 5 (Pressure Control)로 설정

---

## 🔴 문제점

### 수신 패킷 (Slave → Master)
```
Len=0x00000000      # 0 bytes - 응답 데이터 없음!
```

### ❌ 정상 응답이 아님

**정상 응답이라면**:
```
Len=0x00000004
Data=90 00 00 00
```
- `0x90`: Reply Service (0x10 | 0x80)
- `0x00`: Reserved
- `0x00`: General Status (Success)
- `0x00`: Additional Status Size

---

## 🔍 원인 분석

### 1. CIP 핸들러가 호출되지 않음
```c
// AppDNS_DemoApplicationFunctions.c 확인 필요
void AppDNS_HandleExplicitMessage(RCX_PACKET* pRequest, RCX_PACKET* pResponse)
{
    // 이 함수가 호출되는가?
    printf("Explicit Message Received!\n");

    // CIP_HandleSetAttributeSingle()이 호출되는가?
}
```

### 2. 응답 메시지를 생성하지 않음
```c
// AppDNS_ExplicitMsg.c에서 응답 생성 필요
uint8_t CIP_HandleSetAttributeSingle(...)
{
    // ...
    return CIP_ERROR_SUCCESS;  // 이것만으로는 부족!

    // DeviceNet 스택에 응답을 보내야 함
}
```

### 3. Param6이 초기화되지 않았을 가능성
```c
// App_VAT_Parameters.c 확인
// Param6 (index 5)가 올바르게 초기화되었는가?
```

### 4. DeviceNet 스택 통합 누락
- Explicit Message 콜백 등록 안됨
- 응답 패킷 전송 코드 누락
- CIP 라우팅 설정 오류

---

## ✅ 테스트 가능한 데이터 (단계별)

### 단계 1: Get Attribute Single (읽기부터 시작)

읽기가 먼저 성공해야 쓰기를 테스트할 수 있습니다.

#### 1-1. Param1 읽기 (Vendor ID)

**전송 데이터**:
```
Cmd=0x00008020
Len=0x00000008
Data=0E 03 20 01 24 01 30 01
```

**상세 분석**:
```
Byte  Value  의미
----  -----  --------------------------------------------------
0     0x0E   Service Code: Get Attribute Single
1     0x03   Path Size: 3 words
2     0x20   Logical Class ID
3     0x01   Class = 0x01 (Identity)
4     0x24   Logical Instance ID
5     0x01   Instance = 0x01
6     0x30   Logical Attribute ID
7     0x01   Attribute = 0x01 (Vendor ID)
```

**예상 응답**:
```
Len=0x00000006
Data=8E 00 00 00 94 01
```

**응답 상세**:
```
Byte  Value  의미
----  -----  --------------------------------------------------
0     0x8E   Reply Service (0x0E | 0x80)
1     0x00   Reserved
2     0x00   General Status: Success
3     0x00   Additional Status Size
4     0x94   Data: Vendor ID Low = 0x94
5     0x01   Data: Vendor ID High = 0x01
→ Vendor ID = 0x0194 = 404
```

#### 1-2. Param5 읽기 (Device Status)

**전송 데이터**:
```
Cmd=0x00008020
Len=0x00000008
Data=0E 03 20 30 24 01 30 0B
```

**상세 분석**:
```
Byte  Value  의미
----  -----  --------------------------------------------------
0     0x0E   Service Code: Get Attribute Single
1     0x03   Path Size: 3 words
2     0x20   Logical Class ID
3     0x30   Class = 0x30 (VAT Object)
4     0x24   Logical Instance ID
5     0x01   Instance = 0x01
6     0x30   Logical Attribute ID
7     0x0B   Attribute = 0x0B (Device Status)
```

**예상 응답**:
```
Len=0x00000005
Data=8E 00 00 00 02
```

**응답 상세**:
```
Byte  Value  의미
----  -----  --------------------------------------------------
0     0x8E   Reply Service (0x0E | 0x80)
1     0x00   Reserved
2     0x00   General Status: Success
3     0x00   Additional Status Size
4     0x02   Data: Device Status = 2 (Idle)
```

---

### 단계 2: Set Attribute Single (쓰기 테스트)

읽기가 성공한 후에 쓰기를 테스트합니다.

#### 2-1. Param6 쓰기 (Controller Mode = 5)

**전송 데이터** (현재 전송한 것):
```
Cmd=0x00008020
Len=0x00000009
Data=10 03 20 30 24 01 30 0C 05
```

**예상 응답**:
```
Len=0x00000004
Data=90 00 00 00
```

**응답 상세**:
```
Byte  Value  의미
----  -----  --------------------------------------------------
0     0x90   Reply Service (0x10 | 0x80)
1     0x00   Reserved
2     0x00   General Status: Success
3     0x00   Additional Status Size
```

#### 2-2. Param6 읽기 (검증)

쓰기 후 다시 읽어서 값이 변경되었는지 확인합니다.

**전송 데이터**:
```
Cmd=0x00008020
Len=0x00000008
Data=0E 03 20 30 24 01 30 0C
```

**예상 응답**:
```
Len=0x00000005
Data=8E 00 00 00 05
```

**응답 상세**:
```
Byte  Value  의미
----  -----  --------------------------------------------------
0     0x8E   Reply Service (0x0E | 0x80)
1     0x00   Reserved
2     0x00   General Status: Success
3     0x00   Additional Status Size
4     0x05   Data: Controller Mode = 5 ✅ 변경됨!
```

#### 2-3. Read-Only 파라미터 쓰기 (에러 테스트)

**전송 데이터**:
```
Cmd=0x00008020
Len=0x0000000A
Data=10 03 20 01 24 01 30 01 99 99
```

**상세 분석**:
```
Byte  Value  의미
----  -----  --------------------------------------------------
0     0x10   Service Code: Set Attribute Single
1     0x03   Path Size: 3 words
2     0x20   Logical Class ID
3     0x01   Class = 0x01
4     0x24   Logical Instance ID
5     0x01   Instance = 0x01
6     0x30   Logical Attribute ID
7     0x01   Attribute = 0x01 (Vendor ID - Read Only!)
8-9   99 99  Data: 임의의 값
```

**예상 응답** (에러):
```
Len=0x00000004
Data=90 00 0E 00
```

**응답 상세**:
```
Byte  Value  의미
----  -----  --------------------------------------------------
0     0x90   Reply Service (0x10 | 0x80)
1     0x00   Reserved
2     0x0E   General Status: Attribute Not Settable ❌
3     0x00   Additional Status Size
```

---

## 🔧 디버깅 방법

### 1. Printf 디버그 추가

**AppDNS_DemoApplicationFunctions.c**:
```c
void AppDNS_HandleExplicitMessage(RCX_PACKET* pRequest, RCX_PACKET* pResponse)
{
    printf("\n=== Explicit Message Received ===\n");
    printf("Request Len: %d\n", pRequest->tHeader.ulLen);

    // 요청 데이터 출력
    printf("Request Data: ");
    for (uint32_t i = 0; i < pRequest->tHeader.ulLen; i++) {
        printf("%02X ", pRequest->abData[i]);
    }
    printf("\n");

    // CIP 서비스 코드 확인
    uint8_t service = pRequest->abData[0];
    printf("Service Code: 0x%02X ", service);

    if (service == 0x0E) {
        printf("(Get Attribute Single)\n");
    } else if (service == 0x10) {
        printf("(Set Attribute Single)\n");
    } else {
        printf("(Unknown)\n");
    }

    // CIP 핸들러 호출
    uint8_t error = CIP_ProcessExplicitMessage(pRequest->abData,
                                                 pRequest->tHeader.ulLen,
                                                 pResponse->abData,
                                                 &pResponse->tHeader.ulLen);

    printf("Response Len: %d\n", pResponse->tHeader.ulLen);
    printf("Response Data: ");
    for (uint32_t i = 0; i < pResponse->tHeader.ulLen; i++) {
        printf("%02X ", pResponse->abData[i]);
    }
    printf("\n");
    printf("Error Code: 0x%02X\n", error);
}
```

### 2. CIP 메시지 처리 함수 추가

**AppDNS_ExplicitMsg.c에 추가**:
```c
uint8_t CIP_ProcessExplicitMessage(const uint8_t* pRequest, uint32_t reqLen,
                                    uint8_t* pResponse, uint32_t* pRespLen)
{
    if (reqLen < 2) {
        return CIP_ERROR_INVALID_MESSAGE;
    }

    uint8_t service = pRequest[0];
    uint8_t path_size = pRequest[1];  // in words

    // Path 크기 검증
    if (reqLen < (2 + path_size * 2)) {
        return CIP_ERROR_INVALID_MESSAGE;
    }

    // Path 파싱 (Logical Segment)
    uint8_t class_id = 0;
    uint8_t instance_id = 0;
    uint8_t attribute_id = 0;

    // Path 포맷: 20 <class> 24 <instance> 30 <attribute>
    if (pRequest[2] == 0x20) {
        class_id = pRequest[3];
    }
    if (pRequest[4] == 0x24) {
        instance_id = pRequest[5];
    }
    if (pRequest[6] == 0x30) {
        attribute_id = pRequest[7];
    }

    printf("  Class=0x%02X, Instance=0x%02X, Attribute=0x%02X\n",
           class_id, instance_id, attribute_id);

    uint8_t error = CIP_ERROR_SUCCESS;
    uint8_t data_size = 0;

    // 서비스 처리
    if (service == 0x0E) {  // Get Attribute Single
        error = CIP_HandleGetAttributeSingle(class_id, instance_id, attribute_id,
                                               pResponse + 4, &data_size);

        // 응답 헤더 작성
        pResponse[0] = 0x8E;  // Reply Service
        pResponse[1] = 0x00;  // Reserved
        pResponse[2] = error; // General Status
        pResponse[3] = 0x00;  // Additional Status Size

        *pRespLen = 4 + data_size;

    } else if (service == 0x10) {  // Set Attribute Single
        const uint8_t* pData = pRequest + 2 + path_size * 2;
        uint8_t dataLen = reqLen - (2 + path_size * 2);

        error = CIP_HandleSetAttributeSingle(class_id, instance_id, attribute_id,
                                               pData, dataLen);

        // 응답 헤더 작성
        pResponse[0] = 0x90;  // Reply Service
        pResponse[1] = 0x00;  // Reserved
        pResponse[2] = error; // General Status
        pResponse[3] = 0x00;  // Additional Status Size

        *pRespLen = 4;

    } else {
        // Unsupported Service
        error = CIP_ERROR_SERVICE_NOT_SUPPORTED;
        pResponse[0] = service | 0x80;
        pResponse[1] = 0x00;
        pResponse[2] = error;
        pResponse[3] = 0x00;
        *pRespLen = 4;
    }

    return error;
}
```

### 3. AppDNS_ExplicitMsg.h에 선언 추가

```c
/* CIP Message Processor */
uint8_t CIP_ProcessExplicitMessage(const uint8_t* pRequest, uint32_t reqLen,
                                    uint8_t* pResponse, uint32_t* pRespLen);

/* Error Codes */
#define CIP_ERROR_INVALID_MESSAGE           0x04
#define CIP_ERROR_SERVICE_NOT_SUPPORTED     0x08
```

---

## 📋 테스트 순서

### 1단계: 읽기 테스트 (5분)
```
① Param1 읽기 → 응답: 94 01 (404) ✅
② Param5 읽기 → 응답: 02 (Idle) ✅
③ 존재하지 않는 파라미터 읽기 → 응답: 에러 0x09 ✅
```

### 2단계: 쓰기 테스트 (5분)
```
④ Param6 쓰기 (5) → 응답: 성공 ✅
⑤ Param6 읽기 → 응답: 05 ✅ (변경 확인)
⑥ Read-Only 쓰기 → 응답: 에러 0x0E ✅
```

### 3단계: Printf 확인 (5분)
```
⑦ UART 출력 확인
⑧ 각 단계별 로그 분석
⑨ 에러 발생 시 디버깅
```

---

## 🎯 체크리스트

### 코드 확인
- [ ] `AppDNS_HandleExplicitMessage()` 함수 존재
- [ ] `CIP_ProcessExplicitMessage()` 함수 구현
- [ ] `CIP_HandleGetAttributeSingle()` 함수 동작
- [ ] `CIP_HandleSetAttributeSingle()` 함수 동작
- [ ] Param1, Param5, Param6 초기화 완료

### 테스트 확인
- [ ] Get Param1 → 404 반환
- [ ] Get Param5 → 2 반환
- [ ] Set Param6 → 성공
- [ ] Get Param6 → 5 반환
- [ ] Set Param1 (Read-Only) → 에러 0x0E

### 디버그 출력
- [ ] Printf로 요청 데이터 확인
- [ ] Printf로 응답 데이터 확인
- [ ] Printf로 에러 코드 확인

---

## 📊 전체 테스트 데이터 요약표

| 테스트 | Service | Class | Inst | Attr | Data | 전송 Hex | 예상 응답 Hex | 응답 의미 |
|--------|---------|-------|------|------|------|----------|---------------|-----------|
| **1. Get Param1** | 0x0E | 0x01 | 0x01 | 0x01 | - | `0E 03 20 01 24 01 30 01` | `8E 00 00 00 94 01` | Vendor ID = 404 |
| **2. Get Param5** | 0x0E | 0x30 | 0x01 | 0x0B | - | `0E 03 20 30 24 01 30 0B` | `8E 00 00 00 02` | Status = 2 (Idle) |
| **3. Set Param6** | 0x10 | 0x30 | 0x01 | 0x0C | 05 | `10 03 20 30 24 01 30 0C 05` | `90 00 00 00` | Success |
| **4. Get Param6** | 0x0E | 0x30 | 0x01 | 0x0C | - | `0E 03 20 30 24 01 30 0C` | `8E 00 00 00 05` | Mode = 5 |
| **5. Set Param1** | 0x10 | 0x01 | 0x01 | 0x01 | 99 99 | `10 03 20 01 24 01 30 01 99 99` | `90 00 0E 00` | Error 0x0E |

---

**작성일**: 2025-11-05
**버전**: 1.0

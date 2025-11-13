# VAT Explicit Message 테스트 가이드

**작성일**: 2025-11-11
**목적**: netHOST Device Test Application을 사용한 VAT Objects Explicit Message 테스트

---

## 구현된 CIP Objects

### 1. VAT Parameter Object (Class 0x64)
- **Instance**: 1-99 (99개 파라미터)
- **Attributes**:
  - Attr 1: Value (INT32)
  - Attr 2: Min (INT32)
  - Attr 3: Max (INT32)
  - Attr 4: Default (INT32)
  - Attr 5: Unit (UINT8)
  - Attr 6: Access (UINT8: 0=RW, 1=RO, 2=WO)
- **Services**:
  - 0x0E: Get_Attribute_Single
  - 0x10: Set_Attribute_Single
  - 0x01: Get_Attributes_All
  - 0x05: Reset

### 2. VAT Diagnostic Object (Class 0x65)
- **Instance**: 1 (단일 인스턴스)
- **Attributes** (15개):
  - Attr 1: Uptime (UINT32, seconds)
  - Attr 2: Total Cycles (UINT32)
  - Attr 3: Error Count (UINT16)
  - Attr 4: Last Error Code (UINT16)
  - Attr 5: Last Error Timestamp (UINT32)
  - Attr 6: Pressure Min (INT16)
  - Attr 7: Pressure Max (INT16)
  - Attr 8: Pressure Avg (INT16)
  - Attr 9: Position Min (INT16)
  - Attr 10: Position Max (INT16)
  - Attr 11: Network RX Count (UINT32)
  - Attr 12: Network TX Count (UINT32)
  - Attr 13: Network Error Count (UINT16)
  - Attr 14: Temperature (INT16, Celsius)
  - Attr 15: Firmware Version (UINT32, 0xMMmmppbb)
- **Services**:
  - 0x0E: Get_Attribute_Single
  - 0x01: Get_Attributes_All
  - 0x05: Reset

### 3. Assembly Object (Class 0x04)
- **Instances**:
  - 100: Input Assembly (10 bytes)
  - 101: Output Assembly (6 bytes)
  - 102: Config Assembly (4 bytes)

---

## netHOST 패킷 구조

### CIP Service Request 패킷 형식
```
Command:  0x0000308A (DNS_CMD_CIP_SERVICE_IND)
Data:
  [0-3]:   Service (UINT32)
  [4-7]:   Class (UINT32)
  [8-11]:  Instance (UINT32)
  [12-15]: Attribute (UINT32) - Get/Set_Attribute_Single만 해당
  [16+]:   Data (Set operations만 해당)
```

---

## 테스트 시나리오

## 테스트 1: VAT Parameter Object - Get Attribute Single

### 목적
Parameter Instance 1의 Value (Attr 1) 읽기

### 패킷 설정
```
Cmd:         0x0000308A
Data Length: 16 bytes
Data (HEX):
0E 00 00 00    // Service: 0x0E (Get_Attribute_Single)
64 00 00 00    // Class: 0x64 (VAT Parameter)
01 00 00 00    // Instance: 1
01 00 00 00    // Attribute: 1 (Value)
```

### 예상 응답
```
Cmd:         0x0000308B (DNS_CMD_CIP_SERVICE_RES)
State:       0x00000000 (Success)
Data:
00 00 00 00    // GRC: 0x00 (Success)
00 00 00 00    // ERC: 0x00
[4 bytes]      // INT32 Value (예: 00 00 00 00 = 0)
```

---

## 테스트 2: VAT Parameter Object - Set Attribute Single

### 목적
Parameter Instance 1의 Value를 100으로 설정

### 패킷 설정
```
Cmd:         0x0000308A
Data Length: 20 bytes
Data (HEX):
10 00 00 00    // Service: 0x10 (Set_Attribute_Single)
64 00 00 00    // Class: 0x64 (VAT Parameter)
01 00 00 00    // Instance: 1
01 00 00 00    // Attribute: 1 (Value)
64 00 00 00    // Data: 100 (INT32, little-endian)
```

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
00 00 00 00    // GRC: 0x00 (Success)
00 00 00 00    // ERC: 0x00
                // No data for Set operations
```

---

## 테스트 3: VAT Parameter Object - Get Attributes All

### 목적
Parameter Instance 5의 모든 속성 읽기

### 패킷 설정
```
Cmd:         0x0000308A
Data Length: 12 bytes (Attribute 필드 없음)
Data (HEX):
01 00 00 00    // Service: 0x01 (Get_Attributes_All)
64 00 00 00    // Class: 0x64
05 00 00 00    // Instance: 5
```

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
00 00 00 00    // GRC: 0x00
00 00 00 00    // ERC: 0x00
[18 bytes]     // All attributes:
               // Value (4), Min (4), Max (4), Default (4), Unit (1), Access (1)
```

---

## 테스트 4: VAT Diagnostic Object - Get Attribute Single

### 목적
Uptime (Attr 1) 읽기

### 패킷 설정
```
Cmd:         0x0000308A
Data Length: 16 bytes
Data (HEX):
0E 00 00 00    // Service: 0x0E (Get_Attribute_Single)
65 00 00 00    // Class: 0x65 (VAT Diagnostic)
01 00 00 00    // Instance: 1
01 00 00 00    // Attribute: 1 (Uptime)
```

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
00 00 00 00    // GRC: 0x00
00 00 00 00    // ERC: 0x00
[4 bytes]      // UINT32 Uptime in seconds
```

---

## 테스트 5: VAT Diagnostic Object - Get Attributes All

### 목적
모든 Diagnostic 데이터 읽기

### 패킷 설정
```
Cmd:         0x0000308A
Data Length: 12 bytes
Data (HEX):
01 00 00 00    // Service: 0x01 (Get_Attributes_All)
65 00 00 00    // Class: 0x65 (VAT Diagnostic)
01 00 00 00    // Instance: 1
```

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
00 00 00 00    // GRC: 0x00
00 00 00 00    // ERC: 0x00
[30 bytes]     // All diagnostic data:
               // Uptime (4), Total Cycles (4), Error Count (2),
               // Last Error Code (2), Pressure Min (2), Pressure Max (2),
               // Pressure Avg (2), Position Min (2), Position Max (2),
               // Temperature (2), Firmware Version (4)
```

---

## 테스트 6: VAT Diagnostic Object - Reset

### 목적
Diagnostic 통계 초기화

### 패킷 설정
```
Cmd:         0x0000308A
Data Length: 12 bytes
Data (HEX):
05 00 00 00    // Service: 0x05 (Reset)
65 00 00 00    // Class: 0x65
01 00 00 00    // Instance: 1
```

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
00 00 00 00    // GRC: 0x00
00 00 00 00    // ERC: 0x00
                // No data
```

---

## 테스트 7: VAT Parameter Object - Reset

### 목적
Parameter Instance 5를 기본값으로 복원

### 패킷 설정
```
Cmd:         0x0000308A
Data Length: 12 bytes
Data (HEX):
05 00 00 00    // Service: 0x05 (Reset)
64 00 00 00    // Class: 0x64
05 00 00 00    // Instance: 5
```

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
00 00 00 00    // GRC: 0x00
00 00 00 00    // ERC: 0x00
```

---

## 테스트 8: Assembly Object - Get Attribute Single

### 목적
Input Assembly (Instance 100) 데이터 읽기

### 패킷 설정
```
Cmd:         0x0000308A
Data Length: 16 bytes
Data (HEX):
0E 00 00 00    // Service: 0x0E (Get_Attribute_Single)
04 00 00 00    // Class: 0x04 (Assembly)
64 00 00 00    // Instance: 100 (0x64)
03 00 00 00    // Attribute: 3 (Data)
```

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
00 00 00 00    // GRC: 0x00
00 00 00 00    // ERC: 0x00
[10 bytes]     // Input Assembly Data
```

---

## 에러 테스트

### 테스트 9: 잘못된 Instance
```
Cmd:         0x0000308A
Data (HEX):
0E 00 00 00    // Service: Get_Attribute_Single
64 00 00 00    // Class: 0x64
FF 00 00 00    // Instance: 255 (존재하지 않음)
01 00 00 00    // Attribute: 1
```

**예상 응답**:
```
GRC: 0x16 (Object Does Not Exist)
```

### 테스트 10: 잘못된 Attribute
```
Cmd:         0x0000308A
Data (HEX):
0E 00 00 00    // Service: Get_Attribute_Single
64 00 00 00    // Class: 0x64
01 00 00 00    // Instance: 1
FF 00 00 00    // Attribute: 255 (존재하지 않음)
```

**예상 응답**:
```
GRC: 0x14 (Attribute Not Supported)
```

### 테스트 11: Read-Only Parameter 쓰기 시도
```
Cmd:         0x0000308A
Data (HEX):
10 00 00 00    // Service: Set_Attribute_Single
64 00 00 00    // Class: 0x64
03 00 00 00    // Instance: 3 (Access = RO)
01 00 00 00    // Attribute: 1
64 00 00 00    // Data: 100
```

**예상 응답**:
```
GRC: 0x0E (Attribute Not Settable)
```

---

## netHOST 사용 방법

### 1. Send Packet 설정
1. **Dest**: `0x00000020` (유지)
2. **Src**: `0x00000010` (유지)
3. **Cmd**: `0x0000308A` (DNS_CMD_CIP_SERVICE_IND)
4. **Len**: 데이터 바이트 수 입력
5. **Data**: 위 테스트 시나리오의 HEX 데이터 입력
6. **Send Cyclic** 체크 해제
7. **Put Packet** 클릭

### 2. 응답 확인
1. **Received Packet** 섹션에서 응답 확인
2. **Cmd**: `0x0000308B` (응답)
3. **State**: `0x00000000` (성공)
4. **Data**: GRC/ERC 및 응답 데이터 확인

---

## CIP General Response Codes (GRC)

| Code | 이름 | 설명 |
|------|------|------|
| 0x00 | Success | 성공 |
| 0x08 | Service Not Supported | 지원되지 않는 서비스 |
| 0x0E | Attribute Not Settable | 읽기 전용 속성 |
| 0x14 | Attribute Not Supported | 존재하지 않는 속성 |
| 0x16 | Object Does Not Exist | 존재하지 않는 인스턴스 |
| 0x26 | Invalid Attribute Value | 잘못된 속성 값 (범위 초과) |

---

## 디버깅

### 시리얼 출력 모니터링
코드에 `printf` 디버그 메시지가 포함되어 있습니다:

```
[VAT] CIP Service: 0x0E, Class: 0x64, Inst: 1, Attr: 1
[VAT Param] GET Instance 1, Attr 1
[VAT Param] GET Attr1: Value = 0
[VAT] Response sent: GRC=0x00, DataSize=4
```

### 주요 확인 사항
1. **CIP Service 수신 여부**: `[VAT] CIP Service:` 메시지
2. **처리 성공 여부**: GRC 값
3. **응답 데이터 크기**: DataSize 값

---

## 추가 구현 필요 사항

### 1. CIP Service 라우팅 활성화
`App_DemoApplication.c`의 패킷 핸들러에서 VAT 핸들러 호출:

```c
// AppDNS_HandleCipServiceIndication() 함수에 추가
if(ptInd->tHead.ulCmd == DNS_CMD_CIP_SERVICE_IND) {
    // VAT Explicit Handler 호출
    if(VAT_Explicit_HandleCipService(&g_tDnsRsc, ptInd)) {
        return; // VAT에서 처리됨
    }
}
```

### 2. DNS Resource 구조체 초기화
`AppDNS_DemoApplication.c`에서 채널 리소스 초기화 필요

---

## 성공 기준

- [x] 컴파일 성공
- [ ] **Parameter Get_Attribute_Single 성공** (테스트 1)
- [ ] **Parameter Set_Attribute_Single 성공** (테스트 2)
- [ ] **Diagnostic Get_Attributes_All 성공** (테스트 5)
- [ ] **에러 코드 정상 반환** (테스트 9-11)
- [ ] **시리얼 디버그 메시지 출력**

---

**작성 완료**: 2025-11-11

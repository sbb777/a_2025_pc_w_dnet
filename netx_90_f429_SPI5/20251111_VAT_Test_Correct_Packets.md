# VAT Explicit Message 테스트 - 올바른 패킷 포맷

**작성일**: 2025-11-11
**중요**: DNS_CIP_SERVICE_REQ_T 구조체는 **5개 필드**를 포함합니다!

---

## DNS_CIP_SERVICE_REQ_T 구조체

```c
typedef struct DNS_CIP_SERVICE_REQ_Ttag {
  uint32_t    ulService;      // 4 bytes - CIP Service Code
  uint32_t    ulClass;        // 4 bytes - CIP Class ID
  uint32_t    ulInstance;     // 4 bytes - CIP Instance Number
  uint32_t    ulAttribute;    // 4 bytes - CIP Attribute Number
  uint32_t    ulMember;       // 4 bytes - Member/Device ID ← 중요!
  uint8_t     abData[];       // Variable - CIP Service Data
} DNS_CIP_SERVICE_REQ_T;
```

**핵심**: `ulMember` 필드를 반드시 포함해야 합니다!
- 일반적으로 Device ID 또는 배열 인덱스로 사용
- 현재 시스템의 MacID (Node ID)가 1이면 → `01 00 00 00`

---

## 올바른 패킷 예시

### 1. Pressure Setpoint 읽기 (Get)

```
Cmd:         0x0000308A (DNS_CMD_CIP_SERVICE_IND)
Len:         20 bytes (5 fields × 4 bytes)
Data:
0E 00 00 00    // ulService:   0x0E (Get_Attribute_Single)
64 00 00 00    // ulClass:     0x64 (VAT Parameter)
01 00 00 00    // ulInstance:  1 (Pressure Setpoint)
01 00 00 00    // ulAttribute: 1
01 00 00 00    // ulMember:    1 (Device ID)
```

**예상 응답**:
```
Cmd:  0x0000308B
Data: 00 00 00 00 00 00 00 00 88 13
      ↑           ↑           ↑
      GRC=0x00    ERC=0x00    Value=5000 (50.00 mbar)
```

---

### 2. Pressure Setpoint 쓰기 (Set)

```
Cmd:         0x0000308A
Len:         22 bytes (20 + 2 data bytes)
Data:
10 00 00 00    // ulService:   0x10 (Set_Attribute_Single)
64 00 00 00    // ulClass:     0x64
01 00 00 00    // ulInstance:  1
01 00 00 00    // ulAttribute: 1
01 00 00 00    // ulMember:    1 (Device ID)
58 1B          // abData:      7000 (70.00 mbar, little-endian)
```

**예상 응답**:
```
Cmd:  0x0000308B
Data: 00 00 00 00 00 00 00 00
      ↑           ↑
      GRC=0x00    ERC=0x00 (No data for Set)
```

---

### 3. Control Mode 읽기

```
Cmd:         0x0000308A
Len:         20 bytes
Data:
0E 00 00 00    // Service: 0x0E
64 00 00 00    // Class: 0x64
03 00 00 00    // Instance: 3 (Control Mode)
01 00 00 00    // Attribute: 1
01 00 00 00    // Member: 1 (Device ID) ← 추가!
```

**예상 응답**: `00 00 00 00 00 00 00 00 00` (Value=0, Pressure Control)

---

### 4. Control Mode 쓰기 (Position Control로 변경)

```
Cmd:         0x0000308A
Len:         21 bytes
Data:
10 00 00 00    // Service: 0x10
64 00 00 00    // Class: 0x64
03 00 00 00    // Instance: 3
01 00 00 00    // Attribute: 1
01 00 00 00    // Member: 1 (Device ID) ← 추가!
01             // Data: 1 (Position Control)
```

---

### 5. Actual Pressure 읽기 (Read-Only)

```
Cmd:         0x0000308A
Len:         20 bytes
Data:
0E 00 00 00    // Service: 0x0E
64 00 00 00    // Class: 0x64
04 00 00 00    // Instance: 4 (Actual Pressure)
01 00 00 00    // Attribute: 1
01 00 00 00    // Member: 1 (Device ID) ← 추가!
```

---

### 6. Device Status 읽기

```
Cmd:         0x0000308A
Len:         20 bytes
Data:
0E 00 00 00    // Service: 0x0E
64 00 00 00    // Class: 0x64
06 00 00 00    // Instance: 6 (Device Status)
01 00 00 00    // Attribute: 1
01 00 00 00    // Member: 1 (Device ID) ← 추가!
```

**예상 응답**: `00 00 00 00 00 00 00 00 01` (Value=1, Idle)

---

### 7. Diagnostic Uptime 읽기

```
Cmd:         0x0000308A
Len:         20 bytes
Data:
0E 00 00 00    // Service: 0x0E
65 00 00 00    // Class: 0x65 (VAT Diagnostic) ← Class 변경
01 00 00 00    // Instance: 1
01 00 00 00    // Attribute: 1 (Uptime)
01 00 00 00    // Member: 1 (Device ID) ← 추가!
```

**예상 응답**: `00 00 00 00 00 00 00 00 [4 bytes UINT32]`

---

### 8. Diagnostic Get All

```
Cmd:         0x0000308A
Len:         20 bytes
Data:
01 00 00 00    // Service: 0x01 (Get_Attributes_All)
65 00 00 00    // Class: 0x65
01 00 00 00    // Instance: 1
00 00 00 00    // Attribute: 0 (not used for Get_All)
01 00 00 00    // Member: 1 (Device ID) ← 추가!
```

---

## 에러 테스트

### 9. Read-Only Parameter 쓰기 시도

```
Cmd:         0x0000308A
Len:         22 bytes
Data:
10 00 00 00    // Service: 0x10 (Set)
64 00 00 00    // Class: 0x64
04 00 00 00    // Instance: 4 (Actual Pressure - READ ONLY!)
01 00 00 00    // Attribute: 1
01 00 00 00    // Member: 1 (Device ID) ← 추가!
64 00          // Data: 100
```

**예상 응답**: `02 00 00 00 00 00 00 00` (GRC=0x02, Too Much Data/Read-Only)

---

### 10. 범위 초과 값 쓰기

```
Cmd:         0x0000308A
Len:         22 bytes
Data:
10 00 00 00    // Service: 0x10
64 00 00 00    // Class: 0x64
01 00 00 00    // Instance: 1
01 00 00 00    // Attribute: 1
01 00 00 00    // Member: 1 (Device ID) ← 추가!
11 27          // Data: 10001 = 0x2711 (Out of range! Max=10000)
```

**예상 응답**: `09 00 00 00 00 00 00 00` (GRC=0x09, Invalid Attribute Value)

---

## netHOST 입력 방법

### Send Packet 설정 예시 (Pressure Setpoint 읽기)

1. **Cmd**: `0x0000308A`
2. **Len**: `20`
3. **Data**:
   ```
   0E 00 00 00 64 00 00 00 01 00 00 00 01 00 00 00 01 00 00 00
   ```
   또는 스페이스로 구분:
   ```
   0E 00 00 00  64 00 00 00  01 00 00 00  01 00 00 00  01 00 00 00
   ```

---

## 패킷 길이 계산

### Get_Attribute_Single (읽기)
```
Len = 20 bytes
  = ulService (4) + ulClass (4) + ulInstance (4)
    + ulAttribute (4) + ulMember (4)
```

### Set_Attribute_Single (쓰기)
```
Len = 20 + data_size
  예: INT16 쓰기 = 20 + 2 = 22 bytes
  예: UINT8 쓰기 = 20 + 1 = 21 bytes
```

### Get_Attributes_All
```
Len = 20 bytes (data 없음)
```

---

## 중요 체크리스트

- [ ] **ulMember 필드 포함** (Device ID = 1)
- [ ] **Little-Endian 바이트 순서** (예: 7000 = 58 1B)
- [ ] **올바른 Len 값** (Get=20, Set=20+data_size)
- [ ] **Cmd = 0x0000308A** (Indication)

---

## Device ID 확인 방법

현재 시스템의 MacID (Node ID)를 확인하는 방법:

1. **EDS 파일 확인**: `#MacId` 섹션
2. **Stack 설정 확인**: Configuration 패킷
3. **일반적인 값**: 1-63 (DeviceNet 표준)

테스트 시 MacID가 다르면 `ulMember` 값을 해당 ID로 변경하세요.

---

**작성 완료**: 2025-11-11

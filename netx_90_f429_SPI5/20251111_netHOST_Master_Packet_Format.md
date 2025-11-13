# netHOST Master에서 보내는 올바른 패킷 포맷

**작성일**: 2025-11-11
**중요**: netHOST는 **Master 시뮬레이터**이므로 **Master FAL API** 구조를 사용해야 합니다!

---

## 핵심 개념

### Slave 내부 구조 vs Master 네트워크 패킷

```
Master (netHOST)                    Slave (우리 구현)
     │                                    │
     │ FAL API 패킷                       │
     │ (DEVNET_FAL_CMD_GET_ATT_REQ)      │
     │                                    │
     ├─────────── 네트워크 전송 ──────────>│
     │                                    │
     │                              DNS 스택 수신
     │                                    │
     │                              변환 처리
     │                                    │
     │                         DNS_CMD_CIP_SERVICE_IND
     │                                    │
     │                         애플리케이션 핸들러
     │                         (App_VAT_UserObject.c)
```

### 두 가지 구조체 비교

| 항목 | Master FAL API | Slave DNS API |
|------|----------------|---------------|
| **명령 코드** | 0x0000380A (GET) / 0x0000380B (SET) | 0x0000308A (IND) / 0x0000308B (RES) |
| **구조체** | `DN_FAL_SDU_GETSET_ATT_REQ_T` | `DNS_CIP_SERVICE_REQ_T` |
| **첫 필드** | bDeviceAddr (MAC ID) | ulService (CIP Service) |
| **필드 크기** | 혼합 (UINT8, UINT16) | 모두 UINT32 |

---

## Master FAL API 패킷 구조

### DN_FAL_SDU_GETSET_ATT_REQ_T

```c
typedef struct DN_FAL_SDU_GETSET_ATT_REQ_Ttag
{
  uint8_t   bDeviceAddr;      // 1 byte  - Target device MAC ID (0-63)
  uint8_t   abReserved[3];    // 3 bytes - Reserved (0)
  uint16_t  usClass;          // 2 bytes - CIP Class ID (little-endian)
  uint16_t  usInstance;       // 2 bytes - CIP Instance (little-endian)
  uint16_t  usAttribute;      // 2 bytes - CIP Attribute (little-endian)
  uint16_t  usReserved;       // 2 bytes - Reserved (0)
  uint8_t   abData[];         // Variable - Data for SET operation
} DN_FAL_SDU_GETSET_ATT_REQ_T;
```

**총 크기**:
- GET: 12 bytes (고정)
- SET: 12 + data_size bytes

---

## netHOST 테스트 패킷 (올바른 버전)

### 1. Pressure Setpoint 읽기 (Param 1)

**Cmd**: `0x0000380A` (DEVNET_FAL_CMD_GET_ATT_REQ)
**Len**: `12` bytes

**Data**:
```
01 00 00 00  64 00 01 00  01 00 00 00
↑            ↑      ↑      ↑      ↑
│            │      │      │      └─ usReserved (0x0000)
│            │      │      └──────── usAttribute (0x0001 = Attribute 1)
│            │      └─────────────── usInstance (0x0001 = Instance 1)
│            └────────────────────── usClass (0x0064 = Class 0x64)
└─────────────────────────────────── bDeviceAddr (0x01 = MAC ID 1)
                                     abReserved (0x00 0x00 0x00)
```

**바이트별 분석**:
```
Offset  Field         Hex Value   Decimal  설명
------  ------------  ----------  -------  ---------------------------
0       bDeviceAddr   01          1        Target Slave MAC ID = 1
1-3     abReserved    00 00 00    0        Reserved
4-5     usClass       64 00       100      Class 0x64 (VAT Parameter)
6-7     usInstance    01 00       1        Instance 1 (Pressure Setpoint)
8-9     usAttribute   01 00       1        Attribute 1
10-11   usReserved    00 00       0        Reserved
```

**예상 응답** (Cmd: 0x0000380B):
```
Data: 88 13
      ↑
      5000 (little-endian) = 50.00 mbar
```

---

### 2. Pressure Setpoint 쓰기 (7000 = 70.00 mbar)

**Cmd**: `0x0000380B` (DEVNET_FAL_CMD_SET_ATT_REQ)
**Len**: `14` bytes (12 + 2 data)

**Data**:
```
01 00 00 00  64 00 01 00  01 00 00 00  58 1B
↑            ↑      ↑      ↑      ↑      ↑
│            │      │      │      │      └─ abData (0x1B58 = 7000)
│            │      │      │      └──────── usReserved
│            │      │      └─────────────── usAttribute (1)
│            │      └────────────────────── usInstance (1)
│            └───────────────────────────── usClass (0x64)
└────────────────────────────────────────── bDeviceAddr (1)
```

**예상 응답**: Success (GRC=0x00)

---

### 3. Control Mode 읽기 (Param 3)

**Cmd**: `0x0000380A`
**Len**: `12` bytes

**Data**:
```
01 00 00 00  64 00 03 00  01 00 00 00
             ↑      ↑
             Class  Instance 3 (Control Mode)
```

**예상 응답**: `00` (Pressure Control)

---

### 4. Control Mode 쓰기 (Position Control)

**Cmd**: `0x0000380B`
**Len**: `13` bytes

**Data**:
```
01 00 00 00  64 00 03 00  01 00 00 00  01
                                        ↑
                                        1 = Position Control
```

---

### 5. Actual Pressure 읽기 (Read-Only, Param 4)

**Cmd**: `0x0000380A`
**Len**: `12` bytes

**Data**:
```
01 00 00 00  64 00 04 00  01 00 00 00
             ↑      ↑
             Class  Instance 4 (Actual Pressure)
```

**예상 응답**: 2 bytes INT16 (현재 압력 값)

---

### 6. Device Status 읽기 (Param 6)

**Cmd**: `0x0000380A`
**Len**: `12` bytes

**Data**:
```
01 00 00 00  64 00 06 00  01 00 00 00
             ↑      ↑
             Class  Instance 6 (Device Status)
```

**예상 응답**: `01` (Idle)

---

### 7. Diagnostic Uptime 읽기 (Class 0x65)

**Cmd**: `0x0000380A`
**Len**: `12` bytes

**Data**:
```
01 00 00 00  65 00 01 00  01 00 00 00
             ↑      ↑      ↑
             0x65   Inst 1  Attr 1 (Uptime)
```

**예상 응답**: 4 bytes UINT32 (초 단위 uptime)

---

## 에러 테스트

### 8. Read-Only 파라미터 쓰기 시도

**Cmd**: `0x0000380B` (SET)
**Len**: `14` bytes

**Data**:
```
01 00 00 00  64 00 04 00  01 00 00 00  64 00
             ↑      ↑
             Class  Instance 4 (Actual Pressure - READ ONLY!)
```

**예상 응답**: Error GRC=0x02 (Too Much Data / Read-Only)

---

### 9. 범위 초과 값 쓰기

**Cmd**: `0x0000380B`
**Len**: `14` bytes

**Data**:
```
01 00 00 00  64 00 01 00  01 00 00 00  11 27
                                        ↑
                                        0x2711 = 10001 (Out of range!)
```

**예상 응답**: Error GRC=0x09 (Invalid Attribute Value)

---

### 10. 존재하지 않는 인스턴스

**Cmd**: `0x0000380A`
**Len**: `12` bytes

**Data**:
```
01 00 00 00  64 00 FF 00  01 00 00 00
             ↑      ↑
             Class  Instance 255 (존재하지 않음!)
```

**예상 응답**: Error GRC=0x16 (Object Does Not Exist)

---

## netHOST 입력 방법

### Send Packet 화면 설정

#### Pressure Setpoint 읽기 예시

**1. Command (Cmd)**: `0x0000380A`

**2. Length (Len)**: `12`

**3. Data**:
```
01 00 00 00 64 00 01 00 01 00 00 00
```

또는 스페이스로 구분:
```
01 00 00 00  64 00 01 00  01 00 00 00
```

**4. Destination (ulDest)**: `0x00000020` (Channel Token)

**5. Source (ulSrc)**: `0x00000010` (Host Queue Handle)

---

## 동작 흐름 상세

```
1. netHOST Master
   │
   │ Send: Cmd=0x380A, Data=[01 00 00 00 64 00 01 00 01 00 00 00]
   │
   ↓
2. DeviceNet 네트워크 전송
   │
   │ CAN 프레임으로 변환되어 전송
   │
   ↓
3. Slave DNS 스택 수신
   │
   │ FAL 패킷 파싱:
   │   - bDeviceAddr = 1 (자신의 MAC ID 확인)
   │   - usClass = 0x64
   │   - usInstance = 1
   │   - usAttribute = 1
   │
   ↓
4. DNS 스택 → 애플리케이션 변환
   │
   │ DNS_CMD_CIP_SERVICE_IND (0x308A) 생성:
   │   - ulService = 0x0E (Get_Attribute_Single)
   │   - ulClass = 0x64
   │   - ulInstance = 1
   │   - ulAttribute = 1
   │   - ulMember = 0 (내부 처리)
   │
   ↓
5. AppDNS_PacketHandler_callback()
   │
   ↓
6. AppDNS_HandleCipServiceIndication()
   │
   │ Class 0x64 감지 → VAT Handler로 라우팅
   │
   ↓
7. AppDNS_VAT_UserObject_Indication()
   │
   ↓
8. AppDNS_VAT_HandleParameterObject()
   │
   │ Service 0x0E → VAT_Param_Get()
   │   - Param 1 데이터: 88 13 (5000)
   │
   ↓
9. 응답 패킷 구성
   │
   │ DNS_CMD_CIP_SERVICE_RES (0x308B):
   │   - GRC = 0x00 (Success)
   │   - ERC = 0x00
   │   - Data = 88 13
   │
   ↓
10. DNS 스택 → Master 응답 변환
   │
   │ DEVNET_FAL_CMD_GET_ATT_CNF (0x380B):
   │   - Data = 88 13
   │
   ↓
11. netHOST Master 수신
    │
    └─> 화면에 응답 표시: 88 13 (5000 = 50.00 mbar)
```

---

## 왜 이전 패킷이 작동하지 않았나?

### 이전 패킷 (작동 안 함)
```
Cmd: 0x0000308A (DNS_CMD_CIP_SERVICE_IND)
Data: 0E 00 00 00 64 00 00 00 01 00 00 00 01 00 00 00 01 00 00 00
```

**문제점**:
1. **잘못된 명령 코드**: 0x308A는 **Slave 내부 Indication** 패킷
2. **Master는 0x380A를 사용해야 함**: FAL API GET 명령
3. **구조체 불일치**: Master FAL 구조 (12 bytes)와 Slave DNS 구조 (20 bytes)가 다름

### Vendor ID 요청 (작동함)
```
Cmd: 0x0000380A (DEVNET_FAL_CMD_GET_ATT_REQ)
Data: 03 00 00 00 01 00 01 00 01 00 00 00
```

**성공 이유**:
1. ✅ 올바른 명령 코드: 0x380A (Master FAL GET)
2. ✅ 올바른 구조: DN_FAL_SDU_GETSET_ATT_REQ_T
3. ✅ MAC ID 지정: 03 (장치 주소)

---

## 주요 차이점 요약

| 구분 | 잘못된 패킷 | 올바른 패킷 |
|------|-------------|-------------|
| **Cmd** | 0x0000308A (Slave IND) | 0x0000380A (Master GET) |
| **Data 크기** | 20 bytes | 12 bytes |
| **첫 필드** | ulService (4 bytes) | bDeviceAddr (1 byte) |
| **Class 위치** | Offset 4-7 | Offset 4-5 |
| **필드 타입** | 모두 UINT32 | UINT8 + UINT16 혼합 |
| **용도** | Slave 내부 처리 | Master → Slave 네트워크 전송 |

---

## 체크리스트

### netHOST 설정 확인
- [ ] Command = **0x0000380A** (GET) 또는 **0x0000380B** (SET)
- [ ] Length = 12 (GET) 또는 12 + data_size (SET)
- [ ] Data 첫 바이트 = **Slave MAC ID** (예: 01)
- [ ] Class, Instance, Attribute는 **UINT16 little-endian**
- [ ] ulDest = 0x00000020 (Channel Token)

### 예상 응답 확인
- [ ] Response Cmd = 0x0000380B
- [ ] GRC = 0x00 (Success) 또는 에러 코드
- [ ] Data 포함 (GET의 경우)

---

**작성 완료**: 2025-11-11
**중요**: netHOST는 Master이므로 **FAL API (0x380A/0x380B)**를 사용해야 합니다!

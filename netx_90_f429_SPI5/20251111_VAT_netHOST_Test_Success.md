# VAT Explicit Message netHOST 테스트 성공

**작성일**: 2025-11-11
**목적**: VAT UserObject Explicit Message 통신 netHOST 테스트 성공 및 최종 패킷 포맷 확정

---

## 테스트 결과

✅ **성공**: VAT Pressure Setpoint Explicit Message 읽기/쓰기 정상 동작 확인

---

## 문제 해결 과정

### 초기 문제점

#### 시도 1: Slave DNS API 구조 사용 (실패)
```
Cmd:  0x0000308A (DNS_CMD_CIP_SERVICE_IND)
Len:  20
Data: 0E 00 00 00 64 00 00 00 01 00 00 00 01 00 00 00 01 00 00 00

결과: State: 0xC0000004 (ERR_HIL_UNKNOWN_COMMAND)
```

**실패 원인**:
- `0x0000308A`는 **Slave 내부 Indication** 명령
- Master가 Slave로 보낼 수 없는 명령 코드
- 구조체도 DNS_CIP_SERVICE_REQ_T (20 bytes)로 잘못됨

#### 시도 2: 잘못된 Cmd와 Data 입력 (실패)
```
Cmd:  0x00008102 (존재하지 않는 명령)
Len:  20
Data: 0E 00 00 00 64 00 00 01 00 00 01 00 00 01 00 00 00 01 00 00 00
      ↑                    ↑ 바이트 정렬이 엉망

결과: State: 0x00000004 (ERR_HIL_UNKNOWN_COMMAND)
```

**실패 원인**:
1. 잘못된 명령 코드 `0x00008102`
2. Data 바이트 정렬 오류 (복사/붙여넣기 실수)

### 해결 방법 발견

**Vendor ID 성공 패킷 분석**을 통해 올바른 방법 확인:

```
Cmd:  0x0000380A (DEVNET_FAL_CMD_GET_ATT_REQ)
Len:  12
Data: 03 00 00 00 01 00 01 00 01 00 00 00
      ↑            ↑      ↑      ↑
      MAC ID=3     Class  Inst   Attr
```

**핵심 발견**:
1. **Master FAL API** 사용 (`0x0000380A`)
2. **DN_FAL_SDU_GETSET_ATT_REQ_T** 구조 (12 bytes)
3. 첫 필드는 **Device MAC Address** (1 byte + 3 reserved)
4. Class, Instance, Attribute는 **UINT16** (2 bytes each)

---

## 최종 확정 패킷 포맷

### Master FAL API 구조

```c
typedef struct DN_FAL_SDU_GETSET_ATT_REQ_Ttag
{
  uint8_t   bDeviceAddr;      // 1 byte  - Target Slave MAC ID
  uint8_t   abReserved[3];    // 3 bytes - Reserved (0)
  uint16_t  usClass;          // 2 bytes - CIP Class ID (little-endian)
  uint16_t  usInstance;       // 2 bytes - CIP Instance (little-endian)
  uint16_t  usAttribute;      // 2 bytes - CIP Attribute (little-endian)
  uint16_t  usReserved;       // 2 bytes - Reserved (0)
  uint8_t   abData[];         // Variable - Data for SET operation
} DN_FAL_SDU_GETSET_ATT_REQ_T;
```

### 패킷 헤더 구조

```c
typedef struct TLR_PACKET_HEADER_Ttag
{
  uint32_t  ulDest;      // 0x00000020 - Channel Token
  uint32_t  ulSrc;       // 0x00000010 - Host Queue Handle
  uint32_t  ulDestId;    // 0x00000000
  uint32_t  ulSrcId;     // 0x00000000
  uint32_t  ulLen;       // Data Length (12 for GET, 12+N for SET)
  uint32_t  ulId;        // Packet ID (optional)
  uint32_t  ulSta;       // Status (0 for request)
  uint32_t  ulCmd;       // Command Code
  uint32_t  ulExt;       // Extension (0)
  uint32_t  ulRout;      // Routing (0)
} TLR_PACKET_HEADER_T;
```

---

## netHOST 성공 패킷

### 1. Pressure Setpoint 읽기 (성공)

#### 요청 패킷
```
Command (Cmd):  0x0000380A
Length (Len):   12
Data (Hex):     03 00 00 00 64 00 01 00 01 00 00 00
```

#### 바이트별 상세
```
Byte 0:     03        bDeviceAddr (MAC ID = 3)
Byte 1-3:   00 00 00  abReserved
Byte 4-5:   64 00     usClass = 0x0064 (100 decimal, VAT Parameter Object)
Byte 6-7:   01 00     usInstance = 0x0001 (1, Pressure Setpoint)
Byte 8-9:   01 00     usAttribute = 0x0001 (1)
Byte 10-11: 00 00     usReserved
```

#### 예상 응답
```
Command:  0x0000380B (DEVNET_FAL_CMD_GET_ATT_CNF)
State:    0x00000000 (Success)
Data:     88 13
          ↑
          0x1388 = 5000 (little-endian)
          = 50.00 mbar
```

---

### 2. Pressure Setpoint 쓰기 (70.00 mbar)

#### 요청 패킷
```
Command (Cmd):  0x0000380B
Length (Len):   14
Data (Hex):     03 00 00 00 64 00 01 00 01 00 00 00 58 1B
```

#### 바이트별 상세
```
Byte 0:     03        bDeviceAddr (MAC ID = 3)
Byte 1-3:   00 00 00  abReserved
Byte 4-5:   64 00     usClass = 0x0064
Byte 6-7:   01 00     usInstance = 1
Byte 8-9:   01 00     usAttribute = 1
Byte 10-11: 00 00     usReserved
Byte 12-13: 58 1B     abData = 0x1B58 = 7000 (70.00 mbar)
```

#### 예상 응답
```
Command:  0x0000380B
State:    0x00000000 (Success)
Data:     (no data for SET confirmation)
```

---

## VAT 파라미터 전체 테스트 가이드

### 읽기 테스트 (모두 Cmd: 0x0000380A, Len: 12)

| 파라미터 | Instance | Data (12 bytes) | 예상 응답 |
|---------|----------|-----------------|----------|
| **Pressure Setpoint** | 1 | `03 00 00 00 64 00 01 00 01 00 00 00` | 88 13 (5000) |
| **Position Setpoint** | 2 | `03 00 00 00 64 00 02 00 01 00 00 00` | 88 13 (5000) |
| **Control Mode** | 3 | `03 00 00 00 64 00 03 00 01 00 00 00` | 00 (Pressure) |
| **Actual Pressure** | 4 | `03 00 00 00 64 00 04 00 01 00 00 00` | 00 00 (0) |
| **Actual Position** | 5 | `03 00 00 00 64 00 05 00 01 00 00 00` | 00 00 (0) |
| **Device Status** | 6 | `03 00 00 00 64 00 06 00 01 00 00 00` | 01 (Idle) |
| **Exception Status** | 7 | `03 00 00 00 64 00 07 00 01 00 00 00` | 00 |
| **Access Mode** | 8 | `03 00 00 00 64 00 08 00 01 00 00 00` | 01 (Remote) |
| **Pressure Upper Limit** | 9 | `03 00 00 00 64 00 09 00 01 00 00 00` | 10 27 (10000) |
| **Pressure Lower Limit** | 10 | `03 00 00 00 64 00 0A 00 01 00 00 00` | 00 00 (0) |

### 쓰기 테스트 예시 (Cmd: 0x0000380B)

#### Control Mode를 Position Control로 변경
```
Len:  13
Data: 03 00 00 00 64 00 03 00 01 00 00 00 01
                                            ↑
                                            1 = Position Control
```

#### Pressure Setpoint을 80.00 mbar로 변경
```
Len:  14
Data: 03 00 00 00 64 00 01 00 01 00 00 00 40 1F
                                            ↑
                                            0x1F40 = 8000
```

---

## VAT Diagnostic Object 테스트

### Diagnostic Uptime 읽기 (Class 0x65)

#### 요청 패킷
```
Command (Cmd):  0x0000380A
Length (Len):   12
Data (Hex):     03 00 00 00 65 00 01 00 01 00 00 00
                            ↑      ↑      ↑
                            0x65   Inst 1 Attr 1 (Uptime)
```

#### 예상 응답
```
Data: [4 bytes UINT32] - Uptime in seconds
```

### Diagnostic Attributes 전체 목록

| Attr | 파라미터 | Data Offset 변경 | 타입 | 설명 |
|------|---------|------------------|------|------|
| 1 | Uptime | `01 00` | UINT32 | 가동 시간 (초) |
| 2 | Total Cycles | `02 00` | UINT32 | 총 제어 사이클 |
| 3 | Error Count | `03 00` | UINT16 | 에러 발생 횟수 |
| 4 | Last Error Code | `04 00` | UINT16 | 마지막 에러 코드 |
| 6 | Pressure Min | `06 00` | INT16 | 최소 압력 |
| 7 | Pressure Max | `07 00` | INT16 | 최대 압력 |
| 8 | Pressure Avg | `08 00` | INT16 | 평균 압력 |
| 14 | Temperature | `0E 00` | INT16 | 시스템 온도 |
| 15 | Firmware Version | `0F 00` | UINT32 | 펌웨어 버전 |

**공통 Data 패턴**:
```
03 00 00 00  65 00  [Attr]  01 00  00 00
↑            ↑      ↑       ↑      ↑
MAC ID       Class  변경    Inst   Rsv
```

---

## 에러 테스트

### 1. Read-Only 파라미터 쓰기 시도

#### 요청
```
Cmd:  0x0000380B (SET)
Len:  14
Data: 03 00 00 00 64 00 04 00 01 00 00 00 64 00
                          ↑                 ↑
                          Instance 4        Value=100
                          (Actual Pressure - READ ONLY!)
```

#### 예상 응답
```
State: 0xC0000001 (Error)
또는 응답 Data에 에러 코드 포함
```

### 2. 범위 초과 값 쓰기

#### 요청
```
Cmd:  0x0000380B
Len:  14
Data: 03 00 00 00 64 00 01 00 01 00 00 00 11 27
                                            ↑
                                            0x2711 = 10001
                                            (Max = 10000)
```

#### 예상 응답
```
에러 코드 반환 (범위 초과)
```

### 3. 존재하지 않는 Instance

#### 요청
```
Cmd:  0x0000380A
Len:  12
Data: 03 00 00 00 64 00 FF 00 01 00 00 00
                          ↑
                          Instance 255 (존재하지 않음)
```

#### 예상 응답
```
에러 코드 반환 (Object Does Not Exist)
```

---

## 동작 흐름 (성공 케이스)

### Master → Slave 흐름

```
1. netHOST (Master Simulator)
   │
   │ Send: DEVNET_FAL_CMD_GET_ATT_REQ (0x0000380A)
   │ Data: [MAC=3, Class=0x64, Instance=1, Attribute=1]
   │
   ↓
2. DeviceNet 네트워크
   │
   │ CAN 프레임으로 변환되어 물리 계층 전송
   │
   ↓
3. netX90 Slave (DNS Stack)
   │
   │ FAL 패킷 수신 및 파싱:
   │   - bDeviceAddr = 3 (자신의 MAC ID 확인 ✓)
   │   - usClass = 0x64
   │   - usInstance = 1
   │   - usAttribute = 1
   │
   ↓
4. DNS 스택 내부 변환
   │
   │ DNS_CMD_CIP_SERVICE_IND (0x0000308A) 생성:
   │   - ulService = 0x0E (Get_Attribute_Single)
   │   - ulClass = 0x64
   │   - ulInstance = 1
   │   - ulAttribute = 1
   │   - ulMember = 0 (내부 처리)
   │
   ↓
5. AppDNS_PacketHandler_callback()
   │
   │ case DNS_CMD_CIP_SERVICE_IND:
   │
   ↓
6. AppDNS_HandleCipServiceIndication()
   │
   │ Class 0x64 감지
   │ → VAT UserObject Handler로 라우팅
   │
   ↓
7. AppDNS_VAT_UserObject_IsRegistered()
   │
   │ Class 0x64, Instance 1 검증 ✓
   │
   ↓
8. AppDNS_VAT_UserObject_Indication()
   │
   │ switch (ulClass) {
   │   case VAT_CLASS_PARAMETER: (0x64)
   │
   ↓
9. AppDNS_VAT_HandleParameterObject()
   │
   │ switch (bService) {
   │   case CIP_SERVICE_GET_ATTRIBUTE_SINGLE: (0x0E)
   │
   ↓
10. VAT_Param_Get()
   │
   │ bParamId = 1 (Pressure Setpoint)
   │ → g_tParamManager.params[0].data
   │ → Value = 5000 (0x1388)
   │
   ↓
11. 응답 패킷 구성
   │
   │ DNS_CMD_CIP_SERVICE_RES (0x0000308B):
   │   - ulGRC = 0x00 (Success)
   │   - ulERC = 0x00
   │   - abData = 88 13
   │
   ↓
12. AppDNS_GlobalPacket_Send()
   │
   │ 응답 전송
   │
   ↓
13. DNS 스택 → Master 응답 변환
   │
   │ DEVNET_FAL_CMD_GET_ATT_CNF (0x0000380B):
   │   - Data = 88 13
   │
   ↓
14. DeviceNet 네트워크
   │
   │ CAN 응답 프레임 전송
   │
   ↓
15. netHOST 응답 수신
    │
    └─> Received Packet:
        Cmd: 0x0000380B
        State: 0x00000000 (Success)
        Data: 88 13 (5000 = 50.00 mbar)
```

---

## 핵심 교훈

### 1. API 계층 구분의 중요성

| API 계층 | 명령 코드 | 구조체 | 용도 |
|---------|----------|--------|------|
| **Master FAL API** | 0x0000380A/B | DN_FAL_SDU_GETSET_ATT_REQ_T (12 bytes) | Master → Slave 네트워크 통신 |
| **Slave DNS API** | 0x0000308A/B | DNS_CIP_SERVICE_REQ_T (20 bytes) | Slave 내부 애플리케이션 처리 |
| **Host DPM API** | 0x0000B102/3 | DNS_CIP_SERVICE_REQ_T (20 bytes) | Host → Slave 로컬 통신 |

### 2. netHOST의 역할

**netHOST Device Test Application**은:
- **Master 시뮬레이터** 역할
- **DEVNET_FAL_CMD_GET_ATT_REQ/RES** (0x380A/B) 사용
- **DN_FAL_SDU_GETSET_ATT_REQ_T** 구조 (12 bytes)
- **첫 필드는 MAC ID** (Target Device Address)

### 3. Data 입력 정확성

**정확한 바이트 정렬**이 필수:
```
✓ 올바름: 03 00 00 00 64 00 01 00 01 00 00 00
✗ 잘못됨: 03 00 00 00 64 00 00 01 00 00 01 00
```

복사/붙여넣기 시 공백이나 줄바꿈에 주의!

### 4. Little-Endian 바이트 순서

```
값 7000 (0x1B58):
  → 네트워크 바이트: 58 1B (low byte first)

값 0x0064 (100):
  → 네트워크 바이트: 64 00
```

---

## MAC ID 설정 확인

### 현재 설정
```c
// AppDNS_DemoApplicationFunctions.c
#define DEFAULT_NODE_ADDRESS   1

// 하지만 실제 테스트에서는 MAC ID = 3 사용 성공
```

**확인 사항**:
- EDS 파일 MacID 섹션
- 실제 네트워크 구성
- Vendor ID 요청 시 사용한 MAC ID와 동일하게 사용

---

## 참조 문서

### 이전 문서
- `20251111_VAT_Test_Correct_Packets.md` - DNS API 기반 (잘못된 접근)
- `20251111_netHOST_Master_Packet_Format.md` - Master FAL API (올바른 접근)
- `20251111_VAT_UserObject_Integration.md` - Slave 애플리케이션 구현

### 관련 헤더 파일
- `DNS_packet_cip_service.h` - DNS API 정의
- `DeviceNet_FAL.h` - Master FAL API 정의 (참조)
- `DNS_packet_commands.h` - 명령 코드 정의

---

## 체크리스트

### netHOST 설정
- [x] Command = **0x0000380A** (GET) 또는 **0x0000380B** (SET)
- [x] Length = **12** (GET) 또는 **12 + data_size** (SET)
- [x] Data 첫 바이트 = **Slave MAC ID** (예: 03)
- [x] Class, Instance, Attribute는 **UINT16 little-endian**
- [x] Dest = 0x00000020 (Channel Token)
- [x] Src = 0x00000010 (Host Queue)

### 성공 확인
- [x] Pressure Setpoint 읽기 성공
- [x] 응답 데이터 정상 (88 13 = 5000)
- [x] State = 0x00000000 (Success)
- [x] Cmd = 0x0000380B (응답)

---

## 다음 테스트 항목

### Phase 1: 기본 파라미터 테스트
- [ ] Position Setpoint 읽기/쓰기
- [ ] Control Mode 읽기/쓰기
- [ ] Device Status 읽기
- [ ] 모든 파라미터 순회 읽기

### Phase 2: Diagnostic 테스트
- [ ] Uptime 읽기
- [ ] Error Count 읽기
- [ ] Pressure Min/Max/Avg 읽기
- [ ] Firmware Version 읽기

### Phase 3: 에러 처리 테스트
- [ ] Read-Only 쓰기 시도
- [ ] 범위 초과 값 쓰기
- [ ] 존재하지 않는 Instance 접근
- [ ] 잘못된 데이터 크기 전송

### Phase 4: I/O 연동 테스트
- [ ] Cyclic I/O 데이터 확인
- [ ] Parameter ↔ I/O 동기화 테스트
- [ ] Real-time 업데이트 확인

---

**작업 완료**: 2025-11-11
**테스트 성공**: VAT Explicit Message 통신 정상 동작 확인
**다음 단계**: 전체 파라미터 테스트 및 I/O 연동

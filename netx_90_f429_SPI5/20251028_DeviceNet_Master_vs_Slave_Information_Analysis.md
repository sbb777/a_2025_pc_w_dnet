# DeviceNet 마스터 vs 슬레이브 정보 분석

**작업 일시**: 2025-10-28
**작성자**: Claude Code
**목적**: 현재 코드가 조회하는 정보가 마스터인지 슬레이브인지 분석하고, 실제 마스터 정보를 얻는 방법 제시

---

## 핵심 질문

**Q**: 현재 출력되는 펌웨어 정보가 DeviceNet 마스터의 정보인가, 슬레이브의 정보인가?

**A**: **슬레이브(netX90 모듈)의 정보**입니다.

---

## DeviceNet 네트워크 구조

```
┌─────────────────────┐
│  DeviceNet Master   │  ← PLC, Scanner (Rockwell, Siemens 등)
│     (Scanner)       │     실제 제어를 수행하는 마스터
└──────────┬──────────┘
           │
           │ DeviceNet Bus (CAN)
           │ (125k/250k/500k bps)
           │
┌──────────┴──────────┐
│  netX90 Module      │  ← Hilscher DeviceNet Slave
│  (DeviceNet Slave)  │     STM32에 연결된 통신 모듈
└──────────┬──────────┘
           │
           │ SPI Interface
           │
┌──────────┴──────────┐
│   STM32F429         │  ← 호스트 MCU
│   (Host)            │     애플리케이션 실행
└─────────────────────┘
```

**역할**:
- **Master (Scanner)**: DeviceNet 네트워크 제어, I/O 스캔, 진단
- **Slave (netX90)**: Master의 명령 수행, I/O 데이터 교환
- **Host (STM32)**: 애플리케이션 로직, netX90과 데이터 교환

---

## 현재 코드가 조회하는 정보

### 코드 분석

```c
CHANNEL_INFORMATION tChannelInfo;
lRet = xChannelInfo(hChannel, sizeof(CHANNEL_INFORMATION), &tChannelInfo);

printf("  Firmware Name:   %.*s\r\n",
       tChannelInfo.bFWNameLength,
       tChannelInfo.abFWName);
```

### 조회되는 정보

**`xChannelInfo()`가 반환하는 정보**:
- netX90 칩에 로드된 펌웨어의 정보
- STM32에 **직접 연결된 통신 모듈(netX90)**의 정보
- DeviceNet **슬레이브로 동작하는 모듈**의 펌웨어

**예상 출력**:
```
[Firmware Information]
  Firmware Name:   DeviceNet Slave V5.7.0.0
  FW Version:      5.7.0.0
  FW Build Date:   2024-03-15
```

**의미**:
- `"DeviceNet Slave"` → netX90이 슬레이브 역할 수행
- `V5.7.0.0` → Hilscher DeviceNet 슬레이브 펌웨어 버전
- 이것은 **netX90 모듈 자체의 정보**

---

## 마스터 vs 슬레이브 정보 비교

| 항목 | 마스터 (Master/Scanner) | 슬레이브 (netX90) |
|------|------------------------|------------------|
| **위치** | DeviceNet 네트워크 상 | STM32에 직접 연결 |
| **역할** | 네트워크 제어, I/O 스캔 | Master 명령 수행 |
| **제조사** | Rockwell, Siemens, Omron 등 | Hilscher |
| **펌웨어** | Master Scanner 펌웨어 | DeviceNet Slave 펌웨어 |
| **정보 조회** | Explicit Messaging 필요 | `xChannelInfo()` 사용 |
| **현재 코드** | ❌ 조회 안 됨 | ✅ 조회 가능 |

---

## 왜 슬레이브 정보만 조회되는가?

### 1. 통신 계층 구조

```
┌─────────────────────────────────────────┐
│  STM32 Application (Host)               │
├─────────────────────────────────────────┤
│  cifX Toolkit API                       │  ← xChannelInfo()
├─────────────────────────────────────────┤
│  SPI Communication Layer                │
├─────────────────────────────────────────┤
│  netX90 Firmware (DeviceNet Slave)      │  ← 이 정보를 조회
├─────────────────────────────────────────┤
│  DeviceNet Bus (CAN)                    │
├─────────────────────────────────────────┤
│  DeviceNet Master (Scanner)             │  ← 직접 접근 불가
└─────────────────────────────────────────┘
```

**`xChannelInfo()`는**:
- STM32와 netX90 사이의 로컬 통신
- netX90의 펌웨어 정보를 DPM(Dual Port Memory)에서 읽음
- DeviceNet 네트워크를 거치지 않음

---

### 2. DeviceNet I/O Connection

**일반적인 DeviceNet I/O 연결**:

```
Master → Slave: Output Data (주기적, 제어 명령)
Slave → Master: Input Data (주기적, 센서 데이터)
```

**특징**:
- 주기적인 데이터만 교환
- Master의 Identity 정보는 포함되지 않음
- 빠른 실시간 I/O에 최적화

---

## DeviceNet 마스터 정보를 얻는 방법

### 방법 1: Explicit Messaging (CIP Services)

DeviceNet은 CIP(Common Industrial Protocol)를 사용하며, Explicit Messaging으로 디바이스 정보를 조회할 수 있습니다.

#### CIP Identity Object (Class 0x01)

**Object 구조**:
```
Class ID: 0x01 (Identity Object)
Instance: 1 (첫 번째 인스턴스)

Attributes:
  1. Vendor ID (UINT)
  2. Device Type (UINT)
  3. Product Code (UINT)
  4. Revision (Major.Minor)
  5. Status (WORD)
  6. Serial Number (UDINT)
  7. Product Name (SHORT_STRING)  ← 원하는 정보!
```

#### Get_Attribute_Single 서비스

**서비스 코드**: 0x0E

**요청 패킷**:
```
Service: 0x0E (Get_Attribute_Single)
Class: 0x01 (Identity)
Instance: 0x01
Attribute: 0x07 (Product Name)
```

---

### 구현 예시 코드

```c
/**
 * @brief DeviceNet 마스터의 Identity 정보 조회
 *
 * @param hChannel      cifX 채널 핸들
 * @param masterNodeId  마스터 노드 주소 (일반적으로 0 또는 63)
 * @param pProductName  제품 이름 저장 버퍼
 * @param bufferSize    버퍼 크기
 *
 * @return CIFX_NO_ERROR on success
 */
int32_t GetMasterIdentity(CIFXHANDLE hChannel,
                          uint8_t masterNodeId,
                          char* pProductName,
                          uint32_t bufferSize)
{
    int32_t lRet;
    CIFX_PACKET tSendPkt;
    CIFX_PACKET tRecvPkt;

    /* CIP Explicit Message 패킷 구성 */
    memset(&tSendPkt, 0, sizeof(tSendPkt));

    /* 패킷 헤더 설정 */
    tSendPkt.tHeader.ulDest = 0x20;  /* 채널 mailbox */
    tSendPkt.tHeader.ulSrc = 0x10;   /* 호스트 */
    tSendPkt.tHeader.ulLen = 8;      /* 데이터 길이 */
    tSendPkt.tHeader.ulCmd = 0x1234; /* DeviceNet Explicit Message 명령 */

    /* CIP Get_Attribute_Single 데이터 */
    tSendPkt.abData[0] = 0x0E;       /* Service: Get_Attribute_Single */
    tSendPkt.abData[1] = 0x02;       /* Path size (words) */
    tSendPkt.abData[2] = 0x20;       /* Class (8-bit) */
    tSendPkt.abData[3] = 0x01;       /* Class ID: Identity */
    tSendPkt.abData[4] = 0x24;       /* Instance (8-bit) */
    tSendPkt.abData[5] = 0x01;       /* Instance: 1 */
    tSendPkt.abData[6] = 0x30;       /* Attribute (8-bit) */
    tSendPkt.abData[7] = 0x07;       /* Attribute: Product Name */

    /* 패킷 전송 */
    lRet = xChannelPutPacket(hChannel, &tSendPkt, 5000);
    if(lRet != CIFX_NO_ERROR)
    {
        printf("Failed to send packet: 0x%08X\r\n", (unsigned int)lRet);
        return lRet;
    }

    /* 응답 수신 */
    lRet = xChannelGetPacket(hChannel, sizeof(tRecvPkt), &tRecvPkt, 5000);
    if(lRet != CIFX_NO_ERROR)
    {
        printf("Failed to receive packet: 0x%08X\r\n", (unsigned int)lRet);
        return lRet;
    }

    /* 응답 파싱 */
    if(tRecvPkt.abData[0] == 0x8E)  /* Get_Attribute_Single Response */
    {
        uint8_t nameLen = tRecvPkt.abData[2];  /* String length */
        if(nameLen > 0 && nameLen < bufferSize)
        {
            memcpy(pProductName, &tRecvPkt.abData[3], nameLen);
            pProductName[nameLen] = '\0';

            printf("Master Product Name: %s\r\n", pProductName);
            return CIFX_NO_ERROR;
        }
    }

    return CIFX_FUNCTION_FAILED;
}
```

---

### 사용 방법

```c
char szMasterName[64];
int32_t lRet;

/* Master 정보 조회 (Node ID = 0 가정) */
lRet = GetMasterIdentity(hChannel, 0, szMasterName, sizeof(szMasterName));

if(lRet == CIFX_NO_ERROR)
{
    printf("\r\n[Master Information]\r\n");
    printf("  Product Name: %s\r\n", szMasterName);
}
else
{
    printf("Failed to get master information\r\n");
    printf("  Master may not support Explicit Messaging\r\n");
    printf("  Or master node ID is different\r\n");
}
```

---

## 방법 2: DeviceNet 설정 파일에서 확인

### EDS (Electronic Data Sheet) 파일

Master는 보통 EDS 파일을 통해 Slave를 인식합니다.

**Master의 EDS 파일 예시**:
```ini
[Device]
VendorName="Rockwell Automation/Allen-Bradley"
VendorCode=1
ProductName="1756-DNB DeviceNet Scanner"
ProductCode=52
MajorRev=4
MinorRev=12
```

**확인 방법**:
- Master의 설정 소프트웨어에서 확인
- RSNetWorx for DeviceNet (Rockwell)
- SYCON.net (Hilscher)
- 기타 네트워크 스캐닝 도구

---

### 방법 3: 네트워크 스캔

**DeviceNet Network Scanner**:
```
Device List:
  Node 0: Master Scanner (Allen-Bradley 1756-DNB)
  Node 10: Slave Device (VAT Pressure Controller)
  Node 15: Slave Device (Hilscher netX90)
```

**확인 도구**:
- RSNetWorx for DeviceNet
- SYCON.net
- CIP Tool
- Wireshark (DeviceNet 플러그인)

---

## 실제 적용 시 고려사항

### 1. Master Node ID 확인

**일반적인 Node ID**:
- `0`: Master (가장 일반적)
- `63`: Master (일부 시스템)
- `1-62`: Slave devices

**확인 방법**:
```c
// 네트워크 스캔으로 Master 찾기
for(uint8_t nodeId = 0; nodeId < 64; nodeId++)
{
    if(ScanNode(hChannel, nodeId) == MASTER_TYPE)
    {
        printf("Master found at Node ID: %u\r\n", nodeId);
        break;
    }
}
```

---

### 2. Explicit Messaging 지원 여부

**Master가 지원해야 하는 것**:
- Unconnected Explicit Messaging
- CIP Identity Object 접근 허용
- Get_Attribute_Single 서비스

**확인**:
- Master의 사양서 확인
- EDS 파일의 Explicit Messaging 설정 확인

---

### 3. 타임아웃 설정

**Explicit Message는 I/O보다 느림**:
```c
#define EXPLICIT_MSG_TIMEOUT    5000  // 5초
#define IO_COMMUNICATION_TIMEOUT 100  // 100ms
```

**이유**:
- I/O: 주기적, 우선순위 높음
- Explicit: 비주기적, 우선순위 낮음

---

## 현재 시스템에서의 정보 확인

### 현재 확인 가능한 정보

#### 1. netX90 슬레이브 정보 (현재 구현됨)

```c
xChannelInfo(hChannel, sizeof(CHANNEL_INFORMATION), &tChannelInfo);

// 출력 예시
Firmware Name:   DeviceNet Slave V5.7.0.0
FW Version:      5.7.0.0
Board Name:      cifX0
Device Number:   12345
Serial Number:   67890
```

**의미**: STM32에 연결된 netX90 모듈의 정보

---

#### 2. VAT 컨트롤러 정보 (DeviceNet 슬레이브)

```c
// EDS 파일에서 확인 (476297.eds)
Vendor: VAT Vakuumventile AG (Code 404)
Product: VAT Adaptive Pressure Controller
Product Code: 650
Node Address: 10 (설정 가능)
```

**의미**: DeviceNet 버스에 연결된 VAT 디바이스

---

#### 3. DeviceNet Master 정보 (조회 필요)

**현재 상태**: ❌ 구현 안 됨

**필요한 작업**:
1. Explicit Messaging 구현
2. Master Node ID 확인
3. CIP Get_Attribute_Single 패킷 송신
4. 응답 파싱 및 출력

---

## 권장 사항

### 일반적인 사용 케이스

**대부분의 DeviceNet 애플리케이션**:
- Slave는 Master의 정보를 알 필요가 없음
- Master가 네트워크를 제어하고 관리
- Slave는 주어진 I/O 데이터만 처리

**정보 확인이 필요한 경우**:
- 네트워크 진단 도구 개발
- 멀티 마스터 시스템
- 고급 로깅 및 모니터링

---

### 실제 운영 환경

**추천 방법**:

1. **설정 단계** (오프라인):
   - RSNetWorx 등 설정 도구에서 Master 확인
   - EDS 파일로 디바이스 설정
   - 네트워크 토폴로지 문서화

2. **실행 단계** (온라인):
   - Slave는 I/O 데이터에만 집중
   - Master 정보는 필요시 Explicit Message 사용
   - 진단은 Master의 진단 기능 활용

3. **디버깅 단계**:
   - 현재 구현처럼 netX90 정보 로깅
   - DeviceNet 네트워크 스캐너 사용
   - CAN 버스 분석기로 패킷 캡처

---

## 코드 수정 제안

### Option 1: Master 정보 조회 기능 추가 (복잡함)

```c
static void PrintDeviceInformation(CIFXHANDLE hDriver, CIFXHANDLE hChannel)
{
    // 기존 코드 유지 (netX90 슬레이브 정보)

    // Master 정보 조회 추가
    printf("\r\n[Master Information]\r\n");
    char szMasterName[64];
    if(GetMasterIdentity(hChannel, 0, szMasterName, sizeof(szMasterName)) == CIFX_NO_ERROR)
    {
        printf("  Product Name: %s\r\n", szMasterName);
    }
    else
    {
        printf("  Master information not available\r\n");
        printf("  (Explicit Messaging may not be supported)\r\n");
    }
}
```

---

### Option 2: 명확한 레이블링 (간단함, 추천)

```c
static void PrintDeviceInformation(CIFXHANDLE hDriver, CIFXHANDLE hChannel)
{
    // ... 기존 코드 ...

    printf("\r\n[DeviceNet Slave Module (netX90) Information]\r\n");
    printf("  This is the information of netX90 module, NOT the Master\r\n");
    printf("  Firmware Name:   %.*s\r\n",
           tChannelInfo.bFWNameLength,
           tChannelInfo.abFWName);
    printf("  FW Version:      %u.%u.%u.%u\r\n",
           tChannelInfo.usFWMajor,
           tChannelInfo.usFWMinor,
           tChannelInfo.usFWBuild,
           tChannelInfo.usFWRevision);

    printf("\r\n[Note]\r\n");
    printf("  To get Master information, use network configuration tool\r\n");
    printf("  or implement CIP Explicit Messaging\r\n");
}
```

---

## 요약 및 결론

### 현재 상황

| 질문 | 답변 |
|------|------|
| 현재 출력되는 정보는? | netX90 슬레이브 모듈의 정보 |
| "DeviceNet Slave"는? | netX90이 슬레이브로 동작함을 의미 |
| Master 정보가 필요한가? | 일반적으로 불필요, 특수 목적만 필요 |
| Master 정보를 어떻게? | Explicit Messaging 또는 설정 도구 |

---

### 권장 조치

#### 일반 사용자 (I/O 통신만 필요)

**현재 코드로 충분**:
- ✅ netX90 모듈 정보 확인 가능
- ✅ 통신 상태 확인 가능
- ✅ 펌웨어 버전 확인 가능

**Master 정보**:
- 설정 도구(RSNetWorx 등)에서 확인
- 네트워크 문서에 기록

---

#### 고급 사용자 (진단 도구 개발)

**Master 정보 조회 구현**:
1. CIP Explicit Messaging 구현
2. Identity Object 읽기
3. Master Node ID 스캔
4. Product Name 파싱 및 출력

**참고 자료**:
- ODVA CIP Specification
- DeviceNet Specification Volume I & II
- Hilscher DeviceNet Manual

---

## 관련 문서

- `20251028_DeviceNet_Master_Device_Info_Display.md` - 디바이스 정보 출력 구현
- `20251028_DeviceNet_Communication_Log_Analysis.md` - 통신 로그 분석
- `20251027_DNS_V5_EDS_Analysis.md` - EDS 파일 분석
- DeviceNet Specification Volume I (CIP on DeviceNet)
- CIP Common Specification

---

## 변경 이력

| 날짜 | 버전 | 변경 내용 | 작성자 |
|------|------|----------|--------|
| 2025-10-28 | 1.0 | 초기 작성: 마스터 vs 슬레이브 정보 분석 | Claude Code |

---

## 최종 답변

### Q: 펌웨어 이름에 "Slave"라고 찍히는 것은 무엇인가?

**A**: netX90 모듈 자체가 DeviceNet 슬레이브로 동작하기 때문입니다.

```
netX90 Firmware: "DeviceNet Slave V5.7.0.0"
                  ^^^^^^^^^^^^
                  이것은 역할을 나타냄 (Slave)
```

---

### Q: Master의 정보를 얻으려면?

**A**: 두 가지 방법이 있습니다.

**방법 1** (간단): 네트워크 설정 도구 사용
- RSNetWorx for DeviceNet
- SYCON.net
- 네트워크 스캐너

**방법 2** (복잡): CIP Explicit Messaging 구현
- Get_Attribute_Single 서비스
- Identity Object 조회
- 코드 수정 필요

---

### Q: 일반적으로 Master 정보가 필요한가?

**A**: 대부분의 경우 **불필요**합니다.

**이유**:
- Slave는 Master의 명령만 수행
- Master가 네트워크 관리
- I/O 데이터 교환이 주 목적

**필요한 경우**:
- 네트워크 진단 도구 개발
- 고급 로깅 시스템
- 멀티 마스터 환경

---

## 결론

현재 `PrintDeviceInformation()` 함수는 **netX90 슬레이브 모듈의 정보**를 출력하며, 이는 정상적인 동작입니다. DeviceNet 마스터의 정보가 필요한 경우, CIP Explicit Messaging을 구현하거나 네트워크 설정 도구를 사용하는 것을 권장합니다.

대부분의 DeviceNet 애플리케이션에서는 슬레이브 모듈 정보만으로 충분하며, 마스터 정보는 네트워크 설정 단계에서 별도로 관리하는 것이 일반적입니다.

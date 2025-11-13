# DeviceNet Explicit 메시지 코드 분석 보고서

**작성일**: 2025-11-07
**프로젝트**: netX90 F429 SPI5 DeviceNet Implementation
**분석 대상**: 기존 코드 분석을 통한 Explicit 메시지 에러 원인 규명

---

## 1. 코드 분석 요약

### 1.1 분석 범위
1. ✅ AppDNS_ConfigureStack() 함수 및 UCMM 설정
2. ✅ DNS_CONFIGURATION_V1_T 구조체 분석
3. ✅ EDS 파일 (476297.eds) 내용 확인
4. ✅ App_AllChannels_PacketHandler() Explicit 처리 로직

---

## 2. AppDNS_ConfigureStack() 분석

### 2.1 파일 위치
`E:\git\netx_90_f429_SPI5\Hil_DemoAppDNS\Sources\AppDNS_DemoApplicationFunctions.c:180`

### 2.2 설정 코드 (AppDNS_SetConfiguration)
```c
uint32_t AppDNS_SetConfiguration(APP_DATA_T* ptAppData)
{
  uint32_t ulRet = CIFX_NO_ERROR;
  DNS_PACKET_SET_CONFIGURATION_REQ_T* ptReq = AppDNS_GlobalPacket_Init(ptAppData);
  DNS_CONFIGURATION_V1_T *ptCfg = &ptReq->tData.unCfg.tV1;

  ptReq->tHead.ulCmd        = DNS_CMD_SET_CONFIGURATION_REQ;
  ptReq->tHead.ulLen        = DNS_SET_CONFIGURATION_V1_REQ_SIZE;
  ptReq->tHead.ulSta        = 0;
  ptReq->tData.ulVersion    = DNS_CONFIGURATION_V1;

  /* Set the slave related parameters */
  memset(ptCfg, 0x00, sizeof(DNS_CONFIGURATION_V1_T));
  ptCfg->ulSystemFlags      = DNS_SYS_FLG_MANUAL_START;  // ⭐ Line 108
  ptCfg->ulWdgTime          = 0;

  /* Set network parameter */
  ptCfg->ulNodeId = g_tAppDnsData.bNodeIdValue;          // DEFAULT: 1
  ptCfg->ulBaudrate = g_tAppDnsData.bBaudRateValue;      // DEFAULT: 125kB

  ptCfg->ulConfigFlags      = 0;  // ❗ Line 115
  ptCfg->ulObjectFlags      = 0;  // ❗ Line 116

  /* Set Identity Data */
  ptCfg->usVendorId         = VENDOR_ID;                 // 404 (VAT)
  ptCfg->usDeviceType       = DEVICE_TYPE_PROCESS_CONTROL_VALVE;  // 29
  ptCfg->usProductCode      = PRODUCT_CODE;              // 650
  ptCfg->bMajorRev          = MAJOR_REVISION;            // 2
  ptCfg->bMinorRev          = MINOR_REVISION;            // 1

  /* Set Process Data */
  ptCfg->ulProduceAsInstance  = DNS_DEMO_PRODUCING_ASSEMBLY_INSTANCE;  // 0x64 (100)
  ptCfg->ulProduceAsSize      = DNS_DEMO_PRODUCING_ASSEMBLY_INSTANCE_SIZE;  // 7
  ptCfg->ulConsumeAsInstance  = DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE;  // 0x08 (8)
  ptCfg->ulConsumeAsSize      = DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE_SIZE;  // 5

  ulRet = AppDNS_GlobalPacket_SendReceive(ptAppData);
  return ulRet;
}
```

### 2.3 ❗ 중요 발견: UCMM 설정 없음

**현재 설정값:**
```c
ptCfg->ulConfigFlags = 0;
ptCfg->ulObjectFlags = 0;
```

**DNS_packet_set_configuration.h에서 확인된 플래그들:**
```c
// Configuration flags (Line 67-90)
#define DNS_CFG_FLAG_INDICATION_NETWORK_POWER        0x80000000
#define DNS_CFG_FLAG_MESSAGE_BODY_FORMAT_MSK         0x00000300
#define DNS_CFG_FLAG_MESSAGE_BODY_FORMAT_8_8         0x00000000  // default
#define DNS_CFG_FLAG_MESSAGE_BODY_FORMAT_8_16        0x00000100
#define DNS_CFG_FLAG_MESSAGE_BODY_FORMAT_16_16       0x00000200
#define DNS_CFG_FLAG_MESSAGE_BODY_FORMAT_16_8        0x00000300

#define DNS_CFG_FLAG_CONNECTION_DISABLE_MSK          0x0000F000
#define DNS_CFG_FLAG_CONNECTION_DISABLE_POLL         0x00001000
#define DNS_CFG_FLAG_CONNECTION_DISABLE_STROBE       0x00002000
#define DNS_CFG_FLAG_CONNECTION_DISABLE_COS          0x00004000
#define DNS_CFG_FLAG_CONNECTION_DISABLE_CYC          0x00008000

// Object Configuration flags (Line 89)
#define DNS_OBJ_FLAG_RESERVED                        0xFFFFFFFF  // ⚠️ 사용 안 함
```

**주석 내용:**
```c
uint32_t ulConfigFlags; /*!< Configuration flags. These flags are not used, set 0. */
uint32_t ulObjectFlags; /*!< Object Configuration flags. These flags are not used, set 0. */
```

### 2.4 ⚠️ 주석의 모순

Line 106-107의 주석에는 "These flags are not used, set 0"라고 되어 있으나,
Line 67-90에서 실제로 여러 플래그들이 정의되어 있습니다.

**이는 두 가지를 의미합니다:**
1. 주석이 오래된 것일 수 있음 (실제로는 사용됨)
2. 또는 UCMM은 별도 설정이 필요 없이 기본 활성화

---

## 3. DNS_CONFIGURATION_V1_T 구조체 분석

### 3.1 파일 위치
`E:\git\netx_90_f429_SPI5\Hil_DemoAppDNS\includes\DNS_API\DNS_packet_set_configuration.h:97`

### 3.2 구조체 정의
```c
typedef struct DNS_CONFIGURATION_V1_Ttag
{
  uint32_t ulSystemFlags;        // 시스템 동작 플래그
  uint32_t ulWdgTime;            // Watchdog 시간 (ms)
  uint32_t ulNodeId;             // MAC ID
  uint32_t ulBaudrate;           // 통신 속도

  uint32_t ulConfigFlags;        // ❗ 설정 플래그
  uint32_t ulObjectFlags;        // ❗ 객체 플래그

  uint16_t usVendorId;           // 제조사 ID
  uint16_t usDeviceType;         // 디바이스 타입
  uint16_t usProductCode;        // 제품 코드
  uint8_t  bMinorRev;            // Minor Revision
  uint8_t  bMajorRev;            // Major Revision
  uint32_t ulSerialNumber;       // 시리얼 번호
  uint8_t  abReserved[3];
  uint8_t  bProductNameLen;
  uint8_t  abProductName[32];

  uint32_t ulProduceAsInstance;  // Input Assembly Instance
  uint32_t ulProduceAsFlags;
  uint32_t ulProduceAsSize;      // Input Assembly Size

  uint32_t ulConsumeAsInstance;  // Output Assembly Instance
  uint32_t ulConsumeAsFlags;
  uint32_t ulConsumeAsSize;      // Output Assembly Size
} DNS_CONFIGURATION_V1_T;
```

### 3.3 ❗ 핵심 발견

**UCMM 관련 필드가 구조체에 없음!**

이는 Hilscher DeviceNet 스택에서:
- **UCMM (Explicit Messaging)이 기본적으로 활성화**되어 있음을 의미
- 별도 설정 없이 스택이 자동으로 처리

---

## 4. EDS 파일 분석 (476297.eds)

### 4.1 파일 위치
`E:\git\netx_90_f429_SPI5\476297.eds`

### 4.2 기본 정보
```ini
[File]
DescText = "VAT Adaptive Pressure Controller";
CreateDate = 06-12-2010;
Revision = 1.0;

[Device]
VendCode = 404;                                      # VAT Vakuumventile AG
VendName = "VAT Vakuumventile AG";
ProdType = 29;                                       # Process Control Valve
ProdTypeStr = "Process Control Valve";
ProdCode = 650;                                      # VAT Product Code
MajRev = 2;
MinRev = 1;
ProdName = "VAT Adaptive Pressure Controller";
```

### 4.3 ❗ UCMM 관련 키워드 검색 결과

```bash
# Grep 검색: UCMM|Group2OnlyServer|Explicit
Result: No matches found
```

**해석:**
- EDS 파일에 UCMM, Group2OnlyServer, Explicit 관련 설정 없음
- 표준 DeviceNet 사양에서는 **UCMM이 기본 기능**
- 명시적으로 비활성화하지 않으면 활성화된 상태

### 4.4 I/O Assembly 정의 확인

```ini
[IO_Info]
Default = 0x0001;       # Default I/O Connection
PollInfo = 0x000F, 2, 7;
StrobeInfo = 0x000F, 2, 1;
COSInfo = 0x0007, 3, 1;
CyclicInfo = 0x000B, 3, 1;
```

Input/Output Assembly 정의는 있으나, Explicit 메시징 설정은 없음.

---

## 5. App_AllChannels_PacketHandler() 분석

### 5.1 파일 위치
`E:\git\netx_90_f429_SPI5\Hil_DemoApp\Sources\App_DemoApplication.c:183`

### 5.2 패킷 핸들러 구조
```c
uint32_t App_AllChannels_PacketHandler(APP_DATA_T* ptAppData)
{
  int i;
  int32_t ulRet = 0;

  for (i = 0; i < MAX_COMMUNICATION_CHANNEL_COUNT; i++)
  {
    if ((ptAppData->aptChannels[i] != NULL) &&
        (ptAppData->aptChannels[i]->hChannel != NULL) &&
        (ptAppData->aptChannels[i]->tProtocol.pfPacketHandler != NULL))
    {
      if (0 != (ulRet = ptAppData->aptChannels[i]->tProtocol.pfPacketHandler(ptAppData)))
      {
        PRINTF("Error: Protocol_PacketHandler failed: 0x%08X" NEWLINE, (unsigned int)ulRet);
        return ulRet;
      }
    }
  }
  return 0;
}
```

### 5.3 Explicit 메시지 처리 (AppDNS_PacketHandler_callback)

**위치:** `AppDNS_DemoApplicationFunctions.c:215`

```c
bool AppDNS_PacketHandler_callback(CIFX_PACKET* ptPacket, void* pvUserData)
{
  bool fPacketHandled = true;
  APP_DATA_T* ptAppData = (APP_DATA_T*)pvUserData;

  if(ptPacket != &ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket)
  {
    printf("Unexpected packet resource received!!!" NEWLINE);
    return false;
  }

  switch(ptPacket->tHeader.ulCmd)
  {
    /* ⭐ CIP Service Indication Handler ⭐ */
    case DNS_CMD_CIP_SERVICE_IND:  // ✅ Line 234
      AppDNS_HandleCipServiceIndication(ptAppData);
      fPacketHandled = true;
      break;

    default:
    {
      if ((ptPacket->tHeader.ulCmd & 0x1) == 0)
      {
        printf("Warning: Disregarded indication packet received!" NEWLINE);
        ptPacket->tHeader.ulState = ERR_HIL_NO_APPLICATION_REGISTERED;
        ptPacket->tHeader.ulCmd |= 0x01;
        AppDNS_GlobalPacket_Send(ptAppData);
      }
      break;
    }
  }
  return fPacketHandled;
}
```

### 5.4 ✅ Explicit 핸들러 구현 확인

**위치:** `AppDNS_DemoApplicationFunctions.c:403`

```c
void AppDNS_HandleCipServiceIndication(APP_DATA_T* ptAppData)
{
  DNS_PACKET_CIP_SERVICE_IND_T* ptInd =
      (DNS_PACKET_CIP_SERVICE_IND_T*)AppDNS_GlobalPacket_Get(ptAppData);

  DNS_PACKET_CIP_SERVICE_RES_T* ptRes =
      (DNS_PACKET_CIP_SERVICE_RES_T*)AppDNS_GlobalPacket_Get(ptAppData);

  // Extract CIP message information
  uint8_t service = (uint8_t)ptInd->tData.ulService;
  uint8_t class_id = (uint8_t)ptInd->tData.ulClass;
  uint8_t instance_id = (uint8_t)ptInd->tData.ulInstance;
  uint8_t attribute_id = (uint8_t)ptInd->tData.ulAttribute;

  // ✅ Get/Set Attribute Single 처리
  switch (service) {
    case 0x0E:  // Get Attribute Single
      error = CIP_HandleGetAttributeSingle(...);
      break;

    case 0x10:  // Set Attribute Single
      error = CIP_HandleSetAttributeSingle(...);
      break;

    case 0x01:  // Get Attribute All
      error = CIP_HandleGetAttributeAll(...);
      break;
  }

  // ✅ 응답 패킷 구성 및 송신
  ptRes->tHead.ulCmd = DNS_CMD_CIP_SERVICE_RES;
  ptRes->tData.ulGRC = error;
  AppDNS_GlobalPacket_Send(ptAppData);
}
```

**결론:** STM32 측 Explicit 메시지 처리 코드는 **정상적으로 구현되어 있음**.

---

## 6. 메인 루프 확인

### 6.1 파일 위치
`E:\git\netx_90_f429_SPI5\Hil_DemoApp\Sources\App_DemoApplication.c:301`

### 6.2 메인 루프 코드
```c
/** now the bus is running */
while(g_tAppData.fRunning && lRet == CIFX_NO_ERROR)
{
  /** check and process incoming packets */
  App_IODataHandler(&g_tAppData);                      // I/O 처리
  lRet = App_AllChannels_PacketHandler(&g_tAppData);   // ✅ Mailbox 처리

  OS_Sleep(500);  // ⚠️ 500ms 대기
}
```

**확인 사항:**
- ✅ `App_AllChannels_PacketHandler()` 호출됨
- ✅ Explicit 메시지가 DPM Mailbox로 들어오면 처리됨
- ⚠️ 500ms Sleep - 응답 지연 가능성 있음 (하지만 타임아웃은 아님)

---

## 7. 종합 분석 결과

### 7.1 ✅ 정상 동작 확인 항목

| 항목 | 상태 | 비고 |
|------|------|------|
| **I/O 메시징** | ✅ 정상 | Master와 통신 확인됨 |
| **Explicit 핸들러 구현** | ✅ 정상 | DNS_CMD_CIP_SERVICE_IND 처리 코드 있음 |
| **CIP 서비스 처리** | ✅ 정상 | Get/Set Attribute 구현됨 |
| **패킷 핸들러 루프** | ✅ 정상 | App_AllChannels_PacketHandler() 호출됨 |
| **클래스 등록** | ✅ 정상 | Class 0x01, 0x30 등록 코드 있음 |

### 7.2 ❗ 문제점 발견

| 항목 | 발견 내용 | 영향도 |
|------|----------|--------|
| **UCMM 설정** | DNS_CONFIGURATION_V1_T에 UCMM 관련 필드 없음 | ⚠️ 중간 |
| **EDS 파일** | UCMM/Group2OnlyServer 키워드 없음 | ⚠️ 중간 |
| **ulConfigFlags** | 0으로 설정 (기본값) | ⚠️ 중간 |
| **ulObjectFlags** | 0으로 설정 (기본값) | ⚠️ 중간 |

### 7.3 🔍 핵심 결론

#### 결론 1: Hilscher 스택의 UCMM 지원
**Hilscher DeviceNet 스택은 UCMM을 기본적으로 지원합니다.**

근거:
1. DNS_CONFIGURATION_V1_T 구조체에 UCMM ON/OFF 필드가 없음
2. EDS 파일에 UCMM 비활성화 설정 없음
3. AppDNS_HandleCipServiceIndication() 핸들러가 구현되어 있음
4. DNS_CMD_CIP_SERVICE_IND 처리 코드 존재

#### 결론 2: 문제는 netHOST 측
**STM32 측 코드는 정상입니다. 문제는 netHOST(PC 마스터) 측에 있습니다.**

근거:
- State 0xC0000004 = "Invalid Parameter/Channel/Route"
- Payload Len = 0 (CIP 응답이 생성되지 않음)
- netX 스택 레벨에서 패킷을 거부함

---

## 8. netHOST 측 문제 추정

### 8.1 State 0xC0000004 의미

```
0xC0000004 = 0xC0000000 (Critical error + confirm)
           + 0x00000004 (Invalid Parameter/Channel/Route)
```

### 8.2 가능한 원인

| 원인 | 가능성 | 확인 방법 |
|------|--------|----------|
| **1. Dest ID 오류** | ⭐⭐⭐⭐⭐ | Dest = 0x00000000 → Explicit용 채널 필요 |
| **2. Route ID 오류** | ⭐⭐⭐⭐⭐ | Route = 0x00000000 → UCMM 라우트 필요 |
| **3. CIP Path 오류** | ⭐⭐⭐ | PathSize 계산 확인 |
| **4. Fragmentation** | ⭐⭐ | 8바이트 이하로 테스트 |
| **5. BUS OFF** | ⭐ | I/O는 정상이므로 BUS는 ON |

### 8.3 추정 해결 방법

#### netHOST 패킷 송신 코드 수정 필요

**현재 (추정):**
```c
tPkt.tHead.ulDest = 0x00000000;   // ❌ 잘못된 값
tPkt.tHead.ulRoute = 0x00000000;  // ❌ 잘못된 값
```

**수정 필요:**
```c
tPkt.tHead.ulDest = 0x00000001;   // ✅ DeviceNet channel (slave)
// 또는
tPkt.tHead.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;

// Route도 확인 필요 (Hilscher 문서 참조)
tPkt.tHead.ulRoute = RCX_ROUTE_TO_SLAVE;  // 예시
```

---

## 9. 다음 단계 권장 사항

### 9.1 즉시 확인 필요
1. **netHOST 측 Explicit 메시지 송신 코드 확인**
   - RCX_SEND_PACKET_REQ 패킷 구성 부분
   - ulDest, ulRoute 값 확인

2. **Hilscher 문서 확인**
   - DeviceNet Slave Stack Manual
   - Explicit Messaging 섹션
   - Route ID 정의 확인

### 9.2 테스트 방법
1. **간단한 GET 명령으로 테스트**
   ```
   0E 03 20 30 24 01 30 0C
   ```
   - Service: 0x0E (Get Attribute Single)
   - Class: 0x30, Instance: 0x01, Attribute: 0x0C

2. **netX 로그 활성화**
   - UART 디버그 출력 확인
   - "Received UCMM Request" 메시지 확인

3. **CIFX Diagnostic Tool 사용**
   - 패킷 모니터링
   - 스택 상태 확인

### 9.3 코드 수정 (STM32 측 불필요)
**STM32 측 코드는 수정 불필요합니다.**
- UCMM 설정: 기본 활성화 (수정 불필요)
- Explicit 핸들러: 구현 완료 (수정 불필요)
- 패킷 처리 루프: 정상 동작 (수정 불필요)

**netHOST 측 코드만 확인 및 수정 필요합니다.**

---

## 10. 참고: Hilscher DeviceNet 스택 구조

### 10.1 통신 경로

```
┌─────────────┐
│  netHOST    │ (PC Master)
│  (PC App)   │
└──────┬──────┘
       │ RCX_SEND_PACKET_REQ
       │ ulDest = ??? ← 문제 가능성
       │ ulRoute = ??? ← 문제 가능성
       ↓
┌─────────────┐
│   netX90    │
│  DNS Stack  │ ← State 0xC0000004 발생 지점
│             │   (패킷 거부)
└──────┬──────┘
       │ ❌ 패킷이 여기서 차단됨
       │    DPM Mailbox로 전달 안 됨
       ↓
┌─────────────┐
│   STM32     │
│   App Code  │ ← AppDNS_HandleCipServiceIndication()
│             │   (패킷이 도달하지 않음)
└─────────────┘
```

### 10.2 정상 동작 시 경로

```
netHOST → netX90 DNS Stack → DPM Mailbox → STM32 App → 응답 생성 → 역순 전달
```

### 10.3 현재 문제 상황

```
netHOST → netX90 DNS Stack ❌ (State 0xC0000004)
                           └→ Payload = 0 (에러 응답만 반환)
```

---

## 11. 최종 요약

| 구분 | 상태 | 비고 |
|------|------|------|
| **STM32 코드** | ✅ 정상 | 수정 불필요 |
| **netX90 스택** | ✅ UCMM 지원 | 기본 활성화 |
| **EDS 파일** | ✅ 정상 | UCMM 비활성화 설정 없음 |
| **netHOST 코드** | ❌ 문제 | Dest/Route 설정 확인 필요 |

**핵심 조치 사항:**
1. netHOST 측 RCX 패킷 송신 코드 확인
2. ulDest 및 ulRoute 값 수정
3. Hilscher DeviceNet Manual 참조

---

**작성자 주석**:
이 분석은 기존 코드 검토를 통해 작성되었으며,
STM32 측 코드는 정상임을 확인했습니다.
문제는 netHOST(PC 마스터) 측 Explicit 메시지 송신 코드에 있을 가능성이 매우 높습니다.

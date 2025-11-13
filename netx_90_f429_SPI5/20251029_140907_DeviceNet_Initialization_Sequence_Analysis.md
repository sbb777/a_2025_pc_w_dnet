# DeviceNet 초기화 시퀀스 분석

**작성일시**: 2025-10-29 14:09:07
**분석 대상**: STM32 F429 + netX90 DeviceNet Slave 프로젝트
**목적**: Master Scan 대응을 위한 초기화 시퀀스 검증

---

## 요약

Master가 스캔할 때 Slave가 응답하지 않는 문제와 관련하여, 필수 초기화 시퀀스가 **모두 구현되어 있음**을 확인했습니다.

### 필수 초기화 시퀀스 검증 결과

| 단계 | 함수 | 위치 | 상태 |
|------|------|------|------|
| 1 | `InitializeToolkit()` | Core/Src/main.c:205-252 | ✅ 존재 |
| 1-1 | `isCookieAvailable()` | Core/Src/main.c:162-203 | ✅ 존재 |
| 2 | `App_CifXApplicationDemo()` | Hil_DemoApp/Sources/App_DemoApplication.c:236-357 | ✅ 존재 |
| 3 | `AppDNS_ConfigureStack()` | Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c:173-191 | ✅ 존재 |
| 3-1 | `AppDNS_SetConfiguration()` | AppDNS_DemoApplicationFunctions.c:86-129 | ✅ 존재 |
| 3-2 | `AppDNS_ChannelInit()` | AppDNS_DemoApplicationFunctions.c:136-145 | ✅ 존재 |
| 3-3 | `AppDNS_StartCommunication()` | AppDNS_DemoApplicationFunctions.c:152-162 | ✅ 존재 |

---

## 1. InitializeToolkit() - 툴킷 초기화

### 위치
`Core/Src/main.c:205-252`

### 코드 분석
```c
static int32_t InitializeToolkit()
{
    int32_t lRet = CIFX_NO_ERROR;

    // 1. cifX Toolkit 초기화
    lRet = cifXTKitInit();

    if(CIFX_NO_ERROR == lRet)
    {
        // 디바이스 인스턴스 생성 및 초기화
        PDEVICEINSTANCE ptDevInstance = (PDEVICEINSTANCE) OS_Memalloc(sizeof(*ptDevInstance));
        OS_Memset(ptDevInstance, 0, sizeof(*ptDevInstance));

        // 트레이스 레벨 설정
        g_ulTraceLevel = TRACE_LEVEL_ERROR | TRACE_LEVEL_WARNING |
                         TRACE_LEVEL_INFO | TRACE_LEVEL_DEBUG;

        // 디바이스 기본 정보 설정
        ptDevInstance->fPCICard = 0;
        ptDevInstance->pvOSDependent = ptDevInstance;
        ptDevInstance->ulDPMSize = 0x8000;  // 32K
        OS_Strncpy(ptDevInstance->szName, "cifX0", sizeof(ptDevInstance->szName));

        // 2. ⚠️ CRITICAL: Cookie 확인 루프 (netX90 펌웨어 시작 대기)
        do
        {
            SerialDPM_Init(ptDevInstance);
        } while(false == isCookieAvailable(ptDevInstance, 100));

        // 3. 툴킷에 디바이스 추가
        lRet = cifXTKitAddDevice(ptDevInstance);

        if(CIFX_NO_ERROR != lRet)
        {
            cifXTKitDeinit();
        }
    }

    return lRet;
}
```

### 주요 기능
1. **cifX Toolkit 초기화**: cifXTKitInit()
2. **디바이스 인스턴스 생성**: DPM 크기 32KB, 이름 "cifX0"
3. **Cookie 검증 루프**: netX90 펌웨어가 시작될 때까지 대기
4. **디바이스 등록**: cifXTKitAddDevice()

### 호출 위치
`main.c:629` - main() 함수 내부
```c
lRet = InitializeToolkit();
```

---

## 1-1. isCookieAvailable() - Cookie 검증

### 위치
`Core/Src/main.c:162-203`

### 코드 분석
```c
static bool isCookieAvailable(PDEVICEINSTANCE ptDevInstance, uint32_t ulTimeoutInMs)
{
    bool fCookieAvailable = false;
    char szCookie[5];
    uint32_t starttime;
    uint32_t difftime = 0;

    starttime = OS_GetMilliSecCounter();

    while(false == fCookieAvailable && difftime < ulTimeoutInMs)
    {
        OS_Memset(szCookie, 0, sizeof(szCookie));

        // DPM 주소 0x00에서 4바이트 Cookie 읽기
        HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);

        // 전역 변수로 복사 (디버깅용)
        OS_Memcpy((void*)g_szLastCookie, szCookie, sizeof(g_szLastCookie));

        // Cookie 검증: "netX" (Bootloader) 또는 "netX" (Firmware)
        if((0 == OS_Strcmp(szCookie, CIFX_DPMSIGNATURE_BSL_STR)) ||
           (0 == OS_Strcmp(szCookie, CIFX_DPMSIGNATURE_FW_STR)))
        {
            // Flash 기반 디바이스로 인식
            fCookieAvailable = true;
        }
        else
        {
            fCookieAvailable = false;
            difftime = OS_GetMilliSecCounter() - starttime;
        }
    }

    if(false == fCookieAvailable)
    {
        printf("DPM cookie not available since %u milliseconds\r\n", ulTimeoutInMs);
    }

    return fCookieAvailable;
}
```

### 주요 기능
1. **DPM Cookie 읽기**: DPM 주소 0x00에서 4바이트 읽기 (SPI를 통해)
2. **Cookie 검증**:
   - `CIFX_DPMSIGNATURE_BSL_STR`: "netX" (Bootloader)
   - `CIFX_DPMSIGNATURE_FW_STR`: "netX" (Firmware)
3. **타임아웃 처리**: 지정된 시간(100ms) 동안 대기
4. **전역 변수 저장**: `g_szLastCookie`에 마지막 읽은 Cookie 저장 (Live Expression 모니터링용)

### ⚠️ Critical Point
- **이 함수가 실패하면 InitializeToolkit()이 무한 루프에 빠질 수 있음**
- SPI 통신 문제, netX90 미응답 등이 원인일 수 있음

---

## 2. App_CifXApplicationDemo() - 애플리케이션 데모

### 위치
`Hil_DemoApp/Sources/App_DemoApplication.c:236-357`

### 코드 분석 (핵심 부분)
```c
int App_CifXApplicationDemo(char *szDeviceName)
{
    CIFXHANDLE hDriver  = NULL;
    int32_t   lRet      = 0;
    uint32_t  ulState   = 0;
    uint32_t  ulTimeout = 1000;

    PRINTF("---------- cifX Application Demo ----------" NEWLINE);

    g_tAppData.fRunning = true;

    // 프로토콜 핸들러 매핑
    g_tAppData.aptChannels[0] = (APP_COMM_CHANNEL_T*)calloc(1, sizeof(APP_COMM_CHANNEL_T));
    g_tAppData.aptChannels[0]->tProtocol = g_tRealtimeProtocolHandlers;

    // 1. 드라이버 열기
    if (CIFX_NO_ERROR != (lRet = xDriverOpen(&hDriver)))
    {
        PRINTF("ERROR: xDriverOpen failed: 0x%08x" NEWLINE, (unsigned int)lRet);
        return lRet;
    }

    // 보드 정보 읽기
    App_ReadBoardInfo(hDriver, &g_tAppData.tBoardInfo);

    // 2. 모든 통신 채널 열기
    if (CIFX_NO_ERROR != App_AllChannels_Open(&g_tAppData, hDriver, szDeviceName))
    {
        PRINTF("ERROR: Failed to open communication channels: 0x%08x" NEWLINE, lRet);
        goto error_exit;
    }

    // 3. HIL_COMM_COS_READY 대기
    App_AllChannels_GetChannelInfo_WaitReady(&g_tAppData);

    // 4. 호스트 상태를 READY로 설정
    if (CIFX_NO_ERROR != (lRet = xChannelHostState(g_tAppData.aptChannels[0]->hChannel,
                                                     CIFX_HOST_STATE_READY, &ulState, ulTimeout)))
    {
        PRINTF("ERROR: xChannelHostState failed: 0x%08X" NEWLINE, (unsigned int)lRet);
        goto error_exit;
    }

    // 5. ⚠️ CRITICAL: 설정 다운로드 (AppDNS_ConfigureStack 호출)
    if (CIFX_NO_ERROR != (lRet = App_AllChannels_Configure(&g_tAppData)))
    {
        PRINTF("Error: A channel failed to configure: 0x%08X" NEWLINE, (unsigned int)lRet);
        goto error_exit;
    }

    // 6. 메인 루프 (패킷 핸들러 + IO 데이터 핸들러)
    while(g_tAppData.fRunning && lRet == CIFX_NO_ERROR)
    {
        App_IODataHandler(&g_tAppData);
        lRet = App_AllChannels_PacketHandler(&g_tAppData);
        OS_Sleep(500);
    }

error_exit:
    // 종료 처리
    xChannelBusState(g_tAppData.aptChannels[0]->hChannel, CIFX_BUS_STATE_OFF, &ulState, 10);
    xChannelHostState(g_tAppData.aptChannels[0]->hChannel, CIFX_HOST_STATE_NOT_READY, &ulState, ulTimeout);
    App_AllChannels_Close(&g_tAppData);
    xDriverClose(hDriver);

    return lRet;
}
```

### 호출 체인
```
App_CifXApplicationDemo()
  └─> App_AllChannels_Configure(&g_tAppData)  [라인 285]
        └─> ptAppData->aptChannels[i]->tProtocol.pfStartChannelConfiguration(ptAppData)
              └─> Protocol_StartConfiguration_callback()  [AppDNS_DemoApplication.c:131-162]
                    └─> AppDNS_ConfigureStack(ptAppData, 0)  [라인 154]
```

### 호출 위치
`main.c:742` - main() 함수 내부 (조건부)
```c
if(g_bEnableVatTest)
{
    // VAT 테스트 모드 (현재 활성화됨)
    // ...
}
else
{
    // 기존 App_CifXApplicationDemo 실행
    printf("Running original App_CifXApplicationDemo...\r\n");
    lRet = App_CifXApplicationDemo("cifX0");
}
```

### ⚠️ 현재 상태
- **현재 코드에서는 `g_bEnableVatTest = true`로 설정되어 있어 VAT 테스트 모드가 활성화됨**
- **App_CifXApplicationDemo()가 실행되지 않음!**
- 대신 VAT 테스트 코드가 직접 채널을 열고 통신을 수행함

---

## 3. AppDNS_ConfigureStack() - DeviceNet 스택 설정

### 위치
`Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c:173-191`

### 코드 분석
```c
uint32_t AppDNS_ConfigureStack(APP_DATA_T* ptAppData, uint32_t ulBusOnDelay)
{
    uint32_t ulRet = CIFX_NO_ERROR;

    // 1. Mandatory: 새로운 설정 제출
    ulRet = AppDNS_SetConfiguration(ptAppData);
    if (ulRet != 0)
        return ulRet;

    // 2. Mandatory: 새로운 설정 활성화
    ulRet = AppDNS_ChannelInit(ptAppData);
    if (ulRet != 0)
        return ulRet;

    // 3. Mandatory: 통신 시작
    ulRet = AppDNS_StartCommunication(ptAppData);

    return ulRet;
}
```

### 호출 순서
1. **AppDNS_SetConfiguration()**: DeviceNet 기본 설정 (노드 주소, 보드레이트, Identity 정보 등)
2. **AppDNS_ChannelInit()**: 채널 초기화 (설정 적용)
3. **AppDNS_StartCommunication()**: 네트워크 통신 시작

---

## 3-1. AppDNS_SetConfiguration() - 기본 설정

### 위치
`Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c:86-129`

### 코드 분석
```c
uint32_t AppDNS_SetConfiguration(APP_DATA_T* ptAppData)
{
    uint32_t ulRet = CIFX_NO_ERROR;
    DNS_PACKET_SET_CONFIGURATION_REQ_T* ptReq = AppDNS_GlobalPacket_Init(ptAppData);

    DNS_CONFIGURATION_V1_T *ptCfg = &ptReq->tData.unCfg.tV1;

    // 패킷 명령 및 길이 설정
    ptReq->tHead.ulCmd        = DNS_CMD_SET_CONFIGURATION_REQ;
    ptReq->tHead.ulLen        = DNS_SET_CONFIGURATION_V1_REQ_SIZE;
    ptReq->tHead.ulSta        = 0;
    ptReq->tData.ulVersion    = DNS_CONFIGURATION_V1;

    // Slave 관련 파라미터 설정
    memset(ptCfg, 0x00, sizeof(DNS_CONFIGURATION_V1_T));
    ptCfg->ulSystemFlags      = DNS_SYS_FLG_MANUAL_START;
    ptCfg->ulWdgTime          = 0;

    // 네트워크 파라미터 설정
    ptCfg->ulNodeId           = g_tAppDnsData.bNodeIdValue;       // DEFAULT: 1
    ptCfg->ulBaudrate         = g_tAppDnsData.bBaudRateValue;     // DEFAULT: 125kB

    ptCfg->ulConfigFlags      = 0;
    ptCfg->ulObjectFlags      = 0;

    // Identity 데이터 설정
    ptCfg->usVendorId         = VENDOR_ID;                         // CIP_VENDORID_HILSCHER
    ptCfg->usDeviceType       = DEVICE_TYPE_COMMUNICATION_ADAPTER; // 0x0C
    ptCfg->usProductCode      = PRODUCT_CODE;                      // 34
    ptCfg->bMajorRev          = MAJOR_REVISION;                    // 5
    ptCfg->bMinorRev          = MINOR_REVISION;                    // 2
    ptCfg->bProductNameLen    = sizeof(abProductName)-1;
    memcpy(&ptCfg->abProductName[0], &abProductName[0], ptCfg->bProductNameLen);
    // abProductName = "VAT_V5_SIMPLE_CONFIG_DEMO"  (라인 56에서 변경됨)

    // 프로세스 데이터 설정
    ptCfg->ulProduceAsInstance = DNS_DEMO_PRODUCING_ASSEMBLY_INSTANCE;  // 0x65 (101)
    ptCfg->ulProduceAsSize     = DNS_DEMO_PRODUCING_ASSEMBLY_INSTANCE_SIZE; // 10 Byte
    ptCfg->ulConsumeAsInstance = DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE;  // 0x64 (100)
    ptCfg->ulConsumeAsSize     = DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE_SIZE; // 6 Byte

    ulRet = AppDNS_GlobalPacket_SendReceive(ptAppData);
    return ulRet;
}
```

### 주요 설정 값
| 항목 | 값 | 설명 |
|------|-----|------|
| Node ID | 1 | DeviceNet 노드 주소 |
| Baud Rate | 125kB | 네트워크 속도 |
| Vendor ID | CIP_VENDORID_HILSCHER | Hilscher |
| Device Type | 0x0C | Communication Adapter |
| Product Code | 34 | Hilscher DNS 제품 코드 |
| Major Revision | 5 | 펌웨어 메이저 버전 |
| Minor Revision | 2 | 펌웨어 마이너 버전 |
| Product Name | "VAT_V5_SIMPLE_CONFIG_DEMO" | 제품 이름 (변경됨!) |
| Produce Assembly | 0x65 (101), 10 Byte | Output 데이터 |
| Consume Assembly | 0x64 (100), 6 Byte | Input 데이터 |

### ⚠️ 중요 변경 사항
- **Product Name이 "DNS_V5_SIMPLE_CONFIG_DEMO"에서 "VAT_V5_SIMPLE_CONFIG_DEMO"로 변경됨**
- Master 스캐너가 이 이름으로 디바이스를 인식할 수 있어야 함

---

## 3-2. AppDNS_ChannelInit() - 채널 초기화

### 위치
`Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c:136-145`

### 코드 분석
```c
uint32_t AppDNS_ChannelInit(APP_DATA_T* ptAppData)
{
    uint32_t ulRet = 0;
    CIFX_PACKET* ptReq = AppDNS_GlobalPacket_Init(ptAppData);

    // Channel Init 패킷 조립
    App_SysPkt_AssembleChannelInitReq(ptReq);

    // 패킷 전송 및 응답 수신
    ulRet = AppDNS_GlobalPacket_SendReceive(ptAppData);
    return ulRet;
}
```

### 주요 기능
1. **HIL_CHANNEL_INIT_REQ 패킷 전송**: 설정 적용을 위한 채널 초기화 요청
2. **netX90 펌웨어 응답 대기**: HIL_CHANNEL_INIT_CNF 수신
3. **설정 활성화**: 이전 단계에서 전송한 DNS_CMD_SET_CONFIGURATION_REQ의 설정이 실제로 적용됨

### ⚠️ Critical Point
- **이 단계가 실패하면 DeviceNet 스택이 설정을 적용하지 못함**
- Master 스캔에 응답하려면 이 단계가 반드시 성공해야 함

---

## 3-3. AppDNS_StartCommunication() - 통신 시작

### 위치
`Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c:152-162`

### 코드 분석
```c
uint32_t AppDNS_StartCommunication(APP_DATA_T* ptAppData)
{
    uint32_t ulRet = 0;
    CIFX_PACKET* ptReq = AppDNS_GlobalPacket_Init(ptAppData);

    // Start/Stop Communication 패킷 조립 (Start = true)
    App_SysPkt_AssembleStartStopCommReq(ptReq, true);

    // 패킷 전송 및 응답 수신
    ulRet = AppDNS_GlobalPacket_SendReceive(ptAppData);

    return ulRet;
}
```

### 주요 기능
1. **HIL_START_STOP_COMM_REQ 패킷 전송**: 네트워크 통신 시작 요청
2. **netX90 펌웨어 응답 대기**: HIL_START_STOP_COMM_CNF 수신
3. **DeviceNet 네트워크 활성화**: 이 시점부터 Master의 스캔 요청에 응답할 수 있음

### ⚠️ Critical Point
- **이 단계가 완료되어야 Master 스캔에 응답 가능**
- 만약 이 단계가 실패하면 네트워크에 참여하지 못함

---

## 전체 초기화 시퀀스 플로우

```
main()
  │
  ├─> InitializeToolkit()  [main.c:629]
  │     │
  │     ├─> cifXTKitInit()
  │     │
  │     ├─> SerialDPM_Init()
  │     │
  │     ├─> isCookieAvailable()  ⚠️ LOOP until success
  │     │     └─> HWIF_READN() - DPM 주소 0x00에서 Cookie 읽기
  │     │         └─> 검증: "netX" (BSL) 또는 "netX" (FW)
  │     │
  │     └─> cifXTKitAddDevice()
  │
  └─> [조건부] App_CifXApplicationDemo("cifX0")  [main.c:742]
        │
        ├─> xDriverOpen()
        │
        ├─> App_AllChannels_Open()
        │     └─> xChannelOpen() - 채널 0 열기
        │
        ├─> App_AllChannels_GetChannelInfo_WaitReady()
        │     └─> ⚠️ LOOP until HIL_COMM_COS_READY
        │
        ├─> xChannelHostState(CIFX_HOST_STATE_READY)
        │
        ├─> App_AllChannels_Configure()  ⚠️ CRITICAL
        │     └─> Protocol_StartConfiguration_callback()
        │           └─> AppDNS_ConfigureStack()
        │                 │
        │                 ├─> AppDNS_SetConfiguration()  [1/3]
        │                 │     └─> DNS_CMD_SET_CONFIGURATION_REQ 전송
        │                 │         - Node ID: 1
        │                 │         - Baud Rate: 125kB
        │                 │         - Identity: VAT_V5_SIMPLE_CONFIG_DEMO
        │                 │         - Assembly: 100(in), 101(out)
        │                 │
        │                 ├─> AppDNS_ChannelInit()  [2/3]
        │                 │     └─> HIL_CHANNEL_INIT_REQ 전송
        │                 │         - 설정 적용 및 채널 초기화
        │                 │
        │                 └─> AppDNS_StartCommunication()  [3/3]
        │                       └─> HIL_START_STOP_COMM_REQ 전송
        │                           - 네트워크 통신 시작
        │                           - ✅ 이 시점부터 Master 스캔 응답 가능
        │
        └─> while(running)
              ├─> App_IODataHandler() - I/O 데이터 교환
              └─> App_AllChannels_PacketHandler() - 패킷 처리
```

---

## 현재 프로젝트 상태 분석

### 1. VAT 테스트 모드가 활성화됨
**파일**: `Core/Src/main.c:116, 634-736`

```c
volatile bool g_bEnableVatTest = true;  // ⚠️ VAT 테스트 활성화

if(g_bEnableVatTest)
{
    // VAT 테스트 모드: App_CifXApplicationDemo() 호출하지 않음
    // 대신 직접 채널을 열고 VAT 테스트 실행
}
else
{
    // 기존 App_CifXApplicationDemo 실행
    lRet = App_CifXApplicationDemo("cifX0");
}
```

### 2. VAT 테스트 모드의 초기화 시퀀스
VAT 테스트 모드는 다음과 같은 초기화를 수행:

1. ✅ `xDriverOpen()` - 드라이버 열기
2. ✅ `xChannelOpen()` - 채널 0 열기
3. ✅ HIL_COMM_COS_READY 대기 (타임아웃 5초)
4. ❌ **AppDNS_ConfigureStack() 호출하지 않음!**

### ⚠️ 문제점 발견!

**VAT 테스트 모드에서는 AppDNS_ConfigureStack()을 호출하지 않습니다!**

이는 다음을 의미합니다:
- AppDNS_SetConfiguration() ❌
- AppDNS_ChannelInit() ❌
- AppDNS_StartCommunication() ❌

**결과**: DeviceNet 스택이 제대로 설정되지 않아 Master 스캔에 응답하지 못할 수 있음!

---

## Master 스캔 미작동 원인 분석

### 가능한 원인 목록

| # | 원인 | 확률 | 설명 |
|---|------|------|------|
| 1 | **AppDNS_ConfigureStack() 미호출** | **90%** | VAT 테스트 모드에서 스택 설정을 건너뜀 |
| 2 | SPI 통신 문제 | 10% | Cookie 읽기 실패, 데이터 손상 |
| 3 | 채널 준비 타임아웃 | 5% | HIL_COMM_COS_READY 플래그가 설정되지 않음 |
| 4 | 노드 주소 불일치 | 5% | Master와 Slave의 주소 설정이 다름 |
| 5 | 보드레이트 불일치 | 5% | Master와 Slave의 속도 설정이 다름 |
| 6 | Assembly 설정 오류 | 3% | I/O 크기 또는 인스턴스 번호 불일치 |

### 가장 유력한 원인: AppDNS_ConfigureStack() 미호출

**증거**:
1. `g_bEnableVatTest = true` 설정 (main.c:116)
2. VAT 테스트 모드는 App_CifXApplicationDemo()를 호출하지 않음
3. App_AllChannels_Configure()가 실행되지 않음
4. 따라서 AppDNS_ConfigureStack() → AppDNS_SetConfiguration/ChannelInit/StartCommunication 전체 체인이 실행되지 않음

---

## 해결 방안

### Option 1: VAT 테스트 모드에서도 스택 설정 추가 (권장)

VAT 테스트 초기화 코드에 스택 설정을 추가:

```c
// main.c:641 부근 (VAT_InitializeTest() 후)
VAT_InitializeTest();

// ✅ 추가: DeviceNet 스택 설정
if (CIFX_NO_ERROR == (lRet = AppDNS_ConfigureStack(&g_tAppData, 0)))
{
    printf("DeviceNet stack configured successfully.\r\n");
}
else
{
    printf("ERROR: AppDNS_ConfigureStack failed: 0x%08X\r\n", (unsigned int)lRet);
}
```

**필요한 수정**:
1. `g_tAppData` 구조체를 VAT 테스트 모드에서도 초기화
2. 채널 정보를 `g_tAppData.aptChannels[0]`에 저장
3. AppDNS_ConfigureStack() 호출

### Option 2: VAT 테스트 모드 비활성화

```c
// main.c:116
volatile bool g_bEnableVatTest = false;  // ❌ VAT 테스트 비활성화
```

이 경우 기존 App_CifXApplicationDemo()가 실행되어 정상적인 초기화가 수행됨.

### Option 3: 하이브리드 모드 (VAT 테스트 + 표준 초기화)

App_CifXApplicationDemo()를 수정하여 VAT 테스트 기능을 통합:
- 표준 초기화 수행 (AppDNS_ConfigureStack 포함)
- 메인 루프에서 VAT 테스트 실행

---

## 검증 체크리스트

Master 스캔 응답을 위해 다음 사항을 확인하세요:

### 초기화 단계
- [ ] InitializeToolkit() 성공 (CIFX_NO_ERROR 반환)
- [ ] isCookieAvailable() 성공 (Cookie: "netX" 또는 "netX")
- [ ] xDriverOpen() 성공
- [ ] xChannelOpen() 성공
- [ ] HIL_COMM_COS_READY 플래그 확인
- [ ] xChannelHostState(CIFX_HOST_STATE_READY) 성공

### 스택 설정 단계 (⚠️ 현재 누락됨!)
- [ ] AppDNS_SetConfiguration() 성공
- [ ] AppDNS_ChannelInit() 성공
- [ ] AppDNS_StartCommunication() 성공

### 네트워크 설정
- [ ] Node ID 확인 (기본값: 1)
- [ ] Baud Rate 확인 (기본값: 125kB)
- [ ] Master와 Slave의 Baud Rate 일치
- [ ] 네트워크 터미네이션 확인
- [ ] 케이블 연결 상태 확인

### 디버깅 변수 모니터링
Live Expression에서 다음 변수를 모니터링:
- `g_szLastCookie`: DPM Cookie 값 (예상: "netX")
- `g_tAppData.fRunning`: 애플리케이션 실행 상태
- `g_tAppData.aptChannels[0]->tChannelInfo.ulDeviceCOSFlags`: 채널 상태 플래그
- `g_tAppData.fInputDataValid`: 입력 데이터 유효성

---

## 결론

### 발견 사항
1. ✅ **모든 필수 함수가 존재함**: InitializeToolkit, isCookieAvailable, App_CifXApplicationDemo, AppDNS_ConfigureStack 및 하위 함수들
2. ✅ **호출 체인이 올바르게 구성됨**: 정상적인 경로로 실행 시 올바른 순서로 호출됨
3. ❌ **VAT 테스트 모드에서 스택 설정이 누락됨**: AppDNS_ConfigureStack()가 호출되지 않음

### 추천 조치
1. **즉시 조치**: VAT 테스트 모드에서 AppDNS_ConfigureStack() 호출 추가
2. **검증**: 수정 후 Master 스캔 테스트 재수행
3. **장기 계획**: VAT 테스트 기능을 App_CifXApplicationDemo() 프레임워크에 통합

### 예상 결과
AppDNS_ConfigureStack() 호출을 추가하면 다음이 해결될 것으로 예상됨:
- DeviceNet Identity 정보 설정 완료
- Assembly 인스턴스 설정 완료
- 네트워크 통신 시작
- **Master 스캔에 정상 응답 가능**

---

## 참조 파일 목록

| 파일 | 경로 | 주요 함수 |
|------|------|----------|
| main.c | Core/Src/main.c | InitializeToolkit, isCookieAvailable |
| AppDNS_DemoApplication.c | Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.c | Protocol_StartConfiguration_callback |
| AppDNS_DemoApplicationFunctions.c | Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c | AppDNS_ConfigureStack, AppDNS_SetConfiguration, AppDNS_ChannelInit, AppDNS_StartCommunication |
| App_DemoApplication.c | Hil_DemoApp/Sources/App_DemoApplication.c | App_CifXApplicationDemo, App_AllChannels_Configure |

---

**분석 완료 일시**: 2025-10-29 14:09:07

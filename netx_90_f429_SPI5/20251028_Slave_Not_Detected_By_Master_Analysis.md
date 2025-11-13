# Master가 Slave를 검색하지 못하는 문제 원인 분석 및 해결

**작성일시**: 2025-10-28
**작업자**: Claude
**문제**: DeviceNet Master가 network scan 시 Slave를 검출하지 못함

---

## 1. 문제 상황

### 1.1 증상
- **이전**: Master에서 network scan 시 Slave 검출 성공
- **현재**: Master에서 network scan 시 Slave 검출 실패
- **발생 시점**: VAT 테스트 통합 코드 추가 후

### 1.2 사용자 보고
> "코드가 최조 작업전 코드에서는 master에서 network scan 하면 현재 코드의 slave가 검출이 되었는데 현재는 검색이 되지 않는다"

---

## 2. 근본 원인 분석

### 2.1 코드 변경사항 비교

#### 이전 코드 (442425a - 정상 작동)
```c
// main.c
int main(void)
{
    /* 초기화 코드 */
    ...

    lRet = InitializeToolkit();
    if(CIFX_NO_ERROR == lRet)
    {
        /* 기존 App_CifXApplicationDemo 실행 */
        lRet = App_CifXApplicationDemo("cifX0");
    }
}
```

**실행 흐름**:
1. `InitializeToolkit()` - cifX 드라이버 초기화
2. `App_CifXApplicationDemo()` - **DeviceNet 스택 설정**
   - `xDriverOpen()` - 드라이버 열기
   - `App_AllChannels_Open()` - 채널 열기
   - `xChannelHostState()` - **Host Ready 설정**
   - `App_AllChannels_Configure()` - **DeviceNet 설정 다운로드**
   - `App_SysPkt_StartCommunication()` - **네트워크 통신 시작**

#### 현재 코드 (HEAD - 검색 실패)
```c
// main.c
int main(void)
{
    /* 초기화 코드 */
    ...

    lRet = InitializeToolkit();
    if(CIFX_NO_ERROR == lRet)
    {
        if(g_bEnableVatTest)  // ← true로 설정됨
        {
            /* VAT 테스트 모드 */
            lRet = xDriverOpen(&hDriver);
            lRet = xChannelOpen(hDriver, "cifX0", 0, &hChannel);

            // COS_READY 대기
            do {
                xChannelInfo(hChannel, sizeof(CHANNEL_INFORMATION), &tChannelInfo);
                HAL_Delay(100);
            } while(!(tChannelInfo.ulDeviceCOSFlags & HIL_COMM_COS_READY));

            // ❌ Host Ready 설정 누락
            // ❌ DeviceNet 설정 다운로드 누락
            // ❌ 네트워크 통신 시작 누락

            PrintDeviceInformation(hDriver, hChannel);
            VAT_RunTest(hChannel);

            xChannelClose(hChannel);
            xDriverClose(hDriver);
        }
        else
        {
            /* 기존 App_CifXApplicationDemo 실행 */
            lRet = App_CifXApplicationDemo("cifX0");
        }
    }
}
```

**실행 흐름**:
1. `InitializeToolkit()` - cifX 드라이버 초기화
2. **VAT 테스트 모드 실행** (g_bEnableVatTest = true)
   - `xDriverOpen()` - 드라이버 열기 ✓
   - `xChannelOpen()` - 채널 열기 ✓
   - COS_READY 대기 ✓
   - ❌ **`xChannelHostState()` 누락** - Host Ready 미설정
   - ❌ **`App_AllChannels_Configure()` 누락** - DeviceNet 설정 미다운로드
   - ❌ **네트워크 통신 시작 누락**

### 2.2 누락된 초기화 단계

#### Step 1: Host Ready 상태 설정 (누락)
```c
// App_DemoApplication.c:278
if (CIFX_NO_ERROR != (lRet = xChannelHostState(
        g_tAppData.aptChannels[0]->hChannel,
        CIFX_HOST_STATE_READY,
        &ulState,
        ulTimeout)))
{
    PRINTF("ERROR: xChannelHostState failed: 0x%08X" NEWLINE, (unsigned int)lRet);
    goto error_exit;
}
```

**역할**: Host 애플리케이션이 준비되었음을 netX90에 알림

**누락 시 영향**:
- netX90이 Host가 준비되지 않았다고 판단
- 일부 기능이 활성화되지 않을 수 있음

#### Step 2: DeviceNet 스택 설정 다운로드 (누락)
```c
// App_DemoApplication.c:285
if (CIFX_NO_ERROR != (lRet = App_AllChannels_Configure(&g_tAppData)))
{
    PRINTF("Error: A channel failed to configure: 0x%08X" NEWLINE, (unsigned int)lRet);
    goto error_exit;
}
```

**내부 호출 체인**:
```
App_AllChannels_Configure()
  ↓
Protocol_StartConfiguration_callback()  // AppDNS_DemoApplication.c:131
  ↓
AppDNS_ConfigureStack()                // AppDNS_DemoApplicationFunctions.c:172
  ↓
├─ AppDNS_SetConfiguration()           // DNS_CMD_SET_CONFIGURATION_REQ
│  - MAC ID 설정
│  - Baud Rate 설정
│  - Vendor ID, Device Type 설정
│  - Product Name 설정
│
├─ xChannelConfigLock()                // 설정 잠금
│
└─ App_SysPkt_StartCommunication()     // 네트워크 통신 시작
   - Bus On 명령 전송
   - Slave를 네트워크에 등록
```

**AppDNS_SetConfiguration()의 핵심 역할**:
```c
// AppDNS_DemoApplicationFunctions.c
uint32_t AppDNS_SetConfiguration(APP_DATA_T* ptAppData)
{
    DNS_PACKET_SET_CONFIGURATION_REQ_T tSendPkt;

    /* MAC ID 설정 */
    tSendPkt.tData.tConfiguration.usDeviceAddress = 10;  // Slave Node Address

    /* Baud Rate 설정 */
    tSendPkt.tData.tConfiguration.ulBaudrate = 250000;   // 250 kbps

    /* Identity Object 설정 */
    tSendPkt.tData.tConfiguration.usVendorId = 0x011C;   // Vendor ID
    tSendPkt.tData.tConfiguration.usDeviceType = 0x000C; // Device Type
    tSendPkt.tData.tConfiguration.usProductCode = 0x0001;

    /* Product Name 설정 */
    strcpy(tSendPkt.tData.tConfiguration.szProductName, "VAT Pressure Controller");

    /* 스택에 설정 전송 */
    return Pkt_SendReceivePacket(ptAppData, 0, &tSendPkt, TXRX_TIMEOUT);
}
```

**누락 시 영향**:
- **MAC ID 미설정**: Slave가 네트워크 주소를 갖지 못함
- **Identity Object 미등록**: Master가 Slave 정보를 조회할 수 없음
- **네트워크 미등록**: Slave가 DeviceNet 네트워크에 참여하지 못함
- **결과**: Master의 network scan에서 Slave 검출 불가

---

## 3. DeviceNet Slave 등록 프로세스

### 3.1 정상 등록 프로세스

```
┌─────────────────────────────────────────────────┐
│  Step 1: Driver & Channel Initialization       │
│  - xDriverOpen()                                │
│  - xChannelOpen()                               │
│  - Wait for COS_READY                           │
└─────────────────────────┬───────────────────────┘
                          ↓
┌─────────────────────────────────────────────────┐
│  Step 2: Host Ready State                      │
│  - xChannelHostState(CIFX_HOST_STATE_READY)    │
│  → Host 준비 완료 신호                          │
└─────────────────────────┬───────────────────────┘
                          ↓
┌─────────────────────────────────────────────────┐
│  Step 3: DeviceNet Stack Configuration         │
│  - AppDNS_SetConfiguration()                   │
│    • MAC ID: 10                                 │
│    • Baud Rate: 250 kbps                        │
│    • Vendor ID: 0x011C                          │
│    • Device Type: 0x000C                        │
│    • Product Name: "VAT Pressure Controller"    │
│  → DNS_CMD_SET_CONFIGURATION_REQ 전송           │
└─────────────────────────┬───────────────────────┘
                          ↓
┌─────────────────────────────────────────────────┐
│  Step 4: Configuration Lock                    │
│  - xChannelConfigLock()                         │
│  → 설정 변경 방지                               │
└─────────────────────────┬───────────────────────┘
                          ↓
┌─────────────────────────────────────────────────┐
│  Step 5: Start Network Communication           │
│  - App_SysPkt_StartCommunication()             │
│  → Bus On 명령                                  │
│  → Slave를 DeviceNet 네트워크에 등록            │
└─────────────────────────┬───────────────────────┘
                          ↓
┌─────────────────────────────────────────────────┐
│  Step 6: Network Online                        │
│  - Identity Object 활성화                       │
│  - Master scan 응답 가능                        │
│  - I/O Messaging 가능                           │
└─────────────────────────────────────────────────┘
```

### 3.2 현재 코드의 실행 프로세스

```
┌─────────────────────────────────────────────────┐
│  Step 1: Driver & Channel Initialization       │
│  - xDriverOpen()                         ✓     │
│  - xChannelOpen()                        ✓     │
│  - Wait for COS_READY                    ✓     │
└─────────────────────────┬───────────────────────┘
                          ↓
                    ❌ 이후 단계 누락
                          ↓
┌─────────────────────────────────────────────────┐
│  직접 VAT 테스트 실행                           │
│  - PrintDeviceInformation()                     │
│  - VAT_RunTest() - I/O Read/Write              │
└─────────────────────────────────────────────────┘
```

**결과**:
- Identity Object 미등록
- DeviceNet 네트워크 미참여
- Master scan 시 응답 불가
- **I/O Messaging은 COS_READY 상태에서 일부 가능** (설정 없이도 동작 가능)

---

## 4. 왜 VAT 테스트는 부분적으로 작동하는가?

### 4.1 I/O Messaging vs Identity Response

#### I/O Messaging (VAT 테스트)
```c
// vat_devicenet_test.c
int32_t lRet = xChannelIORead(hChannel,
                               0,           // Input area
                               0,           // Offset
                               sizeof(VAT_INPUT_DATA_T),
                               &inputData,
                               TIMEOUT);
```

**특징**:
- DPM (Dual Port Memory) 직접 접근
- DeviceNet 스택 설정과 **독립적**
- COS_READY 상태만 있으면 동작 가능
- 네트워크 등록 없이도 로컬 I/O 영역 접근 가능

**결과**: VAT 테스트는 부분적으로 작동 (I/O Read/Write 성공)

#### Identity Response (Master Scan)
```
Master → Explicit Message → "Get Identity Object"
                                  ↓
                         Slave (netX90)
                         ├─ Identity Object 등록 필요
                         ├─ MAC ID 설정 필요
                         └─ Network Online 상태 필요
                                  ↓
                              ❌ 미설정
                                  ↓
                            응답 없음/타임아웃
```

**특징**:
- DeviceNet 프로토콜 레벨 응답
- Identity Object **등록 필수**
- MAC ID **설정 필수**
- Network Online 상태 **필수**

**결과**: Master scan 실패 (Identity 미등록)

### 4.2 COS_READY vs Network Online

| 상태 | COS_READY | Network Online |
|------|-----------|----------------|
| 의미 | 채널 준비 완료 | 네트워크 참여 완료 |
| 조건 | 펌웨어 로드 완료 | 스택 설정 완료 |
| I/O Read/Write | ✓ 가능 | ✓ 가능 |
| Identity Response | ❌ 불가 | ✓ 가능 |
| Master Scan | ❌ 검출 불가 | ✓ 검출 가능 |

**현재 상태**: COS_READY만 달성, Network Online 미달성

---

## 5. 해결 방법

### 5.1 방법 1: App_CifXApplicationDemo 호출 (권장)

#### 수정 내용
```c
// main.c
int main(void)
{
    ...
    lRet = InitializeToolkit();

    if(CIFX_NO_ERROR == lRet)
    {
        if(g_bEnableVatTest)
        {
            /* ========== 방법 1: 기존 초기화 재사용 ========== */

            /* 기존 App_CifXApplicationDemo 실행하여 DeviceNet 스택 초기화 */
            printf("Initializing DeviceNet stack...\r\n");
            lRet = App_CifXApplicationDemo("cifX0");

            /* 초기화 성공 후 VAT 테스트 실행 */
            if(lRet == CIFX_NO_ERROR)
            {
                printf("\r\nDeviceNet stack initialized successfully!\r\n");
                printf("Starting VAT test...\r\n");

                /* VAT 테스트는 별도 스레드 또는 주기적 호출로 실행 */
                // 참고: App_CifXApplicationDemo는 무한 루프를 포함하므로
                // VAT 테스트를 Protocol_PacketHandler_callback 내에서 실행하거나
                // 별도 방법 필요
            }
        }
        else
        {
            /* 기존 코드 */
            lRet = App_CifXApplicationDemo("cifX0");
        }
    }
}
```

**장점**:
- 가장 간단하고 안전
- 모든 초기화 단계 자동 처리
- 검증된 코드 재사용

**단점**:
- App_CifXApplicationDemo는 무한 루프 포함
- VAT 테스트와의 통합 구조 변경 필요

### 5.2 방법 2: 필수 초기화 단계 추가

#### 수정 내용
```c
// main.c
int main(void)
{
    ...
    if(g_bEnableVatTest)
    {
        /* VAT 테스트 모드 */
        CIFXHANDLE hDriver = NULL;
        CIFXHANDLE hChannel = NULL;

        /* VAT 테스트 초기화 */
        VAT_InitializeTest();

        /* cifX 드라이버 열기 */
        lRet = xDriverOpen(&hDriver);
        if(CIFX_NO_ERROR == lRet)
        {
            /* 채널 0 열기 */
            lRet = xChannelOpen(hDriver, "cifX0", 0, &hChannel);
            if(CIFX_NO_ERROR == lRet)
            {
                /* 채널 준비 대기 */
                CHANNEL_INFORMATION tChannelInfo;
                do {
                    xChannelInfo(hChannel, sizeof(CHANNEL_INFORMATION), &tChannelInfo);
                    HAL_Delay(100);
                } while(!(tChannelInfo.ulDeviceCOSFlags & HIL_COMM_COS_READY));

                printf("\r\n*** Channel ready! ***\r\n");

                /* ========== 추가: Host Ready 설정 ========== */
                uint32_t ulState = 0;
                lRet = xChannelHostState(hChannel, CIFX_HOST_STATE_READY, &ulState, 1000);
                if(lRet != CIFX_NO_ERROR)
                {
                    printf("ERROR: xChannelHostState failed: 0x%08X\r\n", (unsigned int)lRet);
                }
                else
                {
                    printf("Host Ready state set successfully\r\n");
                }

                /* ========== 추가: DeviceNet 스택 설정 ========== */
                printf("Configuring DeviceNet stack...\r\n");

                // g_tAppData 초기화
                extern APP_DATA_T g_tAppData;
                memset(&g_tAppData, 0, sizeof(APP_DATA_T));
                g_tAppData.aptChannels[0] = (APP_COMM_CHANNEL_T*)malloc(sizeof(APP_COMM_CHANNEL_T));
                g_tAppData.aptChannels[0]->hChannel = hChannel;

                // DeviceNet 설정 다운로드
                extern uint32_t AppDNS_ConfigureStack(APP_DATA_T* ptAppData, uint32_t ulBusOnDelay);
                lRet = AppDNS_ConfigureStack(&g_tAppData, 0);

                if(lRet != CIFX_NO_ERROR)
                {
                    printf("ERROR: AppDNS_ConfigureStack failed: 0x%08X\r\n", (unsigned int)lRet);
                }
                else
                {
                    printf("DeviceNet stack configured successfully\r\n");
                    printf("Slave is now visible on network\r\n\r\n");
                }

                /* 디바이스 정보 출력 */
                PrintDeviceInformation(hDriver, hChannel);

                /* VAT 테스트 실행 */
                VAT_RunTest(hChannel);

                /* 채널 닫기 */
                xChannelClose(hChannel);

                /* 메모리 정리 */
                free(g_tAppData.aptChannels[0]);
            }

            /* 드라이버 닫기 */
            xDriverClose(hDriver);
        }

        /* 테스트 종료 */
        VAT_Test_Deinit(&g_tVatContext);
    }
}
```

**장점**:
- 현재 VAT 테스트 구조 유지
- 필요한 초기화만 선택적 추가

**단점**:
- AppDNS_ConfigureStack 함수 접근 필요
- g_tAppData 구조체 초기화 필요
- 복잡도 증가

### 5.3 방법 3: 간소화된 초기화 (최소 구현)

#### 핵심 패킷만 전송
```c
// main.c에 추가
static int32_t ConfigureDeviceNetMinimal(CIFXHANDLE hChannel)
{
    int32_t lRet;

    /* 1. Set Configuration Packet */
    DNS_PACKET_SET_CONFIGURATION_REQ_T tConfigReq;
    DNS_PACKET_SET_CONFIGURATION_CNF_T tConfigCnf;

    memset(&tConfigReq, 0, sizeof(tConfigReq));

    /* 패킷 헤더 설정 */
    tConfigReq.tHead.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;
    tConfigReq.tHead.ulLen = sizeof(DNS_SET_CONFIGURATION_REQ_T);
    tConfigReq.tHead.ulCmd = DNS_CMD_SET_CONFIGURATION_REQ;

    /* DeviceNet 설정 */
    tConfigReq.tData.tConfiguration.usDeviceAddress = 10;      // MAC ID
    tConfigReq.tData.tConfiguration.ulBaudrate = 250000;       // 250 kbps
    tConfigReq.tData.tConfiguration.usVendorId = 0x011C;       // Vendor ID
    tConfigReq.tData.tConfiguration.usDeviceType = 0x000C;     // Device Type
    tConfigReq.tData.tConfiguration.usProductCode = 0x0001;    // Product Code
    strcpy(tConfigReq.tData.tConfiguration.szProductName, "VAT Controller");

    /* 패킷 전송 */
    lRet = xChannelPutPacket(hChannel, (CIFX_PACKET*)&tConfigReq, 5000);
    if(lRet != CIFX_NO_ERROR)
    {
        printf("ERROR: Failed to send SetConfiguration: 0x%08X\r\n", (unsigned int)lRet);
        return lRet;
    }

    /* 응답 수신 */
    lRet = xChannelGetPacket(hChannel, sizeof(tConfigCnf), (CIFX_PACKET*)&tConfigCnf, 5000);
    if(lRet != CIFX_NO_ERROR)
    {
        printf("ERROR: Failed to receive SetConfiguration response: 0x%08X\r\n", (unsigned int)lRet);
        return lRet;
    }

    if(tConfigCnf.tHead.ulSta != CIFX_NO_ERROR)
    {
        printf("ERROR: SetConfiguration failed: 0x%08X\r\n", (unsigned int)tConfigCnf.tHead.ulSta);
        return tConfigCnf.tHead.ulSta;
    }

    printf("DeviceNet configuration sent successfully\r\n");

    /* 2. Configuration Lock */
    lRet = xChannelConfigLock(hChannel, CIFX_CONFIGURATION_LOCK, 5000);
    if(lRet != CIFX_NO_ERROR)
    {
        printf("WARNING: Config lock failed: 0x%08X\r\n", (unsigned int)lRet);
        // Continue anyway
    }

    /* 3. Start Communication */
    HIL_DPM_BUS_STATE_CHANNEL_T tBusState;
    tBusState.ulCmd = CIFX_BUS_STATE_ON;

    lRet = xChannelBusState(hChannel, CIFX_BUS_STATE_ON, &tBusState.ulState, 5000);
    if(lRet != CIFX_NO_ERROR)
    {
        printf("ERROR: Failed to start bus: 0x%08X\r\n", (unsigned int)lRet);
        return lRet;
    }

    printf("DeviceNet bus started - Slave is now online\r\n");

    return CIFX_NO_ERROR;
}

// main() 함수에서 호출
if(lRet == CIFX_NO_ERROR)
{
    printf("\r\n*** Channel ready! ***\r\n");

    /* Host Ready 설정 */
    uint32_t ulState = 0;
    xChannelHostState(hChannel, CIFX_HOST_STATE_READY, &ulState, 1000);

    /* DeviceNet 최소 설정 */
    lRet = ConfigureDeviceNetMinimal(hChannel);

    if(lRet == CIFX_NO_ERROR)
    {
        /* 디바이스 정보 출력 */
        PrintDeviceInformation(hDriver, hChannel);

        /* VAT 테스트 실행 */
        VAT_RunTest(hChannel);
    }
}
```

**장점**:
- 외부 함수 의존성 최소화
- 명시적인 설정 과정
- 디버깅 용이

**단점**:
- 패킷 구조 이해 필요
- 에러 처리 직접 구현
- 유지보수 부담

---

## 6. 권장 솔루션

### 6.1 단계별 적용 전략

#### Phase 1: 임시 해결 (즉시 적용)
```c
// main.c
volatile bool g_bEnableVatTest = false;  // VAT 테스트 비활성화
```

**효과**:
- App_CifXApplicationDemo() 실행
- DeviceNet 스택 정상 초기화
- Master scan 정상 작동
- VAT 테스트는 수동으로 별도 실행

#### Phase 2: 최소 통합 (권장)
```c
// main.c에 방법 3의 ConfigureDeviceNetMinimal() 추가
// VAT 테스트 전에 호출
```

**효과**:
- VAT 테스트 구조 유지
- DeviceNet 초기화 추가
- Master scan 정상 작동
- 최소 코드 변경

#### Phase 3: 완전 통합 (장기)
```c
// App_CifXApplicationDemo 구조 리팩토링
// VAT 테스트를 Protocol_PacketHandler 내에 통합
```

**효과**:
- 단일 실행 경로
- 모든 기능 통합
- 유지보수 용이

### 6.2 즉시 적용 코드 (방법 2 간소화)

```c
// main.c 수정 (Line 694 이후)
if(lRet == CIFX_NO_ERROR)
{
    printf("\r\n*** Channel ready! ***\r\n");
    printf("  COS Flags: 0x%08X\r\n", (unsigned int)tChannelInfo.ulDeviceCOSFlags);
    printf("  Time taken: %lu seconds\r\n\r\n", timeout_count / 10);

    /* ========== DeviceNet 초기화 추가 시작 ========== */

    /* 1. Host Ready 상태 설정 */
    uint32_t ulState = 0;
    printf("Setting Host Ready state...\r\n");
    lRet = xChannelHostState(hChannel, CIFX_HOST_STATE_READY, &ulState, 1000);
    if(lRet != CIFX_NO_ERROR)
    {
        printf("ERROR: xChannelHostState failed: 0x%08X\r\n", (unsigned int)lRet);
    }
    else
    {
        printf("  Host Ready state set successfully\r\n");
    }

    /* 2. DeviceNet 스택 설정 */
    printf("Configuring DeviceNet stack for network visibility...\r\n");

    // AppDNS_ConfigureStack 호출을 위한 최소 초기화
    extern APP_DATA_T g_tAppData;
    extern PROTOCOL_DESCRIPTOR_T g_tRealtimeProtocolHandlers;

    // 채널 0 설정
    if(g_tAppData.aptChannels[0] == NULL)
    {
        g_tAppData.aptChannels[0] = (APP_COMM_CHANNEL_T*)malloc(sizeof(APP_COMM_CHANNEL_T));
        memset(g_tAppData.aptChannels[0], 0, sizeof(APP_COMM_CHANNEL_T));
    }
    g_tAppData.aptChannels[0]->hChannel = hChannel;
    g_tAppData.aptChannels[0]->tProtocol = g_tRealtimeProtocolHandlers;

    // 패킷 통신 초기화
    extern bool Pkt_Init(uint32_t ulChannelIndex, uint32_t ulPacketCnt);
    if(!Pkt_Init(0, 1))
    {
        printf("ERROR: Pkt_Init failed\r\n");
        lRet = CIFX_DRV_INIT_ERROR;
    }
    else
    {
        // DeviceNet 설정 다운로드
        extern uint32_t AppDNS_ConfigureStack(APP_DATA_T* ptAppData, uint32_t ulBusOnDelay);
        lRet = AppDNS_ConfigureStack(&g_tAppData, 0);

        if(lRet != CIFX_NO_ERROR)
        {
            printf("ERROR: AppDNS_ConfigureStack failed: 0x%08X\r\n", (unsigned int)lRet);
            printf("  Slave may not be visible on network\r\n");
        }
        else
        {
            printf("  DeviceNet stack configured successfully\r\n");
            printf("  Slave is now visible on network for Master scan\r\n");
            printf("  MAC ID: 10, Baud: 250 kbps\r\n\r\n");
        }
    }

    /* ========== DeviceNet 초기화 추가 끝 ========== */

    /* 디바이스 정보 출력 */
    PrintDeviceInformation(hDriver, hChannel);

    /* VAT 테스트 실행 */
    VAT_RunTest(hChannel);
}
```

---

## 7. 검증 방법

### 7.1 수정 전 상태 확인
```
Master 측에서:
1. RSNetWorx for DeviceNet 실행
2. Network Scan 실행
3. 결과: Slave Node 10 검출 실패
```

### 7.2 수정 후 상태 확인
```
1. 위의 권장 코드 적용
2. STM32 재빌드 및 다운로드
3. UART 로그 확인:
   ✓ "Host Ready state set successfully"
   ✓ "DeviceNet stack configured successfully"
   ✓ "Slave is now visible on network"

Master 측에서:
4. RSNetWorx Network Scan 실행
5. 결과: Slave Node 10 검출 성공
6. Identity 정보 확인:
   - Vendor ID: 0x011C
   - Device Type: 0x000C
   - Product Name: "VAT Controller"
```

### 7.3 기능 검증
```
1. Master Scan: Slave 검출 ✓
2. Identity Query: Slave 정보 조회 ✓
3. I/O Messaging: VAT 테스트 정상 작동 ✓
4. 에러 카운트: 0 유지 ✓
```

---

## 8. 추가 고려사항

### 8.1 메모리 관리
```c
// 프로그램 종료 시 메모리 해제
if(g_tAppData.aptChannels[0] != NULL)
{
    free(g_tAppData.aptChannels[0]);
    g_tAppData.aptChannels[0] = NULL;
}
```

### 8.2 에러 복구
```c
// DeviceNet 설정 실패 시에도 VAT 테스트 계속 진행 가능
if(lRet != CIFX_NO_ERROR)
{
    printf("WARNING: DeviceNet network registration failed\r\n");
    printf("  Master scan will not detect this slave\r\n");
    printf("  But I/O Messaging (VAT test) may still work\r\n");
    lRet = CIFX_NO_ERROR;  // Continue with VAT test
}
```

### 8.3 디버깅
```c
// 각 단계마다 상세 로그 출력
printf("[DEBUG] xChannelHostState: 0x%08X\r\n", (unsigned int)lRet);
printf("[DEBUG] Pkt_Init: %s\r\n", bRet ? "OK" : "FAIL");
printf("[DEBUG] AppDNS_ConfigureStack: 0x%08X\r\n", (unsigned int)lRet);
```

---

## 9. 결론

### 9.1 핵심 요약

**문제 원인**:
- VAT 테스트 통합 시 App_CifXApplicationDemo() 실행 생략
- DeviceNet 스택 설정 다운로드 누락
- Identity Object, MAC ID, Network Online 상태 미달성

**해결 방법**:
- `xChannelHostState()` - Host Ready 설정
- `AppDNS_ConfigureStack()` - DeviceNet 설정 다운로드
- `App_SysPkt_StartCommunication()` - 네트워크 통신 시작

**권장 액션**:
1. **즉시**: 방법 2의 권장 코드 적용
2. **검증**: Master scan으로 Slave 검출 확인
3. **장기**: App_CifXApplicationDemo 통합 구조 개선

### 9.2 체크리스트

- [ ] Host Ready 상태 설정 추가
- [ ] DeviceNet 스택 설정 다운로드 추가
- [ ] 코드 빌드 및 다운로드
- [ ] UART 로그에서 "Slave is now visible" 메시지 확인
- [ ] Master에서 network scan 실행
- [ ] Slave Node 10 검출 확인
- [ ] Identity 정보 (Vendor ID, Product Name) 확인
- [ ] VAT 테스트 정상 작동 확인

---

**작성 완료일시**: 2025-10-28
**문서 버전**: 1.0
**권장 조치**: 방법 2 권장 코드 즉시 적용

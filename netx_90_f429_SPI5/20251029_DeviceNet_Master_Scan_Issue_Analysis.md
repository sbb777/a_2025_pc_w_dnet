# DeviceNet Master Scan 문제 분석 보고서

**작성일시**: 2025-10-29
**프로젝트**: STM32 F429 + netX90 DeviceNet Slave
**이슈**: DeviceNet Master가 Slave를 스캔할 수 없는 문제

---

## 📋 요약 (Executive Summary)

**문제**: DeviceNet Master가 Slave 디바이스를 스캔할 때 응답이 없음

**근본 원인**: VAT 테스트 모드 활성화로 인해 **DeviceNet 스택 초기화 함수들이 호출되지 않음**

**영향도**: 🔴 **Critical** - DeviceNet 통신 완전 불가

---

## 🔍 필수 초기화 시퀀스 분석

DeviceNet Slave가 Master 스캔에 응답하기 위해서는 다음 초기화 순서가 **반드시** 완료되어야 합니다:

### 1️⃣ InitializeToolkit()
**위치**: `Core\Src\main.c:205-252`

**역할**:
- cifX 툴킷 초기화
- DPM(Dual Port Memory) 통신 설정
- netX90 펌웨어 실행 확인

**내부 호출 순서**:
```c
cifXTKitInit()
  → SerialDPM_Init(ptDevInstance)
  → isCookieAvailable(ptDevInstance, 100)  // DPM cookie 검증
  → cifXTKitAddDevice(ptDevInstance)
```

**Cookie 검증**:
- `isCookieAvailable()` 함수 (`Core\Src\main.c:162-203`)
- DPM 주소 0x00에서 "netX" 또는 "BOOT" 문자열 확인
- netX90 펌웨어가 정상 실행중인지 검증

**현재 상태**: ✅ **호출됨** (`main.c:629`)

---

### 2️⃣ App_CifXApplicationDemo()
**위치**: `Hil_DemoApp\Sources\App_DemoApplication.c:236-357`

**역할**:
- cifX 드라이버 열기
- 통신 채널 열기
- DeviceNet 프로토콜 핸들러 등록
- 스택 설정 및 통신 시작 관리

**내부 호출 순서**:
```c
xDriverOpen(&hDriver)                                    // Line 257
  → App_ReadBoardInfo(hDriver, &g_tAppData.tBoardInfo)  // Line 263
  → App_AllChannels_Open(&g_tAppData, hDriver, "cifX0") // Line 266
  → App_AllChannels_GetChannelInfo_WaitReady()          // Line 275
  → xChannelHostState(CIFX_HOST_STATE_READY)            // Line 278
  → App_AllChannels_Configure(&g_tAppData)              // Line 285 ⭐ 핵심
  → 통신 루프 실행                                       // Line 301
```

**현재 상태**: ❌ **호출되지 않음** (`main.c:742`가 `else` 블록 안에 있음)

---

### 3️⃣ AppDNS_ConfigureStack()
**위치**: `Hil_DemoAppDNS\Sources\AppDNS_DemoApplicationFunctions.c:173-191`

**역할**: DeviceNet Slave 스택의 핵심 초기화 함수

**필수 호출 순서** (모두 `CIFX_NO_ERROR` 반환 필요):

#### 3-1. AppDNS_SetConfiguration()
**위치**: `AppDNS_DemoApplicationFunctions.c:86-129`

**기능**:
- DeviceNet 노드 주소 설정 (기본값: 1)
- 통신 속도(Baudrate) 설정 (기본값: 125kbps)
- CIP Identity Object 설정
  - Vendor ID: Hilscher (283)
  - Device Type: Communication Adapter (0x0C)
  - Product Code: 34
  - Product Name: "VAT_V5_SIMPLE_CONFIG_DEMO"
- Assembly Instance 설정
  - Consuming Assembly: 0x64 (100) - 6 bytes
  - Producing Assembly: 0x65 (101) - 10 bytes

**패킷**: `DNS_CMD_SET_CONFIGURATION_REQ`

**현재 상태**: ❌ **호출되지 않음**

---

#### 3-2. AppDNS_ChannelInit()
**위치**: `AppDNS_DemoApplicationFunctions.c:136-145`

**기능**:
- 설정 파라미터를 DeviceNet 스택에 적용
- 채널 초기화 완료

**패킷**: `HIL_CHANNEL_INIT_REQ`

**현재 상태**: ❌ **호출되지 않음**

---

#### 3-3. AppDNS_StartCommunication()
**위치**: `AppDNS_DemoApplicationFunctions.c:152-162`

**기능**:
- 네트워크 통신 활성화
- **이 함수가 성공해야 Master 스캔에 응답 가능** 🔑

**패킷**: `HIL_START_STOP_COMM_REQ`

**현재 상태**: ❌ **호출되지 않음**

---

### 📊 호출 관계도

```
main()
  └─ InitializeToolkit() ✅
      └─ isCookieAvailable() ✅

  └─ if (g_bEnableVatTest == true)  ← 현재 활성화됨
      ├─ xDriverOpen() ✅
      ├─ xChannelOpen() ✅
      ├─ VAT_RunTest() ✅
      └─ ❌ AppDNS_ConfigureStack() 호출 안됨!

  └─ else (g_bEnableVatTest == false)
      └─ App_CifXApplicationDemo() ❌ 실행되지 않음
          └─ App_AllChannels_Configure()
              └─ Protocol_StartConfiguration_callback()
                  └─ AppDNS_ConfigureStack() ⭐
                      ├─ AppDNS_SetConfiguration()
                      ├─ AppDNS_ChannelInit()
                      └─ AppDNS_StartCommunication() 🔑
```

---

## 🔴 문제 발견

### 현재 코드 상태 (`Core\Src\main.c`)

**Line 109-116**: VAT 테스트 모드 활성화
```c
volatile uint8_t g_VatTestMode = 3;        // Continuous Monitoring
volatile bool g_bEnableVatTest = true;     // ⚠️ VAT 테스트 활성화
```

**Line 635-746**: Main 함수의 조건부 실행
```c
if(g_bEnableVatTest)
{
    /* VAT 테스트 모드 */
    xDriverOpen(&hDriver);
    xChannelOpen(hDriver, "cifX0", 0, &hChannel);

    // ❌ DeviceNet 스택 초기화 함수들이 호출되지 않음!
    // ❌ AppDNS_ConfigureStack() 없음
    // ❌ AppDNS_SetConfiguration() 없음
    // ❌ AppDNS_ChannelInit() 없음
    // ❌ AppDNS_StartCommunication() 없음

    VAT_RunTest(hChannel);  // 바로 VAT 테스트 실행
}
else
{
    /* 기존 App_CifXApplicationDemo 실행 */
    App_CifXApplicationDemo("cifX0");  // ✅ 여기서 모든 초기화 진행
}
```

---

## 💡 근본 원인

1. **VAT 테스트 모드**가 활성화되어 있음 (`g_bEnableVatTest = true`)

2. VAT 모드에서는 **채널만 열고** 바로 테스트를 시작함
   - `xDriverOpen()` ✅
   - `xChannelOpen()` ✅
   - **DeviceNet 스택 설정** ❌

3. `App_CifXApplicationDemo()` 함수가 **호출되지 않음**
   - `else` 블록 안에 있어서 실행되지 않음

4. DeviceNet 스택 초기화 함수들이 **전혀 실행되지 않음**:
   - `AppDNS_SetConfiguration()` - 노드 주소, Baudrate, Identity 설정
   - `AppDNS_ChannelInit()` - 설정 적용
   - `AppDNS_StartCommunication()` - 통신 시작

5. **결과**: DeviceNet Slave가 네트워크에 존재하지 않는 상태
   - Master가 스캔해도 응답 없음
   - CIP Identity Object 미설정
   - Assembly Instance 미설정
   - 통신 활성화 안됨

---

## ✅ 해결 방안

### 방안 1: VAT 테스트 모드에서 DeviceNet 초기화 추가 (권장)

**위치**: `Core\Src\main.c:635-746`

**수정 내용**:
```c
if(g_bEnableVatTest)
{
    /* VAT 테스트 모드 */
    VAT_InitializeTest();

    lRet = xDriverOpen(&hDriver);
    if(CIFX_NO_ERROR == lRet)
    {
        lRet = xChannelOpen(hDriver, "cifX0", 0, &hChannel);
        if(CIFX_NO_ERROR == lRet)
        {
            // ✅ DeviceNet 스택 초기화 추가
            printf("Initializing DeviceNet Stack...\r\n");

            // 전역 g_tAppData 설정
            g_tAppData.aptChannels[0] = (APP_COMM_CHANNEL_T*)calloc(1, sizeof(APP_COMM_CHANNEL_T));
            g_tAppData.aptChannels[0]->hChannel = hChannel;
            g_tAppData.aptChannels[0]->tProtocol = g_tRealtimeProtocolHandlers;

            // DeviceNet 스택 설정
            lRet = AppDNS_ConfigureStack(&g_tAppData, 0);
            if(lRet != CIFX_NO_ERROR)
            {
                printf("ERROR: AppDNS_ConfigureStack failed: 0x%08X\r\n", (unsigned int)lRet);
            }
            else
            {
                printf("DeviceNet Stack initialized successfully!\r\n");

                // 채널 준비 대기 (기존 코드)
                // ...

                PrintDeviceInformation(hDriver, hChannel);
                VAT_RunTest(hChannel);
            }
        }
    }
}
```

**필요 사항**:
- `g_tRealtimeProtocolHandlers` extern 선언 추가
- `AppDNS_ConfigureStack()` 함수 사용을 위한 헤더 포함
- `APP_COMM_CHANNEL_T` 구조체 할당 및 초기화

---

### 방안 2: VAT 테스트 모드 비활성화 (간단)

**위치**: `Core\Src\main.c:116`

**수정**:
```c
volatile bool g_bEnableVatTest = false;  // ✅ VAT 테스트 비활성화
```

**효과**:
- `App_CifXApplicationDemo()` 함수가 실행됨
- DeviceNet 스택이 정상적으로 초기화됨
- Master 스캔에 응답 가능

**단점**:
- VAT 테스트 기능을 사용할 수 없음

---

### 방안 3: 하이브리드 모드 (VAT + DeviceNet)

**개념**: VAT 테스트와 DeviceNet 통신을 모두 활성화

**구현**:
1. `App_CifXApplicationDemo()` 내부의 통신 루프에서 VAT 테스트 호출
2. 또는 VAT 테스트 모드에서 `App_CifXApplicationDemo()`의 초기화 부분만 호출

---

## 📝 검증 체크리스트

수정 후 다음 항목들을 확인하여 DeviceNet 초기화가 완료되었는지 검증:

### 1. 초기화 함수 호출 확인
- [ ] `AppDNS_SetConfiguration()` 호출 및 `CIFX_NO_ERROR` 반환
- [ ] `AppDNS_ChannelInit()` 호출 및 `CIFX_NO_ERROR` 반환
- [ ] `AppDNS_StartCommunication()` 호출 및 `CIFX_NO_ERROR` 반환

### 2. 채널 상태 확인
- [ ] `ulDeviceCOSFlags & HIL_COMM_COS_READY` 플래그 설정됨
- [ ] 채널 준비 대기 루프 정상 완료

### 3. DeviceNet Identity 확인
```c
CHANNEL_INFORMATION tChannelInfo;
xChannelInfo(hChannel, sizeof(CHANNEL_INFORMATION), &tChannelInfo);

// 확인 항목:
// - Firmware Name: "DeviceNet Slave"
// - FW Major: 5
// - FW Minor: 2
// - netX Flags: 통신 활성화 상태
```

### 4. Master 스캔 응답 확인
- [ ] Master가 Slave 노드 주소(기본값: 1)에서 디바이스 발견
- [ ] CIP Identity Object 조회 성공
  - Vendor ID: 283 (Hilscher)
  - Product Code: 34
  - Product Name: "VAT_V5_SIMPLE_CONFIG_DEMO"

### 5. 로그 메시지 확인
```
DeviceNet Stack initialized successfully!
Channel ready!
COS Flags: 0x00000001 (HIL_COMM_COS_READY)
```

---

## 📚 관련 파일 목록

### 핵심 파일
1. **Core\Src\main.c**
   - `InitializeToolkit()` (Line 205-252)
   - `isCookieAvailable()` (Line 162-203)
   - `main()` VAT 테스트 모드 분기 (Line 635-746)

2. **Hil_DemoApp\Sources\App_DemoApplication.c**
   - `App_CifXApplicationDemo()` (Line 236-357)
   - `App_AllChannels_Configure()` (Line 152-173)

3. **Hil_DemoAppDNS\Sources\AppDNS_DemoApplicationFunctions.c**
   - `AppDNS_ConfigureStack()` (Line 173-191) ⭐
   - `AppDNS_SetConfiguration()` (Line 86-129)
   - `AppDNS_ChannelInit()` (Line 136-145)
   - `AppDNS_StartCommunication()` (Line 152-162) 🔑

4. **Hil_DemoAppDNS\Sources\AppDNS_DemoApplication.c**
   - `Protocol_StartConfiguration_callback()` (Line 131-162)
   - `g_tRealtimeProtocolHandlers` 정의 (Line 203-207)

### 헤더 파일
- `App_DemoApplication.h`
- `AppDNS_DemoApplication.h`
- `cifXToolkit.h`
- `DNS_Includes.h`

---

## 🎯 권장 조치

### 즉시 조치 (Priority 1)
1. **방안 1 구현**: VAT 테스트 모드에 DeviceNet 스택 초기화 추가
2. `AppDNS_ConfigureStack()` 호출 결과 확인
3. Master 스캔 테스트 수행

### 단기 조치 (Priority 2)
1. 초기화 실패 시 오류 메시지 개선
2. DeviceNet 상태 모니터링 로그 추가
3. 채널 상태 플래그 실시간 확인 기능 추가

### 장기 개선 (Priority 3)
1. VAT 테스트와 DeviceNet 통신을 병행하는 하이브리드 모드 구현
2. 초기화 시퀀스 자동 검증 기능 추가
3. Master 스캔 응답 자동 테스트 코드 작성

---

## 📖 참고 문서

### DeviceNet 초기화 시퀀스
1. Toolkit 초기화 (`InitializeToolkit`)
2. Driver 열기 (`xDriverOpen`)
3. 채널 열기 (`xChannelOpen`)
4. **DeviceNet 스택 설정** (`AppDNS_ConfigureStack`) ⭐ 필수
   - 노드 설정 (`AppDNS_SetConfiguration`)
   - 채널 초기화 (`AppDNS_ChannelInit`)
   - 통신 시작 (`AppDNS_StartCommunication`) 🔑
5. 채널 준비 대기 (`HIL_COMM_COS_READY`)
6. 통신 루프 실행

### Master 스캔 요구사항
- DeviceNet Slave Identity Object 설정 완료
- Assembly Instance 설정 완료
- 네트워크 통신 활성화 (`AppDNS_StartCommunication` 성공)
- 노드 주소 및 Baudrate 일치

---

## 🔚 결론

**핵심 문제**: VAT 테스트 모드에서 **DeviceNet 스택 초기화 함수들이 호출되지 않음**

**영향**: Master가 Slave를 스캔할 수 없음 (통신 불가)

**해결책**: VAT 테스트 모드에서 `AppDNS_ConfigureStack()` 호출 추가

**예상 소요시간**: 코드 수정 및 테스트 - 약 1-2시간

---

**분석자**: Claude (Anthropic AI Assistant)
**분석 도구**: Code Analysis, Function Call Tracing
**분석 완료일**: 2025-10-29

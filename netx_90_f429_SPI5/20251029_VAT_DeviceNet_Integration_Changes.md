# VAT 모드 DeviceNet 스택 초기화 추가 - 작업 완료 보고서

**작성일시**: 2025-10-29
**작업자**: Claude AI Assistant
**작업 유형**: 코드 수정 (Bug Fix - Critical)
**파일**: `Core\Src\main.c`

---

## 📋 작업 요약

VAT 테스트 모드에서 DeviceNet Master가 Slave를 스캔할 수 없는 문제를 해결하기 위해, **DeviceNet 스택 초기화 코드를 추가**했습니다.

---

## 🎯 수정 목적

**문제**: VAT 테스트 모드 활성화 시 DeviceNet 스택 초기화가 누락되어 Master 스캔 불가

**해결**: VAT 테스트 모드 내에 `AppDNS_ConfigureStack()` 호출 추가

**효과**:
- ✅ DeviceNet Identity Object 설정 완료
- ✅ Assembly Instance 설정 완료
- ✅ 네트워크 통신 활성화
- ✅ Master 스캔 응답 가능

---

## 📝 수정 내용 상세

### 1. 헤더 파일 추가

**위치**: `Core\Src\main.c:37`

**추가된 코드**:
```c
#include "AppDNS_DemoApplication.h"  // DeviceNet 스택 초기화 함수
```

**목적**: `AppDNS_ConfigureStack()` 함수 선언을 포함하기 위함

---

### 2. Extern 변수 선언 추가

**위치**: `Core\Src\main.c:75-78`

**추가된 코드**:
```c
/* ========== DeviceNet 스택 초기화 - extern 선언 ========== */
extern PROTOCOL_DESCRIPTOR_T g_tRealtimeProtocolHandlers;  // AppDNS_DemoApplication.c에서 정의
extern APP_DATA_T g_tAppData;                              // App_SystemPackets.c에서 정의
/* ========== extern 선언 끝 ========== */
```

**목적**:
- `g_tRealtimeProtocolHandlers`: DeviceNet 프로토콜 핸들러 (정의 위치: `Hil_DemoAppDNS\Sources\AppDNS_DemoApplication.c:203`)
- `g_tAppData`: 애플리케이션 데이터 구조체 (정의 위치: `Hil_DemoApp\Sources\App_SystemPackets.c:33`)

---

### 3. DeviceNet 스택 초기화 코드 추가

**위치**: `Core\Src\main.c:658-705`

**추가된 코드**:
```c
/* ========== DeviceNet 스택 초기화 시작 ========== */
printf("\r\n");
printf("========================================\r\n");
printf(" DeviceNet Stack Initialization\r\n");
printf("========================================\r\n");

/* g_tAppData 채널 0 설정 */
if(g_tAppData.aptChannels[0] == NULL)
{
    g_tAppData.aptChannels[0] = (APP_COMM_CHANNEL_T*)calloc(1, sizeof(APP_COMM_CHANNEL_T));
    if(g_tAppData.aptChannels[0] == NULL)
    {
        printf("ERROR: Failed to allocate memory for channel 0\r\n");
        lRet = CIFX_DRV_INIT_ERROR;
    }
}

if(lRet == CIFX_NO_ERROR)
{
    /* 채널 핸들 및 프로토콜 핸들러 설정 */
    g_tAppData.aptChannels[0]->hChannel = hChannel;
    g_tAppData.aptChannels[0]->tProtocol = g_tRealtimeProtocolHandlers;

    printf("Step 1: Calling AppDNS_ConfigureStack()...\r\n");

    /* DeviceNet 스택 설정 */
    lRet = AppDNS_ConfigureStack(&g_tAppData, 0);

    if(lRet == CIFX_NO_ERROR)
    {
        printf("Step 1: ✅ AppDNS_ConfigureStack() SUCCESS\r\n");
        printf("  - AppDNS_SetConfiguration() completed\r\n");
        printf("  - AppDNS_ChannelInit() completed\r\n");
        printf("  - AppDNS_StartCommunication() completed\r\n");
        printf("\r\nDeviceNet Stack initialized successfully!\r\n");
        printf("Master scan should now detect this slave.\r\n");
        printf("========================================\r\n\r\n");
    }
    else
    {
        printf("Step 1: ❌ AppDNS_ConfigureStack() FAILED\r\n");
        printf("  Error Code: 0x%08X\r\n", (unsigned int)lRet);
        printf("\r\nDeviceNet Stack initialization failed!\r\n");
        printf("Master scan will NOT detect this slave.\r\n");
        printf("========================================\r\n\r\n");
    }
}
/* ========== DeviceNet 스택 초기화 끝 ========== */
```

**초기화 순서**:
1. **메모리 할당**: `g_tAppData.aptChannels[0]` 구조체 할당
2. **채널 설정**: `hChannel` 핸들 및 프로토콜 핸들러 연결
3. **스택 초기화**: `AppDNS_ConfigureStack()` 호출
   - 내부적으로 `AppDNS_SetConfiguration()` 실행
   - 내부적으로 `AppDNS_ChannelInit()` 실행
   - 내부적으로 `AppDNS_StartCommunication()` 실행 🔑
4. **결과 확인**: 성공/실패 메시지 출력

---

## 🔄 실행 흐름 변경

### 수정 전 (Before)
```
main()
  └─ InitializeToolkit() ✅
  └─ if (g_bEnableVatTest == true)
      ├─ xDriverOpen() ✅
      ├─ xChannelOpen() ✅
      └─ VAT_RunTest() ✅
      ❌ DeviceNet 스택 초기화 없음!
```

### 수정 후 (After)
```
main()
  └─ InitializeToolkit() ✅
  └─ if (g_bEnableVatTest == true)
      ├─ xDriverOpen() ✅
      ├─ xChannelOpen() ✅
      ├─ ✅ g_tAppData.aptChannels[0] 설정
      ├─ ✅ AppDNS_ConfigureStack()
      │   ├─ AppDNS_SetConfiguration()
      │   ├─ AppDNS_ChannelInit()
      │   └─ AppDNS_StartCommunication() 🔑
      └─ VAT_RunTest() ✅
```

---

## 📊 AppDNS_ConfigureStack() 내부 동작

`AppDNS_ConfigureStack()` 함수는 다음 3단계를 순차적으로 실행합니다:

### Step 1: AppDNS_SetConfiguration()
**파일**: `Hil_DemoAppDNS\Sources\AppDNS_DemoApplicationFunctions.c:86-129`

**기능**:
- DeviceNet 노드 주소 설정: **1** (기본값)
- 통신 속도 설정: **125 kbps** (기본값)
- CIP Identity Object 설정:
  - Vendor ID: **283** (Hilscher)
  - Device Type: **0x0C** (Communication Adapter)
  - Product Code: **34**
  - Product Name: **"VAT_V5_SIMPLE_CONFIG_DEMO"**
  - Major Revision: **5**
  - Minor Revision: **2**
- Assembly Instance 설정:
  - Consuming Assembly: **0x64 (100)** - 6 bytes
  - Producing Assembly: **0x65 (101)** - 10 bytes

**패킷**: `DNS_CMD_SET_CONFIGURATION_REQ`

---

### Step 2: AppDNS_ChannelInit()
**파일**: `Hil_DemoAppDNS\Sources\AppDNS_DemoApplicationFunctions.c:136-145`

**기능**:
- Step 1에서 설정한 파라미터를 DeviceNet 스택에 적용
- 채널 초기화 완료

**패킷**: `HIL_CHANNEL_INIT_REQ`

---

### Step 3: AppDNS_StartCommunication() 🔑
**파일**: `Hil_DemoAppDNS\Sources\AppDNS_DemoApplicationFunctions.c:152-162`

**기능**:
- **네트워크 통신 활성화**
- **Master 스캔에 응답 가능 상태로 전환**
- DeviceNet 버스 상에서 Slave로서 활성화

**패킷**: `HIL_START_STOP_COMM_REQ`

---

## ✅ 검증 방법

수정 후 다음 사항들을 확인하여 DeviceNet 초기화가 완료되었는지 검증하세요:

### 1. 컴파일 확인
- [ ] `Core\Src\main.c` 컴파일 성공
- [ ] 링크 에러 없음
- [ ] 빌드 완료

### 2. 실행 로그 확인
프로그램 실행 시 다음 메시지가 출력되어야 합니다:

```
========================================
 DeviceNet Stack Initialization
========================================
Step 1: Calling AppDNS_ConfigureStack()...
Step 1: ✅ AppDNS_ConfigureStack() SUCCESS
  - AppDNS_SetConfiguration() completed
  - AppDNS_ChannelInit() completed
  - AppDNS_StartCommunication() completed

DeviceNet Stack initialized successfully!
Master scan should now detect this slave.
========================================
```

**실패 시 출력**:
```
Step 1: ❌ AppDNS_ConfigureStack() FAILED
  Error Code: 0x########
```

### 3. 채널 상태 확인
```c
CHANNEL_INFORMATION tChannelInfo;
xChannelInfo(hChannel, sizeof(CHANNEL_INFORMATION), &tChannelInfo);

// 확인 항목:
// - ulDeviceCOSFlags & HIL_COMM_COS_READY == 1
// - Firmware Name: "DeviceNet Slave"
// - FW Version: 5.2.x.x
```

### 4. Master 스캔 테스트
- [ ] DeviceNet Master를 사용하여 네트워크 스캔 실행
- [ ] 노드 주소 **1**에서 Slave 디바이스 발견
- [ ] CIP Identity 정보 조회:
  - Vendor ID: **283** (Hilscher)
  - Product Code: **34**
  - Product Name: **"VAT_V5_SIMPLE_CONFIG_DEMO"**
- [ ] Assembly Instance 확인:
  - Input (Slave → Master): **10 bytes**
  - Output (Master → Slave): **6 bytes**

### 5. 통신 테스트
- [ ] Master에서 Output 데이터 전송
- [ ] Slave가 Input 데이터 정상 수신 (xChannelIORead 성공)
- [ ] Slave가 Output 데이터 정상 전송 (xChannelIOWrite 성공)
- [ ] Master에서 Input 데이터 정상 수신

---

## 🚨 예상 문제 및 해결책

### 문제 1: 컴파일 에러 - AppDNS_DemoApplication.h 파일을 찾을 수 없음

**증상**:
```
fatal error: AppDNS_DemoApplication.h: No such file or directory
```

**원인**: 헤더 파일 경로가 include path에 추가되지 않음

**해결책**:
1. 프로젝트 설정에서 Include Paths 확인
2. 다음 경로 추가:
   ```
   Hil_DemoAppDNS/includes/DemoAppDNS
   ```

---

### 문제 2: 링크 에러 - g_tRealtimeProtocolHandlers undefined reference

**증상**:
```
undefined reference to `g_tRealtimeProtocolHandlers'
```

**원인**: `AppDNS_DemoApplication.c` 파일이 빌드에 포함되지 않음

**해결책**:
1. 프로젝트에서 다음 파일들이 빌드 대상에 포함되어 있는지 확인:
   - `Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.c`
   - `Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c`
2. Makefile 또는 프로젝트 설정에서 소스 경로 추가

---

### 문제 3: 실행 시 AppDNS_ConfigureStack() 실패 (0x########)

**증상**:
```
Step 1: ❌ AppDNS_ConfigureStack() FAILED
  Error Code: 0x########
```

**원인**: DeviceNet 펌웨어 미로드, 채널 상태 불량, SPI 통신 오류 등

**해결책**:
1. **에러 코드 확인**: `cifXErrors.h`에서 에러 코드 의미 확인
2. **일반적인 에러 코드**:
   - `0xC0000001` (CIFX_DEV_NOT_READY): netX90 준비 안됨
   - `0xC000000E` (CIFX_INVALID_COMMAND): 잘못된 패킷 명령
   - `0xC0000011` (CIFX_DEV_NO_COM_FLAG): 통신 플래그 없음
3. **디버깅 단계**:
   - InitializeToolkit() 성공 여부 확인
   - isCookieAvailable() 성공 여부 확인
   - xChannelOpen() 성공 여부 확인
   - SPI 통신 상태 확인
   - netX90 펌웨어 로드 상태 확인

---

### 문제 4: Master 스캔에서 여전히 발견되지 않음

**증상**: AppDNS_ConfigureStack() 성공했지만 Master가 스캔 시 Slave를 찾지 못함

**원인**:
- 노드 주소 불일치
- Baudrate 불일치
- 네트워크 케이블 문제
- 터미네이션 저항 문제

**해결책**:
1. **노드 주소 확인**:
   - Slave 노드 주소: **1** (기본값)
   - Master 스캔 범위에 노드 1이 포함되어 있는지 확인
2. **Baudrate 확인**:
   - Slave Baudrate: **125 kbps** (기본값)
   - Master와 동일한 Baudrate인지 확인
3. **하드웨어 확인**:
   - DeviceNet 케이블 연결 상태
   - 터미네이션 저항 (120Ω) 양쪽 끝에 설치 확인
   - 전원 공급 (24V DC) 확인
4. **네트워크 상태 확인**:
   - LED 상태 확인 (녹색: 정상, 빨간색: 에러)
   - Master의 네트워크 상태 확인

---

## 📦 관련 파일 목록

### 수정된 파일
- ✏️ **Core\Src\main.c** (주요 수정)

### 참조 파일 (수정 없음)
- **Hil_DemoApp\Includes\App_DemoApplication.h**
- **Hil_DemoAppDNS\includes\DemoAppDNS\AppDNS_DemoApplication.h**
- **Hil_DemoAppDNS\Sources\AppDNS_DemoApplication.c**
- **Hil_DemoAppDNS\Sources\AppDNS_DemoApplicationFunctions.c**
- **Hil_DemoApp\Sources\App_SystemPackets.c**

---

## 🎯 다음 단계

### 즉시 수행
1. **빌드**: 프로젝트 전체 빌드 수행
2. **플래시**: STM32F429에 펌웨어 업로드
3. **실행**: 시리얼 로그 확인
4. **검증**: Master 스캔 테스트

### 테스트 시나리오
1. **초기화 검증**:
   - 로그에서 "DeviceNet Stack initialized successfully!" 확인
   - 에러 메시지가 없는지 확인

2. **Master 스캔**:
   - DeviceNet Master 도구 실행
   - 네트워크 스캔 수행
   - 노드 주소 1에서 디바이스 발견 확인

3. **Identity 확인**:
   - CIP Identity Object 조회
   - Product Name이 "VAT_V5_SIMPLE_CONFIG_DEMO"인지 확인
   - Vendor ID가 283인지 확인

4. **통신 테스트**:
   - Master에서 Output 데이터 전송
   - Slave 응답 확인
   - IO 데이터 교환 정상 동작 확인

---

## 📈 예상 결과

### 성공 시
- ✅ 컴파일 성공
- ✅ DeviceNet 스택 초기화 성공
- ✅ Master 스캔에서 Slave 발견
- ✅ CIP Identity 정보 정상 조회
- ✅ IO 데이터 통신 정상

### 실패 시
- ❌ 컴파일 에러 → Include Path 및 소스 파일 확인
- ❌ 런타임 에러 → 에러 코드 분석 및 하드웨어 상태 확인
- ❌ 스캔 실패 → 노드 주소, Baudrate, 케이블, 터미네이션 확인

---

## 📞 기술 지원

문제 발생 시 다음 정보를 포함하여 문의하세요:

1. **컴파일 에러**: 전체 에러 메시지
2. **런타임 에러**:
   - 시리얼 로그 전체
   - AppDNS_ConfigureStack() 반환 에러 코드
   - 채널 상태 정보 (ulDeviceCOSFlags)
3. **스캔 실패**:
   - Master 스캔 결과
   - 노드 주소 및 Baudrate 설정값
   - 네트워크 하드웨어 구성

---

## 📝 변경 이력

| 일시 | 작업 | 설명 |
|------|------|------|
| 2025-10-29 | 초기 작성 | VAT 모드에 DeviceNet 스택 초기화 추가 |

---

**작업 완료**: ✅
**빌드 대기**: 🔄
**테스트 대기**: ⏳

---

**문서 작성자**: Claude (Anthropic AI Assistant)
**분석 기반**: Code Analysis, Function Call Tracing, DeviceNet Protocol Documentation

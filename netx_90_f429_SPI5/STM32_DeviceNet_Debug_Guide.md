# STM32 DeviceNet 디버깅 가이드
**에러 분석 및 CubeIDE 디버깅 방법**

---

## 🔴 현재 에러 분석

### 수신 패킷 정보
```
Dest:   0x00000000 (Host → netX)
Src:    0x00000000 (netX 응답)
Len:    0          ❌ 데이터 없음
State:  0xC0000004 ❌ ERR_HIL_UNKNOWN_COMMAND
Cmd:    0x00008021 (RCX_RECEIVE_PACKET_CNF)
```

### 에러 의미
```c
#define ERR_HIL_UNKNOWN_COMMAND  ((uint32_t)0xC0000004L)
// "Unknown Command in Packet received."
```

**원인**: netX가 CIP 서비스 요청을 인식하지 못함
- CIP 서비스 핸들러가 등록되지 않았거나
- Class 등록이 안되었거나
- 패킷 핸들러가 제대로 구현되지 않음

---

## 🐛 STM32 CubeIDE 디버깅 방법

### 1️⃣ **Breakpoint 디버깅**

#### Step 1: 핵심 브레이크포인트 설정
파일: `Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c`

```c
// ✅ BP1: 패킷 수신 핸들러 진입점
bool AppDNS_PacketHandler_callback(CIFX_PACKET* ptPacket, void* pvUserData)
{
    bool fPacketHandled = true;  // ← 여기에 브레이크포인트

    switch(ptPacket->tHeader.ulCmd) {
        case DNS_CMD_CIP_SERVICE_IND:  // ← 이 case가 실행되는지 확인!
            AppDNS_HandleCipServiceIndication(ptAppData);
            break;
    }
}

// ✅ BP2: CIP 서비스 처리 함수
void AppDNS_HandleCipServiceIndication(APP_DATA_T* ptAppData)
{
    DNS_PACKET_CIP_SERVICE_IND_T* ptInd = ...;  // ← 여기에 브레이크포인트

    uint8_t service = (uint8_t)ptInd->tData.ulService;
    uint8_t class_id = (uint8_t)ptInd->tData.ulClass;
    // 변수 값 확인!
}
```

#### Step 2: 변수 감시 설정 (Variables View)
```
ptPacket->tHeader.ulCmd       ← 0x0000B104 (DNS_CMD_CIP_SERVICE_IND) 확인
ptInd->tData.ulService        ← 0x0E (Get) 또는 0x10 (Set) 확인
ptInd->tData.ulClass          ← 0x01 또는 0x30 확인
ptInd->tData.ulInstance       ← Instance ID 확인
ptInd->tData.ulAttribute      ← Attribute ID 확인
```

---

### 2️⃣ **UART 로그 디버깅**

#### Step 1: UART 출력 확인
```c
// ✅ 이미 구현된 로그 출력
printf("\n=== CIP Service Indication ===\n");
printf("Service:   0x%02X ", service);
printf("Class:     0x%02X\n", class_id);
printf("Instance:  0x%02X\n", instance_id);
printf("Attribute: 0x%02X\n", attribute_id);
```

#### Step 2: 시리얼 터미널 설정
- **Baud Rate**: 115200
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1

**추천 프로그램**:
- Tera Term
- PuTTY
- STM32 CubeMonitor

---

### 3️⃣ **Live Expressions (실시간 변수 감시)**

CubeIDE의 Live Expressions 창에 추가:

```c
// 전역 변수
g_tParamManager.param_count                // 초기화된 파라미터 개수
g_tParamManager.params[0].param_id         // 첫 번째 파라미터 ID
g_tParamManager.params[0].data[0]          // 첫 번째 파라미터 값

// 패킷 정보
ptAppData->aptChannels[0]->tPacket.tHeader.ulCmd    // 현재 명령
ptAppData->aptChannels[0]->tPacket.tHeader.ulState  // 상태 코드
ptAppData->aptChannels[0]->tPacket.tHeader.ulLen    // 데이터 길이
```

---

### 4️⃣ **Memory Browser로 패킷 확인**

#### Step 1: 패킷 메모리 주소 확인
```
Window → Show View → Memory Browser
```

주소 입력:
```
&ptAppData->aptChannels[0]->tPacket
```

#### Step 2: 헤더 구조 분석
```
Offset | Field   | Size | 현재 값      | 의미
-------|---------|------|-------------|-----
0x00   | ulDest  | 4    | 0x00000020  | Channel 0
0x04   | ulSrc   | 4    | 0x00000000  | netX
0x08   | ulDestId| 4    | 0x00000000  |
0x0C   | ulSrcId | 4    | 0x00000000  |
0x10   | ulLen   | 4    | 0x00000000  | ❌ 0이면 문제!
0x14   | ulId    | 4    | 0x????????  | 요청 ID
0x18   | ulSta   | 4    | 0xC0000004  | ❌ 에러!
0x1C   | ulCmd   | 4    | 0x0000B104  | CIP_SERVICE_IND
```

---

## 🔍 체크리스트

### ✅ 필수 확인 사항

#### 1. Class 등록 확인
```c
// AppDNS_Init() 또는 AppDNS_ApplicationDemoStart()에 추가되었는지 확인
uint32_t AppDNS_Init(APP_DATA_T* ptAppData) {
    // ...

    /* VAT Parameter Manager 초기화 */
    VAT_Param_Init(&g_tParamManager);  // ← ✅ 이게 있는지?

    /* VAT CIP Classes 등록 */
    ulRet = AppDNS_RegisterAllVatClasses(ptAppData);  // ← ✅ 이게 있는지?
    if (ulRet != CIFX_NO_ERROR) {
        printf("❌ Class registration failed!\n");
        return ulRet;
    }
}
```

#### 2. 패킷 핸들러 등록 확인
```c
// switch 문에 case가 있는지 확인
bool AppDNS_PacketHandler_callback(CIFX_PACKET* ptPacket, void* pvUserData) {
    switch(ptPacket->tHeader.ulCmd) {
        case DNS_CMD_CIP_SERVICE_IND:  // ← ✅ 이 case가 있는지?
            AppDNS_HandleCipServiceIndication(ptAppData);
            fPacketHandled = true;
            break;
    }
}
```

#### 3. 링크 에러 확인
```
Build → Build Project (Ctrl+B)
```

확인 사항:
- `g_tParamManager` 정의됨? (`App_VAT_Parameters.c:5`)
- `AppDNS_ExplicitMsg.h` include path 설정됨?
- `App_VAT_Parameters.h` include됨?

---

## 📊 디버깅 시나리오

### Scenario A: 패킷 핸들러가 호출 안됨

**증상**: BP1 브레이크포인트가 걸리지 않음

**확인**:
1. 패킷 핸들러 등록 여부
```c
Pkt_RegisterIndicationHandler(DNS_DEMO_CHANNEL_INDEX,
                               AppDNS_PacketHandler_callback,
                               (void*)ptAppData);
```

2. netX 통신 상태
```c
cifXChannel->ulOpenState == CIFX_CHANNEL_OPEN
```

---

### Scenario B: case DNS_CMD_CIP_SERVICE_IND에 안들어감

**증상**: BP1은 걸리는데 BP2는 안걸림

**확인**:
1. `ptPacket->tHeader.ulCmd` 값
   - Watch 창에서 확인: `ptPacket->tHeader.ulCmd`
   - 예상 값: `0x0000B104` (DNS_CMD_CIP_SERVICE_IND)

2. switch 문에 case 추가됐는지 확인

---

### Scenario C: Class가 등록 안됨

**증상**: UART에 "Class registered successfully" 출력 없음

**확인**:
1. `AppDNS_RegisterAllVatClasses()` 호출 여부
2. 반환값 확인 (0이면 성공)
3. UART 로그:
```
=== Registering VAT CIP Classes ===
✅ Class 0x01 registered successfully
✅ Class 0x30 registered successfully
===================================
```

---

### Scenario D: 파라미터 초기화 안됨

**증상**: `g_tParamManager.param_count == 0`

**확인**:
```c
// Live Expressions에 추가
g_tParamManager.param_count  // ← 99여야 함
g_tParamManager.params[0].param_id  // ← 1이어야 함
```

**해결**:
```c
VAT_Param_Init(&g_tParamManager);  // 호출 확인
```

---

## 🎯 빠른 디버깅 플로우

```
1. 빌드 에러 확인
   ↓ OK
2. UART 출력 확인 → Class 등록 성공?
   ↓ NO → AppDNS_RegisterAllVatClasses() 호출 확인
   ↓ YES
3. BP1 설정 → 패킷 수신됨?
   ↓ NO → netX 통신 상태 확인
   ↓ YES
4. BP2 설정 → case에 진입함?
   ↓ NO → switch case 추가 확인
   ↓ YES
5. service, class_id 값 확인
   ↓
6. 파라미터 데이터 확인
   ↓
7. 응답 전송 확인
```

---

## 🛠️ 추가 디버깅 팁

### 조건부 브레이크포인트

특정 Class만 디버깅:
```
Breakpoint → Conditional
조건: class_id == 0x30
```

특정 Service만 디버깅:
```
조건: service == 0x0E
```

---

### SWV (Serial Wire Viewer) 사용

실시간 printf 출력 확인 (UART 없이)

1. `Run → Debug Configurations`
2. `Debugger` 탭
3. `Enable Serial Wire Viewer` 체크
4. `Window → Show View → SWV → SWV ITM Data Console`

---

## 📋 최종 체크리스트

### 빌드 전
- [ ] `g_tParamManager`가 `App_VAT_Parameters.c`에 정의됨
- [ ] `App_VAT_Parameters.h`에 `extern` 선언됨
- [ ] `AppDNS_ExplicitMsg.h` include path 추가됨
- [ ] 모든 필요한 함수들이 선언됨

### 초기화
- [ ] `VAT_Param_Init(&g_tParamManager)` 호출됨
- [ ] `AppDNS_RegisterAllVatClasses()` 호출됨
- [ ] UART에 등록 성공 메시지 출력됨

### 런타임
- [ ] 패킷 핸들러 콜백 등록됨
- [ ] `case DNS_CMD_CIP_SERVICE_IND` 추가됨
- [ ] CIP 서비스 핸들러 함수 구현됨
- [ ] 응답 패킷 전송 코드 있음

---

## 🔗 관련 파일

| 파일 | 위치 | 역할 |
|------|------|------|
| `App_VAT_Parameters.c` | `Hil_DemoApp/Sources/` | 파라미터 정의 |
| `App_VAT_Parameters.h` | `Hil_DemoApp/Includes/` | 파라미터 헤더 |
| `AppDNS_ExplicitMsg.c` | `Hil_DemoAppDNS/Sources/` | CIP 핸들러 |
| `AppDNS_DemoApplicationFunctions.c` | `Hil_DemoAppDNS/Sources/` | 메인 로직 |
| `.cproject` | 프로젝트 루트 | 빌드 설정 |

---

**작성일**: 2025-11-06
**버전**: 1.0
**대상**: STM32F429 + netX90 DeviceNet

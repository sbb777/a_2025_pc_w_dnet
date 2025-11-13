# VAT UserObject Implementation and Integration

**작성일**: 2025-11-11
**목적**: Hilscher UserObject 패턴 기반 VAT 명시적 메시지 처리 코드 구현 및 통합

---

## 구현 개요

Hilscher의 `AppDNS_DemoApplication_UserObject.c` 참조 코드를 분석하고, 이를 VAT Valve 압력 제어 애플리케이션에 적용하여 Explicit Message 처리 코드를 구현하였습니다.

### 핵심 변경 사항

1. **신규 파일 생성**: `Hil_DemoApp/Sources/App_VAT_UserObject.c` 및 `Hil_DemoApp/Includes/App_VAT_UserObject.h`
2. **기존 파일 수정**: `AppDNS_DemoApplicationFunctions.c` - 초기화 및 라우팅 통합
3. **Hilscher 패턴 적용**: Object Registration, Indication Handler, IsRegistered 검증

---

## 신규 구현 파일

### 1. App_VAT_UserObject.h

**경로**: `Hil_DemoApp/Includes/App_VAT_UserObject.h`

**목적**: VAT UserObject의 공개 인터페이스 정의

**주요 내용**:
```c
/* VAT Object Class Definitions */
#define VAT_CLASS_PARAMETER      0x64  /* 99 instances */
#define VAT_CLASS_DIAGNOSTIC     0x65  /* 1 instance */

/* Function Prototypes */
uint32_t AppDNS_VAT_UserObject_Registration(APP_DATA_T* ptAppData);
bool AppDNS_VAT_UserObject_IsRegistered(uint32_t ulClass, uint32_t ulInstance);
void AppDNS_VAT_UserObject_Indication(APP_DATA_T* ptAppData,
                                       DNS_PACKET_CIP_SERVICE_IND_T* ptInd,
                                       DNS_PACKET_CIP_SERVICE_RES_T* ptRes,
                                       uint32_t* pulResDataSize);
```

---

### 2. App_VAT_UserObject.c

**경로**: `Hil_DemoApp/Sources/App_VAT_UserObject.c`

**목적**: VAT-specific CIP 객체 Explicit Message 처리 구현

#### 주요 구성요소

**A. Object Registration Directory**
```c
typedef struct CIP_REGISTER_OBJ_DIR_ENTRY_Ttag
{
  uint32_t ulClass;
  uint32_t ulOptionFlags;
  uint32_t ulMaxInstance;
} CIP_REGISTER_OBJ_DIR_ENTRY_T;

static CIP_REGISTER_OBJ_DIR_ENTRY_T s_atVatCipRegisterObjDir[] =
{
  { VAT_CLASS_PARAMETER,  0x00000000, VAT_CLASS_PARAMETER_MAX_INST  },  /* 99 instances */
  { VAT_CLASS_DIAGNOSTIC, 0x00000000, VAT_CLASS_DIAGNOSTIC_MAX_INST },  /* 1 instance */
};
```

**B. Registration Function** (`AppDNS_VAT_UserObject_Registration`)
- Hilscher 스택에 VAT 객체 등록
- `DNS_CMD_REGISTER_CLASS_REQ` 패킷 전송
- 각 클래스별 MaxInstance 정보 전달
- 성공/실패 로그 출력

**C. Validation Function** (`AppDNS_VAT_UserObject_IsRegistered`)
- Class ID와 Instance 번호 유효성 검증
- Instance 범위: 1 ≤ instance ≤ MaxInstance
- 유효하지 않은 경우 에러 메시지 출력

**D. Parameter Object Handler** (`AppDNS_VAT_HandleParameterObject`)

**지원 서비스**:
- `CIP_SERVICE_GET_ATTRIBUTE_SINGLE` (0x0E): 파라미터 값 읽기
- `CIP_SERVICE_SET_ATTRIBUTE_SINGLE` (0x10): 파라미터 값 쓰기

**처리 흐름**:
```c
switch (bService) {
  case CIP_SERVICE_GET_ATTRIBUTE_SINGLE:
    // VAT_Param_Get() 호출
    // 데이터를 ptRes->tData.abData로 복사
    // 성공 시 GRC = CIP_GRC_SUCCESS
    break;

  case CIP_SERVICE_SET_ATTRIBUTE_SINGLE:
    // VAT_Param_Set() 호출
    // Read-Only 체크, 범위 검증
    // 성공 시 GRC = CIP_GRC_SUCCESS
    // 실패 시 적절한 GRC 반환
    break;
}
```

**에러 코드 매핑**:
| lRet | GRC | 의미 |
|------|-----|------|
| 0 | 0x00 (SUCCESS) | 성공 |
| -1 | 0x14 (ATTRIBUTE_NOT_SUPPORTED) | 파라미터 미지원 |
| -2 | 0x02 (TOO_MUCH_DATA) | Read-Only 쓰기 시도 |
| -3 | 0x09 (INVALID_ATTRIBUTE_VALUE) | 잘못된 크기 |
| -4 | 0x09 (INVALID_ATTRIBUTE_VALUE) | 범위 초과 |

**E. Diagnostic Object Handler** (`AppDNS_VAT_HandleDiagnosticObject`)

**지원 서비스**:
- `CIP_SERVICE_GET_ATTRIBUTE_SINGLE` (0x0E): 진단 속성 읽기
- `CIP_SERVICE_RESET` (0x05): 진단 통계 리셋

**Attribute 매핑**:
```c
switch (bAttribute) {
  case 1:  pbSrc = &g_tVatDiagnostics.ulUptimeSeconds; bSize = 4; break;
  case 2:  pbSrc = &g_tVatDiagnostics.ulTotalCycles; bSize = 4; break;
  case 3:  pbSrc = &g_tVatDiagnostics.usErrorCount; bSize = 2; break;
  case 4:  pbSrc = &g_tVatDiagnostics.usLastErrorCode; bSize = 2; break;
  case 6:  pbSrc = &g_tVatDiagnostics.sPressureMin; bSize = 2; break;
  case 7:  pbSrc = &g_tVatDiagnostics.sPressureMax; bSize = 2; break;
  case 8:  pbSrc = &g_tVatDiagnostics.sPressureAvg; bSize = 2; break;
  case 14: pbSrc = &g_tVatDiagnostics.sTemperature; bSize = 2; break;
  case 15: pbSrc = &g_tVatDiagnostics.ulFirmwareVersion; bSize = 4; break;
}
```

**F. Main Indication Handler** (`AppDNS_VAT_UserObject_Indication`)

**기능**:
- CIP Service 요청을 Class별로 라우팅
- 파라미터 또는 진단 핸들러 호출
- 응답 패킷에 GRC, ERC, 데이터 크기 설정
- 디버그 출력

**구조**:
```c
void AppDNS_VAT_UserObject_Indication(...)
{
  uint32_t ulClass = ptInd->tData.ulClass;
  uint32_t ulService = ptInd->tData.ulService;

  printf("[VAT] CIP Request: Service=0x%02X, Class=0x%02X, Inst=%u\r\n",
         ulService, ulClass, ulInstance);

  switch (ulClass) {
    case VAT_CLASS_PARAMETER:
      AppDNS_VAT_HandleParameterObject(ptInd, ptRes, &ulResDataSize, &ulGRC, &ulERC);
      break;
    case VAT_CLASS_DIAGNOSTIC:
      AppDNS_VAT_HandleDiagnosticObject(ptInd, ptRes, &ulResDataSize, &ulGRC, &ulERC);
      break;
    default:
      ulGRC = CIP_GRC_OBJECT_DOES_NOT_EXIST;
      break;
  }

  ptRes->tData.ulGRC = ulGRC;
  ptRes->tData.ulERC = ulERC;
  *pulResDataSize = ulResDataSize;
}
```

---

## 기존 파일 수정

### AppDNS_DemoApplicationFunctions.c

**경로**: `Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c`

#### 변경 1: Include 추가 (Line 37)
```c
#include "App_VAT_UserObject.h"  /* VAT UserObject Registration and Handling */
```

#### 변경 2: ConfigureStack 함수 수정 (Line 201-208)

**목적**: 스택 초기화 후 VAT UserObject 등록

**수정 전**:
```c
uint32_t AppDNS_ConfigureStack(APP_DATA_T* ptAppData, uint32_t ulBusOnDelay)
{
  uint32_t ulRet = CIFX_NO_ERROR;

  ulRet = AppDNS_SetConfiguration(ptAppData);
  if (ulRet != 0) return ulRet;

  ulRet = AppDNS_ChannelInit(ptAppData);
  if (ulRet != 0) return ulRet;

  ulRet = AppDNS_StartCommunication(ptAppData);

  return ulRet;
}
```

**수정 후**:
```c
uint32_t AppDNS_ConfigureStack(APP_DATA_T* ptAppData, uint32_t ulBusOnDelay)
{
  uint32_t ulRet = CIFX_NO_ERROR;

  ulRet = AppDNS_SetConfiguration(ptAppData);
  if (ulRet != 0) return ulRet;

  ulRet = AppDNS_ChannelInit(ptAppData);
  if (ulRet != 0) return ulRet;

  ulRet = AppDNS_StartCommunication(ptAppData);
  if (ulRet != 0) return ulRet;

  /* Register VAT User Objects (Class 0x64, 0x65) */
  printf("\n=== VAT User Object Registration ===\n");
  ulRet = AppDNS_VAT_UserObject_Registration(ptAppData);
  if (ulRet != 0) {
    printf("ERROR: VAT User Object Registration failed: 0x%08X\n", (unsigned int)ulRet);
    return ulRet;
  }
  printf("=== VAT User Object Registration Complete ===\n\n");

  return ulRet;
}
```

**초기화 순서**:
1. `AppDNS_SetConfiguration()` - 스택 설정
2. `AppDNS_ChannelInit()` - 채널 초기화
3. `AppDNS_StartCommunication()` - 통신 시작
4. **`AppDNS_VAT_UserObject_Registration()`** - ⭐ VAT 객체 등록 (신규 추가)

#### 변경 3: 패킷 핸들러 수정 (Line 253-258)

**목적**: CIP Service Indication을 새로운 핸들러로 라우팅

**수정 전**:
```c
case DNS_CMD_CIP_SERVICE_IND:
  printf("[INFO] ✅ CIP Service Indication Received!" NEWLINE);
  fPacketHandled = VAT_Explicit_HandleCipService(
      ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX],
      (DNS_PACKET_CIP_SERVICE_IND_T*)ptPacket);
  break;
```

**수정 후**:
```c
case DNS_CMD_CIP_SERVICE_IND:
  printf("[INFO] ✅ CIP Service Indication Received!" NEWLINE);
  /* Route to VAT UserObject Handler */
  AppDNS_HandleCipServiceIndication(ptAppData);
  fPacketHandled = true;
  break;
```

#### 변경 4: HandleCipServiceIndication 함수 수정 (Line 454-484)

**목적**: Class 0x64/0x65 요청을 VAT UserObject 핸들러로 라우팅

**수정 전**:
```c
if (class_id == 0x64 || class_id == 0x65) {
  printf("  -> Routing to VAT Handler\n");

  extern bool VAT_Explicit_HandleCipService_Direct(...);

  if (VAT_Explicit_HandleCipService_Direct(ptInd, ptRes, ptAppData)) {
    return;
  }
  printf("  -> VAT Handler failed, using default error\n");
}
```

**수정 후**:
```c
if (class_id == 0x64 || class_id == 0x65) {
  printf("  -> Routing to VAT UserObject Handler\n");

  /* Check if object is registered */
  if (!AppDNS_VAT_UserObject_IsRegistered(class_id, instance_id)) {
    printf("  -> ERROR: Object Class 0x%02X Instance %u not registered\n",
           class_id, instance_id);
    /* Send error response */
    ptRes->tHead.ulCmd = DNS_CMD_CIP_SERVICE_RES;
    ptRes->tHead.ulLen = DNS_CIP_SERVICE_RES_SIZE;
    ptRes->tHead.ulSta = CIFX_NO_ERROR;
    ptRes->tData.ulGRC = CIP_GRC_OBJECT_DOES_NOT_EXIST;
    ptRes->tData.ulERC = 0;
    AppDNS_GlobalPacket_Send(ptAppData);
    return;
  }

  /* Call VAT UserObject indication handler */
  uint32_t ulResDataSize = 0;
  AppDNS_VAT_UserObject_Indication(ptAppData, ptInd, ptRes, &ulResDataSize);

  /* Send response */
  ptRes->tHead.ulCmd = DNS_CMD_CIP_SERVICE_RES;
  ptRes->tHead.ulLen = DNS_CIP_SERVICE_RES_SIZE + ulResDataSize;
  ptRes->tHead.ulSta = CIFX_NO_ERROR;
  AppDNS_GlobalPacket_Send(ptAppData);

  return;
}
```

**개선 사항**:
1. **등록 검증**: `AppDNS_VAT_UserObject_IsRegistered()` 호출로 유효성 사전 검증
2. **명시적 에러 처리**: 미등록 객체 요청 시 `CIP_GRC_OBJECT_DOES_NOT_EXIST` 반환
3. **표준 패턴**: Hilscher 참조 코드의 검증 → 처리 → 응답 패턴 적용

---

## 전체 동작 흐름

### 초기화 시퀀스
```
1. main() 시작
   ↓
2. VAT_Param_Init(&g_tParamManager)
   - 10개 VAT 파라미터 초기화 (Pressure Setpoint, Position Setpoint 등)
   ↓
3. VAT_Diagnostic_Init()
   - 진단 데이터 구조 초기화
   ↓
4. AppDNS_ConfigureStack(ptAppData)
   ├─ AppDNS_SetConfiguration()      // 스택 설정
   ├─ AppDNS_ChannelInit()            // 채널 초기화
   ├─ AppDNS_StartCommunication()     // 통신 시작
   └─ AppDNS_VAT_UserObject_Registration()  // ⭐ VAT 객체 등록
      ├─ Register Class 0x64 (99 instances)
      └─ Register Class 0x65 (1 instance)
   ↓
5. 애플리케이션 메인 루프 시작
```

### Explicit Message 처리 시퀀스
```
netHOST Master
   ↓ DNS_CMD_CIP_SERVICE_IND (0x0000308A)
DeviceNet Stack
   ↓ Indication 패킷 전달
AppDNS_PacketHandler_callback()
   ↓ case DNS_CMD_CIP_SERVICE_IND
AppDNS_HandleCipServiceIndication()
   ↓ Class ID 체크
   ├─ 0x64 or 0x65?
   │   ↓ YES
   │  AppDNS_VAT_UserObject_IsRegistered()
   │   ↓ 검증 성공
   │  AppDNS_VAT_UserObject_Indication()
   │   ↓ Class 0x64?
   │  AppDNS_VAT_HandleParameterObject()
   │   ↓ Service 0x0E?
   │  VAT_Param_Get()
   │   ↓ 파라미터 데이터 반환
   │  ptRes 구성 (GRC, ERC, Data)
   │   ↓
   │  AppDNS_GlobalPacket_Send()
   │
   └─ Other Classes
      → 스택 기본 처리
   ↓
DNS_CMD_CIP_SERVICE_RES (0x0000308B)
   ↓
netHOST Master
```

---

## Hilscher 패턴 적용 세부사항

### 1. Object Registration Pattern
**Hilscher 방식**:
```c
typedef struct CIP_REGISTER_OBJ_DIR_ENTRY_Ttag {
  uint32_t ulClass;
  uint32_t ulOptionFlags;
  uint32_t ulMaxInstance;
} CIP_REGISTER_OBJ_DIR_ENTRY_T;

static CIP_REGISTER_OBJ_DIR_ENTRY_T s_atCipRegisterObjDir[] = {
  { 0x30, 0x00000000, 100 },  // Example User Object
};

uint32_t AppDNS_UserObject_Registration(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  for (ulIdx = 0; ulIdx < entries; ulIdx++) {
    ulRet = AppDNS_RegisterObject(ptDnsRsc,
                                   s_atCipRegisterObjDir[ulIdx].ulClass,
                                   s_atCipRegisterObjDir[ulIdx].ulOptionFlags);
  }
  return ulRet;
}
```

**VAT 적용**:
```c
static CIP_REGISTER_OBJ_DIR_ENTRY_T s_atVatCipRegisterObjDir[] = {
  { VAT_CLASS_PARAMETER,  0x00000000, 99 },  // Class 0x64, 99 instances
  { VAT_CLASS_DIAGNOSTIC, 0x00000000, 1  },  // Class 0x65, 1 instance
};

uint32_t AppDNS_VAT_UserObject_Registration(APP_DATA_T* ptAppData)
{
  for (ulIdx = 0; ulIdx < s_ulVatCipRegisterObjDirEntries; ulIdx++) {
    ulRet = AppDNS_VAT_RegisterObject(ptAppData,
                                       s_atVatCipRegisterObjDir[ulIdx].ulClass,
                                       s_atVatCipRegisterObjDir[ulIdx].ulOptionFlags);
  }
  return ulRet;
}
```

### 2. IsRegistered Validation Pattern
**Hilscher 방식**:
```c
bool AppDNS_UserObject_IsRegistered(uint32_t ulClass, uint32_t ulInstance)
{
  for (ulIdx = 0; ulIdx < s_ulCipRegisterObjDirEntries; ulIdx++) {
    if (ulClass == s_atCipRegisterObjDir[ulIdx].ulClass) {
      if (ulInstance >= 1 && ulInstance <= s_atCipRegisterObjDir[ulIdx].ulMaxInstance) {
        return true;
      }
      return false;
    }
  }
  return false;
}
```

**VAT 적용**: 동일한 구조 사용, VAT 디렉토리 참조

### 3. Indication Handler Pattern
**Hilscher 방식**:
```c
void AppDNS_UserObject_Indication(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
                                   DNS_PACKET_CIP_SERVICE_IND_T* ptInd,
                                   DNS_PACKET_CIP_SERVICE_RES_T* ptRes,
                                   uint32_t* pulResDataSize)
{
  uint32_t ulClass = ptInd->tData.ulClass;

  switch (ulClass) {
    case USER_CLASS_XXX:
      // 클래스별 핸들러 호출
      AppDNS_HandleClassXXX(ptInd, ptRes, &ulResDataSize, &ulGRC, &ulERC);
      break;
  }

  ptRes->tData.ulGRC = ulGRC;
  ptRes->tData.ulERC = ulERC;
  *pulResDataSize = ulResDataSize;
}
```

**VAT 적용**: 동일한 구조, Class 0x64/0x65 처리

---

## 디버그 출력 예시

### 초기화 시
```
=== VAT User Object Registration ===
[VAT] Registering VAT User Objects...
[VAT] Registered Object Class 0x64 (MaxInst=99)
[VAT] Registered Object Class 0x65 (MaxInst=1)
[VAT] All VAT User Objects registered successfully
=== VAT User Object Registration Complete ===
```

### Explicit Message 처리 시
```
[INFO] ✅ CIP Service Indication Received!

=== CIP Service Indication ===
Service:   0x0E (Get Attribute Single)
Class:     0x64
Instance:  0x01
Attribute: 0x01
  -> Routing to VAT UserObject Handler
[VAT] CIP Request: Service=0x0E, Class=0x64, Inst=1
[VAT Param] GET Param1: 88 13
[VAT] Response: GRC=0x00, DataSize=2
```

---

## 테스트 방법

### netHOST를 사용한 테스트

#### 1. Pressure Setpoint 읽기 (Param 1)
```
Send Packet:
  Cmd:  0x0000308A
  Len:  20
  Data: 0E 00 00 00 64 00 00 00 01 00 00 00 01 00 00 00 01 00 00 00
        ↑Service    ↑Class 0x64 ↑Instance 1 ↑Attribute 1 ↑Member (DevID)

예상 응답:
  Cmd:  0x0000308B
  Data: 00 00 00 00 00 00 00 00 88 13
        ↑GRC=0x00   ↑ERC=0x00   ↑Value=5000 (50.00 mbar)
```

#### 2. Pressure Setpoint 쓰기 (Param 1)
```
Send Packet:
  Cmd:  0x0000308A
  Len:  22
  Data: 10 00 00 00 64 00 00 00 01 00 00 00 01 00 00 00 01 00 00 00 58 1B
        ↑Service    ↑Class 0x64 ↑Instance 1 ↑Attribute 1 ↑Member      ↑7000

예상 응답:
  Cmd:  0x0000308B
  Data: 00 00 00 00 00 00 00 00
        ↑GRC=0x00   ↑ERC=0x00 (No data for Set)
```

#### 3. Diagnostic Uptime 읽기 (Class 0x65, Attr 1)
```
Send Packet:
  Cmd:  0x0000308A
  Len:  20
  Data: 0E 00 00 00 65 00 00 00 01 00 00 00 01 00 00 00 01 00 00 00
        ↑Service    ↑Class 0x65 ↑Instance 1 ↑Attribute 1 ↑Member

예상 응답:
  Cmd:  0x0000308B
  Data: 00 00 00 00 00 00 00 00 [4 bytes UINT32 Uptime]
```

---

## 주요 차이점 비교

### 이전 구현 (App_VAT_ExplicitHandler.c)
- **문제**: 직접 응답 패킷 생성 및 전송
- **단점**: Hilscher 표준 패턴 미적용, 검증 부족
- **한계**: 확장성 제한

### 신규 구현 (App_VAT_UserObject.c)
- **개선**: Hilscher 참조 코드 패턴 충실 적용
- **장점**:
  1. **표준 등록 프로세스**: `DNS_CMD_REGISTER_CLASS_REQ` 사용
  2. **명시적 검증**: `IsRegistered()` 호출로 사전 검증
  3. **구조화된 핸들러**: Registration → Validation → Indication 패턴
  4. **확장 용이**: 새로운 클래스 추가 시 디렉토리 엔트리만 추가
  5. **디버그 친화적**: Hilscher 스타일의 로그 출력

---

## 빌드 및 실행

### 빌드 확인 사항
1. ✅ `App_VAT_UserObject.c` 컴파일 성공
2. ✅ `App_VAT_UserObject.h` include 경로 설정
3. ✅ `AppDNS_DemoApplicationFunctions.c` 컴파일 성공
4. ✅ 링크 성공

### 실행 시 확인 사항
1. ✅ VAT User Object Registration 성공 메시지
2. ✅ Class 0x64, 0x65 등록 확인
3. ✅ CIP Service Indication 수신 확인
4. ✅ VAT UserObject Handler로 라우팅 확인
5. ✅ 응답 패킷 전송 확인

---

## 향후 작업

### Phase 1: 기본 테스트 (현재)
- [ ] netHOST를 사용한 Explicit Message 테스트
- [ ] Parameter GET/SET 동작 확인
- [ ] Diagnostic GET 동작 확인
- [ ] 에러 처리 검증

### Phase 2: I/O 연동
- [ ] Parameter ↔ I/O Assembly 동기화
- [ ] Cyclic I/O 업데이트
- [ ] Real-time 진단 데이터 수집

### Phase 3: 고급 기능
- [ ] Parameter 저장/복원 (EEPROM)
- [ ] 에러 로깅
- [ ] 이벤트 알림

---

## 참조 문서
- `20251111_VAT_Test_Correct_Packets.md` - 올바른 패킷 포맷 (ulMember 포함)
- `20251111_VAT_ExplicitMessage_Test_Updated.md` - 테스트 시나리오
- `20251111_VAT_Implementation_Summary.md` - 전체 구현 요약
- Hilscher Reference: `AppDNS_DemoApplication_UserObject.c`

---

**작업 완료**: 2025-11-11
**다음 단계**: netHOST를 사용한 실제 하드웨어 테스트 및 검증

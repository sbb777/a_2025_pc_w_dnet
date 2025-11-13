# DeviceNet CIP 에러 0xC0000004 수정 완료

## 🔴 문제 증상
```
State:  0xC0000004 (ERR_HIL_UNKNOWN_COMMAND)
Len:    0
```

## 🔍 원인 분석

### 핵심 원인
**`DNS_HOST_APP_REGISTRATION` 매크로 미정의**

netX DeviceNet 스택에서 CIP Indication 패킷을 애플리케이션으로 전달하려면, 애플리케이션이 스택에 명시적으로 등록되어야 합니다.

### 코드 분석
`AppDNS_DemoApplication.c:144-150`
```c
#ifdef DNS_HOST_APP_REGISTRATION
    /* Register application */
    App_SysPkt_AssembleRegisterAppReq(...);  // ← 이게 실행 안됨!
    ulRet = Pkt_SendReceivePacket(...);
    if (ulRet != CIFX_NO_ERROR)
      return ulRet;
#endif
```

**결과**: 매크로가 정의되지 않아 애플리케이션 등록 코드가 컴파일되지 않음 → netX가 CIP 요청을 앱으로 전달 안함 → Unknown Command 에러

---

## ✅ 수정 내역

### 1. 헤더 파일 include 추가
**파일**: `Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c:35`
```c
#include "App_VAT_Parameters.h"  // ← 추가
```

### 2. 파라미터 초기화 호출 추가
**파일**: `Hil_DemoAppDNS/Sources/AppDNS_DemoApplicationFunctions.c:188-190`
```c
/* ⭐ Initialize VAT Parameter Manager ⭐ */
VAT_Param_Init(&g_tParamManager);
PRINTF("VAT Parameter Manager initialized with %d parameters\n", g_tParamManager.param_count);
```

### 3. DNS_HOST_APP_REGISTRATION 매크로 정의 (핵심 수정!)
**파일**: `Hil_DemoAppDNS/Sources/AppDNS_DemoApplication.c:21-22`
```c
/* ⭐ Enable Application Registration for CIP Service Handling ⭐ */
#define DNS_HOST_APP_REGISTRATION
```

### 4. g_tParamManager static 제거
**파일**: `Hil_DemoApp/Sources/App_VAT_Parameters.c:5`
```c
// Before: static VAT_PARAM_MANAGER_T g_tParamManager;
// After:
VAT_PARAM_MANAGER_T g_tParamManager;  // ← static 제거
```

### 5. extern 선언 추가
**파일**: `Hil_DemoApp/Includes/App_VAT_Parameters.h:136`
```c
/* Global Parameter Manager instance */
extern VAT_PARAM_MANAGER_T g_tParamManager;
```

### 6. Include path 추가
**파일**: `.cproject` (Build Settings)
```
Hil_DemoAppDNS/includes  ← 추가됨
```

---

## 🔧 동작 원리

### Before (에러)
```
1. 마스터에서 CIP Get/Set 요청 전송
   ↓
2. netX 스택 수신
   ↓
3. 애플리케이션 등록 확인 → ❌ 등록 안됨!
   ↓
4. ERR_HIL_UNKNOWN_COMMAND (0xC0000004) 반환
```

### After (정상)
```
1. 앱 시작 시 DNS_HOST_APP_REGISTRATION 정의됨
   ↓
2. App_SysPkt_AssembleRegisterAppReq() 실행
   ↓
3. 애플리케이션이 netX 스택에 등록됨 ✅
   ↓
4. 마스터에서 CIP 요청 전송
   ↓
5. netX 스택이 DNS_CMD_CIP_SERVICE_IND 전달
   ↓
6. AppDNS_PacketHandler_callback() 호출
   ↓
7. case DNS_CMD_CIP_SERVICE_IND: 실행
   ↓
8. AppDNS_HandleCipServiceIndication() 처리
   ↓
9. CIP_HandleGetAttributeSingle() 실행
   ↓
10. 파라미터 데이터 반환 ✅
```

---

## 📋 빌드 및 테스트

### 1. Clean & Build
```
Project → Clean
Project → Build Project (Ctrl+B)
```

### 2. 다운로드
```
Run → Debug (F11)
```

### 3. UART 로그 확인
**예상 출력**:
```
VAT Parameter Manager initialized with 99 parameters

=== Registering VAT CIP Classes ===
✅ Class 0x01 registered successfully
✅ Class 0x30 registered successfully
===================================
```

### 4. CIP 요청 테스트

#### Test 1: Get Param1 (Vendor ID)
**요청**: `0E 03 20 01 24 01 30 01`
**예상 응답**:
```
State:  0x00000000 (SUCCESS)  ✅
Len:    2
Data:   94 01 (404 = 0x0194)
```

**UART 출력**:
```
=== CIP Service Indication ===
Service:   0x0E (Get Attribute Single)
Class:     0x01
Instance:  0x01
Attribute: 0x01
Data Len:  0
  -> Get: Success, Data=94 01
```

---

## 📊 수정 파일 목록

| 파일 | 수정 내용 | 중요도 |
|------|-----------|--------|
| `AppDNS_DemoApplication.c` | `DNS_HOST_APP_REGISTRATION` 추가 | ⭐⭐⭐ 핵심! |
| `AppDNS_DemoApplicationFunctions.c` | `VAT_Param_Init()` 호출, include 추가 | ⭐⭐⭐ |
| `App_VAT_Parameters.c` | `static` 제거 | ⭐⭐ |
| `App_VAT_Parameters.h` | `extern` 선언 추가 | ⭐⭐ |
| `.cproject` | Include path 추가 | ⭐ |

---

## 🎯 성공 체크리스트

### 빌드
- [ ] 0 errors, 0 warnings
- [ ] `DNS_HOST_APP_REGISTRATION` 정의됨

### 초기화
- [ ] UART에 "VAT Parameter Manager initialized" 출력
- [ ] UART에 "Class 0x01 registered successfully" 출력
- [ ] UART에 "Class 0x30 registered successfully" 출력

### CIP 통신
- [ ] Get 요청: State = 0x00000000 (SUCCESS)
- [ ] Get 요청: Len > 0 (데이터 수신됨)
- [ ] UART에 "CIP Service Indication" 출력
- [ ] UART에 "Get: Success" 출력

---

## 🐛 트러블슈팅

### Q1: 여전히 0xC0000004 에러
**확인사항**:
1. `#define DNS_HOST_APP_REGISTRATION`가 추가됐는지 확인
2. Clean & Rebuild 했는지 확인
3. 새 바이너리를 다운로드했는지 확인

### Q2: UART에 로그가 안나옴
**확인사항**:
1. UART 연결 확인 (Baud: 115200)
2. `printf` 리디렉션 확인
3. 시리얼 터미널 프로그램 실행 확인

### Q3: Class 등록 실패
**확인사항**:
1. netX 통신 상태 확인
2. `AppDNS_ConfigureStack()` 호출 순서 확인
3. Firmware 버전 확인

---

## 📚 관련 문서

- `STM32_DeviceNet_Debug_Guide.md` - 디버깅 가이드
- `VAT_CIP_Test_Procedure.md` - 테스트 절차
- Hilscher DeviceNet Protocol API Manual

---

**작성일**: 2025-11-06
**작성자**: Claude Code Assistant
**버전**: 1.0
**상태**: 수정 완료 ✅

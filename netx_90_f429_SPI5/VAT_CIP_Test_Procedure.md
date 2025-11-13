# VAT CIP Explicit Message 테스트 절차

## 📋 전체 테스트 흐름

```
1. 코드 적용 (5분)
   ↓
2. 빌드 (2분)
   ↓
3. 다운로드 (1분)
   ↓
4. UART 확인 (초기화 로그)
   ↓
5. netHost 테스트 (10분)
   ↓
6. 결과 분석
```

---

## 🔧 1단계: 코드 적용 (5분)

### 1.1 파일 열기
```
D:\git\netx_90_f429_SPI5\Hil_DemoAppDNS\Sources\AppDNS_DemoApplicationFunctions.c
```

### 1.2 헤더 추가 (파일 상단)
**기존 includes 섹션에 추가**:
```c
/* 기존 includes */
#include "DNS_Includes.h"
#include "AppDNS_DemoApplication.h"
...

/* ⭐ 여기에 추가 ⭐ */
#include "DNS_packet_register_class.h"
#include "DNS_packet_cip_service.h"
#include "AppDNS_ExplicitMsg.h"
```

### 1.3 함수 추가 (파일 끝부분)
**`VAT_CIP_Complete_Fix.c`에서 복사**:
- `AppDNS_RegisterClass()`
- `AppDNS_RegisterAllVatClasses()`
- `AppDNS_HandleCipServiceIndication()`

### 1.4 PacketHandler 수정
**기존 `AppDNS_PacketHandler_callback()` 함수 찾기** (약 209줄):
```c
switch( ptPacket->tHeader.ulCmd )
{
#ifdef DNS_HOST_APP_REGISTRATION
 #error Error no indication packet handling implemented!
#endif

  /* ⭐ 여기에 추가 ⭐ */
  case DNS_CMD_CIP_SERVICE_IND:
    AppDNS_HandleCipServiceIndication(ptAppData);
    fPacketHandled = true;
    break;

  default:
  {
    // 기존 코드 유지
  }
}
```

### 1.5 초기화 수정
**`AppDNS_DemoApplication.c` 파일 열기**

**`AppDNS_Init()` 함수 찾기**:
```c
uint32_t AppDNS_Init(APP_DATA_T* ptAppData)
{
  uint32_t ulRet = CIFX_NO_ERROR;

  /* ... Set Configuration ... */
  ulRet = AppDNS_SetConfiguration(ptAppData);
  if (ulRet != CIFX_NO_ERROR) {
    return ulRet;
  }

  /* ⭐ 여기에 추가 ⭐ */
  /* Register VAT CIP Classes */
  ulRet = AppDNS_RegisterAllVatClasses(ptAppData);
  if (ulRet != CIFX_NO_ERROR) {
    PRINTF("Failed to register CIP classes!\n");
    return ulRet;
  }

  /* ... 나머지 초기화 ... */
}
```

---

## 🔨 2단계: 빌드 (2분)

### 2.1 프로젝트 빌드
**IDE에서**:
```
Build → Build All
또는
Ctrl + B
```

### 2.2 빌드 성공 확인
**예상 출력**:
```
Building target: netx90_f429_spi5.elf
Finished building target: netx90_f429_spi5.elf

Build Finished. 0 errors, 0 warnings.
```

### 2.3 에러 발생 시
**컴파일 에러 체크**:
- `DNS_CMD_CIP_SERVICE_IND` 선언 확인
- `AppDNS_ExplicitMsg.h` include 확인
- 함수 프로토타입 선언 확인

---

## 📥 3단계: 다운로드 (1분)

### 3.1 보드 연결 확인
- USB 케이블 연결
- 전원 ON
- ST-Link 연결 확인

### 3.2 펌웨어 다운로드
```
Debug → Download
또는
F8
```

### 3.3 실행
```
Debug → Run
또는
F5
```

---

## 📺 4단계: UART 출력 확인 (초기화)

### 4.1 터미널 설정
**설정**:
- Port: COM? (Device Manager 확인)
- Baud: 115200
- Data: 8 bit
- Stop: 1 bit
- Parity: None

### 4.2 예상 초기화 로그
```
=== DeviceNet Slave Initialization ===
...
Set Configuration: Success

=== Registering VAT CIP Classes ===
✅ Class 0x01 registered successfully
✅ Class 0x30 registered successfully
===================================

Create Assembly: Success
...
Initialization Complete!
```

### 4.3 에러 발생 시
**에러 예**:
```
❌ Failed to register Class 0x01, error: 0x????????
```

**확인 사항**:
- DeviceNet 스택 초기화 완료 여부
- Set Configuration 성공 여부
- 패킷 통신 정상 여부

---

## 🧪 5단계: netHost 테스트 (10분)

### Test 1: Get Param1 (Vendor ID 읽기)

#### 5.1 netHost 전송
**Explicit Message Tool에서**:
```
Cmd:    0x00008020
Len:    0x00000008
Data:   0E 03 20 01 24 01 30 01
```

**또는 간편 입력**:
```
Service:   Get Attribute Single (0x0E)
Class:     0x01 (Identity)
Instance:  0x01
Attribute: 0x01 (Vendor ID)
```

#### 5.2 UART 출력 (예상)
```
=== CIP Service Indication ===
Service:   0x0E (Get Attribute Single)
Class:     0x01
Instance:  0x01
Attribute: 0x01
Data Len:  0
  -> Get: Success, Data=94 01
Response:  Cmd=0x0000B105, Len=30, Error=0x00
==============================
```

#### 5.3 netHost 수신 (예상)
```
State:  0x00000000  ✅ Success!
Len:    0x00000006
Data:   8E 00 00 00 94 01

해석: Vendor ID = 0x0194 = 404 ✅
```

---

### Test 2: Set Param6 (Controller Mode 쓰기)

#### 5.4 netHost 전송
```
Cmd:    0x00008020
Len:    0x00000009
Data:   10 03 20 30 24 01 30 0C 05
```

#### 5.5 UART 출력 (예상)
```
=== CIP Service Indication ===
Service:   0x10 (Set Attribute Single)
Class:     0x30
Instance:  0x01
Attribute: 0x0C
Data Len:  1
  Data: 05
  -> Set: Success
Response:  Cmd=0x0000B105, Len=28, Error=0x00
==============================
```

#### 5.6 netHost 수신 (예상)
```
State:  0x00000000  ✅ Success!
Len:    0x00000004
Data:   90 00 00 00

해석: Set 성공 ✅
```

---

### Test 3: Get Param6 (검증)

#### 5.7 netHost 전송
```
Cmd:    0x00008020
Len:    0x00000008
Data:   0E 03 20 30 24 01 30 0C
```

#### 5.8 UART 출력 (예상)
```
=== CIP Service Indication ===
Service:   0x0E (Get Attribute Single)
Class:     0x30
Instance:  0x01
Attribute: 0x0C
Data Len:  0
  -> Get: Success, Data=05
Response:  Cmd=0x0000B105, Len=29, Error=0x00
==============================
```

#### 5.9 netHost 수신 (예상)
```
State:  0x00000000  ✅
Len:    0x00000005
Data:   8E 00 00 00 05

해석: Controller Mode = 5 ✅ (변경 확인!)
```

---

### Test 4: Set Read-Only (에러 테스트)

#### 5.10 netHost 전송
```
Cmd:    0x00008020
Len:    0x0000000A
Data:   10 03 20 01 24 01 30 01 99 99
```

#### 5.11 UART 출력 (예상)
```
=== CIP Service Indication ===
Service:   0x10 (Set Attribute Single)
Class:     0x01
Instance:  0x01
Attribute: 0x01
Data Len:  2
  Data: 99 99
  -> Set: Error 0x0E
Response:  Cmd=0x0000B105, Len=28, Error=0x0E
==============================
```

#### 5.12 netHost 수신 (예상)
```
State:  0x00000000
Len:    0x00000004
Data:   90 00 0E 00

해석: Error 0x0E (Attribute Not Settable) ✅ 정상!
```

---

## ✅ 6단계: 결과 분석

### 성공 체크리스트

#### 초기화
- [ ] Class 0x01 등록 성공
- [ ] Class 0x30 등록 성공
- [ ] UART에 등록 메시지 출력

#### Test 1: Get Param1
- [ ] UART에 Indication 출력
- [ ] netHost State = 0x00000000
- [ ] netHost Len = 6
- [ ] Data = 94 01 (404)

#### Test 2: Set Param6
- [ ] UART에 Indication 출력
- [ ] UART에 Set Success
- [ ] netHost State = 0x00000000
- [ ] netHost Len = 4
- [ ] Data = 90 00 00 00

#### Test 3: Get Param6
- [ ] netHost Len = 5
- [ ] Data = 8E 00 00 00 05 (값=5)

#### Test 4: Read-Only 에러
- [ ] netHost Len = 4
- [ ] Data = 90 00 0E 00 (Error 0x0E)

---

## 🐛 문제 해결

### Q1: Class 등록 실패
**증상**:
```
❌ Failed to register Class 0x01, error: 0x????????
```

**해결**:
1. DeviceNet 스택 초기화 완료 확인
2. Set Configuration 성공 확인
3. 패킷 통신 타임아웃 확인

### Q2: CIP Indication이 오지 않음
**증상**: UART에 "CIP Service Indication" 출력 없음

**해결**:
1. `AppDNS_PacketHandler_callback()` 수정 확인
2. `case DNS_CMD_CIP_SERVICE_IND:` 추가 확인
3. 빌드 후 다시 다운로드

### Q3: 여전히 State=0xC0000004
**증상**: Class 등록은 성공했는데 여전히 에러

**해결**:
1. Class 등록이 먼저 되었는지 확인
2. `AppDNS_RegisterAllVatClasses()`가 `AppDNS_Init()`에서 호출되는지 확인
3. UART 로그에서 등록 순서 확인

### Q4: 응답이 없음 (Len=0)
**증상**: State는 정상인데 Len=0

**해결**:
1. `AppDNS_HandleCipServiceIndication()` 함수 확인
2. `AppDNS_GlobalPacket_Send()` 호출 확인
3. `ptRes->tHead.ulLen` 설정 확인

---

## 📊 빠른 테스트 요약표

| Test | 전송 Data | 예상 수신 Data | 의미 |
|------|-----------|----------------|------|
| 1️⃣ Get Param1 | `0E 03 20 01 24 01 30 01` | `8E 00 00 00 94 01` | Vendor=404 |
| 2️⃣ Set Param6 | `10 03 20 30 24 01 30 0C 05` | `90 00 00 00` | Success |
| 3️⃣ Get Param6 | `0E 03 20 30 24 01 30 0C` | `8E 00 00 00 05` | Mode=5 |
| 4️⃣ Set RO | `10 03 20 01 24 01 30 01 99 99` | `90 00 0E 00` | Error 0x0E |

---

## 🎯 최종 확인

### 모든 테스트 성공 시
```
✅ Class 등록: OK
✅ Get 동작: OK
✅ Set 동작: OK
✅ 에러 처리: OK

🎉 Explicit Message 완전 동작! 🎉
```

### 다음 단계
1. 나머지 파라미터 초기화 (Param2-99)
2. 추가 Assembly 테스트
3. Flash Save/Load 테스트
4. 장시간 안정성 테스트

---

**작성일**: 2025-11-05
**버전**: 1.0
**예상 소요 시간**: 약 20분

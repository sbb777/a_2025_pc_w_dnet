# PrintDeviceInformation 함수 복구 작업

**작업 일시**: 2025-10-28
**작업자**: Claude Code
**작업 목적**: 누락된 디바이스 정보 출력 함수 복구 및 호출 코드 추가

---

## 문제 상황

### 증상
```
*** Channel ready! ***
  COS Flags: 0x00000001
  Time taken: 0 seconds

========================================
 Running VAT Test Mode: 1
========================================
```

**문제**: 채널 준비 완료 후 바로 VAT 테스트가 실행되며, 디바이스 정보(펌웨어 이름, 버전 등)가 출력되지 않음

---

### 원인 분석

1. **함수 프로토타입 누락**
   - `PrintDeviceInformation()` 함수 선언 없음

2. **함수 구현 누락**
   - 디바이스 정보 조회 및 출력 로직 없음

3. **함수 호출 누락**
   - 채널 준비 후 정보 출력 호출 없음

**추정 원인**: 이전 편집 과정에서 코드가 롤백되거나 linter에 의해 제거됨

---

## 수정 내용

### 파일: `Core/Src/main.c`

#### 1. 함수 프로토타입 추가 (135줄)

**변경 전**:
```c
/* ========== VAT TEST INTEGRATION - 추가 시작 ========== */
static void VAT_InitializeTest(void);
static void VAT_RunTest(CIFXHANDLE hChannel);
/* ========== VAT TEST INTEGRATION - 추가 끝 ========== */
```

**변경 후**:
```c
/* ========== VAT TEST INTEGRATION - 추가 시작 ========== */
static void PrintDeviceInformation(CIFXHANDLE hDriver, CIFXHANDLE hChannel);
static void VAT_InitializeTest(void);
static void VAT_RunTest(CIFXHANDLE hChannel);
/* ========== VAT TEST INTEGRATION - 추가 끝 ========== */
```

---

#### 2. 함수 구현 추가 (262-345줄)

**위치**: `VAT_InitializeTest()` 함수 이전

```c
/**
 * @brief 디바이스 정보 출력
 *
 * @param hDriver   cifX 드라이버 핸들
 * @param hChannel  cifX 채널 핸들
 *
 * 드라이버, 보드, 채널 정보를 조회하여 출력
 */
static void PrintDeviceInformation(CIFXHANDLE hDriver, CIFXHANDLE hChannel)
{
    int32_t lRet = CIFX_NO_ERROR;

    printf("\r\n");
    printf("========================================\r\n");
    printf(" Device Information\r\n");
    printf("========================================\r\n");

    /* 1. 드라이버 정보 */
    DRIVER_INFORMATION tDriverInfo;
    lRet = xDriverGetInformation(hDriver, sizeof(DRIVER_INFORMATION), &tDriverInfo);
    if(lRet == CIFX_NO_ERROR)
    {
        printf("\r\n[Driver Information]\r\n");
        printf("  Driver Version:  %s\r\n", tDriverInfo.abDriverVersion);
        printf("  Board Count:     %lu\r\n", tDriverInfo.ulBoardCnt);
    }
    else
    {
        printf("\r\n[Driver Information] ERROR: 0x%08X\r\n", (unsigned int)lRet);
    }

    /* 2. 보드 정보 */
    BOARD_INFORMATION tBoardInfo;
    lRet = xDriverEnumBoards(hDriver, 0, sizeof(BOARD_INFORMATION), &tBoardInfo);
    if(lRet == CIFX_NO_ERROR)
    {
        printf("\r\n[Board Information]\r\n");
        printf("  Board Name:      %s\r\n", tBoardInfo.abBoardName);
        printf("  Board Alias:     %s\r\n", tBoardInfo.abBoardAlias);
        printf("  Board ID:        0x%08lX\r\n", tBoardInfo.ulBoardID);
        printf("  Device Number:   %lu\r\n", tBoardInfo.tSystemInfo.ulDeviceNumber);
        printf("  Serial Number:   %lu\r\n", tBoardInfo.tSystemInfo.ulSerialNumber);
        printf("  DPM Size:        %lu bytes\r\n", tBoardInfo.ulDpmTotalSize);
        printf("  Channel Count:   %lu\r\n", tBoardInfo.ulChannelCnt);
        printf("  HW Revision:     %u\r\n", tBoardInfo.tSystemInfo.bHwRevision);
        printf("  Device Class:    0x%04X\r\n", tBoardInfo.tSystemInfo.usDeviceClass);
    }
    else
    {
        printf("\r\n[Board Information] ERROR: 0x%08X\r\n", (unsigned int)lRet);
    }

    /* 3. 채널 정보 */
    CHANNEL_INFORMATION tChannelInfo;
    lRet = xChannelInfo(hChannel, sizeof(CHANNEL_INFORMATION), &tChannelInfo);
    if(lRet == CIFX_NO_ERROR)
    {
        printf("\r\n[Channel Information]\r\n");
        printf("  Board Name:      %s\r\n", tChannelInfo.abBoardName);
        printf("  Device Number:   %lu\r\n", tChannelInfo.ulDeviceNumber);
        printf("  Serial Number:   %lu\r\n", tChannelInfo.ulSerialNumber);

        printf("\r\n[Firmware Information]\r\n");
        printf("  Firmware Name:   %.*s\r\n",
               tChannelInfo.bFWNameLength,
               tChannelInfo.abFWName);
        printf("  FW Version:      %u.%u.%u.%u\r\n",
               tChannelInfo.usFWMajor,
               tChannelInfo.usFWMinor,
               tChannelInfo.usFWBuild,
               tChannelInfo.usFWRevision);
        printf("  FW Build Date:   %04u-%02u-%02u\r\n",
               tChannelInfo.usFWYear,
               tChannelInfo.bFWMonth,
               tChannelInfo.bFWDay);

        printf("\r\n[Channel Status]\r\n");
        printf("  Open Count:      %lu\r\n", tChannelInfo.ulOpenCnt);
        printf("  Mailbox Size:    %lu bytes\r\n", tChannelInfo.ulMailboxSize);
        printf("  IO Input Areas:  %lu\r\n", tChannelInfo.ulIOInAreaCnt);
        printf("  IO Output Areas: %lu\r\n", tChannelInfo.ulIOOutAreaCnt);
        printf("  netX Flags:      0x%08lX\r\n", tChannelInfo.ulNetxFlags);
        printf("  Host Flags:      0x%08lX\r\n", tChannelInfo.ulHostFlags);
        printf("  Device COS:      0x%08lX\r\n", tChannelInfo.ulDeviceCOSFlags);
    }
    else
    {
        printf("\r\n[Channel Information] ERROR: 0x%08X\r\n", (unsigned int)lRet);
    }

    printf("========================================\r\n\r\n");
}
```

---

#### 3. 함수 호출 추가 (660-661줄)

**변경 전**:
```c
printf("\r\n*** Channel ready! ***\r\n");
printf("  COS Flags: 0x%08X\r\n",
       (unsigned int)tChannelInfo.ulDeviceCOSFlags);
printf("  Time taken: %lu seconds\r\n\r\n", timeout_count / 10);

/* VAT 테스트 실행 */
VAT_RunTest(hChannel);
```

**변경 후**:
```c
printf("\r\n*** Channel ready! ***\r\n");
printf("  COS Flags: 0x%08X\r\n",
       (unsigned int)tChannelInfo.ulDeviceCOSFlags);
printf("  Time taken: %lu seconds\r\n\r\n", timeout_count / 10);

/* 디바이스 정보 출력 */
PrintDeviceInformation(hDriver, hChannel);

/* VAT 테스트 실행 */
VAT_RunTest(hChannel);
```

---

## 함수 기능 상세

### PrintDeviceInformation()

**목적**: netX90 DeviceNet 마스터의 상세 정보 조회 및 출력

**매개변수**:
- `hDriver`: cifX 드라이버 핸들
- `hChannel`: cifX 채널 핸들

**동작 순서**:

1. **드라이버 정보 조회**
   ```c
   xDriverGetInformation(hDriver, sizeof(DRIVER_INFORMATION), &tDriverInfo);
   ```
   - 드라이버 버전 문자열
   - 등록된 보드 수

2. **보드 정보 조회**
   ```c
   xDriverEnumBoards(hDriver, 0, sizeof(BOARD_INFORMATION), &tBoardInfo);
   ```
   - 보드 이름 및 별칭
   - 보드 ID
   - 디바이스 번호 및 시리얼 번호
   - DPM 크기
   - 채널 수
   - 하드웨어 리비전
   - 디바이스 클래스

3. **채널 정보 조회**
   ```c
   xChannelInfo(hChannel, sizeof(CHANNEL_INFORMATION), &tChannelInfo);
   ```
   - 펌웨어 이름 (최대 63바이트)
   - 펌웨어 버전 (Major.Minor.Build.Revision)
   - 펌웨어 빌드 날짜
   - 채널 상태
   - 메일박스 크기
   - IO 영역 수
   - 플래그 정보

---

## 예상 출력

### 정상 동작 시

```
*** Channel ready! ***
  COS Flags: 0x00000001
  Time taken: 0 seconds

========================================
 Device Information
========================================

[Driver Information]
  Driver Version:  cifX Toolkit V1.6.0.0
  Board Count:     1

[Board Information]
  Board Name:      cifX0
  Board Alias:
  Board ID:        0x00000001
  Device Number:   12345
  Serial Number:   67890
  DPM Size:        32768 bytes
  Channel Count:   1
  HW Revision:     1
  Device Class:    0x0090

[Channel Information]
  Board Name:      cifX0
  Device Number:   12345
  Serial Number:   67890

[Firmware Information]
  Firmware Name:   DeviceNet Slave V5.7.0.0
  FW Version:      5.7.0.0
  FW Build Date:   2024-03-15

[Channel Status]
  Open Count:      1
  Mailbox Size:    1596 bytes
  IO Input Areas:  1
  IO Output Areas: 1
  netX Flags:      0x00000001
  Host Flags:      0x00000001
  Device COS:      0x00000001
========================================

========================================
 Running VAT Test Mode: 1
========================================

Test: Basic Pressure Control
Cycles: 10
Setpoint: 500
...
```

---

### Mock 함수 사용 시

```
[Firmware Information]
  Firmware Name:                          ← 비어있음
  FW Version:      0.0.0.0                ← 모두 0
  FW Build Date:   0000-00-00             ← 모두 0
```

**해결 방법**:
```c
// Project Properties → C/C++ Build → Settings →
// MCU GCC Compiler → Preprocessor → Defined symbols
// 추가: USE_CIFX_API
```

---

### 에러 발생 시

```
[Driver Information] ERROR: 0xC0000001
```

**가능한 원인**:
- 드라이버 핸들 무효
- 드라이버 초기화 실패

**조치**:
```c
lRet = xDriverOpen(&hDriver);
if(lRet != CIFX_NO_ERROR) {
    printf("xDriverOpen failed: 0x%08X\r\n", lRet);
}
```

---

## 검증 방법

### 1. 컴파일 확인
```bash
# STM32CubeIDE에서 빌드
Project → Build Project (Ctrl+B)

# 에러 없이 컴파일 완료 확인
Build Finished. 0 errors, 0 warnings.
```

---

### 2. 함수 호출 확인

**디버거 브레이크포인트 설정**:
```c
// main.c:661줄
PrintDeviceInformation(hDriver, hChannel);  ← 브레이크포인트
```

**변수 확인**:
```
hDriver: 0x20000XXX (유효한 포인터)
hChannel: 0x20000YYY (유효한 포인터)
```

---

### 3. 시리얼 출력 확인

**필수 출력 항목**:
```
✓ [Driver Information]
✓ [Board Information]
✓ [Channel Information]
✓ [Firmware Information]  ← 가장 중요!
✓ [Channel Status]
```

---

### 4. 펌웨어 이름 확인

**성공 케이스**:
```
Firmware Name:   DeviceNet Slave V5.7.0.0
```
→ ✅ 실제 펌웨어 통신 성공

**실패 케이스**:
```
Firmware Name:
```
→ ❌ Mock 함수 사용 중 또는 펌웨어 미로드

---

## 관련 cifX API

### xDriverGetInformation()
```c
int32_t xDriverGetInformation(
    CIFXHANDLE  hDriver,      // 드라이버 핸들
    uint32_t    ulSize,       // sizeof(DRIVER_INFORMATION)
    void*       pvDriverInfo  // DRIVER_INFORMATION 구조체
);
```

**반환값**:
- `CIFX_NO_ERROR` (0): 성공
- `CIFX_INVALID_HANDLE`: 핸들 무효
- `CIFX_INVALID_PARAMETER`: 매개변수 무효

---

### xDriverEnumBoards()
```c
int32_t xDriverEnumBoards(
    CIFXHANDLE  hDriver,     // 드라이버 핸들
    uint32_t    ulBoard,     // 보드 인덱스 (0부터 시작)
    uint32_t    ulSize,      // sizeof(BOARD_INFORMATION)
    void*       pvBoardInfo  // BOARD_INFORMATION 구조체
);
```

**반환값**:
- `CIFX_NO_ERROR` (0): 성공
- `CIFX_INVALID_BOARD`: 보드 인덱스 범위 초과
- `CIFX_INVALID_HANDLE`: 핸들 무효

---

### xChannelInfo()
```c
int32_t xChannelInfo(
    CIFXHANDLE  hChannel,       // 채널 핸들
    uint32_t    ulSize,         // sizeof(CHANNEL_INFORMATION)
    void*       pvChannelInfo   // CHANNEL_INFORMATION 구조체
);
```

**반환값**:
- `CIFX_NO_ERROR` (0): 성공
- `CIFX_INVALID_HANDLE`: 핸들 무효
- `CIFX_DEV_NOT_READY`: 디바이스 준비 안 됨

---

## 데이터 구조체

### DRIVER_INFORMATION
```c
typedef struct DRIVER_INFORMATIONtag
{
  char       abDriverVersion[32];  // 드라이버 버전 문자열
  uint32_t   ulBoardCnt;           // 등록된 보드 수
} DRIVER_INFORMATION;
```

---

### BOARD_INFORMATION
```c
typedef struct BOARD_INFORMATIONtag
{
  int32_t    lBoardError;          // 보드 에러 코드
  char       abBoardName[16];      // 보드 이름 (예: "cifX0")
  char       abBoardAlias[16];     // 보드 별칭
  uint32_t   ulBoardID;            // 고유 보드 ID
  uint32_t   ulSystemError;        // 시스템 에러
  uint32_t   ulPhysicalAddress;    // 물리 메모리 주소
  uint32_t   ulIrqNumber;          // IRQ 번호
  uint8_t    bIrqEnabled;          // IRQ 활성화 플래그
  uint32_t   ulChannelCnt;         // 채널 수
  uint32_t   ulDpmTotalSize;       // DPM 전체 크기 (바이트)
  SYSTEM_CHANNEL_SYSTEM_INFO_BLOCK tSystemInfo;  // 시스템 정보
} BOARD_INFORMATION;
```

---

### CHANNEL_INFORMATION
```c
typedef struct CHANNEL_INFORMATIONtag
{
  char       abBoardName[16];      // 보드 이름
  char       abBoardAlias[16];     // 보드 별칭
  uint32_t   ulDeviceNumber;       // 디바이스 번호
  uint32_t   ulSerialNumber;       // 시리얼 번호

  /* 펌웨어 정보 */
  uint16_t   usFWMajor;            // 메이저 버전
  uint16_t   usFWMinor;            // 마이너 버전
  uint16_t   usFWBuild;            // 빌드 번호
  uint16_t   usFWRevision;         // 리비전
  uint8_t    bFWNameLength;        // 펌웨어 이름 길이
  uint8_t    abFWName[63];         // 펌웨어 이름
  uint16_t   usFWYear;             // 빌드 연도
  uint8_t    bFWMonth;             // 빌드 월 (1-12)
  uint8_t    bFWDay;               // 빌드 일 (1-31)

  /* 채널 상태 */
  uint32_t   ulChannelError;       // 채널 에러
  uint32_t   ulOpenCnt;            // 열림 횟수
  uint32_t   ulPutPacketCnt;       // Put 패킷 수
  uint32_t   ulGetPacketCnt;       // Get 패킷 수
  uint32_t   ulMailboxSize;        // 메일박스 크기
  uint32_t   ulIOInAreaCnt;        // 입력 IO 영역 수
  uint32_t   ulIOOutAreaCnt;       // 출력 IO 영역 수
  uint32_t   ulHskSize;            // 핸드셰이크 크기
  uint32_t   ulNetxFlags;          // netX 상태 플래그
  uint32_t   ulHostFlags;          // 호스트 플래그
  uint32_t   ulHostCOSFlags;       // 호스트 COS 플래그
  uint32_t   ulDeviceCOSFlags;     // 디바이스 COS 플래그
} CHANNEL_INFORMATION;
```

---

## 추가 개선 사항

### 1. 에러 문자열 출력
```c
if(lRet != CIFX_NO_ERROR)
{
    char szErrorMsg[256];
    xDriverGetErrorDescription(lRet, szErrorMsg, sizeof(szErrorMsg));
    printf("Error: %s (0x%08X)\r\n", szErrorMsg, (unsigned int)lRet);
}
```

---

### 2. 여러 보드 열거
```c
DRIVER_INFORMATION tDriverInfo;
xDriverGetInformation(hDriver, sizeof(DRIVER_INFORMATION), &tDriverInfo);

printf("Available Boards:\r\n");
for(uint32_t i = 0; i < tDriverInfo.ulBoardCnt; i++)
{
    BOARD_INFORMATION tBoardInfo;
    if(xDriverEnumBoards(hDriver, i, sizeof(BOARD_INFORMATION), &tBoardInfo) == CIFX_NO_ERROR)
    {
        printf("  [%lu] %s (SN: %lu)\r\n",
               i,
               tBoardInfo.abBoardName,
               tBoardInfo.tSystemInfo.ulSerialNumber);
    }
}
```

---

### 3. IO 영역 크기 확인
```c
CHANNEL_IO_INFORMATION tIOInfo;

// 입력 영역
xChannelIOInfo(hChannel, CIFX_IO_INPUT_AREA, 0,
               sizeof(CHANNEL_IO_INFORMATION), &tIOInfo);
printf("Input Area Size: %lu bytes\r\n", tIOInfo.ulTotalSize);

// 출력 영역
xChannelIOInfo(hChannel, CIFX_IO_OUTPUT_AREA, 0,
               sizeof(CHANNEL_IO_INFORMATION), &tIOInfo);
printf("Output Area Size: %lu bytes\r\n", tIOInfo.ulTotalSize);
```

---

## 문제 해결 가이드

### Q1: Firmware Name이 출력되지 않음

**원인**:
- Mock 함수 사용 중 (`USE_CIFX_API` 미정의)
- 펌웨어가 로드되지 않음

**해결**:
```c
// 빌드 설정 확인
Project Properties → C/C++ Build → Settings →
  MCU GCC Compiler → Preprocessor → Defined symbols

// USE_CIFX_API 추가
```

---

### Q2: 모든 정보가 에러로 출력됨

**원인**:
- 드라이버 또는 채널 핸들 무효
- 초기화 실패

**해결**:
```c
// 드라이버 열기 확인
lRet = xDriverOpen(&hDriver);
printf("xDriverOpen: 0x%08X\r\n", (unsigned int)lRet);

// 채널 열기 확인
lRet = xChannelOpen(hDriver, "cifX0", 0, &hChannel);
printf("xChannelOpen: 0x%08X\r\n", (unsigned int)lRet);
```

---

### Q3: Board Alias가 비어있음

**원인**:
- 보드 별칭이 설정되지 않음 (정상)

**설명**:
- Board Alias는 선택 사항
- 비어있어도 정상 동작

---

## 테스트 체크리스트

### 빌드 전
- [ ] 함수 프로토타입 추가 확인
- [ ] 함수 구현 추가 확인
- [ ] 함수 호출 코드 추가 확인
- [ ] `USE_CIFX_API` 매크로 정의 확인

### 빌드
- [ ] 컴파일 에러 없음
- [ ] 링크 에러 없음
- [ ] 경고 메시지 확인

### 실행
- [ ] 펌웨어 다운로드 성공
- [ ] 시리얼 출력 확인
- [ ] 디바이스 정보 출력 확인
- [ ] 펌웨어 이름 출력 확인

### 검증
- [ ] Firmware Name이 정상 출력됨
- [ ] FW Version이 0.0.0.0이 아님
- [ ] 에러 메시지 없음
- [ ] VAT 테스트 정상 실행됨

---

## 관련 문서

- `20251028_DeviceNet_Master_Device_Info_Display.md` - 디바이스 정보 출력 기능 구현 가이드
- `20251028_DeviceNet_Communication_Log_Analysis.md` - 통신 로그 분석
- `20251028_DeviceNet_10min_Test_Duration_Modification.md` - 10분 테스트 수정
- `netx_tk/Common/cifXAPI/cifXUser.h` - cifX API 정의

---

## 변경 이력

| 날짜 | 버전 | 변경 내용 | 작성자 |
|------|------|----------|--------|
| 2025-10-28 | 1.0 | 초기 작성: PrintDeviceInformation 함수 복구 | Claude Code |

---

## 결론

누락되었던 `PrintDeviceInformation()` 함수를 성공적으로 복구했습니다.

**복구된 기능**:
- ✅ 드라이버 버전 조회 및 출력
- ✅ 보드 정보 조회 및 출력
- ✅ 펌웨어 정보 조회 및 출력 (이름, 버전, 빌드 날짜)
- ✅ 채널 상태 정보 조회 및 출력

**다음 단계**:
1. STM32CubeIDE에서 프로젝트 빌드
2. 펌웨어 다운로드 및 실행
3. 시리얼 터미널에서 디바이스 정보 출력 확인
4. Firmware Name 확인으로 실제 통신 여부 검증

이제 DeviceNet 마스터의 펌웨어 이름과 버전 정보가 정상적으로 출력되어 통신 상태를 명확하게 확인할 수 있습니다.

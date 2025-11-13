# DeviceNet 마스터 디바이스 정보 출력 기능 구현

**작업 일시**: 2025-10-28
**작업자**: Claude Code
**작업 목적**: DeviceNet 통신에서 마스터 디바이스의 이름, 펌웨어 정보 등을 조회하고 출력하는 기능 구현

---

## 작업 개요

cifX API를 사용하여 netX90 DeviceNet 마스터의 상세 정보를 조회하고 출력하는 기능 추가:
- 드라이버 버전 정보
- 보드 정보 (이름, ID, 시리얼 번호)
- 펌웨어 정보 (이름, 버전, 빌드 날짜)
- 채널 상태 정보

---

## 사용된 cifX API

### 1. `xDriverGetInformation()`
```c
int32_t xDriverGetInformation(
    CIFXHANDLE  hDriver,      // 드라이버 핸들
    uint32_t    ulSize,       // 구조체 크기
    void*       pvDriverInfo  // DRIVER_INFORMATION 구조체 포인터
);
```

**반환 정보**:
- `abDriverVersion[32]`: 드라이버 버전 문자열
- `ulBoardCnt`: 사용 가능한 보드 수

---

### 2. `xDriverEnumBoards()`
```c
int32_t xDriverEnumBoards(
    CIFXHANDLE  hDriver,     // 드라이버 핸들
    uint32_t    ulBoard,     // 보드 인덱스 (0부터 시작)
    uint32_t    ulSize,      // 구조체 크기
    void*       pvBoardInfo  // BOARD_INFORMATION 구조체 포인터
);
```

**반환 정보**:
- `abBoardName[16]`: 보드 이름
- `abBoardAlias[16]`: 보드 별칭
- `ulBoardID`: 고유 보드 ID
- `ulSystemError`: 시스템 에러 코드
- `ulPhysicalAddress`: 물리 메모리 주소
- `ulChannelCnt`: 사용 가능한 채널 수
- `ulDpmTotalSize`: DPM 전체 크기
- `tSystemInfo`: 시스템 정보 블록
  - `ulDeviceNumber`: 디바이스 번호
  - `ulSerialNumber`: 시리얼 번호
  - `bHwRevision`: 하드웨어 리비전
  - `usDeviceClass`: 디바이스 클래스

---

### 3. `xChannelInfo()`
```c
int32_t xChannelInfo(
    CIFXHANDLE  hChannel,       // 채널 핸들
    uint32_t    ulSize,         // 구조체 크기
    void*       pvChannelInfo   // CHANNEL_INFORMATION 구조체 포인터
);
```

**반환 정보**:
- `abBoardName[16]`: 보드 이름
- `ulDeviceNumber`: 디바이스 번호
- `ulSerialNumber`: 시리얼 번호

**펌웨어 정보**:
- `usFWMajor`: 메이저 버전
- `usFWMinor`: 마이너 버전
- `usFWBuild`: 빌드 번호
- `usFWRevision`: 리비전 번호
- `bFWNameLength`: 펌웨어 이름 길이
- `abFWName[63]`: 펌웨어 이름
- `usFWYear`: 빌드 연도
- `bFWMonth`: 빌드 월 (1-12)
- `bFWDay`: 빌드 일 (1-31)

**채널 상태**:
- `ulChannelError`: 채널 에러 코드
- `ulOpenCnt`: 채널 열림 횟수
- `ulMailboxSize`: 메일박스 크기
- `ulIOInAreaCnt`: 입력 IO 영역 수
- `ulIOOutAreaCnt`: 출력 IO 영역 수
- `ulNetxFlags`: netX 상태 플래그
- `ulHostFlags`: 호스트 플래그
- `ulDeviceCOSFlags`: 디바이스 COS 플래그

---

## 구현 코드

### 파일: `Core/Src/main.c`

#### 1. 함수 프로토타입 선언 (134-138줄)
```c
/* ========== VAT TEST INTEGRATION - 추가 시작 ========== */
static void PrintDeviceInformation(CIFXHANDLE hDriver, CIFXHANDLE hChannel);
static void VAT_InitializeTest(void);
static void VAT_RunTest(CIFXHANDLE hChannel);
/* ========== VAT TEST INTEGRATION - 추가 끝 ========== */
```

---

#### 2. 함수 구현 (261-344줄)
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

#### 3. 함수 호출 (659-660줄)
```c
/* 디바이스 정보 출력 */
PrintDeviceInformation(hDriver, hChannel);

/* VAT 테스트 실행 */
VAT_RunTest(hChannel);
```

---

## 예상 출력

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
  Board Alias:     netX90_SPI
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
```

---

## 정보 활용 방법

### 1. 통신 성공 확인
```
[Firmware Information]
  Firmware Name:   DeviceNet Slave V5.7.0.0  ← 펌웨어 이름 확인 가능
  FW Version:      5.7.0.0                    ← 버전 정보 확인
```

**의미**:
- Firmware Name이 정상 출력되면 **실제 펌웨어 로드 성공**
- Mock 함수 사용 시 이름이 비어있거나 이상한 값 출력

---

### 2. 디바이스 식별
```
[Board Information]
  Board Name:      cifX0                     ← 보드 이름
  Device Number:   12345                     ← 고유 디바이스 번호
  Serial Number:   67890                     ← 시리얼 번호
```

**활용**:
- 여러 보드 사용 시 식별 가능
- 시리얼 번호로 추적 가능

---

### 3. 통신 상태 확인
```
[Channel Status]
  Device COS:      0x00000001               ← HIL_COMM_COS_READY
  netX Flags:      0x00000001               ← 통신 준비 상태
  IO Input Areas:  1                        ← 입력 영역 설정
  IO Output Areas: 1                        ← 출력 영역 설정
```

**활용**:
- COS 플래그로 통신 상태 확인
- IO 영역 수로 설정 확인

---

### 4. 펌웨어 버전 관리
```
[Firmware Information]
  FW Build Date:   2024-03-15                ← 빌드 날짜
```

**활용**:
- 펌웨어 업데이트 필요 여부 판단
- 버전 호환성 확인

---

## 문제 해결

### 에러 메시지 분석

#### 1. `[Driver Information] ERROR: 0xXXXXXXXX`
**원인**:
- 드라이버 핸들 무효
- 드라이버 초기화 실패

**조치**:
```c
// xDriverOpen() 호출 확인
lRet = xDriverOpen(&hDriver);
if(lRet != CIFX_NO_ERROR) {
    printf("xDriverOpen failed: 0x%08X\r\n", lRet);
}
```

---

#### 2. `[Board Information] ERROR: 0xXXXXXXXX`
**원인**:
- 보드 인덱스 무효 (ulBoard >= ulBoardCnt)
- 보드가 등록되지 않음

**조치**:
```c
// 먼저 보드 수 확인
DRIVER_INFORMATION tDriverInfo;
xDriverGetInformation(hDriver, sizeof(DRIVER_INFORMATION), &tDriverInfo);
printf("Available boards: %lu\r\n", tDriverInfo.ulBoardCnt);
```

---

#### 3. `[Channel Information] ERROR: 0xXXXXXXXX`
**원인**:
- 채널 핸들 무효
- 채널이 열리지 않음

**조치**:
```c
// xChannelOpen() 호출 확인
lRet = xChannelOpen(hDriver, "cifX0", 0, &hChannel);
if(lRet != CIFX_NO_ERROR) {
    printf("xChannelOpen failed: 0x%08X\r\n", lRet);
}
```

---

#### 4. Firmware Name이 비어있음
**원인**:
- Mock 함수 사용 중 (`USE_CIFX_API` 미정의)
- 펌웨어가 로드되지 않음

**조치**:
```c
// 빌드 설정에서 USE_CIFX_API 매크로 정의
// Project Properties → Preprocessor → Defined symbols
// 추가: USE_CIFX_API
```

---

## cifX API 구조체 정의

### DRIVER_INFORMATION
```c
typedef struct DRIVER_INFORMATIONtag
{
  char       abDriverVersion[32];  // 드라이버 버전
  uint32_t   ulBoardCnt;           // 보드 수
} DRIVER_INFORMATION;
```

---

### BOARD_INFORMATION
```c
typedef struct BOARD_INFORMATIONtag
{
  int32_t    lBoardError;                  // 보드 에러
  char       abBoardName[16];              // 보드 이름
  char       abBoardAlias[16];             // 보드 별칭
  uint32_t   ulBoardID;                    // 보드 ID
  uint32_t   ulSystemError;                // 시스템 에러
  uint32_t   ulPhysicalAddress;            // 물리 주소
  uint32_t   ulIrqNumber;                  // IRQ 번호
  uint8_t    bIrqEnabled;                  // IRQ 활성화
  uint32_t   ulChannelCnt;                 // 채널 수
  uint32_t   ulDpmTotalSize;               // DPM 크기
  SYSTEM_CHANNEL_SYSTEM_INFO_BLOCK tSystemInfo;  // 시스템 정보
} BOARD_INFORMATION;
```

---

### CHANNEL_INFORMATION
```c
typedef struct CHANNEL_INFORMATIONtag
{
  char       abBoardName[16];              // 보드 이름
  char       abBoardAlias[16];             // 보드 별칭
  uint32_t   ulDeviceNumber;               // 디바이스 번호
  uint32_t   ulSerialNumber;               // 시리얼 번호

  uint16_t   usFWMajor;                    // FW 메이저 버전
  uint16_t   usFWMinor;                    // FW 마이너 버전
  uint16_t   usFWBuild;                    // FW 빌드 번호
  uint16_t   usFWRevision;                 // FW 리비전
  uint8_t    bFWNameLength;                // FW 이름 길이
  uint8_t    abFWName[63];                 // FW 이름
  uint16_t   usFWYear;                     // FW 빌드 연도
  uint8_t    bFWMonth;                     // FW 빌드 월
  uint8_t    bFWDay;                       // FW 빌드 일

  uint32_t   ulChannelError;               // 채널 에러
  uint32_t   ulOpenCnt;                    // 열림 횟수
  uint32_t   ulPutPacketCnt;               // Put 패킷 수
  uint32_t   ulGetPacketCnt;               // Get 패킷 수
  uint32_t   ulMailboxSize;                // 메일박스 크기
  uint32_t   ulIOInAreaCnt;                // 입력 영역 수
  uint32_t   ulIOOutAreaCnt;               // 출력 영역 수
  uint32_t   ulHskSize;                    // 핸드셰이크 크기
  uint32_t   ulNetxFlags;                  // netX 플래그
  uint32_t   ulHostFlags;                  // 호스트 플래그
  uint32_t   ulHostCOSFlags;               // 호스트 COS 플래그
  uint32_t   ulDeviceCOSFlags;             // 디바이스 COS 플래그
} CHANNEL_INFORMATION;
```

---

## 추가 개선 사항

### 1. 에러 문자열 출력
```c
char szErrorMsg[256];
xDriverGetErrorDescription(lRet, szErrorMsg, sizeof(szErrorMsg));
printf("Error: %s (0x%08X)\r\n", szErrorMsg, lRet);
```

---

### 2. 모든 보드 열거
```c
DRIVER_INFORMATION tDriverInfo;
xDriverGetInformation(hDriver, sizeof(DRIVER_INFORMATION), &tDriverInfo);

for(uint32_t i = 0; i < tDriverInfo.ulBoardCnt; i++)
{
    BOARD_INFORMATION tBoardInfo;
    xDriverEnumBoards(hDriver, i, sizeof(BOARD_INFORMATION), &tBoardInfo);
    printf("Board %lu: %s\r\n", i, tBoardInfo.abBoardName);
}
```

---

### 3. IO 영역 정보
```c
CHANNEL_IO_INFORMATION tIOInfo;
xChannelIOInfo(hChannel, CIFX_IO_INPUT_AREA, 0,
               sizeof(CHANNEL_IO_INFORMATION), &tIOInfo);
printf("Input Area Size: %lu bytes\r\n", tIOInfo.ulTotalSize);
```

---

### 4. System Device 정보
```c
CIFXHANDLE hSysDevice;
xSysdeviceOpen(hDriver, "cifX0", &hSysDevice);

SYSTEM_CHANNEL_SYSTEM_INFORMATION tSysInfo;
xSysdeviceInfo(hSysDevice, CIFX_INFO_CMD_SYSTEM_INFORMATION,
               sizeof(SYSTEM_CHANNEL_SYSTEM_INFORMATION), &tSysInfo);

printf("System Error: 0x%08lX\r\n", tSysInfo.ulSystemError);
printf("DPM Size: %lu bytes\r\n", tSysInfo.ulDpmTotalSize);

xSysdeviceClose(hSysDevice);
```

---

## 관련 파일

### 구현 파일
- `Core/Src/main.c` - 디바이스 정보 출력 함수 구현 및 호출

### 헤더 파일
- `netx_tk/Common/cifXAPI/cifXUser.h` - cifX API 정의
- `cifXToolkit.h` - cifX Toolkit 메인 헤더

---

## 참고 문서

- cifX API Reference Manual
- netX90 Technical Manual
- `20251028_DeviceNet_Communication_Log_Analysis.md` - 통신 로그 분석
- `20251027_VAT_DeviceNet_Test_Guide.md` - VAT 테스트 가이드

---

## 변경 이력

| 날짜 | 버전 | 변경 내용 | 작성자 |
|------|------|----------|--------|
| 2025-10-28 | 1.0 | 초기 작성: 디바이스 정보 출력 기능 구현 | Claude Code |

---

## 결론

cifX API를 사용하여 DeviceNet 마스터 디바이스의 상세 정보를 출력하는 기능을 성공적으로 구현했습니다.

**구현된 기능**:
- ✅ 드라이버 버전 조회 및 출력
- ✅ 보드 정보 조회 및 출력 (이름, ID, 시리얼 번호)
- ✅ 펌웨어 정보 조회 및 출력 (이름, 버전, 빌드 날짜)
- ✅ 채널 상태 정보 조회 및 출력

**활용**:
- 통신 성공 확인 (펌웨어 이름 확인)
- 디바이스 식별 (보드 이름, 시리얼 번호)
- 버전 관리 (펌웨어 버전, 빌드 날짜)
- 상태 모니터링 (COS 플래그, IO 영역)

이 기능을 통해 DeviceNet 통신 상태를 보다 명확하게 파악할 수 있으며, Mock 함수 사용 여부도 쉽게 확인할 수 있습니다.

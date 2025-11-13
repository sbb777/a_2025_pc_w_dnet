# Live Expression용 전역 변수 설정 가이드
**작성일시**: 2025-10-27

---

## 1. 현재 전역 변수 상태 분석

### 📍 전역 변수 정의 위치
- **정의**: `Hil_DemoApp\Sources\App_SystemPackets.c:33`
```c
APP_DATA_T g_tAppData;
```

- **선언**: `Hil_DemoApp\Sources\App_DemoApplication.c:36`
```c
extern APP_DATA_T g_tAppData;
```

### 📦 데이터 구조
`g_tAppData` 구조체 내부에 입출력 데이터 포함:
```c
typedef struct APP_DATA_Ttag
{
  // ... 기타 필드 ...
  APP_PROCESS_DATA_INPUT_T  tInputData;   // 입력 데이터 (6바이트)
  APP_PROCESS_DATA_OUTPUT_T tOutputData;  // 출력 데이터 (10바이트)
  // ...
} APP_DATA_T;
```

---

## 2. Live Expression용 전역 변수 추가

### ✅ 방법 1: 전역 포인터 변수 (권장)

디버거에서 가장 효율적으로 사용할 수 있는 방법입니다.

#### 📄 헤더 파일 수정: `App_DemoApplication.h`

**위치**: `Hil_DemoApp\Includes\App_DemoApplication.h`의 **FUNCTION PROTOTYPES** 섹션 앞에 추가

```c
/*****************************************************************************/
/*! GLOBAL VARIABLES FOR LIVE EXPRESSION (DEBUGGING)                        */
/*****************************************************************************/

#ifdef DEBUG
extern APP_PROCESS_DATA_INPUT_T*  g_ptInputData_LiveView;   /* 입력 데이터 포인터 */
extern APP_PROCESS_DATA_OUTPUT_T* g_ptOutputData_LiveView;  /* 출력 데이터 포인터 */
extern bool*                      g_pfInputDataValid_LiveView; /* 입력 유효성 플래그 */
#endif

/*****************************************************************************/
/*! FUNCTION PROTOTYPES                                                      */
/*****************************************************************************/
```

#### 📄 소스 파일 수정: `App_SystemPackets.c`

**위치**: `Hil_DemoApp\Sources\App_SystemPackets.c`의 전역 변수 섹션

```c
/*****************************************************************************/
/*! General Inclusion Area                                                   */
/*****************************************************************************/

#include <stdint.h>
#include "App_PacketCommunication.h"
#include "App_SystemPackets.h"
#include "Hil_ApplicationCmd.h"
#include "Hil_Results.h"
#include "Hil_SystemCmd.h"
#include "Hil_Packet.h"

APP_DATA_T g_tAppData;

/*****************************************************************************/
/*! GLOBAL VARIABLES FOR LIVE EXPRESSION (DEBUGGING)                        */
/*****************************************************************************/

#ifdef DEBUG
APP_PROCESS_DATA_INPUT_T*  g_ptInputData_LiveView   = NULL;
APP_PROCESS_DATA_OUTPUT_T* g_ptOutputData_LiveView  = NULL;
bool*                      g_pfInputDataValid_LiveView = NULL;
#endif
```

#### 📄 초기화 코드: `App_DemoApplication.c`

**위치**: `App_CifXApplicationDemo()` 함수 내부, 채널 오픈 직후

```c
int App_CifXApplicationDemo(char *szDeviceName)
{
  CIFXHANDLE hDriver  = NULL;
  int32_t   lRet      = 0;
  uint32_t  ulState   = 0;
  uint32_t  ulTimeout = 1000;

  PRINTF("---------- cifX Application Demo ----------" NEWLINE);

  g_tAppData.fRunning = true;

  g_tAppData.aptChannels[0] = (APP_COMM_CHANNEL_T*)calloc(1, sizeof(APP_COMM_CHANNEL_T));
  g_tAppData.aptChannels[0]->tProtocol = g_tRealtimeProtocolHandlers;

  /** 🆕 Live Expression용 포인터 초기화 */
  #ifdef DEBUG
  g_ptInputData_LiveView   = &g_tAppData.tInputData;
  g_ptOutputData_LiveView  = &g_tAppData.tOutputData;
  g_pfInputDataValid_LiveView = &g_tAppData.fInputDataValid;
  #endif

  /* open driver */
  if (CIFX_NO_ERROR != (lRet = xDriverOpen(&hDriver)))
  {
    PRINTF("ERROR: xDriverOpen failed: 0x%08x" NEWLINE, (unsigned int)lRet);
    return lRet;
  }

  // ... 나머지 코드 ...
}
```

---

### ✅ 방법 2: 전역 복사본 변수

실시간 업데이트가 필요한 경우 복사본을 사용합니다.

#### 📄 헤더 파일 수정: `App_DemoApplication.h`

```c
/*****************************************************************************/
/*! GLOBAL VARIABLES FOR LIVE EXPRESSION (DEBUGGING)                        */
/*****************************************************************************/

#ifdef DEBUG
extern APP_PROCESS_DATA_INPUT_T  g_tInputData_LiveCopy;    /* 입력 데이터 복사본 */
extern APP_PROCESS_DATA_OUTPUT_T g_tOutputData_LiveCopy;   /* 출력 데이터 복사본 */
extern bool                      g_fInputDataValid_LiveCopy; /* 입력 유효성 플래그 복사본 */
extern uint32_t                  g_ulUpdateCounter_Live;   /* 업데이트 카운터 */
#endif
```

#### 📄 소스 파일 수정: `App_SystemPackets.c`

```c
APP_DATA_T g_tAppData;

/*****************************************************************************/
/*! GLOBAL VARIABLES FOR LIVE EXPRESSION (DEBUGGING)                        */
/*****************************************************************************/

#ifdef DEBUG
APP_PROCESS_DATA_INPUT_T  g_tInputData_LiveCopy    = {0};
APP_PROCESS_DATA_OUTPUT_T g_tOutputData_LiveCopy   = {0};
bool                      g_fInputDataValid_LiveCopy = false;
uint32_t                  g_ulUpdateCounter_Live   = 0;
#endif
```

#### 📄 업데이트 코드: `App_IODataHandler()`

```c
void App_IODataHandler(void* ptAppResources)
{
  long            lRet      = CIFX_NO_ERROR;
  APP_DATA_T*     ptAppData = (APP_DATA_T*)ptAppResources;

  if(ptAppData->aptChannels[0]->hChannel != NULL)
  {
    /** INPUT DATA **********************************************/
    lRet = xChannelIORead(ptAppData->aptChannels[0]->hChannel, 0, 0,
                          sizeof(ptAppData->tInputData),
                          &ptAppData->tInputData, 0);

    if(lRet != CIFX_NO_ERROR)
    {
      ptAppData->fInputDataValid = false;
    }
    else
    {
      ptAppData->fInputDataValid = true;
    }

    /** OUTPUT DATA ********************************************/
    for (int i = 0; i < 10; i++)
    {
      ptAppData->tOutputData.output[i]++;
    }

    lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel, 0, 0,
                           sizeof(ptAppData->tOutputData),
                           &ptAppData->tOutputData, 0);

    /** 🆕 Live Expression용 복사본 업데이트 */
    #ifdef DEBUG
    memcpy(&g_tInputData_LiveCopy, &ptAppData->tInputData, sizeof(APP_PROCESS_DATA_INPUT_T));
    memcpy(&g_tOutputData_LiveCopy, &ptAppData->tOutputData, sizeof(APP_PROCESS_DATA_OUTPUT_T));
    g_fInputDataValid_LiveCopy = ptAppData->fInputDataValid;
    g_ulUpdateCounter_Live++;
    #endif
  }
}
```

---

## 3. 디버거 Live Expression 설정

### 🔧 STM32CubeIDE / Eclipse 기반

#### Live Expressions 뷰 열기
1. `Window` → `Show View` → `Other...` → `Debug` → `Live Expressions`
2. 또는 디버깅 중 우클릭 → `Live Expressions`

#### 표현식 추가

**방법 1 사용 시 (포인터)**:
```c
// 입력 데이터 전체 배열
g_ptInputData_LiveView->input

// 입력 데이터 개별 바이트
g_ptInputData_LiveView->input[0]
g_ptInputData_LiveView->input[1]
g_ptInputData_LiveView->input[2]
g_ptInputData_LiveView->input[3]
g_ptInputData_LiveView->input[4]
g_ptInputData_LiveView->input[5]

// 출력 데이터 전체 배열
g_ptOutputData_LiveView->output

// 출력 데이터 개별 바이트
g_ptOutputData_LiveView->output[0]
g_ptOutputData_LiveView->output[1]
// ... output[9]까지

// 입력 유효성 플래그
*g_pfInputDataValid_LiveView
```

**방법 2 사용 시 (복사본)**:
```c
// 입력 데이터
g_tInputData_LiveCopy.input

// 출력 데이터
g_tOutputData_LiveCopy.output

// 업데이트 카운터
g_ulUpdateCounter_Live

// 입력 유효성
g_fInputDataValid_LiveCopy
```

**원본 데이터 직접 접근** (항상 가능):
```c
// 가장 직접적인 방법
g_tAppData.tInputData.input
g_tAppData.tOutputData.output
g_tAppData.fInputDataValid
```

---

### 🔧 Keil µVision

#### Watch Window 설정
1. `View` → `Watch Windows` → `Watch 1`
2. 변수 추가:
```c
g_ptInputData_LiveView->input,d
g_ptOutputData_LiveView->output,d
*g_pfInputDataValid_LiveView
```

포맷 지정자:
- `,d` - 10진수 배열
- `,x` - 16진수 배열
- `,b` - 2진수

---

### 🔧 IAR Embedded Workbench

#### Live Watch 설정
1. `View` → `Live Watch`
2. 표현식 추가:
```c
g_ptInputData_LiveView
g_ptOutputData_LiveView
g_tAppData.tInputData
g_tAppData.tOutputData
```

---

## 4. 고급 디버깅 기법

### 📊 Memory Browser를 사용한 실시간 모니터링

#### STM32CubeIDE
1. `Window` → `Show View` → `Memory Browser`
2. 주소 입력:
```
&g_tAppData.tInputData
&g_tAppData.tOutputData
```

#### 16진수 뷰 설정
- **입력 데이터**: 6바이트 (0x00 ~ 0x05)
- **출력 데이터**: 10바이트 (0x00 ~ 0x09)

---

### 📈 변수 추적을 위한 추가 전역 변수

더 자세한 디버깅을 위한 옵션:

```c
/*****************************************************************************/
/*! EXTENDED LIVE EXPRESSION VARIABLES (OPTIONAL)                           */
/*****************************************************************************/

#ifdef DEBUG
typedef struct LIVE_DEBUG_INFO_Ttag
{
  uint32_t ulReadCount;         /* xChannelIORead 호출 횟수 */
  uint32_t ulReadErrorCount;    /* 읽기 에러 발생 횟수 */
  uint32_t ulWriteCount;        /* xChannelIOWrite 호출 횟수 */
  uint32_t ulWriteErrorCount;   /* 쓰기 에러 발생 횟수 */

  uint8_t  abInputHistory[6][10];   /* 최근 10개 입력 데이터 */
  uint8_t  abOutputHistory[10][10]; /* 최근 10개 출력 데이터 */
  uint8_t  ubHistoryIndex;          /* 히스토리 인덱스 */

  int32_t  lLastReadError;      /* 마지막 읽기 에러 코드 */
  int32_t  lLastWriteError;     /* 마지막 쓰기 에러 코드 */
} LIVE_DEBUG_INFO_T;

extern LIVE_DEBUG_INFO_T g_tLiveDebugInfo;
#endif
```

#### 업데이트 코드

```c
#ifdef DEBUG
LIVE_DEBUG_INFO_T g_tLiveDebugInfo = {0};
#endif

void App_IODataHandler(void* ptAppResources)
{
  long            lRet      = CIFX_NO_ERROR;
  APP_DATA_T*     ptAppData = (APP_DATA_T*)ptAppResources;

  if(ptAppData->aptChannels[0]->hChannel != NULL)
  {
    /** INPUT DATA **********************************************/
    lRet = xChannelIORead(ptAppData->aptChannels[0]->hChannel, 0, 0,
                          sizeof(ptAppData->tInputData),
                          &ptAppData->tInputData, 0);

    #ifdef DEBUG
    g_tLiveDebugInfo.ulReadCount++;
    #endif

    if(lRet != CIFX_NO_ERROR)
    {
      ptAppData->fInputDataValid = false;

      #ifdef DEBUG
      g_tLiveDebugInfo.ulReadErrorCount++;
      g_tLiveDebugInfo.lLastReadError = lRet;
      #endif
    }
    else
    {
      ptAppData->fInputDataValid = true;

      #ifdef DEBUG
      /* 입력 데이터 히스토리 저장 */
      memcpy(&g_tLiveDebugInfo.abInputHistory[g_tLiveDebugInfo.ubHistoryIndex % 10],
             &ptAppData->tInputData.input,
             sizeof(ptAppData->tInputData.input));
      #endif
    }

    /** OUTPUT DATA ********************************************/
    for (int i = 0; i < 10; i++)
    {
      ptAppData->tOutputData.output[i]++;
    }

    #ifdef DEBUG
    /* 출력 데이터 히스토리 저장 */
    memcpy(&g_tLiveDebugInfo.abOutputHistory[g_tLiveDebugInfo.ubHistoryIndex % 10],
           &ptAppData->tOutputData.output,
           sizeof(ptAppData->tOutputData.output));
    #endif

    lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel, 0, 0,
                           sizeof(ptAppData->tOutputData),
                           &ptAppData->tOutputData, 0);

    #ifdef DEBUG
    g_tLiveDebugInfo.ulWriteCount++;
    if(lRet != CIFX_NO_ERROR)
    {
      g_tLiveDebugInfo.ulWriteErrorCount++;
      g_tLiveDebugInfo.lLastWriteError = lRet;
    }
    g_tLiveDebugInfo.ubHistoryIndex++;
    #endif
  }
}
```

---

## 5. 컴파일 옵션 설정

### ✅ DEBUG 매크로 활성화 확인

#### STM32CubeIDE
1. `Project` → `Properties` → `C/C++ Build` → `Settings`
2. `MCU GCC Compiler` → `Preprocessor`
3. `Defined symbols (-D)` 섹션에서 `DEBUG` 확인

또는:

#### Debug 빌드 설정에서만 활성화
```c
/* Project Properties → C/C++ Build → Settings → Debug Configuration */
Symbols defined on the command line:
  DEBUG=1
  USE_HAL_DRIVER
  STM32F429xx
```

---

## 6. 사용 예시

### 🎯 실시간 데이터 모니터링

**Live Expression 창에 추가**:
```c
// 기본 모니터링
g_tAppData.tInputData.input
g_tAppData.tOutputData.output
g_tAppData.fInputDataValid

// 포인터 사용 (방법 1)
g_ptInputData_LiveView->input[0]
g_ptInputData_LiveView->input[1]
g_ptOutputData_LiveView->output[0]
g_ptOutputData_LiveView->output[1]

// 복사본 사용 (방법 2)
g_tInputData_LiveCopy.input
g_tOutputData_LiveCopy.output
g_ulUpdateCounter_Live

// 확장 디버그 정보
g_tLiveDebugInfo.ulReadCount
g_tLiveDebugInfo.ulWriteCount
g_tLiveDebugInfo.ulReadErrorCount
g_tLiveDebugInfo.ulWriteErrorCount
```

---

## 7. 권장 사항

### ✅ 성능 최적화
- **Release 빌드**: `#ifdef DEBUG`로 감싸서 프로덕션 코드에서 제외
- **메모리 사용**: 방법 1 (포인터) 사용 시 메모리 오버헤드 최소
- **실시간 성능**: 히스토리 기능은 선택적으로 활성화

### ✅ 디버깅 효율성
- **포인터 방식 (방법 1)**: 메모리 효율적, 실시간 반영
- **복사본 방식 (방법 2)**: 안정적, 비동기 액세스 안전
- **원본 직접 접근**: 가장 간단, 추가 코드 불필요

### ⚠️ 주의사항
1. **Thread Safety**: 인터럽트나 RTOS 사용 시 접근 동기화 필요
2. **Optimization**: 컴파일러 최적화로 변수가 제거될 수 있음 (`volatile` 사용)
3. **Memory Alignment**: 구조체 패딩 확인 필요

---

## 8. 완전한 코드 예시

### 📄 `App_DemoApplication.h` (추가 부분)

**위치**: Line 120 근처, FUNCTION PROTOTYPES 앞

```c
/*****************************************************************************/
/*! GLOBAL VARIABLES FOR LIVE EXPRESSION (DEBUGGING)                        */
/*****************************************************************************/

#ifdef DEBUG
/* 방법 1: 포인터 변수 (메모리 효율적) */
extern APP_PROCESS_DATA_INPUT_T*  g_ptInputData_LiveView;
extern APP_PROCESS_DATA_OUTPUT_T* g_ptOutputData_LiveView;
extern bool*                      g_pfInputDataValid_LiveView;

/* 방법 2: 복사본 변수 (선택적) */
extern APP_PROCESS_DATA_INPUT_T  g_tInputData_LiveCopy;
extern APP_PROCESS_DATA_OUTPUT_T g_tOutputData_LiveCopy;
extern bool                      g_fInputDataValid_LiveCopy;
extern uint32_t                  g_ulUpdateCounter_Live;

/* 확장 디버그 정보 (선택적) */
typedef struct LIVE_DEBUG_INFO_Ttag
{
  uint32_t ulReadCount;
  uint32_t ulReadErrorCount;
  uint32_t ulWriteCount;
  uint32_t ulWriteErrorCount;
  int32_t  lLastReadError;
  int32_t  lLastWriteError;
} LIVE_DEBUG_INFO_T;

extern LIVE_DEBUG_INFO_T g_tLiveDebugInfo;
#endif

/*****************************************************************************/
/*! FUNCTION PROTOTYPES                                                      */
/*****************************************************************************/
```

### 📄 `App_SystemPackets.c` (추가 부분)

**위치**: Line 33 근처, `APP_DATA_T g_tAppData;` 바로 다음

```c
APP_DATA_T g_tAppData;

/*****************************************************************************/
/*! GLOBAL VARIABLES FOR LIVE EXPRESSION (DEBUGGING)                        */
/*****************************************************************************/

#ifdef DEBUG
/* 방법 1: 포인터 초기화 */
APP_PROCESS_DATA_INPUT_T*  g_ptInputData_LiveView   = NULL;
APP_PROCESS_DATA_OUTPUT_T* g_ptOutputData_LiveView  = NULL;
bool*                      g_pfInputDataValid_LiveView = NULL;

/* 방법 2: 복사본 초기화 */
APP_PROCESS_DATA_INPUT_T  g_tInputData_LiveCopy    = {0};
APP_PROCESS_DATA_OUTPUT_T g_tOutputData_LiveCopy   = {0};
bool                      g_fInputDataValid_LiveCopy = false;
uint32_t                  g_ulUpdateCounter_Live   = 0;

/* 확장 디버그 정보 */
LIVE_DEBUG_INFO_T g_tLiveDebugInfo = {0};
#endif
```

### 📄 `App_DemoApplication.c` 수정

#### 초기화 (Line 250 근처)

```c
int App_CifXApplicationDemo(char *szDeviceName)
{
  CIFXHANDLE hDriver  = NULL;
  int32_t   lRet      = 0;
  uint32_t  ulState   = 0;
  uint32_t  ulTimeout = 1000;

  PRINTF("---------- cifX Application Demo ----------" NEWLINE);

  g_tAppData.fRunning = true;

  g_tAppData.aptChannels[0] = (APP_COMM_CHANNEL_T*)calloc(1, sizeof(APP_COMM_CHANNEL_T));
  g_tAppData.aptChannels[0]->tProtocol = g_tRealtimeProtocolHandlers;

  /*****************************************************************************/
  /*! 🆕 Live Expression용 포인터 초기화                                        */
  /*****************************************************************************/
  #ifdef DEBUG
  g_ptInputData_LiveView   = &g_tAppData.tInputData;
  g_ptOutputData_LiveView  = &g_tAppData.tOutputData;
  g_pfInputDataValid_LiveView = &g_tAppData.fInputDataValid;
  PRINTF("DEBUG: Live expression pointers initialized" NEWLINE);
  #endif

  /* open driver */
  // ... 나머지 코드 ...
}
```

#### IO Handler 업데이트 (Line 370 근처)

```c
void App_IODataHandler(void* ptAppResources)
{
  long            lRet      = CIFX_NO_ERROR;
  APP_DATA_T*     ptAppData = (APP_DATA_T*)ptAppResources;

  if(ptAppData->aptChannels[0]->hChannel != NULL)
  {
    /*****************************************************************************/
    /*! INPUT DATA                                                               */
    /*****************************************************************************/
    lRet = xChannelIORead(ptAppData->aptChannels[0]->hChannel, 0, 0,
                          sizeof(ptAppData->tInputData),
                          &ptAppData->tInputData, 0);

    #ifdef DEBUG
    g_tLiveDebugInfo.ulReadCount++;
    #endif

    if(lRet != CIFX_NO_ERROR)
    {
      ptAppData->fInputDataValid = false;

      #ifdef DEBUG
      g_tLiveDebugInfo.ulReadErrorCount++;
      g_tLiveDebugInfo.lLastReadError = lRet;
      #endif
    }
    else
    {
      ptAppData->fInputDataValid = true;
    }

    /*****************************************************************************/
    /*! OUTPUT DATA                                                              */
    /*****************************************************************************/
    for (int i = 0; i < 10; i++)
    {
      ptAppData->tOutputData.output[i]++;
    }

    lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel, 0, 0,
                           sizeof(ptAppData->tOutputData),
                           &ptAppData->tOutputData, 0);

    #ifdef DEBUG
    g_tLiveDebugInfo.ulWriteCount++;
    if(lRet != CIFX_NO_ERROR)
    {
      g_tLiveDebugInfo.ulWriteErrorCount++;
      g_tLiveDebugInfo.lLastWriteError = lRet;
    }

    /* 복사본 업데이트 (방법 2 사용 시) */
    memcpy(&g_tInputData_LiveCopy, &ptAppData->tInputData, sizeof(APP_PROCESS_DATA_INPUT_T));
    memcpy(&g_tOutputData_LiveCopy, &ptAppData->tOutputData, sizeof(APP_PROCESS_DATA_OUTPUT_T));
    g_fInputDataValid_LiveCopy = ptAppData->fInputDataValid;
    g_ulUpdateCounter_Live++;
    #endif
  }
}
```

---

## 9. 검증 및 테스트

### ✅ 컴파일 확인
1. Debug 빌드: 모든 디버그 변수 포함
2. Release 빌드: `#ifdef DEBUG` 블록 제외됨

### ✅ Live Expression 테스트
1. 브레이크포인트 설정: `App_IODataHandler()` 함수 내부
2. 디버거 시작
3. Live Expression 창에서 변수 확인:
   - `g_ptInputData_LiveView->input`
   - `g_ptOutputData_LiveView->output`
   - `g_ulUpdateCounter_Live`

### ✅ 실시간 모니터링
- Continue 버튼으로 실행 계속
- Live Expression 창에서 자동 업데이트 확인
- 500ms 주기로 값 변경 확인

---

## 10. 요약

### 📌 전역 변수 할당 상태
✅ **기존**: `g_tAppData` 전역 변수 존재 (App_SystemPackets.c:33)
✅ **신규**: Live Expression용 전역 변수 추가 필요

### 📌 권장 구현 방법
- **간단한 디버깅**: 방법 1 (포인터 변수)
- **고급 디버깅**: 방법 1 + 확장 디버그 정보
- **안정성 우선**: 방법 2 (복사본 변수)

### 📌 Live Expression 설정
```c
// 가장 간단 (추가 코드 불필요)
g_tAppData.tInputData.input
g_tAppData.tOutputData.output

// 포인터 방식 (권장)
g_ptInputData_LiveView->input
g_ptOutputData_LiveView->output

// 복사본 방식 (안전)
g_tInputData_LiveCopy.input
g_tOutputData_LiveCopy.output
```

---

**문서 끝**

# App_IODataHandler 함수 분석 및 다중 데이터 변경 코드
**작성일시**: 2025-10-27

---

## 1. 현재 코드 동작 분석

### 📍 위치
`Hil_DemoApp\Sources\App_DemoApplication.c:370-410`

### 🔍 현재 코드
```c
void App_IODataHandler(void* ptAppResources)
{
  long            lRet      = CIFX_NO_ERROR;
  APP_DATA_T*     ptAppData = (APP_DATA_T*)ptAppResources;

  if(ptAppData->aptChannels[0]->hChannel != NULL)
  {
    /** INPUT DATA - 데이터 수신 **********************************************/
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

    /** OUTPUT DATA - 데이터 송신 ********************************************/
    ptAppData->tOutputData.output[0]++;  // ← 첫 번째 바이트만 1씩 증가

    lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel, 0, 0,
                           sizeof(ptAppData->tOutputData),
                           &ptAppData->tOutputData, 0);
  }
}
```

---

## 2. 데이터 구조 분석

### 📦 출력 데이터 구조
**파일**: `Hil_DemoApp\Includes\App_DemoApplication.h:70-73`

```c
typedef __HIL_PACKED_PRE struct APP_PROCESS_DATA_OUTPUT_Ttag
{
  uint8_t output[10];  // 10바이트 배열 (output[0] ~ output[9])
} __HIL_PACKED_POST APP_PROCESS_DATA_OUTPUT_T;
```

### 📦 입력 데이터 구조
```c
typedef __HIL_PACKED_PRE struct APP_PROCESS_DATA_INPUT_Ttag
{
  uint8_t input[6];    // 6바이트 배열 (input[0] ~ input[5])
} __HIL_PACKED_POST APP_PROCESS_DATA_INPUT_T;
```

---

## 3. 질문에 대한 답변

### ❓ "첫 자리 숫자가 1씩 증가하는 동작이 맞는가?"

✅ **예, 맞습니다.**

**근거**:
```c
ptAppData->tOutputData.output[0]++;
lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel, 0, 0,
                       sizeof(ptAppData->tOutputData),  // 10바이트 전체 전송
                       &ptAppData->tOutputData, 0);
```

**동작 설명**:
1. `output[0]` 값을 1 증가시킴
2. 전체 10바이트를 `xChannelIOWrite()`로 전송
3. 수신 측에서는 `output[0]` 위치의 값이 계속 증가하는 것을 확인 가능
4. 나머지 `output[1]~output[9]`는 변경 없이 이전 값 유지

**예시 시퀀스**:
```
전송 1: [  1,  0,  0,  0,  0,  0,  0,  0,  0,  0]
전송 2: [  2,  0,  0,  0,  0,  0,  0,  0,  0,  0]
전송 3: [  3,  0,  0,  0,  0,  0,  0,  0,  0,  0]
...
전송 255: [255,  0,  0,  0,  0,  0,  0,  0,  0,  0]
전송 256: [  0,  0,  0,  0,  0,  0,  0,  0,  0,  0] (오버플로우, 다시 0으로)
```

---

## 4. 다중 자리수 변경 코드

### 🎯 목표
첫 번째 자리뿐만 아니라 **여러 바이트를 변경**하여 다양한 패턴 생성

---

### 💡 방법 1: 순차적으로 모든 바이트 증가

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
    // 모든 바이트를 1씩 증가 (독립적으로)
    for(int i = 0; i < 10; i++)
    {
      ptAppData->tOutputData.output[i]++;
    }

    lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel, 0, 0,
                           sizeof(ptAppData->tOutputData),
                           &ptAppData->tOutputData, 0);
  }
}
```

**결과 예시**:
```
전송 1: [  1,  1,  1,  1,  1,  1,  1,  1,  1,  1]
전송 2: [  2,  2,  2,  2,  2,  2,  2,  2,  2,  2]
전송 3: [  3,  3,  3,  3,  3,  3,  3,  3,  3,  3]
```

---

### 💡 방법 2: 16비트 카운터 (Little-Endian)

```c
void App_IODataHandler(void* ptAppResources)
{
  long            lRet      = CIFX_NO_ERROR;
  APP_DATA_T*     ptAppData = (APP_DATA_T*)ptAppResources;
  static uint16_t counter = 0;  // 16비트 카운터 (0~65535)

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
    // 16비트 카운터를 2바이트로 분할 (Little-Endian)
    counter++;
    ptAppData->tOutputData.output[0] = (uint8_t)(counter & 0xFF);        // 하위 바이트
    ptAppData->tOutputData.output[1] = (uint8_t)((counter >> 8) & 0xFF); // 상위 바이트

    lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel, 0, 0,
                           sizeof(ptAppData->tOutputData),
                           &ptAppData->tOutputData, 0);
  }
}
```

**결과 예시**:
```
전송 1:    [  1,  0, ..., ...]  (counter = 1    = 0x0001)
전송 255:  [255,  0, ..., ...]  (counter = 255  = 0x00FF)
전송 256:  [  0,  1, ..., ...]  (counter = 256  = 0x0100)
전송 257:  [  1,  1, ..., ...]  (counter = 257  = 0x0101)
전송 65535:[255,255, ..., ...]  (counter = 65535= 0xFFFF)
전송 65536:[  0,  0, ..., ...]  (counter = 0    = 0x0000, 오버플로우)
```

---

### 💡 방법 3: 32비트 카운터 (4바이트 사용)

```c
void App_IODataHandler(void* ptAppResources)
{
  long            lRet      = CIFX_NO_ERROR;
  APP_DATA_T*     ptAppData = (APP_DATA_T*)ptAppResources;
  static uint32_t counter = 0;  // 32비트 카운터 (0~4,294,967,295)

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
    // 32비트 카운터를 4바이트로 분할 (Little-Endian)
    counter++;
    ptAppData->tOutputData.output[0] = (uint8_t)(counter & 0xFF);
    ptAppData->tOutputData.output[1] = (uint8_t)((counter >> 8) & 0xFF);
    ptAppData->tOutputData.output[2] = (uint8_t)((counter >> 16) & 0xFF);
    ptAppData->tOutputData.output[3] = (uint8_t)((counter >> 24) & 0xFF);

    lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel, 0, 0,
                           sizeof(ptAppData->tOutputData),
                           &ptAppData->tOutputData, 0);
  }
}
```

**결과 예시**:
```
전송 1:        [  1,  0,  0,  0, ..., ...]  (0x00000001)
전송 256:      [  0,  1,  0,  0, ..., ...]  (0x00000100)
전송 65536:    [  0,  0,  1,  0, ..., ...]  (0x00010000)
전송 16777216: [  0,  0,  0,  1, ..., ...]  (0x01000000)
```

---

### 💡 방법 4: 특정 패턴 생성 (교대로 변경)

```c
void App_IODataHandler(void* ptAppResources)
{
  long            lRet      = CIFX_NO_ERROR;
  APP_DATA_T*     ptAppData = (APP_DATA_T*)ptAppResources;
  static uint8_t  pattern_index = 0;

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
    // 홀수 위치와 짝수 위치를 다른 패턴으로 변경
    for(int i = 0; i < 10; i++)
    {
      if(i % 2 == 0)  // 짝수 인덱스 (0, 2, 4, 6, 8)
      {
        ptAppData->tOutputData.output[i]++;
      }
      else            // 홀수 인덱스 (1, 3, 5, 7, 9)
      {
        ptAppData->tOutputData.output[i] += 2;
      }
    }

    lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel, 0, 0,
                           sizeof(ptAppData->tOutputData),
                           &ptAppData->tOutputData, 0);
  }
}
```

**결과 예시**:
```
전송 1: [  1,  2,  1,  2,  1,  2,  1,  2,  1,  2]
전송 2: [  2,  4,  2,  4,  2,  4,  2,  4,  2,  4]
전송 3: [  3,  6,  3,  6,  3,  6,  3,  6,  3,  6]
```

---

### 💡 방법 5: 타임스탬프 기반 (실시간 시간 포함)

```c
#include <time.h>  // 헤더 추가 필요

void App_IODataHandler(void* ptAppResources)
{
  long            lRet      = CIFX_NO_ERROR;
  APP_DATA_T*     ptAppData = (APP_DATA_T*)ptAppResources;
  static uint32_t tick_count = 0;

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
    tick_count++;

    // output[0-3]: 32비트 틱 카운터
    ptAppData->tOutputData.output[0] = (uint8_t)(tick_count & 0xFF);
    ptAppData->tOutputData.output[1] = (uint8_t)((tick_count >> 8) & 0xFF);
    ptAppData->tOutputData.output[2] = (uint8_t)((tick_count >> 16) & 0xFF);
    ptAppData->tOutputData.output[3] = (uint8_t)((tick_count >> 24) & 0xFF);

    // output[4-9]: 시스템 시간 (밀리초)
    uint32_t timestamp_ms = (uint32_t)time(NULL);  // 간단한 타임스탬프
    ptAppData->tOutputData.output[4] = (uint8_t)(timestamp_ms & 0xFF);
    ptAppData->tOutputData.output[5] = (uint8_t)((timestamp_ms >> 8) & 0xFF);
    ptAppData->tOutputData.output[6] = (uint8_t)((timestamp_ms >> 16) & 0xFF);
    ptAppData->tOutputData.output[7] = (uint8_t)((timestamp_ms >> 24) & 0xFF);

    lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel, 0, 0,
                           sizeof(ptAppData->tOutputData),
                           &ptAppData->tOutputData, 0);
  }
}
```

---

### 💡 방법 6: 입력 데이터 기반 출력 (Echo + 변환)

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
    // 입력 데이터의 각 바이트를 2배로 만들어 출력
    // input[0~5] → output[0~5]에 2배 값 전송
    for(int i = 0; i < 6 && i < 10; i++)
    {
      ptAppData->tOutputData.output[i] = ptAppData->tInputData.input[i] * 2;
    }

    // 나머지 출력 바이트는 고정 패턴
    ptAppData->tOutputData.output[6] = 0xAA;
    ptAppData->tOutputData.output[7] = 0xBB;
    ptAppData->tOutputData.output[8] = 0xCC;
    ptAppData->tOutputData.output[9] = 0xDD;

    lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel, 0, 0,
                           sizeof(ptAppData->tOutputData),
                           &ptAppData->tOutputData, 0);
  }
}
```

**결과 예시**:
```
입력:  [ 10,  20,  30,  40,  50,  60]
출력:  [ 20,  40,  60,  80, 100, 120, 0xAA, 0xBB, 0xCC, 0xDD]
```

---

## 5. 권장 사항

### ✅ 테스트 및 디버깅 시 추천
- **방법 2 (16비트 카운터)**: 가장 간단하고 명확한 카운터 동작 확인
- **방법 3 (32비트 카운터)**: 긴 시간 동안 안정성 테스트

### ✅ 실제 애플리케이션에서 추천
- **방법 6 (입력 기반 출력)**: 양방향 통신 검증
- **방법 5 (타임스탬프)**: 통신 지연 및 동기화 측정

### ⚠️ 주의사항
1. **오버플로우**: `uint8_t`는 255를 초과하면 0으로 돌아감
2. **Endianness**: Little-Endian 가정 (ARM Cortex-M 표준)
3. **동기화**: `static` 변수는 함수 호출 간 유지됨
4. **성능**: 루프 연산은 실시간 시스템에서 성능 영향 최소

---

## 6. 요약

### 📌 원래 질문에 대한 답변
**"첫 자리 숫자가 1씩 증가하는 동작이 맞는가?"**
→ ✅ **예, 맞습니다.** `output[0]++`로 인해 첫 번째 바이트만 증가하며, 전체 10바이트가 전송됩니다.

### 📌 다중 자리수 변경
6가지 방법을 제시했으며, 각각의 장단점:

| 방법 | 복잡도 | 테스트 적합성 | 실용성 |
|------|--------|--------------|--------|
| 1. 전체 증가 | 낮음 | ⭐⭐⭐ | ⭐⭐ |
| 2. 16비트 카운터 | 중간 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ |
| 3. 32비트 카운터 | 중간 | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| 4. 패턴 생성 | 중간 | ⭐⭐⭐ | ⭐⭐ |
| 5. 타임스탬프 | 높음 | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| 6. 입력 기반 | 중간 | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |

---

## 7. 참고 정보

### 📂 관련 파일
- 소스 코드: `Hil_DemoApp\Sources\App_DemoApplication.c`
- 헤더 파일: `Hil_DemoApp\Includes\App_DemoApplication.h`
- 함수 위치: Line 370-410

### 🔗 핵심 함수
- `xChannelIORead()`: 입력 데이터 읽기
- `xChannelIOWrite()`: 출력 데이터 쓰기
- `App_IODataHandler()`: 500ms 주기로 호출 (Line 316)

---

**문서 끝**

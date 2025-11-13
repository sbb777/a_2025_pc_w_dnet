
# `App_IODataHandler` 함수 분석

`App_IODataHandler` 함수는 `App_CifXApplicationDemo`의 메인 루프 내에서 주기적으로 호출되며, netX 디바이스와의 실시간 I/O 데이터 교환을 담당합니다. 이 함수는 DPM(Dual-Port Memory)을 통해 빠르고 효율적인 데이터 전송을 수행합니다.

## 주요 기능

1.  **입력 데이터 읽기 (Input Data Handling):**
    *   `xChannelIORead()` 함수를 호출하여 netX 디바이스의 DPM에 있는 입력 데이터 영역(Area 0)의 데이터를 읽어옵니다.
    *   읽어온 데이터는 `ptAppData->tInputData` 구조체에 저장됩니다.
    *   **성공 시:** `xChannelIORead`가 `CIFX_NO_ERROR`를 반환하면, `ptAppData->fInputDataValid` 플래그를 `true`로 설정하여 수신된 데이터가 유효함을 표시합니다. 이후 애플리케이션 로직에서 이 데이터를 사용할 수 있습니다.
    *   **실패 시:** `xChannelIORead`가 오류를 반환하면, `ptAppData->fInputDataValid` 플래그를 `false`로 설정합니다. 오류는 주로 다음과 같은 상황에서 발생할 수 있습니다:
        *   netX 펌웨어가 아직 `ready` 또는 `running` 상태가 아닐 때 (초기화 중).
        *   필드버스 또는 이더넷 IO 연결이 아직 설정되지 않았을 때.

2.  **출력 데이터 쓰기 (Output Data Handling):**
    *   netX 디바이스로 전송할 출력 데이터를 준비합니다. 이 데모 코드에서는 `ptAppData->tOutputData.output[0]++`와 같이 첫 번째 출력 데이터 바이트를 단순히 1씩 증가시켜 데이터가 계속 변경됨을 시뮬레이션합니다.
    *   `xChannelIOWrite()` 함수를 호출하여 `ptAppData->tOutputData` 구조체에 있는 데이터를 netX 디바이스의 DPM에 있는 출력 데이터 영역(Area 0)에 씁니다.
    *   이 함수가 반환하는 오류 코드는 일반적으로 처리하지 않지만, 입력 데이터 읽기와 유사한 이유로 실패할 수 있습니다.

## 소스 코드

```c
void App_IODataHandler(void* ptAppResources)
{
  long            lRet      = CIFX_NO_ERROR;
  APP_DATA_T*     ptAppData = (APP_DATA_T*)ptAppResources;

  if(ptAppData->aptChannels[0]->hChannel != NULL)
  {
    /** INPUT DATA *********************************************************************/
    lRet = xChannelIORead(ptAppData->aptChannels[0]->hChannel, 0, 0, sizeof(ptAppData->tInputData), &ptAppData->tInputData, 0);
    if(lRet != CIFX_NO_ERROR)
    {
      ptAppData->fInputDataValid = false;
    }
    else
    {
      ptAppData->fInputDataValid = true;
    }

    /** OUTPUT DATA ***************************************/
    ptAppData->tOutputData.output[0]++;
    lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel, 0, 0, sizeof(ptAppData->tOutputData), &ptAppData->tOutputData, 0);
    if(lRet != CIFX_NO_ERROR)
    {
      // Error handling (optional)
    }
  }
}
```

## 요약

`App_IODataHandler`는 실시간 제어 애플리케이션의 핵심적인 부분으로, `xChannelIORead`와 `xChannelIOWrite`라는 두 가지 중요한 API를 사용하여 호스트(STM32)와 netX 디바이스 간의 주기적인 데이터 통신을 구현합니다. 이를 통해 센서 데이터 수신 및 액추에이터 제어와 같은 작업을 수행할 수 있습니다.


# `App_CifXApplicationDemo` 함수 분석

`App_CifXApplicationDemo` 함수는 cifX 툴킷을 사용하여 netX 디바이스와의 통신을 설정하고, 데이터 교환을 수행하는 메인 애플리케이션의 핵심 로직을 담당합니다.

## 주요 기능

1.  **초기화 및 드라이버 열기:**
    *   `xDriverOpen()` 함수를 호출하여 cifX 드라이버에 대한 핸들(`hDriver`)을 얻습니다. 이는 툴킷 API를 사용하기 위한 첫 단계입니다.
    *   `g_tAppData.fRunning` 플래그를 `true`로 설정하여 메인 통신 루프가 실행되도록 합니다.
    *   `calloc`을 사용하여 통신 채널 0에 대한 메모리를 할당하고, `g_tRealtimeProtocolHandlers`에 정의된 프로토콜 핸들러를 할당합니다.

2.  **보드 정보 읽기 및 채널 열기:**
    *   `App_ReadBoardInfo()`를 호출하여 연결된 netX 보드의 하드웨어 정보(DPM 크기, 시리얼 번호 등)를 읽고 출력합니다.
    *   `App_AllChannels_Open()` 함수를 통해 `szDeviceName`(예: "cifX0")으로 지정된 디바이스의 통신 채널을 엽니다.

3.  **통신 준비 및 상태 설정:**
    *   `App_AllChannels_GetChannelInfo_WaitReady()`를 호출하여 모든 통신 채널의 상태 플래그(`ulDeviceCOSFlags`)를 확인하고, `HIL_COMM_COS_READY` 비트가 설정될 때까지 대기합니다. 이는 netX 펌웨어가 통신할 준비가 되었음을 의미합니다.
    *   `xChannelHostState()` 함수를 사용하여 호스트(애플리케이션)의 상태를 `CIFX_HOST_STATE_READY`로 설정합니다. 이를 통해 netX 펌웨어는 호스트가 데이터를 교환할 준비가 되었음을 인지합니다.

4.  **구성 다운로드:**
    *   `App_AllChannels_Configure()` 함수를 호출하여 프로토콜에 필요한 구성 정보를 netX 디바이스로 다운로드합니다. 이 단계가 완료되면 버스 통신이 시작될 수 있습니다.

5.  **메인 통신 루프:**
    *   `while(g_tAppData.fRunning && lRet == CIFX_NO_ERROR)` 루프에 진입하여 주기적인 데이터 처리를 수행합니다.
    *   **I/O 데이터 처리:** `App_IODataHandler()`를 호출하여 주기적인(cyclic) I/O 데이터를 netX와 교환합니다.
    *   **패킷 처리:** `App_AllChannels_PacketHandler()`를 호출하여 비동기적인(acyclic) 메일박스 패킷을 처리합니다.
    *   `OS_Sleep(1)`: 1밀리초 동안 대기하여 CPU 점유율을 조절하고 다른 태스크가 실행될 기회를 줍니다.

6.  **종료 처리 (Error Handling & Cleanup):**
    *   루프가 종료되거나 오류가 발생하면 `error_exit` 레이블로 이동합니다.
    *   `xChannelBusState()`를 호출하여 버스 상태를 `CIFX_BUS_STATE_OFF`로 설정하고, `xChannelHostState()`를 `CIFX_HOST_STATE_NOT_READY`로 설정하여 통신을 중단시킵니다.
    *   `App_AllChannels_Close()`를 통해 열었던 모든 통신 채널을 닫습니다.
    *   할당된 채널 메모리를 `free()` 함수로 해제합니다.
    *   `xDriverClose()`를 호출하여 cifX 드라이버를 닫고 모든 관련 리소스를 해제합니다.

## 소스 코드

```c
int App_CifXApplicationDemo(char *szDeviceName)
{
  CIFXHANDLE hDriver  = NULL;
  int32_t   lRet      = 0;
  uint32_t  ulState   = 0;
  uint32_t  ulTimeout = 1000;

  g_tAppData.fRunning = true;

  g_tAppData.aptChannels[0] = (APP_COMM_CHANNEL_T*)calloc(1, sizeof(APP_COMM_CHANNEL_T));
  g_tAppData.aptChannels[0]->tProtocol = g_tRealtimeProtocolHandlers;

  if (CIFX_NO_ERROR != (lRet = xDriverOpen(&hDriver)))
  {
    return lRet;
  }

  App_ReadBoardInfo(hDriver, &g_tAppData.tBoardInfo);

  if (CIFX_NO_ERROR != App_AllChannels_Open(&g_tAppData, hDriver, szDeviceName))
  {
    goto error_exit;
  }

  App_AllChannels_GetChannelInfo_WaitReady(&g_tAppData);

  if (CIFX_NO_ERROR != (lRet = xChannelHostState(g_tAppData.aptChannels[0]->hChannel, CIFX_HOST_STATE_READY, &ulState, ulTimeout)))
  {
    goto error_exit;
  }

  if (CIFX_NO_ERROR != (lRet = App_AllChannels_Configure(&g_tAppData)))
  {
    goto error_exit;
  }

  while(g_tAppData.fRunning && lRet == CIFX_NO_ERROR)
  {
    App_IODataHandler(&g_tAppData);
    lRet = App_AllChannels_PacketHandler(&g_tAppData);
    OS_Sleep(1);
  }

error_exit:
  // ... Cleanup code ...
  xDriverClose(hDriver);
  return lRet;
}
```

## 요약

`App_CifXApplicationDemo`는 cifX 툴킷 API를 사용하여 netX 디바이스와의 통신 라이프사이클(초기화, 구성, 데이터 교환, 종료)을 관리하는 표준적인 절차를 보여줍니다. 오류 처리와 리소스 정리를 포함하여 안정적인 애플리케이션의 골격을 제공합니다.

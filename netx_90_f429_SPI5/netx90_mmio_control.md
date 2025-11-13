# netX 90 MMIO7 제어

이 문서는 STM32 호스트에서 netX 90의 MMIO7을 제어하는 방법을 설명합니다. 제공된 코드는 `cifX` 툴킷을 사용하여 MMIO를 제어하기 위한 패킷 기반 통신을 사용합니다.

## 개요

netX 90의 MMIO는 `cifX` 툴킷의 패킷 통신을 사용하여 제어할 수 있습니다. 이 방법은 DPM에 직접 쓰는 것보다 더 구조적이고 안정적인 접근 방식을 제공합니다. `DIO_SET_LINE_CONFIG_REQ` 패킷을 사용하여 MMIO 라인을 출력으로 구성한 다음, `DIO_SET_LINE_STATE_REQ` 패킷을 사용하여 상태를 설정할 수 있습니다.

## MMIO7 제어 코드

다음 코드는 MMIO7을 출력으로 설정하는 방법을 보여줍니다.

### `mmio_control.h`

```c
#ifndef MMIO_CONTROL_H_
#define MMIO_CONTROL_H_

#include "cifXUser.h"

// App_DemoApplication.c에서 복사
#define DRV_DIO_LINE_MMIO 1

#pragma pack(push, 1)
typedef struct DIO_SET_LINE_CONFIG_REQ_Ttag {
    CIFX_PACKET tHeader;
    uint32_t ulLine;
    uint32_t ulType;
    uint32_t ulFlags;
} DIO_SET_LINE_CONFIG_REQ;
#pragma pack(pop)

int32_t SetMMIO7AsOutput(CIFXHANDLE hChannel);

#endif // MMIO_CONTROL_H_
```

### `mmio_control.c`

```c
#include "mmio_control.h"
#include <string.h> // memset용

/**
 * @brief MMIO7을 출력으로 구성합니다.
 *
 * @param hChannel 채널 핸들.
 * @return 성공 시 CIFX_NO_ERROR, 그렇지 않으면 오류 코드.
 */
int32_t SetMMIO7AsOutput(CIFXHANDLE hChannel)
{
    DIO_SET_LINE_CONFIG_REQ tCfg;
    memset(&tCfg, 0, sizeof(tCfg));

    tCfg.tHeader.ulDest = 0x20;
    tCfg.tHeader.ulCmd = 0xB1; // DIO_SET_LINE_CONFIG
    tCfg.tHeader.ulLen = sizeof(tCfg) - sizeof(tCfg.tHeader);
    tCfg.ulLine = 7; // MMIO7
    tCfg.ulType = DRV_DIO_LINE_MMIO;
    tCfg.ulFlags = 0; // 출력

    int32_t lRet = xChannelPutPacket(hChannel, (CIFX_PACKET*)&tCfg, 1000);
    if (lRet != CIFX_NO_ERROR)
    {
        // 필요한 경우 오류 처리, 지금은 오류 코드만 반환
    }

    return lRet;
}
```

## 코드 통합

위 코드를 기존 프로젝트에 통합하려면 다음 단계를 따르십시오.

1.  **파일 추가**:
    *   `Hil_DemoApp/Includes/mmio_control.h`에 `mmio_control.h` 파일을 만듭니다.
    *   `Hil_DemoApp/Sources/mmio_control.c`에 `mmio_control.c` 파일을 만듭니다.
2.  **`App_DemoApplication.c` 수정**:
    *   `#include "mmio_control.h"`를 `App_DemoApplication.c`의 다른 include문과 함께 추가합니다.
    *   `App_CifXApplicationDemo` 함수에서 `App_AllChannels_Configure`를 호출한 후 `SetMMIO7AsOutput(g_tAppData.aptChannels[0]->hChannel);`를 호출합니다.

```c
// App_CifXApplicationDemo in App_DemoApplication.c

// ...

  /* Download the configuration */
  if (CIFX_NO_ERROR != (lRet = App_AllChannels_Configure(&g_tAppData)))  //Protocol_StartConfiguration
  {
    PRINTF("Error: A channel failed to configure: 0x%08X\n", (unsigned int)lRet);
    goto error_exit;
  }

  // MMIO7을 출력으로 설정
  if (CIFX_NO_ERROR != (lRet = SetMMIO7AsOutput(g_tAppData.aptChannels[0]->hChannel)))
  {
      PRINTF("Error: Failed to set MMIO7 as output: 0x%08X\n", (unsigned int)lRet);
      goto error_exit;
  }

  /** now the bus is running */
  while(g_tAppData.fRunning && lRet == CIFX_NO_ERROR)
  {
// ...
```

## 주의사항

*   **명령 및 대상**: `DIO_SET_LINE_CONFIG`에 사용된 명령(0xB1)과 대상(0x20)은 netX 펌웨어 설명서를 참조하여 확인해야 합니다.
*   **채널 핸들**: 이 코드는 `aptChannels[0]`을 사용합니다. 시스템 구성에 따라 다른 채널을 사용해야 할 수도 있습니다.

```
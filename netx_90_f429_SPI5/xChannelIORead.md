
# `xChannelIORead` 함수 분석

`xChannelIORead` 함수는 cifX 툴킷 API의 일부로, 특정 통신 채널의 I/O 입력 영역(Input Area)에서 데이터를 읽는 데 사용됩니다. 이 함수는 DPM(Dual-Port Memory) 직접 접근 방식과 DMA(Direct Memory Access) 전송 방식을 모두 지원합니다.

## 주요 기능

1.  **파라미터 유효성 검사:**
    *   `hChannel`: 유효한 채널 핸들인지 확인합니다.
    *   `DEV_IsRunning()`: 해당 채널의 펌웨어가 실행 중(`running`) 상태인지 확인합니다. 아니라면 `CIFX_DEV_NOT_RUNNING` 오류를 반환합니다.
    *   `ulAreaNumber`: 요청된 I/O 영역 번호가 유효한 범위 내에 있는지 확인합니다. 범위를 벗어나면 `CIFX_INVALID_PARAMETER`를 반환합니다.
    *   `ulOffset + ulDataLen`: 읽으려는 데이터의 총 크기(오프셋 + 길이)가 해당 I/O 영역의 전체 크기를 초과하는지 확인합니다. 초과하면 `CIFX_INVALID_ACCESS_SIZE` 오류를 반환합니다.

2.  **동작 모드 결정 (DPM vs. DMA):**
    *   채널의 상태 플래그(`ulDeviceCOSFlags`)에 `HIL_COMM_COS_DMA` 비트가 설정되어 있는지 확인하여 DMA 모드 여부를 결정합니다.

3.  **데이터 읽기 (DPM 모드):**
    *   **핸드셰이크(Handshake) 모드:**
        *   `DEV_GetIOBitstate()`를 통해 현재 필요한 핸드셰이크 비트의 상태를 확인합니다.
        *   `DEV_WaitForBitState()` 함수를 사용하여 DPM의 핸드셰이크 플래그가 올바른 상태가 될 때까지 `ulTimeout` 시간 동안 대기합니다. 이는 netX 펌웨어가 새로운 데이터를 DPM에 썼음을 보장합니다.
        *   타임아웃이 발생하면 `CIFX_DEV_EXCHANGE_FAILED` 오류를 반환합니다.
        *   핸드셰이크가 성공하면 `HWIF_READN()` 매크로를 사용하여 DPM의 지정된 오프셋에서 `ulDataLen` 만큼의 데이터를 사용자 버퍼(`pvData`)로 복사합니다.
        *   `DEV_ToggleBit()` 함수를 호출하여 핸드셰이크 비트를 토글(toggle)함으로써, 호스트가 데이터를 읽었음을 netX 펌웨어에 알립니다.
    *   **핸드셰이크 없는(Non-Handshake) 모드:**
        *   별도의 대기 과정 없이 `HWIF_READN()`을 사용하여 즉시 DPM에서 데이터를 읽습니다. 이 경우 데이터의 일관성(consistency)은 보장되지 않을 수 있습니다.

4.  **데이터 읽기 (DMA 모드):**
    *   DMA 모드에서는 I/O 영역 0만 지원됩니다. 다른 영역을 요청하면 `CIFX_DEV_DMA_IO_AREA_NOT_SUPPORTED` 오류를 반환합니다.
    *   DPM 모드의 핸드셰이크 방식과 유사하게 `DEV_WaitForBitState()`로 DMA 전송 완료를 기다립니다.
    *   전송이 완료되면, DPM 대신 호스트 메모리에 있는 DMA 버퍼에서 `OS_Memcpy()`를 사용하여 데이터를 사용자 버퍼로 복사합니다.
    *   `DEV_ToggleBit()`로 핸드셰이크 비트를 토글하여 DMA 읽기 완료를 알립니다.

5.  **동기화 및 오류 처리:**
    *   `OS_WaitMutex()`와 `OS_ReleaseMutex()`를 사용하여 I/O 영역에 대한 동시 접근을 막고 데이터 무결성을 보장합니다.
    *   `DEV_IsCommunicating()` 함수를 통해 통신 상태를 최종적으로 확인하고, 오류가 있을 경우 `lRet`에 해당 오류 코드를 설정하여 반환합니다.

## 소스 코드

```c
int32_t APIENTRY xChannelIORead(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData, uint32_t ulTimeout)
{
  PCHANNELINSTANCE ptChannel   = (PCHANNELINSTANCE)hChannel;
  int32_t          lRet        = CIFX_NO_ERROR;
  PIOINSTANCE      ptIOArea    = NULL;
  uint8_t          bIOBitState = HIL_FLAGS_NONE;

  if(!DEV_IsRunning(ptChannel))
    return CIFX_DEV_NOT_RUNNING;

  if(ulAreaNumber >= ptChannel->ulIOInputAreas)
    return CIFX_INVALID_PARAMETER;

  ptIOArea    = ptChannel->pptIOInputAreas[ulAreaNumber];
  bIOBitState = DEV_GetIOBitstate(ptChannel, ptIOArea, 0);

#ifdef CIFX_TOOLKIT_DMA
  if( ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_DMA) {
    // DMA-specific logic
  } else
#endif
  {
    // DPM-specific logic
    if( (ulOffset + ulDataLen) > ptIOArea->ulDPMAreaLength)
      return CIFX_INVALID_ACCESS_SIZE;

    if ( !OS_WaitMutex( ptIOArea->pvMutex, ulTimeout))
      return CIFX_DRV_CMD_ACTIVE;

    if(HIL_FLAGS_NONE == bIOBitState)
    {
      HWIF_READN(ptChannel->pvDeviceInstance, pvData, &ptIOArea->pbDPMAreaStart[ulOffset], ulDataLen);
      (void)DEV_IsCommunicating(ptChannel, &lRet);
    } else if(!DEV_WaitForBitState(ptChannel, ptIOArea->bHandshakeBit, bIOBitState, ulTimeout))
    {
      lRet = CIFX_DEV_EXCHANGE_FAILED;
    } else
    {
      HWIF_READN(ptChannel->pvDeviceInstance, pvData, &ptIOArea->pbDPMAreaStart[ulOffset], ulDataLen);
      DEV_ToggleBit(ptChannel, (uint32_t)(1UL << ptIOArea->bHandshakeBit));
      (void)DEV_IsCommunicating(ptChannel, &lRet);
    }

    OS_ReleaseMutex( ptIOArea->pvMutex);
  }

  return lRet;
}
```

## 요약

`xChannelIORead`는 netX 디바이스로부터 입력 데이터를 가져오는 표준화된 방법을 제공하는 저수준(low-level) 함수입니다. 핸드셰이크 메커니즘을 통해 데이터 교환의 동기화를 맞추고, DPM과 DMA 두 가지 하드웨어 통신 방식을 모두 지원하여 유연성을 제공합니다.

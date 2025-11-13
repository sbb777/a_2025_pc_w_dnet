/**************************************************************************//**
 * @file
 * @brief   The digital input output example main file.
 * $Revision: 4564 $
 * $Date: 2018-11-26 12:54:42 +0100 (Mo, 26 Nov 2018) $
 * \copyright Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 * \note Exclusion of Liability for this demo software:
 * The following software is intended for and must only be used for reference and in an
 * evaluation laboratory environment. It is provided without charge and is subject to
 * alterations. There is no warranty for the software, to the extent permitted by
 * applicable law. Except when otherwise stated in writing the copyright holders and/or
 * other parties provide the software "as is" without warranty of any kind, either
 * expressed or implied.
 * Please refer to the Agreement in README_DISCLAIMER.txt, provided together with this file!
 * By installing or otherwise using the software, you accept the terms of this Agreement.
 * If you do not agree to the terms of this Agreement, then do not install or use the
 * Software!
 ******************************************************************************/

#include "main.h"

#include <stdbool.h>

/*!
 * A calibration value for the bussy delay function.
 * The intram is around two times faster than sdram.
 * This value is for sdram.
 */
size_t const ulCalibrationValue = 41u;


void delay(size_t ulMiliseconds);
void dioChaseLights(void);
void Error_Handler(void);




static void TransactionCompleteCallback(DRV_SPI_HANDLE_T* ptSPIHandle, void* pvUser);

void Error_Handler(void);



/** Buffer used for SPI slave reception and transmission */

static uint16_t aSlaveRxBuffer[BUFFER_SIZE] = {0}; // 수신 버퍼

static uint16_t aSlaveTxBuffer[BUFFER_SIZE] =    // 전송 버퍼 (응답 데이터)

{

 0x1000, 0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006, 0x1007,

 0x1008, 0x1009, 0x100A, 0x100B, 0x100C, 0x100D, 0x100E, 0x100F,

 0x1010, 0x1011, 0x1012, 0x1013, 0x1014, 0x1015, 0x1016, 0x1017,

 0x1018, 0x1019, 0x101A, 0x101B, 0x101C, 0x101D, 0x101E, 0x101F,

 0x1020, 0x1021, 0x1022, 0x1023, 0x1024, 0x1025, 0x1026, 0x1027,

 0x1028, 0x1029, 0x102A, 0x102B, 0x102C, 0x102D, 0x102E, 0x102F,

 0x1030, 0x1031, 0x1032, 0x1033, 0x1034, 0x1035, 0x1036, 0x1037,

 0x1038, 0x1039, 0x103A, 0x103B, 0x103C, 0x103D, 0x103E, 0x103F

};



static volatile bool bTransactionComplete = false; // 전송 완료 플래그



/*!

* \brief The main of the SPI slave example project.

*

* SPI0를 slave 모드로만 구성하여 외부 마스터의 통신을 대기합니다.

*/

int main(void)

{

 DRV_SPI_HANDLE_T tSPISlaveHandle = {{0}}; /** SPI slave handler */

 DRV_STATUS_E   eRet;           /** driver status */

 uint32_t     ulUserParam = 5678;    /** user parameter */

 uint32_t     ulTransactionCount = 0;  /** transaction counter */



 /** Turn LED3 off (초기 상태) */

 DRV_DIO_ChannelOutReset(DRV_DIO_ID_MMIO_7);



 /** Configure SPI0 as slave */

 tSPISlaveHandle.tConfiguration.eOperationMode     = DRV_OPERATION_MODE_IRQ;      /** 인터럽트 모드 */

 tSPISlaveHandle.tConfiguration.eFSS          = DRV_SPI_FSS_0;          /** FSS0 사용 */

 tSPISlaveHandle.tConfiguration.eFSSStatic       = DRV_SPI_FSS_STATIC_HARDWARE;   /** 하드웨어 제어 FSS */

 tSPISlaveHandle.tConfiguration.uMode.value      = DRV_SPI_MODE_3;          /** SPI Mode 3 */

 tSPISlaveHandle.tConfiguration.eFILTER        = DRV_SPI_FILTER_INACTIVE;     /** 필터 비활성화 */

 tSPISlaveHandle.tConfiguration.eSlaveSigEarly     = DRV_SPI_SLV_SIG_NOT_EARLY;    /** Early signal 비활성화 */

 tSPISlaveHandle.tConfiguration.eDataSize       = DRV_SPI_DATA_SIZE_SELECT_16b;   /** 16비트 데이터 */

 tSPISlaveHandle.tConfiguration.eRxFiFoWm       = DRV_SPI_FIFO_WM_DEFAULT;     /** RX FIFO 워터마크 */

 tSPISlaveHandle.tConfiguration.eTxFiFoWm       = DRV_SPI_FIFO_WM_DEFAULT;     /** TX FIFO 워터마크 */

 tSPISlaveHandle.tConfiguration.eFrequency       = DRV_SPI_FREQUENCY_1_56MHz;    /** 최대 주파수 */

 tSPISlaveHandle.tConfiguration.eBehaviour       = DRV_SPI_BEHAVIOUR_SLAVE;     /** 슬레이브 모드 */

 tSPISlaveHandle.tConfiguration.eSPIDeviceID      = DRV_SPI_DEVICE_ID_SPI0;      /** SPI0 디바이스 */

 tSPISlaveHandle.tConfiguration.fnCompleteCallback   = (DRV_CALLBACK_F) TransactionCompleteCallback;

 tSPISlaveHandle.tConfiguration.pCompleteCallbackHandle= (void*)&ulUserParam;



 /** Initialize SPI0 slave */

 if(DRV_OK != (eRet = DRV_SPI_Init(&tSPISlaveHandle)))

 {

  /** SPI0 초기화 실패 */

  Error_Handler();

 }



 /** LED3를 1초간 켜서 초기화 완료 표시 */

 DRV_DIO_ChannelOutSet(DRV_DIO_ID_MMIO_7);

 for(volatile uint32_t i = 0; i < 1000000; i++); // 간단한 딜레이

 DRV_DIO_ChannelOutReset(DRV_DIO_ID_MMIO_7);



 /** 메인 루프: 지속적으로 SPI 통신 대기 */

 while(1)

 {

  /** Reset transaction complete flag */

  bTransactionComplete = false;



  /** SPI slave 통신 준비 - 수신과 송신 동시 설정 */

  eRet = DRV_SPI_TransmitReceive(&tSPISlaveHandle,

                 (uint8_t*)aSlaveTxBuffer,

                 (uint8_t*)aSlaveRxBuffer,

                 BUFFER_SIZE * 2);

  if(DRV_OK != eRet)

  {

   /** 통신 설정 실패 시 재시도 */

   continue;

  }



  /** 전송 완료 대기 */

  while(!bTransactionComplete)

  {

   /** 대기 중... 콜백에서 플래그가 설정될 때까지 */

  }



  /** 트랜잭션 완료 후 처리 */

  ulTransactionCount++;

  /** LED3를 짧게 깜빡여서 통신 완료 표시 */

  DRV_DIO_ChannelOutSet(DRV_DIO_ID_MMIO_7);

  for(volatile uint32_t i = 0; i < 100000; i++); // 짧은 딜레이

  DRV_DIO_ChannelOutReset(DRV_DIO_ID_MMIO_7);



  /** 수신된 데이터를 다음 전송 데이터에 반영 (에코 + 카운터) */

  for(uint32_t i = 0; i < BUFFER_SIZE; i++)

  {

   aSlaveTxBuffer[i] = aSlaveRxBuffer[i] + ulTransactionCount;

  }

 }



 /** 이 코드는 도달하지 않음 (무한 루프) */

 DRV_SPI_DeInit(&tSPISlaveHandle);

}



/*!

* \brief Transaction complete callback function.

*

* SPI 통신이 완료되면 호출되는 콜백 함수

* \param[in,out] ptSPIHandle SPI 핸들 포인터

* \param[in,out] pvUser 사용자 파라미터 포인터

*/

void TransactionCompleteCallback(DRV_SPI_HANDLE_T* ptSPIHandle, void* pvUser)

{

 DRV_SPI_STATE_E eSPIState;

 uint32_t* pulUserParam = (uint32_t*)pvUser;



 /** SPI 상태 확인 */

 DRV_SPI_GetState(ptSPIHandle, &eSPIState);

 /** FIFO 오류 체크 */

 if((eSPIState & (DRV_SPI_STATE_RX_FIFO_UNDERRUN |

         DRV_SPI_STATE_RX_FIFO_OVERFLOW |

         DRV_SPI_STATE_TX_FIFO_UNDERRUN |

         DRV_SPI_STATE_TX_FIFO_OVERFLOW)) != 0)

 {

  /** FIFO 오류 발생 */

  Error_Handler();

 }



 /** 전송 완료 플래그 설정 */

 bTransactionComplete = true;

 /** 사용자 파라미터 확인 (옵션) */

 if(pulUserParam && (*pulUserParam == 5678))

 {

  /** 올바른 콜백 호출 확인됨 */

 }

}



/*!

* \brief Error handler function.

*

* 오류 발생 시 LED3를 켜고 무한 루프에 진입

*/

void Error_Handler(void)

{

 /** LED3를 켜서 오류 상태 표시 */

 DRV_DIO_ChannelOutSet(DRV_DIO_ID_MMIO_7);

 /** 무한 루프 */

 while(1)

 {

  /** 오류 상태에서 대기 */

 }

}

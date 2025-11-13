/**************************************************************************//**
 * @file    main.c
 * @brief   main file for embedded netx90 targets: e.g. NXHX90-JTAG
 * $Revision: 11134 $
 * $Date: 2024-02-13 09:36:07 +0100 (Di, 13 Feb 2024) $
 ******************************************************************************/

/**************************************************************************************
 Exclusion of Liability for this demo software:
 The following software is intended for and must only be used for reference and in an
 evaluation laboratory environment. It is provided without charge and is subject to
 alterations. There is no warranty for the software, to the extent permitted by
 applicable law. Except when otherwise stated in writing the copyright holders and/or
 other parties provide the software "as is" without warranty of any kind, either
 expressed or implied.
 Please refer to the Agreement in README_DISCLAIMER.txt, provided together with this file!
 By installing or otherwise using the software, you accept the terms of this Agreement.
 If you do not agree to the terms of this Agreement, then do not install or use the
 Software!
 **************************************************************************************/

/**************************************************************************************

 Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.


 **************************************************************************************/

#include "App_DemoApplication.h"
#include "libc_support.h"
#include "cifXToolkit.h"
#include "cifXErrors.h"

#include "netx_drv.h"
#include "stdlib.h"

static void WaitForDpmReady(const PDEVICEINSTANCE ptDevInstance)
{
  /* In order to make sure, that DPM was initialized by the COM CPU,
   * we just wait here until the expected cookie "netX" appears. */
  char szCookie[5];
  do {
      OS_Memset(szCookie, 0, sizeof(szCookie));
      HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);
  }while(0 != OS_Strcmp( szCookie, CIFX_DPMSIGNATURE_FW_STR));
}

static void InitializeToolkit(void)
{
  int32_t lRet = CIFX_NO_ERROR;
  static PDEVICEINSTANCE ptDevInstance;

  lRet = cifXTKitInit();

  if(CIFX_NO_ERROR == lRet)
  {
    ptDevInstance = (PDEVICEINSTANCE)malloc(sizeof(*ptDevInstance));
    memset(ptDevInstance, 0, sizeof(*ptDevInstance));

    /** Set trace level of toolkit */
    g_ulTraceLevel =  TRACE_LEVEL_ERROR   |
                      TRACE_LEVEL_WARNING |
                      TRACE_LEVEL_INFO    |
                      TRACE_LEVEL_DEBUG;

    /** Insert the basic device information into the DeviceInstance structure
          for the toolkit.
          NOTE: The irq number are for information use only, so we skip them here.
          Interrupt is currently not supported and ignored, so we don't need to set it */
    ptDevInstance->ulPhysicalAddress = 0xB0000000U;   //Addr_NX90_idpm_slave_mirror_ext_peri;
    ptDevInstance->ulIrqNumber        = 3;
    ptDevInstance->fIrqEnabled       = 0;
    ptDevInstance->fPCICard          = 0;
    ptDevInstance->fModuleLoad       = 0;

    ptDevInstance->eDeviceType       = eCIFX_DEVICE_DONT_TOUCH;
    ptDevInstance->pfnNotify         = NULL;
    ptDevInstance->pvOSDependent     = ptDevInstance;
    ptDevInstance->ulDPMSize         = 0x8000;        //32K
    ptDevInstance->pbDPM             = (uint8_t*) ptDevInstance->ulPhysicalAddress;
    strncpy(ptDevInstance->szName, "cifX0", sizeof(ptDevInstance->szName));

    /** Wait until the DPM was initialized by the COM CPU */
    WaitForDpmReady(ptDevInstance);

    /** Add the device to the toolkits handled device list */
    lRet = cifXTKitAddDevice(ptDevInstance);

    /** If it succeeded do device tests */
    if(CIFX_NO_ERROR != lRet)
    {
      /** Uninitialize Toolkit, this will remove all handled boards from the toolkit and
            deallocate the device instance */
      cifXTKitDeinit();
    }
  }

  while(CIFX_NO_ERROR != lRet);  /** do not return on init failure */

}

/*****************************************************************************/
/*! Timer interrupt handler                                                  */
/*****************************************************************************/
void DRV_TIM_TIMER1_Callback(void)
{
  App_ApplicationTimerJob();
}

/* Arm Timer 1 is used for handling the registered Timer callback */
static DRV_TIM_HANDLE_T s_tArmTimer1;

static void InitializeTimer()
{
  /** ArmTimer1 1ms interrupt */
  s_tArmTimer1.tConfiguration.eDeviceID      = DRV_TIM_DEVICE_ID_TIMER1;
  s_tArmTimer1.tConfiguration.eOperationMode = DRV_OPERATION_MODE_IRQ;
  s_tArmTimer1.tConfiguration.eCountingMode  = DRV_TIM_COUNTING_MODE_CONTINUOUS;
  s_tArmTimer1.tConfiguration.tPreloadValue  = DRV_TIM_PRELOAD_VALUE_1ms;
  DRV_TIM_Init(&s_tArmTimer1);
  while(DRV_OK != DRV_TIM_Start(&s_tArmTimer1))
    ;
}

int main()
{

  libcInit();

  InitializeToolkit();

  InitializeTimer();

  App_ApplicationDemo("cifX0");

  while(1);  /** do not return from main() */
}




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


 ***************************************************************************************


 **************************************************************************************/

#include "libc_support.h"
#include "netx_drv.h"

void libcInit(void)
{
  /* Libc memory initialisation */
  libc_mem_init();

  /* Init the libc part & redirect printf output to uart */
  libc_file_init();

  /* Deactivate all buffering because we do not want buffering. */
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
  /* This will issue the libc to read each char separately from the console
   * instead of requesting a string of the buffers length. */
  setvbuf(stdin, NULL, _IONBF, 0);

  printf("\r\nUART and libc initialized.\r\n");
}

int _fini(int a)
{
  // An exit was called without an operating system.
  __BKPT();
  DRV_NVIC_SetPendingIRQ(HardFault_IRQn);
  return -1;
}

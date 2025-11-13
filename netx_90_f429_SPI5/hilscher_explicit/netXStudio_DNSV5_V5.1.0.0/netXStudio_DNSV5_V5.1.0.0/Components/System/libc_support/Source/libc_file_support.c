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
 
 Copyright (c) Hilscher GmbH. All Rights Reserved.
 
 **************************************************************************************
 
 Filename:
 $Workfile: libc_file_support.c $
 Last Modification:
 $Author: dirk $
 $Modtime: 5.02.07 12:31 $
 $Revision: 2191 $

 Targets:
 netX10/50/100/500   : yes

 Description:
 libc_file_support.c  Operating system specific functions for libc file support (fopen, ...)
 
 Changes:
 
 Version   Date        Author   Description
 ----------------------------------------------------------------------------------
 1        13.12.2011  SS       initial version
 
 **************************************************************************************/

/*****************************************************************************/
/*! \file libc_file_support.c                                                    
 *   Operating system specific functions for libc file support (fopen, ...)   */
/*****************************************************************************/

#include "libc_file_support.h"
#include "netx_drv.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/reent.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <string.h> /* memset, etc */
#include <stdint.h>

#undef errno
extern int errno;

DRV_UART_HANDLE_T g_tUART = { 0 }; /* global Uart handle */

/*****************************************************************************/
/*! Initialize the libc file support                                         */
/*****************************************************************************/
void libc_file_init()
{
  _REENT_INIT_PTR(_global_impure_ptr);
  _impure_ptr = _global_impure_ptr;

  if(module_firewall_ctrl->firewall_cfg_intlogic_shd_uart_b.rp_app == 1 &&
  module_firewall_ctrl->firewall_cfg_intlogic_shd_uart_b.wp_app == 1)
  {
    g_tUART.tConfiguration.eDeviceID = DRV_UART_DEVICE_ID_UART2;
  }
  else
  {
    g_tUART.tConfiguration.eDeviceID = DRV_UART_DEVICE_ID_UART0;
  }
  /* Init global Uart handle */
  g_tUART.tConfiguration.eOperationMode = DRV_OPERATION_MODE_POLL;
  g_tUART.tConfiguration.eBaudrate = DRV_UART_BAUDRATE_115200;
  g_tUART.tConfiguration.eWordLength = DRV_UART_WORD_LENGTH_8_BITS;
  g_tUART.tConfiguration.eLineControl = DRV_UART_LINE_CONTROL_MASK_FIFO_ENABLE;
  g_tUART.tConfiguration.eLoopBack = DRV_UART_LOOP_BACK_INACTIVE;
  while(DRV_OK != DRV_UART_Init(&g_tUART))
  {
  }
}

/*****************************************************************************/
/*! Opens a file (non-reentrant)
 *   \param szFile     Name and path of the file to open
 *   \param iFlags     Flags for file open (see O_RDONLY, O_WRONLY, O_RDWR, ...
 *   \param iMode      Sharing mode (see S_IRWXU, ...)
 *   \return -1 on error (errno holds last error)                             */
/*****************************************************************************/
int _open(const char* szFile, int iFlags, int iMode)
{
  return -1;
}

/*****************************************************************************/
/*! Closes a file (non-reentrant)
 *   \param iFile  File Handle
 *   \return -1 on error (errno holds last error)                             */
/*****************************************************************************/
int _close(int iFile)
{
  return -1;
}

/*****************************************************************************/
/*! Queries state of a an open file (non-reentrant)
 *   \param iFile    File Handle
 *   \param ptState  Returned state
 *   \return -1 on error (errno holds last error)                             */
/*****************************************************************************/
int _fstat(int iFile, struct stat* ptState)
{
  if((STDOUT_FILENO == iFile) || (STDERR_FILENO == iFile) || (STDIN_FILENO == iFile))
  {
    ptState->st_mode = S_IFCHR;
    return 0;
  }
  else
  {
    errno = EBADF;
    return -1;
  }
}

/*****************************************************************************/
/*! Queries state of a file (non-reentrant)
 *   \param szName   Name of the file to get status for
 *   \param ptState  Returned state
 *   \return -1 on error (errno holds last error)                             */
/*****************************************************************************/
int _stat(const char* szName, struct stat* ptState)
{
  return -1;
}

/*****************************************************************************/
/*! Queries if the file is a terminal (stdout)
 *   \param iFile    File Handle
 *   \return !=0 if device is a terminal/tty device                           */
/*****************************************************************************/
int _isatty(int iFile)
{
  int iRet;

  switch (iFile)
  {
  case STDERR_FILENO:
  case STDOUT_FILENO:
  case STDIN_FILENO:
    iRet = 1;
    break;

  default:
    iRet = 0;
    break;
  }

  return iRet;
}

/*****************************************************************************/
/*! Establish a new name for an exisiting file (non-reentrant)
 *   \param szOldName  Old file name/path
 *   \param szNewName  New file name/path
 *   \return -1 on error (errno holds error)                                  */
/*****************************************************************************/
int _link(const char* szOldName, const char* szNewName)
{
  return -1;
}

/*****************************************************************************/
/*! Removes a file's directory entry (non-reentrant)
 *   \param szName   Name of the file to delete
 *   \return -1 on error (errno holds error)                                  */
/*****************************************************************************/
int _unlink(const char* szName)
{
  return -1;
}

/*****************************************************************************/
/*! Reads from a file (non-reentrant)
 *   \param iFile      File handle
 *   \param pvBuffer   Data to write to file
 *   \param iBytes     number of bytes to write
 *   \return number of bytes successfully read (-1 on error)                  */
/*****************************************************************************/
_ssize_t _read(int iFile, void* pvBuffer, size_t iBytes)
{
  DRV_STATUS_E ret = DRV_ERROR;
  switch (iFile)
  {
  case STDIN_FILENO:
    ret = DRV_UART_Receive(&g_tUART, pvBuffer, iBytes);
    while(ret == DRV_LOCKED)
    {
      ret = DRV_UART_Receive(&g_tUART, pvBuffer, iBytes);
    }
    break;

  default:
    //TODO: Implement for further files
    break;
  }
  if(ret != DRV_OK)
  {
    iBytes = (size_t) -1;
  }
  return iBytes;
}

/*****************************************************************************/
/*! Writes to a file (non-reentrant)
 *   \param iFile      File handle
 *   \param pvBuffer   Data to write to file
 *   \param iBytes     number of bytes to write
 *   \return number of bytes successfully written                             */
/*****************************************************************************/
_ssize_t _write(int iFile, const void* pvBuffer, size_t iBytes)
{
  DRV_STATUS_E ret = DRV_ERROR;
  switch (iFile)
  {
  case STDERR_FILENO:
  case STDOUT_FILENO:
    ret = DRV_UART_Transmit(&g_tUART, pvBuffer, iBytes);
    while(ret == DRV_LOCKED)
    {
      ret = DRV_UART_Transmit(&g_tUART, pvBuffer, iBytes);
    }
    break;
  default:
    //TODO: Implement for further files
    break;
  }

  if(ret != DRV_OK)
  {
    iBytes = (size_t) -1;
  }
  return iBytes;
}

/*****************************************************************************/
/*! Writes to a file (non-reentrant)
 *   \param iFile      File handle
 *   \param iOffset    Offset to adjust
 *   \param iDirection Direction of iOffset (SEEK_SET, SEEK_CUR, SEEK_END)
 *   \return new file offset                                                  */
/*****************************************************************************/
_off_t _lseek(int iFile, _off_t iOffset, int iDirection)
{
  return 0;
}

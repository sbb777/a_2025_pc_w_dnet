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

  $Id: OS_Custom.c 14561 2022-07-26 13:22:39Z RMayer $:

  Description:
    Target system abstraction layer

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2022-04-14  Added options and functions to handle cached I/O buffer access via PLC functions
    2021-09-01  - updated function parameters to match definitions in OS_Dependent.h.
                - changed OS-Time() parameters to 64Bit data types
    2011-12-13  added OS_Time() function body
    2006-08-07  initial version

**************************************************************************************/

/*****************************************************************************/
/*! \file OS_Custom.c
*    Sample Target system abstraction layer. Implementation must be done
*    according to used target system                                         */
/*****************************************************************************/

/* Standard includes */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "OS_Includes.h"
#include "cifXErrors.h"
#include "OS_Dependent.h"
#include "cifXToolkit.h"
#include "netx90_app.h"

#include "netx_drv.h"


static bool s_fInitialized = 0;

/* Systick is used for counting 1 ms ticks */
static DRV_TIM_HANDLE_T s_tSystick;

#define NETX90_IDPM_IRQ_PRIORITY   7

static DEVICEINSTANCE* s_ptNetX90_IrqDevInst = NULL;


/*****************************************************************************/
/*!  \addtogroup CIFX_TK_OS_ABSTRACTION Operating System Abstraction
*    \{                                                                      */
/*****************************************************************************/

/* idle time processing, provided in the application */
void (*g_pfnOSAL_Idle)(void);

/*****************************************************************************/
/*! Memory allocation function
*   \param ulSize    Length of memory to allocate
*   \return Pointer to allocated memory                                      */
/*****************************************************************************/
void* OS_Memalloc(uint32_t ulSize)
{
  void* pt = malloc(ulSize);

  /* Toolkit expect this function to work,
   * therefore we catch NULL pointers it here */
  while(NULL == pt)
    ;
  return pt;
}

/*****************************************************************************/
/*! Memory freeing function
*   \param pvMem Memory block to free                                        */
/*****************************************************************************/
void OS_Memfree(void* pvMem)
{
  free(pvMem);
}

/*****************************************************************************/
/*! Memory reallocating function (used for resizing dynamic toolkit arrays)
*   \param pvMem     Memory block to resize
*   \param ulNewSize new size of the memory block
*   \return pointer to the resized memory block                              */
/*****************************************************************************/
void* OS_Memrealloc(void* pvMem, uint32_t ulNewSize)
{
  void* pt = realloc(pvMem, ulNewSize);

  /* Toolkit expect this function to work,
   * therefore we catch NULL pointers it here */
  while(NULL == pt);

  return pt;
}

/*****************************************************************************/
/*! Memory setting
*   \param pvMem     Memory block
*   \param bFill     Byte to use for memory initialization
*   \param ulSize    Memory size for initialization)                         */
/*****************************************************************************/
void OS_Memset(void* pvMem, unsigned char bFill, uint32_t ulSize)
{
  memset(pvMem, bFill, ulSize);
}

/*****************************************************************************/
/*! Copy memory from one block to another
*   \param pvDest    Destination memory block
*   \param pvSrc     Source memory block
*   \param ulSize    Copy size in bytes                                      */
/*****************************************************************************/
void OS_Memcpy(void* pvDest, void* pvSrc, uint32_t ulSize)
{
  memcpy(pvDest, pvSrc, ulSize);
}

/*****************************************************************************/
/*! Compare two memory blocks
*   \param pvBuf1    First memory block
*   \param pvBuf2    Second memory block
*   \param ulSize    Compare size in bytes
*   \return 0 if both buffers are equal                                      */
/*****************************************************************************/
int OS_Memcmp(void* pvBuf1, void* pvBuf2, uint32_t ulSize)
{
  return memcmp(pvBuf1, pvBuf2, ulSize);
}

/*****************************************************************************/
/*! Move memory
*   \param pvDest    Destination memory
*   \param pvSrc     Source memory
*   \param ulSize    Size in byte to move                                    */
/*****************************************************************************/
void OS_Memmove(void* pvDest, void* pvSrc, uint32_t ulSize)
{
  memmove(pvDest, pvSrc, ulSize);
}


/*****************************************************************************/
/*! Sleep for a specific time
*   \param ulSleepTimeMs  Time in ms to sleep for                            */
/*****************************************************************************/
void OS_Sleep(uint32_t ulSleepTimeMs)
{
  uint32_t ulStartTime = OS_GetMilliSecCounter();

  if(!s_fInitialized)
    return;

  if(ulSleepTimeMs != 0){
    while((OS_GetMilliSecCounter() - ulStartTime) < ulSleepTimeMs)
      /* NOP */;
  }
}

/*****************************************************************************/
/*! Retrieve a counter based on millisecond used for timeout monitoring
*   \return Current counter value (resolution of this value will influence
*           timeout monitoring in driver/toolkit functions(                  */
/*****************************************************************************/
uint32_t OS_GetMilliSecCounter(void)
{
  if(!s_fInitialized)
    return 0;

  uint32_t ulValue, ulNotUsed;

  while(DRV_OK != DRV_TIM_GetValue(&s_tSystick, &ulValue, &ulNotUsed))
    ;

  return ulValue;
}

/*****************************************************************************/
/*! Create an auto reset event
*   \return handle to the created event                                      */
/*****************************************************************************/
#define OSAL_MAX_EVENTS 64
typedef enum { EVENT_STATE_UNUSED, EVENT_STATE_CLEAR, EVENT_STATE_SET } OSAL_EVENT_STATE_E;
volatile static OSAL_EVENT_STATE_E s_eOsalEvents[OSAL_MAX_EVENTS + 1];

void* OS_CreateEvent(void)
{
  uint32_t ulIndex = OSAL_MAX_EVENTS;

  /* Search for an unused event */
  for(ulIndex = OSAL_MAX_EVENTS; ulIndex > 0; ulIndex--)
  {
    if(s_eOsalEvents[ulIndex] == EVENT_STATE_UNUSED)
    {
      s_eOsalEvents[ulIndex] = EVENT_STATE_CLEAR;
      break;
    }
  }
  return (void*) (ulIndex);
}

/*****************************************************************************/
/*! Set an event
*   \param pvEvent Handle to event being signalled                           */
/*****************************************************************************/
void OS_SetEvent(void* pvEvent)
{
  uint32_t ulIndex = (uint32_t) pvEvent;

  if(ulIndex <= OSAL_MAX_EVENTS)
  {
    if(s_eOsalEvents[ulIndex] == EVENT_STATE_CLEAR)
      s_eOsalEvents[ulIndex] = EVENT_STATE_SET;
  }
}

/*****************************************************************************/
/*! Reset an event
*   \param pvEvent Handle to event being reset                               */
/*****************************************************************************/
void OS_ResetEvent(void* pvEvent)
{
  uint32_t ulIndex = (uint32_t) pvEvent;

  if(ulIndex <= OSAL_MAX_EVENTS)
    s_eOsalEvents[ulIndex] = EVENT_STATE_CLEAR;
}

/*****************************************************************************/
/*! Delete an event
*   \param pvEvent Handle to event being deleted                             */
/*****************************************************************************/
void OS_DeleteEvent(void* pvEvent)
{
  uint32_t ulIndex = (uint32_t) pvEvent;

  if(ulIndex <= OSAL_MAX_EVENTS)
    s_eOsalEvents[ulIndex] = EVENT_STATE_UNUSED;
}

/*****************************************************************************/
/*! Wait for the signalling of an event
*   \param pvEvent   Handle to event being wait for
*   \param ulTimeout Timeout in ms to wait for event
*   \return 0 if event was signalled                                         */
/*****************************************************************************/
uint32_t OS_WaitEvent(void* pvEvent, uint32_t ulTimeout)
{
  uint32_t ulResult = CIFX_NO_ERROR;
  uint32_t ulIndex = (uint32_t) pvEvent;
  uint32_t ulStartTime = 0;
  uint32_t ulCurrentTime = 0;

  if(ulIndex > OSAL_MAX_EVENTS)
    ulResult = CIFX_INVALID_PARAMETER;
  else
  {
    ulStartTime = OS_GetMilliSecCounter();
    while(1)
    {
      if(s_eOsalEvents[ulIndex] == EVENT_STATE_SET)
      {
        s_eOsalEvents[ulIndex] = EVENT_STATE_CLEAR;
        break;
      }
      if(s_eOsalEvents[ulIndex] < EVENT_STATE_CLEAR || s_eOsalEvents[ulIndex] > EVENT_STATE_SET)
      {
        ulResult = CIFX_INVALID_PARAMETER;
        break;
      }
      ulCurrentTime = OS_GetMilliSecCounter();
      if((ulCurrentTime - ulStartTime) >= ulTimeout)
      {
        ulResult = CIFX_EVENT_TIMEOUT;
        break;
      }
    }
  }
  return (ulResult);
}

/*****************************************************************************/
/*! Compare two ASCII string
*   \param pszBuf1   First buffer
*   \param pszBuf2   Second buffer
*   \return 0 if strings are equal                                           */
/*****************************************************************************/
int OS_Strcmp(const char* pszBuf1, const char* pszBuf2)
{
  return strcmp(pszBuf1, pszBuf2);
}

/*****************************************************************************/
/*! Compare characters of two strings without regard to case
*   \param pszBuf1   First buffer
*   \param pszBuf2   Second buffer
*   \param ulLen     Number of characters to compare
*   \return 0 if strings are equal                                           */
/*****************************************************************************/
int OS_Strnicmp(const char* pszBuf1, const char* pszBuf2, uint32_t ulLen)
{
  return strncasecmp(pszBuf1, pszBuf2, ulLen);
}

/*****************************************************************************/
/*! Query the length of an ASCII string
*   \param szText    ASCII string
*   \return character count of szText                                        */
/*****************************************************************************/
int OS_Strlen(const char* szText)
{
  return strlen(szText);
}

/*****************************************************************************/
/*! Copies one string to another monitoring the maximum length of the target
*   buffer.
*   \param szDest    Destination buffer
*   \param szSource  Source buffer
*   \param ulLen     Maximum length to copy
*   \return pointer to szDest                                                */
/*****************************************************************************/
char* OS_Strncpy(char* szDest, const char* szSource, uint32_t ulLen)
{
  return strncpy(szDest, szSource, ulLen);
}

/*****************************************************************************/
/*! Create an interrupt safe locking mechanism (Spinlock/critical section)
*   \return handle to the locking object                                     */
/*****************************************************************************/
void* OS_CreateLock(void)
{
  static uint32_t s_aulPrimask = 0;
  return &s_aulPrimask;
}

/*****************************************************************************/
/*! Enter a critical section/spinlock
*   \param pvLock Handle to the locking object                               */
/*****************************************************************************/
void OS_EnterLock(void* pvLock)
{
  uint32_t s_aulPrimask = __get_PRIMASK();

  /* Store PRIMASK value first on Stack, because right here where this comment
   * is written, a irq could happen and this function is entered again... */

  __disable_irq();
  *(uint32_t*)pvLock = s_aulPrimask;
}

/*****************************************************************************/
/*! Leave a critical section/spinlock
*   \param pvLock Handle to the locking object                               */
/*****************************************************************************/
void OS_LeaveLock(void* pvLock)
{
  if (!*(uint32_t*)pvLock){
    __enable_irq();
  }
}

/*****************************************************************************/
/*! Delete a critical section/spinlock object
*   \param pvLock Handle to the locking object being deleted                 */
/*****************************************************************************/
void OS_DeleteLock(void* pvLock)
{
}

/*****************************************************************************/
/*! Create an Mutex object for locking code sections
*   \return handle to the mutex object                                       */
/*****************************************************************************/
void* OS_CreateMutex(void)
{
  return (void*)0xDEADDA7A;
}

/*****************************************************************************/
/*! Wait for mutex
*   \param pvMutex    Handle to the Mutex locking object
*   \param ulTimeout  Wait timeout
*   \return !=0 on succes                                                    */
/*****************************************************************************/
int OS_WaitMutex(void* pvMutex, uint32_t ulTimeout)
{
  return 1;
}

/*****************************************************************************/
/*! Release a mutex section section
*   \param pvMutex Handle to the locking object                              */
/*****************************************************************************/
void OS_ReleaseMutex(void* pvMutex)
{
}

/*****************************************************************************/
/*! Delete a Mutex object
*   \param pvMutex Handle to the mutex object being deleted                  */
/*****************************************************************************/
void OS_DeleteMutex(void* pvMutex)
{
}

/*****************************************************************************/
/*! Opens a file in binary mode
*   \param szFile     Full file name (incl. path if necessary) of the file to open
*   \param pulFileLen Returned length of the opened file
*   \return handle to the file, NULL mean file could not be opened           */
/*****************************************************************************/
void* OS_FileOpen(char* szFile, uint32_t* pulFileLen)
{
  return NULL;
}

/*****************************************************************************/
/*! Closes a previously opened file
*   \param pvFile Handle to the file being closed                            */
/*****************************************************************************/
void OS_FileClose(void* pvFile)
{
}

/*****************************************************************************/
/*! Read a specific amount of bytes from the file
*   \param pvFile   Handle to the file being read from
*   \param ulOffset Offset inside the file, where read starts at
*   \param ulSize   Size in bytes to be read
*   \param pvBuffer Buffer to place read bytes in
*   \return number of bytes actually read from file                          */
/*****************************************************************************/
uint32_t OS_FileRead(void* pvFile, uint32_t ulOffset, uint32_t ulSize, void* pvBuffer)
{
  return 0;
}

/*****************************************************************************/
/*! OS specific initialization (if needed), called during cifXTKitInit()
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t OS_Init(void)
{
  if(!s_fInitialized)
  {
    /* Initialize 1ms systick timer */
    s_tSystick.tConfiguration.eDeviceID      = DRV_TIM_DEVICE_ID_SYSTICK;
    s_tSystick.tConfiguration.eOperationMode = DRV_OPERATION_MODE_IRQ;
    s_tSystick.tConfiguration.eCountingMode  = DRV_TIM_COUNTING_MODE_CONTINUOUS;
    s_tSystick.tConfiguration.tPreloadValue  = DRV_TIM_PRELOAD_VALUE_1ms;

    DRV_TIM_Init(&s_tSystick);

    while(DRV_OK != DRV_TIM_Start(&s_tSystick))
      ;

    s_fInitialized = 1;
  }

  return CIFX_NO_ERROR;
}

/*****************************************************************************/
/*! OS specific de-initialization (if needed), called during cifXTKitInit()  */
/*****************************************************************************/
void OS_Deinit(void)
{
  if(s_fInitialized)
  {
    DRV_TIM_DeInit(&s_tSystick);
    s_fInitialized = 0;
  }
}

/*****************************************************************************/
/*! This functions is called for PCI cards in the toolkit. It is expected to
* write back all BAR's (Base address registers), Interrupt and Command
* Register. These registers are invalidated during cifX Reset and need to be
* re-written after the reset has succeeded
*   \param pvOSDependent OS Dependent Variable passed during call to
*                        cifXTKitAddDevice
*   \param pvPCIConfig   Configuration returned by OS_ReadPCIConfig
*                        (implementation dependent)                          */
/*****************************************************************************/
void OS_WritePCIConfig(void* pvOSDependent, void* pvPCIConfig)
{
  /* Implement PCI register accesss, needed for cifX cards, */
}

/*****************************************************************************/
/*! This functions is called for PCI cards in the toolkit. It is expected to
* read all BAR's (Base address registers), Interrupt and Command Register.
* These registers are invalidated during cifX Reset and need to be
* re-written after the reset has succeeded
*   \param pvOSDependent OS Dependent Variable passed during call to
*                        cifXTKitAddDevice
*   \return pointer to stored register copies (implementation dependent)     */
/*****************************************************************************/
void* OS_ReadPCIConfig(void* pvOSDependent)
{
  /* Implement PCI register access, needed for cifX cards. */
  return NULL;
}

/*****************************************************************************/
/*! This function Maps a DPM pointer to a user application if needed.
*   This example just returns the pointer valid inside the driver.
*   \param pvDriverMem   Pointer to be mapped
*   \param ulMemSize     Size to be mapped
*   \param ppvMappedMem  Returned mapped pointer (usable by application)
*   \param pvOSDependent OS Dependent variable passed during call to
*                        cifXTKitAddDevice
*   \param fCached       Caching option (0 = do not use caching)
*   \return Handle that is needed for unmapping NULL is a mapping failure    */
/*****************************************************************************/
void* OS_MapUserPointer(void* pvDriverMem, uint32_t ulMemSize, void** ppvMappedMem, void* pvOSDependent, unsigned char fCached)
{
  /* We are running in user mode, so it is not necessary to map anything to user space */
  *ppvMappedMem = pvDriverMem;

  return pvDriverMem;
}

/*****************************************************************************/
/*! This function unmaps a previously mapped user application pointer
*   \param phMapping      Handle that was returned by OS_MapUserPointer
*   \param pvOSDependent  OS Dependent variable passed during call to
*                         cifXTKitAddDevice
*   \return !=0 on success                                                   */
/*****************************************************************************/
int OS_UnmapUserPointer(void* phMapping, void* pvOSDependent)
{
  /* We are running in user mode, so it is not necessary to map anything to user space */
  return 1;
}

/*****************************************************************************/
/*! This function flushes a cached memory area to the device buffer
*   \param pvMem      Pointer to the cached memory
*   \param ulMemSize  Length of the cached memory area                   */
/*****************************************************************************/
void OS_FlushCacheMemory_ToDevice(void* pvMem, unsigned long ulMemSize)
{
}

/*****************************************************************************/
/*! This function invalidates a cache buffer to be refreshed by
*   the physical memory.
*   \param pvMem      Pointer to the cached memory
*   \param ulMemSize  Length of the cached memory area                       */
/*****************************************************************************/
void OS_InvalidateCacheMemory_FromDevice(void* pvMem, unsigned long ulMemSize)
{
}

/*****************************************************************************/
/*! This function enables the interrupts for the device physically
*   \param pvOSDependent OS Dependent Variable passed during call to
*                        cifXTKitAddDevice                                   */
/*****************************************************************************/
void OS_EnableInterrupts(void* pvOSDependent)
{
  DEVICEINSTANCE*  ptDevInst = pvOSDependent;
  s_ptNetX90_IrqDevInst = ptDevInst;

  NVIC_SetPriority(idpm_com_host_IRQn, NETX90_IDPM_IRQ_PRIORITY);
  NVIC_EnableIRQ(idpm_com_host_IRQn);
}

/*****************************************************************************/
/*! This function disables the interrupts for the device physically
*   \param pvOSDependent OS Dependent Variable passed during call to
*                        cifXTKitAddDevice                                   */
/*****************************************************************************/
void OS_DisableInterrupts(void* pvOSDependent)
{
}

#ifdef CIFX_TOOLKIT_ENABLE_DSR_LOCK
/*****************************************************************************/
/*! Lock DSR against IST
*   \param pvOSDependent  Device Extension                                   */
/*****************************************************************************/
void OS_IrqLock(void* pvOSDependent)
{
}

/*****************************************************************************/
/*! Unlock DSR against IST
*   \param pvOSDependent  Device Extension                                   */
/*****************************************************************************/
void OS_IrqUnlock(void* pvOSDependent)
{
}
#endif

#ifdef CIFX_TOOLKIT_TIME
/*****************************************************************************/
/*! Get System time
*   \param ptTime   Pointer to store the time value
*   \return actual time value in seconds sincd 01.01.1970                    */
/*****************************************************************************/
uint64_t OS_Time( uint64_t *ptTime)
{
  if (NULL != ptTime)
    *ptTime = 0;

  return 0;
}
#endif

/*****************************************************************************/
/*! DPM Interrupt handler for netX90-APP
 *  Enables the usage of cifXToolkit in interrupt mode, essential for the
 *  implementation of the DPM Sync functionality                             */
/*****************************************************************************/
void
IDPM_IRQHandler(void)
{
  int iResult = 0;

  iResult = cifXTKitISRHandler(s_ptNetX90_IrqDevInst,
                               0);

  if(CIFX_TKIT_IRQ_DSR_REQUESTED == iResult)
  {
    cifXTKitDSRHandler(s_ptNetX90_IrqDevInst);
  }
}

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/

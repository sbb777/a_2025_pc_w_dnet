/*!************************************************************************//*!
 * \file    netx_drv.c
 * \brief   peripheral module driver.
 *          This is the common part of the DRV initialization
 * $Revision: 6124 $
 * $Date: 2019-08-28 20:41:54 +0300 (Wed, 28 Aug 2019) $
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

/* Includes ------------------------------------------------------------------*/
#include "netx_drv.h"
#include "stdio.h"

/*!
 * \addtogroup trace
 * \{
 */
/*!
 * The following part is used for debugging purposes.
 */
#ifndef NDEBUG

/*!
 * This is the counter that points to the next queue element
 */
uint32_t volatile g_ulQueueCounter = 0;

/*!
 * This is the flag if the trace is allowed to run or not.
 */
uint32_t volatile g_ulRunFlag = 0;

/*!
 * This is the queue used for the debug trace functionality
 */
volatile DRV_TRACE_QUEUE_ELEMENT_T g_atTraceQueue[ulTraceQueueSize];

/*lint -save -e606 */

/*!
 * Array, containing the strings of the associated keys
 */
char const * const g_pabDrvTraceKeySrings[] = { "", "\x1B[35mDATA\x1B[0m", "\x1B[34mMODE\x1B[0m", "\x1B[32mBORDER\x1B[0m", "\x1B[32mCOUNTER\x1B[0m",
  "\x1B[32mITERATION\x1B[0m", "\x1B[32mLENGTH\x1B[0m", "\x1B[31mIRQ\x1B[0m", "\x1B[33mMSK\x1B[0m", "\x1B[36mSTATE\x1B[0m", "\x1B[36mINFO\x1B[0m",
  "\x1B[0m11\x1B[0m", "\x1B[0m12\x1B[0m", "\x1B[0m13\x1B[0m", "\x1B[32mCOUNT\x1B[0m" };
/*lint -restore */

size_t u_DRV_TRACE_KEY_STRINGS;

/*!
 * This is the debug trace functionality that is capable of logging a
 * given key and value pair with minimal trade of.
 */
void TRACE(uint32_t ulKey, uint32_t ulValue)
{
  if(!g_ulRunFlag)
  {
    g_atTraceQueue[g_ulQueueCounter].key = (DRV_TRACE_KEY_E) ulKey;
    g_atTraceQueue[g_ulQueueCounter].val = ulValue;
    g_ulQueueCounter++;
    if(g_ulQueueCounter == ulTraceQueueSize)
    {
      g_ulQueueCounter = 0;
    }
  }
}

/*!
 * Function, that resets the ringbuffer of the trace to default values.
 */
void TRACE_RESET(void)
{
  g_ulQueueCounter = 0;
  uint32_t i;
  for(i = 0; i < ulTraceQueueSize; i++)
  {
    g_atTraceQueue[i].key = (DRV_TRACE_KEY_E) 0;
    g_atTraceQueue[i].val = 0;
  }
  UNUSED(g_atTraceQueue);
}

#ifdef DRV_TRACE_PRINTER
/*!
 * Function, that resets the ringbuffer of the trace to default values.
 */
void TRACE_PRINT(void)
{
  TRACE_PAUSE();
  size_t i;
  for(i = g_ulQueueCounter; i < ulTraceQueueSize; i++)
  {
    printf("%3d: %#010lX %s\n", i, g_atTraceQueue[i].val, g_pabDrvTraceKeySrings[g_atTraceQueue[i].key]);
  }
  for(i = 0; i < g_ulQueueCounter; i++)
  {
    printf("%3d: %#010lX %4s\n", i, g_atTraceQueue[i].val, g_pabDrvTraceKeySrings[g_atTraceQueue[i].key]);
  }
  TRACE_RUN();
}
#endif /* DRV_TRACE_PRINTER */

#endif /* NDEBUG */
/*! \} *//* End of group trace */


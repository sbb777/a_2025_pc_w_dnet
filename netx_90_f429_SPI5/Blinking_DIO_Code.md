```c
/***************************************************************************************/
/*! Cyclic IO data handler: fast data exchange with netX via DPM called from a timer ISR
 *
 * \param ptAppData  [in]  Pointer to application data
 */
/***************************************************************************************/
void App_IODataHandler(void* ptAppResources)
{
  long            lRet      = CIFX_NO_ERROR; /** Return value for common error codes  */
  APP_DATA_T*     ptAppData = (APP_DATA_T*)ptAppResources;

  // Timers for blinking
  static uint32_t s_ulBlinkTimer_500ms = 0;
  static uint32_t s_ulBlinkTimer_2000ms = 0;
  uint32_t ulCurrentTime = OS_GetMilliSecCounter();

  if(ptAppData->aptChannels[0]->hChannel != NULL)
  {
    /** INPUT DATA ***********************************************************************/
    lRet = xChannelIORead(ptAppData->aptChannels[0]->hChannel, 0, 0, sizeof(ptAppData->tInputData), &ptAppData->tInputData, 0);
    if(lRet != CIFX_NO_ERROR)
    {
      /** Something failed?
       * Reason for error could be:
       * 1) netX is not "ready" yet. May happen during startup.
       * 2) netX is not "running" yet. May happen during startup in case the netX is not fully configured yet.
       * 3) netX has not yet established an IO connection. */

      ptAppData->fInputDataValid = false;
    }
    else
    {
      /** process newly received input data image */
      ptAppData->fInputDataValid = true;

    }

    /** OUTPUT DATA ***************************************/
    /** update output data image to be sent in this cycle */
    // Blinking logic for 0.5 seconds (500ms) on bit 0
    if((ulCurrentTime - s_ulBlinkTimer_500ms) >= 500)
    {
      s_ulBlinkTimer_500ms = ulCurrentTime;
      ptAppData->tOutputData.output[0] ^= 0x01; // Toggle bit 0
    }

    // Blinking logic for 2 seconds (2000ms) on bit 1
    if((ulCurrentTime - s_ulBlinkTimer_2000ms) >= 2000)
    {
      s_ulBlinkTimer_2000ms = ulCurrentTime;
      ptAppData->tOutputData.output[0] ^= 0x02; // Toggle bit 1
    }

    lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel, 0, 0, sizeof(ptAppData->tOutputData), &ptAppData->tOutputData, 0);
    if(lRet != CIFX_NO_ERROR)
    {
      /** Something failed?
       * Reason for error could be:
       * 1) netX is not "ready" yet. May happen during startup.
       * 2) netX is not "running" yet. May happen during startup in case the netX is not fully configured yet.
       * 3) netX has not yet established an IO connection. */
    }
  }
}
```
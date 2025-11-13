/**************************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

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

**************************************************************************************************/
#ifndef APP_DEMOAPPLICATION_H_
#define APP_DEMOAPPLICATION_H_

#include "App_Config.h"
#include "App_CommChannel.h"
#include "cifXUser.h"


/** A communication channel struct.
 * This struct holds data related to one channel handler.
 */
typedef struct
{
  APP_COMM_CHANNEL_HANDLER_T      tCommChannelHandler;  /** This is the communication channel handler for the cifX communication channel */
  APP_COMM_CHANNEL_HANDLER_RSC_H  hCommChannelRsc;      /** Resource handle for the communication channel handler  */

  CIFXHANDLE            hCifXChannel;        /** Handle of netX DPM communication channel                */
  CHANNEL_INFORMATION   tCifXChannelInfo;    /** DPM channel information. Read during application start. */
} APP_COMM_CHANNEL_T;

typedef struct
{
  CIFXHANDLE          hCifXDriver;      /** Handle of cifX driver                                   */
  BOARD_INFORMATION   tCifXBoardInfo;   /** netX Board information. Read during application start.  */

  /* Data of the communication channels */
  APP_COMM_CHANNEL_T  atCommChannels[USED_COMMUNICATION_CHANNELS];

} APP_DATA_T;


/*! Entry point of cifX Application Demo.
 *
 * This is the main entry point of the demo application.
 * This function internally ends itself in an  "main demo process loop".
 *
 * On an multiprocess system, the priority of this process (task) must
 * be lower than the App_ApplicationTimerJob function calls.
 *
 * \param szBoardName Name of the cifX device, e.g. "cifX0", where the
 *                    application should run.
 *
 * \return never returns, except on an unrecoverable faults.
 */
int App_ApplicationDemo(char *szBoardName);


/*! Cyclic IO data handler
 *
 * This cyclic event is used for calling all Timer tasks of
 * the communication channel. This function must be called
 * once per millisecond.
 *
 */
void App_ApplicationTimerJob();


/* Prototypes of functions implemented in App_DemoApplicationFunctions.c module */

void     App_PrintBoards(const CIFXHANDLE hDriver);

int32_t  App_InitializeChannels(APP_DATA_T* ptAppData, char* szBoardName);
int32_t  App_ConfigureChannels(APP_DATA_T* ptAppData);
void     App_CallRoutineTasks(APP_DATA_T* ptAppData);

void App_AllChannels_Close(APP_DATA_T* ptAppData);


#endif /** APP_DEMOAPPLICATION_H_ */

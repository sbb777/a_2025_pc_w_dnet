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
#ifndef APP_COMMCHANNEL_H_
#define APP_COMMCHANNEL_H_

#include "App_Config.h"
#include "cifXUser.h"


/*! Handle to the protocol specific resources. */
typedef struct APP_COMM_CHANNEL_HANDLER_RSC_Ttag* APP_COMM_CHANNEL_HANDLER_RSC_H;

/* Initialization of communication channel
 *
 * The Initialization function will be called only once. Within this function internal resources can
 * be allocated and initialized. Those resources can be later references via the phCommChHdlRsc variable,
 * which will be passed in all other communication channel handler functions.
 *
 * This function can be used to initialize the netX communication LFW  e.g. Set DDP data (MAC,
 * serial number, OEM data, etc.), set remanent data or setting up security related configuration
 *
 * If the communication channel handler want to use e.g. xChannelRegisterNotification functions,
 * this is also a good pace to put this call here.
 *
 * \param phCommChHdlRsc     [out] Reference to the internal resources
 * \param hCifXChannel       [in]  Handle to the related cifXChannel
 * \param ptCifXChannelInfo  [in]  Channel information structure
 */
typedef int (*App_InitializeCallback)(APP_COMM_CHANNEL_HANDLER_RSC_H *phCommChHdlRsc, CIFXHANDLE hCifXChannel, CHANNEL_INFORMATION *ptCifXChannelInfo);

/* Setup communication channel
 *
 * The setup callback is called in order to send a configuration to the protocol stack or component.
 *
 * After the Bus on Signal is switched to "ON" the application needs to be able to generate IO-Data
 * (e.g. xChannelIOWrite) for the communication stack (otherwise some communication stacks will not
 * go "online".
 *
 * \param phCommChHdlRsc     [in] Reference to the internal resources
 */
typedef int (*App_SetupCallback)(APP_COMM_CHANNEL_HANDLER_RSC_H hCommChHdlRsc);

/* Routine task
 *
 * This callback will be called in a regular basis within the "main loop". This requires that this
 * function has to be "Cooperative" and return when nothing is to do. In addition this function, should
 * not block, because it will all other routine tasks to.
 *
 * \param phCommChHdlRsc     [in] Reference to the internal resources
 */
typedef void (*App_RoutineTaskCallback)(APP_COMM_CHANNEL_HANDLER_RSC_H hCommChHdlRsc);

/* Timer task
 *
 * This callback will be called from a Timer once every millisecond. This can be used to e.g. to
 * exchange IO Data with the communication channel via xChannelIOWrite/xChannelIORead.
 * Note that if the function (all channel hander timer task function combined) needs more than 1ms
 * time, timer ticks may get lost!
 *
 * \note It's not allowed to block within this function!
 *
 * \param phCommChHdlRsc     [in] Reference to the internal resources
 */
typedef void (*App_TimerTaskCallback)(APP_COMM_CHANNEL_HANDLER_RSC_H hCommChHdlRsc);

/** Defines the Interface functions which the protocol entry points for the protocol specific functions */
typedef struct
{
  App_InitializeCallback    pfnInitialize;
  App_SetupCallback         pfnSetup;
  App_RoutineTaskCallback   pfnRoutineTask;
  App_TimerTaskCallback     pfnTimerTask;
} APP_COMM_CHANNEL_HANDLER_T;

#endif /** APP_COMMCHANNEL_H_ */

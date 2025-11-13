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

***************************************************************************************/

#ifndef COMPONENTS_CIFXAPPLICATIONDEMO_INCLUDES_APP_DEMOAPPLICATION_H_
#define COMPONENTS_CIFXAPPLICATIONDEMO_INCLUDES_APP_DEMOAPPLICATION_H_

/*****************************************************************************/
/*! General Inclusion Area                                                   */
/*****************************************************************************/

#include <stdio.h>                  /** Include C standard library input/output header */
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "Hil_Compiler.h"
#include "cifXErrors.h"             /** Include cifX driver API error definition */
#include "cifXUser.h"               /** Include cifX driver API definition       */
#include "Hil_DualPortMemory.h"
//#include "hostAbstractionLayer.h"
//#include "App_EventHandler.h"


/*****************************************************************************/
/*! General Definition Area                                                   */
/*****************************************************************************/

#define MAX_COMMUNICATION_CHANNEL_COUNT   3

#define DEMO_QUIET

#ifndef DEMO_QUIET
#define PRINTF printf
#else
#define PRINTF(...)
#endif

/*****************************************************************************/
/*! FORWARD DECLARATIONS                                                     */
/*****************************************************************************/

enum   APP_EVENT_Etag;
struct APP_DATA_Ttag;

/*****************************************************************************/
/*! PROCESS DATA - VAT Adaptive Pressure Controller                         */
/*****************************************************************************/

/* Master -> Slave (Consumed): Output Assembly 8 - 5 bytes */
/* Control Mode (1 byte), Control Setpoint (2 bytes), Control Instance (1 byte), Reserved (1 byte) */
typedef __HIL_PACKED_PRE struct APP_PROCESS_DATA_INPUT_Ttag /* Consumed data (Master -> Slave) */
{
  uint8_t input[5];  /* VAT Output Assembly 8: 5 bytes */
} __HIL_PACKED_POST APP_PROCESS_DATA_INPUT_T;

/* Slave -> Master (Produced): Input Assembly 100 - 7 bytes */
/* Exception Status (1 byte), Pressure (2 bytes), Position (2 bytes), Device Status (1 byte), Access Mode (1 byte) */
typedef __HIL_PACKED_PRE struct APP_PROCESS_DATA_OUTPUT_Ttag /* Produced data (Slave -> Master) */
{
  uint8_t output[7];  /* VAT Input Assembly 100: 7 bytes */
} __HIL_PACKED_POST APP_PROCESS_DATA_OUTPUT_T;

/*****************************************************************************/
/*! ACYCLIC DATA                                                             */
/*****************************************************************************/

typedef __HIL_PACKED_PRE struct APP_ACYCLIC_DATA_Ttag
{
  /* Acyclic data of sensors */

} __HIL_PACKED_POST APP_ACYCLIC_DATA_T;

/*****************************************************************************/
/*! APPLICATION DEMO RESOURCES                                               */
/*****************************************************************************/

/** Defines entry points for the protocol specific functions */
typedef struct PROTOCOL_DESCRIPTOR_Ttag
{
  uint32_t (*pfStartChannelConfiguration)(struct APP_DATA_Ttag*);                             /* configure the channel     */
  uint32_t (*pfPacketHandler)(struct APP_DATA_Ttag*);                                         /* handle packets            */
} PROTOCOL_DESCRIPTOR_T;

/** A communication channel contains a channel handle, a packet,
    and the protocol descriptor to be associated by the application
*/
typedef struct APP_COMM_CHANNEL_Ttag
{
  CIFX_PACKET             tPacket;
  CIFXHANDLE              hChannel;         /** Handle of netX DPM communication channel                */
  CHANNEL_INFORMATION     tChannelInfo;     /** DPM channel information. Read during application start. */
  PROTOCOL_DESCRIPTOR_T   tProtocol;
} APP_COMM_CHANNEL_T;

typedef struct APP_DATA_Ttag
{
  APP_COMM_CHANNEL_T       *aptChannels[MAX_COMMUNICATION_CHANNEL_COUNT];
  BOARD_INFORMATION         tBoardInfo;       /** netX Board information. Read during application start.  */

  bool                      fRunning;
  bool                      fInputDataValid;

  APP_PROCESS_DATA_INPUT_T  tInputData;       /** Consumed process data. Data that is received from the PLC. */
  APP_PROCESS_DATA_OUTPUT_T tOutputData;      /** Produced process data. Data that is sent to the PLC.       */
  APP_ACYCLIC_DATA_T        tAcyclicData;     /** Application data available via acyclic accesses.           */

} APP_DATA_T;


/*****************************************************************************/
/*! FUNCTION PROTOTYPES                                                      */
/*****************************************************************************/

void App_IODataHandler(void* ptAppResources);
void App_IO_UpdateCycleCounter(APP_DATA_T* ptAppData);

int App_CifXApplicationDemo(char* szDeviceName);
void App_Selftest(void);
void App_ReadBoardInfo(const CIFXHANDLE hDriver, BOARD_INFORMATION* const ptBoardInfo);
void App_ReadChannelInfo(const CIFXHANDLE hChannel, CHANNEL_INFORMATION* const ptChannelInfo);

int32_t  App_AllChannels_Open(APP_DATA_T* ptAppData, CIFXHANDLE hDriver, char* szDeviceName);
uint32_t App_AllChannels_Configure(APP_DATA_T* ptAppData);
uint32_t App_AllChannels_PacketHandler(APP_DATA_T* ptAppData);

void App_AllChannels_EventHandler(APP_DATA_T* ptAppData, enum APP_EVENT_Etag eEvent, uint32_t ulEventData);
void App_AllChannels_TerminalHandler(APP_DATA_T* ptAppData, char* szTerminalBuffer);
void App_AllChannels_WaitReady(APP_DATA_T* ptAppData);
void App_AllChannels_GetChannelInfo_WaitReady(APP_DATA_T* ptAppData);
void App_AllChannels_Close(APP_DATA_T* ptAppData);

typedef struct PROTOCOL_IDENTIFICATION_Ttag
{
  const char* szFirmwareName;
  const uint16_t usFirmwareMajor;
  const uint16_t usFirmwareMinor;
} PROTOCOL_IDENTIFICATION_T;



extern PROTOCOL_IDENTIFICATION_T g_tProtocolIdentification;

#endif /** COMPONENTS_CIFXAPPLICATIONDEMO_INCLUDES_APP_DEMOAPPLICATION_H_ */

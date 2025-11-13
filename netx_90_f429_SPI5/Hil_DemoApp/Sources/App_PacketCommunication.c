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

/*****************************************************************************/
/*! General Inclusion Area                                                   */
/*****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <assert.h>

#include "App_DemoApplication.h"
#include "App_PacketCommunication.h"
#include "Hil_Packet.h"

/*****************************************************************************/
/*! STRUCTURES                                                               */
/*****************************************************************************/

typedef struct PACKET_COMMUNICATION_COUNTER_Ttag
{
  uint32_t ulPacketSendCountSuccess;
  uint32_t ulPacketSendCountError;
  uint32_t ulPacketReceiveCount;

} PACKET_COMMUNICATION_COUNTER_T;

/******************************************************************************/

/** Structure to store application's indication handler. */
typedef struct SYNC_PACKET_INTERFACE_INDICATON_HANDLER_Ttag
{
  bool (*fnHandler)(CIFX_PACKET* ptPacket, void* pvUserData);
  void* pvUserData;
} SYNC_PACKET_INTERFACE_INDICATON_HANDLER_T;

/******************************************************************************/

typedef struct SYNC_PACKET_INTERFACE_QUEUED_PACKET_Ttag
{
  bool fUsed;
  CIFX_PACKET tPacket;
} SYNC_PACKET_INTERFACE_QUEUED_PACKET_T;


/*****************************************************************************/
/*! GLOBAL, STATIC OR EXTERN VARIABLES                                       */
/*****************************************************************************/

/** Static packet ID counter.
 * This counter is used to assign received confirmation
 * packets uniquely to a previously sent request packet. */
static uint32_t s_ulNextPacketId = 1;

/** The LookupCommandCode function is used to print readable text instead of just the command ID. */
extern char* LookupCommandCode(uint32_t ulCmd);

/** Statistic counter for sent/received packets */
static PACKET_COMMUNICATION_COUNTER_T s_tPacketCounter;

/** Storage for Application's indication handler */
static SYNC_PACKET_INTERFACE_INDICATON_HANDLER_T s_atIndicationHandler[MAX_COMMUNICATION_CHANNEL_COUNT];

/** Receive queue for confirmation packets that are not expected when sending a request via Pkt_SendReceivePacket() */
static SYNC_PACKET_INTERFACE_QUEUED_PACKET_T** s_aaptReceiveQueue[MAX_COMMUNICATION_CHANNEL_COUNT];

static int s_aiReceiveQueueDepths[MAX_COMMUNICATION_CHANNEL_COUNT];

/*****************************************************************************/
/*! FUNCTIONS                                                                */
/*****************************************************************************/

bool Pkt_Init(int iChannelIdx, int iReceiveQueueDepth)
{
  int i;

  if (iChannelIdx >= MAX_COMMUNICATION_CHANNEL_COUNT)
  {
    return false;
  }

  if (s_aaptReceiveQueue[iChannelIdx] != NULL)
  {
    return false;
  }

  s_aaptReceiveQueue[iChannelIdx] = (SYNC_PACKET_INTERFACE_QUEUED_PACKET_T**)calloc( iReceiveQueueDepth, sizeof(SYNC_PACKET_INTERFACE_QUEUED_PACKET_T*));

  if (NULL == s_aaptReceiveQueue[iChannelIdx])
  {
    return false;
  }

  for (i = 0; i < iReceiveQueueDepth; i++)
  {
    s_aaptReceiveQueue[iChannelIdx][i] = (SYNC_PACKET_INTERFACE_QUEUED_PACKET_T*)calloc( 1, sizeof(SYNC_PACKET_INTERFACE_QUEUED_PACKET_T) );

    if (NULL == s_aaptReceiveQueue[iChannelIdx][i])
    {
      return false;
    }
  }

  s_aiReceiveQueueDepths[iChannelIdx] = iReceiveQueueDepth;

  return true;
}

/*****************************************************************************/
void Pkt_Deinit(void)
{
  int iChannelIdx;

  for( iChannelIdx = 0; iChannelIdx < MAX_COMMUNICATION_CHANNEL_COUNT; iChannelIdx++ )
  {
    if( s_aaptReceiveQueue[iChannelIdx] != NULL)
    {
      int iCnfQueueIdx;

      for (iCnfQueueIdx = 0; iCnfQueueIdx < s_aiReceiveQueueDepths[iChannelIdx]; iCnfQueueIdx++)
      {
        if (s_aaptReceiveQueue[iChannelIdx][iCnfQueueIdx] != NULL)
        {
          free( s_aaptReceiveQueue[iChannelIdx][iCnfQueueIdx] );
        }
      }

      free( s_aaptReceiveQueue[iChannelIdx] );
    }
  }

  memset( s_aiReceiveQueueDepths, 0, sizeof(s_aiReceiveQueueDepths) );
  memset( s_aaptReceiveQueue,     0, sizeof(s_aaptReceiveQueue) );
}

/*****************************************************************************/
/*! Displays a hex dump on the debug console (16 bytes per line)
 *
 *   \param pbData    [in]  Data buffer that shall be dumped
 *   \param ulDataLen [in]  Number of bytes in pbData
 */
/*****************************************************************************/
static void
Pkt_PrintFormattedHexData( unsigned char* pbData,
                           unsigned long  ulDataLen )
{
  unsigned long ulIdx;

  if(CIFX_MAX_DATA_SIZE < ulDataLen)
  {
    ulDataLen = CIFX_MAX_DATA_SIZE;
  }

  for(ulIdx = 0; ulIdx < ulDataLen; ++ulIdx)
  {
    if(0 == (ulIdx % 16))
    {
      PRINTF(NEWLINE);
    }

    PRINTF("%02X ", pbData[ulIdx]);
  }

  PRINTF(NEWLINE);
}
/*****************************************************************************/
/*! Dumps a packet to the debug console
 *
 *   \param ptPacket  [in] Packet to be dumped
 */
/*****************************************************************************/
static void
Pkt_DumpPacket( CIFX_PACKET* ptPacket )
{
  PRINTF("Dest   : 0x%08X      ID   : 0x%08X" NEWLINE, (unsigned int) ptPacket->tHeader.ulDest, (unsigned int) ptPacket->tHeader.ulId);
  PRINTF("Src    : 0x%08X      Sta  : 0x%08X" NEWLINE, (unsigned int) ptPacket->tHeader.ulSrc, (unsigned int) ptPacket->tHeader.ulState);
  PRINTF("DestID : 0x%08X      Cmd  : 0x%08X" NEWLINE, (unsigned int) ptPacket->tHeader.ulDestId, (unsigned int) ptPacket->tHeader.ulCmd);
  PRINTF("SrcID  : 0x%08X      Ext  : 0x%08X" NEWLINE, (unsigned int) ptPacket->tHeader.ulSrcId, (unsigned int) ptPacket->tHeader.ulExt);
  PRINTF("Len    : 0x%08X      Rout : 0x%08X" NEWLINE, (unsigned int) ptPacket->tHeader.ulLen, (unsigned int) ptPacket->tHeader.ulRout);

  if(ptPacket->tHeader.ulLen)
  {
    PRINTF("Data:");

    /** Displays a hex dump on the debug console (16 bytes per line) */
    Pkt_PrintFormattedHexData(ptPacket->abData, ptPacket->tHeader.ulLen);
  }

  PRINTF(NEWLINE);
}

/*****************************************************************************/
/*****************************************************************************/
uint32_t
Pkt_SendPacket( APP_DATA_T*  ptAppData,
                int          iChannelIdx,
                CIFX_PACKET* ptPacket,
                uint32_t     ulTimeout )
{
  uint32_t lRet = CIFX_NO_ERROR;

  lRet = xChannelPutPacket(ptAppData->aptChannels[iChannelIdx]->hChannel, ptPacket, ulTimeout);

  if(CIFX_NO_ERROR == lRet)
  {
    s_tPacketCounter.ulPacketSendCountSuccess++;
    PRINTF("========================================================" NEWLINE);
    PRINTF("Sent packet:" NEWLINE);
    Pkt_DumpPacket(ptPacket);
    PRINTF("========================================================" NEWLINE);
  }
  else
  {
    s_tPacketCounter.ulPacketSendCountError++;

    PRINTF("========================================================" NEWLINE);
    PRINTF("Sending packet failed with error: 0x%08x" NEWLINE, (unsigned int) lRet);
    Pkt_DumpPacket(ptPacket);
    PRINTF("========================================================" NEWLINE);
  }

  return lRet;
}

/**************************************************************************************/
/**************************************************************************************/
uint32_t
Pkt_ReceivePacket( APP_DATA_T*  ptAppData,
                   int          iChannelIdx,
                   CIFX_PACKET* ptPacket,
                   uint32_t     ulTimeout )
{
  uint32_t lRet = CIFX_NO_ERROR;

  /* ⭐ Debug: Before xChannelGetPacket */
  static uint32_t s_ulCallCount = 0;
  if(++s_ulCallCount % 100 == 0)
  {
    PRINTF("[DEBUG] Pkt_ReceivePacket called %u times" NEWLINE, (unsigned int)s_ulCallCount);
  }

  lRet = xChannelGetPacket(ptAppData->aptChannels[iChannelIdx]->hChannel, sizeof(*ptPacket), ptPacket, ulTimeout);

  /* ⭐ Debug: After xChannelGetPacket */
  if(CIFX_NO_ERROR == lRet)
  {
    PRINTF("========================================================" NEWLINE);
    PRINTF("🎯 PACKET RECEIVED FROM netX! Cmd=0x%08X" NEWLINE, (unsigned int)ptPacket->tHeader.ulCmd);
    PRINTF("========================================================" NEWLINE);
    PRINTF("received packet:" NEWLINE);
    Pkt_DumpPacket(ptPacket);
    PRINTF("========================================================" NEWLINE);
  }
  else if(lRet != CIFX_DEV_GET_NO_PACKET)
  {
    /* Only log real errors, not "no packet" */
    PRINTF("[ERROR] xChannelGetPacket returned: 0x%08X" NEWLINE, (unsigned int)lRet);
  }

  return lRet;
}


/**************************************************************************************/
/**************************************************************************************/

bool
Pkt_RegisterIndicationHandler(int iChannelIdx,
                              bool (*fnHandler)( CIFX_PACKET* ptPacket, void* pvUserData ),
                              void* pvUserData )
{
  if( NULL == s_atIndicationHandler[iChannelIdx].fnHandler)
  {
    s_atIndicationHandler[iChannelIdx].fnHandler = fnHandler;
    s_atIndicationHandler[iChannelIdx].pvUserData = pvUserData;
    return true;
  }
  else
  {
    return false;
  }
}

/**************************************************************************************/
/*! Put a packet into the confirmation queue
 *
 *   \param ptPacket   [in]  Packet to be put into the queue
 *
 *   \return true:  Packet could be queued
 *           false: Packet could not be queued
 */
/**************************************************************************************/
static bool
Pkt_QueueConfirmation( int iChannelIdx, CIFX_PACKET* ptPacket )
{
  int i;
  for(i = 0; i < s_aiReceiveQueueDepths[iChannelIdx]; i++)
  {
    if(s_aaptReceiveQueue[iChannelIdx][i]->fUsed == false)
    {
      s_aaptReceiveQueue[iChannelIdx][i]->fUsed = true;
      s_aaptReceiveQueue[iChannelIdx][i]->tPacket = *ptPacket;
      return true;
    }
  }

  return false;
}

/**************************************************************************************/
/*! Get a packet from the confirmation queue
 *
 *   \param ptPacket [out]  Packet dequeued from the confirmation queue.
 *
 *   \return true:  Packet could be queued
 *           false: Packet could not be queued
 */
/**************************************************************************************/
static bool Pkt_GetPacketFromQueue(int iChannelIdx, CIFX_PACKET* ptPacket)
{
  int i;
  for(i = 0; i < s_aiReceiveQueueDepths[iChannelIdx]; i++)
  {
    if(s_aaptReceiveQueue[iChannelIdx][i]->fUsed == true)
    {
      *ptPacket = s_aaptReceiveQueue[iChannelIdx][i]->tPacket;
      s_aaptReceiveQueue[iChannelIdx][i]->fUsed = false;
      return true;
    }
  }
  return false;
}

/**************************************************************************************/
/*! Get a specific packet (with matching ulCmd and ulId) from the confirmation queue.
 *
 *   \param ulCmd    [in]   ulCmd value of the wanted packet
 *   \param ulId     [in]   ulId value  of the wanted packet
 *   \param ptPacket [out]  Packet dequeued from the confirmation queue.
 *
 *   \return true:  Packet was found
 *           false: Packet could not be found
 */
/**************************************************************************************/
static bool Pkt_TryDequeConfirmation( int iChannelIdx,
                                      uint32_t     ulCmd,
                                      uint32_t     ulId,
                                      CIFX_PACKET* ptPacket )
{
  int i;
  for(i = 0; i < s_aiReceiveQueueDepths[iChannelIdx]; i++)
  {
    if( (s_aaptReceiveQueue[iChannelIdx][i]->fUsed == true)                  &&
        (s_aaptReceiveQueue[iChannelIdx][i]->tPacket.tHeader.ulCmd == ulCmd) &&
        (s_aaptReceiveQueue[iChannelIdx][i]->tPacket.tHeader.ulId == ulId)
       )
    {
      *ptPacket = s_aaptReceiveQueue[iChannelIdx][i]->tPacket;
      s_aaptReceiveQueue[iChannelIdx][i]->fUsed = false;
      return true;
    }
  }
  return false;
}

/**************************************************************************************/
/*! Dispatch a packet to the registered indication handler
 *
 *   \param ptPacket [out]  Packet that shall be dispatched
 *
 *   \return true:  Packet could be dispatched
 *           false: Packet could not be dispatched (no registered indication handler available)
 */
/**************************************************************************************/

static bool
Pkt_DispatchIndication( int iChannelIdx, CIFX_PACKET* ptPacket )
{
  /* Dispatch this indication */
  if(s_atIndicationHandler[iChannelIdx].fnHandler)
  {
    return s_atIndicationHandler[iChannelIdx].fnHandler(ptPacket, s_atIndicationHandler[iChannelIdx].pvUserData);
  }
  else
  {
    return false;
  }
}

/**************************************************************************************/
/**************************************************************************************/

uint32_t
Pkt_CheckReceiveMailbox( APP_DATA_T *ptAppData,
                         int iChannelIdx,
                         CIFX_PACKET* ptPacket )
{
  uint32_t ulRet = CIFX_NO_ERROR;

  /* First, let's see if there are some unprocessed confirmations in our queue. */
  if( false == Pkt_GetPacketFromQueue(iChannelIdx, ptPacket))
  {
    /* No packets in the confirmation queue --> check the receive mailbox. */
    ulRet = Pkt_ReceivePacket(ptAppData, iChannelIdx, ptPacket, RX_TIMEOUT);

    /* ⭐ Debug: Check receive result */
    if(ulRet == CIFX_DEV_GET_NO_PACKET)
    {
      /* This is normal - no packet available */
    }
    else if(ulRet != CIFX_NO_ERROR)
    {
      PRINTF("[DEBUG] Pkt_ReceivePacket failed: 0x%08X" NEWLINE, (unsigned int)ulRet);
    }
  }

  if( CIFX_NO_ERROR == ulRet)
  {
    /* ⭐ Debug: Packet received from netX */
    PRINTF("[DEBUG] Pkt_CheckReceiveMailbox: Received Cmd=0x%08X" NEWLINE,
           (unsigned int)ptPacket->tHeader.ulCmd);

#if !defined( DEMO_QUIET ) && !defined( DEMO_DONT_USE_COMMAND_LOOKUP )
    PRINTF( "[ok] IND:           Packet ulCmd 0x%08x (%s)" NEWLINE NEWLINE,
            (unsigned int)ptPacket->tHeader.ulCmd,
      LookupCommandCode(ptPacket->tHeader.ulCmd));
#endif

    /* ⭐ Debug: Before dispatch */
    PRINTF("[DEBUG] Before Pkt_DispatchIndication" NEWLINE);

    if(!Pkt_DispatchIndication(iChannelIdx, ptPacket))
    {
      /* ⭐ Debug: Dispatch failed - no handler registered */
      PRINTF("[WARN] Pkt_DispatchIndication failed - no handler!" NEWLINE);

      /* Indication was not handled, so we send the response on our own. */
      ptPacket->tHeader.ulCmd |= 0x01; /* Make it a response */
      ptPacket->tHeader.ulLen = 0;
      ptPacket->tHeader.ulState = CIFX_INVALID_COMMAND;

      Pkt_SendPacket(ptAppData, iChannelIdx, ptPacket, TX_TIMEOUT);
    }
    else
    {
      /* ⭐ Debug: Dispatch succeeded */
      PRINTF("[DEBUG] Pkt_DispatchIndication succeeded" NEWLINE);
    }
  }

  return ulRet;
}

/**************************************************************************************/
/**************************************************************************************/

uint32_t
Pkt_SendReceivePacket( APP_DATA_T  *ptAppData,
                       int          iChannelIdx,
                       CIFX_PACKET* ptPacket,
                       uint32_t     ulTimeout )
{
  uint32_t ulRet = CIFX_NO_ERROR;
  uint32_t ulCmd = ptPacket->tHeader.ulCmd;
  uint32_t ulPacketId;

  if(!(ptPacket->tHeader.ulExt & (HIL_PACKET_SEQ_MIDDLE | HIL_PACKET_SEQ_LAST)))
  {
    s_ulNextPacketId++;
  }

  ulPacketId             = s_ulNextPacketId;
  ptPacket->tHeader.ulId = ulPacketId;

  /* issue the request */
  ulRet = Pkt_SendPacket(ptAppData, iChannelIdx, ptPacket, ulTimeout);

  if( CIFX_NO_ERROR != ulRet)
  {
#if !defined( DEMO_QUIET ) && !defined( DEMO_DONT_USE_COMMAND_LOOKUP )
    PRINTF( "[!!] REQ:      Packet ulCmd 0x%08x (%s) failed with error 0x%08x" NEWLINE NEWLINE,
            (unsigned int)ptPacket->tHeader.ulCmd,
            LookupCommandCode(ptPacket->tHeader.ulCmd),
            (unsigned int)ulRet );
#endif
  }
  else
  {
    /* Sending the packet succeeded */

#if !defined( DEMO_QUIET ) && !defined( DEMO_DONT_USE_COMMAND_LOOKUP )
    PRINTF( "[ok] REQ:           Packet ulCmd 0x%08x (%s)" NEWLINE NEWLINE,
            (unsigned int)ulCmd,
            LookupCommandCode(ptPacket->tHeader.ulCmd) );
#endif

    /* Now, let's wait for the confirmation. All indications coming up in the meantime will be dispatched. */
    while(1)
    {
      ulRet = Pkt_ReceivePacket(ptAppData, iChannelIdx , ptPacket, ulTimeout);

      if(CIFX_NO_ERROR == ulRet)
      {
        if((ptPacket->tHeader.ulCmd == (ulCmd | 0x1)) && (ptPacket->tHeader.ulId == ulPacketId))
        {
          /* This is the confirmation we were waiting for. */
          break;
        }
        else if(ptPacket->tHeader.ulCmd & 0x1)
        {
          /* This is a confirmation for another command we have issued, so put it into the receive queue for now. */
#if !defined( DEMO_QUIET ) && !defined( DEMO_DONT_USE_COMMAND_LOOKUP )
          PRINTF("Warning: Unexpected confirmation packet queued! Are you recursively issuing commands in your indication handler!?!" NEWLINE);
#endif
          if (!Pkt_QueueConfirmation(iChannelIdx, ptPacket))
          {
            assert(0);
            PRINTF("FATAL: Confirmation queue overrun. Stopping. Don't issue commands in indication handlers." NEWLINE);
            while (1)
            {
            	OS_Sleep(10);
            }
          }
        }
        else
        {
          if(!Pkt_DispatchIndication(iChannelIdx, ptPacket))
          {
            /* Indication was not handled, so we send the response on our own. */
            ptPacket->tHeader.ulCmd |= 0x01; /* Make it a response */
            ptPacket->tHeader.ulLen = 0;
            ptPacket->tHeader.ulState = CIFX_INVALID_COMMAND;

            Pkt_SendPacket(ptAppData, iChannelIdx, ptPacket, TX_TIMEOUT);
          }

          if(Pkt_TryDequeConfirmation(iChannelIdx, (ulCmd | 0x1), ulPacketId, ptPacket))
          {
            /* The confirmation we are waiting for was queued up during indication handling, we're fine. */
#if !defined( DEMO_QUIET ) && !defined( DEMO_DONT_USE_COMMAND_LOOKUP )
            PRINTF("A queued-up confirmation packet was processed!" NEWLINE NEWLINE);
#endif
            break;
          }
        }
      }
      else if( CIFX_DEV_GET_NO_PACKET == ulRet)
      {
        /* No packet within the given timeout received. */
        break;
      }
    }

    if( CIFX_NO_ERROR == ulRet)
    {
#if !defined( DEMO_QUIET ) && !defined( DEMO_DONT_USE_COMMAND_LOOKUP )
      if(ptPacket->tHeader.ulState == 0)
      {
        PRINTF( "[ok] REQ->CNF:      Packet ulCmd 0x%08x (%s)" NEWLINE NEWLINE,
                (unsigned int)ulCmd,
                LookupCommandCode(ptPacket->tHeader.ulCmd) );
      }
      else
      {
        PRINTF( "[!!] REQ->CNF:      Packet ulCmd 0x%08x (%s)  Status 0x%08x " NEWLINE NEWLINE,
                (unsigned int)ulCmd,
                LookupCommandCode(ptPacket->tHeader.ulCmd),
                (unsigned int)ptPacket->tHeader.ulState );
      }
#endif
    }
    else
    {
      PRINTF("[!!] SEND/RECV of packet failed with Status: 0x%08x" NEWLINE NEWLINE, (unsigned int) ulRet);
    }
  }

  return ulRet;
}

/**************************************************************************************/
/**************************************************************************************/

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

#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "App_Config.h"
#include "App_DemoApplication.h"
#include "App_PacketCommunication.h"

#include "Hil_Packet.h"
#include "Hil_Results.h"


#define PKT_PACKET_SEND_TIMEOUT   250 /* This is the maximum time the modules try to send a packet */
#define PKT_MAX_PACKET_HANDLERS   3   /* Number is handlers which can be registered per handler (channel) */
#define PKT_MAX_HEX_DUMP_SIZE     CIFX_MAX_DATA_SIZE


/** Structure to store application's packet handler functions. */
typedef struct
{
  Pkt_HandlePacketCallback  pfnHandler;
  void*                     pvUserData;
} PKT_INTERFACE_HANDLER_T;

/** Packet communication interface resources */
typedef struct PKT_INTERFACE_Ttag
{
  CIFXHANDLE              hCifXChannel;
  PKT_INTERFACE_HANDLER_T atPktHndl[PKT_MAX_PACKET_HANDLERS];

  uint32_t                ulSendReceivePacketId; /* unique id for send/receive function */
} PKT_INTERFACE_T;


#ifdef PKT_PACKET_TRACE_PRINTF
/*! print a hex dump on the debug console (16 bytes per line)
 *
 *   \param pbData    [in]  Data buffer that shall be dumped
 *   \param ulDataLen [in]  Number of bytes in pbData
 *
 */
static void
Pkt_PrintHexData(unsigned char* pbData,
                 unsigned long  ulDataLen )
{

  if(PKT_MAX_HEX_DUMP_SIZE < ulDataLen){
    ulDataLen = PKT_MAX_HEX_DUMP_SIZE;
  }

  for(int i = 0; i < ulDataLen; i++){
    if(0 == (i % 16)){
      PKT_PACKET_TRACE_PRINTF(NEWLINE);
    }
    PKT_PACKET_TRACE_PRINTF("%02X ", pbData[i]);
  }
}

/*! Dumps a packet to the debug console.
 *
 *   \param ptPacket  [in] Pointer to buffer where packet is stored
 *
 */
static void
Pkt_PrintPacket( CIFX_PACKET* ptPacket )
{
  PKT_PACKET_TRACE_PRINTF("Dest  : 0x%08X    ID  : 0x%08X" NEWLINE, (unsigned int) ptPacket->tHeader.ulDest, (unsigned int) ptPacket->tHeader.ulId);
  PKT_PACKET_TRACE_PRINTF("Src   : 0x%08X    Sta : 0x%08X" NEWLINE, (unsigned int) ptPacket->tHeader.ulSrc, (unsigned int) ptPacket->tHeader.ulState);
  PKT_PACKET_TRACE_PRINTF("DestID: 0x%08X    Cmd : 0x%08X" NEWLINE, (unsigned int) ptPacket->tHeader.ulDestId, (unsigned int) ptPacket->tHeader.ulCmd);
  PKT_PACKET_TRACE_PRINTF("SrcID : 0x%08X    Ext : 0x%08X" NEWLINE, (unsigned int) ptPacket->tHeader.ulSrcId, (unsigned int) ptPacket->tHeader.ulExt);
  PKT_PACKET_TRACE_PRINTF("Len   : 0x%08X    Rout: 0x%08X" NEWLINE, (unsigned int) ptPacket->tHeader.ulLen, (unsigned int) ptPacket->tHeader.ulRout);

  if(ptPacket->tHeader.ulLen)
  {
    PKT_PACKET_TRACE_PRINTF("Data:");
    Pkt_PrintHexData(ptPacket->abData, ptPacket->tHeader.ulLen);
  }

  PKT_PACKET_TRACE_PRINTF(NEWLINE);
  PKT_PACKET_TRACE_PRINTF(NEWLINE);
}
#else
#define Pkt_PrintPacket(Pkt)
#endif

/*! Put packet into mailbox.
 *
 * \param hCifXChannel [in] CifX channel handle
 * \param ptPacket     [in] Pointer to buffer where packet is stored
 * \param ulTimeout    [in] Time to be wait for a packet
 *
 */
static uint32_t
Pkt_PutPacket( CIFXHANDLE   hCifXChannel,
                CIFX_PACKET* ptPacket,
                uint32_t     ulTimeout )
{
  uint32_t lRet = CIFX_NO_ERROR;

  lRet = xChannelPutPacket(hCifXChannel, ptPacket, ulTimeout);

  if(CIFX_NO_ERROR == lRet)
  {
    PRINTF("Packet sent:" NEWLINE);
    Pkt_PrintPacket(ptPacket);
  }
  else
  {
    PRINTF("Packet sent, failed with error: 0x%08x" NEWLINE, (unsigned int) lRet);
    Pkt_PrintPacket(ptPacket);
  }

  return lRet;
}

/*! Get packet from mailbox.
 *
 * \param hCifXChannel [in]  CifX channel handle
 * \param ptPacket     [out] Pointer to buffer where packet will be stored
 * \param ulTimeout    [in]  Time to be wait for a packet
 *
 */
static uint32_t
Pkt_GetPacket(CIFXHANDLE   hCifXChannel,
              CIFX_PACKET* ptPacket,
              uint32_t     ulTimeout)
{
  uint32_t lRet = CIFX_NO_ERROR;

  lRet = xChannelGetPacket(hCifXChannel, sizeof(*ptPacket), ptPacket, ulTimeout);

  if(CIFX_NO_ERROR == lRet)
  {
    PRINTF("Packet received:" NEWLINE);
    Pkt_PrintPacket(ptPacket);
  }

  return lRet;
}

/*! Dispatch a packet to the registered indication handlers.
 *
 * The function literate over all registered handers, if no handler process this packet
 * the function will automatically generate a response for indications, or drop
 * confirmations silently.
 *
 *   \param ptPacket [out]  Packet that shall be dispatched
 *
 */
static void
Pkt_DispatchIndication(PKT_INTERFACE_T *ptPktIfRsc, CIFX_PACKET* ptPacket )
{
  int   i           = 0;
  bool  fDispatched = false;

  /* Iterate over registered packet handlers */
  while(i < PKT_MAX_PACKET_HANDLERS){
    PKT_INTERFACE_HANDLER_T *ptInterfacePktHandler = &ptPktIfRsc->atPktHndl[i++];

    /* Dispatch this indication */
    if(ptInterfacePktHandler->pfnHandler != NULL){
      fDispatched = ptInterfacePktHandler->pfnHandler(ptInterfacePktHandler->pvUserData, ptPacket);
      if( fDispatched ){
        break; /* Packet has been successfully dispatched, stop the loop */
      }
    }
  }

  if(!fDispatched && 0 == (ptPacket->tHeader.ulCmd & 1)){
   /* This was an indication no one has handled, so we will generate a response */
   ptPacket->tHeader.ulCmd  |= 0x01;
   ptPacket->tHeader.ulLen   = 0;
   ptPacket->tHeader.ulState = ERR_HIL_NO_APPLICATION_REGISTERED;

   Pkt_PutPacket(ptPktIfRsc->hCifXChannel, ptPacket, 0xFFFFFFFF);
  }
}



/**************************************************************************************************/
/*  Public functions                                                                              */
/**************************************************************************************************/
bool
Pkt_Init(PKT_INTERFACE_H *phPktIfRsc, CIFXHANDLE hCifXChannel)
{
  PKT_INTERFACE_T *ptPktIfRsc = calloc(1, sizeof(PKT_INTERFACE_T));

  if (NULL == ptPktIfRsc){
    return false;
  }

  ptPktIfRsc->hCifXChannel           = hCifXChannel;

  /* Let's start with 512.000, nice 'round' number in hex and dec */
  ptPktIfRsc->ulSendReceivePacketId = 0x7D000;

  *phPktIfRsc = ptPktIfRsc;
  return true;
}

/**************************************************************************************************/
void
Pkt_Deinit(PKT_INTERFACE_H *phPktIfRsc)
{
  if(NULL == *phPktIfRsc){
    free(*phPktIfRsc);
    *phPktIfRsc = NULL;
  }
}

/**************************************************************************************************/
bool
Pkt_RegisterPacketHandler(PKT_INTERFACE_T *ptPktIfRsc,
                              Pkt_HandlePacketCallback pfnHandler,
                              void* pvUserData )
{
  int  i = 0;

  while(i < PKT_MAX_PACKET_HANDLERS)
  {
    PKT_INTERFACE_HANDLER_T *ptInterfacePktHandler = &ptPktIfRsc->atPktHndl[i++];

    /* Dispatch this indication */
    if(ptInterfacePktHandler->pfnHandler == NULL)
    {
      ptInterfacePktHandler->pfnHandler = pfnHandler;
      ptInterfacePktHandler->pvUserData = pvUserData;

      return true;
    }
  }

  return false;
}

/**************************************************************************************************/
void
Pkt_CheckReceiveMailbox(PKT_INTERFACE_T *ptPktIfRsc, CIFX_PACKET* ptPacket)
{
  uint32_t    ulRet     = CIFX_NO_ERROR;

  /* Check the receive mailbox. */
  ulRet = Pkt_GetPacket(ptPktIfRsc->hCifXChannel, ptPacket, 0);

  if( CIFX_NO_ERROR == ulRet)
  {
    Pkt_DispatchIndication(ptPktIfRsc, ptPacket);
  }
}

/**************************************************************************************************/
uint32_t
Pkt_SendPacket(PKT_INTERFACE_T *ptPktIfRsc,
               CIFX_PACKET     *ptPacket,
               uint32_t        ulTimeout)
{
  uint32_t ulRet      = CIFX_NO_ERROR;

  /* issue the request */
  ulRet = Pkt_PutPacket(ptPktIfRsc->hCifXChannel, ptPacket, ulTimeout);

  return ulRet;
}



/**************************************************************************************************/
uint32_t
Pkt_SendReceivePacket(PKT_INTERFACE_T *ptPktIfRsc,
                      CIFX_PACKET*    ptPacket,
                      uint32_t        ulTimeout )
{
  uint32_t ulRet      = CIFX_NO_ERROR;
  uint32_t ulCmd      = ptPacket->tHeader.ulCmd;

  if(!(ptPacket->tHeader.ulExt & (HIL_PACKET_SEQ_MIDDLE | HIL_PACKET_SEQ_LAST))){
    ptPacket->tHeader.ulId = ptPktIfRsc->ulSendReceivePacketId;
  }

  /* issue the request */
  ulRet = Pkt_PutPacket(ptPktIfRsc->hCifXChannel, ptPacket, PKT_PACKET_SEND_TIMEOUT);

  if( CIFX_NO_ERROR == ulRet)
  {
    /* Now, let's wait for the confirmation. All indications coming up in the meantime will be dispatched. */
    while(1)
    {
      ulRet = Pkt_GetPacket(ptPktIfRsc->hCifXChannel, ptPacket, ulTimeout);

      if(CIFX_NO_ERROR == ulRet)
      {
        if((ptPacket->tHeader.ulCmd == (ulCmd | 0x1)) && (ptPacket->tHeader.ulId == ptPktIfRsc->ulSendReceivePacketId))
        {
          /* This is the confirmation we were waiting for. */
          break;
        }
        else
        {
          /* Dispatch out of sequence packets directly. */
          Pkt_DispatchIndication(ptPktIfRsc, ptPacket);
        }
      }
      else if( CIFX_DEV_GET_NO_PACKET == ulRet)
      {
        /* No packet within the given timeout received. */
        break;
      }
    }
  }

  /* Update packet ID counter */
  ptPktIfRsc->ulSendReceivePacketId++;
  return ulRet;
}


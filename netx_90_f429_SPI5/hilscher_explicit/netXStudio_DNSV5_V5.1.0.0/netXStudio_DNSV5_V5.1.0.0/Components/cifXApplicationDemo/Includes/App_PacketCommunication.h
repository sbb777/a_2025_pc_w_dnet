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


Synchronous Packet Interface module:
  This module enables the application to handle cifX packets in a synchronous way. Often there is
  the need to send a packet and wait for the corresponding confirmation (e.g. during start up).
  In addition this module can be used to process all other packets, indications and asynchronous
  issued packets, within a "packet handler" call back function.



**************************************************************************************************/
#ifndef APP_PACKETCOMMUNICATION_H_
#define APP_PACKETCOMMUNICATION_H_

#include <stdint.h>
#include <stdbool.h>
#include "cifXUser.h"
#include "cifXErrors.h"

/* Handle to the packet handler */
typedef struct PKT_INTERFACE_Ttag *PKT_INTERFACE_H;

/*! Handle packet callback.
 * This function must be implemented by the application, to handle all packets which shall be
 * processed.
 *
 *  \param pvUserData  [in] User parameter provided during Pkt_RegisterIndicationHandler call
 *  \param ptPacket    [in] Packet which was received
 *
 *   \return  If the packet could be "processed" by the application the function shall return true.
 *
 *            "processed" means, if the packet was a indication ((tHeader.ulCmd & 0x1)==0) is true),
 *            the indication has been processed and a response has been send (or will be send).
 *            If the packet was a confirmation the status was checked (and the content was handled).
 *
 *                               Note: fnHandler is supposed to return true, in case the
 *                                     packet was processed. Otherwise, false shall be returned.
 *
 *
 *  */
typedef bool (*Pkt_HandlePacketCallback)(void* pvUserData, CIFX_PACKET* ptPacket);



/*! Initialization the packet handler for the given channel
 *
 * After initialization the application should not call xChannelPutPacket() or xChannelGetPacket()
 * directly. Instead the abstracted functions
 *  - Pkt_SendPacket()
 *  - Pkt_SendReceivePacket()
 *  - Pkt_CheckReceiveMailbox()
 * shall be used.
 *
 *   \param phPktIfRsc          [in,out] Pointer where the handle of the packet handler
 *                                       shall be stored.
 *   \param hCifXChannel        [in]     Handle to the cifX channel which should be used.
 *
 *   \return true on success, otherwise false.
 */
bool Pkt_Init(PKT_INTERFACE_H* phPktIfRsc, CIFXHANDLE hCifXChannel);

/*! Deinitialization of the packet handler
 *
 * \param hPktIfRsc [in] Handle to the packet handler
 */
void Pkt_Deinit(PKT_INTERFACE_H* phPktIfRsc);

/** Register an packet handler
 *
 * This function can be used to register an packet handler. This packet handler can than
 * process the incoming indications and confirmation packets.
 * Please refer the Pkt_HandlePacketCallback description about the implementation of this
 * callback.
 *
 * \param hPktIfRsc   [in] Handle to the packet handler.
 * \param fnHandler   [in] Packet handler callback function that will be called on every incoming
 *                         packet, except for synchronous issued packets issued by
 *                         Pkt_SendReceivePacket() function.
 * \param pvUserData  [in] User parameter that will be provided when fnHandler() is called.
 *
 * \return   true, if handler could be registered successfully, false otherwise.
 */
bool Pkt_RegisterPacketHandler(PKT_INTERFACE_H hPktIfRsc, Pkt_HandlePacketCallback pfnHandler,void* pvUserData);


/** Send a packet
 *
 * This function is used for sending packet without waiting for an response. This function should
 * be used e.g. to send responses to indications or issue requests without waiting for the response.
 *
 * \param hPktIfRsc [in]  Handle to the packet handler.
 * \param ptPacket  [in]  Pointer to packet which should be issued.
 * \param ulTimeout [in]  Timeout value in milliseconds that will be used for sending the request.
 *
 * \return == CIFX_NO_ERROR:  Request packet could be sent.
 *         != CIFX_NO_ERROR:  Sending of the request failed.
 */
uint32_t Pkt_SendPacket(PKT_INTERFACE_H hPktIfRsc, CIFX_PACKET* ptPacket, uint32_t ulTimeout);

/** Synchronous send packet and wait for response
 *
 * This function can be used to issue a packet and wait for the corresponding
 * response/confirmation.
 * After the packet was issued, the function waits for the corresponding response/confirmation.
 * packets arriving in between, will be dispatched to the registered Pkt_HandlePacketCallback
 * functions.
 *
 * Note: You should avoid calling this function within the registered Pkt_HandlePacketCallback
 *       callback. Doing so will lead into a recursion.
 *
 * Note: The function will use the ID field of the packet (ptPacket->tHeader.ulId) to mark the
 *       packet. The corresponding response is matched by comparing:
 *          (Send ptPacket->tHeader.ulId  == received ptPacket->tHeader.ulId AND
 *           Send ptPacket->tHeader.ulCmd == received ptPacket->tHeader.ulCmd | 1)
 *
 * \param hPktIfRsc [in]     Handle to the packet handler.
 * \param ptPacket  [in,out] Pointer to packet which should be issued. And buffer to store the
 *                           corresponding response.
 * \param ulTimeout [in]     Timeout value in milliseconds that will be used for sending the request
 *                           and wait for the response so the function may wait up to 2x the given
 *                           timeout.
 *
 * \return == CIFX_NO_ERROR: Request packet could be sent and corresponding response
 *                           received successfully.
 *         != CIFX_NO_ERROR: Sending of the request failed or receiving confirmation packet failed.
 *
 * Note: On a successful reception of a packet the application still has to check the response
 *       status code of the packet.
 */
uint32_t Pkt_SendReceivePacket(PKT_INTERFACE_H hPktIfRsc, CIFX_PACKET* ptPacket, uint32_t ulTimeout);


/** Poll the receive mailbox for packets.
 *
 * Note: If no registered handler will process the packet, the function will send a response with
 *       an error code set to ERR_HIL_NO_APPLICATION_REGISTERED. The function will block infinite
 *       until the response could be sent!
 *
 * \param ptAppData   [in]  Pointer to the application's resources
 * \param iChannelIdx [in]  Index of the channel the packet shall be sent/received
 * \param ptPacket    [in]  Pointer to packet resource. This resource will be used to receive a
 *                          packet from the mailbox and pass it to the registered handler.
 *
 */
void Pkt_CheckReceiveMailbox(PKT_INTERFACE_H hPktIfRsc, CIFX_PACKET* ptPacket);

#endif /* APP_PACKETCOMMUNICATION_H_ */

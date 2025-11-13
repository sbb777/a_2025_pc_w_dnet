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

#ifndef COMPONENTS_CIFXAPPLICATIONDEMO_INCLUDES_APP_PACKETCOMMUNICATION_H_
#define COMPONENTS_CIFXAPPLICATIONDEMO_INCLUDES_APP_PACKETCOMMUNICATION_H_

/*****************************************************************************/
/*! General Inclusion Area                                                   */
/*****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "cifXUser.h"
#include "cifXErrors.h"

/*****************************************************************************/
/*! DEFINITIONS                                                              */
/*****************************************************************************/

#define NEWLINE "\r\n"

/* Timeout values for sending (TX) and receiving (RX) packets. */
#define TX_TIMEOUT       500 /* milliseconds */
#define RX_TIMEOUT        10 /* milliseconds */
#define TXRX_TIMEOUT     500 /* milliseconds */

/*****************************************************************************/
/*! FORWARD DECLARATIONS                                                     */
/*****************************************************************************/

struct APP_DATA_Ttag;

/*****************************************************************************/
/*! FUNCTION PROTOTYPES                                                      */
/*****************************************************************************/

/*****************************************************************************/
/*! Allocate memory for packet handling on the given channel with a
 *   specific confirmation queue depth.
 *
 *   Note: this function can be called right at the beginning of the
 *         "Start Configuration" function (e.g. Protocol_StartConfiguration).
 *         This way each channel demo implementation can decide on its own how
 *         deep the confirmation queue must be.
 *         The cleanup of the allocated memory is done automatically when the
 *         generic demo application shuts down(call of Pkt_Deinit).
 *         Therefore, there is no need for the channel demo implementations
 *         to cleanup their packets.
 *
*   \param iChannelIdx        [in] Index of the channel
*   \param iReceiveQueueDepth [in] Confirmation queue depth. This is the number
*                                  of confirmation packets that can be put on hold
*                                  in case we receive a confirmation that we
*                                  are not waiting for. This queue is used only
*                                  in case packets are sent/received with the
*                                  function Pkt_SendReceivePacket()
*
*   \return true on success, otherwise false.
 */
/*****************************************************************************/
bool Pkt_Init(int iChannelIdx, int iReceiveQueueDepth);

/*****************************************************************************/
/*! Free memory for packet handling for all channels */
/*****************************************************************************/
void Pkt_Deinit(void);

/*****************************************************************************/
/*! Send a packet via the given channel
 *
*   \param ptAppData   [in] Pointer to the application's resources
*   \param iChannelIdx [in] Index of the channel the packet shall be sent on
*   \param ptPacket    [in] Packet to be sent
*   \param ulTimeout   [in] Time in ms to wait for successful sending.
*/
/*****************************************************************************/
uint32_t
Pkt_SendPacket( struct APP_DATA_Ttag* ptAppData,
                int                   iChannelIdx,
                CIFX_PACKET*          ptPacket,
                uint32_t              ulTimeout );

/**************************************************************************************/
/*! Retrieve a packet from the channel mailbox
 *
*   \param ptAppData   [in]  Pointer to the application's resources
*   \param iChannelIdx [in]  Index of the channel the packet shall be received from
*   \param ptPacket    [out] Received packet if function succeeds
*   \param ulTimeout   [in]  Time in ms to wait for available message
 *
 *   \return CIFX_NO_ERROR on success
 */
/**************************************************************************************/
uint32_t
Pkt_ReceivePacket( struct APP_DATA_Ttag* ptAppData,
                   int                   iChannelIdx,
                   CIFX_PACKET*          ptPacket,
                   uint32_t              ulTimeout );

/**************************************************************************************/
/**************************************************************************************/

/*   Synchronous Packet Interface

  This packet interface enables the user to send/receive request/confirmation packets in a synchronous fashion.
  The handling of indication packets is completely separated from that.

  To do so, the user:
    1) Registers his indication callback function using Pkt_RegisterIndicationHandler()
    2) Uses Pkt_SendReceivePacket() for sending/receiving request/confirmation packets
    3) Makes sure the function Pkt_CheckReceiveMailbox() gets called cyclically in order to poll the receive
       mailbox for indication packets.
 */

/****************************************************************************************/
/** This function can be used to register an indication packet handler.
 *  Every time an indication is received, this handler will be called.
 *
 * \param iChannelIdx [in]     Index of the channel the indication handle shall be registered for
 * \param fnHandler   [in]     Callback function that will be called every
 *                             time an indication packet is received.
 *                             Note: fnHandler is supposed to return true, in case the
 *                                   packet was processed. Otherwise, false shall be returned.
 * \param pvUserData  [in]     Parameter that will be provided when calling fnHandler()
 *
 * \return   true, if handler could be registered successfully, false otherwise.
 */
bool Pkt_RegisterIndicationHandler( int iChannelIdx, bool(*fnHandler)(CIFX_PACKET* ptPacket, void* pvUserData), void* pvUserData );

/****************************************************************************************/
/** This function can be used to transmit a request packet and receive the corresponding
 *  confirmation in a synchronous manner.
 *  After sending the request packet, the function waits for the confirmation packet.
 *
 *  During waiting, also indication packets and other (unexpected) confirmation packets are handled:
 *    - Indication packets are dispatched by using the registered
 *      indication handler (see Pkt_RegisterIndicationHandler).
 *    - Unexpected confirmation packets are put into a queue for later processing.
 *
 * \param ptAppData   [in]      Pointer to the application's resources
 * \param iChannelIdx [in]      Index of the channel the packet shall be sent/received
 * \param ptPacket    [in,out]  Pointer to request packet. This resource will also be
 *                              used to provided the confirmation packet content to the caller.
 * \param ulTimeout   [in]      Timeout value in milliseconds that will be used for sending the request and
 *                             receiving the confirmation packet.
 *
 * \return    CIFX_NO_ERROR:    Request packet could be sent/received successfully.
 *         != CIFX_NO_ERROR:    Sending request packet or receiving confirmation packet failed.
 *         Note: this return code is not affected by the packet status of the confirmation packet.
 */
uint32_t Pkt_SendReceivePacket( struct APP_DATA_Ttag* ptAppData, int iChannelIdx, CIFX_PACKET* ptPacket, uint32_t ulTimeout );

/****************************************************************************************/
/** This function can be used to poll for mailbox packets and automatically pass them to the
 *  indication handler (registered via Pkt_RegisterIndicationHandler() ).
 *
 *  Note: All packets not processed by the indication handler
 *        will be returned with status code CIFX_INVALID_COMMAND.
 *
 * \param ptAppData   [in]  Pointer to the application's resources
 * \param iChannelIdx [in]  Index of the channel the packet shall be sent/received
 * \param ptPacket    [in]  Pointer to packet resource. This resource will be used to receive a packet
 *                          from the mailbox and pass it to the indication handler.
 *
 * \return    CIFX_NO_ERROR:    Request packet could be sent/received successfully.
 *         != CIFX_NO_ERROR:    Sending request packet or receiving confirmation packet failed.
 *         Note: this return code is not affected by the packet status of the confirmation packet.
 */
uint32_t Pkt_CheckReceiveMailbox(struct APP_DATA_Ttag* ptAppData, int iChannelIdx, CIFX_PACKET* ptPacket );

#endif /* COMPONENTS_CIFXAPPLICATIONDEMO_INCLUDES_APP_PACKETCOMMUNICATION_H_ */

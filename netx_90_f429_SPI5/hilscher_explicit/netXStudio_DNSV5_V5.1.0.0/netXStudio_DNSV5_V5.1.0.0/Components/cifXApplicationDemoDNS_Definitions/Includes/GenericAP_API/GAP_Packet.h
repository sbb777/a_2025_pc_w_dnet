/**************************************************************************************
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $Id: GAP_Packet.h 83203 2019-05-21 07:16:02Z ThomasBaranczyk $: *//*!

  \file GAP_Packet.h
  Generic AP Public Packet API.

Changes:
  Date          Description
  -----------------------------------------------------------------------------------
  2018-08-13    created
**************************************************************************************/

#ifndef GAP_PACKET_H_
#define GAP_PACKET_H_

#include <stdint.h>
#include "Hil_Compiler.h"
#include "Hil_Packet.h"

/*! Generic AP commands
 * \ingroup genap_api_packets
 *
 * Command codes for the Requests supported/used by Generic AP.
 * Usually the request/confirmation pair is used by the host application to make the
 * Generic AP to do something whereas the indication/response pair is used by the
 * Generic AP to indicate an event to the host application.
 */
typedef enum GENAP_CMD_Etag
{
  GENAP_GET_COMPONENT_IDS_REQ     = 0x0000AD00,   /**< Get Component IDs - Request */
  GENAP_GET_COMPONENT_IDS_CNF     = 0x0000AD01,   /**< Get Component IDs - Confirmation */
} GENAP_CMD_E;

/******************************************************************************/
/*! \defgroup genap_api_packets Generic AP API packets
 *
 * A summary of all packets received or send by Generic AP.
 *
 * @{*/


/******************************************************************************/
/*! \defgroup GENAP_GET_COMPONENT_IDS_REQ   Get Component IDs request
 *
 * This service shall be used by the host application to read out all
 * component related information details that are registered at GenAP.
 * In case the host application wants to store remanent data, the host application
 * has to iterate over all components that indicate remanent data and generate
 * a HIL_SET_REMANENT_DATA_REQ with the respective Component ID during the
 * configuration phase.
 *
 * \note In case component states remanent data size to be 0 no
 *       HIL_SET_REMANENT_DATA_REQ needs to be created for the component.
 * \note Details regarding remanent data handling are obtained from
 *       \ref HIL_SET_REMANENT_DATA_REQ.
 *
 * @{*/

/*! Get ComponentIDs Request structure. */
typedef HIL_EMPTY_PACKET_T      GENAP_GET_COMPONENT_IDS_REQ_T;

/*! Packet data size. */
#define GENAP_GET_COMPONENT_IDS_REQ_SIZE  (0)

/*! Component Details data structure */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST GENAP_GET_COMPONENT_DETAILS_DATA_Ttag
{
  /*! Component ID */
  uint32_t ulComponentId;
  /*! Remanent Data size in bytes.
   * \note In case of zero the component has no remanent data.  */
  uint32_t ulRemanentDataSize;
  /*! Major version */
  uint16_t usVersionMajor;
  /*! Minor version */
  uint16_t usVersionMinor;
  /*! Build version */
  uint16_t usVersionBuild;
  /*! Revision version */
  uint16_t usVersionRevision;
} GENAP_GET_COMPONENT_DETAILS_DATA_T;

/*! Get ComponentIDs Confirmation data structure */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST GENAP_GET_COMPONENT_IDS_CNF_DATA_Ttag
{
  /*! Number of components in this confirmation */
  uint32_t ulNumberComponents;
  /*! Array of components registered at GenAP */
  GENAP_GET_COMPONENT_DETAILS_DATA_T atComponents[__HIL_VARIABLE_LENGTH_ARRAY];
} GENAP_GET_COMPONENT_IDS_CNF_DATA_T;

/*! Get ComponentIDs Confirmation structure */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST GENAP_GET_COMPONENT_IDS_CNF_Ttag
{
  HIL_PACKET_HEADER_T                 tHead;  /*!< Packet header. */
  GENAP_GET_COMPONENT_IDS_CNF_DATA_T  tData;  /*!< Packet data. */
} GENAP_GET_COMPONENT_IDS_CNF_T;

/*! Packet data size. */
#define GENAP_GET_COMPONENT_IDS_CNF_SIZE  (sizeof(GENAP_GET_COMPONENT_IDS_CNF_DATA_T))

/*! @}*************************************************************************/

/*! @}*************************************************************************/

#endif /* GAP_PACKET_H_ */

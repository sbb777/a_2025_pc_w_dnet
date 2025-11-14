/**************************************************************************************
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
$Id:  $:

Description:
This header provides CIP Hilscher specific object definitions.
 - Hilscher specific object class IDs
 - Hilscher specific object attribute IDs
 - Hilscher specific object services

These definitions contribute to the LFW API of the DeviceNet Slave stack and are
applicable to the DPM packet interface.

**************************************************************************************/
#ifndef __CIP_HILSCHER_DEFINITIONS_H
#define __CIP_HILSCHER_DEFINITIONS_H

/******************************************************************************/
/*               HILSCHER SPECIFIC CIP CLASS ID                               */
/******************************************************************************/
#define CIPHIL_CLASS_IO_MAPPING                                          0x402
#define CIPHIL_CLASS_MN_STATUS                                           0x404


/******************************************************************************/
/*            HILSCHER SPECIFIC CONNECTION CLASS ATTRIBUTES                   */
/******************************************************************************/
#define CIPHIL_CLASS_CONNECTION_ATT_CONSUME_ASSEMBLY                      0x64
#define CIPHIL_CLASS_CONNECTION_ATT_PRODUCE_ASSEMBLY                      0x65


/******************************************************************************/
/*            HILSCHER SPECIFIC IO MAPPING CLASS ATTRIBUTES                   */
/******************************************************************************/
#define CIPHIL_CLASS_IO_MAPPING_ATT_STATUS                                0x01
#define CIPHIL_CLASS_IO_MAPPING_ATT_LENGTH                                0x02
#define CIPHIL_CLASS_IO_MAPPING_ATT_DATA                                  0x03


/******************************************************************************/
/*            HILSCHER SPECIFIC MODULE_NETWORK_STATUS CLASS ATTRIBUTES        */
/******************************************************************************/
#define CIPHIL_CLASS_MNS_ATT_MODULE_STATUS                                0x01
#define CIPHIL_CLASS_MNS_ATT_NETWORK_STATUS                               0x02


/******************************************************************************/
/*            HILSCHER SPECIFIC SERVICES                                      */
/******************************************************************************/
#define CIPHIL_SERVICE_GET_ATTR_OPTION                                  0xFF33
#define CIPHIL_SERVICE_SET_ATTR_OPTION                                  0xFF34
#define CIPHIL_SERVICE_MODIFY_STATUS                                    0x0501

/******************************************************************************/
/*            HILSCHER SERVICE DATA STRUCTURES and DEFINES                    */
/******************************************************************************/

#ifdef __HIL_PRAGMA_PACK_ENABLE
#pragma __HIL_PRAGMA_PACK_1(CIP_COMMON)
#endif

/* Flags for attribute access control */
#define CIP_FLG_SET_ACCESS_MASK     0x00F0  /**< Attribute set access rights                      */
#define CIP_FLG_SET_ACCESS_BUS      0x0010  /**< Attribute can be set by fieldbus                 */
#define CIP_FLG_SET_ACCESS_USER     0x0020  /**< Attribute can be set by user                     */
#define CIP_FLG_SET_ACCESS_ADMIN    0x0040  /**< Attribute is only settable from inside the stack */
#define CIP_FLG_SET_ACCESS_NONE     0x0080  /**< Attribute is not settable                        */

#define CIP_FLG_GET_ACCESS_MASK     0x0F00  /**< Attribute get access rights                      */
#define CIP_FLG_GET_ACCESS_BUS      0x0100  /**< Attribute can be read by fieldbus                */
#define CIP_FLG_GET_ACCESS_USER     0x0200  /**< Attribute can be read by user                    */
#define CIP_FLG_GET_ACCESS_ADMIN    0x0400  /**< Attribute is only readable from inside the stack */
#define CIP_FLG_GET_ACCESS_NONE     0x0800  /**< Attribute is not readable                        */

#define CIP_FLG_TREAT_MASK          0xF000  /**< Attribute treatment policy                       */
#define CIP_FLG_TREAT_FORWARD       0x1000  /**< Service to attribute will be forwarded to user   */
#define CIP_FLG_TREAT_NOTIFY        0x2000  /**< Change of attribute data will be notified to and
                                                 has to be acknowledged by the user               */
#define CIP_FLG_TREAT_DISABLE       0x4000  /**< Attribute is disabled and not accessible         */
#define CIP_FLG_TREAT_PROTECTED     0x8000  /**< Atrribute is protected
                                                 (not settable, when protect mode is active)      */
/* Set attribute option service data */
typedef struct CIPHIL_SET_ATTR_OPTION_Ttag
{
  uint16_t  usMask;
  uint16_t  usValue;
} CIPHIL_SET_ATTR_OPTION_T;

/* Identity object Modify status service data structure */
typedef CIPHIL_SET_ATTR_OPTION_T CIPHIL_MODIFY_STATUS_T;

/* Get attribute option service data */
typedef struct CIPHIL_ATTR_OPTION_Ttag
{
  uint16_t  usOption;
} CIPHIL_ATTR_OPTION_T;

/* Attribute values for the Module Network Status object */
#define  CIPHIL_MNS_MODULE_STATUS_NO_POWER            (0)
#define  CIPHIL_MNS_MODULE_STATUS_SELFTEST            (1)
#define  CIPHIL_MNS_MODULE_STATUS_STANDBY             (2)
#define  CIPHIL_MNS_MODULE_STATUS_OPERATE             (3)
#define  CIPHIL_MNS_MODULE_STATUS_RECOVERABLE_FAULT   (4)
#define  CIPHIL_MNS_MODULE_STATUS_UNRECOVERBLE_FAULT  (5)

#define  CIPHIL_MNS_NETWORK_STATUS_NO_POWER            (0)
#define  CIPHIL_MNS_NETWORK_STATUS_NO_CONNECTION       (1)
#define  CIPHIL_MNS_NETWORK_STATUS_CONNECTED           (2)
#define  CIPHIL_MNS_NETWORK_STATUS_CONNECTION_TIMEOUT  (3)
#define  CIPHIL_MNS_NETWORK_STATUS_CRITICAL_LINK_FAULT (4)


/* Module Network Status Object attribute all data */
typedef struct CIPHIL_CLASS_MNS_ATT_ALL_Ttag
{
  uint32_t  ulModuleStatus;
  uint32_t  ulNetworkStatus;
} CIPHIL_CLASS_MNS_ATT_ALL_T;


#ifdef __HIL_PRAGMA_PACK_ENABLE
#pragma __HIL_PRAGMA_UNPACK_1(CIP_COMMON)
#endif

/******************************************************************************/
#endif /* #ifndef __CIP_HILSCHER_DEFINITIONS_H */

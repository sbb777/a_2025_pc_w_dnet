/**************************************************************************************
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
$Id:  $:

Description:
This header provides CIP common object definitions.
 - Common object class IDs
 - Common object attribute IDs
 - Common object services
 - Common geneneral error codes


These definitions contribute to the LFW API of the DeviceNet Slave stack and are
applicable to the DPM packet interface.

**************************************************************************************/
#ifndef __CIP_COMMON_DEFINITIONS_H
#define __CIP_COMMON_DEFINITIONS_H


/******************************************************************************/
/*                         COMMON CIP CLASS ID                                */
/******************************************************************************/
#define CIP_CLASS_IDENTITY                                                0x01
#define CIP_CLASS_MESSAGE_ROUTER                                          0x02
#define CIP_CLASS_DEVICENET                                               0x03
#define CIP_CLASS_ASSEMBLY                                                0x04
#define CIP_CLASS_CONNECTION                                              0x05
#define CIP_CLASS_PARAMETER                                               0x0F
#define CIP_CLASS_ACKNOWLEDGE_HANDLER                                     0x2B


/******************************************************************************/
/*                     COMMON CIP CLASS ATTRIBUTES                            */
/******************************************************************************/
#define CIP_CLASS_ATT_ALL                                                0x00
#define CIP_CLASS_ATT_REVISION                                           0x01
#define CIP_CLASS_ATT_MAX_INSTANCE                                       0x02
#define CIP_CLASS_ATT_NUMBER_OF_INSTANCES                                0x03
#define CIP_CLASS_ATT_MAXID_CLASS_ATTRIBUTE                              0x06
#define CIP_CLASS_ATT_MAXID_INSTANCE_ATTRIBUTE                           0x07


/******************************************************************************/
/*                 COMMON CIP VENDOR SPECIFIC ATTRIBUTE RANGE                 */
/******************************************************************************/
#define CIP_CLASS_ATT_VENDOR_SPECIFIC_START                              0x64
#define CIP_CLASS_ATT_VENDOR_SPECIFIC_END                                0xC7


/******************************************************************************/
/*                     COMMON CIP IDENTITY CLASS DEFINITION                   */
/******************************************************************************/
#define CIP_CLASS_IDENTITY_ATT_VENDOR_ID                                  0x01
#define CIP_CLASS_IDENTITY_ATT_DEVICE_TYPE                                0x02
#define CIP_CLASS_IDENTITY_ATT_PRODUCT_CODE                               0x03
#define CIP_CLASS_IDENTITY_ATT_REVISION                                   0x04
#define CIP_CLASS_IDENTITY_ATT_STATUS                                     0x05
#define CIP_CLASS_IDENTITY_ATT_SERIAL_NUMBER                              0x06
#define CIP_CLASS_IDENTITY_ATT_PRODUCT_NAME                               0x07


/******************************************************************************/
/*                     CIP CLASS IDENTITY ATT STATUS DEFINITIONS              */
/******************************************************************************/
#define CIP_CLASS_IDENTITY_ATT_STATUS_CONFIGURED                          0x0004

#define CIP_CLASS_IDENTITY_ATT_STATUS_FAULT_MASK                          0x0F00
#define CIP_CLASS_IDENTITY_ATT_STATUS_FAULT_MINOR_RECOVERABLE             0x0100
#define CIP_CLASS_IDENTITY_ATT_STATUS_FAULT_MINOR_UNRECOVERABLE           0x0200

#define CIP_CLASS_IDENTITY_ATT_STATUS_FAULT_MAJOR_RECOVERABLE             0x0400
#define CIP_CLASS_IDENTITY_ATT_STATUS_FAULT_MAJOR_UNRECOVERABLE           0x0800

#define CIP_CLASS_IDENTITY_ATT_STATUS_EXT_DEVICE_STATE_MASK               0x00F0


/******************************************************************************/
/*                COMMON CIP MESSAGE ROUTER CLASS ATTRIBUTE                   */
/******************************************************************************/
#define CIP_CLASS_MESSAGE_ROUTER_ATT_NUMBER_AVAILABLE                     0x02


/******************************************************************************/
/*                     COMMON CIP DEVICENET CLASS ATTRIBUTE                   */
/******************************************************************************/
#define CIP_CLASS_DEVICENET_ATT_MACID                                     0x01
#define CIP_CLASS_DEVICENET_ATT_BAUDRATE                                  0x02
#define CIP_CLASS_DEVICENET_ATT_BOI                                       0x03
#define CIP_CLASS_DEVICENET_ATT_BUS_OFF_COUNTER                           0x04
#define CIP_CLASS_DEVICENET_ATT_ALLOCATION_INFO                           0x05
#define CIP_CLASS_DEVICENET_ATT_MACID_SWITCH_CHANGED                      0x06
#define CIP_CLASS_DEVICENET_ATT_BAUDRATE_SWITCH_CHANGED                   0x07
#define CIP_CLASS_DEVICENET_ATT_MACID_SWITCH_VALUE                        0x08
#define CIP_CLASS_DEVICENET_ATT_BAUDRATE_SWITCH_VALUE                     0x09


/******************************************************************************/
/*                   COMMON CIP CONNECTION CLASS DEFINITION                   */
/******************************************************************************/
#define CIP_CLASS_CONNECTION_ATT_STATE                                    0x01
#define CIP_CLASS_CONNECTION_ATT_INSTANCE_TYPE                            0x02
#define CIP_CLASS_CONNECTION_ATT_TRANSPORTCLASS_TRIGGER                   0x03
#define CIP_CLASS_CONNECTION_ATT_DEVICENET_PRODUCED_CONNECTION_ID         0x04
#define CIP_CLASS_CONNECTION_ATT_DEVICENET_CONSUMED_CONNECTION_ID         0x05
#define CIP_CLASS_CONNECTION_ATT_DEVICENET_INITIAL_COMM_CHARACTERISTICS   0x06
#define CIP_CLASS_CONNECTION_ATT_PRODUCED_CONNECTION_SIZE                 0x07
#define CIP_CLASS_CONNECTION_ATT_CONSUMED_CONNECTION_SIZE                 0x08
#define CIP_CLASS_CONNECTION_ATT_EXPECTED_PACKET_RATE                     0x09
#define CIP_CLASS_CONNECTION_ATT_CIP_PRODUCED_CONNECTION_ID               0x0A
#define CIP_CLASS_CONNECTION_ATT_CIP_CONSUMED_CONNECTION_ID               0x0B
#define CIP_CLASS_CONNECTION_ATT_WATCHDOG_TIMEOUT_ACTION                  0x0C
#define CIP_CLASS_CONNECTION_ATT_PRODUCED_CONNECTION_PATH_LENGTH          0x0D
#define CIP_CLASS_CONNECTION_ATT_PRODUCED_CONNECTION_PATH                 0x0E
#define CIP_CLASS_CONNECTION_ATT_CONSUMED_CONNECTION_PATH_LENGTH          0x0F
#define CIP_CLASS_CONNECTION_ATT_CONSUMED_CONNECTION_PATH                 0x10
#define CIP_CLASS_CONNECTION_ATT_PRODUCTION_INHIBIT_TIME                  0x11
#define CIP_CLASS_CONNECTION_ATT_CONNECTION_TIMEOUT_MULTIPLIER            0x12
#define CIP_CLASS_CONNECTION_ATT_CONNECTION_BINDING_LIST                  0x13


/******************************************************************************/
/*                     COMMON CIP DEVICENET CLASS ATTRIBUTE                   */
/******************************************************************************/
#define CIP_CLASS_ASSEMBLY_ATT_NUMBER_OF_MEMBER                           0x01
#define CIP_CLASS_ASSEMBLY_ATT_MEMBER_LIST                                0x02
#define CIP_CLASS_ASSEMBLY_ATT_DATA                                       0x03
#define CIP_CLASS_ASSEMBLY_ATT_SIZE                                       0x04


/******************************************************************************/
/*                         COMMON CIP SERVICE CODE                            */
/******************************************************************************/
#define CIP_SERVICE_GET_ATTRIBUTES_ALL                                    0x01
#define CIP_SERVICE_SET_ATTRIBUTES_ALL                                    0x02
#define CIP_SERVICE_GET_ATTRIBUTE_LIST                                    0x03
#define CIP_SERVICE_SET_ATTRIBUTE_LIST                                    0x04
#define CIP_SERVICE_RESET                                                 0x05
#define CIP_SERVICE_START                                                 0x06
#define CIP_SERVICE_STOP                                                  0x07
#define CIP_SERVICE_CREATE                                                0x08
#define CIP_SERVICE_DELETE                                                0x09
#define CIP_SERVICE_MULTIPLE_SERVICE_PACKET                               0x0A
#define CIP_SERVICE_APPLY_ATTRIBUTES                                      0x0D
#define CIP_SERVICE_GET_ATTRIBUTE_SINGLE                                  0x0E
#define CIP_SERVICE_SET_ATTRIBUTE_SINGLE                                  0x10
#define CIP_SERVICE_FIND_NEXT_OBJECT_INSTANCE                             0x11
#define CIP_SERVICE_ERROR_RESPONSE                                        0x14
#define CIP_SERVICE_RESTORE                                               0x15
#define CIP_SERVICE_SAVE                                                  0x16
#define CIP_SERVICE_NO_OPERATION                                          0x17
#define CIP_SERVICE_GET_MEMBER                                            0x18
#define CIP_SERVICE_SET_MEMBER                                            0x19
#define CIP_SERVICE_INSERT_MEMBER                                         0x1A
#define CIP_SERVICE_REMOVE_MEMBER                                         0x1B
#define CIP_SERVICE_GROUPSYNC                                             0x1C
/* Reserved Service for future use 0x0B,0x0C,0x0F,0x12,13,0x1D..0x31        */


/******************************************************************************/
/*                      COMMON CIP GENERAL ERROR CODE                         */
/******************************************************************************/
/* Service was successfully performed by the object specified.                */
#define CIP_GRC_SUCCESS                                               0x00

/* A connection related service failed along the connection path.             */
#define CIP_GRC_CONNECTION_FAILURE                                    0x01

/* Resources needed for the object to perform the service were unavailable    */
#define CIP_GRC_RESOURCE_UNAVAILABLE                                  0x02

/* See Status Code 0x20,  preferred value to use for this condition           */
#define CIP_GRC_INVALID_PARAMETER_VALUE                               0x03

/* The path segment identifier or the segment syntax was not understood by the*
   processing node. Path processing stops on a path segment error             */
#define CIP_GRC_PATH_SEGMENT_ERROR                                    0x04

/* The path is referencing an object class, instance or structure element that*
 * is not known or is not contained in the processing node. Path processing   *
 * shall stop when a path destination unknown error is encountered.           */
#define CIP_GRC_PATH_DESTINATION_UNKNOWN                              0x05

/* Only part of the expected data was transferred.                            */
#define CIP_GRC_PARTIAL_TRANSFER                                      0x06

/* The messaging connection was lost.                                         */
#define CIP_GRC_CONNECTION_LOST                                       0x07

/* The requested service was not implemented/defined for this Class/Instance  */
#define CIP_GRC_SERVICE_NOT_SUPPORTED                                 0x08

/* Invalid attribute data detected                                            */
#define CIP_GRC_INVALID_ATTRIBUTE_VALUE                               0x09

/* An attribute in the Get/Set_Attribute_List response has a non-zero status  */
#define CIP_GRC_ATTRIBUTE_LIST_ERROR                                  0x0A

/* The object is already in the mode/state being requested by the service     */
#define CIP_GRC_ALREADY_IN_REQUESTED_STATE                            0x0B

/* The object cannot perform the requested service in its current mode/state  */
#define CIP_GRC_OBJECT_STATE_CONFLICT                                 0x0C

/* The requested instance of object to be created already exists              */
#define CIP_GRC_OBJECT_ALREADY_EXISTS                                 0x0D

/* A request to modify a non-modifiable attribute was received                */
#define CIP_GRC_ATTRIBUTE_NOT_SETTABLE                                0x0E

/* A permission/privilege check failed                                        */
#define CIP_GRC_PRIVILEGE_VIOLATION                                   0x0F

/* The device’s current mode/state prohibits the execution of the service     */
#define CIP_GRC_DEVICE_STATE_CONFLICT                                 0x10

/* The data to be transmitted in the response buffer is larger than the       *
 * allocated response buffer                                                  */
#define CIP_GRC_REPLY_DATA_TOO_LARGE                                  0x11

/* The service specified an operation that is going to fragment a primitive   *
 * data value, i.e. half a REAL data type.                                    */
#define CIP_GRC_FRAG_OF_A_PRIMITIVE_VALUE                             0x12

/* The service did not supply enough data to perform the specified operation. */
#define CIP_GRC_NOT_ENOUGH_DATA                                       0x13

/* The attribute specified in the request is not supported                    */
#define CIP_GRC_ATTRIBUTE_NOT_SUPPORTED                               0x14

/* The service supplied more data than was expected                           */
#define CIP_GRC_TOO_MUCH_DATA                                         0x15

/* The object specified does not exist in the device                          */
#define CIP_GRC_OBJECT_DOES_NOT_EXIST                                 0x16

/* The frag sequence for this service is not currently active for this data   */
#define CIP_GRC_SRV_FRAG_SEQ_NOT_IN_PROGRESS                          0x17

/* The attribute data of this object was not saved prior to the service       */
#define CIP_GRC_NO_STORED_ATTRIBUTE_DATA                              0x18

/*The attr data of this object was not saved due to failure during the attempt*/
#define CIP_GRC_STORE_OPERATION_FAILURE                               0x19

/*The service request packet was too large for transmission on a network in   *
 *the path to the destination. The routing dev was forced aborting the service*/
#define CIP_GRC_ROUTING_FAIL_REQ_PCK_TOO_BIG                          0x1A

/* The service response packet was too large for transmission on a network in *
 * the path from the destination. The routing device was forced aborting      *
 * the service.                                                               */
#define CIP_GRC_ROUTING_FAIL_RES_PCK_TOO_BIG                          0x1B

/* The service did not supply an attribute in a list of attributes that       *
 * was needed by the service to perform the requested behavior.               */
#define CIP_GRC_MISSING_ATTR_LIST_ENTRY_DATA                          0x1C

/* The service is returning the list of attributes supplied with status       *
   information for those attributes that were invalid                         */
#define CIP_GRC_INVALID_ATTR_VALUE_LIST                               0x1D

/* An embedded service resulted in an error                                   */
#define CIP_GRC_EMBEDDED_SERVICE_ERROR                                0x1E

/* A vendor specific error has been encountered. The Additional Code Field of *
 * the Error Response defines the particular error encountered. Use of this   *
 * General Error Code should only be performed when none of the Error Codes   *
 * presented in this table or within an Object Class definition accurately    *
 * reflect the error.                                                         */
#define CIP_GRC_VENDOR_SPECIFIC_ERROR                                 0x1F

/* A parameter associated with the request was invalid. This code is used when*
 * a parameter does not meet the requirements of this specification and/or the*
 * requirements defined in an Application Object Specification.               */
#define CIP_GRC_INVALID_PARAMETER                                     0x20

/* An attempt was made to write to a write-once medium (e.g. WORM drive, PROM)*
 * that has already been written, or to modify a value that cannot be changed *
 * once established.                                                          */
#define CIP_GRC_WRITE_ONCE_VALUE_OR_MEDIUM                            0x21

/* An invalid reply is received (e.g. reply service code does not match the   *
 * request service code, or reply message is shorter than the minimum expected*
 * reply size). This status code can serve for other causes of invalid replies*/
#define CIP_GRC_INVALID_REPLY_RECEIVED                                0x22

/* The message received is larger than the receiving buffer can handle.       *
 * The entire message was discarded                                           */
#define CIP_GRC_BUFFER_OVERFLOW                                       0x23

/* The format of the received message is not supported by the server          */
#define CIP_GRC_MESSAGE_FORMAT_ERROR                                  0x24

/* The Key Segment that was included as the first segment in the path does not*
 * match the destination module. The object specific status shall indicate    *
 * which part of the key check failed                                         */
#define CIP_GRC_KEY_FAILURE_IN_PATH                                   0x25

/* The size of the path which was sent with the Service Request is either not *
 * large enough to allow the Request to be routed to an object or too         *
 * much routing data was included.                                            */
#define CIP_GRC_PATH_SIZE_INVALID                                     0x26

/* An attempt was made to set an attr that is not able to be set at this time */
#define CIP_GRC_UNEXPECTED_ATTRIBUTE_IN_LIST                          0x27

/* The Member ID specified in the request does not exist in the specified     *
 * Class/Instance/Attribute                                                   */
#define CIP_GRC_INVALID_MEMBER_ID                                     0x28

/* A request to modify a non-modifiable member was received                   */
#define CIP_GRC_MEMBER_NOT_SETTABLE                                   0x29

/* This error code may only be reported by DeviceNet Group 2 Only servers with*
 * 4K or less code space and only in place of Service not supported,          *
 * Attribute not supported and Attribute not settable                         */
#define CIP_GRC_GROUP_2_ONLY_SERVER_GEN_FAIL                          0x2A

/* A CIP to Modbus translator received an unknown Modbus Exception Code       */
#define CIP_GRC_UNKNOWN_MODBUS_ERROR                                  0x2B

/* A request to read a non-readable attribute was received                    */
#define CIP_GRC_ATTRIBUTE_NOT_GETTABLE                                0x2C

/* Reserved by CIP for future extensions            2D - CF
   This range of error codes is to be used to indicate Object Class specific
   errors. Use of this range should only be performed when none of the Error
   Codes presented in this table accurately reflect the error */
/* Reserved for Object Class and service errors     D0 - FF */

/******************************************************************************/
#endif /* #ifndef __CIP_COMMON_DEFINITIONS_H */

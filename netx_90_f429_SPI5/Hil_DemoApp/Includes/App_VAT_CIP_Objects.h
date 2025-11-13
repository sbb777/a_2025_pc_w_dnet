#ifndef APP_VAT_CIP_OBJECTS_H_
#define APP_VAT_CIP_OBJECTS_H_

#include <stdint.h>

/******************************************************************************
 * VAT CIP CLASS DEFINITIONS
 ******************************************************************************/

/* Standard CIP Classes */
#define CIP_CLASS_IDENTITY          0x01
#define CIP_CLASS_MESSAGE_ROUTER    0x02
#define CIP_CLASS_DEVICENET         0x03
#define CIP_CLASS_ASSEMBLY          0x04
#define CIP_CLASS_CONNECTION        0x05

/* VAT Custom Classes */
#define VAT_CLASS_PARAMETER         0x64  /* 100 - Parameter Object */
#define VAT_CLASS_DIAGNOSTIC        0x65  /* 101 - Diagnostic Object */
#define VAT_CLASS_CALIBRATION       0x66  /* 102 - Calibration Object */
#define VAT_CLASS_VALVE_CONTROL     0x67  /* 103 - Valve Control Object */

/******************************************************************************
 * CIP SERVICE CODES
 ******************************************************************************/
#define CIP_SERVICE_GET_ATTRIBUTES_ALL      0x01
#define CIP_SERVICE_SET_ATTRIBUTES_ALL      0x02
#define CIP_SERVICE_GET_ATTRIBUTE_LIST      0x03
#define CIP_SERVICE_SET_ATTRIBUTE_LIST      0x04
#define CIP_SERVICE_RESET                   0x05
#define CIP_SERVICE_START                   0x06
#define CIP_SERVICE_STOP                    0x07
#define CIP_SERVICE_CREATE                  0x08
#define CIP_SERVICE_DELETE                  0x09
#define CIP_SERVICE_GET_ATTRIBUTE_SINGLE    0x0E
#define CIP_SERVICE_SET_ATTRIBUTE_SINGLE    0x10

/******************************************************************************
 * CIP GENERAL RESPONSE CODES
 ******************************************************************************/
#define CIP_GRC_SUCCESS                     0x00
#define CIP_GRC_CONNECTION_FAILURE          0x01
#define CIP_GRC_RESOURCE_UNAVAILABLE        0x02
#define CIP_GRC_INVALID_PARAMETER           0x03
#define CIP_GRC_PATH_SEGMENT_ERROR          0x04
#define CIP_GRC_PATH_DESTINATION_UNKNOWN    0x05
#define CIP_GRC_PARTIAL_TRANSFER            0x06
#define CIP_GRC_CONNECTION_LOST             0x07
#define CIP_GRC_SERVICE_NOT_SUPPORTED       0x08
#define CIP_GRC_INVALID_ATTRIBUTE_VALUE     0x09
#define CIP_GRC_ATTRIBUTE_LIST_ERROR        0x0A
#define CIP_GRC_ALREADY_IN_REQUESTED_MODE   0x0B
#define CIP_GRC_OBJECT_STATE_CONFLICT       0x0C
#define CIP_GRC_OBJECT_ALREADY_EXISTS       0x0D
#define CIP_GRC_ATTRIBUTE_NOT_SETTABLE      0x0E
#define CIP_GRC_PRIVILEGE_VIOLATION         0x0F
#define CIP_GRC_DEVICE_STATE_CONFLICT       0x10
#define CIP_GRC_REPLY_DATA_TOO_LARGE        0x11
#define CIP_GRC_FRAGMENT_PRIMITIVE          0x12
#define CIP_GRC_NOT_ENOUGH_DATA             0x13
#define CIP_GRC_ATTRIBUTE_NOT_SUPPORTED     0x14
#define CIP_GRC_TOO_MUCH_DATA               0x15
#define CIP_GRC_OBJECT_DOES_NOT_EXIST       0x16
#define CIP_GRC_PERMISSION_DENIED           0x26

/******************************************************************************
 * VAT PARAMETER OBJECT (Class 0x64) ATTRIBUTES
 ******************************************************************************/
#define VAT_PARAM_ATTR_PRESSURE_SETPOINT    1   /* INT, R/W, Pressure setpoint */
#define VAT_PARAM_ATTR_POSITION_SETPOINT    2   /* INT, R/W, Position setpoint */
#define VAT_PARAM_ATTR_CONTROLLER_MODE      3   /* USINT, R/W, Controller mode */
#define VAT_PARAM_ATTR_CONTROL_INSTANCE     4   /* USINT, R/W, Control instance */
#define VAT_PARAM_ATTR_DEVICE_STATUS        5   /* USINT, R, Device status */
#define VAT_PARAM_ATTR_EXCEPTION_STATUS     6   /* USINT, R, Exception status */
#define VAT_PARAM_ATTR_ACCESS_MODE          7   /* USINT, R/W, Access mode */
#define VAT_PARAM_ATTR_CURRENT_PRESSURE     8   /* INT, R, Current pressure */
#define VAT_PARAM_ATTR_PRESSURE_UNITS       9   /* UINT, R/W, Pressure units */
#define VAT_PARAM_ATTR_POSITION_UNITS       10  /* UINT, R/W, Position units */
#define VAT_PARAM_ATTR_CURRENT_POSITION     11  /* INT, R, Current position */
#define VAT_PARAM_ATTR_AUTO_LEARN           12  /* USINT, R/W, Auto learn enable */
#define VAT_PARAM_ATTR_CALIB_SCALE          13  /* USINT, R/W, Calibration scale */
#define VAT_PARAM_ATTR_ZERO_ADJUST          14  /* USINT, W, Zero adjust */
#define VAT_PARAM_ATTR_VALVE_ADDRESS        15  /* USINT, R/W, Valve address */
#define VAT_PARAM_ATTR_VALVE_ACTION         16  /* USINT, W, Valve action */

/******************************************************************************
 * VAT DIAGNOSTIC OBJECT (Class 0x65) ATTRIBUTES
 ******************************************************************************/
#define VAT_DIAG_ATTR_UPTIME                1   /* UDINT, R, Uptime in seconds */
#define VAT_DIAG_ATTR_TOTAL_CYCLES          2   /* UDINT, R, Total cycles */
#define VAT_DIAG_ATTR_ERROR_COUNT           3   /* UINT, R, Error count */
#define VAT_DIAG_ATTR_LAST_ERROR_CODE       4   /* UINT, R, Last error code */
#define VAT_DIAG_ATTR_LAST_ERROR_TIMESTAMP  5   /* UDINT, R, Last error timestamp */
#define VAT_DIAG_ATTR_PRESSURE_MIN          6   /* INT, R, Minimum pressure */
#define VAT_DIAG_ATTR_PRESSURE_MAX          7   /* INT, R, Maximum pressure */
#define VAT_DIAG_ATTR_PRESSURE_AVG          8   /* INT, R, Average pressure */
#define VAT_DIAG_ATTR_POSITION_MIN          9   /* INT, R, Minimum position */
#define VAT_DIAG_ATTR_POSITION_MAX          10  /* INT, R, Maximum position */
#define VAT_DIAG_ATTR_NETWORK_RX_COUNT      11  /* UDINT, R, Network RX count */
#define VAT_DIAG_ATTR_NETWORK_TX_COUNT      12  /* UDINT, R, Network TX count */
#define VAT_DIAG_ATTR_NETWORK_ERROR_COUNT   13  /* UINT, R, Network error count */
#define VAT_DIAG_ATTR_TEMPERATURE           14  /* INT, R, Temperature (Celsius) */
#define VAT_DIAG_ATTR_FIRMWARE_VERSION      15  /* UDINT, R, Firmware version */

/******************************************************************************
 * ATTRIBUTE ACCESS FLAGS
 ******************************************************************************/
#define ATTR_ACCESS_READ_ONLY               0x01
#define ATTR_ACCESS_WRITE_ONLY              0x02
#define ATTR_ACCESS_READ_WRITE              0x03

#endif /* APP_VAT_CIP_OBJECTS_H_ */

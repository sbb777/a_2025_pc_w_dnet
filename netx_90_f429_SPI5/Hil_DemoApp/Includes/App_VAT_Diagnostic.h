#ifndef APP_VAT_DIAGNOSTIC_H_
#define APP_VAT_DIAGNOSTIC_H_

#include <stdint.h>
#include <stdbool.h>
#include "AppDNS_DemoApplication.h"
#include "DNS_Includes.h"

/******************************************************************************
 * DIAGNOSTIC DATA STRUCTURE
 ******************************************************************************/
typedef struct VAT_DIAGNOSTIC_DATA_Ttag {
    /* System Statistics */
    uint32_t ulUptimeSeconds;           /* Uptime in seconds */
    uint32_t ulTotalCycles;             /* Total control cycles */
    uint16_t usErrorCount;              /* Total error count */
    uint16_t usLastErrorCode;           /* Last error code */
    uint32_t ulLastErrorTimestamp;      /* Last error timestamp (seconds) */

    /* Pressure Statistics */
    int16_t sPressureMin;               /* Minimum pressure recorded */
    int16_t sPressureMax;               /* Maximum pressure recorded */
    int16_t sPressureAvg;               /* Average pressure (recent 100 samples) */
    int32_t lPressureSum;               /* Sum for average calculation */
    uint16_t usPressureSampleCount;     /* Sample count for average */

    /* Position Statistics */
    int16_t sPositionMin;               /* Minimum position recorded */
    int16_t sPositionMax;               /* Maximum position recorded */

    /* Network Statistics */
    uint32_t ulNetworkRxCount;          /* Network RX packet count */
    uint32_t ulNetworkTxCount;          /* Network TX packet count */
    uint16_t usNetworkErrorCount;       /* Network error count */

    /* System Status */
    int16_t sTemperature;               /* System temperature (Celsius) */
    uint32_t ulFirmwareVersion;         /* Firmware version (0xMMmmppbb) */
} VAT_DIAGNOSTIC_DATA_T;

/******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/
extern VAT_DIAGNOSTIC_DATA_T g_tVatDiagnostics;

/******************************************************************************
 * FUNCTION PROTOTYPES
 ******************************************************************************/

/**
 * @brief Initialize diagnostic data structure
 */
void VAT_Diagnostic_Init(void);

/**
 * @brief Update diagnostic statistics (called periodically)
 *
 * Should be called every 100ms to update statistics
 */
void VAT_Diagnostic_Update(void);

/**
 * @brief Record an error event
 *
 * @param usErrorCode  Error code to record
 */
void VAT_Diagnostic_RecordError(uint16_t usErrorCode);

/**
 * @brief Reset diagnostic statistics
 *
 * Clears all statistics except firmware version
 */
void VAT_Diagnostic_Reset(void);

/**
 * @brief Handle CIP service requests for Diagnostic Object
 *
 * @param ptDnsRsc        DNS channel resource
 * @param ptInd           CIP service indication
 * @param ptRes           CIP service response (output)
 * @param pulResDataSize  Response data size (output)
 * @return CIP General Response Code (GRC)
 */
uint32_t VAT_Diagnostic_HandleService(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    DNS_PACKET_CIP_SERVICE_IND_T*  ptInd,
    DNS_PACKET_CIP_SERVICE_RES_T*  ptRes,
    uint32_t*                      pulResDataSize);

#endif /* APP_VAT_DIAGNOSTIC_H_ */

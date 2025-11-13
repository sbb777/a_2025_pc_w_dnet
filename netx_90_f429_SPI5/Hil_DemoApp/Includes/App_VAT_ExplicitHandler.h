#ifndef APP_VAT_EXPLICIT_HANDLER_H_
#define APP_VAT_EXPLICIT_HANDLER_H_

#include <stdint.h>
#include <stdbool.h>
#include "AppDNS_DemoApplication.h"
#include "DNS_Includes.h"

/******************************************************************************
 * EXPLICIT MESSAGE HANDLER - MAIN ENTRY POINTS
 ******************************************************************************/

/**
 * @brief Handle CIP Service Request (Direct integration with APP_DATA_T)
 *
 * @param ptInd      CIP service indication packet
 * @param ptRes      CIP service response packet
 * @param ptAppData  Application data structure
 * @return true if handled successfully
 */
bool VAT_Explicit_HandleCipService_Direct(
    DNS_PACKET_CIP_SERVICE_IND_T* ptInd,
    DNS_PACKET_CIP_SERVICE_RES_T* ptRes,
    APP_DATA_T* ptAppData);

/**
 * @brief Handle CIP Service Request (DNS_CMD_CIP_SERVICE_IND)
 *        Legacy interface for APP_DNS_CHANNEL_HANDLER_RSC_T
 *
 * @param ptDnsRsc   DNS channel resource
 * @param ptInd      CIP service indication packet
 * @return true if handled successfully
 */
bool VAT_Explicit_HandleCipService(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    DNS_PACKET_CIP_SERVICE_IND_T*  ptInd);

/**
 * @brief Handle Explicit Read Request (DNS_CMD_EXPLICIT_READ_IND)
 *
 * @param ptDnsRsc   DNS channel resource
 * @param ptPacket   Packet structure
 * @return true if handled successfully
 */
bool VAT_Explicit_HandleRead(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    CIFX_PACKET* ptPacket);

/**
 * @brief Handle Explicit Write Request (DNS_CMD_EXPLICIT_WRITE_IND)
 *
 * @param ptDnsRsc   DNS channel resource
 * @param ptPacket   Packet structure
 * @return true if handled successfully
 */
bool VAT_Explicit_HandleWrite(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    CIFX_PACKET* ptPacket);

/******************************************************************************
 * CLASS-SPECIFIC SERVICE HANDLERS
 ******************************************************************************/

/**
 * @brief Handle VAT Parameter Object (Class 0x64) services
 *
 * @param ptDnsRsc        DNS channel resource
 * @param ptInd           CIP service indication
 * @param ptRes           CIP service response (output)
 * @param pulResDataSize  Response data size (output)
 * @return CIP General Response Code (GRC)
 */
uint32_t VAT_Parameter_HandleService(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    DNS_PACKET_CIP_SERVICE_IND_T*  ptInd,
    DNS_PACKET_CIP_SERVICE_RES_T*  ptRes,
    uint32_t*                      pulResDataSize);

/**
 * @brief Handle VAT Diagnostic Object (Class 0x65) services
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

/**
 * @brief Handle Assembly Object (Class 0x04) services
 *
 * @param ptDnsRsc        DNS channel resource
 * @param ptInd           CIP service indication
 * @param ptRes           CIP service response (output)
 * @param pulResDataSize  Response data size (output)
 * @return CIP General Response Code (GRC)
 */
uint32_t VAT_Assembly_HandleService(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    DNS_PACKET_CIP_SERVICE_IND_T*  ptInd,
    DNS_PACKET_CIP_SERVICE_RES_T*  ptRes,
    uint32_t*                      pulResDataSize);

/******************************************************************************
 * PARAMETER HELPER FUNCTIONS
 ******************************************************************************/

/**
 * @brief Get all attributes of a parameter instance
 *
 * @param bInstance       Parameter instance (1-99)
 * @param ptRes           Response packet
 * @param pulResDataSize  Response data size (output)
 * @return CIP General Response Code (GRC)
 */
uint32_t VAT_Parameter_GetAll(
    uint8_t bInstance,
    DNS_PACKET_CIP_SERVICE_RES_T* ptRes,
    uint32_t* pulResDataSize);

/**
 * @brief Reset parameter to default value
 *
 * @param bInstance  Parameter instance (1-99)
 * @return CIP General Response Code (GRC)
 */
uint32_t VAT_Parameter_Reset(uint8_t bInstance);

/******************************************************************************
 * SYNCHRONIZATION FUNCTIONS
 ******************************************************************************/

/**
 * @brief Synchronize Parameters to I/O Data
 *
 * Called periodically to sync parameter changes to Output Assembly
 */
void VAT_Sync_ParametersToIoData(void);

/**
 * @brief Synchronize I/O Data to Parameters
 *
 * Called periodically to update read-only parameters from Input Assembly
 */
void VAT_Sync_IoDataToParameters(void);

#endif /* APP_VAT_EXPLICIT_HANDLER_H_ */

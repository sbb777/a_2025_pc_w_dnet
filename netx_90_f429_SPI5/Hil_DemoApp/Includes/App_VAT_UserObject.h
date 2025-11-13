/**************************************************************************************
 * VAT User Object Header
 *
 * Public interface for VAT-specific CIP objects for Explicit Message communication
 * Based on Hilscher's AppDNS_DemoApplication_UserObject.h pattern
 **************************************************************************************/

#ifndef APP_VAT_USEROBJECT_H
#define APP_VAT_USEROBJECT_H

#include <stdint.h>
#include <stdbool.h>
#include "DNS_Includes.h"
#include "App_DemoApplication.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
 * VAT Object Class Definitions
 ***************************************************************************************************/

/* VAT Parameter Object (Class 0x64) */
#define VAT_CLASS_PARAMETER                     0x64
#define VAT_CLASS_PARAMETER_MAX_INST            99      /* 99 parameters */

/* VAT Diagnostic Object (Class 0x65) */
#define VAT_CLASS_DIAGNOSTIC                    0x65
#define VAT_CLASS_DIAGNOSTIC_MAX_INST           1       /* Single instance */

/***************************************************************************************************
 * Public Function Prototypes
 ***************************************************************************************************/

/**
 * @brief Register all VAT objects at the stack at startup
 *
 * Registers VAT Parameter Object (Class 0x64) and VAT Diagnostic Object (Class 0x65)
 * with the DeviceNet stack. This function should be called during application initialization.
 *
 * @param ptAppData     Pointer to application data structure
 * @return              CIFX_NO_ERROR on success, error code otherwise
 */
uint32_t AppDNS_VAT_UserObject_Registration(APP_DATA_T* ptAppData);

/**
 * @brief Check if a requested object is a registered VAT user object
 *
 * Validates that the requested Class and Instance are within the registered VAT object range.
 *
 * @param ulClass       CIP Class ID (should be 0x64 or 0x65 for VAT objects)
 * @param ulInstance    CIP Instance number
 * @return              true if object is registered, false otherwise
 */
bool AppDNS_VAT_UserObject_IsRegistered(uint32_t ulClass, uint32_t ulInstance);

/**
 * @brief VAT User Object Indication Handler - Main Entry Point
 *
 * This function handles all incoming CIP requests directed to VAT objects
 * (Class 0x64 Parameter and Class 0x65 Diagnostic) that have been registered
 * by the host application.
 *
 * Supported services:
 * - Parameter Object (0x64):
 *   - GET_ATTRIBUTE_SINGLE (0x0E): Read parameter value
 *   - SET_ATTRIBUTE_SINGLE (0x10): Write parameter value (RW only)
 *
 * - Diagnostic Object (0x65):
 *   - GET_ATTRIBUTE_SINGLE (0x0E): Read diagnostic attribute
 *   - RESET (0x05): Reset diagnostic statistics
 *
 * @param ptAppData         Pointer to application data structure
 * @param ptInd             Pointer to CIP service indication packet
 * @param ptRes             Pointer to CIP service response packet (output)
 * @param pulResDataSize    Pointer to response data size (output)
 */
void AppDNS_VAT_UserObject_Indication(APP_DATA_T* ptAppData,
                                       DNS_PACKET_CIP_SERVICE_IND_T* ptInd,
                                       DNS_PACKET_CIP_SERVICE_RES_T* ptRes,
                                       uint32_t* pulResDataSize);

#ifdef __cplusplus
}
#endif

#endif /* APP_VAT_USEROBJECT_H */

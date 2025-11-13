#ifndef APPDNS_EXPLICITMSG_H_
#define APPDNS_EXPLICITMSG_H_

#include <stdint.h>

/******************************************************************************
 * CIP EXPLICIT MESSAGE HANDLER
 ******************************************************************************/

/* CIP Error Codes */
#define CIP_ERROR_SUCCESS                   0x00
#define CIP_ERROR_INVALID_MESSAGE           0x04
#define CIP_ERROR_SERVICE_NOT_SUPPORTED     0x08
#define CIP_ERROR_INVALID_ATTRIBUTE         0x09
#define CIP_ERROR_ATTRIBUTE_NOT_SETTABLE    0x0E
#define CIP_ERROR_INVALID_SIZE              0x15

/* Main CIP Message Processor (parses request and generates response) */
uint8_t CIP_ProcessExplicitMessage(const uint8_t* pRequest, uint32_t reqLen,
                                    uint8_t* pResponse, uint32_t* pRespLen);

/* Main explicit message handler */
uint8_t CIP_HandleExplicitMessage(uint8_t service_code, uint8_t class_id, uint8_t instance_id,
                                   uint8_t attribute_id, const uint8_t* pRequestData, uint8_t requestSize,
                                   uint8_t* pResponseData, uint16_t* pResponseSize);

/* Individual service handlers */
uint8_t CIP_HandleGetAttributeSingle(uint8_t class_id, uint8_t instance_id, uint8_t attribute_id,
                                      uint8_t* pResponse, uint8_t* pResponseSize);

uint8_t CIP_HandleSetAttributeSingle(uint8_t class_id, uint8_t instance_id, uint8_t attribute_id,
                                      const uint8_t* pData, uint8_t dataSize);

uint8_t CIP_HandleGetAttributeAll(uint8_t class_id, uint8_t instance_id,
                                   uint8_t* pResponse, uint16_t* pResponseSize);

uint8_t CIP_HandleSave(void);
uint8_t CIP_HandleReset(void);

#endif /* APPDNS_EXPLICITMSG_H_ */

/**
 * @file AppDNS_DemoApplication.h
 * @brief DeviceNet Demo Application Header
 * @date 2025-01-13
 *
 * This file contains the main DeviceNet application interface.
 */

#ifndef APPDNS_DEMOAPPLICATION_H
#define APPDNS_DEMOAPPLICATION_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Initialize DeviceNet Demo Application
 * @return 0 on success, negative on error
 */
int AppDNS_DemoApplication_Init(void);

/**
 * @brief Run DeviceNet Demo Application main loop
 * @return 0 on success, negative on error
 */
int AppDNS_DemoApplication_Run(void);

/**
 * @brief Get DeviceNet initialization status
 * @return 1 if initialized, 0 otherwise
 */
uint8_t AppDNS_GetInitStatus(void);

/**
 * @brief Get stub mode counter (debug)
 * @return Current counter value (stub mode only)
 */
uint32_t AppDNS_GetStubCounter(void);

/**
 * @brief Set input data (data to send to Master)
 * @param pData Pointer to data buffer
 * @param len Length of data (max DEVICENET_INPUT_SIZE)
 * @return 0 on success, negative on error
 * @note Only available in FULL mode (ENABLE_DEVICENET=1)
 */
int AppDNS_SetInputData(const uint8_t *pData, uint8_t len);

/**
 * @brief Get output data (data received from Master)
 * @param pData Pointer to destination buffer
 * @param len Length to read (max DEVICENET_OUTPUT_SIZE)
 * @return 0 on success, negative on error
 * @note Only available in FULL mode (ENABLE_DEVICENET=1)
 */
int AppDNS_GetOutputData(uint8_t *pData, uint8_t len);

/**
 * @brief Get I/O statistics
 * @param pReadCount Pointer to store read count (can be NULL)
 * @param pWriteCount Pointer to store write count (can be NULL)
 * @param pErrorCount Pointer to store error count (can be NULL)
 * @note Only available in FULL mode (ENABLE_DEVICENET=1)
 */
void AppDNS_GetStatistics(uint32_t *pReadCount, uint32_t *pWriteCount, uint32_t *pErrorCount);

#ifdef __cplusplus
}
#endif

#endif /* APPDNS_DEMOAPPLICATION_H */

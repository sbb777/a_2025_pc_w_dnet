/**
 * @file AppDNS_DemoApplication.c
 * @brief DeviceNet Demo Application Implementation
 * @date 2025-01-13
 *
 * This file contains the main DeviceNet application implementation.
 * Supports both STUB mode (minimal) and FULL mode (with SDK).
 */

/* Includes ------------------------------------------------------------------*/
#include "AppDNS_DemoApplication.h"
#include "DeviceNet_Config.h"
#include <stdint.h>
#include <string.h>

#if ENABLE_DEVICENET
#include "cifXToolkit.h"
#include "cifXHWFunctions.h"
#include "SerialDPMInterface.h"
#endif

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define DEVICENET_DEVICE_NAME       "cifX0"
#define DEVICENET_CHANNEL_INDEX     0
#define CIFX_IO_TIMEOUT_MS          10

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
#if ENABLE_DEVICENET
/* Full DeviceNet variables */
static uint8_t g_deviceNetInitialized = 0;
static uint8_t g_inputData[DEVICENET_INPUT_SIZE] = {0};
static uint8_t g_outputData[DEVICENET_OUTPUT_SIZE] = {0};

/* cifX handles */
static CIFXHANDLE g_hDriver = NULL;
static CIFXHANDLE g_hChannel = NULL;

/* Statistics */
static uint32_t g_ioReadCount = 0;
static uint32_t g_ioWriteCount = 0;
static uint32_t g_ioErrorCount = 0;

#else
/* Stub mode variables */
static uint8_t g_stubCounter = 0;
#endif

/* Private function prototypes -----------------------------------------------*/
#if ENABLE_DEVICENET
static int32_t DeviceNet_InitDriver(void);
static int32_t DeviceNet_OpenChannel(void);
static int32_t DeviceNet_IOExchange(void);
#endif

/* Private functions ---------------------------------------------------------*/

#if ENABLE_DEVICENET
/**
 * @brief Initialize cifX driver
 * @return 0 on success, negative on error
 */
static int32_t DeviceNet_InitDriver(void)
{
    int32_t lRet;

    /* Initialize Serial DPM (SPI interface) */
    lRet = SerialDPM_Init();
    if (lRet != CIFX_NO_ERROR)
    {
        return -1;  /* SPI init failed */
    }

    /* Open cifX driver */
    lRet = xDriverOpen(&g_hDriver);
    if (lRet != CIFX_NO_ERROR)
    {
        return -2;  /* Driver open failed */
    }

    return 0;  /* Success */
}

/**
 * @brief Open DeviceNet channel
 * @return 0 on success, negative on error
 */
static int32_t DeviceNet_OpenChannel(void)
{
    int32_t lRet;

    /* Open channel 0 */
    lRet = xChannelOpen(g_hDriver,
                        DEVICENET_DEVICE_NAME,
                        DEVICENET_CHANNEL_INDEX,
                        &g_hChannel);

    if (lRet != CIFX_NO_ERROR)
    {
        return -1;  /* Channel open failed */
    }

    return 0;  /* Success */
}

/**
 * @brief Perform I/O data exchange
 * @return 0 on success, negative on error
 */
static int32_t DeviceNet_IOExchange(void)
{
    int32_t lRet;

    /* Read input data from DeviceNet (Master -> Slave data) */
    lRet = xChannelIORead(g_hChannel,
                          0,  /* Area number (0 for standard I/O) */
                          0,  /* Offset */
                          DEVICENET_OUTPUT_SIZE,
                          g_outputData,
                          CIFX_IO_TIMEOUT_MS);

    if (lRet == CIFX_NO_ERROR)
    {
        g_ioReadCount++;

        /* Process received data here */
        /* Example: Copy to application buffers, trigger events, etc. */
    }
    else
    {
        g_ioErrorCount++;
    }

    /* Write output data to DeviceNet (Slave -> Master data) */
    lRet = xChannelIOWrite(g_hChannel,
                           0,  /* Area number (0 for standard I/O) */
                           0,  /* Offset */
                           DEVICENET_INPUT_SIZE,
                           g_inputData,
                           CIFX_IO_TIMEOUT_MS);

    if (lRet == CIFX_NO_ERROR)
    {
        g_ioWriteCount++;
    }
    else
    {
        g_ioErrorCount++;
    }

    return 0;  /* Success */
}
#endif /* ENABLE_DEVICENET */

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize DeviceNet Demo Application
 * @return 0 on success, negative on error
 */
int AppDNS_DemoApplication_Init(void)
{
#if ENABLE_DEVICENET
    int32_t lRet;

    /* FULL MODE: Complete DeviceNet initialization */

    /* 1. Initialize cifX driver */
    lRet = DeviceNet_InitDriver();
    if (lRet != 0)
    {
        return lRet;
    }

    /* 2. Open DeviceNet channel */
    lRet = DeviceNet_OpenChannel();
    if (lRet != 0)
    {
        return lRet;
    }

    /* 3. Initialize I/O data buffers */
    memset(g_inputData, 0, sizeof(g_inputData));
    memset(g_outputData, 0, sizeof(g_outputData));

    /* 4. Reset statistics */
    g_ioReadCount = 0;
    g_ioWriteCount = 0;
    g_ioErrorCount = 0;

    /* Mark as initialized */
    g_deviceNetInitialized = 1;

    return 0;  /* Success */

#else
    /* STUB MODE: Minimal initialization */
    g_stubCounter = 0;
    return 0;  /* Success */
#endif
}

/**
 * @brief Run DeviceNet Demo Application main loop
 * @return 0 on success, negative on error
 */
int AppDNS_DemoApplication_Run(void)
{
#if ENABLE_DEVICENET
    /* FULL MODE: Handle DeviceNet communication */

    if (!g_deviceNetInitialized)
        return -1;  /* Not initialized */

    /* Perform cyclic I/O data exchange */
    DeviceNet_IOExchange();

    /* Handle explicit messages (future enhancement) */
    /* TODO: Implement explicit message handling with xChannelGetPacket/PutPacket */

    return 0;  /* Success */

#else
    /* STUB MODE: Increment counter (minimal operation) */
    g_stubCounter++;
    return 0;  /* Success */
#endif
}

/**
 * @brief Get DeviceNet initialization status
 * @return 1 if initialized, 0 otherwise
 */
uint8_t AppDNS_GetInitStatus(void)
{
#if ENABLE_DEVICENET
    return g_deviceNetInitialized;
#else
    return 1;  /* Stub always "initialized" */
#endif
}

/**
 * @brief Get stub mode counter (debug)
 * @return Current counter value (stub mode only)
 */
uint32_t AppDNS_GetStubCounter(void)
{
#if ENABLE_DEVICENET
    return 0;  /* Not applicable in full mode */
#else
    return g_stubCounter;
#endif
}

#if ENABLE_DEVICENET
/**
 * @brief Set input data (data to send to Master)
 * @param pData Pointer to data buffer
 * @param len Length of data (max DEVICENET_INPUT_SIZE)
 * @return 0 on success, negative on error
 */
int AppDNS_SetInputData(const uint8_t *pData, uint8_t len)
{
    if (!g_deviceNetInitialized)
        return -1;

    if (len > DEVICENET_INPUT_SIZE)
        len = DEVICENET_INPUT_SIZE;

    memcpy(g_inputData, pData, len);
    return 0;
}

/**
 * @brief Get output data (data received from Master)
 * @param pData Pointer to destination buffer
 * @param len Length to read (max DEVICENET_OUTPUT_SIZE)
 * @return 0 on success, negative on error
 */
int AppDNS_GetOutputData(uint8_t *pData, uint8_t len)
{
    if (!g_deviceNetInitialized)
        return -1;

    if (len > DEVICENET_OUTPUT_SIZE)
        len = DEVICENET_OUTPUT_SIZE;

    memcpy(pData, g_outputData, len);
    return 0;
}

/**
 * @brief Get I/O statistics
 * @param pReadCount Pointer to store read count
 * @param pWriteCount Pointer to store write count
 * @param pErrorCount Pointer to store error count
 */
void AppDNS_GetStatistics(uint32_t *pReadCount, uint32_t *pWriteCount, uint32_t *pErrorCount)
{
    if (pReadCount)
        *pReadCount = g_ioReadCount;
    if (pWriteCount)
        *pWriteCount = g_ioWriteCount;
    if (pErrorCount)
        *pErrorCount = g_ioErrorCount;
}
#endif /* ENABLE_DEVICENET */

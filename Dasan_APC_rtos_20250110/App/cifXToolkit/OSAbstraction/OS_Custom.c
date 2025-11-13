/**
 * @file OS_Custom.c
 * @brief FreeRTOS OS Abstraction Layer for cifXToolkit
 * @date 2025-01-10
 *
 * This file provides FreeRTOS-specific implementations of OS abstraction functions
 * required by the cifXToolkit library.
 */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include <string.h>
#include <stdint.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Create a mutex
 * @return Mutex handle or NULL on failure
 */
void* OS_CreateMutex(void)
{
    return (void*)xSemaphoreCreateMutex();
}

/**
 * @brief Wait for mutex with timeout
 * @param pvMutex Mutex handle
 * @param ulTimeout Timeout in milliseconds (0xFFFFFFFF for infinite)
 * @return 0 on success, -1 on failure/timeout
 */
int OS_WaitMutex(void* pvMutex, uint32_t ulTimeout)
{
    TickType_t xTicksToWait;

    if (pvMutex == NULL)
        return -1;

    if (ulTimeout == 0xFFFFFFFF)
        xTicksToWait = portMAX_DELAY;
    else
        xTicksToWait = pdMS_TO_TICKS(ulTimeout);

    if (xSemaphoreTake((SemaphoreHandle_t)pvMutex, xTicksToWait) == pdTRUE)
        return 0;
    else
        return -1;
}

/**
 * @brief Release a mutex
 * @param pvMutex Mutex handle
 */
void OS_ReleaseMutex(void* pvMutex)
{
    if (pvMutex != NULL)
        xSemaphoreGive((SemaphoreHandle_t)pvMutex);
}

/**
 * @brief Delete a mutex
 * @param pvMutex Mutex handle
 */
void OS_DeleteMutex(void* pvMutex)
{
    if (pvMutex != NULL)
        vSemaphoreDelete((SemaphoreHandle_t)pvMutex);
}

/**
 * @brief Allocate memory
 * @param ulSize Size in bytes
 * @return Pointer to allocated memory or NULL on failure
 */
void* OS_Memalloc(uint32_t ulSize)
{
    return pvPortMalloc(ulSize);
}

/**
 * @brief Free allocated memory
 * @param pvMem Pointer to memory
 */
void OS_Memfree(void* pvMem)
{
    if (pvMem != NULL)
        vPortFree(pvMem);
}

/**
 * @brief Copy memory
 * @param pvDest Destination pointer
 * @param pvSrc Source pointer
 * @param ulSize Size in bytes
 */
void OS_Memcpy(void* pvDest, const void* pvSrc, uint32_t ulSize)
{
    memcpy(pvDest, pvSrc, ulSize);
}

/**
 * @brief Set memory to a value
 * @param pvMem Memory pointer
 * @param bFill Fill value
 * @param ulSize Size in bytes
 */
void OS_Memset(void* pvMem, uint8_t bFill, uint32_t ulSize)
{
    memset(pvMem, bFill, ulSize);
}

/**
 * @brief Compare memory
 * @param pvBuf1 First buffer
 * @param pvBuf2 Second buffer
 * @param ulSize Size in bytes
 * @return 0 if equal, non-zero otherwise
 */
int OS_Memcmp(const void* pvBuf1, const void* pvBuf2, uint32_t ulSize)
{
    return memcmp(pvBuf1, pvBuf2, ulSize);
}

/**
 * @brief Sleep for specified milliseconds
 * @param ulSleepTimeMs Sleep time in milliseconds
 */
void OS_Sleep(uint32_t ulSleepTimeMs)
{
    vTaskDelay(pdMS_TO_TICKS(ulSleepTimeMs));
}

/**
 * @brief Get millisecond counter
 * @return Current tick count in milliseconds
 */
uint32_t OS_GetMilliSecCounter(void)
{
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

/**
 * @brief Compare strings
 * @param pszBuf1 First string
 * @param pszBuf2 Second string
 * @return 0 if equal, non-zero otherwise
 */
int OS_Strcmp(const char* pszBuf1, const char* pszBuf2)
{
    return strcmp(pszBuf1, pszBuf2);
}

/**
 * @brief Compare strings case-insensitive with length limit
 * @param pszBuf1 First string
 * @param pszBuf2 Second string
 * @param ulLen Maximum length to compare
 * @return 0 if equal, non-zero otherwise
 */
int OS_Strnicmp(const char* pszBuf1, const char* pszBuf2, uint32_t ulLen)
{
    return strncasecmp(pszBuf1, pszBuf2, ulLen);
}

/**
 * @brief Get string length
 * @param szText String
 * @return Length of string
 */
uint32_t OS_Strlen(const char* szText)
{
    return (uint32_t)strlen(szText);
}

/* File functions - Stubbed (not used in typical embedded applications) */

void* OS_FileOpen(const char* szFile, ...)
{
    /* Not implemented - file system not used */
    return NULL;
}

void OS_FileClose(void* pvFile)
{
    /* Not implemented - file system not used */
}

uint32_t OS_FileRead(void* pvFile, void* pvBuffer, uint32_t ulSize)
{
    /* Not implemented - file system not used */
    return 0;
}

uint32_t OS_FileWrite(void* pvFile, const void* pvBuffer, uint32_t ulSize)
{
    /* Not implemented - file system not used */
    return 0;
}

int OS_FileSeek(void* pvFile, uint32_t ulOffset, int iOrigin)
{
    /* Not implemented - file system not used */
    return -1;
}

uint32_t OS_FileGetsize(void* pvFile)
{
    /* Not implemented - file system not used */
    return 0;
}

# OSAbstraction Directory

This directory contains the Operating System abstraction layer for cifXToolkit.

## Files to Create/Modify

### 1. OS_Custom.c (FreeRTOS Adaptation)
This file must be created to provide FreeRTOS-specific implementations:

#### Required Functions:
```c
/* Mutex functions */
void* OS_CreateMutex(void);
int   OS_WaitMutex(void* pvMutex, uint32_t ulTimeout);
void  OS_ReleaseMutex(void* pvMutex);
void  OS_DeleteMutex(void* pvMutex);

/* Memory functions */
void* OS_Memalloc(uint32_t ulSize);
void  OS_Memfree(void* pvMem);
void  OS_Memcpy(void* pvDest, const void* pvSrc, uint32_t ulSize);
void  OS_Memset(void* pvMem, uint8_t bFill, uint32_t ulSize);
int   OS_Memcmp(const void* pvBuf1, const void* pvBuf2, uint32_t ulSize);

/* Time functions */
void     OS_Sleep(uint32_t ulSleepTimeMs);
uint32_t OS_GetMilliSecCounter(void);

/* String functions */
int      OS_Strcmp(const char* pszBuf1, const char* pszBuf2);
int      OS_Strnicmp(const char* pszBuf1, const char* pszBuf2, uint32_t ulLen);
uint32_t OS_Strlen(const char* szText);

/* File functions (can be stubbed if not using file system) */
void* OS_FileOpen(const char* szFile, ...);
void  OS_FileClose(void* pvFile);
// ... other file functions
```

### 2. OS_SPICustom.c (STM32 HAL SPI5 Adaptation)
This file must be created to provide STM32 HAL SPI5-specific implementations:

#### Required Functions:
```c
/* SPI Hardware Abstraction */
int32_t SPI_Init(void);
int32_t SPI_Deinit(void);
int32_t SPI_Transfer(uint8_t* pbSend, uint8_t* pbRecv, uint32_t ulLen);
int32_t SPI_SetCS(uint8_t bActive);
int32_t SPI_GetSRDY(void);

/* GPIO for handshaking */
void GPIO_InitHandshake(void);
void GPIO_SetCS(GPIO_PinState state);
GPIO_PinState GPIO_GetSRDY(void);
```

## Integration Requirements

### FreeRTOS Integration (OS_Custom.c)
- Use `xSemaphoreCreateMutex()` for mutex creation
- Use `xSemaphoreTake()` / `xSemaphoreGive()` for mutex operations
- Use `vTaskDelay()` for sleep functions
- Use `xTaskGetTickCount()` for time functions

### STM32 HAL SPI5 Integration (OS_SPICustom.c)
- Use `HAL_SPI_TransmitReceive()` for SPI transfer
- Configure SPI5 for:
  - Master mode
  - Full duplex
  - 8-bit data size
  - Clock polarity/phase as per netX 90 requirements
- Implement GPIO control for:
  - CS (Chip Select) signal
  - SRDY (Service Ready) input monitoring

## Timing Requirements

According to netX 90 specifications:
- SPI clock: Up to 20 MHz
- CS setup time: Minimum 10ns
- SRDY polling: Check before each transfer
- Inter-byte delay: None required for continuous transfers

## Source Location

Reference implementations can be found in:
- cifXToolkit example projects
- Application notes for FreeRTOS integration
- netX 90 SPI DPM documentation

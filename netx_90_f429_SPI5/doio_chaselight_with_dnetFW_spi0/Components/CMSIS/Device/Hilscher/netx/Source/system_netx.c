/**************************************************************************//**
 * @file     system_netx.c
 * @brief    CMSIS Cortex-M4 Device Peripheral Access Layer Source File for
 *           Device netx90_app
 * @version  V1.0.0
 * @date     14. August 2017
 ******************************************************************************/
/*
 * Copyright (c) 2009-2016 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "cmsis_gcc.h"
#include "system_netx.h"
#include "netx90_app.h"
#include "Crc32_04C11DB7.h"

/*!---------------------------------------------------------------------------
 Define for system clock speed
 *----------------------------------------------------------------------------*/
#define SYSTEM_CLOCK    (100000000UL)

/*!---------------------------------------------------------------------------
 System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = SYSTEM_CLOCK; /* System Clock Frequency (Core Clock)*/

/*---------------------------------------------------------------------------
 Clock functions
 *----------------------------------------------------------------------------*/

/*!
 * Function for setting the system core clock variable to default
 */
void SystemCoreClockUpdate(void) /* Get Core Clock Frequency      */
{
  SystemCoreClock = SYSTEM_CLOCK;
}

/*!
 * Fault handler for system init reset vector
 */
void SystemInitFault(void)
{
  while(1)
    ;
}

#ifdef NVIC_SOFT_VECTOR
/*!
 * Global used as software reset vector.
 */
void * g_avpfVTOR[__VectorsSize] __attribute__(( aligned(128)))   ;
#endif

/*!
 * Global array where the extracted Temperature Sensor data is written to.
 */
NETX_TEMP_SENSOR_DATA_T g_atTempSensorData[2];

#ifndef SYSTEM_TEMPERATURE_FLOATING_POINT_SUPPORT
/*!
 * Global integer value in which the steepness of the temperature curve is stored.
 */
int g_TempGradient = (int) 0xfffa6048ul;

/*!
 * Global integer value in which the y-intercept of the temperature curve is stored.
 */
int g_TempIntercept = 2455;

/*!
 * Global function that calculates the temperature value by the given ADCValue.
 * Temperature coefficients are recalculated according to extracted from Flash data.
 */
void SystemTemperatureByADCValue(uint32_t ulADCValue, int* TempValue)
{
  *TempValue = (((int) ulADCValue) - g_TempIntercept) * 0xFFFF / g_TempGradient;
}
#else /* SYSTEM_TEMPERATURE_FLOATING_POINT_SUPPORT */
/*!
 * Global integer where the steepness of the temperature curve is stored.
 */
float g_TempGradient = -5.624;

/*!
 * Global integer where the y-intercept of the temperature curve is stored.
 */
float g_TempIntercept = 2455;

/*!
 * Global integer where the y-intercept of the temperature curve is stored.
 */
void SystemTemperatureByADCValue(uint32_t ulADCValue, float* TempValue)
{
  *TempValue = (((float)ulADCValue) - g_TempIntercept) / g_TempGradient;
}
#endif /* SYSTEM_TEMPERATURE_FLOATING_POINT_SUPPORT */

/*!
 * Globals for the trampoline structure of the intflash access functions.
 */
//extern DRV_TRAMPOLINE_T tPageReaderFunction;
//extern DRV_TRAMPOLINE_T tPageFlasherFunction;
volatile uint32_t ulRead = 0;

/*!
 * Function for extracting the Temperature Sensor data from the info page
 */
void PageReader(void)
{
  volatile NETX_TEMP_SENSOR_DATA_T * const ptTempSensorDataPosition = (void * const ) 0x00000800;
  __disable_irq();
  iflash_cfg2->iflash_reset = 1u; // reset flash for cache reset
  ulRead = iflash_cfg2->iflash_reset;
  iflash_cfg2->iflash_reset = 0u; // enable it again with an empty cache
  ulRead = iflash_cfg2->iflash_reset;
  iflash_cfg2->iflash_ifren_cfg = 1u; // show info page
  ulRead = iflash_cfg2->iflash_ifren_cfg;
  __DSB();
  g_atTempSensorData[0] = ptTempSensorDataPosition[0];
  g_atTempSensorData[1] = ptTempSensorDataPosition[1];
  __DSB();
  iflash_cfg2->iflash_ifren_cfg = 0u; // hide info page
  ulRead = iflash_cfg2->iflash_ifren_cfg;
  iflash_cfg2->iflash_reset = 1u; // reset flash for cache reset
  ulRead = iflash_cfg2->iflash_reset;
  iflash_cfg2->iflash_reset = 0u; // enable it again with an empty cache
  ulRead = iflash_cfg2->iflash_reset;
  __DSB();
  __ISB();
  __enable_irq();
}

void* g_avpfHVTOR[16] __attribute__(( aligned(128))) = { 0, 0, SystemInitFault, SystemInitFault, SystemInitFault, SystemInitFault, SystemInitFault,
  SystemInitFault, SystemInitFault, SystemInitFault, SystemInitFault, SystemInitFault, SystemInitFault, SystemInitFault, SystemInitFault,
  SystemInitFault };

/*!
 * Function executing the info page reading.
 *
 * \return void
 */
void callPageReader()
{
  uintptr_t ulOriginalVTOR;
  if(tPageReaderFunction.entry != 0)
  {
    ulOriginalVTOR = SCB->VTOR;
    SCB->VTOR = (uintptr_t) g_avpfHVTOR;
    memcpy(tPageReaderFunction.target, tPageReaderFunction.source, tPageReaderFunction.length);
    __DSB();
    __ISB();
    tPageReaderFunction.entry();
    ulRead = ulRead;
    SCB->VTOR = ulOriginalVTOR;
  }
}

/*!
 *
 */
typedef enum DRV_FLASHER_JOB_Etag
{
  DRV_FLASHER_JOB_ERASE, /*!< */
  DRV_FLASHER_JOB_COPY, /*!< */
  DRV_FLASHER_JOB_ERASE_COPY, /*!< */
} DRV_FLASHER_JOB_E;

#define intflash2 ((uint32_t*)0x00200000)

/*!
 * Function stub for programming data into the flash.
 */
void PageFlasher(DRV_FLASHER_JOB_E eJob, void * pvDestination, void * pvSource, size_t size)
{
  __disable_irq();
  iflash_cfg2->iflash_reset = 1u; // reset flash
  ulRead = iflash_cfg2->iflash_reset;
  iflash_cfg2->iflash_reset = 0u; // enable it again with an empty cache
  ulRead = iflash_cfg2->iflash_reset;
  __DSB();
  // Keep in mind, that there are redundancy pages.
  // Keep in mind, that the xpic/other masters might read from flash.
  // Keep in mind, that the flash is located in a different data switch as the
  // iflash control logic.
  // Erase Page
  // Copy Page
  iflash_cfg2->iflash_reset = 1u; // reset flash for cache reset
  ulRead = iflash_cfg2->iflash_reset;
  iflash_cfg2->iflash_reset = 0u; // enable it again with an empty cache
  ulRead = iflash_cfg2->iflash_reset;
  __DSB();
  __ISB();
  __enable_irq();
}

/*!
 * Pointer to the Page extraction function for linker purposes.
 */
void (* const pPageReader)(void) = PageReader;

/*!
 * Pointer to the Page flasher function for linker purposes.
 */
void (* const pPageFlasher)(DRV_FLASHER_JOB_E, void *, void *, size_t) = PageFlasher;

/*!
 * Function concerned with the calling of all C++ default constructors in the init array.
 *
 * \return void
 */
static void callConstructors(void)
{

  /*lint -save -e681 */
  // Call each function in the list.
  for(void (**p)() = &__preinit_array_start; p < &__preinit_array_end; ++p)
  {
    (*p)();
  }
  // Call each function in the list.
  for(void (**p)() = &__init_array_start; p < &__init_array_end; ++p)
  {
    (*p)();
  }
  /*lint -restore */
}

/*!
 * Function verifying the checksum of the extracted from the info page data.
 * Calculation of CRC takes about 400 microseconds.
 *
 * \return bool
 */
static inline bool
IsSensorDataCrcValid(void)
{
  bool     fCrcValid = false;
  uint32_t ulCrc32 = 0;

  ulCrc32 = CRC32_Calculate(&g_atTempSensorData[0], (sizeof(NETX_TEMP_SENSOR_DATA_T) - sizeof(uint32_t)));
  if(ulCrc32 == g_atTempSensorData[0].ulCrc32)
  {
    fCrcValid = true;
  }

  return fCrcValid;
}

/*!---------------------------------------------------------------------------
 Constants needed for recalculation Temperature Sensor coefficients.
 *----------------------------------------------------------------------------*/
#define ADC_RESOLUTION              (0xFFF)
#define TEMP_INTERCEPT_VOLTAGE_mV   (1559)
#define ADC_VREF_RESOLUTION         (0x3FFF)

/*!---------------------------------------------------------------------------
 ADC_VREF_NO_FLASH_VALUE is used to verify if there is data in FLASH.
 For older revisions FLASH cells are all set 0xFF.
 *----------------------------------------------------------------------------*/
#define ADC_VREF_NO_FLASH_VALUE     (0xFFFF)

#ifndef SYSTEM_TEMPERATURE_FLOATING_POINT_SUPPORT
#define TEMP_GRADIENT_INTERIM       (233960)      /* (0.00357 *1000 * 0xFFFF) */
#else
#define TEMP_GRADIENT_INTERIM       (float)(3.57) /* (0.00357 *1000) */
#endif

/*!
 * Calculates Vref voltage from ADC Vref value read from FLASH.
 */
static inline uint32_t
CalculateVrefVoltage(void)
{
  uint32_t ulVrefVoltage = (g_atTempSensorData[0].usAdcVrefValue * 1000) / ADC_VREF_RESOLUTION;

  return ulVrefVoltage;
}

/*!
 * Recalculates Temperature Intercept with Vref Voltage value read from FLASH.
 */
static inline void
RecalculateTempIntercept(uint32_t ulVrefVoltage)
{
  g_TempIntercept = (ADC_RESOLUTION * TEMP_INTERCEPT_VOLTAGE_mV) / ulVrefVoltage;
}

/*!
 * Recalculates Temperature Gradient with Vref Voltage value read from FLASH.
 */
static inline void
RecalculateTempGradient(uint32_t ulVrefVoltage)
{
  g_TempGradient = ((TEMP_GRADIENT_INTERIM * ADC_RESOLUTION) / ulVrefVoltage) * (-1);
}

/*!
 * Recalculates Temperature Coefficients with Vref Voltage value read from FLASH
 * if CRC valid.
 * Reading data from FLASH together with recalculation takes about 400 microseconds.
 * CRC calculation takes about another 400 microseconds.
 */
static void
RecalculateTempCoefficients(void)
{
  if((ADC_VREF_NO_FLASH_VALUE != g_atTempSensorData[0].usAdcVrefValue) &&
     (IsSensorDataCrcValid()))
  {
    uint32_t ulVrefVoltage = CalculateVrefVoltage();

    RecalculateTempIntercept(ulVrefVoltage);
    RecalculateTempGradient(ulVrefVoltage);
  }
}


/*!---------------------------------------------------------------------------
 System initialization function
 *----------------------------------------------------------------------------*/
void SystemInit(void)
{
#ifdef NVIC_SOFT_VECTOR
  memcpy(g_avpfVTOR, __Vectors, sizeof(g_avpfVTOR));
  SCB->VTOR = (uintptr_t) g_avpfVTOR;
#endif
#if __FPU_USED == 1u
  /* enable FPU if available and used */
  SCB->CPACR |= ((3UL << (10*2)) |             /* set CP10 Full Access               */
                 (3UL << (11*2))  );           /* set CP11 Full Access               */
#endif

  SystemCoreClockUpdate();
  callConstructors();
  callPageReader();
  RecalculateTempCoefficients();
}


/*!************************************************************************//*!
 * \file    netx_drv_tim.h
 * \brief   Header file of timer driver
 * $Revision: 11372 $
 * $Date: 2024-06-06 15:07:46 +0300 (Thu, 06 Jun 2024) $
 * \copyright Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 * \note Exclusion of Liability for this demo software:
 * The following software is intended for and must only be used for reference and in an
 * evaluation laboratory environment. It is provided without charge and is subject to
 * alterations. There is no warranty for the software, to the extent permitted by
 * applicable law. Except when otherwise stated in writing the copyright holders and/or
 * other parties provide the software "as is" without warranty of any kind, either
 * expressed or implied.
 * Please refer to the Agreement in README_DISCLAIMER.txt, provided together with this file!
 * By installing or otherwise using the software, you accept the terms of this Agreement.
 * If you do not agree to the terms of this Agreement, then do not install or use the
 * Software!
 ******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#include "netx_drv.h"
#ifndef __NETX_DRV_TIM_H
#define __NETX_DRV_TIM_H

#ifdef __cplusplus
extern "C"
{
#endif

/*! \addtogroup netX_Driver
 * \{
 */

/*!
 * \addtogroup TIM
 * \{
 * \brief
 */

/*!
 * \brief Union of the possible timer device pointers.
 *
 * This union is used in the timer handle to access the devices.
 */
typedef union DRV_TIM_DEVICE_Utag
{
  DRV_GPIO_DEVICE_T* ptGpio;
  DRV_TIMER_DEVICE_T* ptTimer;
  DRV_SYSTIME_DEVICE_T* ptSystime;
  SysTick_Type* ptSysTick;
  DRV_SYSTIME_LT_DEVICE_T* ptSystimlt;
} DRV_TIM_DEVICE_U;

/*!
 * \brief Time structure of the timer driver.
 *
 * This structure contains the tick count based on the system clock
 * that runs with 100Mhz.
 *
 */
typedef enum DRV_TIM_PRELOAD_VALUE_Etag
{
  DRV_TIM_PRELOAD_VALUE_1Hz = 100000000ul,/*!< Preload value for 1Hz. */
  DRV_TIM_PRELOAD_VALUE_1s = 100000000ul,/*!< Preload value for 1s. */
  DRV_TIM_PRELOAD_VALUE_2Hz = 50000000ul,/*!< Preload value for 2Hz. */
  DRV_TIM_PRELOAD_VALUE_500ms = 50000000ul,/*!< Preload value for 500ms. */
  DRV_TIM_PRELOAD_VALUE_10Hz = 10000000ul,/*!< Preload value for 10Hz. */
  DRV_TIM_PRELOAD_VALUE_100ms = 10000000ul,/*!< Preload value for 100ms. */
  DRV_TIM_PRELOAD_VALUE_100Hz = 1000000ul,/*!< Preload value for 100Hz. */
  DRV_TIM_PRELOAD_VALUE_10ms = 1000000ul,/*!< Preload value for 10ms. */
  DRV_TIM_PRELOAD_VALUE_1KHz = 100000ul,/*!< Preload value for 1KHz. */
  DRV_TIM_PRELOAD_VALUE_1ms = 100000ul,/*!< Preload value for 1ms. */
  DRV_TIM_PRELOAD_VALUE_2KHz = 50000ul,/*!< Preload value for 2KHz. */
  DRV_TIM_PRELOAD_VALUE_500us = 50000ul,/*!< Preload value for 500us. */
  DRV_TIM_PRELOAD_VALUE_10KHz = 10000ul,/*!< Preload value for 10KHz. */
  DRV_TIM_PRELOAD_VALUE_100us = 10000ul,/*!< Preload value for 100us. */
  DRV_TIM_PRELOAD_VALUE_100KHz = 1000ul,/*!< Preload value for 100KHz. */
  DRV_TIM_PRELOAD_VALUE_10us = 1000ul,/*!< Preload value for 10us. */
  DRV_TIM_PRELOAD_VALUE_1MHz = 100ul,/*!< Preload value for 1MHz. */
  DRV_TIM_PRELOAD_VALUE_1us = 100ul,/*!< Preload value for 1us. */
  DRV_TIM_PRELOAD_VALUE_2MHz = 50ul,/*!< Preload value for 2MHz. */
  DRV_TIM_PRELOAD_VALUE_500ns = 50ul,/*!< Preload value for 500ns. */
  DRV_TIM_PRELOAD_VALUE_10MHz = 10ul,/*!< Preload value for 10MHz. */
  DRV_TIM_PRELOAD_VALUE_100ns = 10ul,/*!< Preload value for 100ns. */
  DRV_TIM_PRELOAD_VALUE_100MHz = 1ul,/*!< Preload value for 100MHz. */
  DRV_TIM_PRELOAD_VALUE_10ns = 1ul,/*!< Preload value for 10ns. */
  DRV_TIM_PRELOAD_VALUE_DEFAULT = 0ul,/*!< Preload value of a stopped/paused timer or the systime unit. */
} DRV_TIM_PRELOAD_VALUE_E;

/*!
 * \brief Enumeration of the timer State.
 */
typedef enum DRV_TIM_STATE_Etag
{
  DRV_TIM_STATE_DEFAULT = 0x00ul, /*!< Peripheral not yet initialized or disabled  */
  DRV_TIM_STATE_STOPPED = 0x01ul, /*!< Peripheral Initialized and ready for use, timer stopped   */
  DRV_TIM_STATE_PAUSED = 0x02ul, /*!< Timer is paused   */
  DRV_TIM_STATE_RUNNING = 0x03ul, /*!< Timer is running */
  DRV_TIM_STATE_TIMEOUT = 0x04ul, /*!< Timeout state */
  DRV_TIM_STATE_ERROR = 0x05ul /*!< Reception process is ongoing */
} DRV_TIM_STATE_E;

/*!
 * \brief Enumeration of the external event mode.
 *
 * It defines if the timer uses external events and in which way they will be used
 */
typedef enum DRV_TIM_EXTERNAL_EVENT_MODE_Etag
{
  DRV_TIM_EXTERNAL_EVENT_MODE_DEFAULT = 0x00ul, /*!< No external events are used. */
  DRV_TIM_EXTERNAL_EVENT_MODE_EXTERNAL_EVENTS = 0x01ul, /*!< Each event increments the counter. */
  DRV_TIM_EXTERNAL_EVENT_MODE_WATCHDOG = 0x02ul, /*!< The counter is reset by an event. */
  DRV_TIM_EXTERNAL_EVENT_MODE_AUTO_RUN = 0x03ul /*!< The counter runs only by external signal. */
} DRV_TIM_EXTERNAL_EVENT_MODE_E;

/*!
 * \brief Enumeration defines the kind of external event signal.
 *
 * The external event mode will be triggerd by the following condition. The condition can be inverted
 * by inversion of channel with the digital input output driver.
 */
typedef enum DRV_TIM_EXTERNAL_EVENT_TRIGGER_Etag
{
  DRV_TIM_EXTERNAL_EVENT_MODE_HIGH_LEVEL = 0x00ul, /*!< External event is triggered by high level. */
  DRV_TIM_EXTERNAL_EVENT_MODE_RISING_EDGE = 0x01ul /*!< External event is triggered by rising edge. */
} DRV_TIM_EXTERNAL_EVENT_TRIGGER_E;

/*!
 * \brief The time base that is used for the timer.
 *
 * This time base will be used for the timer. This is necessary because system timers might be stopped for debugging
 * and also if the counter will be used in the DRV_TIM_COUNTING_MODE_COMPARE mode.
 */
typedef enum DRV_TIM_SYSTIME_REFERENCE_Etag
{
  DRV_TIM_SYSTIME_REFERENCE_DEFAULT = 0x00ul, /*!< The default time base. (APP) */
  DRV_TIM_SYSTIME_REFERENCE_COM = 0x10ul, /*!< SYSTIME COM */
  DRV_TIM_SYSTIME_REFERENCE_COM_UC = 0x11ul, /*!< SYSTIME COM UC */
  DRV_TIM_SYSTIME_REFERENCE_APP = 0x12ul, /*!< SYSTIME APP */
  DRV_TIM_SYSTIME_REFERENCE_RESERVED = 0x13ul, /*!< RESERVED */
  DRV_TIM_SYSTIME_REFERENCE_MIN = DRV_TIM_SYSTIME_REFERENCE_COM /*!< The minimum enumeration value for boundary check. */
} DRV_TIM_SYSTIME_REFERENCE_E;

/*!
 * \brief Enumeration of the available counting modes.
 *
 * They decide in which way the counter reloads and how events are generated.
 */
typedef enum DRV_TIM_COUNTING_MODE_Etag
{
  DRV_TIM_COUNTING_MODE_ONCE = 0x00ul, /*!< Counts only once and does not reload. */
  DRV_TIM_COUNTING_MODE_CONTINUOUS = 0x01ul, /*!< Counts continuously and reloads from reload/preload register. */
  DRV_TIM_COUNTING_MODE_COMPARE = 0x02ul, /*!< Compares the counter value with reference clock value and issues an event. */
  DRV_TIM_COUNTING_MODE_RESERVED = 0x03ul, /*!< Reserved mode, behaviour not defined.*/
} DRV_TIM_COUNTING_MODE_E;

/*!
 * \brief Enumeration of the counting symmetries available.
 *
 * Defines in which way the counter counts. From low to high or from low to high/2 and back to low.
 */
typedef enum DRV_TIM_SYMMETRIC_Etag
{
  DRV_TIM_SYMMETRIC_SAWTOOTH = 0x00ul, /*!< asymmetric mode */
  DRV_TIM_SYMMETRIC_TRIANGLE = 0x01ul /*!< symmetric mode */
} DRV_TIM_SYMMETRIC_E;

/*!
 * \brief Timer base Configuration Structure definition
 *
 * The configuration SHALL be changed before initializing the device and shall not be changed
 * afterwards.
 */
typedef struct DRV_TIM_CONFIGURATION_Ttag
{
  DRV_TIM_DEVICE_ID_E eDeviceID; /*!< */
  uint32_t uMax; /*!< Maximum Timer/Counter compare value in microseconds */
  DRV_TIM_PRELOAD_VALUE_E tPreloadValue; /*!< Ticks in 100MHz or on external events. In triangle mode it ticks from 0 to T/2 and back with [T] ticks and on sawtooth from 0 to T  in [T+1] ticks. For systime compare it holds the software counter value in seconds.*/
  DRV_TIM_EXTERNAL_EVENT_MODE_E eExternalEventMode; /*!< Defines what shall happen if there is an external event on the eInputReference*/
  DRV_TIM_EXTERNAL_EVENT_TRIGGER_E eExternalEventTrigger; /*!< Defines what triggers an external event.*/
  DRV_TIM_SYSTIME_REFERENCE_E eSystimeReference; /*!< Which system clock shall be used as time base. */
  DRV_DIO_ID_T eDioIdInputReference; /*!< Which dio channel will be used for event detection*/
  DRV_DIO_MSK_T eDioMskOutputReference; /*!< Where the signal of the counter is linked to.*/
  DRV_TIM_COUNTING_MODE_E eCountingMode; /*!< How the counter reloads.*/
  DRV_TIM_SYMMETRIC_E eSymmetric; /*!< How the counter counts.*/
  DRV_OPERATION_MODE_E eOperationMode; /*!< Which programming method (DMA/IRQ/POLL) is used.*/
} DRV_TIM_CONFIGURATION_T;

/*!
 * \brief Timer Base Handle Structure definition
 *
 * The configuration SHALL be changed before initializing the device and shall not be changed
 * afterwards.
 * The rest of it SHALL not be modified outside of the driver, even if it appears to be possible.
 * \struct DRV_TIM_HANDLE_T
 * \struct DRV_TIM_HANDLE_Ttag
 */
typedef struct DRV_TIM_HANDLE_Ttag
{
  uint32_t ulSubID;/*!< \private Which dimension element of the device is used. */
  DRV_TIM_DEVICE_U ptDevice;/*!< \private Pointer to the device. */
  DRV_TIM_CONFIGURATION_T tConfiguration; /*!< TIM Base configuration parameters */
  DRV_LOCK_T tLock; /*!< \private The drivers locking variable used as internal mutex */
  DRV_TIM_STATE_E eState; /*!< \private The error variable used for return values*/
} DRV_TIM_HANDLE_T;

/*!
 * The Clock rate of the chip.
 */
#define DRV_TIM_CLOCK_RATE SystemCoreClock

/*!
 * \brief Initializes the TIM Base peripheral
 * \memberof DRV_TIM_HANDLE_T
 */
DRV_STATUS_E DRV_TIM_Init(DRV_TIM_HANDLE_T * const ptTim);

/*!
 * \brief  DeInitializes the TIM Base peripheral
 * \memberof DRV_TIM_HANDLE_T
 */
DRV_STATUS_E DRV_TIM_DeInit(DRV_TIM_HANDLE_T * const ptTim);

/*!
 * \brief  Starts the TIM generation.
 * \memberof DRV_TIM_HANDLE_T
 */
DRV_STATUS_E DRV_TIM_Start(DRV_TIM_HANDLE_T * const ptTim);

/*!
 * \brief  Stops the TIM generation.
 * \memberof DRV_TIM_HANDLE_T
 */
DRV_STATUS_E DRV_TIM_Stop(DRV_TIM_HANDLE_T * const ptTim);

/*!
 * \brief  Pauses the TIM generation.
 * \memberof DRV_TIM_HANDLE_T
 */
DRV_STATUS_E DRV_TIM_Pause(DRV_TIM_HANDLE_T * const ptTim);

/*!
 * \brief  Waits at least until uSkipCnt events have occurred.
 * \memberof DRV_TIM_HANDLE_T
 */
DRV_STATUS_E DRV_TIM_Wait(DRV_TIM_HANDLE_T * const ptTim, uint32_t ulTickCnt);

/*!
 * \brief This function sets the threshold used for the specified channel.
 * \memberof DRV_TIM_HANDLE_T
 */
DRV_STATUS_E DRV_TIM_ChannelSetThreshold(DRV_TIM_HANDLE_T * const ptTim, DRV_DIO_ID_T ulChannelID, uint32_t ulTC);

/*!
 * \brief This function gets the captured value of the specified channel.
 * \memberof DRV_TIM_HANDLE_T
 */
DRV_STATUS_E DRV_TIM_ChannelGetCapture(DRV_TIM_HANDLE_T * const ptTim, DRV_DIO_ID_T ulChannelID, uint32_t * const ulValue);

/*!
 * \brief Attaches a given callback to the counter.
 * \memberof DRV_TIM_HANDLE_T
 */
DRV_STATUS_E DRV_TIM_IRQAttach(DRV_TIM_HANDLE_T * const ptTim, DRV_CALLBACK_F pfnUserClb, void* pvUser);

/*!
 * \brief Stores the state of the given counter.
 * \memberof DRV_TIM_HANDLE_T
 */
DRV_STATUS_E DRV_TIM_GetState(DRV_TIM_HANDLE_T * const ptTim, DRV_TIM_STATE_E * const peState);

/*!
 * \brief This function latches all system time counters.
 * \details All systimes have to be read once, after that, they return the actual value. A latch does not expire.
 * \memberof DRV_TIM_HANDLE_T
 * \return void
 */
__STATIC_FORCEINLINE void DRV_TIM_LatchSystimes(void){
  DRV_SYSTIME_LT_DEVICE->intlogic_lt_systimes_latch = (uint32_t) -1;
}

/*!
 * \brief Stores the current counter value
 * \memberof DRV_TIM_HANDLE_T
 */
DRV_STATUS_E DRV_TIM_GetValue(DRV_TIM_HANDLE_T * const ptTim, uint32_t * const ptSoftwareValue, uint32_t * const ptHardwareValue);

/*!
 * \brief Sets a new preload value.
 * \memberof DRV_TIM_HANDLE_T
 */
DRV_STATUS_E DRV_TIM_SetPreload(DRV_TIM_HANDLE_T * const ptTim, DRV_TIM_PRELOAD_VALUE_E tCounterValue);

/*!
 * \brief This is the callback of the GPIOTIM 0 IRQ.
 * \public
 */
void DRV_TIM_GPIOTIM0_Callback(void);

/*!
 * \brief This is the callback of the GPIOTIM 1 IRQ.
 * \public
 */
void DRV_TIM_GPIOTIM1_Callback(void);

/*!
 * \brief This is the callback of the GPIOTIM 2 IRQ.
 * \public
 */
void DRV_TIM_GPIOTIM2_Callback(void);

/*!
 * \brief This is the callback of the TIMER 0 IRQ.
 * \public
 */
void DRV_TIM_TIMER0_Callback(void);

/*!
 * \brief This is the callback of the TIMER 1 IRQ.
 * \public
 */
void DRV_TIM_TIMER1_Callback(void);

/*!
 * \brief This is the callback of the TIMER 2 IRQ.
 * \public
 */
void DRV_TIM_TIMER2_Callback(void);

/*!
 * \brief This is the callback of the SYSTIME COMPARE IRQ.
 * \public
 */
void DRV_TIM_SYSTIME_COMPARE_Callback(void);

/*!
 * \brief This is the callback of the SYSTEM TICK IRQ.
 * \public
 */
void DRV_TIM_ARMSYSTICK_Callback(void);

/*! \} *//* End of group Timer */

/*! \} *//* End of group netx_Driver */

#ifdef __cplusplus
}
#endif

#endif /* __NETX_DRV_TIM_H */

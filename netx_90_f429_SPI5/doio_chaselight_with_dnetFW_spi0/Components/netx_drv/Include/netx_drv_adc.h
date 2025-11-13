/*!************************************************************************//*!
 * \file    netx_drv_adc.h
 * \brief   Header file of ADC DRV module.
 * $Revision: 10952 $
 * $Date: 2023-11-30 10:32:15 +0200 (Thu, 30 Nov 2023) $
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
#ifndef NETX_DRV_ADC_H_
#define NETX_DRV_ADC_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*! \addtogroup netX_Driver
 * \{
 */

/*!
 * \addtogroup ADC
 * \{
 */

#include <stdint.h>
#include <stdbool.h>

/*!
 * \brief Base address for writing the measurement results.
 *
 * Word (16 bit) aligned address, LSB is ignored.
 * In case of DRV_ADC_SEQ_32Bit_MODE_ENABLED, bit 1 will be ignored.
 */
#define  DRV_ADC_SEQ_BASE_ADDRESS_MIN  ((void*)0x00000000) /*!< Min value for boundary checks.*/
#define  DRV_ADC_SEQ_BASE_ADDRESS_MAX  ((void*)0xFFFFFFFF) /*!< Max value for boundary checks.*/

/*!
 * \brief Default parameter for the initialization of DRV_ADC_CONFIGURATION_T
 */
#define DRV_ADC_CONFIG_DEFAULT {\
  .eAdcClckPeriod                     = DRV_ADC_CLK_PERIOD_DEFAULT,\
  .eVrefBufferEnable                  = DRV_ADC_VREF_BUFFER_DISABLED,{\
  .eSoftReset                         = DRV_ADC_SOFT_RESET_INACTIVE,\
  .ePowerDown                         = DRV_ADC_ENABLE},{\
  .eSoftReset                         = DRV_ADC_SOFT_RESET_INACTIVE,\
  .ePowerDown                         = DRV_ADC_ENABLE},\
  .auDeadTimeDelay                    = {0, 0, 0, 0, 0, 0}}

/*!
 * \brief Default parameter for the initialization of DRV_ADC_MEAS_CONFIGURATION_T
 */
#define DRV_ADC_SEQ_MEAS_CONFIG_DEFAULT {\
  .eEnable                            = DRV_ADC_SEQ_MEAS_DISABLE,\
  .eAdrOffset                         = DRV_ADC_SEQ_BASE_ADR_OFFSET_NONE,\
  .eInputChannel                      = DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_0,\
  .eOversample                        = DRV_ADC_SEQ_MEAS_OVERSAMPLING_SUM_1,\
  .eTrigger                           = DRV_ADC_SEQ_MEAS_TRIGGER_IMMEDIATELY}

/*!
 * \brief Default parameter for the initialization of DRV_ADC_SEQ_CONFIGURATION_T
 */
#define DRV_ADC_SEQ_CONFIG_DEFAULT {\
  .eDeviceID                          = (DRV_ADC_SEQ_DEVICE_ID_E)0,\
  .eOperationMode                     = DRV_OPERATION_MODE_POLL,\
  .eDmaModeDisable                    = DRV_ADC_SEQ_DMA_MODE_DISABLED,\
  .e32BitMode                         = DRV_ADC_SEQ_32Bit_MODE_DISABLED,\
  .eVrefVdd3                          = DRV_ADC_SEQ_VREF_VDD3_DISABLED,\
  .eClkSync                           = DRV_ADC_SEQ_CLK_SYNC_DISABLED,\
  .eClkPhase                          = DRV_ADC_SEQ_CLK_PHASE_DEFAULT,\
  .eClkPeriod                         = DRV_ADC_SEQ_CLK_PERIOD_DEFAULT,\
  .eChannelTrackingTime               = {DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_DEFAULT, DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_DEFAULT,\
                                         DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_DEFAULT, DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_DEFAULT,\
                                         DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_DEFAULT, DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_DEFAULT,\
                                         DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_DEFAULT, DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_DEFAULT},\
  .eContinuousMode                    = DRV_ADC_SEQ_CONTINUOUS_MODE_DISABLED,\
  .pvBaseAdr                          = DRV_ADC_SEQ_BASE_ADDRESS_MIN,\
  .tMeasurement                       = {DRV_ADC_SEQ_MEAS_CONFIG_DEFAULT, DRV_ADC_SEQ_MEAS_CONFIG_DEFAULT,\
                                         DRV_ADC_SEQ_MEAS_CONFIG_DEFAULT, DRV_ADC_SEQ_MEAS_CONFIG_DEFAULT,\
                                         DRV_ADC_SEQ_MEAS_CONFIG_DEFAULT, DRV_ADC_SEQ_MEAS_CONFIG_DEFAULT,\
                                         DRV_ADC_SEQ_MEAS_CONFIG_DEFAULT, DRV_ADC_SEQ_MEAS_CONFIG_DEFAULT},\
  .fnSequenceCompleteCallback         = 0,\
  .pSequenceCompleteCallbackHandle    = 0,\
  .fnMeasurementCompleteCallback      = {0, 0, 0, 0, 0, 0, 0, 0},\
  .pMeasurementCompleteCallbackHandle = {0, 0, 0, 0, 0, 0, 0, 0}}

/*!
 * \brief Result was not accessible after the measurement.
 *
 * Possible reason is that the next measurement has already
 * over written the result, before the callback or the irq had the chance to save it.
 */
#define DRV_ADC_SEQ_CATCHED_NO_RESULT 0xffff

/*!
 * \brief Default settings for adc device driver.
 *
 * Use this define to initialize the adc-driver handle-structure with default values.
 */
#define DRV_ADC_INITIALIZER (DRV_ADC_HANDLE_T){0, {0, 0, 0, 0}, DRV_ADC_CONFIG_DEFAULT, DRV_LOCK_INITIALIZER_VALUE}

/*!
 * \brief Default settings for adc sequencer.
 *
 * Use this define to initialize the adc-driver-sequencer handle-structure with default values.
 */
#define DRV_ADC_SEQ_INITIALIZER (DRV_ADC_SEQ_HANDLE_T){0, 0, DRV_ADC_SEQ_CONFIG_DEFAULT, DRV_LOCK_INITIALIZER_VALUE, {0, 0, 0, 0, 0, 0}}

/*!
 * \brief The adc sequencer device IDs.
 *
 * They are used to identify the adc sequencer devices in the driver context.
 */
typedef enum DRV_ADC_Device_MSK_Etag
{
  DRV_ADC_SEQ_DEVICE_MSK_0 = 0x1u,/*!< The ADC drivers internal mask for ADC Sequencer 0.*/
  DRV_ADC_SEQ_DEVICE_MSK_1 = 0x2u,/*!< The ADC drivers internal mask for ADC Sequencer 1.*/
  DRV_ADC_SEQ_DEVICE_MSK_2 = 0x4u,/*!< The ADC drivers internal mask for ADC Sequencer 2.*/
  DRV_ADC_SEQ_DEVICE_MSK_3 = 0x8u,/*!< The ADC drivers internal mask for ADC Sequencer 3.*/
  DRV_ADC_SEQ_DEVICE_MSK_MIN = DRV_ADC_SEQ_DEVICE_MSK_0, /*!< Min value for boundary checks.*/
  DRV_ADC_SEQ_DEVICE_MSK_MAX = 0xFu /*!< Max value for boundary checks.*/
} DRV_ADC_SEQ_DEVICE_MSK_E;

/*!
 * \brief Configuring the ADC-Reference-Voltage-Source
 *
 * Three modes are possible related to the effective ADC-Reference-voltage-input.
 * 1. When pHndSeq->tConfiguration.eVrefVdd3 is set to 'DRV_ADC_SEQ_VREF_VDD3_DISABLED' actually the 'VREF_ADC-pin' is not used.
 * In this case effectively VDDIO(3V3) is used as an internal reference-voltage. (s.a. 'ptSequencer->ptDevice->madc_seq_cfg_b.vref_vdd3').
 * Otherwise, when set to 'DRV_ADC_SEQ_VREF_VDD3_ENABLED' one of the following modes is active. (s.a. 'ptDriver->ptDevice->madc_adc01_static_cfg_b.vref_buffer_enable')
 *
 * 2. When pHndADC->tConfiguration.eVrefBufferEnable is set to 'DRV_ADC_VREF_BUFFER_ENABLED'
 * the netX-90 'VREF_ADC-pin' is driven by the internal 2.6V-Reference.
 *
 * 3. When pHndADC->tConfiguration.eVrefBufferEnable is set to 'DRV_ADC_VREF_BUFFER_DISABLED'
 * the netX-90 'VREF_ADC-pin' must be driven by an external Reference-Voltage (e.g. filtered 3.3V).
 *
 * Note that mode-1 might be be more inaccurate due to noise on VDD3.
 * Note that for mode-2 and -3, an external capacitor must be placed between the pins VREF_ADC and VSS_REF.
 * Note that even if mode-1 is configured and thus the 'VREF_ADC-pin' not used,
 * the 'eVrefBufferEnable' still controls if the 'VREF_ADC-pin' is driven by the 2.6V-Reference or not !
 * Also make sure when driving the 'VREF_ADC-pin' by an external reference-voltage, do not activate 'eVrefBufferEnable' in this case
 * since this might lead to unwanted currents ! (Such currents have even side-effects on mode-1.)
 * And note that the pin VSS_REF must always be connected to the GND-net ! (otherwise the on-chip DC/DC will not work !)
 * \sa DRV_ADC_SEQ_VREF_VDD3_E, DRV_ADC_SEQ_VREF_VDD3_Etag
*/
typedef enum DRV_ADC_VREF_BUFFER_Etag
{
  DRV_ADC_VREF_BUFFER_ENABLED  = 0x1u,  /*!< Drive 2.6V on 'VREF_ADC-pin' */
  DRV_ADC_VREF_BUFFER_DISABLED = 0x0u,  /*!< Behave passive .*/
  DRV_ADC_VREF_BUFFER_MIN = DRV_ADC_VREF_BUFFER_DISABLED, /*!< Min value for boundary checks.*/
  DRV_ADC_VREF_BUFFER_MAX = DRV_ADC_VREF_BUFFER_ENABLED /*!< Max value for boundary checks.*/
} DRV_ADC_VREF_BUFFER_E;

/*!
 * \brief Enable or Disable the DMA mode.
 *
 * If DMA mode disabled, results are not written to memory and can only access by the result register.
 */
typedef enum DRV_ADC_SEQ_DMA_MODE_Etag
{
  DRV_ADC_SEQ_DMA_MODE_ENABLED = 0x0u,/*!< Enables the DMA mode.(Default)*/
  DRV_ADC_SEQ_DMA_MODE_DISABLED = 0x1u,/*!< Disable the DMA mode.*/
  DRV_ADC_SEQ_DMA_MODE_MIN = DRV_ADC_SEQ_DMA_MODE_ENABLED, /*!< Min value for boundary checks.*/
  DRV_ADC_SEQ_DMA_MODE_MAX = DRV_ADC_SEQ_DMA_MODE_DISABLED /*!< Max value for boundary checks .*/
} DRV_ADC_SEQ_DMA_MODE_E;
/*!
 * \brief The copy engine only uses 32bit addresses and DWord access.
 * This mode wastes memory but speeds up the copy by not running read-modify-write cycles.
 * If deactivated, the default 16bit addresses and Word access is used.
 */
typedef enum DRV_ADC_SEQ_32Bit_MODE_Etag
{
  DRV_ADC_SEQ_32Bit_MODE_ENABLED = 0x1u,/*!< Enables the 32bit addressing mode.*/
  DRV_ADC_SEQ_32Bit_MODE_DISABLED = 0x0u,/*!< Disable the 32bit addressing mode.(Default)*/
  DRV_ADC_SEQ_32Bit_MODE_MIN = DRV_ADC_SEQ_32Bit_MODE_DISABLED, /*!< Min value for boundary checks.*/
  DRV_ADC_SEQ_32Bit_MODE_MAX = DRV_ADC_SEQ_32Bit_MODE_ENABLED /*!< Max value for boundary checks .*/
} DRV_ADC_SEQ_32Bit_MODE_E;

/*!
 * \brief Use core-voltage of 3.3V as reference or use Vref of the adc (which can be external or 2.6V).
 * \sa DRV_ADC_VREF_BUFFER_ENABLED, DRV_ADC_VREF_BUFFER_Etag
 */
typedef enum DRV_ADC_SEQ_VREF_VDD3_Etag
{
  DRV_ADC_SEQ_VREF_VDD3_ENABLED = 0x1u,/*!< Use reference from pin VDD3  (Analog core supply).*/
  DRV_ADC_SEQ_VREF_VDD3_DISABLED = 0x0u,/*!< Use reference from pin VREF_ADC (internally driven C or external reference, s. madc_adc01_static_cfg-vref_buffer_enable).*/
  DRV_ADC_SEQ_VREF_VDD3_MIN = DRV_ADC_SEQ_VREF_VDD3_DISABLED, /*!< Min value for boundary checks.*/
  DRV_ADC_SEQ_VREF_VDD3_MAX = DRV_ADC_SEQ_VREF_VDD3_ENABLED /*!< Max value for boundary checks.*/
} DRV_ADC_SEQ_VREF_VDD3_E;

/*!
 * \brief Synchronization of the adc clocks.
 */
typedef enum DRV_ADC_SEQ_CLK_SYNC_Etag
{
  DRV_ADC_SEQ_CLK_SYNC_DISABLED = 0x0u,/*!< The rising edges of adcclk are generated independently of the other ADCs. (Default)*/
  DRV_ADC_SEQ_CLK_SYNC_ENABLED = 0x1u,/*!< Use adc_clock_phase for defined clock phases in relation to other ADC sequencers.*/
  DRV_ADC_SEQ_CLK_SYNC_MIN = DRV_ADC_SEQ_CLK_SYNC_DISABLED, /*!< Min value for boundary checks.*/
  DRV_ADC_SEQ_CLK_SYNC_MAX = DRV_ADC_SEQ_CLK_SYNC_ENABLED /*!< Max value for boundary checks.*/
} DRV_ADC_SEQ_CLK_SYNC_E;

/*!
 * \brief Generation of the rising edge of the adcclk is delayed until the global clk_phase counter matches this value.
 * NOTE: The rising edge of the adcclk ending the first sample period of a triggered measurement is NEVER delayed.
 * \warning It is not feasible to differ from the default value. The option to change it is for debugging purposes only.
 */
typedef enum DRV_ADC_SEQ_CLK_PHASE_Etag
{
  DRV_ADC_SEQ_CLK_PHASE_DEFAULT = 0xFFu,/*!< Default setting of the device.*/
  DRV_ADC_SEQ_CLK_PHASE_MIN = 0x00u, /*!< Min value for boundary checks.*/
  DRV_ADC_SEQ_CLK_PHASE_MAX = 0xFFu /*!< Max value for boundary checks .*/
} DRV_ADC_SEQ_CLK_PHASE_E;

/*!
 * \brief Duration of an adcclk period in system clock cycles-1.
 * \warning It is not feasible to differ from the default value. The option to change it is for debugging purposes only.
 * For odd values the high phase of adcclk is one system clock cycle longer than the low phase.
 */
typedef enum DRV_ADC_SEQ_CLK_PERIOD_Etag
{
  DRV_ADC_SEQ_CLK_PERIOD_DEFAULT = 0x2u,/*!< Default setting of the device.*/
  DRV_ADC_SEQ_CLK_PERIOD_MIN = 0x00u, /*!< Min value for boundary checks.*/
  DRV_ADC_SEQ_CLK_PERIOD_MAX = 0xFFu /*!< Max value for boundary checks.*/
} DRV_ADC_SEQ_CLK_PERIOD_E;

/*!
 * \brief ADC sample extension for input channel 0
 *
 * Length of 2nd adcclk in steps of 10ns system clock(~ delay of 3rd adcclk edge).
 * The capacitor inside ADC needs time to be charged depending on the driving strength of the external signal.
 * For 12 bit precision, a settling to time of 9 tau should be ensured.
 * tau can be calculated by tau = 9*(ADC_MUX_RSER + Rext) * ADC_MUX_CIN, with Rext being additional resistance from devices external to the SOC.
 * The setting of tt_add also depends on the setting of
 * madc_seq_cfg.adcclk_period and can be calculate by the following formula:
 * tt_add = ceil( tau/10ns - madc_seq_cfg.adcclk_period ) - 2
 * Note:
 * madc_seq_cfg.adcclk_period and madc_seq_tracking_time_mux[x].tt_add will count down to 0
 * meaning that the value set in the registers is increased by 1.
 * To compensate this the tt_add formula includes a '-2'.
 * Note: ADC_MUX_RSET(1.5kOhm) and ADC_MUX_CIN(10pF) are defined in the electrical parameters.
 * If the ADC is driven by low-ohmic shunt resistor (Rext = 200mOhm), and adcclk_period = 2, the tt_add is:
 * tt_add = ceil(9 * (1.5 kOhm + 0.2 Ohm) * 10 pF / 10 ns - 2) - 2
 * tt_add = ceil(135 ns / 10 ns - 2 - 2) = 10
 * Set tt_add=10 if calculated value is smaller than 10.
 */
typedef enum DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_Etag
{
  DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_DEFAULT = 0x0Au,/*!< Default setting of the device.*/
  DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_MIN = 0x00u, /*!< Min value for boundary checks.*/
  DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_MAX = 0xFFu /*!< Max value for boundary checks.*/
} DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_E;

/*!
 * \brief Measurements of the sequencer
 */
typedef enum DRV_ADC_SEQ_MEASUREMENT_Etag
{
  DRV_ADC_SEQ_MEASUREMENT_0 = 0u,/*!< Measurement 0 active.*/
  DRV_ADC_SEQ_MEASUREMENT_1 = 1u,/*!< Measurement 1 active.*/
  DRV_ADC_SEQ_MEASUREMENT_2 = 2u,/*!< Measurement 2 active.*/
  DRV_ADC_SEQ_MEASUREMENT_3 = 3u,/*!< Measurement 3 active.*/
  DRV_ADC_SEQ_MEASUREMENT_4 = 4u,/*!< Measurement 4 active.*/
  DRV_ADC_SEQ_MEASUREMENT_5 = 5u,/*!< Measurement 5 active.*/
  DRV_ADC_SEQ_MEASUREMENT_6 = 6u,/*!< Measurement 6 active.*/
  DRV_ADC_SEQ_MEASUREMENT_7 = 7u,/*!< Measurement 7 active.*/
  DRV_ADC_SEQ_MEASUREMENT_MIN = DRV_ADC_SEQ_MEASUREMENT_0, /*!< Min value for boundary checks.*/
  DRV_ADC_SEQ_MEASUREMENT_MAX = DRV_ADC_SEQ_MEASUREMENT_7 /*!< Max value for boundary checks.*/
} DRV_ADC_SEQ_MEASUREMENT_E;

/*!
 * \brief Enable measurement configuration.
 *
 * A measurement sequence consists of up to 8 measurements
 */
typedef enum DRV_ADC_SEQ_MEAS_ENABLE_Etag
{
  DRV_ADC_SEQ_MEAS_DISABLE = 0x0u,/*!< Disable measurement. (Default)*/
  DRV_ADC_SEQ_MEAS_ENABLE = 0x1u,/*!< Enable measurement.*/
  DRV_ADC_SEQ_MEAS_ENABLE_MIN = DRV_ADC_SEQ_MEAS_DISABLE, /*!< Min value for boundary checks.*/
  DRV_ADC_SEQ_MEAS_ENABLE_MAX = DRV_ADC_SEQ_MEAS_ENABLE /*!< Max value for boundary checks.*/
} DRV_ADC_SEQ_MEAS_ENABLE_E;

/*!
 * \brief Address offset specified in 16 bit words where the sum will be stored.
 *
 * In case of DRV_ADC_SEQ_32Bit_MODE_ENABLED, this value will be interpreted as 32-bit address.
 */
typedef enum DRV_ADC_SEQ_MEAS_ADR_OFFSET_Etag
{
  DRV_ADC_SEQ_BASE_ADR_OFFSET_NONE = 0x00u, /*!< None address offset.*/
  DRV_ADC_SEQ_BASE_ADR_OFFSET_MIN = DRV_ADC_SEQ_BASE_ADR_OFFSET_NONE, /*!< Min value for boundary checks.*/
  DRV_ADC_SEQ_BASE_ADR_OFFSET_MAX = 0x1Fu, /*!< Max value for boundary checks.*/
} DRV_ADC_SEQ_MEAS_ADR_OFFSET_E;

/*!
 * \brief Input channel multiplexer setting.
 *
 * The input multiplexer will always be set 1 system clock (10ns) before SOF.
 * It will be reset after sampling to ensure a not-connected phase at the one-hot-coded multiplexer switches.
 */
typedef enum DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_Etag
{
  DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_0 = 0x0u,/*!< The input channel 0.*/
  DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_1 = 0x1u,/*!< The input channel 1.*/
  DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_2 = 0x2u,/*!< The input channel 2.*/
  DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_3 = 0x3u,/*!< The input channel 3.*/
  DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_4 = 0x4u,/*!< The input channel 4.*/
  DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_5 = 0x5u,/*!< The input channel 5.*/
  DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_6 = 0x6u,/*!< The input channel 6.*/
  DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_7 = 0x7u,/*!< The input channel 7.*/
  DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_MIN = DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_0, /*!< Min value for boundary checks.*/
  DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_MAX = DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_7 /*!< Max value for boundary checks.*/
} DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_E;

/*!
 * \brief Number of samples minus one to sum for this measurement i.e. 0:sum 1 samples, 1:sum 2 samples, ...
 */
typedef enum DRV_ADC_SEQ_MEAS_OVERSAMPLING_Etag
{
  DRV_ADC_SEQ_MEAS_OVERSAMPLING_SUM_1 = 0x0ul,/*!< Sum 1 sample.*/
  DRV_ADC_SEQ_MEAS_OVERSAMPLING_SUM_2 = 0x1ul,/*!< Sum 2 samples.*/
  DRV_ADC_SEQ_MEAS_OVERSAMPLING_SUM_3 = 0x2ul,/*!< Sum 3 samples.*/
  DRV_ADC_SEQ_MEAS_OVERSAMPLING_SUM_4 = 0x3ul,/*!< Sum 4 samples.*/
  DRV_ADC_SEQ_MEAS_OVERSAMPLING_SUM_5 = 0x4ul,/*!< Sum 5 samples.*/
  DRV_ADC_SEQ_MEAS_OVERSAMPLING_SUM_6 = 0x5ul,/*!< Sum 6 samples.*/
  DRV_ADC_SEQ_MEAS_OVERSAMPLING_SUM_7 = 0x6ul,/*!< Sum 7 samples.*/
  DRV_ADC_SEQ_MEAS_OVERSAMPLING_SUM_8 = 0x7ul,/*!< Sum 8 samples.*/
  DRV_ADC_SEQ_MEAS_OVERSAMPLING_MIN = DRV_ADC_SEQ_MEAS_OVERSAMPLING_SUM_1, /*!< Min value for boundary checks.*/
  DRV_ADC_SEQ_MEAS_OVERSAMPLING_MAX = DRV_ADC_SEQ_MEAS_OVERSAMPLING_SUM_8 /*!< Max value for boundary checks.*/
} DRV_ADC_SEQ_MEAS_OVERSAMPLING_E;

/*!
 * \brief Trigger condition for measurement
 */
typedef enum DRV_ADC_SEQ_MEAS_TRIGGER_Etag
{
  DRV_ADC_SEQ_MEAS_TRIGGER_CONDITION_MIN = 0x0ul,/*!< Extended counter (PWM) minimum value.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_CONDITION_MAX = 0x0FFFFul,/*!< Extended counter (PWM) maximum value.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_DELAYED_DTEVT_0 = 0x10000ul,/*!< delayed with dead time delay of PWM channel 0.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_DELAYED_DTEVT_1 = 0x10001ul,/*!< delayed with dead time delay of PWM channel 1.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_DELAYED_DTEVT_2 = 0x10002ul,/*!< delayed with dead time delay of PWM channel 2.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_DELAYED_DTEVT_3 = 0x10003ul,/*!< delayed with dead time delay of PWM channel 3.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_DELAYED_DTEVT_4 = 0x10004ul,/*!< delayed with dead time delay of PWM channel 4.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_DELAYED_DTEVT_5 = 0x10005ul,/*!< delayed with dead time delay of PWM channel 5.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_GPIO_APP_COUNTER_0 = 0x10006ul,/*!< GPIO_APP_COUNTER0 = 0.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_GPIO_APP_COUNTER_1 = 0x10007ul,/*!< GPIO_APP_COUNTER1 = 0.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_GPIO_APP_COUNTER_2 = 0x10008ul,/*!< GPIO_APP_COUNTER2 = 0.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_POS_EDGE_XC_TRIGGER_0 = 0x10009ul,/*!< posedge of xc_trigger[0].*/
  DRV_ADC_SEQ_MEAS_TRIGGER_NEG_EDGE_XC_TRIGGER_0 = 0x1000aul,/*!< posedge of xc_trigger[1].*/
  DRV_ADC_SEQ_MEAS_TRIGGER_POS_EDGE_XC_TRIGGER_1 = 0x1000bul,/*!< negedge of xc_trigger[0].*/
  DRV_ADC_SEQ_MEAS_TRIGGER_NEG_EDGE_XC_TRIGGER_1 = 0x1000cul,/*!< negedge of xc_trigger[1].*/
  DRV_ADC_SEQ_MEAS_TRIGGER_POS_EDGE_XC_SAMPLE_0 = 0x1000dul,/*!< posedge of xc_sample[0].*/
  DRV_ADC_SEQ_MEAS_TRIGGER_NEG_EDGE_XC_SAMPLE_0 = 0x1000eul,/*!< posedge of xc_sample[1].*/
  DRV_ADC_SEQ_MEAS_TRIGGER_POS_EDGE_XC_SAMPLE_1 = 0x1000ful,/*!< negedge of xc_sample[0].*/
  DRV_ADC_SEQ_MEAS_TRIGGER_NEG_EDGE_XC_SAMPLE_1 = 0x10010ul,/*!< negedge of xc_sample[1].*/
  DRV_ADC_SEQ_MEAS_TRIGGER_RESERVED_MIN = 0x10011ul,/*!< RESERVED min id.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_RESERVED_MAX = 0x1FFFEul,/*!< RESERVED max id.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_IMMEDIATELY = 0x1FFFFul,/*!< no trigger, measurement executes immediately after end of sampling phase.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_MIN = DRV_ADC_SEQ_MEAS_TRIGGER_CONDITION_MIN, /*!< Min value for boundary checks.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_MAX = DRV_ADC_SEQ_MEAS_TRIGGER_IMMEDIATELY /*!< Max value for boundary checks.*/
} DRV_ADC_SEQ_MEAS_TRIGGER_E;

/*!
 * \brief Continuous or Single Shot mode of the device and api.
 */
typedef enum DRV_ADC_SEQ_CONTINUOUS_MODE_Etag
{
  DRV_ADC_SEQ_CONTINUOUS_MODE_DISABLED = 0x0ul,/*!< Continuous mode is disabled. (Default)*/
  DRV_ADC_SEQ_CONTINUOUS_MODE_ENABLED = 0x1ul,/*!< Continuous mode is enabled.*/
  DRV_ADC_SEQ_CONTINUOUS_MODE_MIN = DRV_ADC_SEQ_CONTINUOUS_MODE_DISABLED, /*!< Min value for boundary checks.*/
  DRV_ADC_SEQ_CONTINUOUS_MODE_MAX = DRV_ADC_SEQ_CONTINUOUS_MODE_ENABLED /*!< Max value for boundary checks.*/
} DRV_ADC_SEQ_CONTINUOUS_MODE_E;

/*!
 * \brief Max value of global ADC synchronization counter:
 *
 * ADCs running at same adcclk might interfere. Therefore the ADCs should be able to run in different clk-phases.
 * Adcclk generation will be done within the ADC sequencers.
 * This value is used to configure a global counter for clock phase reference. It's value should be the same or an natural numbered multiple of the value configured in ADC sequencers.
 */
typedef enum DRV_ADC_CLK_PERIOD_Etag
{
  DRV_ADC_CLK_PERIOD_DEFAULT = 2u,/*!< The default value.*/
  DRV_ADC_CLK_PERIOD_MIN = 0u, /*!< Min value for boundary checks.*/
  DRV_ADC_CLK_PERIOD_MAX = 0xffu /*!< Max value for boundary checks.*/
} DRV_ADC_CLK_PERIOD_E;

/*!
 * \brief The enumeration for the available measurements
 */
typedef enum DRV_ADC_STATE_MEASUREMENT_Etag
{
  DRV_ADC_STATE_MEASUREMENT_0 = 0u,/*!< Measurement 0 active.*/
  DRV_ADC_STATE_MEASUREMENT_1 = 1u,/*!< Measurement 1 active.*/
  DRV_ADC_STATE_MEASUREMENT_2 = 2u,/*!< Measurement 2 active.*/
  DRV_ADC_STATE_MEASUREMENT_3 = 3u,/*!< Measurement 3 active.*/
  DRV_ADC_STATE_MEASUREMENT_4 = 4u,/*!< Measurement 4 active.*/
  DRV_ADC_STATE_MEASUREMENT_5 = 5u,/*!< Measurement 5 active.*/
  DRV_ADC_STATE_MEASUREMENT_6 = 6u,/*!< Measurement 6 active.*/
  DRV_ADC_STATE_MEASUREMENT_7 = 7u,/*!< Measurement 7 active.*/
  DRV_ADC_STATE_MEASUREMENT_IDLE = 8u,/*!< Measurement idle / not measuring.*/
  DRV_ADC_STATE_MEASUREMENT_MAX = DRV_ADC_STATE_MEASUREMENT_IDLE /*!< Max value for boundary checks.*/
} DRV_ADC_STATE_MEASUREMENT_E;

/*!
 * \brief Soft-Reset configuration of the ADCs.
 */
typedef enum DRV_ADC_SOFT_RESET_Etag
{
  DRV_ADC_SOFT_RESET_ACTIVE = 0u,/*!< Soft-Reset is active (Default).*/
  DRV_ADC_SOFT_RESET_INACTIVE = 1u,/*!< Soft-Reset is inactive. */
  DRV_ADC_SOFT_RESET_MIN = DRV_ADC_SOFT_RESET_ACTIVE, /*!< Min value for boundary checks.*/
  DRV_ADC_SOFT_RESET_MAX = DRV_ADC_SOFT_RESET_INACTIVE /*!< Max value for boundary checks.*/
} DRV_ADC_SOFT_RESET_E;

/*!
 * \brief Power-down mode configuration of the ADCs.
 */
typedef enum DRV_ADC_POWER_DOWN_Etag
{
  DRV_ADC_DISABLE = 0u,/*!< Disable ADC (Power-down) (Default).*/
  DRV_ADC_ENABLE = 1u,/*!< Enable ADC (Power-up). */
  DRV_ADC_POWER_DOWN_MIN = DRV_ADC_DISABLE, /*!< Min value for boundary checks.*/
  DRV_ADC_POWER_DOWN_MAX = DRV_ADC_ENABLE /*!< Max value for boundary checks.*/
} DRV_ADC_POWER_DOWN_E;

/*!
 * \brief Dead time delay configuration of the ADCs.
 *
 * Delay in steps of system clock (10ns) between Dead Time EVenT from PWM module and trigger.
 * In case of 2nd DTEVT within delay time, the second DTEVT will be lost.
 */
typedef enum DRV_ADC_DEADTIME_Etag
{
  DRV_ADC_DEADTIME_DEFAULT = 0x000au,/*!< Dead time delay of 100ns (Default).*/
  DRV_ADC_DEADTIME_MIN = 0x0000u,/*!< Min value for boundary checks. */
  DRV_ADC_DEADTIME_MAX = 0xffffu, /*!< Max value for boundary checks. */
} DRV_ADC_DEADTIME_E;

/*!
 * \brief Measurement structure.
 */
typedef struct DRV_ADC_SEQ_MEAS_Ttag
{
  DRV_ADC_SEQ_MEAS_ENABLE_E eEnable; /*!< Enable the measurement.*/
  DRV_ADC_SEQ_MEAS_ADR_OFFSET_E eAdrOffset; /*!< Address offset specified in 16 bit words where the sum will be stored.*/
  DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_E eInputChannel; /*!< Input channel of the measurement.*/
  DRV_ADC_SEQ_MEAS_OVERSAMPLING_E eOversample; /*!< Number of samples for this measurement.*/
  DRV_ADC_SEQ_MEAS_TRIGGER_E eTrigger; /*!< Trigger condition for measurement.*/
} DRV_ADC_SEQ_MEAS_T;

/*!
 * \brief Structure for the buffers used for the sequencers data io.
 */
typedef struct DRV_ADC_SEQ_BUFFER_Ttag
{
  bool fEnable; /*!< Enable the buffer.*/
  void* pvBufferAdr; /*!< Start address of the buffer.*/
  volatile uint32_t ulPosition; /*!< Position of the last sample.*/
  uint32_t ulMaxCount; /*!< Max. count of buffer elements.*/
  DRV_CALLBACK_F fnWrapCallback; /*!< The callback if the series buffer full and next run will coming.*/
  void* pWrapCallbackkHandle; /*!< The handle associated with the ADC series buffer callback.*/
} DRV_ADC_SEQ_BUFFER_T;

/*!
 * \brief Structure containing the state of the adc sequencer.
 */
typedef struct DRV_ADC_STATE_Ttag
{
  DRV_ADC_STATE_MEASUREMENT_E eActiveMeasurement; /*!< Contains the enumeration of the measurements to use.*/
  uint8_t ulHalfClockCycle; /*!< Contains the count of the half clock cycle.*/
} DRV_ADC_STATE_T;

/*!
 * \brief The configuration of the driver.
 *
 * This structure shall be modified to change configuration parameters.
 * It is evaluated during the init routine and used during the handles lifetime.
 * The structure shall not be modified after initialization of the device.
 */
typedef struct DRV_ADC_SEQ_CONFIGURATION_Ttag
{
  DRV_ADC_SEQ_DEVICE_ID_E eDeviceID;        /*!< The device to be used*/
  DRV_OPERATION_MODE_E eOperationMode;      /*!< Which programming method (DMA/IRQ/POLL) is used.*/
  DRV_ADC_SEQ_DMA_MODE_E eDmaModeDisable;   /*!< Enable/Disable DMA mode for coping of results.*/
  DRV_ADC_SEQ_32Bit_MODE_E e32BitMode;      /*!< Use 32Bit increment (fast, more space) or 16Bit increments (slower because of read modify write but less space).*/
  DRV_ADC_SEQ_VREF_VDD3_E eVrefVdd3;        /*!< Reference Select of ADC */
  DRV_ADC_SEQ_CLK_SYNC_E eClkSync;          /*!< Independent or synchronous clock. */
  DRV_ADC_SEQ_CLK_PHASE_E eClkPhase;        /*!< Shall be set to default value. Generation of the rising edge of the adcclk is delayed until the global clk_phase counter matches this value.\par
                                                 NOTE: The rising edge of the adcclk ending the first sample period of a triggered measurement is NEVER delayed. */
  DRV_ADC_SEQ_CLK_PERIOD_E eClkPeriod;      /*!< Shall be set to default value. Duration of an adcclk period in system clock cycles-1
                                                 For odd values the high phase of adcclk is one system clock cycle longer than the low phase. */
  DRV_ADC_SEQ_CHANNEL_TRACKING_TIME_E eChannelTrackingTime[DRV_ADC_INPUTS_MAX];
                                            /*!< ADC sample extension for input channel. Length of 2nd adcclk in steps of 10ns system clock(~ delay of 3rd adcclk edge).
                                                 The capacitor inside ADC needs time to be charged depending on the driving strength of the external signal.
                                                 For 12 bit precision, a settling to time of 9 tau should be ensured.
                                                 tau can be calculated by tau = 9*(ADC_MUX_RSER + Rext) * ADC_MUX_CIN,
                                                 with Rext being additional resistance from devices external to the SOC.
                                                 The setting of tt_add also depends on the setting of madc_seq_cfg.adcclk_period and can be calculate by the following formula:
                                                 tt_add = ceil( tau/10ns - madc_seq_cfg.adcclk_period ) - 2
                                                 Note:
                                                 madc_seq_cfg.adcclk_period and madc_seq_tracking_time_mux[x].tt_add will count down to 0
                                                 meaning that the value set in the registers is increased by 1.
                                                 To compensate this the tt_add formula includes a '-2'.
                                                 Note: ADC_MUX_RSET(1.5kOhm) and ADC_MUX_CIN(10pF) are defined in the electrical parameters.
                                                 If the ADC is driven by low-ohmic shunt resistor (Rext = 200mOhm), and adcclk_period = 2, the tt_add is:
                                                 tt_add = ceil(9 * (1.5 kOhm + 0.2 Ohm) * 10 pF / 10 ns - 2) - 2
                                                 tt_add = ceil(135 ns / 10 ns - 2 - 2) = 10
                                                 Set tt_add=10 if calculated value is smaller than 10. */
  DRV_ADC_SEQ_CONTINUOUS_MODE_E eContinuousMode;/*!< Defines if the sequencer captures continuously or once.*/
  void* pvBaseAdr;                              /*!< Base address for writing the measurement results.*/
  DRV_ADC_SEQ_MEAS_T tMeasurement[DRV_ADC_MEASUREMENTS_MAX];/*!< Every Sequencer can includes up to DRV_ADC_MEASUREMENTS_MAX measurements.*/
  DRV_CALLBACK_F fnSequenceCompleteCallback;/*!< The callback used if a ADC sequence conversion complete*/
  void* pSequenceCompleteCallbackHandle;    /*!< The handle associated with the ADC sequence conversion complete callback.*/
  DRV_CALLBACK_F fnMeasurementCompleteCallback[DRV_ADC_MEASUREMENTS_MAX];   /*!< The callback used if a single measurement conversion complete*/
  void* pMeasurementCompleteCallbackHandle[DRV_ADC_MEASUREMENTS_MAX];       /*!< The handle associated with the single measurement conversion complete callback.*/
} DRV_ADC_SEQ_CONFIGURATION_T;

/*!
 * brief Type definition of DRV_ADC_HANDLE_Ttag  DRV_ADC_HANDLE_T
 */
typedef struct DRV_ADC_HANDLE_Ttag  DRV_ADC_HANDLE_T;

/*!
 * brief Type definition of DRV_ADC_SEQ_HANDLE_Ttag  DRV_ADC_SEQ_HANDLE_T
 */
typedef struct DRV_ADC_SEQ_HANDLE_Ttag  DRV_ADC_SEQ_HANDLE_T;


/*!
 * \brief Type definition of the separate ADC configuration structure.
 */
typedef struct DRV_ADC_STATIC_CFG_Ttag
{
  DRV_ADC_SOFT_RESET_E eSoftReset; /*!< Activate/Deactivate the Soft-Reset functionality of the ADC.*/
  DRV_ADC_POWER_DOWN_E ePowerDown; /*!< Enable/Disable the Power-down mode of the ADC. */
} DRV_ADC_STATIC_CFG_T;

/*!
 * \brief The Sequence-Handle related to actual conversion functions.
 *
 * This handle is used for functions like DRV_ADC_Seq_Init(), DRV_ADC_Seq_GetState(), DRV_ADC_Seq_GetSample().
 * The default configuration using DRV_ADC_SEQ_INITIALIZER might need to be modified before
 * calling the _Init-function and shall not be changed afterwards.
 * \struct DRV_ADC_SEQ_HANDLE_T
 * \struct DRV_ADC_SEQ_HANDLE_Ttag
 * \sa DRV_ADC_HANDLE_T
 */
typedef struct DRV_ADC_SEQ_HANDLE_Ttag
{
  DRV_ADC_SEQ_DEVICE_T* ptDevice; /*!< \private The ADC device register as bitfield and value unions.*/
  DRV_ADC_HANDLE_T* ptParent; /*!< \private Pointer to the adc device handle. */
  DRV_ADC_SEQ_CONFIGURATION_T tConfiguration; /*!< Contains the configuration associated with the sequencer.*/
  DRV_LOCK_T tLock; /*!< \private The drivers locking variable used as internal mutex*/
  DRV_ADC_SEQ_BUFFER_T tBuffer; /*!< \private Contains the pointer and position to record the next sample.*/
} DRV_ADC_SEQ_HANDLE_T;

/*!
 * \brief The configuration of the driver.
 *
 * This structure shall be modified to change configuration parameters.
 * It is evaluated during the init routine and used during the handles lifetime.
 * The structure shall not be modified after initialization of the device.
 */
typedef struct DRV_ADC_CONFIGURATION_Ttag
{
  DRV_ADC_CLK_PERIOD_E eAdcClckPeriod; /*!< This value is used to configure a global counter for clock phase reference. It's value should be the same or an natural numbered multiple of the value configured in ADC sequencers.*/
  DRV_ADC_VREF_BUFFER_E eVrefBufferEnable; /*!< Defines if external reference voltage is used or the internal.*/
  DRV_ADC_STATIC_CFG_T tStaticCfg01; /*!< Static configuration signals for ADC0 and ADC1. */
  DRV_ADC_STATIC_CFG_T tStaticCfg23; /*!< Static configuration signals for ADC2 and ADC3. */
  uint16_t auDeadTimeDelay[6];/*!< Defines the dead time delay..*/
} DRV_ADC_CONFIGURATION_T;

/*!
 * \brief The Main-Handle used by DRV_ADC_Driver_Init(),  DRV_ADC_Driver_DeInit() and some other functions.
 * \struct DRV_ADC_HANDLE_T
 * \struct DRV_ADC_HANDLE_Ttag
 * The configuration SHALL be changed before initializing the device and shall not be changed afterwards.
 * The rest of it SHALL not be modified outside of the driver, even if it appears to be possible.
 * \sa DRV_ADC_SEQ_HANDLE_T
 */
typedef struct DRV_ADC_HANDLE_Ttag
{
  DRV_ADC_DEVICE_T* ptDevice; /*!< \private The ADC device register as bitfield and value unions.*/
  DRV_ADC_SEQ_HANDLE_T* aptAdcSequencer[4]; /*!< Pointer to the sequencers initiated. */
  DRV_ADC_CONFIGURATION_T tConfiguration; /*!< Contains the configuration of all ADC sequencers.*/
  DRV_LOCK_T tLock; /*!< \private The drivers locking variable used as internal mutex*/
} DRV_ADC_HANDLE_T;

/*!
 * \brief Initializes the adc device and handle by the given configuration.
 * \memberof DRV_ADC_HANDLE_T
 */
DRV_STATUS_E DRV_ADC_Driver_Init(DRV_ADC_HANDLE_T * const ptDriver);
/*!
 * \brief Deinitializes the adc device and handle.
 * \memberof DRV_ADC_HANDLE_T
 */
DRV_STATUS_E DRV_ADC_Driver_DeInit(DRV_ADC_HANDLE_T * const ptDriver);
/*!
 * \brief Starts the sequencers given by the mask (software trigger).
 * \memberof DRV_ADC_HANDLE_T
 */
DRV_STATUS_E DRV_ADC_Start(DRV_ADC_HANDLE_T * const ptDriver, DRV_ADC_SEQ_DEVICE_MSK_E eSequencerMask);
/*!
 * \brief Stops the sequencers given by the mask.
 * \memberof DRV_ADC_HANDLE_T
 */
DRV_STATUS_E DRV_ADC_Stop(DRV_ADC_HANDLE_T * const ptDriver, DRV_ADC_SEQ_DEVICE_MSK_E eSequencerMask);
/*!
 * \brief Initializes the adc sequencer and its handle by the given configuration.
 * \memberof DRV_ADC_SEQ_HANDLE_T
 */
DRV_STATUS_E DRV_ADC_Seq_Init(DRV_ADC_SEQ_HANDLE_T * const ptSequencer, DRV_ADC_HANDLE_T * const ptDriver);
/*!
 * \brief Deinitializes the adc sequencer and its handle.
 * \memberof DRV_ADC_SEQ_HANDLE_T
 */
DRV_STATUS_E DRV_ADC_Seq_DeInit(DRV_ADC_SEQ_HANDLE_T * const ptSequencer);
/*!
 * \brief Set mode of the sequencer
 * \memberof DRV_ADC_SEQ_HANDLE_T
 */
DRV_STATUS_E DRV_ADC_Seq_SetMode(DRV_ADC_SEQ_HANDLE_T * const ptSequencer, DRV_ADC_SEQ_CONTINUOUS_MODE_E eMode);
/*!
 * \brief Starts the given adc sequencer.
 * \memberof DRV_ADC_SEQ_HANDLE_T
 */
DRV_STATUS_E DRV_ADC_Seq_Start(DRV_ADC_SEQ_HANDLE_T * const ptSequencer);
/*!
 * \brief Stops the given adc sequencer.
 * \memberof DRV_ADC_SEQ_HANDLE_T
 */
DRV_STATUS_E DRV_ADC_Seq_Stop(DRV_ADC_SEQ_HANDLE_T * const ptSequencer);
/*!
 * \brief Reset the given adc sequencer.
 * \memberof DRV_ADC_SEQ_HANDLE_T
 */
DRV_STATUS_E DRV_ADC_Seq_Reset(DRV_ADC_SEQ_HANDLE_T * const ptSequencer);
/*!
 * \brief Changes the parameters of the given adc sequencer measurement.
 * \memberof DRV_ADC_SEQ_HANDLE_T
 */
DRV_STATUS_E DRV_ADC_Seq_Meas_ChangeConfig(DRV_ADC_SEQ_HANDLE_T * const ptSequencer, DRV_ADC_SEQ_MEASUREMENT_E eMeasurement,
  DRV_ADC_SEQ_MEAS_INPUT_CHANNEL_E eInputChannel, DRV_ADC_SEQ_MEAS_OVERSAMPLING_E eOversampling, DRV_ADC_SEQ_MEAS_TRIGGER_E eTrigger);
/*!
 * \brief Aquires the state of the given adc sequencer.
 * \memberof DRV_ADC_SEQ_HANDLE_T
 */
DRV_STATUS_E DRV_ADC_Seq_GetState(DRV_ADC_SEQ_HANDLE_T * const ptSequencer, DRV_ADC_STATE_T * const ptState);
/*!
 * \brief Stores the current acquired sample of the  the given adc.
 * \memberof DRV_ADC_SEQ_HANDLE_T
 */
DRV_STATUS_E DRV_ADC_Seq_GetSample(DRV_ADC_SEQ_HANDLE_T * const ptSequencer, uint32_t* const pulSample, uint8_t* const pbMeasNr);
/*!
 * \brief Stores the last sample of the  the given adc.
 * \memberof DRV_ADC_SEQ_HANDLE_T
 */
DRV_STATUS_E DRV_ADC_Seq_GetLastSample(DRV_ADC_SEQ_HANDLE_T * const ptSequencer, uint32_t* const pulSample, uint8_t* const pbMeasNr);
/*!
 * \brief Stores a series of aquired sample of the given adc.
 * \memberof DRV_ADC_SEQ_HANDLE_T
 */
DRV_STATUS_E DRV_ADC_Seq_SetSampleSeriesBuffer(DRV_ADC_SEQ_HANDLE_T * const ptSequencer, uint16_t* pulSampleSeries, size_t tSize,
  DRV_CALLBACK_F fnCallback, void* pUser);

/*! \} *//* End of group ADC */

/*! \} *//* End of group netx_Driver */

#ifdef __cplusplus
}
#endif

#endif /* NETX_DRV_ADC_H_ */

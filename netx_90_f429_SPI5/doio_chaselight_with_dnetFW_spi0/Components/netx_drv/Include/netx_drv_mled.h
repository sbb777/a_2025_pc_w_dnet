/*!************************************************************************//*!
 * \file    netx_drv_mled.h
 * \brief   Header file of MLED DRV module.
 * $Revision: 11200 $
 * $Date: 2024-03-15 18:01:34 +0200 (Fri, 15 Mar 2024) $
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
#ifndef NETX_DRV_MLED_H_
#define NETX_DRV_MLED_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*! \addtogroup netX_Driver
 * \{
 */

/*!
 * \addtogroup MLED
 * \{
 */

/*!***************************************************************************/
/*!
 * \brief Type definition analog to the dio definition for the mleds as mask
 */
typedef uint32_t DRV_MLED_MSK_T;

/*!
 * \brief Type definition analog to the dio definition for the mleds by id
 */
typedef uint32_t DRV_MLED_ID_T;

/*!
 * \brief The MLED prescale counter border as f_pwm = f_clk / (512 * (prescale_counter_max + 1))
 */
typedef enum DRV_MLED_PRESCALE_BORDER_Etag
{
  DRV_MLED_PRESCALE_BORDER_RESET = 0x00ul,/*!< The lowest border.*/
  DRV_MLED_PRESCALE_BORDER_1 = 0x01ul,/*!< The lowest border.*/
  DRV_MLED_PRESCALE_BORDER_DEFAULT = 0x400ul,/*!< The lowest border.*/
  DRV_MLED_PRESCALE_BORDER_1024 = 0x400ul,/*!< The lowest border.*/
  DRV_MLED_PRESCALE_BORDER_MIN = DRV_MLED_PRESCALE_BORDER_1, /*!< Min value for border checking.*/
  DRV_MLED_PRESCALE_BORDER_MAX = DRV_MLED_PRESCALE_BORDER_1024,/*!< Max value for border ckecking.*/
} DRV_MLED_PRESCALE_BORDER_E;

/*!
 * \brief The MLED blink counter border as f_blink = 50 Hz / (blink_counter_border)
 */
typedef enum DRV_MLED_BLINK_BORDER_Etag
{
  DRV_MLED_BLINK_BORDER_RESET = 0x00ul,/*!< The lowest border.*/
  DRV_MLED_BLINK_BORDER_1 = 0x01ul,/*!< The lowest border.*/
  DRV_MLED_BLINK_BORDER_DEFAULT = 0x32ul,/*!< The lowest border.*/
  DRV_MLED_BLINK_BORDER_512 = 0x200ul,/*!< The lowest border.*/
  DRV_MLED_BLINK_BORDER_MIN = DRV_MLED_BLINK_BORDER_1, /*!< Min value for border checking.*/
  DRV_MLED_BLINK_BORDER_MAX = DRV_MLED_BLINK_BORDER_512,/*!< Max value for border ckecking.*/
} DRV_MLED_BLINK_BORDER_E;

/*!
 * \brief The MLED brightness duty=([value]-1)/255
 */
typedef enum DRV_MLED_BRIGHTNESS_Etag
{
  DRV_MLED_BRIGHTNESS_RESET = 0x00ul,/*!< The value is not modified, so in reset is is reset or if it was changed it is not modified.*/
  DRV_MLED_BRIGHTNESS_1 = 0x01ul,/*!< The darkest(off).*/
  DRV_MLED_BRIGHTNESS_OFF = DRV_MLED_BRIGHTNESS_1,/*!< The darkest(off).*/
  DRV_MLED_BRIGHTNESS_256 = 0x100ul,/*!< The brightest.*/
  DRV_MLED_BRIGHTNESS_FULL = DRV_MLED_BRIGHTNESS_256,/*!< The brightest.*/
  DRV_MLED_BRIGHTNESS_DEFAULT = DRV_MLED_BRIGHTNESS_256,/*! The default brightness is 255*/
  DRV_MLED_BRIGHTNESS_MIN = DRV_MLED_BRIGHTNESS_1, /*!< Min value for border checking.*/
  DRV_MLED_BRIGHTNESS_MAX = DRV_MLED_BRIGHTNESS_256,/*!< Max value for border ckecking.*/
} DRV_MLED_BRIGHTNESS_E;

/*!
 * \brief The MLED modes
 */
typedef enum DRV_MLED_MODE_Etag
{
  DRV_MLED_MODE_STANDARD = 0x00ul,/*!< The invert, set and reset will change the state of the mled.*/
  DRV_MLED_MODE_LINE_REGISTER = 0x01ul,/*!< Sets the mled line mode to act after the line register.*/
  DRV_MLED_MODE_BLINK = 0x02ul,/*!< Puts the mled channels to blink.*/
  DRV_MLED_MODE_BYPASS = 0x03ul,/*!< Bypasses the pwm and puts the pin to the (not)inverted level.*/
  DRV_MLED_MODE_MAX = DRV_MLED_MODE_BYPASS,/*!< Max value for border ckecking.*/
} DRV_MLED_MODE_E;

/*!
 * \brief The configuration of the driver.
 *
 * This structure shall be modified to change configuration parameters.
 * It is evaluated during the init routine and used during the handles lifetime.
 * The structure shall not be modified after initialization of the device.
 */
typedef struct DRV_MLED_CONFIGURATION_Ttag
{
  DRV_MLED_PRESCALE_BORDER_E ePrescaleBorder;/*!< This defines the frequency of the mled pwm.*/
  DRV_MLED_BLINK_BORDER_E eBlinkBorder;/*!< This value defines the speed of the blinking mode.*/
  DRV_MLED_BRIGHTNESS_E eBrightnessDefault;/*!< This value is used as the default brightness of all MLEDs.*/
} DRV_MLED_CONFIGURATION_T;

/*!
 * \brief The handle of the driver.
 *
 * The configuration SHALL be changed before initializing the device and shall not be changed
 * afterwards.
 * The rest of it SHALL not be modified outside of the driver, even if it appears to be possible.
 * \struct DRV_MLED_HANDLE_T
 * \struct DRV_MLED_HANDLE_Ttag
 */
typedef struct DRV_MLED_HANDLE_Ttag
{
  DRV_MLED_DEVICE_T* ptDevice; /*!< \private The device register as bitfield and value unions.*/
  DRV_MLED_CONFIGURATION_T tConfiguration; /*!< Contains the configuration of the device.*/
  DRV_LOCK_T tLock; /*!< \private The drivers locking variable used as internal mutex. */
} DRV_MLED_HANDLE_T;

/*!
 * \brief Initializes the mled device and handle by the given configuration.
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_Init(DRV_MLED_HANDLE_T * const ptDriver);

/*!
 * \brief Deinitializes the mled device and handle.
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_DeInit(DRV_MLED_HANDLE_T * const ptDriver);

/*!
 * \brief Sets the mode of the MLED.
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_ChannelSetMode(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_ID_T uMledID, DRV_MLED_MODE_E eMode);

/*!
 * \brief Sets the brightness of the MLED.
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_ChannelSetBrightness(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_ID_T uMledID, DRV_MLED_BRIGHTNESS_E uBrightness);
/*!
 * \brief Inverts the MLED.
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_ChannelInvert(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_ID_T uMledID);

/*!
 * \brief Sets the MLED to set.
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_ChannelOutSet(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_ID_T uMledID);

/*!
 * \brief Sets the MLED to reset.
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_ChannelOutReset(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_ID_T uMledID);

/*!
 * \brief Manipulates the mled line and sets the bits given by the mask to the specified value.
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_LineMaskedSet(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_MSK_T uMask, DRV_MLED_MSK_T uMledMSK);

/*!
 * \brief Manipulates the MLED line and sets the given bits.
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_LineOutSet(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_MSK_T uMledMSK);

/*!
 * \brief Manipulates the MLED line and resets the given bits.
 * \memberof DRV_MLED_HANDLE_T
 */
DRV_STATUS_E DRV_MLED_LineOutReset(DRV_MLED_HANDLE_T * const ptDriver, DRV_MLED_MSK_T uMledMSK);


/*! \} *//* End of group MLED */

/*! \} *//* End of group netx_Driver */

#ifdef __cplusplus
}
#endif

#endif /* NETX_DRV_MLED_H_ */

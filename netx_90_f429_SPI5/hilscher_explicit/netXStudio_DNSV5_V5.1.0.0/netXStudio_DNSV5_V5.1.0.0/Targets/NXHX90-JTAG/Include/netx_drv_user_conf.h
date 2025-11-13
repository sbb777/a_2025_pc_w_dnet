/**************************************************************************//**
 * \file    user_drv_conf.h
 * \brief   DRV configuration file.
 * $Revision: 4539 $
 * $Date: 2018-11-22 17:27:59 +0100 (Do, 22 Nov 2018) $
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


/*!
 * Content of the user driver configuration file.
 */
#ifndef TARGETS_DRV_CONF_H_
#define TARGETS_DRV_CONF_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * This file contains a template for the drivers configuration options.
 * Everything is enabled and configured with default settings.
 */

/*!
 * Choose your weapon!
 */
#define DRV_DEVICE_NETX_90
/*!
 * Choose your weapon!
 */
//#define DRV_DEVICE_NETX_90_MPW

/*!
 * Because the consistency checks take some time each driver api call, one might
 * want to deactivate them by defining this macro.
 */
//#define DRV_HANDLE_CHECK_INACTIVE

/*!
 * Define that enables the compilation of the cortex driver and its inclusion into the
 * headers.
 */
#define DRV_CORTEX_MODULE_ENABLED

/*!
 * Define that enables the compilation of the dio driver and its inclusion into the
 * headers.
 */
#define DRV_DIO_MODULE_ENABLED

/*!
 * Define that enables the compilation of the spi driver and its inclusion into the
 * headers.
 */
#define DRV_SPI_MODULE_ENABLED

/*!
 * Define that enables the compilation of the uart driver and its inclusion into the
 * headers.
 */
#define DRV_UART_MODULE_ENABLED

/*!
 * Define that enables the compilation of the tim driver and its inclusion into the
 * headers.
 */
#define DRV_TIM_MODULE_ENABLED

/*!
 * Define that enables the compilation of the biss driver and its inclusion into the
 * headers.
 */
#define DRV_BISS_MODULE_ENABLED

/*!
 * Define that enables the compilation of the adc driver and its inclusion into the
 * headers.
 */
#define DRV_ADC_MODULE_ENABLED

/*!
 * Define that enables the compilation of the ethernet xpic driver and its inclusion into the
 * headers.
 */
#define DRV_ETH_XPIC_MODULE_ENABLED

/*!
 * Define that enables the compilation of the xpic driver and its inclusion into the
 * headers.
 */
#define DRV_XPIC_MODULE_ENABLED

/*!
 * Define that enables the compilation of the MLED driver and its inclusion into the
 * headers.
 */
#define DRV_MLED_MODULE_ENABLED

/*!
 * Define that enables the compilation of the dmac driver and its inclusion into the
 * headers.
 */
#define DRV_DMAC_MODULE_ENABLED

/*!
 * The length of the dmac linked list used in the driver.
 * One element is capable of 4095 transfers.
 */
#define DRV_DMAC_LIST_LENGTH 4u

#ifdef __cplusplus
}
#endif

#endif /* TARGETS_DRV_CONF_H_ */

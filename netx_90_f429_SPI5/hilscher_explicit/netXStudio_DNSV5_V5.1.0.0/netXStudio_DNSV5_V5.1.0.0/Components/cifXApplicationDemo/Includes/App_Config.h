/**************************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

Exclusion of Liability for this demo software:
  The following software is intended for and must only be used for reference and in an
  evaluation laboratory environment. It is provided without charge and is subject to
  alterations. There is no warranty for the software, to the extent permitted by
  applicable law. Except when otherwise stated in writing the copyright holders and/or
  other parties provide the software "as is" without warranty of any kind, either
  expressed or implied.
  Please refer to the Agreement in README_DISCLAIMER.txt, provided together with this file!
  By installing or otherwise using the software, you accept the terms of this Agreement.
  If you do not agree to the terms of this Agreement, then do not install or use the
  Software!

**************************************************************************************************/
#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

/** Used communication channels
 *
 * This define controls how many communication channels are are processed by the application.
 * The handling of the channels is delegated to APP_COMM_CHANNEL_HANDLERs which are linked via
 * hard coded variable names.
 * Channel 0: Handled by g_tRealtimeProtocolHandler
 * Channel 1: Handled by g_tNetworkServicesHandlers
 * Channel 2: Handled by g_tNetProxyHandlers
 *
 * Note: If you want to have different value per target, you can set this define also in the
 *       related build target e.g.:
 *          bld.program (target = "Simple_nx90_app_usecase_a_sdram",
 *                       ...
 *                       defines = [USED_COMMUNICATION_CHANNELS=2], # This demo uses two channels
 *                       ...
 */
#ifndef USED_COMMUNICATION_CHANNELS
  #define USED_COMMUNICATION_CHANNELS   1
#endif


/** Suppresses demo print outs
 *
 * If set the demo application will not print informations about the performed steps
 * of the application.
 *
 * Note: These print outs may impact the performance of the demo application.
 */
//#define DEMO_QUIET /* Comment in to disable printouts globally */

#ifndef DEMO_QUIET
  #include <stdio.h>
  #define PRINTF printf
#else
  #define PRINTF(...)
#endif


/** Defines a printf function for packet trace
 *
 * Note: you can comment out this line to deactivate packet communication print (trace) only
 * */
#define PKT_PACKET_TRACE_PRINTF PRINTF




/** Use windows style new lines */
#define NEWLINE "\r\n"



#endif /** APP_CONFIG_H_ */

/**************************************************************************************
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
**************************************************************************************/

/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************/

#ifndef DNS_DEMOAPPLICATION_CONFIG_H_
#define DNS_DEMOAPPLICATION_CONFIG_H_

/*************************************************************************************/
/*************************************************************************************/
/* #define DNS_HOST_APP_REGISTRATION */


/*************************************************************************************
 Enable this macro enable the print out option of the OMB host example.
 Per default it is disabled. For debugging and development it can be enabled.
*************************************************************************************/
#define DNS_PRINTF_ENABLE

#ifdef DNS_PRINTF_ENABLE
  #include <stdio.h>
  #define DNS_PRINTF printf
#else
  #define DNS_PRINTF(...)
#endif

/*************************************************************************************
 Enable this macro enable the terminal handler for command execution.
 Per default it is disabled. For debugging and development it can be enabled.
*************************************************************************************/
#define DNS_HOST_APP_TERMINAL_COMMAND_ENABLE


#endif /* DNS_DEMOAPPLICATION_CONFIG_H_ */

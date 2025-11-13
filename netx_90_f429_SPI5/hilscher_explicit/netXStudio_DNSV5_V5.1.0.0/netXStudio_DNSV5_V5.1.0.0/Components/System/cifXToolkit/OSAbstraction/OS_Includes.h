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

***************************************************************************************

  $Id: OS_Includes.h 6603 2014-10-02 14:57:53Z stephans $:

  Description:
    Headerfile for specific target system includes, data types and definitions

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2006-08-08  initial version (special OS dependencies must be added)

**************************************************************************************/

#ifndef __OS_INCLUDES__H
#define __OS_INCLUDES__H

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "cmsis_gcc.h"

/* Ignore redundant declarations to suppress warning caused by
   redundant declaration in cifXToolkit.h and cifXHWFunctions.h */
#pragma GCC diagnostic ignored "-Wredundant-decls"

#define min(a,b) ((a<b)?a:b)
#define max(a,b) ((a<b)?b:a)
#define UNREFERENCED_PARAMETER(x) (x=x)
#define APIENTRY

#endif /* __OS_INCLUDES__H */

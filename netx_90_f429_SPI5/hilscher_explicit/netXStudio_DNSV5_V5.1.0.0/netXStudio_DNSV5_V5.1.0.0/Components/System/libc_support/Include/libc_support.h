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

   Copyright (c) Hilscher GmbH. All Rights Reserved.

 **************************************************************************************

   Filename:
    $Workfile: libc_file_support.h $
   Last Modification:
    $Author: dirk $
    $Modtime: 1.02.07 17:11 $
    $Revision: 2135 $

   Targets:
     Win32/ANSI   : yes
     Win32/Unicode: yes (define _UNICODE)
     WinCE        : no

   Description:
   libc_file_support.h  Registration of debug output drivers for libc printf, etc.

   Changes:

     Version   Date        Author   Description
     ----------------------------------------------------------------------------------

**************************************************************************************/

/*****************************************************************************/
/*! \file libc_file_support.h
*   Registration of debug output drivers for libc printf, etc.               */
/*****************************************************************************/


#ifndef COMPONENTS_LIBC_SUPPORT_LIBC_SUPPORT_H_
#define COMPONENTS_LIBC_SUPPORT_LIBC_SUPPORT_H_

#include <stdio.h>
#include "libc_file_support.h"
#include "libc_mem_support.h"

void libcInit(void);


#endif /* COMPONENTS_LIBC_SUPPORT_LIBC_SUPPORT_H_ */

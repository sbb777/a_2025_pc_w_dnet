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
    $Workfile: libc_mem_support.h $
   Last Modification:
    $Author: dirk $
    $Modtime: 1.02.07 17:08 $
    $Revision: 2135 $
   
   Targets:
     Win32/ANSI   : yes
     Win32/Unicode: yes (define _UNICODE)
     WinCE        : no
 
   Description:
   libc_mem_support.h  Operating system specific functions for libc memory support (malloc, ...)
       
   Changes:
 
     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
      1        07.12.2006  MT       initial version
 
**************************************************************************************/

/*****************************************************************************/
/*! \file libc_mem_support.h                                                    
*   Operating system specific functions for libc memory support (malloc, ...)*/
/*****************************************************************************/

#ifndef __LIBC_MEM_SUPPORT__H
#define __LIBC_MEM_SUPPORT__H

void libc_mem_init();

#endif /* __LIBC_MEM_SUPPORT__H */

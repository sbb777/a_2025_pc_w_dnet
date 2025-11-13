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
    $Workfile: libc_mem_support.c $
   Last Modification:
    $Author: dirk $
    $Modtime: 17.07.09 11:02 $
    $Revision: 2135 $

   Targets:
     Win32/ANSI   : yes
     Win32/Unicode: yes (define _UNICODE)
     WinCE        : no

   Description:
   libc_mem_support.c  Operating system specific functions for libc memory support (malloc, ...)

   Changes:

     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
      4        28.01.2010  MS       Changed:
                                      - Changed include sequence to eliminate compiler warning
                                        regarding printf redefinition.
      3        17.07.2009  MT       Changed:
                                      - Introduced global variables g_pchHeapStart / g_pchHeapMAx
                                        to changed heap after startup (e.g. NXF File that now runs
                                        on a different sized SDRAM)
      2        05.02.2007  MT       Changed:
                                      - goto start label caused malloc/free to crash, due to
                                        uninitialized variables in libc (now initialized
                                        via external references)
      1        07.12.2006  MT       initial version

**************************************************************************************/

/*****************************************************************************/
/*! \file libc_mem_support.c
*   Operating system specific functions for libc memory support (malloc, ...)*/
/*****************************************************************************/

#include <sys/types.h>
#include <sys/reent.h>

#ifdef _DEBUG
#include <stdio.h>
#endif

#include <malloc.h>
#include <string.h>

/*****************************************************************************/
/* External variables from malloc implementation, to allow soft resets, where
*  static variables are not reset on rcX                                     */
/*****************************************************************************/
struct malloc_chunk
{
  size_t prev_size; /* Size of previous chunk (if free). */
  size_t size;      /* Size in bytes, including overhead. */
  struct malloc_chunk* fd;   /* double links -- used only if free. */
  struct malloc_chunk* bk;
};

#define NAV             128   /* number of bins */

#define DEFAULT_TRIM_THRESHOLD (128L * 1024L)
#define DEFAULT_TOP_PAD        (0)

typedef struct malloc_chunk* mbinptr;
#define SIZE_SZ        sizeof(size_t)
#define bin_at(i)      ((mbinptr)((char*)&(av_[2*(i) + 2]) - 2*SIZE_SZ))
#define IAV(i)         bin_at(i), bin_at(i)

extern char*            __malloc_sbrk_base;
extern unsigned long    __malloc_max_sbrked_mem;
extern unsigned long    __malloc_max_total_mem;
extern struct mallinfo  __malloc_current_mallinfo;
extern unsigned long    __malloc_trim_threshold;
extern unsigned long    __malloc_top_pad;
extern mbinptr          __malloc_av_[NAV * 2 + 2];

/*****************************************************************************/


extern char  __HeapBase;    /* Defined by the linker */
extern char  __HeapLimit;   /* Defined by the linker */

char*        g_pchHeapStart;
char*        g_pchHeapMax;
static char* s_pchHeapCurrentEnd;

/*****************************************************************************/
/*! Initialize memory libC support                                           */
/*****************************************************************************/
void libc_mem_init()
{
  int iIdx;
  
  g_pchHeapStart      = &__HeapBase;
  g_pchHeapMax        = &__HeapLimit;

  __malloc_sbrk_base      = (char*)-1;
  __malloc_max_sbrked_mem = 0;
  __malloc_max_total_mem  = 0;
  memset(&__malloc_current_mallinfo, 0, sizeof(__malloc_current_mallinfo));

  __malloc_trim_threshold = DEFAULT_TRIM_THRESHOLD;
  __malloc_top_pad        = DEFAULT_TOP_PAD;

  /* first two elements always NULL*/
  __malloc_av_[0] = NULL;
  __malloc_av_[1] = NULL;

  for(iIdx = 2; iIdx < sizeof(__malloc_av_) / sizeof(__malloc_av_[0]); iIdx += 2)
  {
    __malloc_av_[iIdx]     = (mbinptr)&__malloc_av_[iIdx - 2];
    __malloc_av_[iIdx + 1] = (mbinptr)&__malloc_av_[iIdx - 2];
  }

  s_pchHeapCurrentEnd = g_pchHeapStart;
}

/*****************************************************************************/
/*! Non-Reentrant version of increase program data space
*   \param iIncr     Size to increase heap pointer
*   \return previous heap pointer (NULL on error)                            */
/*****************************************************************************/
void * sbrk(ptrdiff_t iIncr)
{
  void * pvBase = (void *)-1;

  if( (s_pchHeapCurrentEnd + iIncr) <= g_pchHeapMax)
  {
    s_pchHeapCurrentEnd += iIncr;
    pvBase = s_pchHeapCurrentEnd;
  }

  return pvBase;
}

/*****************************************************************************/
/*! Reentrant version of increase program data space
*   \param ptReent   Pointer to global data block which holds errno
*   \param iIncr     Size to de-/increase heap pointer
*   \return previous heap pointer (NULL on error)                            */
/*****************************************************************************/
caddr_t _sbrk_r(struct _reent* ptReent, size_t iIncr)
{
  caddr_t pvBase = (caddr_t)-1;


  if( (s_pchHeapCurrentEnd + iIncr) <= g_pchHeapMax )
  {
    pvBase = s_pchHeapCurrentEnd;
    s_pchHeapCurrentEnd += iIncr;
  }

  return pvBase;
}

 /****************************************************************************/
/*! Multithreading lock for malloc routines
*   \param ptReent Pointer to global data block which holds errno         */
/*****************************************************************************/
 void __malloc_lock(struct _reent* ptReent)
{
}

 /****************************************************************************/
/*! Multithreading unlock for malloc routines
*   \param ptReent      Pointer to global data block which holds errno         */
/*****************************************************************************/
 void __malloc_unlock(struct _reent* ptReent)
{
}

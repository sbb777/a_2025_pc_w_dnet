/*!************************************************************************//*!
 * \file    netx_drv_undef_cpp_conflicted_defines.h
 * \brief   Header file for resolving conflicts between register definition file netx90_app.h
 * and standard c++ libraries
 * $Revision: 8126 $
 * $Date: 2020-08-28 20:18:26 +0200 (Fr, 28 Aug 2020) $
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

/* Define to prevent recursive inclusion */
#ifndef NETX_DRV_CPP_UNDEFS
#define NETX_DRV_CPP_UNDEFS

/* A sequence of undefs for netx90_app.h defines, known to have conflicts with c++ standard libraries */

/* If compilation without redefinition warnings is prefered, NO_CPP_REDEFINITION_WARNINGS has to be defined. */
#ifdef NO_CPP_REDEFINITION_WARNINGS
#undef hash
#undef random
#else
#define hash hash
#define random random
#endif

/* If the former defines are still needed, they can be used with new, unique names
 * #define netx90_drv_hash    ((hash_Type*)   hash_BASE)
 * #define netx90_drv_random  ((random_Type*) random_BASE)
 */

#endif

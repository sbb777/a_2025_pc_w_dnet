/**************************************************************************//**
 * @file    netx90_app_hboot_header.c
 * @brief   netx90 app hboot header
 * $Revision: 6633 $
 * $Date: 2019-12-19 12:11:59 +0100 (Do, 19 Dez 2019) $
 ******************************************************************************/

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


 **************************************************************************************/
#include <stdio.h>

#include "netx_drv.h"
#include "Hil_FileHeaderV3.h"
#include "Hil_SharedDefines.h"

#ifndef HIL_HW_DEV_CLASS_CHIP_NETX_90_APP_FOR_COM_USECASE_A
#define HIL_HW_DEV_CLASS_CHIP_NETX_90_APP_FOR_COM_USECASE_A HIL_HW_DEV_CLASS_CHIP_NETX_90_COM
#endif

#ifndef HIL_HW_DEV_CLASS_CHIP_NETX_90_APP_FOR_COM_USECASE_C
#define HIL_HW_DEV_CLASS_CHIP_NETX_90_APP_FOR_COM_USECASE_C HIL_HW_DEV_CLASS_CHIP_NETX_90_COM_HIFSDR
#endif

#ifndef HIL_HW_ASSEMBLY_OPTION1
#define HIL_HW_ASSEMBLY_OPTION1 HIL_HW_ASSEMBLY_ETHERNET
#endif

#ifndef HIL_HW_ASSEMBLY_OPTION2
#define HIL_HW_ASSEMBLY_OPTION2 HIL_HW_ASSEMBLY_ETHERNET
#endif

#ifndef HIL_HW_ASSEMBLY_OPTION3
#define HIL_HW_ASSEMBLY_OPTION3 HIL_HW_ASSEMBLY_UNDEFINED
#endif

#ifndef HIL_HW_ASSEMBLY_OPTION4
#define HIL_HW_ASSEMBLY_OPTION4 HIL_HW_ASSEMBLY_UNDEFINED
#endif

#ifndef NETX90_EXTERNAL_INTERRUPT_VECTORS_NUMBER
#define NETX90_EXTERNAL_INTERRUPT_VECTORS_NUMBER 96
#endif

NETX_CPU_BOOTBLOCK_T app_cpu_hboot_header_nai __attribute__ ((section (".app_cpu_hboot_boot_header_nai"))) =
{
  .ulMagic = NETX_CPU_BOOTBLOCK_MAGIC,
  .ulFlashOffsetBytes = 0,
  .pulNextHeader = 0,
  .pulDestination = 0,
  .ulImageSizeDword = 0,
  .ulFlashSelection = 0,
  .ulSignature = NETX_CPU_BOOTBLOCK_SIGNATURE,
  .ulReserved0 = 0,
  .aulHash = { 0 },
  .ulBootHeaderChecksum = 0,
};

NETX_CPU_BOOTBLOCK_T app_cpu_hboot_header_nae __attribute__ ((section (".app_cpu_hboot_boot_header_nae"))) =
{
  .ulMagic = NETX_CPU_BOOTBLOCK_MAGIC,
  .ulFlashOffsetBytes = 0,
  .pulNextHeader = 0,
  .pulDestination = 0,
  .ulImageSizeDword = 0,
  .ulFlashSelection = 0,
  .ulSignature = NETX_CPU_BOOTBLOCK_SIGNATURE,
  .ulReserved0 = 0,
  .aulHash = { 0 },
  .ulBootHeaderChecksum = 0,
};

void print_app_hboot_header(NETX_CPU_BOOTBLOCK_T* ptHeader)
{
  printf("\nprint_app_hboot_header\n");
  printf("  Pointer: 0x%08x\n", (uintptr_t) ptHeader);
  printf("  .ulMagic               = 0x%08x\n", (unsigned int) ptHeader->ulMagic);
  printf("  .ulFlashOffsetBytes    = 0x%08x\n", (unsigned int) ptHeader->ulFlashOffsetBytes);
  printf("  .pulNextHeader         = 0x%08x\n", (unsigned int) ptHeader->pulNextHeader);
  printf("  .pulDestination        = 0x%08x\n", (unsigned int) ptHeader->pulDestination);
  printf("  .ulImageSizeDword      = 0x%08x\n", (unsigned int) ptHeader->ulImageSizeDword);
  printf("  .ulFlashSelection      = 0x%08x\n", (unsigned int) ptHeader->ulFlashSelection);
  printf("  .ulSignature           = 0x%08x\n", (unsigned int) ptHeader->ulSignature);
  printf("  .aulHash               = 0x%08x\n", (unsigned int) ptHeader->aulHash[0]);
  printf("  .ulBootHeaderChecksum  = 0x%08x\n", (unsigned int) ptHeader->ulBootHeaderChecksum);
}

HIL_FILE_NAI_NAE_APP_HEADER_V3_0_T app_cpu_header_nai __attribute__ ((section (".app_cpu_header_nai"))) =
{
  .tBootHeader =
  {
    .ulMagicCookie = HIL_FILE_HEADER_FIRMWARE_NAI_COOKIE,
    .ulAppChecksum = 0,
    .ulAppFileSize = 0,
    .ulSignature = HIL_FILE_BOOT_HEADER_SIGNATURE,
    .ulBootHeaderChecksum = 0,
  },
  .tCommonHeader =
  {
    .ulHeaderVersion = HIL_VERSION_COMMON_HEADER_3_0,
    .ulHeaderLength = sizeof(app_cpu_header_nai),
    .ulDataSize = 0,
    .ulDataStartOffset = (16 + NETX90_EXTERNAL_INTERRUPT_VECTORS_NUMBER)*4 + sizeof(app_cpu_hboot_header_nai) + sizeof(app_cpu_header_nai), // 0x02C0
    .bNumModuleInfos = 0,
    .aulMD5 = {0},
    .ulTagListSize = 0,
    .ulTagListOffset = 0,
    .ulTagListSizeMax = 0,
    .ulCommonCRC32 = 0,
    .ulHeaderCRC32 = 0,
  },
  .tDeviceInfo =
  {
    .ulStructVersion = HIL_VERSION_DEVICE_INFO_V1_0,
    .usManufacturer = HIL_MANUFACTURER_HILSCHER_GMBH,
    .usDeviceClass = HIL_HW_DEV_CLASS_CHIP_NETX_90_APP_FOR_COM_USECASE_C,
    .bHwCompatibility = 0,
    .bChipType = HIL_DEV_CHIP_TYPE_NETX90,
    .ausHwOptions = {HIL_HW_ASSEMBLY_OPTION1, HIL_HW_ASSEMBLY_OPTION2, HIL_HW_ASSEMBLY_OPTION3, HIL_HW_ASSEMBLY_OPTION4},
    .ulLicenseFlags1 = 0,
    .ulLicenseFlags2 = 0,
    .usNetXLicenseID = 0,
    .usNetXLicenseFlags = 0,
    .ausFwVersion = {0},
    .ulDeviceNumber = 0,
    .ulFwNumber = 0,
    .ulSerialNumber = 0,
  },
};


HIL_FILE_NAI_NAE_APP_HEADER_V3_0_T app_cpu_header_nae __attribute__ ((section (".app_cpu_header_nae"))) =
{
  .tBootHeader =
  {
    .ulMagicCookie = HIL_FILE_HEADER_FIRMWARE_NAE_COOKIE,
    .ulAppChecksum = 0,
    .ulAppFileSize = 0,
    .ulSignature = HIL_FILE_BOOT_HEADER_SIGNATURE,
    .ulBootHeaderChecksum = 0,
  },
  .tCommonHeader =
  {
    .ulHeaderVersion = HIL_VERSION_COMMON_HEADER_3_0,
    .ulHeaderLength = sizeof(app_cpu_header_nae),
    .ulDataSize = 0,
    .ulDataStartOffset = sizeof(app_cpu_hboot_header_nae) + sizeof(app_cpu_header_nae), // 0x0100
    .bNumModuleInfos = 0,
    .aulMD5 = {0},
    .ulTagListSize = 0,
    .ulTagListOffset = 0,
    .ulTagListSizeMax = 0,
    .ulCommonCRC32 = 0,
    .ulHeaderCRC32 = 0,
  },
  .tDeviceInfo =
  {
    .ulStructVersion = HIL_VERSION_DEVICE_INFO_V1_0,
    .usManufacturer = HIL_MANUFACTURER_HILSCHER_GMBH,
    .usDeviceClass = HIL_HW_DEV_CLASS_CHIP_NETX_90_APP_FOR_COM_USECASE_C,
    .bHwCompatibility = 0,
    .bChipType = HIL_DEV_CHIP_TYPE_NETX90,
    .ausHwOptions = {HIL_HW_ASSEMBLY_OPTION1, HIL_HW_ASSEMBLY_OPTION2, HIL_HW_ASSEMBLY_OPTION3, HIL_HW_ASSEMBLY_OPTION4},
    .ulLicenseFlags1 = 0,
    .ulLicenseFlags2 = 0,
    .usNetXLicenseID = 0,
    .usNetXLicenseFlags = 0,
    .ausFwVersion = {0},
    .ulDeviceNumber = 0,
    .ulFwNumber = 0,
    .ulSerialNumber = 0,
  },
};

void print_app_file_header(HIL_FILE_NAI_NAE_APP_HEADER_V3_0_T* ptHeader)
{
  printf("\nprint_app_file_header\n");
  printf("Pointer: 0x%08x\n", (uintptr_t) ptHeader);
  printf("\napp_cpu_header.tBootHeader\n");
  printf(" Pointer: 0x%08x\n", (uintptr_t) &ptHeader->tBootHeader);
  printf("  .ulMagicCookie         = 0x%08x\n", (unsigned int) ptHeader->tBootHeader.ulMagicCookie);
  printf("  .ulAppChecksum         = 0x%08x\n", (unsigned int) ptHeader->tBootHeader.ulAppChecksum);
  printf("  .ulAppFileSize         = 0x%08x\n", (unsigned int) ptHeader->tBootHeader.ulAppFileSize);
  printf("  .ulSignature           = 0x%08x\n", (unsigned int) ptHeader->tBootHeader.ulSignature);
  printf("  .ulBootHeaderChecksum  = 0x%08x\n", (unsigned int) ptHeader->tBootHeader.ulBootHeaderChecksum);
  printf("\nptHeader->tCommonHeader\n");
  printf("  Pointer: 0x%08x\n", (uintptr_t) &ptHeader->tCommonHeader);
  printf("  .ulHeaderVersion       = 0x%08x\n", (unsigned int) ptHeader->tCommonHeader.ulHeaderVersion);
  printf("  .ulHeaderLength        = 0x%08x\n", (unsigned int) ptHeader->tCommonHeader.ulHeaderLength);
  printf("  .ulDataSize            = 0x%08x\n", (unsigned int) ptHeader->tCommonHeader.ulDataSize);
  printf("  .ulDataStartOffset     = 0x%08x\n", (unsigned int) ptHeader->tCommonHeader.ulDataStartOffset);
  printf("  .bNumModuleInfos       = 0x%08x\n", (unsigned int) ptHeader->tCommonHeader.bNumModuleInfos);
  printf("  .aulMD5                = 0x%08x\n", (unsigned int) ptHeader->tCommonHeader.aulMD5[0]);
  printf("  .ulTagListSize         = 0x%08x\n", (unsigned int) ptHeader->tCommonHeader.ulTagListSize);
  printf("  .ulTagListOffset       = 0x%08x\n", (unsigned int) ptHeader->tCommonHeader.ulTagListOffset);
  printf("  .ulTagListSizeMax      = 0x%08x\n", (unsigned int) ptHeader->tCommonHeader.ulTagListSizeMax);
  printf("  .ulCommonCRC32         = 0x%08x\n", (unsigned int) ptHeader->tCommonHeader.ulCommonCRC32);
  printf("  .ulHeaderCRC32         = 0x%08x\n", (unsigned int) ptHeader->tCommonHeader.ulHeaderCRC32);
  printf("\nptHeader->tDeviceInfo\n");
  printf("  Pointer: 0x%08x\n", (uintptr_t) &ptHeader->tDeviceInfo);
  printf("  .ulStructVersion       = 0x%08x\n", (unsigned int) ptHeader->tDeviceInfo.ulStructVersion);
  printf("  .usManufacturer        = 0x%08x\n", (unsigned int) ptHeader->tDeviceInfo.usManufacturer);
  printf("  .usDeviceClass         = 0x%08x\n", (unsigned int) ptHeader->tDeviceInfo.usDeviceClass);
  printf("  .bHwCompatibility      = 0x%08x\n", (unsigned int) ptHeader->tDeviceInfo.bHwCompatibility);
  printf("  .bChipType             = 0x%08x\n", (unsigned int) ptHeader->tDeviceInfo.bChipType);
  printf("  .ausHwOptions          = 0x%08x\n", (unsigned int) ptHeader->tDeviceInfo.ausHwOptions[0]);
  printf("  .ulLicenseFlags1       = 0x%08x\n", (unsigned int) ptHeader->tDeviceInfo.ulLicenseFlags1);
  printf("  .ulLicenseFlags2       = 0x%08x\n", (unsigned int) ptHeader->tDeviceInfo.ulLicenseFlags2);
  printf("  .usNetXLicenseID       = 0x%08x\n", (unsigned int) ptHeader->tDeviceInfo.usNetXLicenseID);
  printf("  .usNetXLicenseFlags    = 0x%08x\n", (unsigned int) ptHeader->tDeviceInfo.usNetXLicenseFlags);
  printf("  .ausFwVersion          = 0x%08x\n", (unsigned int) ptHeader->tDeviceInfo.ausFwVersion[0]);
  printf("  .ulDeviceNumber        = 0x%08x\n", (unsigned int) ptHeader->tDeviceInfo.ulDeviceNumber);
  printf("  .ulFwNumber            = 0x%08x\n", (unsigned int) ptHeader->tDeviceInfo.ulFwNumber);
  printf("  .ulSerialNumber        = 0x%08x\n", (unsigned int) ptHeader->tDeviceInfo.ulSerialNumber);
}

void print_app_header_nai(void)
{
  print_app_hboot_header(&app_cpu_hboot_header_nai);
  print_app_file_header(&app_cpu_header_nai);
}

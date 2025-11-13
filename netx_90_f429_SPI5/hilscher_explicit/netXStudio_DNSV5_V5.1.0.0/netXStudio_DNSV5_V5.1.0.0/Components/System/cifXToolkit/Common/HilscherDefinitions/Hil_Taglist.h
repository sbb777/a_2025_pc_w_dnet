/**************************************************************************************

  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: Hil_Taglist.h 399 2021-05-12 12:38:58Z ABessler $:

  Description:
    The Hilscher tag list is a data structure which can be added to the binary firmware
    file. The data which is stored within this list can be manipulated via a special
    tag list editor tool without recompiling the firmware itself.

    The firmware also can access the data of the tag list and use the data to accept
    behavior or configurations which is compiled into the firmware binary.

    This file defines all related tag codes and structures.

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2016-11-30  File created.

**************************************************************************************/
#ifndef HIL_TAGLIST_H_
#define HIL_TAGLIST_H_

#include <stdint.h>
#include "Hil_Compiler.h"

/**************************************************************************************
  Tag type code ranges and segmentation.
**************************************************************************************/

/* Tag type code modifier, if set the tag shall be ignored by the firmware */
#define HIL_TAG_IGNORE_FLAG                               0x80000000

/* Tag type mask */
#define HIL_TAG_MASK                                      0x7FFFFFFF

/* Tag type ranges */
#define HIL_TAG_SPECIAL_START                             0x00000000
#define HIL_TAG_SPECIAL_END                               0x000007FF

#define HIL_TAG_GENERAL_START                             0x00000800
#define HIL_TAG_GENERAL_END                               0x00000FFF

#define HIL_TAG_FIRMWARE_START                            0x00001000
#define HIL_TAG_FIRMWARE_END                              0x0FFFFFFF

#define HIL_TAG_FACILITY_START                            0x10000000
#define HIL_TAG_FACILITY_END                              0x1FFFFFFF

#define HIL_TAG_USER_START                                0x20000000
#define HIL_TAG_USER_END                                  0x2FFFFFFF

#define HIL_TAG_PROTOCOL_START                            0x30000000
#define HIL_TAG_PROTOCOL_END                              0x3FFFFFFF

#define HIL_TAG_BSL_START                                 0x40000000
#define HIL_TAG_BSL_END                                   0x4FFFFFFF




/**************************************************************************************
  Tag type codes.
  NOTE: New tag codes are coordinated by the netX tools department (NXT).
**************************************************************************************/

/* Tag types from the special tag code range */
#define HIL_TAG_END_OF_LIST                               0x00000000



/* Tag types from the general tag code range */
#define HIL_TAG_MEMSIZE                                   0x00000800
#define HIL_TAG_MIN_PERSISTENT_STORAGE_SIZE               0x00000801
#define HIL_TAG_MIN_OS_VERSION                            0x00000802
#define HIL_TAG_MAX_OS_VERSION                            0x00000803
#define HIL_TAG_MIN_CHIP_REV                              0x00000804
#define HIL_TAG_MAX_CHIP_REV                              0x00000805
#define HIL_TAG_NUM_COMM_CHANNEL                          0x00000806  /* deprecated */



/* Tag types from the firmware tag code range */
#define HIL_TAG_TASK_GROUP                                0x00001000
#define HIL_TAG_IT_STATIC_TASK_PARAMETER_BLOCK            0x00001001  /* deprecated */
#define HIL_TAG_IT_STATIC_TASK_ENTRY                      0x00001002  /* deprecated */
#define HIL_TAG_TASK                                      0x00001003

#define HIL_TAG_TIMER                                     0x00001010

#define HIL_TAG_INTERRUPT_GROUP                           0x00001020
#define HIL_TAG_INTERRUPT                                 0x00001023

#define HIL_TAG_UART                                      0x00001030

#define HIL_TAG_LED                                       0x00001040
#define HIL_TAG_IOPIN                                     0x00001041  /* tag structure description is not available */
#define HIL_TAG_SWAP_LNK_ACT_LED                          0x00001042  /* tag structure description is not available */

#define HIL_TAG_XC                                        0x00001050

#define HIL_TAG_DPM_COMM_CHANNEL                          0x00001060
#define HIL_TAG_DPM_SETTINGS                              0x00001061
#define HIL_TAG_DPM_BEHAVIOUR                             0x00001062

#define HIL_TAG_REMANENT_DATA_RESPONSIBLE                 0x00001070

#define HIL_TAG_DDP_MODE_AFTER_STARTUP                    0x00001081

#define HIL_TAG_PHY_ENABLE_TIMEOUT                        0x00001090


/* Tag types from the Ethernet Interface facility tag code range */
#define HIL_TAG_PNS_ETHERNET_PARAMS                       0x100F0000

#define HIL_TAG_EIF_EDD_CONFIG                            0x105D0000
#define HIL_TAG_EIF_EDD_INSTANCE                          0x105D0001
#define HIL_TAG_EIF_NDIS_ENABLE                           0x105D0002



/* Tag types from the netX Diagnostics and Remote Access facility tag code range */
#define HIL_TAG_DIAG_IF_CTRL_UART                         0x10820000
#define HIL_TAG_DIAG_IF_CTRL_USB                          0x10820001
#define HIL_TAG_DIAG_IF_CTRL_TCP                          0x10820002
#define HIL_TAG_DIAG_TRANSPORT_CTRL_CIFX                  0x10820010
#define HIL_TAG_DIAG_TRANSPORT_CTRL_PACKET                0x10820011

/* Tag types for specific usecase */
#define HIL_TAG_SERVX_PORT_NUMBER                         0x10920000

#define HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX100_PARAMS     0x10960000
#define HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX50_PARAMS      0x10960001

#define HIL_TAG_NETPLC_IO_HANDLER_ENABLE                  0x10a30000 /* tag structure description is not available */
#define HIL_TAG_NETPLC_IO_HANDLER_DIGITAL                 0x10a30001 /* tag structure description is not available */
#define HIL_TAG_NETPLC_IO_HANDLER_ANALOG                  0x10a30002 /* tag structure description is not available */

                                                       /* 0x10e00000 Tag id is worn out */
#define HIL_TAG_NF_GEN_DIAG_RESOURCES                     0x10e00001
#define HIL_TAG_NF_PROFI_ENERGY_MODES                     0x10e00002
#define HIL_TAG_NF_PN_IOL_PROFILE_PADDING                 0x10e00003
#define HIL_TAG_NF_PN_IOL_PROFILE_DIO_IN_IOLM             0x10e00004
#define HIL_TAG_NF_SWAP_COM_LEDS                          0x10e00005

#define HIL_TAG_LWIP_PORTS_FOR_IP_ZERO                    0x10e90000
#define HIL_TAG_LWIP_NETIDENT_BEHAVIOUR                   0x10e90001
#define HIL_TAG_LWIP_QUANTITY_STRUCTURE                   0x10e90002

/* Tag types from the user defined tag code range */



/* Tag types from the protocol tag code range */
/* TagID is 0x3ppppnn where pppp is the protocol class and nnn is the identifier of the specific tag */
#define HIL_TAG_CO_DEVICEID                               0x30004000

#define HIL_TAG_CCL_DEVICEID                              0x30005000

#define HIL_TAG_COMPONET_DEVICEID                         0x30006000 /* tag structure description is not available */

#define HIL_TAG_DEVICENET_DEVICEID                        0x30008000 /* tag structure description is not available */
#define HIL_TAG_DEVICENET_CAN_SAMPLING                    0x30008001 /* tag structure description is not available */

#define HIL_TAG_ECS_DEVICEID                              0x30009000 /* tag structure description is not available */
#define HIL_TAG_ECS_ENABLE_BOOTSTRAP                      0x30009001 /* tag structure description is not available */
#define HIL_TAG_ECS_SELECT_SOE_COE                        0x30009002 /* tag structure description is not available */
#define HIL_TAG_ECS_CONFIG_EOE                            0x30009003 /* tag structure description is not available */
#define HIL_TAG_ECS_MBX_SIZE                              0x30009004 /* tag structure description is not available */
#define HIL_TAG_ECM_ENI_BUS_STATE                         0x30009005

#define HIL_TAG_EIP_DEVICEID                              0x3000A000
#define HIL_TAG_EIP_EDD_CONFIGURATION                     0x3000A001 /* Tag is obsolete */
#define HIL_TAG_EIP_DLR_PROTOCOL                          0x3000A002
#define HIL_TAG_EIP_EIS_CONFIG                            0x3000A003 /* Tag ID shall only be internal, not exposed through tag list editor */
#define HIL_TAG_EIP_EIS_RESOURCES                         0x3000A004

#define HIL_TAG_DP_DEVICEID                               0x30013000 /* tag structure description is not available */

#define HIL_TAG_PN_DEVICEID                               0x30015000
#define HIL_TAG_PROFINET_FEATURES                         0x30015001
#define HIL_TAG_PROFINET_FEATURES_V2                      0x30015002
#define HIL_TAG_PROFINET_SYSTEM_REDUNDANCY_FEATURES       0x30015003

#define HIL_TAG_S3S_DEVICEID                              0x30018000 /* tag structure description is not available */

#define HIL_TAG_TCP_PORT_NUMBERS                          0x30019000

#define HIL_TAG_PLS_DEVICEID                              0x3001A000 /* tag structure description is not available */



/* Tag types from the 2nd stage loader tag code range */
#define HIL_TAG_BSL_SDRAM_PARAMS                          0x40000000 /* tag structure description is not available */
#define HIL_TAG_BSL_HIF_PARAMS                            0x40000001 /* tag structure description is not available */
#define HIL_TAG_BSL_SDMMC_PARAMS                          0x40000002 /* tag structure description is not available */
#define HIL_TAG_BSL_UART_PARAMS                           0x40000003 /* tag structure description is not available */
#define HIL_TAG_BSL_USB_PARAMS                            0x40000004 /* tag structure description is not available */
#define HIL_TAG_BSL_MEDIUM_PARAMS                         0x40000005 /* tag structure description is not available */
#define HIL_TAG_BSL_EXTSRAM_PARAMS                        0x40000006 /* tag structure description is not available */
#define HIL_TAG_BSL_HWDATA_PARAMS                         0x40000007 /* tag structure description is not available */
#define HIL_TAG_BSL_FSU_PARAMS                            0x40000008 /* tag structure description is not available */
#define HIL_TAG_BSL_MMIO_NETX50_PARAMS                    0x40000009 /* tag structure description is not available */
#define HIL_TAG_BSL_MMIO_NETX10_PARAMS                    0x4000000A /* tag structure description is not available */
#define HIL_TAG_BSL_HIF_NETX10_PARAMS                     0x4000000B /* tag structure description is not available */
#define HIL_TAG_BSL_USB_DESCR_PARAMS                      0x4000000C /* tag structure description is not available */
#define HIL_TAG_BSL_DISK_POS_PARAMS                       0x4000000D /* tag structure description is not available */
#define HIL_TAG_BSL_BACKUP_POS_PARAMS                     0x4000000E /* tag structure description is not available */
#define HIL_TAG_BSL_MMIO_NETX51_52_PARAMS                 0x4000000F /* tag structure description is not available */
#define HIL_TAG_BSL_HIF_NETX51_52_PARAMS                  0x40000010 /* tag structure description is not available */
#define HIL_TAG_BSL_SERFLASH_PARAMS                       0x40000011 /* tag structure description is not available */


/**************************************************************************************
  General tag list definitions
**************************************************************************************/

/* Macro for forcing an instance of a tag list or single tag into a separate
 * ".taglist" section (needed for NXFs) */
#define __SEC_TAGLIST__       __attribute__ ((section (".taglist")))

#define HIL_TAGLIST_START_TOKEN  "TagList>"
#define HIL_TAGLIST_END_TOKEN    "<TagList"

/** Taglist header.
 * Taglist for netX90/netX4000 based firmwares have a proper header and footer.
 * This enclosure don't exist in nxf firmware files.
 *
 * File header of nxf files HIL_FILE_COMMON_HEADER_V3_0_T.ulTagListOffset points
 * to the first tag structure.
 * |  File header of nxi files HIL_FILE_COMMON_HEADER_V3_0_T.ulTagListOffset points
 * |  to this header structure.
 * |  |
 * |  \-> [Header] - HIL_TAGLIST_HEADER_T   <- not present in nxf files (this structure)
 * \----> [Tag]    - HIL_TAG_*
 *        [Tag]    - HIL_TAG_*
 *        ...      - ...
 *        [Tag]    - HIL_TAG_END_OF_LIST_T  <- Always required
 *        [Footer] - HIL_TAGLIST_FOOTER_T   <- Not present in nxf files
 */
typedef struct
{
  /*! Start token of the taglist data area.
   * This field must contain the token sting defined by HIL_TAGLIST_START_TOKEN. */
  uint8_t abStartToken[8];

  /*! Size of the taglist data area.
   * \note This includes the Header and Footer and possible padding/ spare space */
  uint16_t  usTagListSize;

  /*! Size of the filled taglist data.
   * \note This is the size of all tags in the taglist without header,footer and spare space*/
  uint16_t  usContentSize;
} HIL_TAGLIST_HEADER_T;

/** Taglist footer.
 * Taglist footer for use with netX90/netX4000 based firmwares.
 */
typedef struct
{
  /*! Reserved for future usage */
  uint32_t  ulReserved;

  /*! End token of the taglist data area.
   * This field must contain the token string defined by HIL_TAGLIST_END_TOKEN. */
  uint8_t abEndToken[8];
} HIL_TAGLIST_FOOTER_T;

/* Tag header with type code and length of following tag data */
typedef struct __HIL_ALIGNED_DWORD__
{
  uint32_t ulTagType;
  uint32_t ulTagDataLength;
} HIL_TAG_HEADER_T;

/* Identifier string for named resources */
typedef struct
{
  char abName[16];
} HIL_TAG_IDENTIFIER_T;



/**************************************************************************************
  End of tag list tag definitions.
  Tag codes: HIL_TAG_END_OF_LIST
**************************************************************************************/
typedef struct
{
  HIL_TAG_HEADER_T tHeader;
} HIL_TAG_END_OF_LIST_T;



/**************************************************************************************
  UINT32 tag definitions.
  Tag codes: HIL_TAG_MEMSIZE, HIL_TAG_MIN_PERSISTENT_STORAGE_SIZE,
             HIL_TAG_MIN_CHIP_REV, HIL_TAG_MAX_CHIP_REV
**************************************************************************************/
typedef struct
{
  uint32_t ulValue;
} HIL_TAG_UINT32_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T      tHeader;
  HIL_TAG_UINT32_DATA_T tData;
} HIL_TAG_UINT32_T;



/**************************************************************************************
  Version tag definitions.
  Tag codes: HIL_TAG_MIN_OS_VERSION, HIL_TAG_MAX_OS_VERSION
**************************************************************************************/
typedef struct
{
  uint16_t usMajor;
  uint16_t usMinor;
  uint16_t usBuild;
  uint16_t usRevision;
} HIL_TAG_VERSION_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T       tHeader;
  HIL_TAG_VERSION_DATA_T tData;
} HIL_TAG_VERSION_T;



/**************************************************************************************
  LED tag definitions.
  Tag codes: HIL_TAG_LED

  This tag is used to modify physical LED connection settings in the firmware.
**************************************************************************************/

/* resource codes for LED tag */
#define HIL_TAG_LED_RESOURCE_TYPE_GPIO              1
#define HIL_TAG_LED_RESOURCE_TYPE_PIO               2
#define HIL_TAG_LED_RESOURCE_TYPE_HIFPIO            3

/* polarity codes for LED tag */
#define HIL_TAG_LED_POLARITY_NORMAL                 0
#define HIL_TAG_LED_POLARITY_INVERTED               1

typedef struct
{
  HIL_TAG_IDENTIFIER_T tIdentifier;         /* rcX LED object identifier, read-only */
  uint32_t             ulUsesResourceType;  /* RX_PERIPHERAL_TYPE_PIO or RX_PERIPHERAL_TYPE_GPIO (see rX_Config.h) */
  uint32_t             ulPinNumber;         /* PIO or GPIO index number */
  uint32_t             ulPolarity;          /* control code for GPIO polarity (see rX_Config.h) */
} HIL_TAG_LED_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T   tHeader;
  HIL_TAG_LED_DATA_T tData;
} HIL_TAG_LED_T;



/**************************************************************************************
  Group of statically defined tasks tag definitions.
  Tag code: HIL_TAG_TASK_GROUP

  Used to modify the priority of an group of task with the same group reference number.
  The priority and token values are added to the statically defined values.
**************************************************************************************/
typedef struct
{
  char      szTaskListName[64];   /* group name, read-only */
  uint32_t  ulBasePriority;       /* base priority for the tasks in the group */
  uint32_t  ulBaseToken;          /* base token for the tasks in the group */
  uint32_t  ulRange;              /* number of tasks in the group, read-only */
  uint32_t  ulTaskGroupRef;       /* group reference number (common to all tasks in the group), read-only */
} HIL_TAG_TASK_GROUP_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T          tHeader;
  HIL_TAG_TASK_GROUP_DATA_T tData;
} HIL_TAG_TASK_GROUP_T;



/**************************************************************************************
  Task tag definitions.
  Tag code: HIL_TAG_TASK

  Used to modify an individual task priority. The priority value replaces the defined
  values.
**************************************************************************************/
typedef struct
{
  HIL_TAG_IDENTIFIER_T  tIdentifier;        /* rcX task object identifier, read-only */
  uint32_t              ulPriority;         /* task priority offset */
  uint32_t              ulToken;            /* task token offset */
} HIL_TAG_TASK_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T      tHeader;
  HIL_TAG_TASK_DATA_T   tData;
} HIL_TAG_TASK_T;



/**************************************************************************************
  Interrupt group tag definitions.
  Tag code: HIL_TAG_INTERRUPT_GROUP

  Used to modify the priority of an group of interrupts with the same group reference
  number. The priority and token values are added to the statically defined values.
**************************************************************************************/
typedef struct
{
  char      szInterruptListName[64];          /* group name, read-only */
  uint32_t  ulBaseIntPriority;                /* base interrupt priority for the interrupts in the group */
  uint32_t  ulRangeInt;                       /* number of interrupts in the group, read-only */
  uint32_t  ulBaseTaskPriority;               /* base task priority if one of the handlers is configured to run in task mode */
  uint32_t  ulBaseTaskToken;                  /* base task token if one of the handlers is configured to run in task mode */
  uint32_t  ulRangeTask;                      /* number of interrupts in the group that execute in task mode, read-only */
  uint32_t  ulInterruptGroupRef;              /* group reference number (common to all interrupts in the group), read-only */
} HIL_TAG_INTERRUPT_GROUP_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T               tHeader;
  HIL_TAG_INTERRUPT_GROUP_DATA_T tData;
} HIL_TAG_INTERRUPT_GROUP_T;



/**************************************************************************************
  Interrupt tag definitions.
  Tag code: HIL_TAG_INTERRUPT

  Used to modify an individual interrupt priority. The priority value replaces the
  defined values.
**************************************************************************************/
typedef struct
{
  HIL_TAG_IDENTIFIER_T  tIdentifier;          /* rcX interrupt object identifier, read-only */
  uint32_t              ulInterruptPriority;  /* interrupt priority offset */
  uint32_t              ulTaskPriority;       /* interrupt handler task priority offset */
  uint32_t              ulTaskToken;          /* interrupt handler task token offset */
} HIL_TAG_INTERRUPT_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T         tHeader;
  HIL_TAG_INTERRUPT_DATA_T tData;
} HIL_TAG_INTERRUPT_T;



/**************************************************************************************
  Timer tag definitions.
  Tag code: HIL_TAG_TIMER

  Used to change a timer number.
**************************************************************************************/
typedef struct
{
  HIL_TAG_IDENTIFIER_T  tIdentifier;  /* rcX timer object identifier, read-only */
  uint32_t              ulTimNum;     /* netX hardware timer number */
} HIL_TAG_TIMER_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T     tHeader;
  HIL_TAG_TIMER_DATA_T tData;
} HIL_TAG_TIMER_T;



/**************************************************************************************
  UART settings tag definitions.
  Tag code: HIL_TAG_UART

  Used to set UART configuration settings.
**************************************************************************************/
typedef struct
{
  HIL_TAG_IDENTIFIER_T  tIdentifier;       /* rcX UART object identifier, read-only */
  uint32_t              ulUrtNumber;       /* netX UART number (see rX_Config.h) */
  uint32_t              ulBaudRate;        /* baud rate control code (see rX_Config.h) */
  uint32_t              ulParity;          /* parity control code (see rX_Config.h) */
  uint32_t              ulStopBits;        /* stop bits control code (see rX_Config.h) */
  uint32_t              ulDataBits;        /* data bits control code (see rX_Config.h) */
  uint32_t              ulRxFifoLevel;     /* "rx ready" trigger level for Rx FIFO (set to 0 to force immediate notification, set to 1..16 to enable FIFO) */
  uint32_t              ulTxFifoLevel;     /* "tx empty" trigger level for Tx FIFO (set to 0 to force immediate send, set to 1..16 to enable FIFO */
  uint32_t              ulRtsMode;         /* control code for RTS signal behavior (see rX_Config.h) */
  uint32_t              ulRtsPolarity;     /* control code for RTS signal polarity (see rX_Config.h) */
  uint32_t              ulRtsForerun;      /* RTS signal forerun with respect to TxD (in bits or in UART clock ticks depending on ulRtsMode) */
  uint32_t              ulRtsTrail;        /* RTS signal trail with respect to TxD (in bits or in UART clock ticks depending on ulRtsMode) */
  uint32_t              ulCtsMode;         /* control code for CTS signal behavior (see rX_Config.h) */
  uint32_t              ulCtsPolarity;     /* control code for CTS signal polarity (see rX_Config.h) */
} HIL_TAG_UART_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T    tHeader;
  HIL_TAG_UART_DATA_T tData;
} HIL_TAG_UART_T;



/**************************************************************************************
  XC settings tag definitions.
  Tag code: HIL_TAG_XC

  Used to modify the XC unit which should be used.
**************************************************************************************/
typedef struct
{
  HIL_TAG_IDENTIFIER_T  tIdentifier;  /* rcX xC object identifier, read-only */
  uint32_t              ulXcId;       /* netX xC unit number */
} HIL_TAG_XC_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T  tHeader;
  HIL_TAG_XC_DATA_T tData;
} HIL_TAG_XC_T;



/**************************************************************************************
  DPM communication channels tag definitions.
  Tag code: HIL_TAG_DPM_COMM_CHANNEL

  Used to modify the RX_HIF_CHANNEL_Ts and the RX_HIF_CHANNEL_BLOCK_Ts describing
  communication channels. The I/O data size values replaces the statically defined
  values at system startup time. The values are also used for calculating the start
  offsets of the subsequent channels and blocks.
**************************************************************************************/

/* Maximum number of communication channels for DPM communication channels tag */
#define DPM_MAX_COMM_CHANNELS                   4

typedef struct
{
  uint32_t ulNumCommChannels;                 /* number of communication channels to be instantiated (1 .. DPM_MAX_COMM_CHANNELS) */
  struct
  { /* total communication channel size is (0x1000 + ulInDataSize + ulOutDataSize) */
    uint32_t ulInDataSize;                    /* total size of the normal priority input data area (area 0) in bytes */
    uint32_t ulOutDataSize;                   /* total size of the normal priority output data area (area 0) in bytes */
  } atCommChannelSizes[DPM_MAX_COMM_CHANNELS];
} HIL_TAG_DPM_COMM_CHANNEL_DATA_T;



typedef struct
{
  HIL_TAG_HEADER_T                  tHeader;
  HIL_TAG_DPM_COMM_CHANNEL_DATA_T   tData;
} HIL_TAG_DPM_COMM_CHANNEL_T;


/**************************************************************************************
  DPM settings tag definitions.
  Tag code: HIL_TAG_DPM_SETTINGS

  Used to modify the RX_HIF_SET_T describing DPM location and access. The values
  replaces the statically defined values at system startup time.
**************************************************************************************/
typedef struct
{
  uint32_t ulDpmMode;                  /* DPM mode, 2 (8-bit) / 3 (16-bit) / 5 (PCI), default: 5) */
  uint32_t ulDpmSize;                  /* DPM size in bytes, default: 16384 for comX, 32768 for other targets) */
  uint32_t ulDpmBaseAddress;           /* DPM base address (in INTRAM), default: 0x00018000) */
} HIL_TAG_DPM_SETTINGS_DATA_T;


typedef struct
{
  HIL_TAG_HEADER_T            tHeader;
  HIL_TAG_DPM_SETTINGS_DATA_T tData;
} HIL_TAG_DPM_SETTINGS_T;



/**************************************************************************************
  UART interface of netX Diagnostics and Remote Access component tag definitions.
  Tag code: HIL_TAG_DIAG_IF_CTRL_UART
**************************************************************************************/
typedef struct
{
  uint8_t bEnableFlag;                        /* TRUE: activate this interface, FALSE: do not use this interface */
  uint8_t bIfNumber;                          /* netX UART number to use */
  uint8_t abReserved[2];
} HIL_TAG_DIAG_IF_CTRL_UART_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                 tHeader;
  HIL_TAG_DIAG_IF_CTRL_UART_DATA_T tData;
} HIL_TAG_DIAG_IF_CTRL_UART_T;



/**************************************************************************************
  USB interface of netX Diagnostics and Remote Access component tag definitions.
  Tag code: HIL_TAG_DIAG_IF_CTRL_USB
**************************************************************************************/
typedef struct
{
  uint8_t bEnableFlag;                        /* TRUE: activate this interface, FALSE: do not use this interface */
  uint8_t bIfNumber;                          /* netX USB interface number to use (currently, 0 is the only valid value) */
  uint8_t abReserved[2];
} HIL_TAG_DIAG_IF_CTRL_USB_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                tHeader;
  HIL_TAG_DIAG_IF_CTRL_USB_DATA_T tData;
} HIL_TAG_DIAG_IF_CTRL_USB_T;



/**************************************************************************************
  TCP interface of netX Diagnostics and Remote Access component tag definitions.
  Tag code: HIL_TAG_DIAG_IF_CTRL_TCP
**************************************************************************************/
typedef struct
{
  uint8_t bEnableFlag;                        /* TRUE: activate this interface, FALSE: do not use this interface */
  uint8_t bReserved;
  uint16_t usPortNumber;                      /* TCP port number, typically HIL_TRANSPORT_IP_PORT (50111, see HilTransport.h) */
} HIL_TAG_DIAG_IF_CTRL_TCP_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                tHeader;
  HIL_TAG_DIAG_IF_CTRL_TCP_DATA_T tData;
} HIL_TAG_DIAG_IF_CTRL_TCP_T;



/**************************************************************************************
  cifX transport interface of netX Diagnostics and Remote Access component tag
  definitions.
  Tag code: HIL_TAG_DIAG_TRANSPORT_CTRL_CIFX
**************************************************************************************/
typedef struct
{
  uint8_t bEnableFlag;                        /* TRUE: activate support for this transport type, FALSE: do not use this transport type */
  uint8_t abReserved[3];
} HIL_TAG_DIAG_TRANSPORT_CTRL_CIFX_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                        tHeader;
  HIL_TAG_DIAG_TRANSPORT_CTRL_CIFX_DATA_T tData;
} HIL_TAG_DIAG_TRANSPORT_CTRL_CIFX_T;



/**************************************************************************************
  Packet transport interface of netX Diagnostics and Remote Access component tag
  definitions.
  Tag code: HIL_TAG_DIAG_TRANSPORT_CTRL_PACKET
**************************************************************************************/
typedef struct
{
  uint8_t bEnableFlag;                        /* TRUE: activate support for this transport type, FALSE: do not use this transport type */
  uint8_t abReserved[3];
} HIL_TAG_DIAG_TRANSPORT_CTRL_PACKET_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                          tHeader;
  HIL_TAG_DIAG_TRANSPORT_CTRL_PACKET_DATA_T tData;
} HIL_TAG_DIAG_TRANSPORT_CTRL_PACKET_T;



/**************************************************************************************
  Webserver port settings tag definitions.
  Tag code: HIL_TAG_SERVX_PORT_NUMBER
**************************************************************************************/
typedef struct
{
  uint16_t usPort;     /* 0: do not start servX, other value: port to use by servX */
  uint16_t usReserved;
} HIL_TAG_SERVX_PORT_NUMBER_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                      tHeader;
  HIL_TAG_SERVX_PORT_NUMBER_DATA_T      tData;
} HIL_TAG_SERVX_PORT_NUMBER_T;









/***************************************************************************************/
/***************************************************************************************/
/*                          Component and Stack specific tags                          */
/***************************************************************************************/
/***************************************************************************************/


/**************************************************************************************
  AP Task DPM Behaviour settings tag definitions.
  Tag code: HIL_TAG_DPM_BEHAVIOUR
**************************************************************************************/
typedef struct
{
  uint8_t      bComStateLegacyMode;         /* 1: Enable legacy mode for "ulCommunicationState */
  uint8_t      bReserved1;
  uint8_t      bReserved2;
  uint8_t      bReserved3;
} HIL_TAG_DPM_BEHAVIOUR_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T             tHeader;
  HIL_TAG_DPM_BEHAVIOUR_DATA_T tData;
} HIL_TAG_DPM_BEHAVIOUR_T;


/**************************************************************************************
  AP task remanent data handling responsibility.
  Tag code: HIL_TAG_REMANENT_DATA_RESPONSIBLE

  With this tag, you can adjust whether loading and storing of remanent data is entirely
  performed by the communication firmware or by the host application. Per default, the
  stack will handle remanent data.

  When remanent data handling is configured to be done in the host domain, the host
  application has to provide remanent data for each relevant component of the firmware
  via packet HIL_SET_REMANENT_DATA_REQ during initialization.
  Subsequently, it must handle the packets HIL_STORE_REMANENT_DATA_IND with which modified
  remanent data for each component is submitted in order for the host to store it onto a
  non-volatile storage device during runtime.

  Depending on the specific stack implementation, there still may be the need for the
  host to store certain non-volatile data even if the stack is responsible for remanent
  data storage. Please refer to the manual of your particular Firmware for further
  information.

**************************************************************************************/
typedef struct
{
  /** Responsibility switch.
   *   - 0: Communication firmware stores remanent data
   *   - 1: Host stores remanent data */
  uint8_t      bHandledByHost;

  /** Reserved field for future use
   * Set to 0 to avoid unwanted behavior with upcoming version. */
  uint8_t      abReserved[3];

} HIL_TAG_REMANENT_DATA_RESPONSIBLE_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                          tHeader;
  HIL_TAG_REMANENT_DATA_RESPONSIBLE_DATA_T  tData;
} HIL_TAG_REMANENT_DATA_RESPONSIBLE_T;

/**************************************************************************************
  LWIP netident behavior.
  Tag code: HIL_TAG_LWIP_NETIDENT_BEHAVIOUR

  With this tag, you can adjust whether the firmware shall activate the
  Hilscher netident protocol, which is build-in in the IP stack, or not.

  By default netident protocol is enabled.

**************************************************************************************/
typedef struct
{
  /** netident behavior.
   *   - 0: netident is disabled
   *   - 1: netident is enabled */
  uint8_t      bNetidentEnable;

  /** Reserved field for future use
   * Set to 0 to avoid unwanted behavior with upcoming version. */
  uint8_t      abReserved[3];

} HIL_TAG_LWIP_NETIDENT_BEHAVIOUR_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                        tHeader;
  HIL_TAG_LWIP_NETIDENT_BEHAVIOUR_DATA_T  tData;
} HIL_TAG_LWIP_NETIDENT_BEHAVIOUR_T;

/**************************************************************************************
  LWIP quantity structure.
  Tag code: HIL_TAG_LWIP_QUANTITY_STRUCTURE

  With this tag, you can adjust the resources allocated and provided by the
  build-in IP stack integrated in the firmware.

  The number of Socket API services at DPM via Mailbox can be configured.
  The number of sockets for Socket API usage inside the IP stack can be configured.

**************************************************************************************/
typedef struct
{
  /** number of Socket API services at DPM level.*/
  uint8_t      bNumberDpmSocketServices;
  /** number of handled socket (for Socket API usage) in IP stack. */
  uint8_t      bNumberSockets;

  /** Reserved field for future use
   * Set to 0 to avoid unwanted behavior with upcoming version. */
  uint8_t      abReserved[2];

} HIL_TAG_LWIP_QUANTITY_STRUCTURE_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                        tHeader;
  HIL_TAG_LWIP_QUANTITY_STRUCTURE_DATA_T  tData;
} HIL_TAG_LWIP_QUANTITY_STRUCTURE_T;



/**************************************************************************************
  DeviceNet CAN Sample point.
  Tag code: HIL_TAG_DEVICENET_CAN_SAMPLING

  The sample point location is defined as <=80% of the bit timing. With this option
  it is possible to shift the sample point between the recommended default setting to
  an alternative setting.
  This may solve timing issue when devices are used, which run out of specification
  regarding to the clock timings.

  NOTE: Using this setting may make the product non-conform to the specification!

   Baud rate  | default timing | alternative timing
  ------------|----------------|-------------------
  250   kBaud |      90,0%     |      80,0%
  125   kBaud |      90,0%     |      80,0%
  100   kBaud |      90,0%     |      80,0%
   50   kBaud |      90,0%     |      80,0%
   20   kBaud |      90,0%     |      80,0%
   12,5 kBaud |      90,0%     |      80,0%
   10   kBaud |      90,0%     |      80,0%

**************************************************************************************/
typedef struct
{
  /** Sample point timing settings.
   *   - 0: default timing
   *   - 1: alternative timing
   */
  uint8_t      bEnableAlternativeSamplePointTimings;

  /** Reserved field for future use.
   * Set to 0 to avoid unwanted behavior with upcoming version. */
  uint8_t      abReserved[3];

} HIL_TAG_DEVICENET_CAN_SAMPLING_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                       tHeader;
  HIL_TAG_DEVICENET_CAN_SAMPLING_DATA_T  tData;
} HIL_TAG_DEVICENET_CAN_SAMPLING_T;

/**************************************************************************************
  CANopen Device Identification
  Tag code: HIL_TAG_CO_DEVICEID
**************************************************************************************/
typedef  struct
{
  uint32_t ulVendorId;
  uint32_t ulProductCode;
  uint16_t usMajRev;
  uint16_t usMinRev;
  uint16_t usDeviceProfileNumber;
  uint16_t usAdditionalInfo;

} HIL_CO_DEVICEID_ID_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T          tHeader;
  HIL_CO_DEVICEID_ID_DATA_T tData;

} HIL_CO_DEVICEID_ID_T;

/**************************************************************************************
  CCLink Device Identification
  Tag code: HIL_TAG_CCL_DEVICEID
**************************************************************************************/
typedef struct
{
  uint32_t ulVendorId;
  uint32_t ulModelType;
  uint32_t ulSwVersion;

} HIL_CCL_DEVICEID_ID_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T          tHeader;
  HIL_CCL_DEVICEID_ID_DATA_T tData;

} HIL_CCL_DEVICEID_ID_T;


/**************************************************************************************
  DDP Mode after firmware startup.
  Tag code: HIL_TAG_DDP_MODE_AFTER_STARTUP

  With this tag, you can control the DDP mode on firmware startup.
  The DDP is integrated in the firmware operating system.
  The DDP offers two modes: "active" or "passive" mode.
  This influences the possibilities of your application in regards of the usage
  of the DDP Set API to modify parameter in DDP.

  In "passive" mode the usage of the DDP Set API is possible (for writable
  parameters). The firmware will treat the writable DDP parameters as invalid
  (with all corresponding consequences) until the application informs the firmware
  that DDP parameters are now valid.
  The application is required to use the DDP API to set DDP mode to "active" later.

  Consequences of "passive" DDP mode (incomplete list)
  - protocol specific configuration is rejected
  - NDIS is not working

  In "active" mode the usage of DDP Set API is not possible and will be rejected.
  The firmware will directly use and activate the DDP parameters (which are
  typically contained in FDL).

  By default (if taglist is unchanged) the DDP starts in "active" more.
  In consequence, no DDP parameter can be changed with DDP Set API.

**************************************************************************************/
typedef struct
{
  /** DDP mode after firmware startup.
   *   - 0: Startup in mode "passive". Using DDP Set API is possible
   *   - 1: Startup in mode "active".  Using DDP Set API is impossible
   *   - other: reserved  */
  uint8_t      bDdpModeAfterStartup;

  /** Reserved field for future use
   * Set to 0 to avoid unwanted behavior with upcoming version. */
  uint8_t      abReserved[3];

} HIL_TAG_DDP_INITIAL_STATE_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                  tHeader;
  HIL_TAG_DDP_INITIAL_STATE_DATA_T  tData;
} HIL_TAG_DDP_INITIAL_STATE_T;

/**************************************************************************************
  Phy enable tag after firmware startup.
  Tag code: HIL_TAG_PHY_ENABLE_TIMEOUT

  With this tag, you can set the timeout to enable the Phys after firmware startup.
  If no Phy configuration is provided to main protocol stack integrated in the firmware
  up to this timeout, all Phys will be enabled with default settings.
  The default settings are Autonegotiation and Autocrossover.

  If the main protocol stack gets its valid Phy configuration prior the timeout hits,
  the Phys will be configured with the parameters from configuration.
  If the main protocol stack gets its valid Phy configuration after the timeout hits,
  the protocol stack will reparameterize the Phys according to its configuration.
  This may lead to short link loss in case the configuration is different to defaults.

  By default (if taglist is unchanged) the Phy enable timeout is inactive and set to 0s.
  In this case the Phy is not switched on by the firmware and a valid Phy
  configuration of the protocol stack is needed.
  If the timeout is set to a value greater than 0s, the firmware will check the Phys
  state after the given timeout. If Phys are not configured, they will be enabled.

**************************************************************************************/
/* Phy timeout value definitions */
#define HIL_TAG_PHY_ENABLE_TIMEOUT_DISABLED     (0)   /* never enable Phy without configuration */
#define HIL_TAG_PHY_ENABLE_TIMEOUT_MIN          (1)   /* wait for 1 second, minimum allowed value */
#define HIL_TAG_PHY_ENABLE_TIMEOUT_MAX          (300) /* wait for 300 second, minimum allowed value */
#define HIL_TAG_PHY_ENABLE_TIMEOUT_DEFAULT      (0)   /* do not enable Phy without configuration, default value */

typedef struct
{
  /** Phy enable timeout in seconds after firmware startup.
   *   - 0: wait for protocol stack configuration.
   *   - 1-300: Phy shall be enabled 1-300 seconds after firmware startup
   *   - other: reserved  */
  uint32_t      ulPhyEnableTimeoutAfterStartup;

} HIL_TAG_PHY_ENABLE_TIMEOUT_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                   tHeader;
  HIL_TAG_PHY_ENABLE_TIMEOUT_DATA_T  tData;
} HIL_TAG_PHY_ENABLE_TIMEOUT_T;

/**************************************************************************************
  Ethernet interface EDD instance settings tag definitions.
  Tag code: HIL_TAG_EIF_EDD_INSTANCE
**************************************************************************************/
typedef struct
{
  uint32_t ulEddInstanceNo;                   /* instance number of the EDD (0 .. (DPM_MAX_COMM_CHANNELS - 1)) */
} HIL_TAG_EIF_EDD_INSTANCE_DATA_T;


typedef struct
{
  HIL_TAG_HEADER_T                  tHeader;
  HIL_TAG_EIF_EDD_INSTANCE_DATA_T   tData;
} HIL_TAG_EIF_EDD_INSTANCE_T;



/**************************************************************************************
  Ethernet interface config settings tag definitions.
  Tag code: HIL_TAG_EIF_EDD_CONFIG
**************************************************************************************/

/* EDD type definitions for Ethernet Interface configuration tag */
#define RX_EIF_EDD_TYPE_VIRTUAL                 0   /* virtual EDD attached to TCP stack */
#define RX_EIF_EDD_TYPE_STD_MAC                 1   /* single-port standard Ethernet interface */
#define RX_EIF_EDD_TYPE_2PORT_SWITCH            2   /* 2-port switch */
#define RX_EIF_EDD_TYPE_2PORT_HUB               3   /* 2-port hub */

/* Ethernet Interface component when used without run-time linking to an RTE communication stack */
typedef struct
{
  uint32_t ulEddType;                         /* type of the EDD (see EDD type definitions for Ethernet Interface configuration) */
  uint32_t ulFirstXcNumber;                   /* number of the first (or the only) xC used */
} HIL_TAG_EIF_EDD_CONFIG_DATA_T;


typedef struct
{
  HIL_TAG_HEADER_T                  tHeader;
  HIL_TAG_EIF_EDD_CONFIG_DATA_T     tData;
} HIL_TAG_EIF_EDD_CONFIG_T;



/**************************************************************************************
  Ethernet interface NDIS settings tag definitions.
  Tag code: HIL_TAG_EIF_NDIS_ENABLE
**************************************************************************************/
typedef struct
{
  uint32_t      ulNDISEnable;  /* 0: NDIS is disabled, 1: NDIS is enabled */
} HIL_TAG_EIF_NDIS_ENABLE_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                tHeader;
  HIL_TAG_EIF_NDIS_ENABLE_DATA_T  tData;
} HIL_TAG_EIF_NDIS_ENABLE_T;




/**************************************************************************************
  Ethernet interface TCP Port settings tag definitions.
  Tag code: HIL_TAG_TCP_PORT_NUMBERS
**************************************************************************************/
#define HIL_TAG_ETHINTF_TCPUDP_PORT_NUMBERS_RANGE_START_DEFAULT     (1024) /* Default value for start port (ulPortStart) */
#define HIL_TAG_ETHINTF_TCPUDP_PORT_NUMBERS_RANGE_END_DEFAULT       (2048) /* Default value for end port   (ulPortEnd)   */

typedef struct
{
  /* Note: The range which is (ulPortEnd - ulPortStart) must not go below a limit of 1024 */

  uint32_t    ulPortStart;                  /* TCP/UDP-Port range, start port. */
  uint32_t    ulPortEnd;                    /* TCP/UDP-Port range, end port */
  uint32_t    ulNumberOfProtocolStackPorts; /* Number of ports the protocol stack uses. */
  uint32_t    ulNumberOfUserPorts;          /* Number of additional ports the user put into the list (ausPortList)*/
  uint16_t    ausPortList[20];              /* Port list */
} HIL_TAG_TCP_PORT_NUMBERS_DATA_T;


typedef struct
{
  HIL_TAG_HEADER_T                tHeader;
  HIL_TAG_TCP_PORT_NUMBERS_DATA_T tData;

} HIL_TAG_ETHINTF_TCPUDP_PORT_NUMBERS_T;


/**************************************************************************************
  netFIELD generic diagnosis resources configuration.
  Tag code: HIL_TAG_NF_GEN_DIAG_RESOURCES
**************************************************************************************/
typedef struct
{
  /* Number of additional netPROXY Generic Diagnosis instances required by the OEM
   * application for own diagnosis functions. Allowed values 32...256 in steps of 8  */
  uint16_t usNumOfDiagnosisInstances;

  /* Reserved, set to zero */
  uint8_t abReserved[2];

} HIL_TAG_NF_GEN_DIAG_RESOURCES_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                     tHeader;
  HIL_TAG_NF_GEN_DIAG_RESOURCES_DATA_T tData;
} HIL_TAG_NF_GEN_DIAG_RESOURCES_T;


/**************************************************************************************
  netFIELD PROFIenergy support.
  Tag code: HIL_TAG_NF_PROFI_ENERGY_MODES
**************************************************************************************/
typedef struct
{
  /* Activation of PROFIenergy feature and setting the number of supported PROFIenergy
   * modes. Allowed values:
   *  - 0    == PROFIenergy is disabled
   *  - 1..8 == PROFIenergy is supported with 1..8 modes */
  uint8_t bPROFIenergyMode;

  /* Reserved, set to zero */
  uint8_t abReserved[3];

} HIL_TAG_NF_PROFI_ENERGY_MODES_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                     tHeader;
  HIL_TAG_NF_PROFI_ENERGY_MODES_DATA_T tData;
} HIL_TAG_NF_PROFI_ENERGY_MODES_T;


/**************************************************************************************
  netFIELD PROFINET IO-Link profile submodule padding.
  Tag code: HIL_TAG_NF_PN_IOL_PROFILE_PADDING
**************************************************************************************/
#define HIL_TAG_NF_PN_IOL_PROFILE_PADDING_PADMODE_UNALIGNMENT     0
#define HIL_TAG_NF_PN_IOL_PROFILE_PADDING_PADMODE_2BYTE_ALIGNMENT 1
#define HIL_TAG_NF_PN_IOL_PROFILE_PADDING_PADMODE_4BYTE_ALIGNMENT 2

typedef struct
{
  /* PROFINET IO-Link profile submodule padding.
   * Allowed values: All HIL_TAG_NF_PN_IOL_PROFILE_PADDING_PADMODE_* defines */
  uint8_t bProfilePaddingMode;

  /* Reserved, set to zero */
  uint8_t abReserved[3];

} HIL_TAG_NF_PN_IOL_PROFILE_PADDING_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                        tHeader;
  HIL_TAG_NF_PN_IOL_PROFILE_PADDING_DATA_T tData;
} HIL_TAG_NF_PN_IOL_PROFILE_PADDING_T;


/**************************************************************************************
  netFIELD PROFINET IO-Link profile DIO in IOLM.
  Tag code: HIL_TAG_NF_PN_IOL_PROFILE_DIO_IN_IOLM
**************************************************************************************/
#define HIL_TAG_NF_PN_IOL_PROFILE_DIO_IN_IOLM_DISABLED    0
#define HIL_TAG_NF_PN_IOL_PROFILE_DIO_IN_IOLM_ENABLED     1

typedef struct
{
  /* PROFINET IO-Link profile submodule DIO in IOLM.
   * Allowed values: All HIL_TAG_NF_PN_IOL_PROFILE_DIO_IN_IOLM_* defines */
  uint8_t bDioInIolm;

  /* Reserved, set to zero */
  uint8_t abReserved[3];

} HIL_TAG_NF_PN_IOL_PROFILE_DIO_IN_IOLM_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                             tHeader;
  HIL_TAG_NF_PN_IOL_PROFILE_DIO_IN_IOLM_DATA_T tData;
} HIL_TAG_NF_PN_IOL_PROFILE_DIO_IN_IOLM_T;


/**************************************************************************************
  netFIELD Swap COM LEDs.
  Tag code: HIL_TAG_NF_SWAP_COM_LEDS

  This tag allows to swap the COM0 and COM1 indicator LEDs with each other.

**************************************************************************************/
typedef struct
{
  /* Activate swapping
   * Allowed values:
   *   - TRUE:  Swap COM0 with COM1
   *   - FALSE: Don't swap */
  uint8_t bSwapComLeds;

  /* Reserved, set to zero */
  uint8_t abReserved[3];

} HIL_TAG_NF_SWAP_COM_LEDS_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                tHeader;
  HIL_TAG_NF_SWAP_COM_LEDS_DATA_T tData;
} HIL_TAG_NF_SWAP_COM_LEDS_T;


/**************************************************************************************
  PROFINET Ethernet parameter settings tag definitions.
  Tag code: HIL_TAG_PNS_ETHERNET_PARAMS
**************************************************************************************/
#define HIL_TAG_PNS_ETHERNET_FIBEROPTICMODE_OFF            (0)
#define HIL_TAG_PNS_ETHERNET_FIBEROPTICMODE_ON             (1)
#define HIL_TAG_PNS_ETHERNET_FIBEROPTICMODE_PORT0_ONLY_ON  (2)
#define HIL_TAG_PNS_ETHERNET_FIBEROPTICMODE_PORT1_ONLY_ON  (3)

typedef struct
{
  uint8_t       bActivePortsBf;  /* each bit for one port */
  uint8_t       bFiberOpticMode; /* see defines above */
  uint8_t       abReserved[2];
} HIL_TAG_PNS_ETHERNET_PARAMS_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                    tHeader;
  HIL_TAG_PNS_ETHERNET_PARAMS_DATA_T  tData;
} HIL_TAG_PNS_ETHERNET_PARAMS_T;



/**************************************************************************************
  PROFINET Fiberoptic parameter settings tag definitions for netX50.
  Tag code: HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX50_PARAMS
**************************************************************************************/
typedef struct
{
  uint8_t      bSDA1PinIdx; /* mmio number */
  uint8_t      bSDA2PinIdx; /* mmio number */
  uint8_t      bSCLPinIdx;  /* mmio number */
  uint8_t      bReserved;
} HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX50_PARAMS_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                                     tHeader;
  HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX50_PARAMS_DATA_T  tData;
} HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX50_PARAMS_T;


/**************************************************************************************
  PROFINET Fiberoptic parameter settings tag definitions for netX100/netX500.
  Tag code: HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX100_PARAMS
**************************************************************************************/
#define HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_PINTYPE_NONE   (0)
#define HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_PINTYPE_GPIO   (1)
#define HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_PINTYPE_PIO    (2)

typedef struct
{
  uint8_t      bSelectPinType;   /* see value definitions above */
  uint8_t      bSelectPinInvert; /* 0: non invert, 1: invert */
  uint8_t      bSelectPinIdx;    /* gpio/pio number */
  uint8_t      bReserved;
} HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX100_PARAMS_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                                      tHeader;
  HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX100_PARAMS_DATA_T  tData;
} HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX100_PARAMS_T;


/**************************************************************************************
  Ethercat Master ENI BusState behavior tag definitions.
  Tag code: HIL_TAG_ECM_ENI_BUS_STATE
**************************************************************************************/
#define HIL_TAG_ECM_ENI_BUS_STATE_OFF  (0) /* BusState off after ChannelInit (application controlled startup) */
#define HIL_TAG_ECM_ENI_BUS_STATE_ON   (1) /* BusState on after ChannelInit (automatic startup) */

typedef struct
{
  /* Target bus state for ENI files on ChannelInit */
  uint32_t ulTargetBusState;
} HIL_TAG_ECM_ENI_BUS_STATE_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                 tHeader;
  HIL_TAG_ECM_ENI_BUS_STATE_DATA_T tData;
} HIL_TAG_ECM_ENI_BUS_STATE_T;


/**************************************************************************************
  Ethernet/IP DeviceID settings tag definitions.
  Tag code: HIL_TAG_EIP_DEVICEID
**************************************************************************************/
typedef struct
{
  uint16_t usVendorID;
  uint16_t usDeviceType;
  uint16_t usProductCode;
  uint8_t  bMajRev;
  uint8_t  bMinRev;
  uint8_t  abProductName[32];
} HIL_TAG_EIP_DEVICEID_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T            tHeader;
  HIL_TAG_EIP_DEVICEID_DATA_T tData;
} HIL_TAG_EIP_DEVICEID_T;


/**************************************************************************************
  Ethernet/IP DLR configuration.
  Tag code: HIL_TAG_EIP_DLR_PROTOCOL
**************************************************************************************/
#define HIL_TAG_EIP_DLR_PROTOCOL_OFF      0   /* DLR functionality is turned OFF */
#define HIL_TAG_EIP_DLR_PROTOCOL_ON       1   /* DLR functionality is turned ON */

typedef struct
{
  uint32_t ulEnableDLR;           /* Enable/disable DLR functionality */

} HIL_TAG_EIP_DLR_PROTOCOL_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                tHeader;
  HIL_TAG_EIP_DLR_PROTOCOL_DATA_T tData;
} HIL_TAG_EIP_DLR_PROTOCOL_T;


/**************************************************************************************
  Ethernet/IP resources configuration.
  Tag code: HIL_TAG_EIP_EIS_RESOURCES
**************************************************************************************/
typedef struct
{
  /** Maximum number of supported CIP services the host can register (not bound to any user object), default = 10 */
  uint16_t usMaxUserServices;

  /** Maximum overall number of CIP objects (Stack-internal and host-registered), default = 32 */
  uint16_t usMaxObjects;

  /* Maximum number of Assembly Instances the host can register, default = 10 */
  uint16_t usMaxAssemblyInstance;

  /** Depth of each UDP receive queue (there is exactly one global queue for I/O frames, and one queue for Encapsulation/UDP), default = 8 */
  uint16_t usMaxUdpQueueElements;

  /** Maximum number of TCP connections/sockets
   *  Maximum number of concurrent class 3 connections
   *  Maximum number of concurrent UCCM requests
   *  default = 8
   */
  uint16_t usMaxTcpConnections;

  /** Depth of the TCP send and receive queues for each TCP connection, default = 8*/
  uint16_t usMaxTcpQueueElements;

  /** Maximum number of parallel IO connections, default = 5 */
  uint16_t usMaxIOConnections;
} HIL_TAG_EIP_EIS_RESOURCES_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                  tHeader;
  HIL_TAG_EIP_EIS_RESOURCES_DATA_T  tData;
} HIL_TAG_EIP_EIS_RESOURCES_T;


/**************************************************************************************
  Ethernet/IP configuration. (For internal use only)
  Tag code: HIL_TAG_EIP_EIS_CONFIG
**************************************************************************************/
typedef struct
{
  HIL_TAG_HEADER_T                 tHeader;
  HIL_TAG_EIP_EIS_RESOURCES_DATA_T tData;
} HIL_TAG_EIP_EIS_CONFIG_T;


/**************************************************************************************
  PROFINET ProductID settings tag definitions.
  Tag code: HIL_TAG_PN_DEVICEID
**************************************************************************************/
typedef struct
{
  uint32_t ulVendorId;  /* the PROFINET VendorID to use */
  uint32_t ulDeviceId;  /* the PROFINET DeviceID to use */
} HIL_TAG_PN_DEVICEID_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T             tHeader;
  HIL_TAG_PN_DEVICEID_DATA_T   tData;
} HIL_TAG_PN_DEVICEID_T;


/**************************************************************************************
  PROFINET IO-Device Feature settings tag definitions.
  Tag code: HIL_TAG_PROFINET_FEATURES
**************************************************************************************/
typedef struct
{
  uint8_t      bNumAdditionalIoAR;     /* 0: only 1 cyclic Profinet connection is possible, for allowed values refer to PNS API Manual for details */
  uint8_t      bIoSupervisorSupported; /* 0: IO Supervisor communication is not accepted by firmware / 1: IO Supervisor communication is accepted by firmware */
  uint8_t      bIRTSupported;          /* 0: IRT communication is not accepted by firmware / 1: IRT communication is accepted by firmware */
  uint8_t      bReserved;
  uint16_t     usMinDeviceInterval;    /* the MinDeviceInterval according to GSDML file of the product (allowed values: Power of two in range [8 - 4096]) */
  uint8_t      abReserved[2];
} HIL_TAG_PROFINET_FEATURES_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                      tHeader;
  HIL_TAG_PROFINET_FEATURES_DATA_T      tData;
} HIL_TAG_PROFINET_FEATURES_T;


/**************************************************************************************
  PROFINET IO-Device Feature V2 settings tag definitions.
  Tag code: HIL_TAG_PROFINET_FEATURES_V2
**************************************************************************************/
typedef struct
{
  /** Maximum number of user submodules supported by the product. Allowed values [1, 1000] */
  uint16_t     usNumSubmodules;
  /* Minimum size of RPC buffers in KB. Allowed values [4, 64]. */
  uint8_t      bRPCBufferSize;
  /** Number of additional IO ARs available for Shared Device and SystemRedundancy. Nonzero value enables additional IO connections. For allowed values refer to PNS API Manual for details */
  uint8_t      bNumAdditionalIOAR;
  /** Number of implicit ARs used for Read Implicit Services. Allowed values [1, 8]. */
  uint8_t      bNumImplicitAR;
  /** Number of parallel Device Access ARs supported by device. Allowed values [0, 1]. */
  uint8_t      bNumDAAR;
  /** Maximum number of diagnosis entries generated by application. Allowed values [0, 1000] */
  uint16_t     usNumSubmDiagnosis;
} HIL_TAG_PROFINET_FEATURES_V2_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                         tHeader;
  HIL_TAG_PROFINET_FEATURES_V2_DATA_T      tData;
} HIL_TAG_PROFINET_FEATURES_V2_T;


/**************************************************************************************
  PROFINET IO-Device SystemRedundancy Feature tag definitions.
  Tag code: HIL_TAG_PROFINET_SYSTEM_REDUNDANCY_FEATURES
**************************************************************************************/
typedef struct
{
  /** Number of AR Sets supported by the device. Set to non-zero value to allow SR type connections. Allowed values [0, 1]. */
  uint8_t      bNumberOfARSets;
  /** 32Bits alignment */
  uint8_t      abPadding[3];
} HIL_TAG_PROFINET_SYSTEM_REDUNDANCY_FEATURES_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                                        tHeader;
  HIL_TAG_PROFINET_SYSTEM_REDUNDANCY_FEATURES_DATA_T      tData;
} HIL_TAG_PROFINET_SYSTEM_REDUNDANCY_FEATURES_T;


/**************************************************************************************
  IP Ports for usage with IP 0.0.0.0.
  Tag code: HIL_TAG_LWIP_PORTS_FOR_IP_ZERO

  With this tag, you can set IP ports which the LWIP stack integrated in firmware
  shall handle even if it does not have a valid IP configuration.
  These ports will be usable with Multicast communication only.

  Setting a port to 0 means that this entry in taglist shall be ignored. This allows
  e.g. only activating a single port (or even zero ports).

  By default (if taglist is unchanged) the ports shall be set to 0.
**************************************************************************************/
typedef struct
{
  uint16_t      usPort0;
  uint16_t      usPort1;

} HIL_TAG_LWIP_PORTS_FOR_IP_ZERO_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                       tHeader;
  HIL_TAG_LWIP_PORTS_FOR_IP_ZERO_DATA_T  tData;
} HIL_TAG_LWIP_PORTS_FOR_IP_ZERO_T;







/**************************************************************************************
  Backward Compatibility Definitions.
  NOTE: It is recommend to update components which using those definitions
**************************************************************************************/
#ifndef HIL_TAG_DISABLE_COMPATIBILITY_MODE
  #define HIL_MOD_TAG_IGNORE_FLAG                     HIL_TAG_IGNORE_FLAG
  #define HIL_MOD_TAG_END                             HIL_TAG_END_OF_LIST
  #define HIL_MOD_TAG_NUM_COMM_CHANNEL                HIL_TAG_NUM_COMM_CHANNEL
  #define HIL_MOD_TAG_IT_STATIC_TASKS                 HIL_TAG_TASK_GROUP
  #define HIL_MOD_TAG_IT_STATIC_TASK_PARAMETER_BLOCK  HIL_TAG_IT_STATIC_TASK_PARAMETER_BLOCK
  #define HIL_MOD_TAG_IT_STATIC_TASK_ENTRY            HIL_TAG_IT_STATIC_TASK_ENTRY
  #define HIL_MOD_TAG_IT_TIMER                        HIL_TAG_TIMER
  #define HIL_MOD_TAG_IT_INTERRUPT                    HIL_TAG_INTERRUPT_GROUP
  #define HIL_MOD_TAG_IT_LED                          HIL_TAG_LED
  #define HIL_MOD_TAG_IT_XC                           HIL_TAG_XC
  #define HIL_MOD_TAG_IT_LED_RESOURCE_TYPE_PIO        HIL_TAG_LED_RESOURCE_TYPE_PIO
  #define HIL_MOD_TAG_IT_LED_RESOURCE_TYPE_GPIO       HIL_TAG_LED_RESOURCE_TYPE_GPIO
  #define HIL_MOD_TAG_IT_LED_RESOURCE_TYPE_HIFPIO     HIL_TAG_LED_RESOURCE_TYPE_HIFPIO
  #define HIL_MOD_TAG_IT_LED_POLARITY_NORMAL          HIL_TAG_LED_POLARITY_NORMAL
  #define HIL_MOD_TAG_IT_LED_POLARITY_INVERTED        HIL_TAG_LED_POLARITY_INVERTED


  typedef HIL_TAG_HEADER_T                            HIL_MODULE_TAG_ENTRY_HEADER_T;
  typedef HIL_TAG_IDENTIFIER_T                        HIL_MOD_TAG_IDENTIFIER_T;
  typedef HIL_TAG_UINT32_T                            HIL_MODULE_TAG_ENTRY_UINT32_T;
  typedef HIL_TAG_TASK_GROUP_DATA_T                   HIL_MOD_TAG_IT_STATIC_TASKS_T;
  typedef HIL_TAG_TASK_GROUP_T                        HIL_MOD_TAG_IT_STATIC_TASKS_TAG_T;
  typedef HIL_TAG_INTERRUPT_GROUP_DATA_T              HIL_MOD_TAG_IT_INTERRUPT_T;
  typedef HIL_TAG_INTERRUPT_GROUP_T                   HIL_MOD_TAG_IT_INTERRUPT_TAG_T;
  typedef HIL_TAG_TIMER_DATA_T                        HIL_MOD_TAG_IT_TIMER_T;
  typedef HIL_TAG_TIMER_T                             HIL_MOD_TAG_IT_TIMER_TAG_T;
  typedef HIL_TAG_LED_DATA_T                          HIL_MOD_TAG_IT_LED_T;
  typedef HIL_TAG_LED_T                               HIL_MOD_TAG_IT_LED_TAG_T;
  typedef HIL_TAG_XC_DATA_T                           HIL_MOD_TAG_IT_XC_T;
  typedef HIL_TAG_XC_T                                HIL_MOD_TAG_IT_XC_TAG_T;

  typedef struct
  {
    char szTaskName[16];      /* task name, read-only */
    uint32_t ulTaskGroupRef;  /* group reference number (common to all tasks in the group), read-only */
    uint32_t ulPriority;      /* task priority (offset) relative to task group's base task priority */
    uint32_t ulToken;         /* task token (offset) relative to task group's base task token */
  } HIL_MOD_TAG_IT_STATIC_TASK_ENTRY_T;

  typedef struct
  {
    HIL_MODULE_TAG_ENTRY_HEADER_T      tHeader;
    HIL_MOD_TAG_IT_STATIC_TASK_ENTRY_T tData;
  } HIL_MOD_TAG_IT_STATIC_TASK_ENTRY_TAG_T;

  /* generic task parameter block / substructure referenced by index */
  typedef struct
  {
    uint32_t ulSubstructureIdx;  /* read-only */
    uint32_t ulTaskIdentifier;   /* read-only, Task identifier as specified in TLR_TaskIdentifier.h */
    uint32_t ulParameterVersion; /* read-only, parameter version as specified by particular task */
  } HIL_MOD_TAG_IT_STATIC_TASK_PARAMETER_BLOCK_T;
#endif /* HIL_TAG_DISABLE_COMPATIBILITY_MODE */
/**************************************************************************************
  End of backward compatibility Definitions.
**************************************************************************************/




#endif  /* HIL_TAGLIST_H_ */

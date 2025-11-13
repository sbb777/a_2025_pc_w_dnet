/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: HIL_ProductCodeCip.h$:

  Description:
    Definitions of Product codes for CIP devices from Hilscher
    CIP Vendor code of Hilscher is: 283
    For each Product Type up to 2^16 Product codes can be assigned

    The number range is administered by the SPC Group.

**************************************************************************************/
#ifndef __HIL_PRODUCTCODECIP_H
#define __HIL_PRODUCTCODECIP_H

#ifdef __cplusplus
  extern "C" {
#endif  /* __cplusplus */

/* Our CIP Vendor ID
 * Shall only be used with devices directly sold by Hilscher (e.g. cifX or netTAPs)
 */
#define CIP_VENDORID_HILSCHER 283

  /* Product Type: 0x00 Generic Device
   * Deprecated for new devices.
   * Instead, define the new Product Code for Device Type 0x2B (Generic Device, keyable) */
  #define PRODUCT_TYP_GENERIC 0x00

    #define PRODUCT_CODE_GEN_GENERIC          0x0000
    #define PRODUCT_CODE_EIS_NETX_CBAB32      0x0100

  /* The following product types are not used by Hilscher:
   * - 0x01 Control Station (Obsolete Device Profile)
   * - 0x02 AC Drive Device
   * - 0x03 Motor Overload
   * - 0x04 Limit Switch
   * - 0x05 Inductive Proximity Switch
   * - 0x06 Photoelectric Sensor
   * - 0x07 General Purpose Discrete I/O
   * - 0x08 Encoder (Obsolete Device Profile)
   * - 0x09 Resolver
   * - 0x0A General Purpose Analog I/O (Obsolete Device Profile)
   */

  /* Product Type: 0x0C Communications Adapter */
  #define PRODUCT_TYP_COMMUNICATION    0x0C

    /* DeviceNet communication product range: 0x0000 - 0x00FF */
    #define PRODUCT_CODE_RESERVED1          0x0000
    #define PRODUCT_CODE_RESERVED2          0x0001
    #define PRODUCT_CODE_DPM_1788           0x0001
    #define PRODUCT_CODE_DPS_1788           0x0002
    #define PRODUCT_CODE_DNS_COM            0x0002
    #define PRODUCT_CODE_DNS_CIF30          0x0003
    #define PRODUCT_CODE_DPM_1769           0x0003
    #define PRODUCT_CODE_DNS_PKV30          0x0004
    #define PRODUCT_CODE_DPS_1769           0x0004
    #define PRODUCT_CODE_DNM_COM            0x0005
    #define PRODUCT_CODE_DNM_CIF30          0x0006
    #define PRODUCT_CODE_DNM_CIF104         0x0007
    #define PRODUCT_CODE_DNM_CIF50          0x0008
    #define PRODUCT_CODE_DNM_CIF60          0x0009
    #define PRODUCT_CODE_DNS_CIF60          0x000A
    #define PRODUCT_CODE_DNS_CIF50          0x000B
    #define PRODUCT_CODE_DNM_CIF100         0x000C
    #define PRODUCT_CODE_DNM_CIF80          0x000D
    #define PRODUCT_CODE_DNM_PMC            0x000E
    #define PRODUCT_CODE_DNS_CIF80          0x000F
    #define PRODUCT_CODE_DNS_PMC            0x0010
    #define PRODUCT_CODE_DNS_COMCA          0x0011
    #define PRODUCT_CODE_DNS_EC1            0x0013
    #define PRODUCT_CODE_DNM_PKV50          0x0014
    #define PRODUCT_CODE_DNM_COMC           0x0015
    #define PRODUCT_CODE_DNM_CIF104P        0x0016
    #define PRODUCT_CODE_DNM_NETTAP         0x0017
    #define PRODUCT_CODE_DNS_CIF104P        0x0017
    #define PRODUCT_CODE_DNS_CBEC1          0x0018
    #define PRODUCT_CODE_DNS_NETTAP         0x0019
    #define PRODUCT_CODE_DNS_NT40           0x001A
    #define PRODUCT_CODE_DNM_CIFX50         0x001B
    #define PRODUCT_CODE_DNS_CIFX50         0x001C
    #define PRODUCT_CODE_DNM_NXSB100        0x001D
    #define PRODUCT_CODE_DNS_NXSB100        0x001E
    #define PRODUCT_CODE_DNM_NETX_EVAL      0x001F
    #define PRODUCT_CODE_DNS_NETX_EVAL      0x0020
    #define PRODUCT_CODE_DNM_NETX           0x0021
    #define PRODUCT_CODE_DNS_NETX           0x0022
    #define PRODUCT_CODE_DNS_NIC50          0x0023
    #define PRODUCT_CODE_DNM_NETX_50        0x0024
    #define PRODUCT_CODE_DNS_NETX_50        0x0025
    #define PRODUCT_CODE_DNM_NETX_100       0x0026
    #define PRODUCT_CODE_DNS_NETX_100       0x0027
    #define PRODUCT_CODE_DNM_NETX_500       0x0028
    #define PRODUCT_CODE_DNS_NETX_500       0x0029
    #define PRODUCT_CODE_DNS_NXSTK          0x002A
    #define PRODUCT_CODE_DNS_NT100          0x002B
    #define PRODUCT_CODE_DNM_NT100          0x002C
    #define PRODUCT_CODE_DNS_NT50           0x002D
    #define PRODUCT_CODE_DNL_NT50           0x002E
    #define PRODUCT_CODE_DNS_NB100          0x002F
    #define PRODUCT_CODE_DNM_NB100          0x0030
    #define PRODUCT_CODE_DNM_COMX           0x0031   /* COMX 100XX-DN/DNM */
    #define PRODUCT_CODE_DNS_COMX           0x0032   /* COMX 100XX-DN/DNS */
    #define PRODUCT_CODE_DNM_NETJACK100     0x0033   /* NJ 100XX-DN/DNM   */
    #define PRODUCT_CODE_DNS_NETJACK100     0x0034   /* NJ 100XX-DN/DNS   */
    #define PRODUCT_CODE_DNS_NETJACK10      0x0035   /* NJ 10X-DNS/DNS    */
    #define PRODUCT_CODE_DNS_COMX10         0x0036   /* COMX 10XX-DNS/DNS */
    #define PRODUCT_CODE_DNS_NETX_10        0x0037   /* NETX 10-DN /DNS   */
    #define PRODUCT_CODE_DNS_NETX_51        0x0038
    #define PRODUCT_CODE_DNS_NETJACK51      0x0039
    #define PRODUCT_CODE_DNS_COMX51         0x003A
    #define PRODUCT_CODE_DNS_NETX_52        0x003B
    #define PRODUCT_CODE_DNS_NRP_10         0x003C
    #define PRODUCT_CODE_DNS_COMX52         0x003D
    #define PRODUCT_CODE_DNS_NETX_90        0x003E
    #define PRODUCT_CODE_DNS_NETX_4000      0x003F
    #define PRODUCT_CODE_DNS_NETJACK52      0x0040
    #define PRODUCT_CODE_DNS_CIFX_FAMILY    0x0041
    #define PRODUCT_CODE_DNS_NETX_FAMILY    0x0042
    #define PRODUCT_CODE_DNS_COMX_FAMILY    0x0043
    #define PRODUCT_CODE_DNS_NETJACK_FAMILY 0x0044

    /* EtherNetIP communication product range: 0x0100 - 0x01FF */
    #define PRODUCT_CODE_EIS_NETX_CIFX              0x0101
    #define PRODUCT_CODE_EIM_NETX_CIFX              0x0102
    #define PRODUCT_CODE_EIS_NETX_COMX              0x0103
    #define PRODUCT_CODE_EIM_NETX_COMX              0x0104
    #define PRODUCT_CODE_EIS_NETX500                0x0105
    #define PRODUCT_CODE_EIM_NETX500                0x0106
    #define PRODUCT_CODE_EIS_NETX50                 0x0107
    #define PRODUCT_CODE_EIM_NETX50                 0x0108
    #define PRODUCT_CODE_EIS_NETX100                0x0109
    #define PRODUCT_CODE_EIM_NETX100                0x010A
    #define PRODUCT_CODE_EIS_NXSTK                  0x010B
    #define PRODUCT_CODE_EIS_NIC50                  0x010D
    #define PRODUCT_CODE_EIM_NIC50                  0x010E
    #define PRODUCT_CODE_EIS_NETTAP                 0x010F
    #define PRODUCT_CODE_EIS_NXIO50                 0x0110
    #define PRODUCT_CODE_EIS_NETBRICK100            0x0111
    #define PRODUCT_CODE_EIM_NETBRICK100            0x0112
    #define PRODUCT_CODE_EIS_NETTAP50               0x0113
    #define PRODUCT_CODE_EIL_NETTAP50               0x0114
    #define PRODUCT_CODE_EIM_NETTAP                 0x0115
    #define PRODUCT_CODE_EIS_NETJACK50              0x0116
    #define PRODUCT_CODE_EIS_NETJACK100             0x0117
    #define PRODUCT_CODE_EIM_NETJACK100             0x0118
    #define PRODUCT_CODE_EIS_NETX_COMX51            0x0119
    #define PRODUCT_CODE_EIS_NETJACK51              0x011A
    #define PRODUCT_CODE_EIS_NETX_51                0x011B
    #define PRODUCT_CODE_EIS_NETX_52                0x011C
    #define PRODUCT_CODE_EIS_NRP_52                 0x011D
    #define PRODUCT_CODE_EIS_NIC52                  0x011E
    #define PRODUCT_CODE_EIS_NTEDGE                 0x011F  /* NT IJCX-GB-RE */
    #define PRODUCT_CODE_EIS_NTTAP151               0x0120  /* NT 151 */
    #define PRODUCT_CODE_EIM_NTTAP151               0x0121  /* NT 151 */
    #define PRODUCT_CODE_EIS_NXHAT52                0x0122  /* NxHAT52  */
    #define PRODUCT_CODE_EIS_NRP_51                 0x0123
    #define PRODUCT_CODE_EIM_NETX4000               0x0124
    #define PRODUCT_CODE_EIS_NIOT_E_TIB100_GE_RE    0x0125
    #define PRODUCT_CODE_EIS_NIOT_E_TPI51_EN_RE     0x0126
    #define PRODUCT_CODE_EIS_NIOT_E_NPI3_51_EN      0x0127
    #define PRODUCT_CODE_EIS_NRP_51_IO              0x0128
    #define PRODUCT_CODE_EIS_NETX_90                0x0129
    #define PRODUCT_CODE_EIS_NETX_4000              0x012A
    #define PRODUCT_CODE_EIS_NRP_90                 0x012B  /* NRP 90-RE/EIS      */
    #define PRODUCT_CODE_EIS_CIFX_NETX90            0x012C  /* CIFX NETX90 RE/EIS */

    /* CompoNet communication product range: 0x0200 - 0x02FF */
    #define PRODUCT_CODE_CPS_NETX       0x0200
    #define PRODUCT_CODE_CPS_CIFX       0x0201
    #define PRODUCT_CODE_CPS_NIC50      0x0202
    #define PRODUCT_CODE_CPS_NETX_50    0x0203
    #define PRODUCT_CODE_CPS_NETX_100   0x0204
    #define PRODUCT_CODE_CPS_NETX_500   0x0205
    #define PRODUCT_CODE_CPS_NETX_10    0x0206
    #define PRODUCT_CODE_CPS_COMX_10    0x0207
    #define PRODUCT_CODE_CPS_NIC10      0x0208

    /* Demo devices (used for certification purposes) product range: 0x0300 - 0x03FF */
    #define PRODUCT_CODE_EIS_SIMPLE_DEMO_1    0x0300  /*!< Simple demo application for Stack V2         */
    #define PRODUCT_CODE_EIS_EXTENDED_DEMO_1  0x0301  /*!< Extended demo application for Stack V2       */
    #define PRODUCT_CODE_EIS_SIMPLE_DEMO_2    0x0302  /*!< Simple demo application for Stack V3 and V5  */
    #define PRODUCT_CODE_EIS_EXTENDED_DEMO_2  0x0303  /*!< Extended demo for Stack V3 and V5            */

    /* netFIELD product range, for more information see (kb.hilscher.com/x/VhXVBw): 0x0400 - 0x04FF */
    #define PRODUCT_CODE_NFD30_90_EIS_IOLMW       0x0400  /*!< 16 Track Wireless IO-Link Master 30 mm */
    #define PRODUCT_CODE_NFD60_90_EIS_IOLMA_12P   0x0401  /*!< 8 Port Class A IO-Link Master 60 mm */
    #define PRODUCT_CODE_NFD60_90_EIS_16DIO_12P   0x0402  /*!< 16 Digital Input/Output 60 mm */
    #define PRODUCT_CODE_NFD60_90_RTE_IOLMA_12P   0x0403  /*!< 8 Port Class A IO-Link Master 60 mm EVAL*/
    #define PRODUCT_CODE_NFD30_90_RTE_IOLMW       0x0404  /*!< 16 Track Wireless IO-Link Master 30 mm EVAL*/
    #define PRODUCT_CODE_NFD60_90_EIS_16DI_12P    0x0405  /*!< 16 Digital Input 60 mm */
    #define PRODUCT_CODE_NFD60_90_EIS_16DO_12P    0x0406  /*!< 16 Digital Output 60 mm */



  /* The following product types are not used by Hilscher:
   * - 0x0D Barcode Scanner (Obsolete Device Profile)
   * - 0x0E Programmable Logic Controller
   * - 0x10 Position Controller
   * - 0x11 Weigh Scale (Obsolete Device Profile)
   * - 0x12 Message Display (Obsolete Device Profile)
   * - 0x14 Servo Drives (Obsolete Device Profile)
   * - 0x15 Contactor
   * - 0x16 Motor Starter
   * - 0x17 Softstart Starter
   * - 0x18 Human-Machine Interface
   * - 0x19 Pneumatic Valve(s) (Obsolete Device Profile)
   * - 0x1A Mass Flow Controller
   * - 0x1B Pneumatic Valve(s)
   * - 0x1C Vacuum Pressure Gauge
   * - 0x1D Process Control Valve
   * - 0x1E Residual Gas Analyzer
   * - 0x1F DC Power Generator
   * - 0x20 RF Power Generator
   * - 0x21 Turbomolecular Vacuum Pump
   * - 0x22 Encoder
   * - 0x23 Safety Discrete I/O Device
   * - 0x24 Fluid Flow Controller
   * - 0x25 CIP Motion Drive
   * - 0x26 CompoNet Repeater
   * - 0x27 Mass Flow Controller
   * - 0x28 CIP Modbus Device
   * - 0x29 CIP Modbus Translator
   * - 0x2A Safety Analog I/O Device
   */

  /* Product Type: 0x2B Generic Device (keyable) */
  #define PRODUCT_TYP_GENERIC_KEYABLE 0x2B

    #define PRODUCT_CODE_NS90_RE_SPE_EIS      0x0101 /* netSwitch netX90 SPE */



  /* The following product types are not used by Hilscher:
   * - 0x2C Managed Ethernet Switch
   * - 0x2D CIP Motion Safety Drive Device
   * - 0x2E Safety Drive Device
   * - 0x2F CIP Motion Encoder
   * - 0x30 CIP Motion Converter
   * - 0x31 CIP Motion I/O
   * - 0x32 ControlNet Physical Layer Component
   * - 0x33 Circuit Breaker
   * - 0x34 HART Device
   * - 0x35 CIP-HART Translator
   * - 0x36 IO-Link Master
   * - 0xC8 Embedded Component
   */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __HIL_PRODUCTCODECIP_H */

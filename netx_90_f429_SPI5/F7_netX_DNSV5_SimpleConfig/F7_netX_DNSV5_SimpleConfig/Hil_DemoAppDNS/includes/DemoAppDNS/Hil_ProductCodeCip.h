/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: HIL_ProductCodeCip.h$:

  Description:
    Definitions of Product codes

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2010-03-23  File created.

**************************************************************************************/


#ifndef __HIL_PRODUCTCODECIP_H
#define __HIL_PRODUCTCODECIP_H

#ifdef __cplusplus
  extern "C" {
#endif  /* __cplusplus */


  #define CIP_VENDORID_HILSCHER 283

  /*
  ** Product Type 000 Generic Device
  */
  #define PRODUCT_TYP_GENERIC 0

  #define PRODUCT_CODE_GEN_GENERIC          0x0000

  #define PRODUCT_CODE_EIS_NETX_CBAB32      0x0100

  /*
  ** Product Type 002 AC Drive
  */

  /*
  ** Product Type 003 Motor Overload
  */

  /*
  ** Product Type 004 Limit Switch
  */

  /*
  ** Product Type 005 Inductive Proximity Switch
  */

  /*
  ** Product Type 006 Photoelectric Sensor
  */

  /*
  ** Product Type 007 General Purpose Discrete I/O
  */

  /*
  ** Product Type 011 Demo Adapter
  */


  /*
  ** Product Type 12 Communication Adapter
  */
    #define PRODUCT_TYP_COMMUNICATION    12

    /* 0x0000 - 0x00FF: DeviceNet */
    #define PRODUCT_CODE_RESERVED1       0x0000
    #define PRODUCT_CODE_RESERVED2       0x0001
    #define PRODUCT_CODE_DPM_1788        0x0001
    #define PRODUCT_CODE_DPS_1788        0x0002
    #define PRODUCT_CODE_DNS_COM         0x0002
    #define PRODUCT_CODE_DNS_CIF30       0x0003
    #define PRODUCT_CODE_DPM_1769        0x0003
    #define PRODUCT_CODE_DNS_PKV30       0x0004
    #define PRODUCT_CODE_DPS_1769        0x0004
    #define PRODUCT_CODE_DNM_COM         0x0005
    #define PRODUCT_CODE_DNM_CIF30       0x0006
    #define PRODUCT_CODE_DNM_CIF104      0x0007
    #define PRODUCT_CODE_DNM_CIF50       0x0008
    #define PRODUCT_CODE_DNM_CIF60       0x0009
    #define PRODUCT_CODE_DNS_CIF60       0x000A
    #define PRODUCT_CODE_DNS_CIF50       0x000B
    #define PRODUCT_CODE_DNM_CIF100      0x000C
    #define PRODUCT_CODE_DNM_CIF80       0x000D
    #define PRODUCT_CODE_DNM_PMC         0x000E
    #define PRODUCT_CODE_DNS_CIF80       0x000F
    #define PRODUCT_CODE_DNS_PMC         0x0010
    #define PRODUCT_CODE_DNS_COMCA       0x0011
    #define PRODUCT_CODE_DNS_EC1         0x0013
    #define PRODUCT_CODE_DNM_PKV50       0x0014
    #define PRODUCT_CODE_DNM_COMC        0x0015
    #define PRODUCT_CODE_DNM_CIF104P     0x0016
    #define PRODUCT_CODE_DNM_NETTAP      0x0017
    #define PRODUCT_CODE_DNS_CIF104P     0x0017
    #define PRODUCT_CODE_DNS_CBEC1       0x0018
    #define PRODUCT_CODE_DNS_NETTAP      0x0019
    #define PRODUCT_CODE_DNS_NT40        0x001A
    #define PRODUCT_CODE_DNM_CIFX50      0x001B
    #define PRODUCT_CODE_DNS_CIFX50      0x001C
    #define PRODUCT_CODE_DNM_NXSB100     0x001D
    #define PRODUCT_CODE_DNS_NXSB100     0x001E
    #define PRODUCT_CODE_DNM_NETX_EVAL   0x001F
    #define PRODUCT_CODE_DNS_NETX_EVAL   0x0020
    #define PRODUCT_CODE_DNM_NETX        0x0021
    #define PRODUCT_CODE_DNS_NETX        0x0022
    #define PRODUCT_CODE_DNS_NIC50       0x0023
    #define PRODUCT_CODE_DNM_NETX_50     0x0024
    #define PRODUCT_CODE_DNS_NETX_50     0x0025
    #define PRODUCT_CODE_DNM_NETX_100    0x0026
    #define PRODUCT_CODE_DNS_NETX_100    0x0027
    #define PRODUCT_CODE_DNM_NETX_500    0x0028
    #define PRODUCT_CODE_DNS_NETX_500    0x0029
    #define PRODUCT_CODE_DNS_NXSTK       0x002A
    #define PRODUCT_CODE_DNS_NT100       0x002B
    #define PRODUCT_CODE_DNM_NT100       0x002C
    #define PRODUCT_CODE_DNS_NT50        0x002D
    #define PRODUCT_CODE_DNL_NT50        0x002E
    #define PRODUCT_CODE_DNS_NB100       0x002F
    #define PRODUCT_CODE_DNM_NB100       0x0030
    #define PRODUCT_CODE_DNM_COMX        0x0031   /* COMX 100XX-DN/DNM */
    #define PRODUCT_CODE_DNS_COMX        0x0032   /* COMX 100XX-DN/DNS */
    #define PRODUCT_CODE_DNM_NETJACK100  0x0033   /* NJ 100XX-DN/DNM   */
    #define PRODUCT_CODE_DNS_NETJACK100  0x0034   /* NJ 100XX-DN/DNS   */
    #define PRODUCT_CODE_DNS_NETJACK10   0x0035   /* NJ 10X-DNS/DNS    */
    #define PRODUCT_CODE_DNS_COMX10      0x0036   /* COMX 10XX-DNS/DNS */
    #define PRODUCT_CODE_DNS_NETX_10     0x0037   /* NETX 10-DN /DNS   */
    #define PRODUCT_CODE_DNS_NETX_51     0x0038
    #define PRODUCT_CODE_DNS_NETJACK51   0x0039
    #define PRODUCT_CODE_DNS_COMX51      0x003A
    #define PRODUCT_CODE_DNS_NETX_52     0x003B
    #define PRODUCT_CODE_DNS_NRP_10      0x003C

    /* 0x0100 - 0x01FF: EtherNetIP */
    #define PRODUCT_CODE_EIS_NETX_CIFX   0x0101
    #define PRODUCT_CODE_EIM_NETX_CIFX   0x0102
    #define PRODUCT_CODE_EIS_NETX_COMX   0x0103
    #define PRODUCT_CODE_EIM_NETX_COMX   0x0104
    #define PRODUCT_CODE_EIS_NETX500     0x0105
    #define PRODUCT_CODE_EIM_NETX500     0x0106
    #define PRODUCT_CODE_EIS_NETX50      0x0107
    #define PRODUCT_CODE_EIM_NETX50      0x0108
    #define PRODUCT_CODE_EIS_NETX100     0x0109
    #define PRODUCT_CODE_EIM_NETX100     0x010A
    #define PRODUCT_CODE_EIS_NXSTK       0x010B
    #define PRODUCT_CODE_EIS_NIC50       0x010D
    #define PRODUCT_CODE_EIM_NIC50       0x010E
    #define PRODUCT_CODE_EIS_NETTAP      0x010F
    #define PRODUCT_CODE_EIS_NXIO50      0x0110
    #define PRODUCT_CODE_EIS_NETBRICK100 0x0111
    #define PRODUCT_CODE_EIM_NETBRICK100 0x0112
    #define PRODUCT_CODE_EIS_NETTAP50    0x0113
    #define PRODUCT_CODE_EIL_NETTAP50    0x0114
    #define PRODUCT_CODE_EIM_NETTAP      0x0115
    #define PRODUCT_CODE_EIS_NETJACK50   0x0116
    #define PRODUCT_CODE_EIS_NETJACK100  0x0117
    #define PRODUCT_CODE_EIM_NETJACK100  0x0118
    #define PRODUCT_CODE_EIS_NETX_COMX51 0x0119
    #define PRODUCT_CODE_EIS_NETJACK51   0x011A
    #define PRODUCT_CODE_EIS_NETX_51     0x011B
    #define PRODUCT_CODE_EIS_NETX_52     0x011C
    #define PRODUCT_CODE_EIS_NRP_52      0x011D
    #define PRODUCT_CODE_EIS_NIC52       0x011E
    #define PRODUCT_CODE_EIS_NTEDGE      0x011F   /* NT IJCX-GB-RE */
    #define PRODUCT_CODE_EIS_NTTAP151    0x0120   /* NT 151 */
    #define PRODUCT_CODE_EIM_NTTAP151    0x0121   /* NT 151 */
    #define PRODUCT_CODE_EIS_NXHAT52     0x0122   /* NxHAT52  */
    #define PRODUCT_CODE_EIS_NRP_51      0x0123
    #define PRODUCT_CODE_EIM_NETX4000    0x0124
    #define PRODUCT_CODE_EIS_NIOT_E_TIB100_GE_RE 0x125
    #define PRODUCT_CODE_EIS_NIOT_E_TPI51_EN_RE  0x126
    #define PRODUCT_CODE_EIS_NIOT_E_NPI3_51_EN   0x127
    #define PRODUCT_CODE_EIS_NRP_51_IO   0x0128
    #define PRODUCT_CODE_EIS_NETX_90     0x0129
    #define PRODUCT_CODE_EIS_NETX_4000   0x012A




    /* 0x0200 - 0x02FF: CompoNet */
    #define PRODUCT_CODE_CPS_NETX        0x0200
    #define PRODUCT_CODE_CPS_CIFX        0x0201
    #define PRODUCT_CODE_CPS_NIC50       0x0202
    #define PRODUCT_CODE_CPS_NETX_50     0x0203
    #define PRODUCT_CODE_CPS_NETX_100    0x0204
    #define PRODUCT_CODE_CPS_NETX_500    0x0205
    #define PRODUCT_CODE_CPS_NETX_10     0x0206
    #define PRODUCT_CODE_CPS_COMX_10     0x0207
    #define PRODUCT_CODE_CPS_NIC10       0x0208


    /* 0x0300 - 0x03FF: Demo devices (used for certification purposes) */
    #define PRODUCT_CODE_EIS_SIMPLE_DEMO_1    0x0300  /*!< Simple demo application for Stack V2         */
    #define PRODUCT_CODE_EIS_EXTENDED_DEMO_1  0x0301  /*!< Extended demo application for Stack V2       */
    #define PRODUCT_CODE_EIS_SIMPLE_DEMO_2    0x0302  /*!< Simple demo application for Stack V3 and V5  */
    #define PRODUCT_CODE_EIS_EXTENDED_DEMO_2  0x0303  /*!< Extended demo for Stack V3 and V5            */


  /*
  ** Product Type 014 Programmable Logic Controller
  */

  /*
  ** Product Type 016 Position Controller
  */

  /*
  ** Product Type 019 DC Drive
  */

  /*
  ** Product Type 021 Contactor
  */

  /*
  ** Product Type 022 Motor Starter
  */

  /*
  ** Product Type 023 Soft Starter
  */

  /*
  ** Product Type 024 Human Machine Interface
  */

  /*
  ** Product Type 026 Mass Flow Controller
  */

  /*
  ** Product Type 026 Mass Flow Meter
  */

  /*
  ** Product Type 027 Pneumatic Valve
  */

  /*
  ** Product Type 028 Vacuum/Pressure Gauge
  */

  /*
  ** Product Type 029 Process Control Valve
  */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __HIL_PRODUCTCODECIP_H */

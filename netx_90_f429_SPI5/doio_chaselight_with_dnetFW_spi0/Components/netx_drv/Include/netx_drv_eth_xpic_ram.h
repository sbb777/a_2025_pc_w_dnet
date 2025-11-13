/*!************************************************************************//*!
 * \file     netx_drv_eth_xpic_ram.h
 * \brief    Ethernet xpic mac  peripheral module driver.
 * $Revision: 4646 $
 * $Date: 2018-12-19 09:47:36 +0200 (Wed, 19 Dec 2018) $
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
/*********************************************************************/
/* from regdef.v by regdef2h auto-generated C-header file            */
/* please see the regdef.html file for detailed register description */
/*********************************************************************/

#include "netx_drv.h"
#ifndef ETH_XPIC_RAM_regdef_h
#define ETH_XPIC_RAM_regdef_h

#ifdef __cplusplus
extern "C"
{
#endif

/*! \addtogroup netX_Driver
 * \{
 */

/*! \addtogroup Legacy
 * \{
 */

/*!
 * \addtogroup ETH_XPIC
 * \{
 */

#include <stdint.h>

/* ===================================================================== */

/* Area of ETH_XPIC_DRAM */

/* ===================================================================== */


/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_RAM_CONSTANTPOOL */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_RAM_CONSTANTPOOL           0x00000000U

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_RAM_PARAMETERS */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_RAM_PARAMETERS           0x00000400U

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_SHM_ADDR */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_SHM_ADDR           0x00000400U

#define MSK_XPIC_RAM_SHM_ADDR_ADR 0xffffffffU
#define SRT_XPIC_RAM_SHM_ADDR_ADR 0

/* all used bits of 'XPIC_RAM_SHM_ADDR': */
#define MSK_USED_BITS_XPIC_RAM_SHM_ADDR 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_FRMBUF_ADDR */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_FRMBUF_ADDR           0x00000404U

#define MSK_XPIC_RAM_FRMBUF_ADDR_ADR 0xffffffffU
#define SRT_XPIC_RAM_FRMBUF_ADDR_ADR 0

/* all used bits of 'XPIC_RAM_FRMBUF_ADDR': */
#define MSK_USED_BITS_XPIC_RAM_FRMBUF_ADDR 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_FRAMEBUF_SIZE */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_FRAMEBUF_SIZE           0x00000408U

#define MSK_XPIC_RAM_FRAMEBUF_SIZE_VAL 0xffffffffU
#define SRT_XPIC_RAM_FRAMEBUF_SIZE_VAL 0

/* all used bits of 'XPIC_RAM_FRAMEBUF_SIZE': */
#define MSK_USED_BITS_XPIC_RAM_FRAMEBUF_SIZE 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_MACADR_HASH_TABLE0 */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_MACADR_HASH_TABLE0           0x0000040CU

#define MSK_XPIC_RAM_MACADR_HASH_TABLE0_ADR 0xffffffffU
#define SRT_XPIC_RAM_MACADR_HASH_TABLE0_ADR 0

/* all used bits of 'XPIC_RAM_MACADR_HASH_TABLE0': */
#define MSK_USED_BITS_XPIC_RAM_MACADR_HASH_TABLE0 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_MACADR_HASH_TABLE1 */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_MACADR_HASH_TABLE1           0x00000410U

#define MSK_XPIC_RAM_MACADR_HASH_TABLE1_ADR 0xffffffffU
#define SRT_XPIC_RAM_MACADR_HASH_TABLE1_ADR 0

/* all used bits of 'XPIC_RAM_MACADR_HASH_TABLE1': */
#define MSK_USED_BITS_XPIC_RAM_MACADR_HASH_TABLE1 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_MACADR_HASH_TABLE2 */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_MACADR_HASH_TABLE2           0x00000414U

#define MSK_XPIC_RAM_MACADR_HASH_TABLE2_RES 0xffffffffU
#define SRT_XPIC_RAM_MACADR_HASH_TABLE2_RES 0

/* all used bits of 'XPIC_RAM_MACADR_HASH_TABLE2': */
#define MSK_USED_BITS_XPIC_RAM_MACADR_HASH_TABLE2 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_MACADR_HASH_TABLE3 */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_MACADR_HASH_TABLE3           0x00000418U

#define MSK_XPIC_RAM_MACADR_HASH_TABLE3_RES 0xffffffffU
#define SRT_XPIC_RAM_MACADR_HASH_TABLE3_RES 0

/* all used bits of 'XPIC_RAM_MACADR_HASH_TABLE3': */
#define MSK_USED_BITS_XPIC_RAM_MACADR_HASH_TABLE3 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_RX_IRQ_ADR */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_RX_IRQ_ADR           0x0000041CU

#define MSK_XPIC_RAM_RX_IRQ_ADR_ADR 0xffffffffU
#define SRT_XPIC_RAM_RX_IRQ_ADR_ADR 0

/* all used bits of 'XPIC_RAM_RX_IRQ_ADR': */
#define MSK_USED_BITS_XPIC_RAM_RX_IRQ_ADR 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_RX_IRQ_VAL */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_RX_IRQ_VAL           0x00000420U

#define MSK_XPIC_RAM_RX_IRQ_VAL_VAL 0xffffffffU
#define SRT_XPIC_RAM_RX_IRQ_VAL_VAL 0

/* all used bits of 'XPIC_RAM_RX_IRQ_VAL': */
#define MSK_USED_BITS_XPIC_RAM_RX_IRQ_VAL 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_IRQ_ADR */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_IRQ_ADR           0x00000424U

#define MSK_XPIC_RAM_TX_IRQ_ADR_ADR 0xffffffffU
#define SRT_XPIC_RAM_TX_IRQ_ADR_ADR 0

/* all used bits of 'XPIC_RAM_TX_IRQ_ADR': */
#define MSK_USED_BITS_XPIC_RAM_TX_IRQ_ADR 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_IRQ_VAL */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_IRQ_VAL           0x00000428U

#define MSK_XPIC_RAM_TX_IRQ_VAL_VAL 0xffffffffU
#define SRT_XPIC_RAM_TX_IRQ_VAL_VAL 0

/* all used bits of 'XPIC_RAM_TX_IRQ_VAL': */
#define MSK_USED_BITS_XPIC_RAM_TX_IRQ_VAL 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_LOCAL_MAC_ADDRESS_HI */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_LOCAL_MAC_ADDRESS_HI           0x0000042CU

#define MSK_XPIC_RAM_LOCAL_MAC_ADDRESS_HI_VAL 0xffffffffU
#define SRT_XPIC_RAM_LOCAL_MAC_ADDRESS_HI_VAL 0

/* all used bits of 'XPIC_RAM_LOCAL_MAC_ADDRESS_HI': */
#define MSK_USED_BITS_XPIC_RAM_LOCAL_MAC_ADDRESS_HI 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_LOCAL_MAC_ADDRESS_LO */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_LOCAL_MAC_ADDRESS_LO           0x00000430U

#define MSK_XPIC_RAM_LOCAL_MAC_ADDRESS_LO_VAL  0x0000ffffU
#define SRT_XPIC_RAM_LOCAL_MAC_ADDRESS_LO_VAL  0
#define MSK_XPIC_RAM_LOCAL_MAC_ADDRESS_LO_RES1 0xffff0000U
#define SRT_XPIC_RAM_LOCAL_MAC_ADDRESS_LO_RES1 16

/* all used bits of 'XPIC_RAM_LOCAL_MAC_ADDRESS_LO': */
#define MSK_USED_BITS_XPIC_RAM_LOCAL_MAC_ADDRESS_LO 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_ABORT_MODE */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_ABORT_MODE           0x00000434U

#define MSK_XPIC_RAM_TX_ABORT_MODE_VAL 0xffffffffU
#define SRT_XPIC_RAM_TX_ABORT_MODE_VAL 0

/* all used bits of 'XPIC_RAM_TX_ABORT_MODE': */
#define MSK_USED_BITS_XPIC_RAM_TX_ABORT_MODE 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_ETH_CONFIG */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_ETH_CONFIG           0x00000438U

#define MSK_XPIC_RAM_ETH_CONFIG_VAL 0xffffffffU
#define SRT_XPIC_RAM_ETH_CONFIG_VAL 0

/* all used bits of 'XPIC_RAM_ETH_CONFIG': */
#define MSK_USED_BITS_XPIC_RAM_ETH_CONFIG 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_ETH_TX_CONFIG */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_ETH_TX_CONFIG           0x0000043CU

#define MSK_XPIC_RAM_ETH_TX_CONFIG_VAL 0xffffffffU
#define SRT_XPIC_RAM_ETH_TX_CONFIG_VAL 0

/* all used bits of 'XPIC_RAM_ETH_TX_CONFIG': */
#define MSK_USED_BITS_XPIC_RAM_ETH_TX_CONFIG 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_MIIMU_REQ_CYCLE */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_MIIMU_REQ_CYCLE           0x00000440U

#define MSK_XPIC_RAM_MIIMU_REQ_CYCLE_TIME 0xffffffffU
#define SRT_XPIC_RAM_MIIMU_REQ_CYCLE_TIME 0

/* all used bits of 'XPIC_RAM_MIIMU_REQ_CYCLE': */
#define MSK_USED_BITS_XPIC_RAM_MIIMU_REQ_CYCLE 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_MIIMU_READ_PHYLINK */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_MIIMU_READ_PHYLINK           0x00000444U

#define MSK_XPIC_RAM_MIIMU_READ_PHYLINK_VAL 0xffffffffU
#define SRT_XPIC_RAM_MIIMU_READ_PHYLINK_VAL 0

/* all used bits of 'XPIC_RAM_MIIMU_READ_PHYLINK': */
#define MSK_USED_BITS_XPIC_RAM_MIIMU_READ_PHYLINK 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_MIIMU_READ_PHYMODE */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_MIIMU_READ_PHYMODE           0x00000448U

#define MSK_XPIC_RAM_MIIMU_READ_PHYMODE_VAL 0xffffffffU
#define SRT_XPIC_RAM_MIIMU_READ_PHYMODE_VAL 0

/* all used bits of 'XPIC_RAM_MIIMU_READ_PHYMODE': */
#define MSK_USED_BITS_XPIC_RAM_MIIMU_READ_PHYMODE 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_MIIMU_READ_PHYSPEED */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_MIIMU_READ_PHYSPEED           0x0000044CU

#define MSK_XPIC_RAM_MIIMU_READ_PHYSPEED_VAL 0xffffffffU
#define SRT_XPIC_RAM_MIIMU_READ_PHYSPEED_VAL 0

/* all used bits of 'XPIC_RAM_MIIMU_READ_PHYSPEED': */
#define MSK_USED_BITS_XPIC_RAM_MIIMU_READ_PHYSPEED 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_PHYLINK_MSK */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_PHYLINK_MSK           0x00000450U

#define MSK_XPIC_RAM_PHYLINK_MSK_VAL 0xffffffffU
#define SRT_XPIC_RAM_PHYLINK_MSK_VAL 0

/* all used bits of 'XPIC_RAM_PHYLINK_MSK': */
#define MSK_USED_BITS_XPIC_RAM_PHYLINK_MSK 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_PHYMODE_MSK */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_PHYMODE_MSK           0x00000454U

#define MSK_XPIC_RAM_PHYMODE_MSK_VAL 0xffffffffU
#define SRT_XPIC_RAM_PHYMODE_MSK_VAL 0

/* all used bits of 'XPIC_RAM_PHYMODE_MSK': */
#define MSK_USED_BITS_XPIC_RAM_PHYMODE_MSK 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_PHYSPEED_MSK */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_PHYSPEED_MSK           0x00000458U

#define MSK_XPIC_RAM_PHYSPEED_MSK_VAL 0xffffffffU
#define SRT_XPIC_RAM_PHYSPEED_MSK_VAL 0

/* all used bits of 'XPIC_RAM_PHYSPEED_MSK': */
#define MSK_USED_BITS_XPIC_RAM_PHYSPEED_MSK 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_MIIMU_LINK_REQ_NXT */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_MIIMU_LINK_REQ_NXT           0x0000045CU

#define MSK_XPIC_RAM_MIIMU_LINK_REQ_NXT_VAL 0xffffffffU
#define SRT_XPIC_RAM_MIIMU_LINK_REQ_NXT_VAL 0

/* all used bits of 'XPIC_RAM_MIIMU_LINK_REQ_NXT': */
#define MSK_USED_BITS_XPIC_RAM_MIIMU_LINK_REQ_NXT 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_MIIMU_MODE_REQ_NXT */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_MIIMU_MODE_REQ_NXT           0x00000460U

#define MSK_XPIC_RAM_MIIMU_MODE_REQ_NXT_VAL 0xffffffffU
#define SRT_XPIC_RAM_MIIMU_MODE_REQ_NXT_VAL 0

/* all used bits of 'XPIC_RAM_MIIMU_MODE_REQ_NXT': */
#define MSK_USED_BITS_XPIC_RAM_MIIMU_MODE_REQ_NXT 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_MIIMU_SPEED_REQ_NXT */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_MIIMU_SPEED_REQ_NXT           0x00000464U

#define MSK_XPIC_RAM_MIIMU_SPEED_REQ_NXT_VAL 0xffffffffU
#define SRT_XPIC_RAM_MIIMU_SPEED_REQ_NXT_VAL 0

/* all used bits of 'XPIC_RAM_MIIMU_SPEED_REQ_NXT': */
#define MSK_USED_BITS_XPIC_RAM_MIIMU_SPEED_REQ_NXT 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_RAM_VAR */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_RAM_VAR           0x00000468U

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_FRAMEBUF_DATA_SIZE */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_FRAMEBUF_DATA_SIZE           0x00000468U

#define MSK_XPIC_RAM_FRAMEBUF_DATA_SIZE_VAL 0xffffffffU
#define SRT_XPIC_RAM_FRAMEBUF_DATA_SIZE_VAL 0

/* all used bits of 'XPIC_RAM_FRAMEBUF_DATA_SIZE': */
#define MSK_USED_BITS_XPIC_RAM_FRAMEBUF_DATA_SIZE 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_FRMBUF_ADDR_NEG */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_FRMBUF_ADDR_NEG           0x0000046CU

#define MSK_XPIC_RAM_FRMBUF_ADDR_NEG_ADR 0xffffffffU
#define SRT_XPIC_RAM_FRMBUF_ADDR_NEG_ADR 0

/* all used bits of 'XPIC_RAM_FRMBUF_ADDR_NEG': */
#define MSK_USED_BITS_XPIC_RAM_FRMBUF_ADDR_NEG 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_MIIMU_PC */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_MIIMU_PC           0x00000470U

#define MSK_XPIC_RAM_MIIMU_PC_VAL 0xffffffffU
#define SRT_XPIC_RAM_MIIMU_PC_VAL 0

/* all used bits of 'XPIC_RAM_MIIMU_PC': */
#define MSK_USED_BITS_XPIC_RAM_MIIMU_PC 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_MIIMU_REQ_ID */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_MIIMU_REQ_ID           0x00000474U

#define MSK_XPIC_RAM_MIIMU_REQ_ID_VAL 0xffffffffU
#define SRT_XPIC_RAM_MIIMU_REQ_ID_VAL 0

/* all used bits of 'XPIC_RAM_MIIMU_REQ_ID': */
#define MSK_USED_BITS_XPIC_RAM_MIIMU_REQ_ID 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_PHYLINK */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_PHYLINK           0x00000478U

#define MSK_XPIC_RAM_PHYLINK_VAL 0xffffffffU
#define SRT_XPIC_RAM_PHYLINK_VAL 0

/* all used bits of 'XPIC_RAM_PHYLINK': */
#define MSK_USED_BITS_XPIC_RAM_PHYLINK 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_PHYMODE */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_PHYMODE           0x0000047CU

#define MSK_XPIC_RAM_PHYMODE_VAL 0xffffffffU
#define SRT_XPIC_RAM_PHYMODE_VAL 0

/* all used bits of 'XPIC_RAM_PHYMODE': */
#define MSK_USED_BITS_XPIC_RAM_PHYMODE 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_PHYSPEED */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_PHYSPEED           0x00000480U

#define MSK_XPIC_RAM_PHYSPEED_VAL 0xffffffffU
#define SRT_XPIC_RAM_PHYSPEED_VAL 0

/* all used bits of 'XPIC_RAM_PHYSPEED': */
#define MSK_USED_BITS_XPIC_RAM_PHYSPEED 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_SLOT_TIME */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_SLOT_TIME           0x00000484U

#define MSK_XPIC_RAM_SLOT_TIME_VAL 0xffffffffU
#define SRT_XPIC_RAM_SLOT_TIME_VAL 0

/* all used bits of 'XPIC_RAM_SLOT_TIME': */
#define MSK_USED_BITS_XPIC_RAM_SLOT_TIME 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_RAM_RX_VAR */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_RAM_RX_VAR           0x00000488U

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_RX_LEN_STAT */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_RX_LEN_STAT           0x00000488U

#define MSK_XPIC_RAM_RX_LEN_STAT_VAL 0xffffffffU
#define SRT_XPIC_RAM_RX_LEN_STAT_VAL 0

/* all used bits of 'XPIC_RAM_RX_LEN_STAT': */
#define MSK_USED_BITS_XPIC_RAM_RX_LEN_STAT 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_RX_FRM_STAT */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_RX_FRM_STAT           0x0000048CU

#define MSK_XPIC_RAM_RX_FRM_STAT_VAL 0xffffffffU
#define SRT_XPIC_RAM_RX_FRM_STAT_VAL 0

/* all used bits of 'XPIC_RAM_RX_FRM_STAT': */
#define MSK_USED_BITS_XPIC_RAM_RX_FRM_STAT 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_RX_FRM_LEN */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_RX_FRM_LEN           0x00000490U

#define MSK_XPIC_RAM_RX_FRM_LEN_VAL 0xffffffffU
#define SRT_XPIC_RAM_RX_FRM_LEN_VAL 0

/* all used bits of 'XPIC_RAM_RX_FRM_LEN': */
#define MSK_USED_BITS_XPIC_RAM_RX_FRM_LEN 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_RX_FILL */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_RX_FILL           0x00000494U

#define MSK_XPIC_RAM_RX_FILL_VAL 0xffffffffU
#define SRT_XPIC_RAM_RX_FILL_VAL 0

/* all used bits of 'XPIC_RAM_RX_FILL': */
#define MSK_USED_BITS_XPIC_RAM_RX_FILL 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_RX_OFS_IP_DATA */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_RX_OFS_IP_DATA           0x00000498U

#define MSK_XPIC_RAM_RX_OFS_IP_DATA_VAL 0xffffffffU
#define SRT_XPIC_RAM_RX_OFS_IP_DATA_VAL 0

/* all used bits of 'XPIC_RAM_RX_OFS_IP_DATA': */
#define MSK_USED_BITS_XPIC_RAM_RX_OFS_IP_DATA 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_RX_FRAMEBUF_DATA_LIMIT */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_RX_FRAMEBUF_DATA_LIMIT           0x0000049CU

#define MSK_XPIC_RAM_RX_FRAMEBUF_DATA_LIMIT_VAL 0xffffffffU
#define SRT_XPIC_RAM_RX_FRAMEBUF_DATA_LIMIT_VAL 0

/* all used bits of 'XPIC_RAM_RX_FRAMEBUF_DATA_LIMIT': */
#define MSK_USED_BITS_XPIC_RAM_RX_FRAMEBUF_DATA_LIMIT 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_RAM_RX_BUF */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_RAM_RX_BUF           0x000004A0U

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_RAM_RX_ERR_TAB */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_RAM_RX_ERR_TAB           0x000005A0U

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_RAM_RX_FRWD_FIFO_TAB */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_RAM_RX_FRWD_FIFO_TAB           0x000005B0U

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_RAM_RX_IRQ_MSK_TAB */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_RAM_RX_IRQ_MSK_TAB           0x000005C0U

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_RAM_IRQ_STACK */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_RAM_IRQ_STACK           0x000005D0U

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_IRQ_STACK_PC */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_IRQ_STACK_PC           0x000005D0U

#define MSK_XPIC_RAM_IRQ_STACK_PC_VAL 0xffffffffU
#define SRT_XPIC_RAM_IRQ_STACK_PC_VAL 0

/* all used bits of 'XPIC_RAM_IRQ_STACK_PC': */
#define MSK_USED_BITS_XPIC_RAM_IRQ_STACK_PC 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_IRQ_STACK_STAT */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_IRQ_STACK_STAT           0x000005D4U

#define MSK_XPIC_RAM_IRQ_STACK_STAT_VAL 0xffffffffU
#define SRT_XPIC_RAM_IRQ_STACK_STAT_VAL 0

/* all used bits of 'XPIC_RAM_IRQ_STACK_STAT': */
#define MSK_USED_BITS_XPIC_RAM_IRQ_STACK_STAT 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_IRQ_STACK_R0 */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_IRQ_STACK_R0           0x000005D8U

#define MSK_XPIC_RAM_IRQ_STACK_R0_VAL 0xffffffffU
#define SRT_XPIC_RAM_IRQ_STACK_R0_VAL 0

/* all used bits of 'XPIC_RAM_IRQ_STACK_R0': */
#define MSK_USED_BITS_XPIC_RAM_IRQ_STACK_R0 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_IRQ_STACK_REQ_HOST */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_IRQ_STACK_REQ_HOST           0x000005DCU

#define MSK_XPIC_RAM_IRQ_STACK_REQ_HOST_VAL 0xffffffffU
#define SRT_XPIC_RAM_IRQ_STACK_REQ_HOST_VAL 0

/* all used bits of 'XPIC_RAM_IRQ_STACK_REQ_HOST': */
#define MSK_USED_BITS_XPIC_RAM_IRQ_STACK_REQ_HOST 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_RAM_TX_VAR */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_RAM_TX_VAR           0x000005E0U

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_STATE_PC */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_STATE_PC           0x000005E0U

#define MSK_XPIC_RAM_TX_STATE_PC_VAL 0xffffffffU
#define SRT_XPIC_RAM_TX_STATE_PC_VAL 0

/* all used bits of 'XPIC_RAM_TX_STATE_PC': */
#define MSK_USED_BITS_XPIC_RAM_TX_STATE_PC 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_FINISH_PC */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_FINISH_PC           0x000005E4U

#define MSK_XPIC_RAM_TX_FINISH_PC_VAL 0xffffffffU
#define SRT_XPIC_RAM_TX_FINISH_PC_VAL 0

/* all used bits of 'XPIC_RAM_TX_FINISH_PC': */
#define MSK_USED_BITS_XPIC_RAM_TX_FINISH_PC 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_LENGTH */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_LENGTH           0x000005E8U

#define MSK_XPIC_RAM_TX_LENGTH_VAL 0xffffffffU
#define SRT_XPIC_RAM_TX_LENGTH_VAL 0

/* all used bits of 'XPIC_RAM_TX_LENGTH': */
#define MSK_USED_BITS_XPIC_RAM_TX_LENGTH 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_LENGTH_WRITTEN */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_LENGTH_WRITTEN           0x000005ECU

#define MSK_XPIC_RAM_TX_LENGTH_WRITTEN_VAL 0xffffffffU
#define SRT_XPIC_RAM_TX_LENGTH_WRITTEN_VAL 0

/* all used bits of 'XPIC_RAM_TX_LENGTH_WRITTEN': */
#define MSK_USED_BITS_XPIC_RAM_TX_LENGTH_WRITTEN 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_FRAMEBUF_DATA_LIMIT */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_FRAMEBUF_DATA_LIMIT           0x000005F0U

#define MSK_XPIC_RAM_TX_FRAMEBUF_DATA_LIMIT_VAL 0xffffffffU
#define SRT_XPIC_RAM_TX_FRAMEBUF_DATA_LIMIT_VAL 0

/* all used bits of 'XPIC_RAM_TX_FRAMEBUF_DATA_LIMIT': */
#define MSK_USED_BITS_XPIC_RAM_TX_FRAMEBUF_DATA_LIMIT 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_RETRY */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_RETRY           0x000005F4U

#define MSK_XPIC_RAM_TX_RETRY_VAL 0xffffffffU
#define SRT_XPIC_RAM_TX_RETRY_VAL 0

/* all used bits of 'XPIC_RAM_TX_RETRY': */
#define MSK_USED_BITS_XPIC_RAM_TX_RETRY 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_FOLLOW_FREE */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_FOLLOW_FREE           0x000005F8U

#define MSK_XPIC_RAM_TX_FOLLOW_FREE_VAL 0xffffffffU
#define SRT_XPIC_RAM_TX_FOLLOW_FREE_VAL 0

/* all used bits of 'XPIC_RAM_TX_FOLLOW_FREE': */
#define MSK_USED_BITS_XPIC_RAM_TX_FOLLOW_FREE 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_FOLLOW_BUF */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_FOLLOW_BUF           0x000005FCU

#define MSK_XPIC_RAM_TX_FOLLOW_BUF_VAL 0xffffffffU
#define SRT_XPIC_RAM_TX_FOLLOW_BUF_VAL 0

/* all used bits of 'XPIC_RAM_TX_FOLLOW_BUF': */
#define MSK_USED_BITS_XPIC_RAM_TX_FOLLOW_BUF 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_FOLLOW_DLR */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_FOLLOW_DLR           0x00000600U

#define MSK_XPIC_RAM_TX_FOLLOW_DLR_VAL 0xffffffffU
#define SRT_XPIC_RAM_TX_FOLLOW_DLR_VAL 0

/* all used bits of 'XPIC_RAM_TX_FOLLOW_DLR': */
#define MSK_USED_BITS_XPIC_RAM_TX_FOLLOW_DLR 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_FOLLOW_HI */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_FOLLOW_HI           0x00000604U

#define MSK_XPIC_RAM_TX_FOLLOW_HI_VAL 0xffffffffU
#define SRT_XPIC_RAM_TX_FOLLOW_HI_VAL 0

/* all used bits of 'XPIC_RAM_TX_FOLLOW_HI': */
#define MSK_USED_BITS_XPIC_RAM_TX_FOLLOW_HI 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_FOLLOW_LO */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_FOLLOW_LO           0x00000608U

#define MSK_XPIC_RAM_TX_FOLLOW_LO_VAL 0xffffffffU
#define SRT_XPIC_RAM_TX_FOLLOW_LO_VAL 0

/* all used bits of 'XPIC_RAM_TX_FOLLOW_LO': */
#define MSK_USED_BITS_XPIC_RAM_TX_FOLLOW_LO 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_FOLLOW_FRAME */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_FOLLOW_FRAME           0x0000060CU

#define MSK_XPIC_RAM_TX_FOLLOW_FRAME_VAL 0xffffffffU
#define SRT_XPIC_RAM_TX_FOLLOW_FRAME_VAL 0

/* all used bits of 'XPIC_RAM_TX_FOLLOW_FRAME': */
#define MSK_USED_BITS_XPIC_RAM_TX_FOLLOW_FRAME 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_RAM_TX_SND_TAB */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_RAM_TX_SND_TAB           0x00000610U

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_RAM_RESERVED */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_RAM_RESERVED           0x00000624U

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_RX_STATE */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_RX_STATE           0x00000624U

#define MSK_XPIC_RAM_RX_STATE_DROPPED                  0x00000001U
#define SRT_XPIC_RAM_RX_STATE_DROPPED                  0
#define MSK_XPIC_RAM_RX_STATE_FINISH                   0x00000002U
#define SRT_XPIC_RAM_RX_STATE_FINISH                   1
#define MSK_XPIC_RAM_RX_STATE_PRIO_HI                  0x00000004U
#define SRT_XPIC_RAM_RX_STATE_PRIO_HI                  2
#define MSK_XPIC_RAM_RX_STATE_PRIO_RT                  0x00000008U
#define SRT_XPIC_RAM_RX_STATE_PRIO_RT                  3
#define MSK_XPIC_RAM_RX_STATE_FRWD_FRAME_TO_PORT0      0x00000010U
#define SRT_XPIC_RAM_RX_STATE_FRWD_FRAME_TO_PORT0      4
#define MSK_XPIC_RAM_RX_STATE_FRWD_FRAME_TO_PORT1      0x00000020U
#define SRT_XPIC_RAM_RX_STATE_FRWD_FRAME_TO_PORT1      5
#define MSK_XPIC_RAM_RX_STATE_FRWD_FRAME_TO_PORT2      0x00000040U
#define SRT_XPIC_RAM_RX_STATE_FRWD_FRAME_TO_PORT2      6
#define MSK_XPIC_RAM_RX_STATE_FRWD_FRAME_TO_PORT3      0x00000080U
#define SRT_XPIC_RAM_RX_STATE_FRWD_FRAME_TO_PORT3      7
#define MSK_XPIC_RAM_RX_STATE_VLAN_TAG                 0x00000100U
#define SRT_XPIC_RAM_RX_STATE_VLAN_TAG                 8
#define MSK_XPIC_RAM_RX_STATE_IP_PACKET                0x00000200U
#define SRT_XPIC_RAM_RX_STATE_IP_PACKET                9
#define MSK_XPIC_RAM_RX_STATE_DATA_PARSED              0x00000400U
#define SRT_XPIC_RAM_RX_STATE_DATA_PARSED              10
#define MSK_XPIC_RAM_RX_STATE_FORWARDED                0x00000800U
#define SRT_XPIC_RAM_RX_STATE_FORWARDED                11
#define MSK_XPIC_RAM_RX_STATE_FRWD_FRAME_TO_LOCAL_PORT 0x00001000U
#define SRT_XPIC_RAM_RX_STATE_FRWD_FRAME_TO_LOCAL_PORT 12
#define MSK_XPIC_RAM_RX_STATE_RES1                     0xffffe000U
#define SRT_XPIC_RAM_RX_STATE_RES1                     13

/* all used bits of 'XPIC_RAM_RX_STATE': */
#define MSK_USED_BITS_XPIC_RAM_RX_STATE 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_RAM_TX_STATE */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_RAM_TX_STATE           0x00000628U

#define MSK_XPIC_RAM_TX_STATE_ACTIVE 0x00000001U
#define SRT_XPIC_RAM_TX_STATE_ACTIVE 0
#define MSK_XPIC_RAM_TX_STATE_RES1   0xfffffffeU
#define SRT_XPIC_RAM_TX_STATE_RES1   1

/* all used bits of 'XPIC_RAM_TX_STATE': */
#define MSK_USED_BITS_XPIC_RAM_TX_STATE 0xffffffffU




/*********************************************************************/
/* area structure definitions                                        */
/*********************************************************************/



#endif

#ifndef ETH_XPIC_RAM_struct_h
#define ETH_XPIC_RAM_struct_h

typedef struct XPIC_RAM_CONSTANTPOOL_Ttag
{
  volatile uint32_t aulDataField[256];
} XPIC_RAM_CONSTANTPOOL_T;

typedef struct XPIC_RAM_PARAMETERS_Ttag
{
  volatile uint32_t ulXPIC_RAM_SHM_ADDR;
  volatile uint32_t ulXPIC_RAM_FRMBUF_ADDR;
  volatile uint32_t ulXPIC_RAM_FRAMEBUF_SIZE;
  volatile uint32_t ulXPIC_RAM_MACADR_HASH_TABLE0;
  volatile uint32_t ulXPIC_RAM_MACADR_HASH_TABLE1;
  volatile uint32_t ulXPIC_RAM_MACADR_HASH_TABLE2;
  volatile uint32_t ulXPIC_RAM_MACADR_HASH_TABLE3;
  volatile uint32_t ulXPIC_RAM_RX_IRQ_ADR;
  volatile uint32_t ulXPIC_RAM_RX_IRQ_VAL;
  volatile uint32_t ulXPIC_RAM_TX_IRQ_ADR;
  volatile uint32_t ulXPIC_RAM_TX_IRQ_VAL;
  volatile uint32_t ulXPIC_RAM_LOCAL_MAC_ADDRESS_HI;
  volatile uint32_t ulXPIC_RAM_LOCAL_MAC_ADDRESS_LO;
  volatile uint32_t ulXPIC_RAM_TX_ABORT_MODE;
  volatile uint32_t ulXPIC_RAM_ETH_CONFIG;
  volatile uint32_t ulXPIC_RAM_ETH_TX_CONFIG;
  volatile uint32_t ulXPIC_RAM_MIIMU_REQ_CYCLE;
  volatile uint32_t ulXPIC_RAM_MIIMU_READ_PHYLINK;
  volatile uint32_t ulXPIC_RAM_MIIMU_READ_PHYMODE;
  volatile uint32_t ulXPIC_RAM_MIIMU_READ_PHYSPEED;
  volatile uint32_t ulXPIC_RAM_PHYLINK_MSK;
  volatile uint32_t ulXPIC_RAM_PHYMODE_MSK;
  volatile uint32_t ulXPIC_RAM_PHYSPEED_MSK;
  volatile uint32_t ulXPIC_RAM_MIIMU_LINK_REQ_NXT;
  volatile uint32_t ulXPIC_RAM_MIIMU_MODE_REQ_NXT;
  volatile uint32_t ulXPIC_RAM_MIIMU_SPEED_REQ_NXT;
} XPIC_RAM_PARAMETERS_T;

typedef struct XPIC_RAM_VAR_Ttag
{
  volatile uint32_t ulXPIC_RAM_FRAMEBUF_DATA_SIZE;
  volatile uint32_t ulXPIC_RAM_FRMBUF_ADDR_NEG;
  volatile uint32_t ulXPIC_RAM_MIIMU_PC;
  volatile uint32_t ulXPIC_RAM_MIIMU_REQ_ID;
  volatile uint32_t ulXPIC_RAM_PHYLINK;
  volatile uint32_t ulXPIC_RAM_PHYMODE;
  volatile uint32_t ulXPIC_RAM_PHYSPEED;
  volatile uint32_t ulXPIC_RAM_SLOT_TIME;
} XPIC_RAM_VAR_T;

typedef struct XPIC_RAM_RX_VAR_Ttag
{
  volatile uint32_t ulXPIC_RAM_RX_LEN_STAT;
  volatile uint32_t ulXPIC_RAM_RX_FRM_STAT;
  volatile uint32_t ulXPIC_RAM_RX_FRM_LEN;
  volatile uint32_t ulXPIC_RAM_RX_FILL;
  volatile uint32_t ulXPIC_RAM_RX_OFS_IP_DATA;
  volatile uint32_t ulXPIC_RAM_RX_FRAMEBUF_DATA_LIMIT;
} XPIC_RAM_RX_VAR_T;

typedef struct XPIC_RAM_RX_BUF_Ttag
{
  volatile uint32_t aulDataField[64];
} XPIC_RAM_RX_BUF_T;

typedef struct XPIC_RAM_RX_ERR_TAB_Ttag
{
  volatile uint32_t aulDataField[4];
} XPIC_RAM_RX_ERR_TAB_T;

typedef struct XPIC_RAM_RX_FRWD_FIFO_TAB_Ttag
{
  volatile uint32_t aulDataField[4];
} XPIC_RAM_RX_FRWD_FIFO_TAB_T;

typedef struct XPIC_RAM_RX_IRQ_MSK_TAB_Ttag
{
  volatile uint32_t aulDataField[4];
} XPIC_RAM_RX_IRQ_MSK_TAB_T;

typedef struct XPIC_RAM_IRQ_STACK_Ttag
{
  volatile uint32_t ulXPIC_RAM_IRQ_STACK_PC;
  volatile uint32_t ulXPIC_RAM_IRQ_STACK_STAT;
  volatile uint32_t ulXPIC_RAM_IRQ_STACK_R0;
  volatile uint32_t ulXPIC_RAM_IRQ_STACK_REQ_HOST;
} XPIC_RAM_IRQ_STACK_T;

typedef struct XPIC_RAM_TX_VAR_Ttag
{
  volatile uint32_t ulXPIC_RAM_TX_STATE_PC;
  volatile uint32_t ulXPIC_RAM_TX_FINISH_PC;
  volatile uint32_t ulXPIC_RAM_TX_LENGTH;
  volatile uint32_t ulXPIC_RAM_TX_LENGTH_WRITTEN;
  volatile uint32_t ulXPIC_RAM_TX_FRAMEBUF_DATA_LIMIT;
  volatile uint32_t ulXPIC_RAM_TX_RETRY;
  volatile uint32_t ulXPIC_RAM_TX_FOLLOW_FREE;
  volatile uint32_t ulXPIC_RAM_TX_FOLLOW_BUF;
  volatile uint32_t ulXPIC_RAM_TX_FOLLOW_DLR;
  volatile uint32_t ulXPIC_RAM_TX_FOLLOW_HI;
  volatile uint32_t ulXPIC_RAM_TX_FOLLOW_LO;
  volatile uint32_t ulXPIC_RAM_TX_FOLLOW_FRAME;
} XPIC_RAM_TX_VAR_T;

typedef struct XPIC_RAM_TX_SND_TAB_Ttag
{
  volatile uint32_t aulDataField[5];
} XPIC_RAM_TX_SND_TAB_T;

typedef struct XPIC_RAM_RESERVED_Ttag
{
  volatile uint32_t ulXPIC_RAM_RX_STATE;
  volatile uint32_t ulXPIC_RAM_TX_STATE;
} XPIC_RAM_RESERVED_T;

/* combined 12 structures */
typedef struct ETH_XPIC_RAM_Ttag {
  XPIC_RAM_CONSTANTPOOL_T tXPIC_RAM_CONSTANTPOOL;
  XPIC_RAM_PARAMETERS_T tXPIC_RAM_PARAMETERS;
  XPIC_RAM_VAR_T tXPIC_RAM_VAR;
  XPIC_RAM_RX_VAR_T tXPIC_RAM_RX_VAR;
  XPIC_RAM_RX_BUF_T tXPIC_RAM_RX_BUF;
  XPIC_RAM_RX_ERR_TAB_T tXPIC_RAM_RX_ERR_TAB;
  XPIC_RAM_RX_FRWD_FIFO_TAB_T tXPIC_RAM_RX_FRWD_FIFO_TAB;
  XPIC_RAM_RX_IRQ_MSK_TAB_T tXPIC_RAM_RX_IRQ_MSK_TAB;
  XPIC_RAM_IRQ_STACK_T tXPIC_RAM_IRQ_STACK;
  XPIC_RAM_TX_VAR_T tXPIC_RAM_TX_VAR;
  XPIC_RAM_TX_SND_TAB_T tXPIC_RAM_TX_SND_TAB;
  XPIC_RAM_RESERVED_T tXPIC_RAM_RESERVED;
} ETH_XPIC_RAM_T;


/*! \} *//* End of group ETH_XPIC */

/*! \} *//* End of group Legacy */

/*! \} *//* End of group netX_Driver */

#ifdef __cplusplus
}
#endif

#endif /* ETH_XPIC_RAM_struct_h */


/*!************************************************************************//*!
 * \file     netx_drv_eth_xpic_def.h
 * \brief    Ethernet xpic mac peripheral module driver.
 * $Revision: 4646 $
 * $Date: 2018-12-19 08:47:36 +0100 (Mi, 19 Dez 2018) $
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
#include "netx_drv.h"
#ifndef _DRV_ETH_XPIC_H
#define _DRV_ETH_XPIC_H

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

/* Area of ETH_FRAMEBUF */

/* ===================================================================== */


/* --------------------------------------------------------------------- */
/* Register AREA_ETH_FRAMEBUF_INFO */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_ETH_FRAMEBUF_INFO          0x00000000U

/* --------------------------------------------------------------------- */
/* Register ETH_FRAMEBUF_SIZE */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_ETH_FRAMEBUF_SIZE          0x00000000U

#define MSK_ETH_FRAMEBUF_SIZE_VAL 0xffffffffU
#define SRT_ETH_FRAMEBUF_SIZE_VAL 0

/* all used bits of 'ETH_FRAMEBUF_SIZE': */
#define MSK_USED_BITS_ETH_FRAMEBUF_SIZE 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register ETH_FRAMEBUF_INFO */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_ETH_FRAMEBUF_INFO          0x00000004U

#define MSK_ETH_FRAMEBUF_INFO_BUFFER_NUMBER 0x000000ffU
#define SRT_ETH_FRAMEBUF_INFO_BUFFER_NUMBER 0
#define MSK_ETH_FRAMEBUF_INFO_CON_EN        0x00000100U
#define SRT_ETH_FRAMEBUF_INFO_CON_EN        8
#define MSK_ETH_FRAMEBUF_INFO_MULTIREQ_CNT  0x00000e00U
#define SRT_ETH_FRAMEBUF_INFO_MULTIREQ_CNT  9
#define MSK_ETH_FRAMEBUF_INFO_ERROR_CODE    0x00007000U
#define SRT_ETH_FRAMEBUF_INFO_ERROR_CODE    12
#define MSK_ETH_FRAMEBUF_INFO_VALID         0x00008000U
#define SRT_ETH_FRAMEBUF_INFO_VALID         15
#define MSK_ETH_FRAMEBUF_INFO_RES           0xffff0000U
#define SRT_ETH_FRAMEBUF_INFO_RES           16

/* all used bits of 'ETH_FRAMEBUF_INFO': */
#define MSK_USED_BITS_ETH_FRAMEBUF_INFO 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register ETH_FRAMEBUF_STATE */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_ETH_FRAMEBUF_STATE          0x00000008U

#define MSK_ETH_FRAMEBUF_STATE_VAL 0xffffffffU
#define SRT_ETH_FRAMEBUF_STATE_VAL 0

/* all used bits of 'ETH_FRAMEBUF_STATE': */
#define MSK_USED_BITS_ETH_FRAMEBUF_STATE 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register ETH_FRAMEBUF_ID */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_ETH_FRAMEBUF_ID          0x0000000CU

#define MSK_ETH_FRAMEBUF_ID_VAL 0xffffffffU
#define SRT_ETH_FRAMEBUF_ID_VAL 0

/* all used bits of 'ETH_FRAMEBUF_ID': */
#define MSK_USED_BITS_ETH_FRAMEBUF_ID 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register AREA_ETH_FRAMEBUF_RX_TIMESTAMP */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_ETH_FRAMEBUF_RX_TIMESTAMP          0x00000010U

/* --------------------------------------------------------------------- */
/* Register ETH_FRAMEBUF_TS_RX_NS */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_ETH_FRAMEBUF_TS_RX_NS          0x00000010U

#define MSK_ETH_FRAMEBUF_TS_RX_NS_VAL 0xffffffffU
#define SRT_ETH_FRAMEBUF_TS_RX_NS_VAL 0

/* all used bits of 'ETH_FRAMEBUF_TS_RX_NS': */
#define MSK_USED_BITS_ETH_FRAMEBUF_TS_RX_NS 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register ETH_FRAMEBUF_TS_RX_S */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_ETH_FRAMEBUF_TS_RX_S          0x00000014U

#define MSK_ETH_FRAMEBUF_TS_RX_S_VAL 0xffffffffU
#define SRT_ETH_FRAMEBUF_TS_RX_S_VAL 0

/* all used bits of 'ETH_FRAMEBUF_TS_RX_S': */
#define MSK_USED_BITS_ETH_FRAMEBUF_TS_RX_S 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register AREA_ETH_FRAMEBUF_TX_TIMESTAMP */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_ETH_FRAMEBUF_TX_TIMESTAMP          0x00000018U

/* --------------------------------------------------------------------- */
/* Register ETH_FRAMEBUF_TS_TX_NS */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_ETH_FRAMEBUF_TS_TX_NS          0x00000018U

#define MSK_ETH_FRAMEBUF_TS_TX_NS_VAL 0xffffffffU
#define SRT_ETH_FRAMEBUF_TS_TX_NS_VAL 0

/* all used bits of 'ETH_FRAMEBUF_TS_TX_NS': */
#define MSK_USED_BITS_ETH_FRAMEBUF_TS_TX_NS 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register ETH_FRAMEBUF_TS_TX_S */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_ETH_FRAMEBUF_TS_TX_S          0x0000001CU

#define MSK_ETH_FRAMEBUF_TS_TX_S_VAL 0xffffffffU
#define SRT_ETH_FRAMEBUF_TS_TX_S_VAL 0

/* all used bits of 'ETH_FRAMEBUF_TS_TX_S': */
#define MSK_USED_BITS_ETH_FRAMEBUF_TS_TX_S 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register AREA_ETH_FRAMEBUF_DATA */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_ETH_FRAMEBUF_DATA          0x00000020U

/* --------------------------------------------------------------------- */
/* Register ETH_FRAMEBUF_DATA_START */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_ETH_FRAMEBUF_DATA_START          0x00000020U

#define MSK_ETH_FRAMEBUF_DATA_START_VAL 0xffffffffU
#define SRT_ETH_FRAMEBUF_DATA_START_VAL 0

/* all used bits of 'ETH_FRAMEBUF_DATA_START': */
#define MSK_USED_BITS_ETH_FRAMEBUF_DATA_START 0xffffffffU




/*********************************************************************/
/* area structure definitions                                        */
/*********************************************************************/



#endif

#ifndef ETH_FRAMEBUF_struct_h
#define ETH_FRAMEBUF_struct_h

typedef struct ETH_FRAMEBUF_INFO_Ttag
{
  volatile uint32_t ulETH_FRAMEBUF_SIZE;
  volatile uint32_t ulETH_FRAMEBUF_INFO;
  volatile uint32_t ulETH_FRAMEBUF_STATE;
  volatile uint32_t ulETH_FRAMEBUF_ID;
} ETH_FRAMEBUF_INFO_T;

typedef struct ETH_FRAMEBUF_RX_TIMESTAMP_Ttag
{
  volatile uint32_t ulETH_FRAMEBUF_TS_RX_NS;
  volatile uint32_t ulETH_FRAMEBUF_TS_RX_S;
} ETH_FRAMEBUF_RX_TIMESTAMP_T;

typedef struct ETH_FRAMEBUF_TX_TIMESTAMP_Ttag
{
  volatile uint32_t ulETH_FRAMEBUF_TS_TX_NS;
  volatile uint32_t ulETH_FRAMEBUF_TS_TX_S;
} ETH_FRAMEBUF_TX_TIMESTAMP_T;

typedef struct ETH_FRAMEBUF_DATA_Ttag
{
  volatile uint32_t ulETH_FRAMEBUF_DATA_START;
} ETH_FRAMEBUF_DATA_T;

/* combined 4 structures */
typedef struct ETH_FRAMEBUF_Ttag {
  ETH_FRAMEBUF_INFO_T tETH_FRAMEBUF_INFO;
  ETH_FRAMEBUF_RX_TIMESTAMP_T tETH_FRAMEBUF_RX_TIMESTAMP;
  ETH_FRAMEBUF_TX_TIMESTAMP_T tETH_FRAMEBUF_TX_TIMESTAMP;
  ETH_FRAMEBUF_DATA_T tETH_FRAMEBUF_DATA;
} ETH_FRAMEBUF_T;


/* ===================================================================== */

/* Area of ETH_XPIC_SHM */

/* ===================================================================== */


/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_SHM_CNT */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_SHM_CNT          0x00000000U

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_CNT_RX_OK */
/* => number of successfully received frames */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_CNT_RX_OK          0x00000000U

#define MSK_XPIC_SHM_CNT_RX_OK_VAL 0xffffffffU
#define SRT_XPIC_SHM_CNT_RX_OK_VAL 0

/* all used bits of 'XPIC_SHM_CNT_RX_OK': */
#define MSK_USED_BITS_XPIC_SHM_CNT_RX_OK 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_CNT_RX_JUMBO */
/* => rx_frame_len > 1522 detected */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_CNT_RX_JUMBO          0x00000004U

#define MSK_XPIC_SHM_CNT_RX_JUMBO_VAL 0xffffffffU
#define SRT_XPIC_SHM_CNT_RX_JUMBO_VAL 0

/* all used bits of 'XPIC_SHM_CNT_RX_JUMBO': */
#define MSK_USED_BITS_XPIC_SHM_CNT_RX_JUMBO 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_CNT_RX_CRC_ERR */
/* => frame with wrong CRC and no dribble nibble received */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_CNT_RX_CRC_ERR          0x00000008U

#define MSK_XPIC_SHM_CNT_RX_CRC_ERR_VAL 0xffffffffU
#define SRT_XPIC_SHM_CNT_RX_CRC_ERR_VAL 0

/* all used bits of 'XPIC_SHM_CNT_RX_CRC_ERR': */
#define MSK_USED_BITS_XPIC_SHM_CNT_RX_CRC_ERR 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_CNT_RX_ALIGNMENT_ERR */
/* => frame with wrong CRC and dribble nibble received */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_CNT_RX_ALIGNMENT_ERR          0x0000000CU

#define MSK_XPIC_SHM_CNT_RX_ALIGNMENT_ERR_VAL 0xffffffffU
#define SRT_XPIC_SHM_CNT_RX_ALIGNMENT_ERR_VAL 0

/* all used bits of 'XPIC_SHM_CNT_RX_ALIGNMENT_ERR': */
#define MSK_USED_BITS_XPIC_SHM_CNT_RX_ALIGNMENT_ERR 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_CNT_RX_MII_RXERR */
/* => external rxerr signal was active in rx frame */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_CNT_RX_MII_RXERR          0x00000010U

#define MSK_XPIC_SHM_CNT_RX_MII_RXERR_VAL 0xffffffffU
#define SRT_XPIC_SHM_CNT_RX_MII_RXERR_VAL 0

/* all used bits of 'XPIC_SHM_CNT_RX_MII_RXERR': */
#define MSK_USED_BITS_XPIC_SHM_CNT_RX_MII_RXERR 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_CNT_RX_RUNT */
/* => runt frame received */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_CNT_RX_RUNT          0x00000014U

#define MSK_XPIC_SHM_CNT_RX_RUNT_VAL 0xffffffffU
#define SRT_XPIC_SHM_CNT_RX_RUNT_VAL 0

/* all used bits of 'XPIC_SHM_CNT_RX_RUNT': */
#define MSK_USED_BITS_XPIC_SHM_CNT_RX_RUNT 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_CNT_RX_COLLISION_FRAGMENT */
/* => collision fragment received */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_CNT_RX_COLLISION_FRAGMENT          0x00000018U

#define MSK_XPIC_SHM_CNT_RX_COLLISION_FRAGMENT_VAL 0xffffffffU
#define SRT_XPIC_SHM_CNT_RX_COLLISION_FRAGMENT_VAL 0

/* all used bits of 'XPIC_SHM_CNT_RX_COLLISION_FRAGMENT': */
#define MSK_USED_BITS_XPIC_SHM_CNT_RX_COLLISION_FRAGMENT 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_CNT_RX_DROP */
/* => rx frame dropped due to low resources */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_CNT_RX_DROP          0x0000001CU

#define MSK_XPIC_SHM_CNT_RX_DROP_VAL 0xffffffffU
#define SRT_XPIC_SHM_CNT_RX_DROP_VAL 0

/* all used bits of 'XPIC_SHM_CNT_RX_DROP': */
#define MSK_USED_BITS_XPIC_SHM_CNT_RX_DROP 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_CNT_RX_NEXT_BEFORE_FIN */
/* => frame received before preceding frame was acked (due short IFG or very short frame) */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_CNT_RX_NEXT_BEFORE_FIN          0x00000020U

#define MSK_XPIC_SHM_CNT_RX_NEXT_BEFORE_FIN_VAL 0xffffffffU
#define SRT_XPIC_SHM_CNT_RX_NEXT_BEFORE_FIN_VAL 0

/* all used bits of 'XPIC_SHM_CNT_RX_NEXT_BEFORE_FIN': */
#define MSK_USED_BITS_XPIC_SHM_CNT_RX_NEXT_BEFORE_FIN 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_CNT_TX_OK */
/* => number of successfully transmitted frames */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_CNT_TX_OK          0x00000024U

#define MSK_XPIC_SHM_CNT_TX_OK_VAL 0xffffffffU
#define SRT_XPIC_SHM_CNT_TX_OK_VAL 0

/* all used bits of 'XPIC_SHM_CNT_TX_OK': */
#define MSK_USED_BITS_XPIC_SHM_CNT_TX_OK 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_CNT_TX_SINGLE_COLLISIONS */
/* => number of tx collisions at first try */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_CNT_TX_SINGLE_COLLISIONS          0x00000028U

#define MSK_XPIC_SHM_CNT_TX_SINGLE_COLLISIONS_VAL 0xffffffffU
#define SRT_XPIC_SHM_CNT_TX_SINGLE_COLLISIONS_VAL 0

/* all used bits of 'XPIC_SHM_CNT_TX_SINGLE_COLLISIONS': */
#define MSK_USED_BITS_XPIC_SHM_CNT_TX_SINGLE_COLLISIONS 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_CNT_TX_MULTIPLE_COLLISIONS */
/* => number of tx collisions during retries */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_CNT_TX_MULTIPLE_COLLISIONS          0x0000002CU

#define MSK_XPIC_SHM_CNT_TX_MULTIPLE_COLLISIONS_VAL 0xffffffffU
#define SRT_XPIC_SHM_CNT_TX_MULTIPLE_COLLISIONS_VAL 0

/* all used bits of 'XPIC_SHM_CNT_TX_MULTIPLE_COLLISIONS': */
#define MSK_USED_BITS_XPIC_SHM_CNT_TX_MULTIPLE_COLLISIONS 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_CNT_TX_LATE_COLLISIONS */
/* => number of tx late collisions */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_CNT_TX_LATE_COLLISIONS          0x00000030U

#define MSK_XPIC_SHM_CNT_TX_LATE_COLLISIONS_VAL 0xffffffffU
#define SRT_XPIC_SHM_CNT_TX_LATE_COLLISIONS_VAL 0

/* all used bits of 'XPIC_SHM_CNT_TX_LATE_COLLISIONS': */
#define MSK_USED_BITS_XPIC_SHM_CNT_TX_LATE_COLLISIONS 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_CNT_FATAL */
/* => errors occur when xPIC is too slow */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_CNT_FATAL          0x00000034U

#define MSK_XPIC_SHM_CNT_FATAL_RX_OVERFLOW 0x0000ffffU
#define SRT_XPIC_SHM_CNT_FATAL_RX_OVERFLOW 0
#define MSK_XPIC_SHM_CNT_FATAL_TX_UNDERRUN 0xffff0000U
#define SRT_XPIC_SHM_CNT_FATAL_TX_UNDERRUN 16

/* all used bits of 'XPIC_SHM_CNT_FATAL': */
#define MSK_USED_BITS_XPIC_SHM_CNT_FATAL 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_SHM_IRQ */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_SHM_IRQ          0x00000038U

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_IRQ_MSK */
/* => IRQ enable mask */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_IRQ_MSK          0x00000038U

#define MSK_XPIC_SHM_IRQ_MSK_RX_OK     0x00000001U
#define SRT_XPIC_SHM_IRQ_MSK_RX_OK     0
#define MSK_XPIC_SHM_IRQ_MSK_RX_IND_RT 0x00000002U
#define SRT_XPIC_SHM_IRQ_MSK_RX_IND_RT 1
#define MSK_XPIC_SHM_IRQ_MSK_RX_IND_HI 0x00000004U
#define SRT_XPIC_SHM_IRQ_MSK_RX_IND_HI 2
#define MSK_XPIC_SHM_IRQ_MSK_RX_IND_LO 0x00000008U
#define SRT_XPIC_SHM_IRQ_MSK_RX_IND_LO 3
#define MSK_XPIC_SHM_IRQ_MSK_RX_ERR    0x00000010U
#define SRT_XPIC_SHM_IRQ_MSK_RX_ERR    4
#define MSK_XPIC_SHM_IRQ_MSK_TX_CNF    0x00000020U
#define SRT_XPIC_SHM_IRQ_MSK_TX_CNF    5
#define MSK_XPIC_SHM_IRQ_MSK_FATAL_ERR 0x00000040U
#define SRT_XPIC_SHM_IRQ_MSK_FATAL_ERR 6
#define MSK_XPIC_SHM_IRQ_MSK_RES2      0xffffff80U
#define SRT_XPIC_SHM_IRQ_MSK_RES2      7

/* all used bits of 'XPIC_SHM_IRQ_MSK': */
#define MSK_USED_BITS_XPIC_SHM_IRQ_MSK 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_IRQ_IND */
/* => IRQ indication */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_IRQ_IND          0x0000003CU

#define MSK_XPIC_SHM_IRQ_IND_RX_OK     0x00000001U
#define SRT_XPIC_SHM_IRQ_IND_RX_OK     0
#define MSK_XPIC_SHM_IRQ_IND_RX_IND_RT 0x00000002U
#define SRT_XPIC_SHM_IRQ_IND_RX_IND_RT 1
#define MSK_XPIC_SHM_IRQ_IND_RX_IND_HI 0x00000004U
#define SRT_XPIC_SHM_IRQ_IND_RX_IND_HI 2
#define MSK_XPIC_SHM_IRQ_IND_RX_IND_LO 0x00000008U
#define SRT_XPIC_SHM_IRQ_IND_RX_IND_LO 3
#define MSK_XPIC_SHM_IRQ_IND_RX_ERR    0x00000010U
#define SRT_XPIC_SHM_IRQ_IND_RX_ERR    4
#define MSK_XPIC_SHM_IRQ_IND_TX_CNF    0x00000020U
#define SRT_XPIC_SHM_IRQ_IND_TX_CNF    5
#define MSK_XPIC_SHM_IRQ_IND_FATAL_ERR 0x00000040U
#define SRT_XPIC_SHM_IRQ_IND_FATAL_ERR 6
#define MSK_XPIC_SHM_IRQ_IND_RES2      0xffffff80U
#define SRT_XPIC_SHM_IRQ_IND_RES2      7

/* all used bits of 'XPIC_SHM_IRQ_IND': */
#define MSK_USED_BITS_XPIC_SHM_IRQ_IND 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_IRQ_CNF */
/* => IRQ confirmation */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_IRQ_CNF          0x00000040U

#define MSK_XPIC_SHM_IRQ_CNF_RX_OK     0x00000001U
#define SRT_XPIC_SHM_IRQ_CNF_RX_OK     0
#define MSK_XPIC_SHM_IRQ_CNF_RX_IND_RT 0x00000002U
#define SRT_XPIC_SHM_IRQ_CNF_RX_IND_RT 1
#define MSK_XPIC_SHM_IRQ_CNF_RX_IND_HI 0x00000004U
#define SRT_XPIC_SHM_IRQ_CNF_RX_IND_HI 2
#define MSK_XPIC_SHM_IRQ_CNF_RX_IND_LO 0x00000008U
#define SRT_XPIC_SHM_IRQ_CNF_RX_IND_LO 3
#define MSK_XPIC_SHM_IRQ_CNF_RX_ERR    0x00000010U
#define SRT_XPIC_SHM_IRQ_CNF_RX_ERR    4
#define MSK_XPIC_SHM_IRQ_CNF_TX_CNF    0x00000020U
#define SRT_XPIC_SHM_IRQ_CNF_TX_CNF    5
#define MSK_XPIC_SHM_IRQ_CNF_FATAL_ERR 0x00000040U
#define SRT_XPIC_SHM_IRQ_CNF_FATAL_ERR 6
#define MSK_XPIC_SHM_IRQ_CNF_RES2      0xffffff80U
#define SRT_XPIC_SHM_IRQ_CNF_RES2      7

/* all used bits of 'XPIC_SHM_IRQ_CNF': */
#define MSK_USED_BITS_XPIC_SHM_IRQ_CNF 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_MIIMU_REQ */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_MIIMU_REQ          0x00000044U

#define MSK_XPIC_SHM_MIIMU_REQ_VAL 0xffffffffU
#define SRT_XPIC_SHM_MIIMU_REQ_VAL 0

/* all used bits of 'XPIC_SHM_MIIMU_REQ': */
#define MSK_USED_BITS_XPIC_SHM_MIIMU_REQ 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_SHM_PARAM */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_SHM_PARAM          0x00000048U

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_VLAN_PRIORITIZATION */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_VLAN_PRIORITIZATION          0x00000048U

#define MSK_XPIC_SHM_VLAN_PRIORITIZATION_VAL  0x0000000fU
#define SRT_XPIC_SHM_VLAN_PRIORITIZATION_VAL  0
#define MSK_XPIC_SHM_VLAN_PRIORITIZATION_RES1 0xfffffff0U
#define SRT_XPIC_SHM_VLAN_PRIORITIZATION_RES1 4

/* all used bits of 'XPIC_SHM_VLAN_PRIORITIZATION': */
#define MSK_USED_BITS_XPIC_SHM_VLAN_PRIORITIZATION 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_DSCP_PRIORITIZATION */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_DSCP_PRIORITIZATION          0x0000004CU

#define MSK_XPIC_SHM_DSCP_PRIORITIZATION_VAL  0x0000007fU
#define SRT_XPIC_SHM_DSCP_PRIORITIZATION_VAL  0
#define MSK_XPIC_SHM_DSCP_PRIORITIZATION_RES1 0xffffff80U
#define SRT_XPIC_SHM_DSCP_PRIORITIZATION_RES1 7

/* all used bits of 'XPIC_SHM_DSCP_PRIORITIZATION': */
#define MSK_USED_BITS_XPIC_SHM_DSCP_PRIORITIZATION 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_PROMISC_MODE */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_PROMISC_MODE          0x00000050U

#define MSK_XPIC_SHM_PROMISC_MODE_EN  0x00000001U
#define SRT_XPIC_SHM_PROMISC_MODE_EN  0
#define MSK_XPIC_SHM_PROMISC_MODE_RES 0xfffffffeU
#define SRT_XPIC_SHM_PROMISC_MODE_RES 1

/* all used bits of 'XPIC_SHM_PROMISC_MODE': */
#define MSK_USED_BITS_XPIC_SHM_PROMISC_MODE 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_SHM_STAT */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_SHM_STAT          0x00000054U

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_STAT_RX_LEN_STAT */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_STAT_RX_LEN_STAT          0x00000054U

#define MSK_XPIC_SHM_STAT_RX_LEN_STAT_VAL 0xffffffffU
#define SRT_XPIC_SHM_STAT_RX_LEN_STAT_VAL 0

/* all used bits of 'XPIC_SHM_STAT_RX_LEN_STAT': */
#define MSK_USED_BITS_XPIC_SHM_STAT_RX_LEN_STAT 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_CONNECTION_STATE */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_CONNECTION_STATE          0x00000058U

#define MSK_XPIC_SHM_CONNECTION_STATE_SPEED  0x00000001U
#define SRT_XPIC_SHM_CONNECTION_STATE_SPEED  0
#define MSK_XPIC_SHM_CONNECTION_STATE_DUPLEX 0x00000002U
#define SRT_XPIC_SHM_CONNECTION_STATE_DUPLEX 1
#define MSK_XPIC_SHM_CONNECTION_STATE_LINK   0x00000004U
#define SRT_XPIC_SHM_CONNECTION_STATE_LINK   2
#define MSK_XPIC_SHM_CONNECTION_STATE_RES    0x7ffffff8U
#define SRT_XPIC_SHM_CONNECTION_STATE_RES    3
#define MSK_XPIC_SHM_CONNECTION_STATE_TOGGLE 0x80000000U
#define SRT_XPIC_SHM_CONNECTION_STATE_TOGGLE 31

/* all used bits of 'XPIC_SHM_CONNECTION_STATE': */
#define MSK_USED_BITS_XPIC_SHM_CONNECTION_STATE 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_STAT_MII_REGVAL */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_STAT_MII_REGVAL          0x0000005CU

#define MSK_XPIC_SHM_STAT_MII_REGVAL_MIIMU 0x0000ffffU
#define SRT_XPIC_SHM_STAT_MII_REGVAL_MIIMU 0
#define MSK_XPIC_SHM_STAT_MII_REGVAL_DATA  0xffff0000U
#define SRT_XPIC_SHM_STAT_MII_REGVAL_DATA  16

/* all used bits of 'XPIC_SHM_STAT_MII_REGVAL': */
#define MSK_USED_BITS_XPIC_SHM_STAT_MII_REGVAL 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_SHM_PFIFO */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_SHM_PFIFO          0x00000060U

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_PFIFO_EMPTY_PTR_HOST */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_PFIFO_EMPTY_PTR_HOST          0x00000060U

#define MSK_XPIC_SHM_PFIFO_EMPTY_PTR_HOST_OFS 0x000000ffU
#define SRT_XPIC_SHM_PFIFO_EMPTY_PTR_HOST_OFS 0
#define MSK_XPIC_SHM_PFIFO_EMPTY_PTR_HOST_RES 0xffffff00U
#define SRT_XPIC_SHM_PFIFO_EMPTY_PTR_HOST_RES 8

/* all used bits of 'XPIC_SHM_PFIFO_EMPTY_PTR_HOST': */
#define MSK_USED_BITS_XPIC_SHM_PFIFO_EMPTY_PTR_HOST 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_PFIFO_EMPTY_PTR_MAC */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_PFIFO_EMPTY_PTR_MAC          0x00000064U

#define MSK_XPIC_SHM_PFIFO_EMPTY_PTR_MAC_OFS 0x000000ffU
#define SRT_XPIC_SHM_PFIFO_EMPTY_PTR_MAC_OFS 0
#define MSK_XPIC_SHM_PFIFO_EMPTY_PTR_MAC_RES 0xffffff00U
#define SRT_XPIC_SHM_PFIFO_EMPTY_PTR_MAC_RES 8

/* all used bits of 'XPIC_SHM_PFIFO_EMPTY_PTR_MAC': */
#define MSK_USED_BITS_XPIC_SHM_PFIFO_EMPTY_PTR_MAC 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_PFIFO_REQ_HOST */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_PFIFO_REQ_HOST          0x00000068U

#define MSK_XPIC_SHM_PFIFO_REQ_HOST_OFS_LO 0x000000ffU
#define SRT_XPIC_SHM_PFIFO_REQ_HOST_OFS_LO 0
#define MSK_XPIC_SHM_PFIFO_REQ_HOST_OFS_HI 0x0000ff00U
#define SRT_XPIC_SHM_PFIFO_REQ_HOST_OFS_HI 8
#define MSK_XPIC_SHM_PFIFO_REQ_HOST_RES    0xffff0000U
#define SRT_XPIC_SHM_PFIFO_REQ_HOST_RES    16

/* all used bits of 'XPIC_SHM_PFIFO_REQ_HOST': */
#define MSK_USED_BITS_XPIC_SHM_PFIFO_REQ_HOST 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_PFIFO_REQ_MAC */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_PFIFO_REQ_MAC          0x0000006CU

#define MSK_XPIC_SHM_PFIFO_REQ_MAC_OFS_LO 0x000000ffU
#define SRT_XPIC_SHM_PFIFO_REQ_MAC_OFS_LO 0
#define MSK_XPIC_SHM_PFIFO_REQ_MAC_OFS_HI 0x0000ff00U
#define SRT_XPIC_SHM_PFIFO_REQ_MAC_OFS_HI 8
#define MSK_XPIC_SHM_PFIFO_REQ_MAC_RES    0xffff0000U
#define SRT_XPIC_SHM_PFIFO_REQ_MAC_RES    16

/* all used bits of 'XPIC_SHM_PFIFO_REQ_MAC': */
#define MSK_USED_BITS_XPIC_SHM_PFIFO_REQ_MAC 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_PFIFO_CNF_HOST */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_PFIFO_CNF_HOST          0x00000070U

#define MSK_XPIC_SHM_PFIFO_CNF_HOST_OFS 0x000000ffU
#define SRT_XPIC_SHM_PFIFO_CNF_HOST_OFS 0
#define MSK_XPIC_SHM_PFIFO_CNF_HOST_RES 0xffffff00U
#define SRT_XPIC_SHM_PFIFO_CNF_HOST_RES 8

/* all used bits of 'XPIC_SHM_PFIFO_CNF_HOST': */
#define MSK_USED_BITS_XPIC_SHM_PFIFO_CNF_HOST 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_PFIFO_CNF_MAC */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_PFIFO_CNF_MAC          0x00000074U

#define MSK_XPIC_SHM_PFIFO_CNF_MAC_OFS 0x000000ffU
#define SRT_XPIC_SHM_PFIFO_CNF_MAC_OFS 0
#define MSK_XPIC_SHM_PFIFO_CNF_MAC_RES 0xffffff00U
#define SRT_XPIC_SHM_PFIFO_CNF_MAC_RES 8

/* all used bits of 'XPIC_SHM_PFIFO_CNF_MAC': */
#define MSK_USED_BITS_XPIC_SHM_PFIFO_CNF_MAC 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_PFIFO_IND_HOST */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_PFIFO_IND_HOST          0x00000078U

#define MSK_XPIC_SHM_PFIFO_IND_HOST_OFS_LO 0x000000ffU
#define SRT_XPIC_SHM_PFIFO_IND_HOST_OFS_LO 0
#define MSK_XPIC_SHM_PFIFO_IND_HOST_OFS_HI 0x0000ff00U
#define SRT_XPIC_SHM_PFIFO_IND_HOST_OFS_HI 8
#define MSK_XPIC_SHM_PFIFO_IND_HOST_RES    0xffff0000U
#define SRT_XPIC_SHM_PFIFO_IND_HOST_RES    16

/* all used bits of 'XPIC_SHM_PFIFO_IND_HOST': */
#define MSK_USED_BITS_XPIC_SHM_PFIFO_IND_HOST 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register XPIC_SHM_PFIFO_IND_MAC */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_XPIC_SHM_PFIFO_IND_MAC          0x0000007CU

#define MSK_XPIC_SHM_PFIFO_IND_MAC_OFS_LO 0x000000ffU
#define SRT_XPIC_SHM_PFIFO_IND_MAC_OFS_LO 0
#define MSK_XPIC_SHM_PFIFO_IND_MAC_OFS_HI 0x0000ff00U
#define SRT_XPIC_SHM_PFIFO_IND_MAC_OFS_HI 8
#define MSK_XPIC_SHM_PFIFO_IND_MAC_RES    0xffff0000U
#define SRT_XPIC_SHM_PFIFO_IND_MAC_RES    16

/* all used bits of 'XPIC_SHM_PFIFO_IND_MAC': */
#define MSK_USED_BITS_XPIC_SHM_PFIFO_IND_MAC 0xffffffffU

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_SHM_PFIFO_EMPTY_PTR */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_SHM_PFIFO_EMPTY_PTR          0x00000080U

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_SHM_PFIFO_REQ_LO */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_SHM_PFIFO_REQ_LO          0x000000C0U

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_SHM_PFIFO_REQ_HI */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_SHM_PFIFO_REQ_HI          0x00000100U

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_SHM_PFIFO_CNF */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_SHM_PFIFO_CNF          0x00000140U

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_SHM_PFIFO_IND_LO */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_SHM_PFIFO_IND_LO          0x00000180U

/* --------------------------------------------------------------------- */
/* Register AREA_XPIC_SHM_PFIFO_IND_HI */
/* =>  */
/* => Mode:  */
/* --------------------------------------------------------------------- */

#define REL_Adr_AREA_XPIC_SHM_PFIFO_IND_HI          0x000001C0U




/*********************************************************************/
/* area structure definitions                                        */
/*********************************************************************/


typedef struct ETH_XPIC_SHM_AREA_Ttag
{
  volatile uint32_t  ulXPIC_SHM_CNT_RX_OK;
  volatile uint32_t  ulXPIC_SHM_CNT_RX_JUMBO;
  volatile uint32_t  ulXPIC_SHM_CNT_RX_CRC_ERR;
  volatile uint32_t  ulXPIC_SHM_CNT_RX_ALIGNMENT_ERR;
  volatile uint32_t  ulXPIC_SHM_CNT_RX_MII_RXERR;
  volatile uint32_t  ulXPIC_SHM_CNT_RX_RUNT;
  volatile uint32_t  ulXPIC_SHM_CNT_RX_COLLISION_FRAGMENT;
  volatile uint32_t  ulXPIC_SHM_CNT_RX_DROP;
  volatile uint32_t  ulXPIC_SHM_CNT_RX_NEXT_BEFORE_FIN;
  volatile uint32_t  ulXPIC_SHM_CNT_TX_OK;
  volatile uint32_t  ulXPIC_SHM_CNT_TX_SINGLE_COLLISIONS;
  volatile uint32_t  ulXPIC_SHM_CNT_TX_MULTIPLE_COLLISIONS;
  volatile uint32_t  ulXPIC_SHM_CNT_TX_LATE_COLLISIONS;
  volatile uint32_t  ulXPIC_SHM_CNT_FATAL;
  volatile uint32_t  ulXPIC_SHM_IRQ_MSK;
  volatile uint32_t  ulXPIC_SHM_IRQ_IND;
  volatile uint32_t  ulXPIC_SHM_IRQ_CNF;
  volatile uint32_t  ulXPIC_SHM_MIIMU_REQ;
  volatile uint32_t  ulXPIC_SHM_VLAN_PRIORITIZATION;
  volatile uint32_t  ulXPIC_SHM_DSCP_PRIORITIZATION;
  volatile uint32_t  ulXPIC_SHM_PROMISC_MODE;
  volatile uint32_t  ulXPIC_SHM_STAT_RX_LEN_STAT;
  volatile uint32_t  ulXPIC_SHM_CONNECTION_STATE;
  volatile uint32_t  ulXPIC_SHM_STAT_MII_REGVAL;
  volatile uint32_t  ulXPIC_SHM_PFIFO_EMPTY_PTR_HOST;
  volatile uint32_t  ulXPIC_SHM_PFIFO_EMPTY_PTR_MAC;
  volatile uint32_t  ulXPIC_SHM_PFIFO_REQ_HOST;
  volatile uint32_t  ulXPIC_SHM_PFIFO_REQ_MAC;
  volatile uint32_t  ulXPIC_SHM_PFIFO_CNF_HOST;
  volatile uint32_t  ulXPIC_SHM_PFIFO_CNF_MAC;
  volatile uint32_t  ulXPIC_SHM_PFIFO_IND_HOST;
  volatile uint32_t  ulXPIC_SHM_PFIFO_IND_MAC;
  volatile uint32_t  ulAREA_XPIC_SHM_PFIFO_EMPTY_PTR;
  volatile uint32_t aulReserved0[15];
  volatile uint32_t  ulAREA_XPIC_SHM_PFIFO_REQ_LO;
  volatile uint32_t aulReserved1[15];
  volatile uint32_t  ulAREA_XPIC_SHM_PFIFO_REQ_HI;
  volatile uint32_t aulReserved2[15];
  volatile uint32_t  ulAREA_XPIC_SHM_PFIFO_CNF;
  volatile uint32_t aulReserved3[15];
  volatile uint32_t  ulAREA_XPIC_SHM_PFIFO_IND_LO;
  volatile uint32_t aulReserved4[15];
  volatile uint32_t  ulAREA_XPIC_SHM_PFIFO_IND_HI;
} ETH_XPIC_SHM_AREA_T;


#endif

#ifndef ETH_XPIC_SHM_struct_h
#define ETH_XPIC_SHM_struct_h

typedef struct XPIC_SHM_CNT_Ttag
{
  volatile uint32_t ulXPIC_SHM_CNT_RX_OK;
  volatile uint32_t ulXPIC_SHM_CNT_RX_JUMBO;
  volatile uint32_t ulXPIC_SHM_CNT_RX_CRC_ERR;
  volatile uint32_t ulXPIC_SHM_CNT_RX_ALIGNMENT_ERR;
  volatile uint32_t ulXPIC_SHM_CNT_RX_MII_RXERR;
  volatile uint32_t ulXPIC_SHM_CNT_RX_RUNT;
  volatile uint32_t ulXPIC_SHM_CNT_RX_COLLISION_FRAGMENT;
  volatile uint32_t ulXPIC_SHM_CNT_RX_DROP;
  volatile uint32_t ulXPIC_SHM_CNT_RX_NEXT_BEFORE_FIN;
  volatile uint32_t ulXPIC_SHM_CNT_TX_OK;
  volatile uint32_t ulXPIC_SHM_CNT_TX_SINGLE_COLLISIONS;
  volatile uint32_t ulXPIC_SHM_CNT_TX_MULTIPLE_COLLISIONS;
  volatile uint32_t ulXPIC_SHM_CNT_TX_LATE_COLLISIONS;
  volatile uint32_t ulXPIC_SHM_CNT_FATAL;
} XPIC_SHM_CNT_T;

typedef struct XPIC_SHM_IRQ_Ttag
{
  volatile uint32_t ulXPIC_SHM_IRQ_MSK;
  volatile uint32_t ulXPIC_SHM_IRQ_IND;
  volatile uint32_t ulXPIC_SHM_IRQ_CNF;
  volatile uint32_t ulXPIC_SHM_MIIMU_REQ;
} XPIC_SHM_IRQ_T;

typedef struct XPIC_SHM_PARAM_Ttag
{
  volatile uint32_t ulXPIC_SHM_VLAN_PRIORITIZATION;
  volatile uint32_t ulXPIC_SHM_DSCP_PRIORITIZATION;
  volatile uint32_t ulXPIC_SHM_PROMISC_MODE;
} XPIC_SHM_PARAM_T;

typedef struct XPIC_SHM_STAT_Ttag
{
  volatile uint32_t ulXPIC_SHM_STAT_RX_LEN_STAT;
  volatile uint32_t ulXPIC_SHM_CONNECTION_STATE;
  volatile uint32_t ulXPIC_SHM_STAT_MII_REGVAL;
} XPIC_SHM_STAT_T;

typedef struct XPIC_SHM_PFIFO_Ttag
{
  volatile uint32_t ulXPIC_SHM_PFIFO_EMPTY_PTR_HOST;
  volatile uint32_t ulXPIC_SHM_PFIFO_EMPTY_PTR_MAC;
  volatile uint32_t ulXPIC_SHM_PFIFO_REQ_HOST;
  volatile uint32_t ulXPIC_SHM_PFIFO_REQ_MAC;
  volatile uint32_t ulXPIC_SHM_PFIFO_CNF_HOST;
  volatile uint32_t ulXPIC_SHM_PFIFO_CNF_MAC;
  volatile uint32_t ulXPIC_SHM_PFIFO_IND_HOST;
  volatile uint32_t ulXPIC_SHM_PFIFO_IND_MAC;
} XPIC_SHM_PFIFO_T;

typedef struct XPIC_SHM_PFIFO_EMPTY_PTR_Ttag
{
  volatile uint32_t aulDataField[16];
} XPIC_SHM_PFIFO_EMPTY_PTR_T;

typedef struct XPIC_SHM_PFIFO_REQ_LO_Ttag
{
  volatile uint32_t aulDataField[16];
} XPIC_SHM_PFIFO_REQ_LO_T;

typedef struct XPIC_SHM_PFIFO_REQ_HI_Ttag
{
  volatile uint32_t aulDataField[16];
} XPIC_SHM_PFIFO_REQ_HI_T;

typedef struct XPIC_SHM_PFIFO_CNF_Ttag
{
  volatile uint32_t aulDataField[16];
} XPIC_SHM_PFIFO_CNF_T;

typedef struct XPIC_SHM_PFIFO_IND_LO_Ttag
{
  volatile uint32_t aulDataField[16];
} XPIC_SHM_PFIFO_IND_LO_T;

typedef struct XPIC_SHM_PFIFO_IND_HI_Ttag
{
  volatile uint32_t aulDataField[16];
} XPIC_SHM_PFIFO_IND_HI_T;

/* combined 11 structures */
typedef struct ETH_XPIC_SHM_Ttag {
  XPIC_SHM_CNT_T tXPIC_SHM_CNT;
  XPIC_SHM_IRQ_T tXPIC_SHM_IRQ;
  XPIC_SHM_PARAM_T tXPIC_SHM_PARAM;
  XPIC_SHM_STAT_T tXPIC_SHM_STAT;
  XPIC_SHM_PFIFO_T tXPIC_SHM_PFIFO;
  XPIC_SHM_PFIFO_EMPTY_PTR_T tXPIC_SHM_PFIFO_EMPTY_PTR;
  XPIC_SHM_PFIFO_REQ_LO_T tXPIC_SHM_PFIFO_REQ_LO;
  XPIC_SHM_PFIFO_REQ_HI_T tXPIC_SHM_PFIFO_REQ_HI;
  XPIC_SHM_PFIFO_CNF_T tXPIC_SHM_PFIFO_CNF;
  XPIC_SHM_PFIFO_IND_LO_T tXPIC_SHM_PFIFO_IND_LO;
  XPIC_SHM_PFIFO_IND_HI_T tXPIC_SHM_PFIFO_IND_HI;
} ETH_XPIC_SHM_T;

#define ETH_XPIC_MII_CFG_PREAMBLE_SUPPRESS 0             
#define ETH_XPIC_MII_CFG_PREAMBLE_SEND     1 

#define ETH_XPIC_MII_CFG_MDC_PERIOD_400NS  0
#define ETH_XPIC_MII_CFG_MDC_PERIOD_800NS  1

#define ETH_XPIC_MII_CFG_RTA_1BIT          0
#define ETH_XPIC_MII_CFG_RTA_2BIT          1


#define MSK_ETH_XPIC_MII_CFG_AUTODET_LINK    0x00000001U
#define SRT_ETH_XPIC_MII_CFG_AUTODET_LINK    0
#define MSK_ETH_XPIC_MII_CFG_AUTODET_MODE    0x00000002U
#define SRT_ETH_XPIC_MII_CFG_AUTODET_MODE    1
#define MSK_ETH_XPIC_MII_CFG_DEFAULT_DUPLEX  0x00000004U
#define SRT_ETH_XPIC_MII_CFG_DEFAULT_DUPLEX  2
#define MSK_ETH_XPIC_MII_CFG_REG_ADR_DUPLEX  0x000000f8U
#define SRT_ETH_XPIC_MII_CFG_REG_ADR_DUPLEX  3
#define MSK_ETH_XPIC_MII_CFG_BIT_IDX_DUPLEX  0x00000f00U
#define SRT_ETH_XPIC_MII_CFG_BIT_IDX_DUPLEX  8
#define MSK_ETH_XPIC_MII_CFG_BIT_VAL_DUPLEX  0x00001000U
#define SRT_ETH_XPIC_MII_CFG_BIT_VAL_DUPLEX  12
#define MSK_ETH_XPIC_MII_CFG_DEFAULT_SPEED   0x00002000U
#define SRT_ETH_XPIC_MII_CFG_DEFAULT_SPEED   13
#define MSK_ETH_XPIC_MII_CFG_REG_ADR_SPEED   0x0007c000U
#define SRT_ETH_XPIC_MII_CFG_REG_ADR_SPEED   14
#define MSK_ETH_XPIC_MII_CFG_BIT_IDX_SPEED   0x00780000U
#define SRT_ETH_XPIC_MII_CFG_BIT_IDX_SPEED   19
#define MSK_ETH_XPIC_MII_CFG_BIT_VAL_SPEED   0x00800000U
#define SRT_ETH_XPIC_MII_CFG_BIT_VAL_SPEED   23
#define MSK_ETH_XPIC_MII_CFG_PHY_ADDRESS     0x1f000000U
#define SRT_ETH_XPIC_MII_CFG_PHY_ADDRESS     24
#define MSK_ETH_XPIC_MII_CFG_MDC_PERIOD      0x20000000U
#define SRT_ETH_XPIC_MII_CFG_MDC_PERIOD      29
#define MSK_ETH_XPIC_MII_CFG_PREAMBLE        0x40000000U
#define SRT_ETH_XPIC_MII_CFG_PREAMBLE        30
#define MSK_ETH_XPIC_MII_CFG_RTA             0x80000000U
#define SRT_ETH_XPIC_MII_CFG_RTA             31

enum {
	BFW_ETH_XPIC_MII_CFG_AUTODET_LINK   = 1,  /* [0] */
	BFW_ETH_XPIC_MII_CFG_AUTODET_MODE   = 1,  /* [1] */
	BFW_ETH_XPIC_MII_CFG_DEFAULT_DUPLEX = 1,  /* [2] */
	BFW_ETH_XPIC_MII_CFG_REG_ADR_DUPLEX = 5,  /* [7:3] */
	BFW_ETH_XPIC_MII_CFG_BIT_IDX_DUPLEX = 4,  /* [11:8] */
	BFW_ETH_XPIC_MII_CFG_BIT_VAL_DUPLEX = 1,  /* [12] */
	BFW_ETH_XPIC_MII_CFG_DEFAULT_SPEED  = 1,  /* [13] */
	BFW_ETH_XPIC_MII_CFG_REG_ADR_SPEED  = 5,  /* [18:14] */
	BFW_ETH_XPIC_MII_CFG_BIT_IDX_SPEED  = 4,  /* [22:19] */
	BFW_ETH_XPIC_MII_CFG_BIT_VAL_SPEED  = 1,  /* [23] */
	BFW_ETH_XPIC_MII_CFG_PHY_ADDRESS    = 5,  /* [28:24] */
	BFW_ETH_XPIC_MII_CFG_MDC_PERIOD     = 1,  /* [29] */
	BFW_ETH_XPIC_MII_CFG_PREAMBLE       = 1,  /* [30] */
	BFW_ETH_XPIC_MII_CFG_RTA            = 1   /* [31] */
};

typedef struct ETH_XPIC_MII_CFG_BIT_Ttag {
	unsigned int fAutoDetectLink    : BFW_ETH_XPIC_MII_CFG_AUTODET_LINK;   /* Auto detection of LINK via MDIO (Register 1, Bit 2)  0: Off, 1: On */
	unsigned int fAutoDetectMode    : BFW_ETH_XPIC_MII_CFG_AUTODET_MODE;   /* Auto detection of Speed/DUPLEX mode via MDIO  0: Off, 1: On        */
	unsigned int uDefaultDuplex     : BFW_ETH_XPIC_MII_CFG_DEFAULT_DUPLEX; /* Initial DUPLEX mode selector  0: Half DUPLEX, 1: Full DUPLEX       */
	unsigned int uRegAdrDuplex      : BFW_ETH_XPIC_MII_CFG_REG_ADR_DUPLEX; /* MDIO register address for auto detection of DUPLEX mode            */
	unsigned int uBitIdxDuplex      : BFW_ETH_XPIC_MII_CFG_BIT_IDX_DUPLEX; /* MDIO bit index for auto detection of DUPLEX mode                   */
	unsigned int uBitValHalfDuplex  : BFW_ETH_XPIC_MII_CFG_BIT_VAL_DUPLEX; /* MDIO bit value indicating HALF DUPLEX operating mode               */
	unsigned int uDefaultSpeed      : BFW_ETH_XPIC_MII_CFG_DEFAULT_SPEED;  /* Initial speed selector  0: 10Mbps, 1: 100Mbps                      */
	unsigned int uRegAdrSpeed       : BFW_ETH_XPIC_MII_CFG_REG_ADR_SPEED;  /* MDIO register address for auto detection of speed                  */
	unsigned int uBitIdxSpeed       : BFW_ETH_XPIC_MII_CFG_BIT_IDX_SPEED;  /* MDIO bit index for auto detection of speed                         */
	unsigned int uBitVal10Mbps      : BFW_ETH_XPIC_MII_CFG_BIT_VAL_SPEED;  /* MDIO bit value indicating 10Mbps operating mode                    */
	unsigned int uPhyAddress        : BFW_ETH_XPIC_MII_CFG_PHY_ADDRESS;    /* MDIO PHY address for auto detection of DUPLEX mode and speed       */
	unsigned int uMdcPeriod         : BFW_ETH_XPIC_MII_CFG_MDC_PERIOD;     /* MDC period: 0: 220ns; 1: 400ns                                     */
	unsigned int uPreamble          : BFW_ETH_XPIC_MII_CFG_PREAMBLE;       /* MDIO PREAMBLE: 0: suppress; 1: send PREAMBLE                        */
	unsigned int uRta               : BFW_ETH_XPIC_MII_CFG_RTA;            /* MDIO Read Turn Around bits: 0: 1bit; 1: 2bits                      */
} ETH_XPIC_MII_CFG_BIT_T;

typedef union {
	uint32_t               val;
	ETH_XPIC_MII_CFG_BIT_T bf;
} ETH_XPIC_MII_CFG_T;

#define VAL_ETH_XPIC_MII_CFG_FIX_100M_FD (MSK_ETH_XPIC_MII_CFG_DEFAULT_DUPLEX | MSK_ETH_XPIC_MII_CFG_DEFAULT_SPEED)

#define VAL_ETH_XPIC_MII_CFG_INTEL_LXT971A(phyaddr) (\
   (1                                    << SRT_ETH_XPIC_MII_CFG_AUTODET_LINK)\
  |(1                                    << SRT_ETH_XPIC_MII_CFG_AUTODET_MODE)\
  |(0                                    << SRT_ETH_XPIC_MII_CFG_DEFAULT_DUPLEX)\
  |(0x11                                 << SRT_ETH_XPIC_MII_CFG_REG_ADR_DUPLEX)\
  |(0x09                                 << SRT_ETH_XPIC_MII_CFG_BIT_IDX_DUPLEX)\
  |(0                                    << SRT_ETH_XPIC_MII_CFG_BIT_VAL_DUPLEX)\
  |(0                                    << SRT_ETH_XPIC_MII_CFG_DEFAULT_SPEED)\
  |(0x11                                 << SRT_ETH_XPIC_MII_CFG_REG_ADR_SPEED)\
  |(0x0e                                 << SRT_ETH_XPIC_MII_CFG_BIT_IDX_SPEED)\
  |(0                                    << SRT_ETH_XPIC_MII_CFG_BIT_VAL_SPEED)\
  |(phyaddr                              << SRT_ETH_XPIC_MII_CFG_PHY_ADDRESS)\
  |(ETH_XPIC_MII_CFG_MDC_PERIOD_400NS    << SRT_ETH_XPIC_MII_CFG_MDC_PERIOD)\
  |(ETH_XPIC_MII_CFG_PREAMBLE_SEND       << SRT_ETH_XPIC_MII_CFG_PREAMBLE)\
  |(ETH_XPIC_MII_CFG_RTA_2BIT            << SRT_ETH_XPIC_MII_CFG_RTA))
  

#define VAL_ETH_XPIC_MII_CFG_RENESAS_UPD606(phyaddr) (\
   (1                                    << SRT_ETH_XPIC_MII_CFG_AUTODET_LINK)\
  |(1                                    << SRT_ETH_XPIC_MII_CFG_AUTODET_MODE)\
  |(1                                    << SRT_ETH_XPIC_MII_CFG_DEFAULT_DUPLEX)\
  |(0x1f                                 << SRT_ETH_XPIC_MII_CFG_REG_ADR_DUPLEX)\
  |(0x04                                 << SRT_ETH_XPIC_MII_CFG_BIT_IDX_DUPLEX)\
  |(0                                    << SRT_ETH_XPIC_MII_CFG_BIT_VAL_DUPLEX)\
  |(0                                    << SRT_ETH_XPIC_MII_CFG_DEFAULT_SPEED)\
  |(0x1f                                 << SRT_ETH_XPIC_MII_CFG_REG_ADR_SPEED)\
  |(0x02                                 << SRT_ETH_XPIC_MII_CFG_BIT_IDX_SPEED)\
  |(1                                    << SRT_ETH_XPIC_MII_CFG_BIT_VAL_SPEED)\
  |(phyaddr                              << SRT_ETH_XPIC_MII_CFG_PHY_ADDRESS)\
  |(ETH_XPIC_MII_CFG_MDC_PERIOD_400NS    << SRT_ETH_XPIC_MII_CFG_MDC_PERIOD)\
  |(ETH_XPIC_MII_CFG_PREAMBLE_SEND       << SRT_ETH_XPIC_MII_CFG_PREAMBLE)\
  |(ETH_XPIC_MII_CFG_RTA_2BIT            << SRT_ETH_XPIC_MII_CFG_RTA))


#define VAL_ETH_XPIC_MII_CFG_BROADCOM_BCM5221(phyaddr) (\
   (1                                    << SRT_ETH_XPIC_MII_CFG_AUTODET_LINK)\
  |(1                                    << SRT_ETH_XPIC_MII_CFG_AUTODET_MODE)\
  |(0                                    << SRT_ETH_XPIC_MII_CFG_DEFAULT_DUPLEX)\
  |(0x1c                                 << SRT_ETH_XPIC_MII_CFG_REG_ADR_DUPLEX)\
  |(0x00                                 << SRT_ETH_XPIC_MII_CFG_BIT_IDX_DUPLEX)\
  |(0                                    << SRT_ETH_XPIC_MII_CFG_BIT_VAL_DUPLEX)\
  |(0                                    << SRT_ETH_XPIC_MII_CFG_DEFAULT_SPEED)\
  |(0x1c                                 << SRT_ETH_XPIC_MII_CFG_REG_ADR_SPEED)\
  |(0x01                                 << SRT_ETH_XPIC_MII_CFG_BIT_IDX_SPEED)\
  |(1                                    << SRT_ETH_XPIC_MII_CFG_BIT_VAL_SPEED)\
  |(phyaddr                              << SRT_ETH_XPIC_MII_CFG_PHY_ADDRESS)\
  |(ETH_XPIC_MII_CFG_MDC_PERIOD_400NS    << SRT_ETH_XPIC_MII_CFG_MDC_PERIOD)\
  |(ETH_XPIC_MII_CFG_PREAMBLE_SEND       << SRT_ETH_XPIC_MII_CFG_PREAMBLE)\
  |(ETH_XPIC_MII_CFG_RTA_2BIT            << SRT_ETH_XPIC_MII_CFG_RTA))

/* Traffic classes */
#define MAX_ETH_XPIC_TC_VLAN_PRIO     8
#define DFLT_ETH_XPIC_TC_VLAN_PRIO    7
#define MAX_ETH_XPIC_TC_DSCP_PRIO    64
#define DFLT_ETH_XPIC_TC_DSCP_PRIO   42

/* TX confirmation error codes */
typedef uint8_t ETH_XPIC_CNF_ERR;
#define ETH_XPIC_CNF_ERROR_CODE_TX_SUCCESSFUL_WITHOUT_RETRIES  0x00U
#define ETH_XPIC_CNF_ERROR_CODE_TX_SUCCESSFUL_WITH_RETRIES     0x01U
#define ETH_XPIC_CNF_ERROR_CODE_TX_FAILED_LATE_COLLISION       0x02U
#define ETH_XPIC_CNF_ERROR_CODE_TX_FAILED_LINK_DOWN_DURING_TX  0x03U
#define ETH_XPIC_CNF_ERROR_CODE_TX_FAILED_EXCESSIVE_COLLISION  0x04U
#define ETH_XPIC_CNF_ERROR_CODE_TX_FAILED_UTX_UFL_DURING_TX    0x05U
#define ETH_XPIC_CNF_ERROR_CODE_TX_FAILED_FRAME_ABORTED        0x06U
#define ETH_XPIC_CNF_ERROR_CODE_TX_BLOCKED                     0x07U
#define ETH_XPIC_CNF_ERROR_CODE_INVALID                        0x08U

/* Number of bytes in a double tagged Ethernet frame, 4-byte-aligned */
#define ETH_XPIC_FRAME_SIZE_MAX             (1528)

/* Size in bytes of one communication block */
#ifndef ETH_XPIC_FRAME_BUFFER_SIZE
#define ETH_XPIC_FRAME_BUFFER_SIZE          (sizeof(ETH_FRAMEBUF_T) + ETH_XPIC_FRAME_SIZE_MAX)
#endif


/*! \} *//* End of group ETH_XPIC */

/*! \} *//* End of group Legacy */

/*! \} *//* End of group netX_Driver */

#ifdef __cplusplus
}
#endif
#endif /* _DRV_ETH_XPIC_H */

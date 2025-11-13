/**************************************************************************//**
 * \file     netx_drv_eth_mac_xpic.c
 * \brief    ETH MAC controller module driver.
 *
 *
 * $Revision: 6592 $
 * $Date: 2019-12-17 14:14:57 +0100 (Di, 17 Dez 2019) $
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
#include "netx_drv_user_conf.h"
#ifdef DRV_ETH_XPIC_MODULE_ENABLED

/*! \defgroup ETH_XPIC ETH_XPIC
 * \{
 * \brief ETH_XPIC module driver
 *
 * \details This driver is one of our legacy HALs. The support for the legacy hall's is
 * very limited. They are developed and maintained by our chip development and
 * verification department and provide only a hardware near abstraction layer. They
 * are in most cases not convenient or do not support as much functionality as that
 */
#ifdef DRV_DEVICE_NETX_90_MPW
#error "The MPW chip is no longer supported!"
#else
#ifdef DRV_DEVICE_NETX_90
#include "regdef/netx90_app/regdef_netx90_arm_app.h"
#else
#error "No chip Selected!"
#endif
#endif
#include "netx_drv_eth_xpic.h"

#include <string.h>

#define HAL_ETH_STD_MAC_XPIC_TARGET_NX90_APP //edited
/****************************************************************************************
 *  custom/other stuff begin
 ****************************************************************************************/

#ifndef _HW_CONCAT
#define _HW_CONCAT(a,b) a ## b
#define HW_MSK(bf)         _HW_CONCAT(MSK_NX90_, bf)
#define HW_SRT(bf)         _HW_CONCAT(SRT_NX90_, bf)
#define HW_DFLT_BF_VAL(bf) _HW_CONCAT(DFLT_BF_VAL_NX90_, bf)
#define HW_DFLT_VAL(reg)   _HW_CONCAT(DFLT_VAL_NX90_, reg)
#define HW_REGADR(reg)     _HW_CONCAT(Adr_NX90_, reg)
#define HW_AREAADR(area)   _HW_CONCAT(Addr_NX90_, area)
#define HW_TYPE(area)      _HW_CONCAT(NX90_, area)
#endif

#define NX_WRITE32(var,val) (var) = (val)
#define NX_READ32(var)      (var)
#define NX_WRITE16(var,val) (var) = (val)
#define NX_READ16(var)      (var)
#define NX_WRITE8(var,val)  (var) = (val)
#define NX_READ8(var)       (var)
/* Copy data from netX to Host */
#define NX_READMEM(dst, src, len) memcpy(dst, src, len)
/* Copy data from Host to netX */
#define NX_WRITEMEM(dst, src, len) memcpy(dst, src, len)
/****************************************************************************************
 *  custom/other stuff end
 ****************************************************************************************/

extern const uint32_t BuildTime_eth_xpic_prg1[7];
extern const uint32_t XpicCode_eth_xpic_prg1[1797];

#define XcCode_eth_xpic_prg1 XpicCode_eth_xpic_prg1

#define PrgSiz_eth_xpic_prg1        XpicCode_eth_xpic_prg1[0]
#define TrlSiz_eth_xpic_prg1        XpicCode_eth_xpic_prg1[1]

#define PrgSrt_eth_xpic_prg1        &XpicCode_eth_xpic_prg1[2]
#define PrgStp_eth_xpic_prg1        &XpicCode_eth_xpic_prg1[3]

#define TrlSrt_eth_xpic_prg1        &XpicCode_eth_xpic_prg1[3]
#define TrlStp_eth_xpic_prg1        &XpicCode_eth_xpic_prg1[1797]

#define ETH_XPIC_INST 0
#define ETH_XPIC_PRG XpicCode_eth_xpic_prg1
#define ETH_XPIC_ARMIRQ_ADR Adr_NX90_mcp_xpic_app_hs_irq_set_raw
#define ETH_XPIC_ARMIRQ_VAL 0x00100000
#define ETH_XPIC_SHM_BASE   Addr_NX90_intram7
#define ETH_XPIC_DFLT_TX_OUTPUT_PHASE 3 /* PosEdge + 5cc */

/*__USE_XPIC_LOADER_FUNC_LOAD
 __USE_XPIC_LOADER_FUNC_RESET
 __USE_XPIC_LOADER_FUNC_START
 __USE_XPIC_DRAM
 __USE_XPIC_INTRAM*/
//HW_PTR_XPIC(s_aptXpic)
//static NX90_XPIC_AREA_T* const s_aptXpic[] = { (NX90_XPIC_AREA_T*) Addr_NX90_xpic_app_regs };
//HW_PTR_XPIC_DEBUG(s_aptXpicDebug)
//static NX90_XPIC_DEBUG_AREA_T* const s_aptXpicDebug[] = { (NX90_XPIC_DEBUG_AREA_T*) Addr_NX90_xpic_app_debug };
//HW_PTR_XPIC_DRAM(s_apulXpicDram)
static uint32_t* const s_apulXpicDram[] = { (uint32_t*) Addr_NX90_xpic_app_dram };
//HW_PTR_XPIC_PRAM(s_apulXpicPram)
//static uint32_t* const s_apulXpicPram[] = { (uint32_t*) Addr_NX90_xpic_app_pram };

/*****************************************************************************/
/* Definitions                                                               */
/*****************************************************************************/

/* Byte offset of the start of the shared memory to the INTRAM base address
 * This definition is useful to reduce the memory used by the MAC.
 * The default value of (0) will use the whole 32 kByte and yields 20 frame buffers
 * Example: "#define OFS_ETH_XPIC_SHM_AREA 0x4000" reduces the memory usage to 16 kByte and 10 frame buffers,
 * while the first 16 kByte of INTRAM can be used by the application.
 */
#ifndef OFS_ETH_XPIC_SHM_AREA
#define OFS_ETH_XPIC_SHM_AREA               0
#endif

/* Base address of shared memory used by xPIC (netX address range) */
#ifndef ADR_ETH_XPIC_SHM_AREA
#define ADR_ETH_XPIC_SHM_AREA               (ETH_XPIC_SHM_BASE + OFS_ETH_XPIC_SHM_AREA)
#endif

/* Base address of first communication block used by xPIC (netX address range) */
#ifndef ADR_ETH_XPIC_FRAME_BUFFER_AREA
#define ADR_ETH_XPIC_FRAME_BUFFER_AREA      (ADR_ETH_XPIC_SHM_AREA + sizeof(ETH_XPIC_SHM_T))
#endif

/* Base address of shared memory used by host (host address range) */
#ifndef HOSTPTR_ETH_XPIC_SHM_AREA
#define HOSTPTR_ETH_XPIC_SHM_AREA           (ETH_XPIC_SHM_BASE + OFS_ETH_XPIC_SHM_AREA)
#endif

/* Base address of first communication block used by host (host address range) */
#ifndef HOSTPTR_ETH_XPIC_FRAME_BUFFER_AREA
#define HOSTPTR_ETH_XPIC_FRAME_BUFFER_AREA  (HOSTPTR_ETH_XPIC_SHM_AREA + sizeof(ETH_XPIC_SHM_T))
#endif

/* Size in bytes reserved for communication blocks */
#ifndef ETH_XPIC_FRAME_BUFFER_AREA_SIZE
#define ETH_XPIC_FRAME_BUFFER_AREA_SIZE     (0x8000 - sizeof(ETH_XPIC_SHM_T) - OFS_ETH_XPIC_SHM_AREA)
#endif

/* Number of communication blocks available */
#ifndef ETH_XPIC_FRAMEBUF_CNT
#define ETH_XPIC_FRAMEBUF_CNT               (ETH_XPIC_FRAME_BUFFER_AREA_SIZE / ETH_XPIC_FRAME_BUFFER_SIZE)
#endif

/* MII output phase configuration */
#ifndef ETH_XPIC_TX_OUTPUT_PHASE
#define ETH_XPIC_TX_OUTPUT_PHASE ETH_XPIC_DFLT_TX_OUTPUT_PHASE
#endif

#define XC_INTRAM_START_ADDRESS 0

/* Link is always indicated in PHY register 1, bit 2 */
#define ETH_XPIC_MIIM_LINK_STATUS_REG  1
#define ETH_XPIC_MIIM_LINK_STATUS_BIT  2

#define ETH_XPIC_MIIM_REQ_LINK         0
#define ETH_XPIC_MIIM_REQ_MODE         1
#define ETH_XPIC_MIIM_REQ_SPEED        2

#define ETH_XPIC_ETH_CFG_PHYMODE (HW_MSK(eth_config_rx_systime_sfd) | HW_MSK(eth_config_phy_mode))
#define ETH_XPIC_ETH_CFG_MACMODE (HW_MSK(eth_config_rx_systime_sfd) | HW_MSK(eth_config_rx_delay_inputs))
#define ETH_XPIC_ETH_TX_CFG_FD   (1  << HW_SRT(eth_tx_config_tx_systime_sfd))\
  |(1  << HW_SRT(eth_tx_config_tx_watermark_start))\
  |(16 << HW_SRT(eth_tx_config_tx_preamble_len))\
  |(24 << HW_SRT(eth_tx_config_tx_min_ifg_cycles))\
  |(16 << HW_SRT(eth_tx_config_tx_crs_low_cycles))\
  |(ETH_XPIC_TX_OUTPUT_PHASE << HW_SRT(eth_tx_config_tx_output_phase))
#define ETH_XPIC_ETH_TX_CFG_HD (ETH_XPIC_ETH_TX_CFG_FD | HW_MSK(eth_tx_config_half_duplex))

#define ETH_XPIC_PFIFO_DEPTH  (sizeof(XPIC_SHM_PFIFO_EMPTY_PTR_T))
#define ETH_XPIC_PFIFO_BORDER (ETH_XPIC_PFIFO_DEPTH - 1U)

#define HOSTPTR_ETH_XPIC_FRAME_BUFFER(elem)  (HOSTPTR_ETH_XPIC_FRAME_BUFFER_AREA + elem * ETH_XPIC_FRAME_BUFFER_SIZE)

#ifdef DRV_DEVICE_NETX_90_MPW
#define MIIMU_SRT(bf) HW_SRT(_HW_CONCAT(miimu_, bf))
#define MIIMU_MSK(bf) HW_MSK(_HW_CONCAT(miimu_, bf))
#else
#ifdef DRV_DEVICE_NETX_90
#define MIIMU_SRT(bf) HW_SRT(_HW_CONCAT(eth_miimu_, bf))
#define MIIMU_MSK(bf) HW_MSK(_HW_CONCAT(eth_miimu_, bf))
#else
#error "No chip Selected!"
#endif
#endif

typedef enum
{
  DRV_ETH_XPIC_STATE_UNDEF = 0, DRV_ETH_XPIC_STATE_RESET, DRV_ETH_XPIC_STATE_INITIALIZED, DRV_ETH_XPIC_STATE_RUNNING
} DRV_ETH_XPIC_STATE;

/*****************************************************************************/
/* Variables                                                                 */
/*****************************************************************************/

static volatile DRV_ETH_XPIC_STATE s_eState = DRV_ETH_XPIC_STATE_UNDEF;
static uint8_t s_abMacAdr[6] = { 0, 1, 2, 3, 4, 5 };
static ETH_XPIC_SHM_AREA_T* s_ptXpicShm = NULL;

#ifdef DEBUG
ETH_FRAMEBUF_T* g_aptFrames[ETH_XPIC_FRAMEBUF_CNT];
#endif

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/

/*****************************************************************************/
/*! Reset HAL
 * \description
 *   Resets the HAL and stops the xPIC
 * \class
 *   DRV_ETH_XPIC
 * \params
 *   pvUser    [in]  User defined parameter passed into system calls
 * \return
 *   DRV_ETH_XPIC_OK                                                 */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_Reset(void* pvUser)
{
  /* Reset xPIC */
  DRV_XPIC_Reset(ETH_XPIC_INST, pvUser);

  s_eState = DRV_ETH_XPIC_STATE_RESET;

  return DRV_ETH_XPIC_OK;
}

/*****************************************************************************/
/*! Initialize HAL
 * \description
 *   Initializes the HAL according to the setting given in ptConfig. This function
 *   must be called before any other except DRV_ETH_Xpic_Reset().
 * \class
 *   Control Service Class
 * \params
 *   ptConfig  [in]  Runtime parameters or NULL for default settings
 *   pvUser    [in]  User defined parameter passed into system calls
 * \return
 *   DRV_ETH_XPIC_OK                                                 */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_Initialize(DRV_ETH_XPIC_CONFIG_T const* ptConfig, void* pvUser)
{
  ETH_XPIC_RAM_T* ptPrgData;
  uint32_t ulMacAdrHi, ulMacAdrLo;
  uint32_t ulDuplex, ulSpeed, ulMiimu;
  uint8_t bFifoElement;
  volatile uint8_t* pbFifo;
  ETH_FRAMEBUF_T* ptFrame;

#ifndef __DRV_ETH_XPIC_DISABLE_CHECKS__
  if(ptConfig == NULL)
    return DRV_ETH_XPIC_INVALID_ARG;
#endif

  s_ptXpicShm = (ETH_XPIC_SHM_AREA_T*) HOSTPTR_ETH_XPIC_SHM_AREA;

  /* Load microcode*/
  DRV_XPIC_Load(ETH_XPIC_INST, ETH_XPIC_PRG, pvUser);

  /* Set MAC address */
  ulMacAdrHi = ((uint32_t) s_abMacAdr[5] << 8) | ((uint32_t) s_abMacAdr[4]);
  ulMacAdrLo = ((uint32_t) s_abMacAdr[3] << 24) | ((uint32_t) s_abMacAdr[2] << 16) | ((uint32_t) s_abMacAdr[1] << 8) | ((uint32_t) s_abMacAdr[0]);
  if((ulMacAdrHi | ulMacAdrLo) == 0)
  {
    ulMacAdrHi = 0xffffffff;
  }

  /* Set Parameters in xPIC DRAM */
  ptPrgData = (ETH_XPIC_RAM_T*) s_apulXpicDram[ETH_XPIC_INST];

  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_SHM_ADDR, ADR_ETH_XPIC_SHM_AREA);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_FRMBUF_ADDR, ADR_ETH_XPIC_FRAME_BUFFER_AREA);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_FRAMEBUF_SIZE, ETH_XPIC_FRAME_BUFFER_SIZE);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_MACADR_HASH_TABLE0, XC_INTRAM_START_ADDRESS);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_MACADR_HASH_TABLE1, XC_INTRAM_START_ADDRESS + 512);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_RX_IRQ_ADR, ETH_XPIC_ARMIRQ_ADR);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_RX_IRQ_VAL, ETH_XPIC_ARMIRQ_VAL);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_TX_IRQ_ADR, ETH_XPIC_ARMIRQ_ADR);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_TX_IRQ_VAL, ETH_XPIC_ARMIRQ_VAL);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_LOCAL_MAC_ADDRESS_HI, ulMacAdrHi);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_LOCAL_MAC_ADDRESS_LO, ulMacAdrLo);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_TX_ABORT_MODE, 1UL << HW_SRT(eth_tx_config_tx_abort_frame));
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_ETH_CONFIG, ptConfig->fPhyMode ? ETH_XPIC_ETH_CFG_PHYMODE : ETH_XPIC_ETH_CFG_MACMODE);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_ETH_TX_CONFIG,
    (ptConfig->fPhyMode || ptConfig->tMiiCfg.bf.uDefaultDuplex) ? ETH_XPIC_ETH_TX_CFG_FD : ETH_XPIC_ETH_TX_CFG_HD);
#ifdef ETH_XPIC_MIIMU_REQ_CYCLE
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_MIIMU_REQ_CYCLE, ETH_XPIC_MIIMU_REQ_CYCLE);
#endif

  /* AutoDetection of PHY link status */
  ulMiimu = 0;
  if(!ptConfig->fPhyMode)
  {
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.fAutoDetectLink << MIIMU_SRT(snrdy);
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.uPreamble << MIIMU_SRT(preamble);
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.uMdcPeriod << MIIMU_SRT(mdc_period);
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.uRta << MIIMU_SRT(rta);
    ulMiimu |= (uint32_t) ETH_XPIC_MIIM_LINK_STATUS_REG << MIIMU_SRT(regaddr); /* Link is always in register 1, bit 2 */
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.uPhyAddress << MIIMU_SRT(phyaddr);
  }
  NX_WRITE32(ptPrgData->tXPIC_RAM_VAR.ulXPIC_RAM_PHYLINK, ptConfig->fPhyMode ? 1UL << (ETH_XPIC_MIIM_LINK_STATUS_BIT + MIIMU_SRT(data)) : 0);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_MIIMU_READ_PHYLINK, ulMiimu);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_PHYLINK_MSK, 1UL << (ETH_XPIC_MIIM_LINK_STATUS_BIT + MIIMU_SRT(data)));
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_MIIMU_LINK_REQ_NXT,
    ptConfig->tMiiCfg.bf.fAutoDetectMode ? ETH_XPIC_MIIM_REQ_MODE : ETH_XPIC_MIIM_REQ_LINK);

  /* AutoDetection of PHY DUPLEX mode */
  ulMiimu = 0;
  if(!ptConfig->fPhyMode)
  {
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.fAutoDetectMode << MIIMU_SRT(snrdy);
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.uPreamble << MIIMU_SRT(preamble);
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.uMdcPeriod << MIIMU_SRT(mdc_period);
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.uRta << MIIMU_SRT(rta);
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.uRegAdrDuplex << MIIMU_SRT(regaddr);
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.uPhyAddress << MIIMU_SRT(phyaddr);
  }
  ulDuplex = (uint32_t) ptConfig->tMiiCfg.bf.uDefaultDuplex ^ (uint32_t) ptConfig->tMiiCfg.bf.uBitValHalfDuplex;

  NX_WRITE32(ptPrgData->tXPIC_RAM_VAR.ulXPIC_RAM_PHYMODE, ulDuplex << (ptConfig->tMiiCfg.bf.uBitIdxDuplex + MIIMU_SRT(data)));
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_MIIMU_READ_PHYMODE, ulMiimu);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_PHYMODE_MSK, 1UL << (ptConfig->tMiiCfg.bf.uBitIdxDuplex + MIIMU_SRT(data)));
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_MIIMU_MODE_REQ_NXT,
    (uint32_t) ( (ptConfig->tMiiCfg.bf.uRegAdrDuplex == ptConfig->tMiiCfg.bf.uRegAdrSpeed) ? (ptConfig->tMiiCfg.bf.fAutoDetectLink ? ETH_XPIC_MIIM_REQ_LINK : ETH_XPIC_MIIM_REQ_MODE) : ETH_XPIC_MIIM_REQ_SPEED));

  /* AutoDetection of PHY speed */
  ulMiimu = 0;
  if(!ptConfig->fPhyMode)
  {
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.fAutoDetectMode;
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.uPreamble << MIIMU_SRT(preamble);
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.uMdcPeriod << MIIMU_SRT(mdc_period);
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.uRta << MIIMU_SRT(rta);
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.uRegAdrSpeed << MIIMU_SRT(regaddr);
    ulMiimu |= (uint32_t) ptConfig->tMiiCfg.bf.uPhyAddress << MIIMU_SRT(phyaddr);
  }
  ulSpeed = (uint32_t) ptConfig->tMiiCfg.bf.uDefaultSpeed ^ (uint32_t) ptConfig->tMiiCfg.bf.uBitVal10Mbps;

  NX_WRITE32(ptPrgData->tXPIC_RAM_VAR.ulXPIC_RAM_PHYSPEED, ulSpeed << (ptConfig->tMiiCfg.bf.uBitIdxSpeed + MIIMU_SRT(data)));
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_MIIMU_READ_PHYSPEED, ulMiimu);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_PHYSPEED_MSK, 1UL << (ptConfig->tMiiCfg.bf.uBitIdxSpeed + MIIMU_SRT(data)));
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_MIIMU_SPEED_REQ_NXT,
    ptConfig->tMiiCfg.bf.fAutoDetectLink ? ETH_XPIC_MIIM_REQ_LINK : ETH_XPIC_MIIM_REQ_MODE);

  if(ptConfig->tMiiCfg.bf.fAutoDetectLink == 0)
    NX_WRITE32(ptPrgData->tXPIC_RAM_VAR.ulXPIC_RAM_MIIMU_REQ_ID, ETH_XPIC_MIIM_REQ_MODE);

  /* Slot time is 512 bit times */
  if(ptConfig->tMiiCfg.bf.uDefaultSpeed == 1)
    NX_WRITE32(ptPrgData->tXPIC_RAM_VAR.ulXPIC_RAM_SLOT_TIME, 512);
  else
    NX_WRITE32(ptPrgData->tXPIC_RAM_VAR.ulXPIC_RAM_SLOT_TIME, 5120);

  /* Initialize shared memory */
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_OK, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_JUMBO, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_CRC_ERR, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_ALIGNMENT_ERR, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_MII_RXERR, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_RUNT, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_COLLISION_FRAGMENT, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_DROP, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_NEXT_BEFORE_FIN, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_CNT_TX_OK, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_CNT_TX_SINGLE_COLLISIONS, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_CNT_TX_MULTIPLE_COLLISIONS, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_CNT_TX_LATE_COLLISIONS, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_CNT_FATAL, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_IRQ_MSK, 0xffffffff);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_IRQ_IND, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_IRQ_CNF, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_MIIMU_REQ, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_VLAN_PRIORITIZATION, DFLT_ETH_XPIC_TC_VLAN_PRIO);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_DSCP_PRIORITIZATION, DFLT_ETH_XPIC_TC_DSCP_PRIO);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_PROMISC_MODE, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_CONNECTION_STATE,
    (((uint32_t ) ptConfig->tMiiCfg.bf.uDefaultSpeed << SRT_XPIC_SHM_CONNECTION_STATE_SPEED)
      | ((uint32_t ) ptConfig->tMiiCfg.bf.uDefaultDuplex << SRT_XPIC_SHM_CONNECTION_STATE_DUPLEX)));
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_PFIFO_EMPTY_PTR_HOST, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_PFIFO_EMPTY_PTR_MAC, ETH_XPIC_FRAMEBUF_CNT);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_PFIFO_REQ_HOST, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_PFIFO_REQ_MAC, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_PFIFO_CNF_HOST, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_PFIFO_CNF_MAC, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_PFIFO_IND_HOST, 0);
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_PFIFO_IND_MAC, 0);

  /* Fill empty pointer FIFO and prepare frame buffers */
  pbFifo = (volatile uint8_t*) &s_ptXpicShm->ulAREA_XPIC_SHM_PFIFO_EMPTY_PTR;
  for(bFifoElement = 0; bFifoElement < ETH_XPIC_FRAMEBUF_CNT; bFifoElement++)
  {
    ptFrame = (ETH_FRAMEBUF_T*) HOSTPTR_ETH_XPIC_FRAME_BUFFER(bFifoElement);
    NX_WRITE32(ptFrame->tETH_FRAMEBUF_INFO.ulETH_FRAMEBUF_INFO, bFifoElement | MSK_ETH_FRAMEBUF_INFO_VALID);
    /*lint -save -e661 */
    /*lint -save -e662 */
    NX_WRITE8(pbFifo[bFifoElement], bFifoElement);
    /*lint -restore */
    /*lint -restore */
#ifdef DEBUG
    g_aptFrames[bFifoElement] = ptFrame;
#endif
  }

  s_eState = DRV_ETH_XPIC_STATE_INITIALIZED;

  return DRV_ETH_XPIC_OK;
}

/*****************************************************************************/
/*! Set Ethernet Hardware Address
 * \description
 *   Set the Ethernet address of the controller.
 *   Received frames are indicated only if the frame's destination address matches the controller's address.
 *   This function must be called after DRV_ETH_Xpic_Initialize() and before DRV_ETH_Xpic_Start().
 * \class
 *   Control Service Class
 * \params
 *   pabMacAdr  [in]  Pointer of an array of bytes of length 6 containing the MAC address in binary format
 * \return
 *   DRV_ETH_XPIC_OK                                                 */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_SetMacAddr(uint8_t const* pabMacAdr)
{
  uint32_t ulMacAdrHi, ulMacAdrLo;
  ETH_XPIC_RAM_T* ptPrgData = (ETH_XPIC_RAM_T*) s_apulXpicDram[ETH_XPIC_INST];

#ifndef __DRV_ETH_XPIC_DISABLE_CHECKS__
  if(s_eState != DRV_ETH_XPIC_STATE_INITIALIZED)
    return DRV_ETH_XPIC_NOT_INITIALIZED;
#endif

  /* Remember MAC address */
  memcpy(s_abMacAdr, pabMacAdr, 6);

  /* Set MAC address */
  ulMacAdrHi = ((uint32_t) s_abMacAdr[5] << 8) | ((uint32_t) s_abMacAdr[4]);
  ulMacAdrLo = ((uint32_t) s_abMacAdr[3] << 24) | ((uint32_t) s_abMacAdr[2] << 16) | ((uint32_t) s_abMacAdr[1] << 8) | ((uint32_t) s_abMacAdr[0]);
  if((ulMacAdrHi | ulMacAdrLo) == 0)
  {
    ulMacAdrHi = 0xffffffff;
  }

  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_LOCAL_MAC_ADDRESS_HI, ulMacAdrHi);
  NX_WRITE32(ptPrgData->tXPIC_RAM_PARAMETERS.ulXPIC_RAM_LOCAL_MAC_ADDRESS_LO, ulMacAdrLo);

  return DRV_ETH_XPIC_OK;
}

/*****************************************************************************/
/*! Get Ethernet Hardware Address
 * \description
 *   Get the Ethernet address of the controller.
 * \class
 *   Status Service Class
 * \params
 *   pabMacAdr  [out]  Pointer of an array of bytes of length 6 into which the MAC address is written in binary format
 * \return
 *   DRV_ETH_XPIC_OK                                                 */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_GetMacAddr(uint8_t* pabMacAdr)
{
  memcpy(pabMacAdr, s_abMacAdr, 6);
  return DRV_ETH_XPIC_OK;
}

/*****************************************************************************/
/*! Set Operational Parameters
 * \description
 *   Sets one operational parameter of the controller.
 * \class
 *   Control Service Class
 * \params
 *   eParam  [in]  The parameter type to be set
 *   ulValue [in]  The parameter value
 * \return
 *   DRV_ETH_XPIC_OK
 *   DRV_ETH_XPIC_INVALID_ARG  */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_SetParam(DRV_ETH_XPIC_PARAM eParam, uint32_t ulValue)
{
  switch (eParam)
  {
  case DRV_ETH_XPIC_PARAM_IRQ_MASK:
    NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_IRQ_MSK, ulValue);
    break;

  case DRV_ETH_XPIC_PARAM_VLAN_PRIORITIZATION:
    NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_VLAN_PRIORITIZATION, ulValue);
    break;

  case DRV_ETH_XPIC_PARAM_DSCP_PRIORITIZATION:
    NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_DSCP_PRIORITIZATION, ulValue);
    break;

  case DRV_ETH_XPIC_PROMISC_MODE:
    NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_PROMISC_MODE, ulValue);
    break;

  default:
    return DRV_ETH_XPIC_INVALID_ARG;
  }
  return DRV_ETH_XPIC_OK;
}

/*****************************************************************************/
/*! Get Operational Parameters
 * \description
 *   Gets one operational parameter of the controller.
 * \class
 *   Status Service Class
 * \params
 *   eParam   [in]  The parameter type to be read
 *   pulValue [out] The parameter value
 * \return
 *   DRV_ETH_XPIC_OK
 *   DRV_ETH_XPIC_INVALID_ARG            */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_GetParam(DRV_ETH_XPIC_PARAM eParam, uint32_t* pulValue)
{
  switch (eParam)
  {
  case DRV_ETH_XPIC_PARAM_IRQ_MASK:
    *pulValue = NX_READ32(s_ptXpicShm->ulXPIC_SHM_IRQ_MSK);
    break;

  case DRV_ETH_XPIC_PARAM_VLAN_PRIORITIZATION:
    *pulValue = NX_READ32(s_ptXpicShm->ulXPIC_SHM_VLAN_PRIORITIZATION);
    break;

  case DRV_ETH_XPIC_PARAM_DSCP_PRIORITIZATION:
    *pulValue = NX_READ32(s_ptXpicShm->ulXPIC_SHM_DSCP_PRIORITIZATION);
    break;

  case DRV_ETH_XPIC_PROMISC_MODE:
    *pulValue = NX_READ32(s_ptXpicShm->ulXPIC_SHM_PROMISC_MODE);
    break;

  case DRV_ETH_XPIC_CONNECTION_STATE:
    *pulValue = NX_READ32(s_ptXpicShm->ulXPIC_SHM_CONNECTION_STATE);
    break;

  default:
    return DRV_ETH_XPIC_INVALID_ARG;
  }
  return DRV_ETH_XPIC_OK;
}

/*****************************************************************************/
/*! Start HAL
 * \description
 *   Starts the HAL and the xPIC
 * \class
 *   Control Service Class
 * \params
 *   pvUser    [in]  User defined parameter passed into system calls
 * \return
 *   DRV_ETH_XPIC_OK                                                 */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_Start(void* pvUser)
{
  s_eState = DRV_ETH_XPIC_STATE_RUNNING;

  DRV_XPIC_Start(ETH_XPIC_INST, pvUser);

  return DRV_ETH_XPIC_OK;
}

/*****************************************************************************/
/*! Get and Confirm Interrupts
 * \description
 *   Get a bit array indicating all current active interrupts.
 *   All indicated interrupts are confirmed by this function.
 * \class
 *   Control Service Class
 * \params
 *   pulIrq    [out]  The current interrupt bits are stored at
 *                    this location
 * \return
 *   DRV_ETH_XPIC_OK                                                 */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_GetConfirmIrq(uint32_t* pulIrq)
{
  uint32_t ulIrqCnf = NX_READ32(s_ptXpicShm->ulXPIC_SHM_IRQ_CNF);
  uint32_t ulIrqAct = NX_READ32(s_ptXpicShm->ulXPIC_SHM_IRQ_IND) ^ ulIrqCnf;

  /* Confirm IRQs */
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_IRQ_CNF, ulIrqCnf ^ ulIrqAct);

#ifndef __DRV_ETH_XPIC_DISABLE_CHECKS__
  if(pulIrq == NULL)
    return DRV_ETH_XPIC_INVALID_ARG;
#endif

  /* Return IRQs */
  *pulIrq = ulIrqAct;

  return DRV_ETH_XPIC_OK;
}

/*****************************************************************************/
/*! Get Frame
 * \description
 *   Return an empty Ethernet frame that can be filled and sent with
 *   DRV_ETH_Xpic_Send()
 * \class
 *   Transmission Service Class
 * \params
 *   pptFrame    [out]  A pointer to a new Ethernet frame buffer is stored at
 *                      this location
 * \return
 *   DRV_ETH_XPIC_OK A
 *   DRV_ETH_XPIC_OUT_OF_FRAMES     */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_GetFrame(ETH_FRAMEBUF_T** pptFrame)
{
  unsigned int uiFifoOfs = NX_READ32(s_ptXpicShm->ulXPIC_SHM_PFIFO_EMPTY_PTR_HOST);
  volatile uint8_t* pbFifo = (volatile uint8_t*) &s_ptXpicShm->ulAREA_XPIC_SHM_PFIFO_EMPTY_PTR;
  uint8_t bFifoElement;
  volatile uint32_t* pulNextPtr;
  ETH_FRAMEBUF_T* ptFrame;

#ifndef __DRV_ETH_XPIC_DISABLE_CHECKS__
  if(pptFrame == NULL)
    return DRV_ETH_XPIC_INVALID_ARG;
#endif

  if(uiFifoOfs == NX_READ32(s_ptXpicShm->ulXPIC_SHM_PFIFO_EMPTY_PTR_MAC))
  {
    *pptFrame = NULL;
    return DRV_ETH_XPIC_NO_FRAME_AVAILABLE;
  }

  /* get empty frame buffer from FIFO */
  bFifoElement = NX_READ8(pbFifo[uiFifoOfs++]);
  uiFifoOfs &= ETH_XPIC_PFIFO_BORDER;
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_PFIFO_EMPTY_PTR_HOST, uiFifoOfs);

  /* get frame pointer */
  ptFrame = (ETH_FRAMEBUF_T*) HOSTPTR_ETH_XPIC_FRAME_BUFFER(bFifoElement);

  /* make next pointer invalid */
  pulNextPtr = ((uint32_t*) (ptFrame + 1)) - 1;
  NX_WRITE32(pulNextPtr[0], 0xffffffff);

  /* return frame */
  *pptFrame = ptFrame;

  return DRV_ETH_XPIC_OK;
}

/*****************************************************************************/
/*! Send Frame
 * \description
 *   Send a frame. The frame is identified by a handle that must be allocated by
 *   DRV_ETH_Xpic_GetFrame()
 * \class
 *   Transmission Service Class
 * \params
 *   ptFrame     [in]  The Ethernet frame to be sent
 *   fConfirm    [in]  false: Frame is sent without confirmation, frame may be dropped on errors
 *                     true: Frame is sent with confirmation
 *   fHiPriority [in]  false/true: Send with low/high priority
 * \return
 *   DRV_ETH_XPIC_OK
 *   DRV_ETH_XPIC_INVALID_HANDLE     */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_Send(ETH_FRAMEBUF_T* ptFrame, bool fConfirm, bool fHiPriority)
{
  volatile uint8_t* pbFifoOfs = (volatile uint8_t*) &s_ptXpicShm->ulXPIC_SHM_PFIFO_REQ_HOST;
  volatile uint8_t* pbFifo =
    fHiPriority ? (volatile uint8_t*) &s_ptXpicShm->ulAREA_XPIC_SHM_PFIFO_REQ_HI : (volatile uint8_t*) &s_ptXpicShm->ulAREA_XPIC_SHM_PFIFO_REQ_LO;
  uint8_t bFifoElement;
  unsigned int uiFifoOfs;
  uint32_t ulInfo;

#ifndef __DRV_ETH_XPIC_DISABLE_CHECKS__
  if(ptFrame == NULL)
    return DRV_ETH_XPIC_INVALID_ARG;
#endif

  ulInfo = NX_READ32(ptFrame->tETH_FRAMEBUF_INFO.ulETH_FRAMEBUF_INFO);
  bFifoElement = ulInfo & MSK_ETH_FRAMEBUF_INFO_BUFFER_NUMBER;

  /* set frame confirmation flag */
  if(fConfirm)
    ulInfo |= MSK_ETH_FRAMEBUF_INFO_CON_EN;
  else
    ulInfo &= ~MSK_ETH_FRAMEBUF_INFO_CON_EN;
  NX_WRITE32(ptFrame->tETH_FRAMEBUF_INFO.ulETH_FRAMEBUF_INFO, ulInfo);

  /* put frame into request FIFO */
  uiFifoOfs = NX_READ8(pbFifoOfs[fHiPriority ? 1 : 0]);
  NX_WRITE8(pbFifo[uiFifoOfs++], bFifoElement);
  uiFifoOfs &= ETH_XPIC_PFIFO_BORDER;
  NX_WRITE8(pbFifoOfs[fHiPriority ? 1 : 0], (uint8_t ) uiFifoOfs);

  return DRV_ETH_XPIC_OK;
}

/*****************************************************************************/
/*! Get Number of Queued Frame Confirmations
 * \description
 *   Retrieve the number of available frame send confirmations.
 *   DRV_ETH_Xpic_GetSendCnf() should be called as many times as this function returns.
 * \class
 *   Transmission Service Class
 * \return
 *   Number of pending confirmations */
/*****************************************************************************/
unsigned int DRV_ETH_Xpic_GetSendCnfFillLevel(void)
{
  unsigned int uiMacPtr = NX_READ32(s_ptXpicShm->ulXPIC_SHM_PFIFO_CNF_MAC);
  unsigned int uiHostPtr = NX_READ32(s_ptXpicShm->ulXPIC_SHM_PFIFO_CNF_HOST);

  if(uiHostPtr > uiMacPtr)
    uiHostPtr = uiMacPtr + (ETH_XPIC_PFIFO_DEPTH - uiHostPtr);
  else
    uiHostPtr = uiMacPtr - uiHostPtr;

  return uiHostPtr;
}

/*****************************************************************************/
/*! Get Frame Confirmation
 * \description
 *   Retrieve the confirmation of a frame that was sent via DRV_ETH_Xpic_Send()
 *   with the fConfirm flag enabled. After analyzing the confirmation, the frame
 *   must be released with DRV_ETH_Xpic_ReleaseFrame()
 * \class
 *   Transmission Service Class
 * \params
 *   pptFrame     [out]  A pointer to the sent Ethernet frame
 *   pulErrorCode [out]  The result code of the frame transmission
 * \return
 *   DRV_ETH_XPIC_OK
 *   DRV_ETH_XPIC_NO_FRAME_AVAILABLE  */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_GetSendCnf(ETH_FRAMEBUF_T** pptFrame, ETH_XPIC_CNF_ERR* pulErrorCode)
{
  uint32_t ulInfo;
  uint8_t bFrameBufNum;
  volatile uint8_t* pbFifo = (volatile uint8_t*) &s_ptXpicShm->ulAREA_XPIC_SHM_PFIFO_CNF;
  unsigned int uiFifoOfs = NX_READ32(s_ptXpicShm->ulXPIC_SHM_PFIFO_CNF_HOST);
  ETH_FRAMEBUF_T* ptFrame;

#ifndef __DRV_ETH_XPIC_DISABLE_CHECKS__
  if(pptFrame == NULL)
    return DRV_ETH_XPIC_INVALID_ARG;
#endif

  if(uiFifoOfs == NX_READ32(s_ptXpicShm->ulXPIC_SHM_PFIFO_CNF_MAC))
  {
    *pptFrame = NULL;
    if(pulErrorCode != NULL)
      *pulErrorCode = ETH_XPIC_CNF_ERROR_CODE_INVALID;
    return DRV_ETH_XPIC_NO_FRAME_AVAILABLE;
  }

  /* get confirmed frame from FIFO */
  bFrameBufNum = NX_READ8(pbFifo[uiFifoOfs++]) & MSK_ETH_FRAMEBUF_INFO_BUFFER_NUMBER;
  uiFifoOfs &= ETH_XPIC_PFIFO_BORDER;
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_PFIFO_CNF_HOST, uiFifoOfs);

  /* get frame pointer */
  ptFrame = (ETH_FRAMEBUF_T*) HOSTPTR_ETH_XPIC_FRAME_BUFFER(bFrameBufNum);

  /* get result code */
  if(pulErrorCode != NULL)
  {
    ulInfo = NX_READ32(ptFrame->tETH_FRAMEBUF_INFO.ulETH_FRAMEBUF_INFO);
    *pulErrorCode = (ETH_XPIC_CNF_ERR) ((ulInfo & MSK_ETH_FRAMEBUF_INFO_ERROR_CODE) >> SRT_ETH_FRAMEBUF_INFO_ERROR_CODE);
  }

  /* return frame */
  *pptFrame = ptFrame;

  return DRV_ETH_XPIC_OK;
}

/*****************************************************************************/
/*! Get Number of Queued Received Frames
 * \description
 *   Retrieve the number of received frames.
 *   DRV_ETH_Xpic_Recv() should be called as many times as this function returns.
 * \class
 *   Reception Service Class
 * \params
 *   fHiPriority [in]   false/true: Get number of low/high priority frames
 * \return
 *   Number of pending received frames */
/*****************************************************************************/
unsigned int DRV_ETH_Xpic_GetRecvFillLevel(bool fHiPriority)
{
  unsigned int uiHostOfs = NX_READ32(s_ptXpicShm->ulXPIC_SHM_PFIFO_IND_HOST);
  unsigned int uiMacOfs = NX_READ32(s_ptXpicShm->ulXPIC_SHM_PFIFO_IND_MAC);

  if(fHiPriority)
  {
    uiHostOfs = (uiHostOfs & MSK_XPIC_SHM_PFIFO_IND_HOST_OFS_HI) >> SRT_XPIC_SHM_PFIFO_IND_HOST_OFS_HI;
    uiMacOfs = (uiMacOfs & MSK_XPIC_SHM_PFIFO_IND_MAC_OFS_HI) >> SRT_XPIC_SHM_PFIFO_IND_MAC_OFS_HI;
  }
  else
  {
    uiHostOfs = (uiHostOfs & MSK_XPIC_SHM_PFIFO_IND_HOST_OFS_LO) >> SRT_XPIC_SHM_PFIFO_IND_HOST_OFS_LO;
    uiMacOfs = (uiMacOfs & MSK_XPIC_SHM_PFIFO_IND_MAC_OFS_LO) >> SRT_XPIC_SHM_PFIFO_IND_MAC_OFS_LO;
  }

  if(uiHostOfs > uiMacOfs)
    uiHostOfs = uiMacOfs + (ETH_XPIC_PFIFO_DEPTH - uiHostOfs);
  else
    uiHostOfs = uiMacOfs - uiHostOfs;

  return uiHostOfs;
}

/*****************************************************************************/
/*! Receive Frame
 * \description
 *   Receive a frame. After analyzing the frame it must be released
 *   with DRV_ETH_Xpic_ReleaseFrame()
 * \class
 *   Reception Service Class
 * \params
 *   pptFrame    [out]  A pointer to the received Ethernet frame is stored at
 *                      this location
 *   fHiPriority [in]   false/true Receive frame from low/high indication FIFO
 * \return
 *   DRV_ETH_XPIC_OK
 *   DRV_ETH_XPIC_NO_FRAME_AVAILABLE */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_Recv(ETH_FRAMEBUF_T** pptFrame, bool fHiPriority)
{
  volatile uint8_t* pbHostOfs = (volatile uint8_t*) &s_ptXpicShm->ulXPIC_SHM_PFIFO_IND_HOST;
  volatile uint8_t* pbMacOfs = (volatile uint8_t*) &s_ptXpicShm->ulXPIC_SHM_PFIFO_IND_MAC;
  volatile uint8_t* pbFifo =
    fHiPriority ? (volatile uint8_t*) &s_ptXpicShm->ulAREA_XPIC_SHM_PFIFO_IND_HI : (volatile uint8_t*) &s_ptXpicShm->ulAREA_XPIC_SHM_PFIFO_IND_LO;
  unsigned int uiFifoOfs = NX_READ8(pbHostOfs[fHiPriority ? 1 : 0]);
  uint8_t bFrameBufNum;
  ETH_FRAMEBUF_T* ptFrame;

#ifndef __DRV_ETH_XPIC_DISABLE_CHECKS__
  if(pptFrame == NULL)
    return DRV_ETH_XPIC_INVALID_ARG;
#endif

  if(uiFifoOfs == NX_READ8(pbMacOfs[fHiPriority ? 1 : 0]))
  {
    *pptFrame = NULL;
    return DRV_ETH_XPIC_NO_FRAME_AVAILABLE;
  }

  /* get received frame from FIFO */
  bFrameBufNum = NX_READ8(pbFifo[uiFifoOfs++]) & MSK_ETH_FRAMEBUF_INFO_BUFFER_NUMBER;
  uiFifoOfs &= ETH_XPIC_PFIFO_BORDER;
  NX_WRITE8(pbHostOfs[fHiPriority ? 1 : 0], (uint8_t ) uiFifoOfs);

  /* get frame pointer */
  ptFrame = (ETH_FRAMEBUF_T*) HOSTPTR_ETH_XPIC_FRAME_BUFFER(bFrameBufNum);

  /* return frame */
  *pptFrame = ptFrame;

  return DRV_ETH_XPIC_OK;
}

/*****************************************************************************/
/*! Release Frame
 * \description
 *   Release a frame that was previously acquired via DRV_ETH_Xpic_Recv() or
 *   DRV_ETH_Xpic_GetSendCnf()
 * \class
 *   Reception Service Class
 * \params
 *   ptFrame     [in]  The Ethernet frame to be released
 * \return
 *   DRV_ETH_XPIC_OK
 *   DRV_ETH_XPIC_INVALID_HANDLE */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_ReleaseFrame(ETH_FRAMEBUF_T* ptFrame)
{
  volatile uint8_t* pbFifo = (volatile uint8_t*) &s_ptXpicShm->ulAREA_XPIC_SHM_PFIFO_EMPTY_PTR;
  unsigned int uiFifoOfs = NX_READ32(s_ptXpicShm->ulXPIC_SHM_PFIFO_EMPTY_PTR_HOST);
  uint8_t bFifoElement;

#ifndef __DRV_ETH_XPIC_DISABLE_CHECKS__
  if(ptFrame == NULL)
    return DRV_ETH_XPIC_INVALID_ARG;
#endif

  bFifoElement = MSK_ETH_FRAMEBUF_INFO_BUFFER_NUMBER & NX_READ32(ptFrame->tETH_FRAMEBUF_INFO.ulETH_FRAMEBUF_INFO);
  NX_WRITE32(ptFrame->tETH_FRAMEBUF_INFO.ulETH_FRAMEBUF_INFO, MSK_ETH_FRAMEBUF_INFO_VALID + bFifoElement);

  uiFifoOfs = (uiFifoOfs == 0U) ? ETH_XPIC_PFIFO_BORDER : uiFifoOfs - 1U;
  /*lint -save -e661 */
  /*lint -save -e662 */
  NX_WRITE8(pbFifo[uiFifoOfs], bFifoElement);
  /*lint -restore */
  /*lint -restore */
  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_PFIFO_EMPTY_PTR_HOST, uiFifoOfs);

  return DRV_ETH_XPIC_OK;
}

/*****************************************************************************/
/*! Get MAC Counters
 * \description
 *   Retrieves the diagnostic frame counters.
 * \class
 *   Status Service Class
 * \params
 *   ptCounters   [out]  A copy of the internal diagnostic MAC counters
 * \return
 *   DRV_ETH_XPIC_OK
 *   DRV_ETH_XPIC_INVALID_ARG */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_GetCounters(XPIC_SHM_CNT_T* ptCounters)
{
#ifndef __DRV_ETH_XPIC_DISABLE_CHECKS__
  if(ptCounters == NULL)
    return DRV_ETH_XPIC_INVALID_ARG;
#endif

  ptCounters->ulXPIC_SHM_CNT_RX_OK = NX_READ32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_OK);
  ptCounters->ulXPIC_SHM_CNT_RX_JUMBO = NX_READ32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_JUMBO);
  ptCounters->ulXPIC_SHM_CNT_RX_CRC_ERR = NX_READ32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_CRC_ERR);
  ptCounters->ulXPIC_SHM_CNT_RX_ALIGNMENT_ERR = NX_READ32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_ALIGNMENT_ERR);
  ptCounters->ulXPIC_SHM_CNT_RX_MII_RXERR = NX_READ32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_MII_RXERR);
  ptCounters->ulXPIC_SHM_CNT_RX_RUNT = NX_READ32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_RUNT);
  ptCounters->ulXPIC_SHM_CNT_RX_COLLISION_FRAGMENT = NX_READ32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_COLLISION_FRAGMENT);
  ptCounters->ulXPIC_SHM_CNT_RX_DROP = NX_READ32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_DROP);
  ptCounters->ulXPIC_SHM_CNT_RX_NEXT_BEFORE_FIN = NX_READ32(s_ptXpicShm->ulXPIC_SHM_CNT_RX_NEXT_BEFORE_FIN);
  ptCounters->ulXPIC_SHM_CNT_TX_OK = NX_READ32(s_ptXpicShm->ulXPIC_SHM_CNT_TX_OK);
  ptCounters->ulXPIC_SHM_CNT_TX_SINGLE_COLLISIONS = NX_READ32(s_ptXpicShm->ulXPIC_SHM_CNT_TX_SINGLE_COLLISIONS);
  ptCounters->ulXPIC_SHM_CNT_TX_MULTIPLE_COLLISIONS = NX_READ32(s_ptXpicShm->ulXPIC_SHM_CNT_TX_MULTIPLE_COLLISIONS);
  ptCounters->ulXPIC_SHM_CNT_TX_LATE_COLLISIONS = NX_READ32(s_ptXpicShm->ulXPIC_SHM_CNT_TX_LATE_COLLISIONS);
  ptCounters->ulXPIC_SHM_CNT_FATAL = NX_READ32(s_ptXpicShm->ulXPIC_SHM_CNT_FATAL);

  return DRV_ETH_XPIC_OK;
}

/*****************************************************************************/
/*! Read PHY Register
 * \description
 *   Read out a register of an external PHY via MDIO.
 *   The xPIC uses the MIIMU unit to read out the PHY's Speed and DUPLEX information.
 *   The common function NX4000_MIIMU_ReadPhyReg() also accesses the MIIMU unit and must not be called after DRV_ETH_Xpic_Start().
 *   Instead this function can be called which has the same function parameters as NX4000_MIIMU_ReadPhyReg().
 *   If this function returns DRV_ETH_XPIC_NOT_RUNNING the function NX4000_MIIMU_ReadPhyReg() should be called instead.
 * \class
 *   Control Service Class
 * \params
 *   uMiimuPreamble              [in]  MIIMU Preamble
 *   uMiimuMdcFreq               [in]  MIIMU MDC Frequency
 *   uMiimuRtaField              [in]  MIIMU RTA Field
 *   uMiimuPhyAddr               [in]  MIIMU PHY Address
 *   uMiimuReqAddr               [in]  MIIMU Register Address
 *   pusData                     [out] MIIMU Data
 *   pvUser                      [in]  User specific parameter
 * \return
 *   DRV_ETH_XPIC_OK
 *   DRV_ETH_XPIC_NOT_RUNNING */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_ReadPhyReg(unsigned int uMiimuPreamble, unsigned int uMiimuMdcFreq, unsigned int uMiimuRtaField,
  unsigned int uMiimuPhyAddr, unsigned int uMiimuReqAddr, uint16_t* pusData, void* pvUser)
{
  uint32_t ulRegVal;

#ifndef __DRV_ETH_XPIC_DISABLE_CHECKS__
  if(s_eState != DRV_ETH_XPIC_STATE_RUNNING)
    return DRV_ETH_XPIC_NOT_RUNNING;
#endif

  do
  {
    ulRegVal = NX_READ32(s_ptXpicShm->ulXPIC_SHM_MIIMU_REQ);
  } while((ulRegVal & MIIMU_MSK(snrdy)) != 0);

  ulRegVal = ((uint32_t) uMiimuPreamble << MIIMU_SRT(preamble)) | ((uint32_t) uMiimuMdcFreq << MIIMU_SRT(mdc_period))
    | ((uint32_t) uMiimuRtaField << MIIMU_SRT(rta)) | ((uint32_t) uMiimuReqAddr << MIIMU_SRT(regaddr))
    | ((uint32_t) uMiimuPhyAddr << MIIMU_SRT(phyaddr)) | ((uint32_t) 1 << MIIMU_SRT(rta)) | ((uint32_t) 1 << MIIMU_SRT(snrdy));

  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_MIIMU_REQ, ulRegVal);

  do
  {
    ulRegVal = NX_READ32(s_ptXpicShm->ulXPIC_SHM_MIIMU_REQ);
  } while((ulRegVal & MIIMU_MSK(snrdy)) != 0);

  *pusData = (uint16_t) ((ulRegVal & MIIMU_MSK(data)) >> MIIMU_SRT(data));

  return DRV_ETH_XPIC_OK;
}

/*****************************************************************************/
/*! Write PHY Register
 * \description
 *   Write into a register of an external PHY via MDIO.
 *   The xPIC uses the MIIMU unit to read out the PHY's Speed and DUPLEX information.
 *   The common function NX4000_MIIMU_WritePhyReg() also accesses the MIIMU unit and must not be called after DRV_ETH_Xpic_Start().
 *   Instead this function can be called which has the same function parameters as NX4000_MIIMU_WritePhyReg().
 *   If this function returns DRV_ETH_XPIC_NOT_RUNNING the function NX4000_MIIMU_WritePhyReg() should be called instead.
 * \class
 *   Control Service Class
 * \params
 *   uMiimuPreamble              [in]  MIIMU Preamble
 *   uMiimuMdcFreq               [in]  MIIMU MDC Frequency
 *   uMiimuPhyAddr               [in]  MIIMU PHY Address
 *   uMiimuReqAddr               [in]  MIIMU Register Address
 *   usData                      [in]  MIIMU Data
 *   pvUser                      [in]  User specific parameter
 * \return
 *   DRV_ETH_XPIC_OK
 *   DRV_ETH_XPIC_NOT_RUNNING */
/*****************************************************************************/
DRV_ETH_XPIC_RESULT DRV_ETH_Xpic_WritePhyReg(unsigned int uMiimuPreamble, unsigned int uMiimuMdcFreq, unsigned int uMiimuPhyAddr,
  unsigned int uMiimuReqAddr, uint16_t usData, void* pvUser)
{
  uint32_t ulRegVal;

#ifndef __DRV_ETH_XPIC_DISABLE_CHECKS__
  if(s_eState != DRV_ETH_XPIC_STATE_RUNNING)
    return DRV_ETH_XPIC_NOT_RUNNING;
#endif

  do
  {
    ulRegVal = NX_READ32(s_ptXpicShm->ulXPIC_SHM_MIIMU_REQ);
  } while((ulRegVal & MIIMU_MSK(snrdy)) != 0);

  ulRegVal = ((uint32_t) uMiimuPreamble << MIIMU_SRT(preamble)) | ((uint32_t) uMiimuMdcFreq << MIIMU_SRT(mdc_period))
    | ((uint32_t) uMiimuReqAddr << MIIMU_SRT(regaddr)) | ((uint32_t) uMiimuPhyAddr << MIIMU_SRT(phyaddr)) | ((uint32_t) usData << MIIMU_SRT(data))
    | MIIMU_MSK(opmode) |
    MIIMU_MSK(rta) |
    MIIMU_MSK(snrdy);

  NX_WRITE32(s_ptXpicShm->ulXPIC_SHM_MIIMU_REQ, ulRegVal);

  return DRV_ETH_XPIC_OK;
}

const uint32_t BuildTime_eth_xpic_prg1[7] = { 53, 55, 6, 3, 8, 119, 1567493753 };

const uint32_t XpicCode_eth_xpic_prg1[1797] = { 0x00000004, /* program size */
0x00001c08, /* trailing loads size */
0x00000000,
/* trailing loads */
0xff880000, 0xff480000, 0xff880004, 0xff200200, 0xff880008, 0xff900100, 0xff88000c, 0xff900000, 0xff880010, 0x8000001a, 0xff880014, 0x80000001,
  0xff880018, 0x04000000, 0xff88001c, 0xff480038, 0xff880020, 0x00008100, 0xff880024, 0x0000ff00, 0xff880028, 0x80000000, 0xff88002c, 0x00004000,
  0xff880030, 0x00005000, 0xff880034, 0x1c000000, 0xff880400, 0x00000000, 0xff880404, 0x00000000, 0xff880408, 0x00000000, 0xff88040c, 0x00000000,
  0xff880410, 0x00000000, 0xff880414, 0x00000000, 0xff880418, 0x00000000, 0xff88041c, 0x00000000, 0xff880420, 0x00000000, 0xff880424, 0x00000000,
  0xff880428, 0x00000000, 0xff88042c, 0x00000000, 0xff880430, 0x00000000, 0xff880434, 0x00000000, 0xff880438, 0x00000000, 0xff88043c, 0x00000000,
  0xff880440, 0x01000000, 0xff880444, 0x00000000, 0xff880448, 0x00000000, 0xff88044c, 0x00000000, 0xff880450, 0x00000000, 0xff880454, 0x00000000,
  0xff880458, 0x00000000, 0xff88045c, 0x00000000, 0xff880460, 0x00000000, 0xff880464, 0x00000000, 0xff880468, 0x00000000, 0xff88046c, 0x00000000,
  0xff880470, 0x00000000, 0xff880474, 0x00000000, 0xff880478, 0x00000000, 0xff88047c, 0x00000000, 0xff880480, 0x00000000, 0xff880484, 0x00000000,
  0xff880488, 0x00000000, 0xff88048c, 0x00000000, 0xff880490, 0x00000000, 0xff880494, 0x00000000, 0xff880498, 0x00000000, 0xff88049c, 0x00000000,
  0xff8805a0, 0x04000400, 0xff8805a4, 0x040c0408, 0xff8805a8, 0x04100410, 0xff8805ac, 0x04100410, 0xff8805c0, 0x00000009, 0xff8805c4, 0x00000005,
  0xff8805c8, 0x00000003, 0xff8805cc, 0x00000003, 0xff8805d0, 0x00000000, 0xff8805d4, 0x00000000, 0xff8805d8, 0x00000000, 0xff8805dc, 0x00000000,
  0xff8805e0, 0x00000000, 0xff8805e4, 0x00000000, 0xff8805e8, 0x00000000, 0xff8805ec, 0x00000000, 0xff8805f0, 0x00000000, 0xff8805f4, 0x00000000,
  0xff8805f8, 0x00000000, 0xff8805fc, 0x00000000, 0xff880600, 0x00000000, 0xff880604, 0x00000000, 0xff880608, 0x00000000, 0xff88060c, 0x00000000,
  0xff880610, 0x24242408, 0xff880614, 0x1c1c1c24, 0xff880618, 0x1414141c, 0xff88061c, 0x0c0c0c14, 0xff880620, 0x0808080c, 0xff880624, 0x00000000,
  0xff880628, 0x00000000, 0xff882000, 0x80100004, 0xff882004, 0x0de10004, 0xff882008, 0xc0000015, 0xff88200c, 0x91a9fffd, 0xff882010, 0x93a9fffd,
  0xff882014, 0x95a9fffd, 0xff882018, 0x99e80100, 0xff88201c, 0x077f800f, 0xff882020, 0x81e80102, 0xff882024, 0x00017fdc, 0xff882028, 0xa1e8011a,
  0xff88202c, 0xbfe80122, 0xff882030, 0xbfe80124, 0xff882034, 0xbfe80123, 0xff882038, 0xbfe80183, 0xff88203c, 0x80100007, 0xff882040, 0xa1e8017e,
  0xff882044, 0xbfe8017f, 0xff882048, 0x80100b78, 0xff88204c, 0xa1e8011c, 0xff882050, 0x81e80101, 0xff882054, 0x2001800f, 0xff882058, 0xa1e8011b,
  0xff88205c, 0xbf480000, 0xff882060, 0xbf480006, 0xff882064, 0xbf480001, 0xff882068, 0xbf480007, 0xff88206c, 0xb748000c, 0xff882070, 0xb708000a,
  0xff882074, 0x81e8010e, 0xff882078, 0x0401000f, 0xff88207c, 0x1c010003, 0xff882080, 0xa1080000, 0xff882084, 0x81e8010f, 0xff882088, 0x040100ff,
  0xff88208c, 0x1c01002e, 0xff882090, 0xa1080001, 0xff882094, 0xb708000d, 0xff882098, 0x91a9ffdd, 0xff88209c, 0xc0000010, 0xff8820a0, 0xb7080007,
  0xff8820a4, 0xb7080008, 0xff8820a8, 0x81a9ffda, 0xff8820ac, 0xa108000e, 0xff8820b0, 0x80100134, 0xff8820b4, 0xa108000a, 0xff8820b8, 0x81a9ffd7,
  0xff8820bc, 0xa108000f, 0xff8820c0, 0x80100614, 0xff8820c4, 0xa108000b, 0xff8820c8, 0x80100001, 0xff8820cc, 0xa1080000, 0xff8820d0, 0xd80021f0,
  0xff8820d4, 0x81a9ffd1, 0xff8820d8, 0xa1080004, 0xff8820dc, 0x0df07fc9, 0xff8820e0, 0xdffffff1, 0xff8820e4, 0x8010181a, 0xff8820e8, 0xa108000c,
  0xff8820ec, 0x81080000, 0xff8820f0, 0x1c012000, 0xff8820f4, 0xa1080000, 0xff8820f8, 0x1c1e8111, 0xff8820fc, 0x1c008112, 0xff882100, 0x11e10001,
  0xff882104, 0x81e80110, 0xff882108, 0x181f801f, 0xff88210c, 0xa1480007, 0xff882110, 0x80100001, 0xff882114, 0xa148000e, 0xff882118, 0x8010004b,
  0xff88211c, 0xa1480006, 0xff882120, 0x18ff800f, 0xff882124, 0x18df800f, 0xff882128, 0x187f800f, 0xff88212c, 0x84100214, 0xff882130, 0xc0000400,
  0xff882134, 0x81a9ffb9, 0xff882138, 0xa1080007, 0xff88213c, 0x8010015c, 0xff882140, 0xa108000a, 0xff882144, 0x91a9ffaf, 0xff882148, 0x18ff800f,
  0xff88214c, 0x18df800f, 0xff882150, 0x84100674, 0xff882154, 0xa5e80178, 0xff882158, 0xc8000010, 0xff88215c, 0x8108000b, 0xff882160, 0x10210008,
  0xff882164, 0xc0000141, 0xff882168, 0x85080006, 0xff88216c, 0xa308000d, 0xff882170, 0xa308000a, 0xff882174, 0x185f80cb, 0xff882178, 0xa5e80122,
  0xff88217c, 0x8308000a, 0xff882180, 0x10230180, 0xff882184, 0xc0000041, 0xff882188, 0xb7e80122, 0xff88218c, 0x10211812, 0xff882190, 0xc0003d51,
  0xff882194, 0x10210001, 0xff882198, 0xc0000061, 0xff88219c, 0x9be80178, 0xff8821a0, 0x200c817a, 0xff8821a4, 0xc8000010, 0xff8821a8, 0x11ef0002,
  0xff8821ac, 0xc0001391, 0xff8821b0, 0x1c3e8122, 0xff8821b4, 0x81080002, 0xff8821b8, 0x1001001f, 0xff8821bc, 0xa1e80125, 0xff8821c0, 0xc000020b,
  0xff8821c4, 0xa3880015, 0xff8821c8, 0x1cef0002, 0xff8821cc, 0xbfe80122, 0xff8821d0, 0x80100008, 0xff8821d4, 0xa108000c, 0xff8821d8, 0x0df78001,
  0xff8821dc, 0xc0001ae5, 0xff8821e0, 0xa3e80123, 0xff8821e4, 0x102307ff, 0xff8821e8, 0xa3e80124, 0xff8821ec, 0x202d8001, 0xff8821f0, 0x00230003,
  0xff8821f4, 0x46258001, 0xff8821f8, 0x50028125, 0xff8821fc, 0xc000110b, 0xff882200, 0x19bf81a2, 0xff882204, 0x11ef0002, 0xff882208, 0xc00010b1,
  0xff88220c, 0x9be8011c, 0xff882210, 0x81a9ff83, 0xff882214, 0x85080008, 0xff882218, 0xd8002190, 0xff88221c, 0x83800064, 0xff882220, 0x89800060,
  0xff882224, 0x0de98001, 0xff882228, 0xc0000e65, 0xff88222c, 0x00237fff, 0xff882230, 0x1023003f, 0xff882234, 0x00b90080, 0xff882238, 0x0de38004,
  0xff88223c, 0xc0000dc5, 0xff882240, 0x86b80801, 0xff882244, 0xa3800064, 0xff882248, 0xd80021f0, 0xff88224c, 0x83e80102, 0xff882250, 0x54078001,
  0xff882254, 0x00808101, 0xff882258, 0x00a90020, 0xff88225c, 0x86880001, 0xff882260, 0xba880002, 0xff882264, 0x00037fdc, 0xff882268, 0xa1e80127,
  0xff88226c, 0xb6981401, 0xff882270, 0x81480009, 0xff882274, 0x8348000a, 0xff882278, 0x21e58001, 0xff88227c, 0xa4880004, 0xff882280, 0x0001818b,
  0xff882284, 0xa0880005, 0xff882288, 0x84100290, 0xff88228c, 0xdffff8e0, 0xff882290, 0x0001800b, 0xff882294, 0x83080004, 0xff882298, 0xa2c80128,
  0xff88229c, 0xa2b80a06, 0xff8822a0, 0xffffffa0, 0xff8822a4, 0x84100290, 0xff8822a8, 0x801004a0, 0xff8822ac, 0x21ed000c, 0xff8822b0, 0xdffff7c6,
  0xff8822b4, 0x83f81200, 0xff8822b8, 0x11e30001, 0xff8822bc, 0xc00001f1, 0xff8822c0, 0x83880014, 0xff8822c4, 0x85e80128, 0xff8822c8, 0xc00000db,
  0xff8822cc, 0x2044810c, 0xff8822d0, 0xc00000d5, 0xff8822d4, 0x85e40252, 0xff8822d8, 0x2044810b, 0xff8822dc, 0xc0000075, 0xff8822e0, 0x1cef1000,
  0xff8822e4, 0x00670200, 0xff8822e8, 0x11ef1030, 0xff8822ec, 0xc00000b1, 0xff8822f0, 0x1cef0030, 0xff8822f4, 0xc0000060, 0xff8822f8, 0x1cef1030,
  0xff8822fc, 0x1c670200, 0xff882300, 0x84100308, 0xff882304, 0xdffff520, 0xff882308, 0x0001800b, 0xff88230c, 0x83080004, 0xff882310, 0xa2c80128,
  0xff882314, 0xa2b80a06, 0xff882318, 0xffffffa0, 0xff88231c, 0x84100308, 0xff882320, 0x801004ac, 0xff882324, 0x21ed0014, 0xff882328, 0xdffff406,
  0xff88232c, 0x83fa0e00, 0xff882330, 0x0de27f3c, 0xff882334, 0xc0000155, 0xff882338, 0x1cef0100, 0xff88233c, 0x83fa0e00, 0xff882340, 0x463b8001,
  0xff882344, 0x85880012, 0xff882348, 0x21e38002, 0xff88234c, 0xc0000046, 0xff882350, 0x1cef0004, 0xff882354, 0x84100308, 0xff882358, 0x83fa0e00,
  0xff88235c, 0x0de30800, 0xff882360, 0xc0000275, 0xff882364, 0x1cef0200, 0xff882368, 0x83f81200, 0xff88236c, 0x1023000f, 0xff882370, 0x40258001,
  0xff882374, 0x0023000c, 0xff882378, 0x11ef0100, 0xff88237c, 0xc0000041, 0xff882380, 0x00230004, 0xff882384, 0xa3e80126, 0xff882388, 0x83f81200,
  0xff88238c, 0x46258001, 0xff882390, 0x1023003f, 0xff882394, 0x85880013, 0xff882398, 0x21e38002, 0xff88239c, 0xc0000066, 0xff8823a0, 0x1cef0004,
  0xff8823a4, 0xc0000040, 0xff8823a8, 0x04ef0004, 0xff8823ac, 0x841003b4, 0xff8823b0, 0xdfffefc0, 0xff8823b4, 0x202c8127, 0xff8823b8, 0x46258001,
  0xff8823bc, 0x50038000, 0xff8823c0, 0x0001800b, 0xff8823c4, 0x83080004, 0xff8823c8, 0xa2b80a06, 0xff8823cc, 0xffffffc0, 0xff8823d0, 0x21ec8127,
  0xff8823d4, 0xc00000a5, 0xff8823d8, 0x841003b4, 0xff8823dc, 0xdfffee60, 0xff8823e0, 0x19bf8002, 0xff8823e4, 0x1cef0800, 0xff8823e8, 0xbe880000,
  0xff8823ec, 0x81080002, 0xff8823f0, 0x1001001f, 0xff8823f4, 0xd80021f0, 0xff8823f8, 0x83880007, 0xff8823fc, 0x00230001, 0xff882400, 0xa3880007,
  0xff882404, 0x1cef0001, 0xff882408, 0x8410040c, 0xff88240c, 0x0001800b, 0xff882410, 0x9f080004, 0xff882414, 0xffffffe0, 0xff882418, 0xdfffec80,
  0xff88241c, 0x11ef0001, 0xff882420, 0xc00008d1, 0xff882424, 0x3a388123, 0xff882428, 0x85e80124, 0xff88242c, 0x802005a0, 0xff882430, 0x21e50040,
  0xff882434, 0xc0000096, 0xff882438, 0x80100014, 0xff88243c, 0x10230004, 0xff882440, 0x00038000, 0xff882444, 0x00457ffc, 0xff882448, 0x83980000,
  0xff88244c, 0x00230001, 0xff882450, 0xa3980000, 0xff882454, 0x21e10000, 0xff882458, 0xc0000718, 0xff88245c, 0xa4880000, 0xff882460, 0x11ef1000,
  0xff882464, 0xc00002d1, 0xff882468, 0xd8002190, 0xff88246c, 0x81800064, 0xff882470, 0x00590080, 0xff882474, 0x102700ff, 0xff882478, 0xa2581200,
  0xff88247c, 0x1001003f, 0xff882480, 0xa1800064, 0xff882484, 0xd80021f0, 0xff882488, 0x8188000e, 0xff88248c, 0x10010001, 0xff882490, 0xc0000841,
  0xff882494, 0x8388000f, 0xff882498, 0x85880010, 0xff88249c, 0x04238000, 0xff8824a0, 0x04418002, 0xff8824a4, 0x1c238002, 0xff8824a8, 0xa388000f,
  0xff8824ac, 0x81e80107, 0xff8824b0, 0x83e80108, 0xff8824b4, 0xa3f80000, 0xff8824b8, 0xc0000700, 0xff8824bc, 0xa6880001, 0xff8824c0, 0x11ef0004,
  0xff8824c4, 0xc0000101, 0xff8824c8, 0x005901c0, 0xff8824cc, 0x8180007d, 0xff8824d0, 0xa6880001, 0xff8824d4, 0xa6581200, 0xff8824d8, 0x1001003f,
  0xff8824dc, 0xa180007d, 0xff8824e0, 0xc00000e0, 0xff8824e4, 0x00590180, 0xff8824e8, 0x8180007c, 0xff8824ec, 0xa6880001, 0xff8824f0, 0xa6581200,
  0xff8824f4, 0x1001003f, 0xff8824f8, 0xa180007c, 0xff8824fc, 0x100f000c, 0xff882500, 0x8388000e, 0xff882504, 0x80080170, 0xff882508, 0x10038000,
  0xff88250c, 0xc0000461, 0xff882510, 0x8388000f, 0xff882514, 0x85880010, 0xff882518, 0x04238000, 0xff88251c, 0x04418002, 0xff882520, 0x1c238002,
  0xff882524, 0xa388000f, 0xff882528, 0x81e80107, 0xff88252c, 0x83e80108, 0xff882530, 0xa3f80000, 0xff882534, 0xc0000320, 0xff882538, 0xc000018d,
  0xff88253c, 0x11ef0001, 0xff882540, 0xc0000051, 0xff882544, 0xbe880000, 0xff882548, 0xd8002190, 0xff88254c, 0x81800064, 0xff882550, 0x00590080,
  0xff882554, 0x102700ff, 0xff882558, 0xa2581200, 0xff88255c, 0x1001003f, 0xff882560, 0xa1800064, 0xff882564, 0xd80021f0, 0xff882568, 0x8188000e,
  0xff88256c, 0x10010010, 0xff882570, 0xc0000141, 0xff882574, 0x8388000f, 0xff882578, 0x85880010, 0xff88257c, 0x04238000, 0xff882580, 0x04418002,
  0xff882584, 0x1c238002, 0xff882588, 0xa388000f, 0xff88258c, 0x81e80107, 0xff882590, 0x83e80108, 0xff882594, 0xa3f80000, 0xff882598, 0x8108000a,
  0xff88259c, 0x10010180, 0xff8825a0, 0xdfffdc01, 0xff8825a4, 0x11e10100, 0xff8825a8, 0xc00000a1, 0xff8825ac, 0x83880008, 0xff8825b0, 0x00230001,
  0xff8825b4, 0xa3880008, 0xff8825b8, 0xc0000200, 0xff8825bc, 0x8384001a, 0xff8825c0, 0x00230001, 0xff8825c4, 0xa384001a, 0xff8825c8, 0x8188000e,
  0xff8825cc, 0x10010040, 0xff8825d0, 0xc0000141, 0xff8825d4, 0x8388000f, 0xff8825d8, 0x85880010, 0xff8825dc, 0x04238000, 0xff8825e0, 0x04418002,
  0xff8825e4, 0x1c238002, 0xff8825e8, 0xa388000f, 0xff8825ec, 0x81e80107, 0xff8825f0, 0x83e80108, 0xff8825f4, 0xa3f80000, 0xff8825f8, 0x83080000,
  0xff8825fc, 0x04432000, 0xff882600, 0xa5080000, 0xff882604, 0x1c010040, 0xff882608, 0xa108000a, 0xff88260c, 0xa3080000, 0xff882610, 0xdfffd880,
  0xff882614, 0xbbe80174, 0xff882618, 0xbde80175, 0xff88261c, 0xa1e80176, 0xff882620, 0x1dfe817d, 0xff882624, 0xc0000081, 0xff882628, 0x80100001,
  0xff88262c, 0xa148000c, 0xff882630, 0xc0000160, 0xff882634, 0x80100001, 0xff882638, 0xa148000c, 0xff88263c, 0x8188001a, 0xff882640, 0xa1e80177,
  0xff882644, 0x8188001b, 0xff882648, 0x0de08177, 0xff88264c, 0xc0000095, 0xff882650, 0x8010004b, 0xff882654, 0xa1480006, 0xff882658, 0xc0000060,
  0xff88265c, 0x80101811, 0xff882660, 0xa108000c, 0xff882664, 0x81e80176, 0xff882668, 0x9de80175, 0xff88266c, 0x9be80174, 0xff882670, 0xd4000010,
  0xff882674, 0x8388001a, 0xff882678, 0x8188001b, 0xff88267c, 0x0c238000, 0xff882680, 0x11e27e69, 0xff882684, 0xc0000101, 0xff882688, 0x46118000,
  0xff88268c, 0x00390100, 0xff882690, 0xc0000010, 0xff882694, 0x86381200, 0xff882698, 0x1001003f, 0xff88269c, 0xa180006d, 0xff8826a0, 0xc00000e0,
  0xff8826a4, 0x100100ff, 0xff8826a8, 0x003900c0, 0xff8826ac, 0xc0000010, 0xff8826b0, 0x86381200, 0xff8826b4, 0x1001003f, 0xff8826b8, 0xa180006c,
  0xff8826bc, 0x100700ff, 0xff8826c0, 0x83e80102, 0xff8826c4, 0x54018001, 0xff8826c8, 0x00808101, 0xff8826cc, 0x8e100001, 0xff8826d0, 0x86880001,
  0xff8826d4, 0xba880002, 0xff8826d8, 0x00a90020, 0xff8826dc, 0x1c5fe004, 0xff8826e0, 0xc00010e5, 0xff8826e4, 0x500507ff, 0xff8826e8, 0x4c01003c,
  0xff8826ec, 0xa1080005, 0xff8826f0, 0x18df800f, 0xff8826f4, 0xa1e8017a, 0xff8826f8, 0x82b80a06, 0xff8826fc, 0xa3080003, 0xff882700, 0x80100a78,
  0xff882704, 0xa1e80179, 0xff882708, 0x82b80a06, 0xff88270c, 0xa3080003, 0xff882710, 0x83e8011a, 0xff882714, 0xa3e8017c, 0xff882718, 0x82b80a06,
  0xff88271c, 0xa3080003, 0xff882720, 0x80100810, 0xff882724, 0x10447e41, 0xff882728, 0x82b80a06, 0xff88272c, 0xa3080003, 0xff882730, 0xa1e80178,
  0xff882734, 0xc8000010, 0xff882738, 0x82b80a06, 0xff88273c, 0xa3080003, 0xff882740, 0x8e100001, 0xff882744, 0xbfe8017d, 0xff882748, 0x82b80a06,
  0xff88274c, 0xa3080003, 0xff882750, 0x80100a78, 0xff882754, 0xa1e80179, 0xff882758, 0x82b80a06, 0xff88275c, 0xa3080003, 0xff882760, 0x83e8011a,
  0xff882764, 0xa3e8017c, 0xff882768, 0x82b80a06, 0xff88276c, 0xa3080003, 0xff882770, 0x85e8017a, 0xff882774, 0x10447e2d, 0xff882778, 0x80100810,
  0xff88277c, 0xa1e80178, 0xff882780, 0x1c7e8180, 0xff882784, 0xc00000e5, 0xff882788, 0xbfe80180, 0xff88278c, 0x80100004, 0xff882790, 0x1c3e8181,
  0xff882794, 0xc00001b5, 0xff882798, 0x1c3e8182, 0xff88279c, 0xc0000160, 0xff8827a0, 0x1c7e8181, 0xff8827a4, 0xc00000a5, 0xff8827a8, 0xbfe80181,
  0xff8827ac, 0x80100002, 0xff8827b0, 0x1c3e8182, 0xff8827b4, 0xc00000a0, 0xff8827b8, 0x1c7e8182, 0xff8827bc, 0xbfe80182, 0xff8827c0, 0x80100001,
  0xff8827c4, 0x183f800f, 0xff8827c8, 0x1c00817e, 0xff8827cc, 0xa1e8017e, 0xff8827d0, 0xc000007b, 0xff8827d4, 0xbfe8017f, 0xff8827d8, 0xc8000010,
  0xff8827dc, 0x100300ff, 0xff8827e0, 0x83e80102, 0xff8827e4, 0x54018001, 0xff8827e8, 0x00008101, 0xff8827ec, 0xa1e8017f, 0xff8827f0, 0xc8000010,
  0xff8827f4, 0x8410085c, 0xff8827f8, 0x80880000, 0xff8827fc, 0x11e07e0b, 0xff882800, 0xc00000b1, 0xff882804, 0xa1e8017a, 0xff882808, 0xc000082a,
  0xff88280c, 0xa1080005, 0xff882810, 0x8410087c, 0xff882814, 0x82b80a06, 0xff882818, 0xa3080003, 0xff88281c, 0xa5e80178, 0xff882820, 0x82b80a06,
  0xff882824, 0xa3080003, 0xff882828, 0x82b80a06, 0xff88282c, 0xa3080003, 0xff882830, 0x82b80a06, 0xff882834, 0xa3080003, 0xff882838, 0x85080009,
  0xff88283c, 0x00457fec, 0xff882840, 0x81480009, 0xff882844, 0x8348000a, 0xff882848, 0x21e58001, 0xff88284c, 0xa4880006, 0xff882850, 0x0001818b,
  0xff882854, 0xa0880007, 0xff882858, 0xc8000010, 0xff88285c, 0x80880000, 0xff882860, 0x11e07df2, 0xff882864, 0xc00000d1, 0xff882868, 0xa1e8017a,
  0xff88286c, 0xc000050a, 0xff882870, 0xa1080005, 0xff882874, 0x8410087c, 0xff882878, 0xa5e80178, 0xff88287c, 0x50010010, 0xff882880, 0x204c817c,
  0xff882884, 0x50058000, 0xff882888, 0x00010610, 0xff88288c, 0x82b80a06, 0xff882890, 0x01bbe800, 0xff882894, 0xa3080003, 0xff882898, 0xc0000000,
  0xff88289c, 0x82b80a06, 0xff8828a0, 0xa3080003, 0xff8828a4, 0x82b80a06, 0xff8828a8, 0xa3080003, 0xff8828ac, 0x82b80a06, 0xff8828b0, 0xa3080003,
  0xff8828b4, 0x21ec817a, 0xff8828b8, 0xc0000086, 0xff8828bc, 0x84100928, 0xff8828c0, 0xa5e80178, 0xff8828c4, 0xc8000010, 0xff8828c8, 0x21ec817c,
  0xff8828cc, 0xc0000056, 0xff8828d0, 0xc8000010, 0xff8828d4, 0x80b80006, 0xff8828d8, 0x100100ff, 0xff8828dc, 0x83e80102, 0xff8828e0, 0x54018001,
  0xff8828e4, 0x00a08101, 0xff8828e8, 0x20ad8005, 0xff8828ec, 0x00237ffc, 0xff8828f0, 0x0002817c, 0xff8828f4, 0xa1e8017c, 0xff8828f8, 0xc8000010,
  0xff8828fc, 0x18ff800f, 0xff882900, 0x8210004b, 0xff882904, 0xa3480006, 0xff882908, 0xc0000100, 0xff88290c, 0x81080001, 0xff882910, 0x1c00810d,
  0xff882914, 0xa1080001, 0xff882918, 0x82101811, 0xff88291c, 0xa308000d, 0xff882920, 0x84100a5c, 0xff882924, 0xa5e80179, 0xff882928, 0x80100001,
  0xff88292c, 0xa108000d, 0xff882930, 0x8108000c, 0xff882934, 0xc8000010, 0xff882938, 0x10031810, 0xff88293c, 0xc00000b1, 0xff882940, 0x1c231810,
  0xff882944, 0xa308000a, 0xff882948, 0x9be80179, 0xff88294c, 0x18df800f, 0xff882950, 0x82101811, 0xff882954, 0xa308000d, 0xff882958, 0x11e10010,
  0xff88295c, 0xc0000291, 0xff882960, 0x11e11000, 0xff882964, 0xc00004b1, 0xff882968, 0x82100020, 0xff88296c, 0xa308000a, 0xff882970, 0x1c3e817d,
  0xff882974, 0xc00000b5, 0xff882978, 0x8188000a, 0xff88297c, 0x00010001, 0xff882980, 0xa188000a, 0xff882984, 0xc0000480, 0xff882988, 0x8188000b,
  0xff88298c, 0x00010001, 0xff882990, 0xa188000b, 0xff882994, 0x21e3000f, 0xff882998, 0xc00003e8, 0xff88299c, 0x1c667da4, 0xff8829a0, 0x84100a94,
  0xff8829a4, 0xa5e80179, 0xff8829a8, 0xc8000010, 0xff8829ac, 0x8184001b, 0xff8829b0, 0x00010001, 0xff8829b4, 0xa184001b, 0xff8829b8, 0x1c667d9e,
  0xff8829bc, 0x8188000e, 0xff8829c0, 0x10010040, 0xff8829c4, 0xc0000141, 0xff8829c8, 0x8388000f, 0xff8829cc, 0x85880010, 0xff8829d0, 0x04238000,
  0xff8829d4, 0x04418002, 0xff8829d8, 0x1c238002, 0xff8829dc, 0xa388000f, 0xff8829e0, 0x81e80107, 0xff8829e4, 0x83e80108, 0xff8829e8, 0xa3f80000,
  0xff8829ec, 0x84100a94, 0xff8829f0, 0xa5e80179, 0xff8829f4, 0xc8000010, 0xff8829f8, 0x8188000c, 0xff8829fc, 0x00010001, 0xff882a00, 0xa188000c,
  0xff882a04, 0x1c672000, 0xff882a08, 0x84100a94, 0xff882a0c, 0xa5e80179, 0xff882a10, 0xc8000010, 0xff882a14, 0x84100a20, 0xff882a18, 0xa5e80179,
  0xff882a1c, 0xc8000010, 0xff882a20, 0x6c22817d, 0xff882a24, 0xa3e8017d, 0xff882a28, 0x5023000b, 0xff882a2c, 0x8548000a, 0xff882a30, 0x46038002,
  0xff882a34, 0x28230001, 0xff882a38, 0x6e238001, 0xff882a3c, 0x10038000, 0xff882a40, 0x83e80121, 0xff882a44, 0x54038000, 0xff882a48, 0x841006d8,
  0xff882a4c, 0xa5e80178, 0xff882a50, 0x4c01004b, 0xff882a54, 0xa1480006, 0xff882a58, 0xc8000010, 0xff882a5c, 0x83080001, 0xff882a60, 0x04227d75,
  0xff882a64, 0xa3080001, 0xff882a68, 0xba880002, 0xff882a6c, 0x40590006, 0xff882a70, 0x1c658003, 0xff882a74, 0xc0000100, 0xff882a78, 0x81880009,
  0xff882a7c, 0x00010001, 0xff882a80, 0xa1880009, 0xff882a84, 0xba880002, 0xff882a88, 0x1dfe817d, 0xff882a8c, 0xc0000045, 0xff882a90, 0x1c671000,
  0xff882a94, 0x11e70100, 0xff882a98, 0xc00002a1, 0xff882a9c, 0xba880002, 0xff882aa0, 0x81800074, 0xff882aa4, 0x00390140, 0xff882aa8, 0xa6880001,
  0xff882aac, 0xa6381200, 0xff882ab0, 0x1001003f, 0xff882ab4, 0xa1800074, 0xff882ab8, 0x8188000e, 0xff882abc, 0x10010020, 0xff882ac0, 0xc0000261,
  0xff882ac4, 0x8388000f, 0xff882ac8, 0x85880010, 0xff882acc, 0x04238000, 0xff882ad0, 0x04418002, 0xff882ad4, 0x1c238002, 0xff882ad8, 0xa388000f,
  0xff882adc, 0x83e80109, 0xff882ae0, 0x81e8010a, 0xff882ae4, 0xa1f80001, 0xff882ae8, 0xc0000120, 0xff882aec, 0xba880002, 0xff882af0, 0x81800064,
  0xff882af4, 0x00390080, 0xff882af8, 0x106700ff, 0xff882afc, 0xa6381200, 0xff882b00, 0x1001003f, 0xff882b04, 0xa1800064, 0xff882b08, 0x187f800f,
  0xff882b0c, 0x1c9e817f, 0xff882b10, 0xc0000181, 0xff882b14, 0x82101811, 0xff882b18, 0xa308000c, 0xff882b1c, 0x00a90020, 0xff882b20, 0x1c5fe004,
  0xff882b24, 0xdfffeec5, 0xff882b28, 0xa5e8017a, 0xff882b2c, 0x500507ff, 0xff882b30, 0xa1080005, 0xff882b34, 0x84100738, 0xff882b38, 0xa5e80178,
  0xff882b3c, 0xdfffb100, 0xff882b40, 0x18ff800f, 0xff882b44, 0x84100674, 0xff882b48, 0xa5e80178, 0xff882b4c, 0xbfe8017d, 0xff882b50, 0x8188001a,
  0xff882b54, 0x8388001b, 0xff882b58, 0x0de18001, 0xff882b5c, 0xc0000085, 0xff882b60, 0x82101811, 0xff882b64, 0xa308000c, 0xff882b68, 0xdfffafa0,
  0xff882b6c, 0x8010004b, 0xff882b70, 0xa1480006, 0xff882b74, 0xc8000010, 0xff882b78, 0x83880011, 0xff882b7c, 0x11e30001, 0xff882b80, 0xc0000111,
  0xff882b84, 0x8348000c, 0xff882b88, 0x10230002, 0xff882b8c, 0xdfffb121, 0xff882b90, 0xa348000c, 0xff882b94, 0x4024811d, 0xff882b98, 0xc0000010,
  0xff882b9c, 0x82280111, 0xff882ba0, 0xa3f80000, 0xff882ba4, 0x80100a28, 0xff882ba8, 0xa1480007, 0xff882bac, 0x80100bb8, 0xff882bb0, 0xa1e8011c,
  0xff882bb4, 0xdfffafe0, 0xff882bb8, 0x8348000c, 0xff882bbc, 0x10230002, 0xff882bc0, 0xdfffaf81, 0xff882bc4, 0xa348000c, 0xff882bc8, 0x82100a28,
  0xff882bcc, 0x81f80000, 0xff882bd0, 0x11e10001, 0xff882bd4, 0xc00006b1, 0xff882bd8, 0x82100b78, 0xff882bdc, 0xa3e8011c, 0xff882be0, 0x83880011,
  0xff882be4, 0x11e30001, 0xff882be8, 0xc0000061, 0xff882bec, 0xa1880011, 0xff882bf0, 0xc00005a0, 0xff882bf4, 0xa1880017, 0xff882bf8, 0x4024811d,
  0xff882bfc, 0x01a30c04, 0xff882c00, 0x82280117, 0xff882c04, 0xc0000060, 0xff882c08, 0xc0000140, 0xff882c0c, 0xc0000320, 0xff882c10, 0xa3e8011d,
  0xff882c14, 0x83a9fd05, 0xff882c18, 0x10008114, 0xff882c1c, 0x0de0811e, 0xff882c20, 0xc00003c5, 0xff882c24, 0xa1e8011e, 0xff882c28, 0x1c230004,
  0xff882c2c, 0xc0000360, 0xff882c30, 0xa3e8011d, 0xff882c34, 0x83a9fcfd, 0xff882c38, 0x10008115, 0xff882c3c, 0x0de0811f, 0xff882c40, 0xc0000105,
  0xff882c44, 0xa1e8011f, 0xff882c48, 0x1c230002, 0xff882c4c, 0xd8002190, 0xff882c50, 0x81080001, 0xff882c54, 0x0c007cf5, 0xff882c58, 0xa1080001,
  0xff882c5c, 0xd80021f0, 0xff882c60, 0x41e4811d, 0xff882c64, 0xc00001a5, 0xff882c68, 0x81880017, 0xff882c6c, 0xc0000060, 0xff882c70, 0xa3e8011d,
  0xff882c74, 0x83a9fced, 0xff882c78, 0x10008116, 0xff882c7c, 0x0de08120, 0xff882c80, 0xc00000c5, 0xff882c84, 0xa1e80120, 0xff882c88, 0x1c230001,
  0xff882c8c, 0x81e80121, 0xff882c90, 0x0c011600, 0xff882c94, 0xa1e80121, 0xff882c98, 0x81880016, 0xff882c9c, 0x0c038000, 0xff882ca0, 0xa1880016,
  0xff882ca4, 0x83e80110, 0xff882ca8, 0xa3480007, 0xff882cac, 0xdfffa820 };

/*! \} *//* End of group ETH_XPIC */

#endif /* DRV_ETH_MODULE_DISABLED */

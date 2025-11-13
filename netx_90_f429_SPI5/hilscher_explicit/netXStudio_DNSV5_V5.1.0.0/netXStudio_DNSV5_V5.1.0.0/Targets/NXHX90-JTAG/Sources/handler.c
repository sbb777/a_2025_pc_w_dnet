/*
 * handler.c
 *
 *  Created on: 29.04.2019
 *      Author: AndreGross
 */

#include "netx_drv.h"

#define FAULT_TRAP(); __BKPT(0);while(1)

void BusFault_InlineHandler(void)
{
  uintptr_t pFaultPointer = 0;
  bool bBFARVALID = false;
  bool bSTKERR = false;
  bool bUNSTKERR = false;
  bool bIMPRECISERR = false;
  bool bPRECISERR = false;
  bool bIBUSERR = false;
  if(SCB->CFSR & SCB_CFSR_BFARVALID_Msk)
  {
    // Store to pFaultPointer if Bus Fault Address is Valid
    bBFARVALID = true;
    pFaultPointer = SCB->BFAR;
  }
  if(SCB->CFSR & SCB_CFSR_STKERR_Msk)
  {
    // Error in storing to the stack pointer
    bSTKERR = true;
  }
  if(SCB->CFSR & SCB_CFSR_UNSTKERR_Msk)
  {
    // Error in loading from the stack pointer
    bUNSTKERR = true;
  }
  if(SCB->CFSR & SCB_CFSR_IMPRECISERR_Msk)
  {
    // Error in data fetching without BFARVALID
    bIMPRECISERR = true;
  }
  if(SCB->CFSR & SCB_CFSR_PRECISERR_Msk)
  {
    // Error in data fetching with BFARVALID
    bPRECISERR = true;
  }
  if(SCB->CFSR & SCB_CFSR_IBUSERR_Msk)
  {
    // Error in instruction fetching with BFARVALID
    bIBUSERR = true;
  }
  UNUSED(bBFARVALID);
  UNUSED(pFaultPointer);
  UNUSED(bSTKERR);
  UNUSED(bUNSTKERR);
  UNUSED(bIMPRECISERR);
  UNUSED(bPRECISERR);
  UNUSED(bIBUSERR);
  __BKPT(0);
}

void MemManage_InlineHandler(void)
{
  // MPU Fault Handler
  uintptr_t pFaultPointer = 0;
  bool bMMARVALID = false;
  bool bMSTKERR = false;
  bool bMUNSTKERR = false;
  bool bDACCVIOL = false;
  bool bIACCVIOL = false;
  if(SCB->CFSR & SCB_CFSR_MMARVALID_Msk)
  {
    // Store to pFaultPointer if Mem Manage Fault Address is Valid
    bMMARVALID = true;
    pFaultPointer = SCB->MMFAR;
  }
  if(SCB->CFSR & SCB_CFSR_MSTKERR_Msk)
  {
    // Error in storing to the stack pointer
    bMSTKERR = true;
  }
  if(SCB->CFSR & SCB_CFSR_MUNSTKERR_Msk)
  {
    // Error in loading from the stack pointer
    bMUNSTKERR = true;
  }
  if(SCB->CFSR & SCB_CFSR_DACCVIOL_Msk)
  {
    // Error in data fetching without MMARVALID
    bDACCVIOL = true;
  }
  if(SCB->CFSR & SCB_CFSR_IACCVIOL_Msk)
  {
    // Error in instruction fetching with MMARVALID
    bIACCVIOL = true;
  }
  UNUSED(bMMARVALID);
  UNUSED(pFaultPointer);
  UNUSED(bMSTKERR);
  UNUSED(bMUNSTKERR);
  UNUSED(bDACCVIOL);
  UNUSED(bIACCVIOL);
  __BKPT(0);
}

void UsageFault_InlineHandler(void)
{
  // MPU Fault Handler
  bool bDIVBYZERO = false;
  bool bUNALIGNED = false;
  bool bNOCP = false;
  bool bINVPC = false;
  bool bINVSTATE = false;
  bool bUNDEFINSTR = false;
  if(SCB->CFSR & SCB_CFSR_DIVBYZERO_Msk)
  {
    // Error in storing to the stack pointer
    bDIVBYZERO = true;
  }
  if(SCB->CFSR & SCB_CFSR_UNALIGNED_Msk)
  {
    // Error in loading from the stack pointer
    bUNALIGNED = true;
  }
  if(SCB->CFSR & SCB_CFSR_NOCP_Msk)
  {
    // Error in data fetching without MMARVALID
    bNOCP = true;
  }
  if(SCB->CFSR & SCB_CFSR_INVPC_Msk)
  {
    // Error in instruction fetching with MMARVALID
    bINVPC = true;
  }
  if(SCB->CFSR & SCB_CFSR_INVSTATE_Msk)
  {
    // Error in instruction fetching with MMARVALID
    bINVSTATE = true;
  }
  if(SCB->CFSR & SCB_CFSR_UNDEFINSTR_Msk)
  {
    // Error in instruction fetching with MMARVALID
    bUNDEFINSTR = true;
  }
  UNUSED(bDIVBYZERO);
  UNUSED(bUNALIGNED);
  UNUSED(bNOCP);
  UNUSED(bINVPC);
  UNUSED(bINVSTATE);
  UNUSED(bUNDEFINSTR);
  __BKPT(0);
}

void NMI_Handler(void)
{
  FAULT_TRAP();
}

void HardFault_Handler(void)
{
  if(SCB->HFSR & SCB_HFSR_DEBUGEVT_Msk)
  {
    // Debugging related hard fault
    FAULT_TRAP();
  }
  if(SCB->HFSR & SCB_HFSR_VECTTBL_Msk)
  {
    // Debugging related reset vector read
    FAULT_TRAP();
  }
  if(SCB->HFSR & SCB_HFSR_FORCED_Msk)
  {
    // A configurable fault was received but was not able to
    // be serviced
    if(SCB->CFSR & SCB_CFSR_USGFAULTSR_Msk)
    {
      UsageFault_InlineHandler();
    }
    if(SCB->CFSR & SCB_CFSR_BUSFAULTSR_Msk)
    {
      BusFault_InlineHandler();
    }
    if(SCB->CFSR & SCB_CFSR_MEMFAULTSR_Msk)
    {
      MemManage_InlineHandler();
    }
  }
  FAULT_TRAP();
}

void MemManage_Handler(void)
{
  MemManage_InlineHandler();
  FAULT_TRAP();
}

void BusFault_Handler(void)
{
  BusFault_InlineHandler();
  FAULT_TRAP();
}

void UsageFault_Handler(void)
{
  UsageFault_InlineHandler();
  FAULT_TRAP();
}

void SVC_Handler(void)
{
  // This handler is raised by the SVC instruction
  FAULT_TRAP();
}

void DebugMon_Handler(void)
{
  if(SCB->DFSR & SCB_DFSR_BKPT_Msk)
  {
    __BKPT(0);
  }
  if(SCB->DFSR & SCB_DFSR_DWTTRAP_Msk)
  {
    __BKPT(0);
  }
  if(SCB->DFSR & SCB_DFSR_EXTERNAL_Msk)
  {
    __BKPT(0);
  }
  if(SCB->DFSR & SCB_DFSR_HALTED_Msk)
  {
    __BKPT(0);
  }
  if(SCB->DFSR & SCB_DFSR_VCATCH_Msk)
  {
    __BKPT(0);
  }
  __BKPT(0);
}

void PendSV_Handler(void)
{
  // This handler is raised by the SVC
  FAULT_TRAP();
}

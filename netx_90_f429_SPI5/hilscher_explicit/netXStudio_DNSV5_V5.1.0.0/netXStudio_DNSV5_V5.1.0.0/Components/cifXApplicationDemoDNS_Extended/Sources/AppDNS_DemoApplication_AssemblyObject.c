/***************************************************************************************************
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
***************************************************************************************************/

/***************************************************************************************************
*
* Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
*
***************************************************************************************************/
/* Includes */
#include "Hil_Results.h"
#include "DNS_Includes.h"
#include "AppDNS_DemoApplication.h"


/*************************************************************************************************/
/* Assembly Directory Entry                                                                      */
/*************************************************************************************************/
typedef struct CIP_ASSEMBLY_DIR_ENTRY_Ttag
{
  uint32_t ulInstance;
  uint32_t ulSize;
  uint32_t ulOffset;
  uint32_t ulFlags;
} CIP_ASSEMBLY_DIR_ENTRY_T;


/*************************************************************************************************/
/* List of additional assembly instances that shall be registered                                */
/*************************************************************************************************/
#define SUPPORT_ADD_AS_OBJECTS

#define DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE_102  102
#define DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE_103  103

#define APP_PROCESS_DATA_INPUT_SIZE   6
#define APP_PROCESS_DATA_OUTPUT_SIZE  10

#define DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE_102_SIZE       APP_PROCESS_DATA_INPUT_SIZE-2     /* 6-2 Byte */
#define DNS_DEMO_PRODUCING_ASSEMBLY_INSTANCE_103_SIZE       APP_PROCESS_DATA_OUTPUT_SIZE-2-2  /* 10-2-2 Byte */



#ifdef SUPPORT_ADD_AS_OBJECTS
CIP_ASSEMBLY_DIR_ENTRY_T s_atCipAssemblyDir[] =
{
  /* Instance,                                 Size,                                      Offset,    Flags */
   { DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE_102, DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE_102_SIZE, 0,  DNS_AS_TYPE_CONSUME },
   { DNS_DEMO_CONSUMING_ASSEMBLY_INSTANCE_103, DNS_DEMO_PRODUCING_ASSEMBLY_INSTANCE_103_SIZE, 2,  DNS_AS_TYPE_PRODUCE },
};

/* Number of assembly instances */
uint32_t s_ulCipAssemblyDirEntries = sizeof(s_atCipAssemblyDir) / sizeof(s_atCipAssemblyDir[0]);

/* Pointer to assembly list */
CIP_ASSEMBLY_DIR_ENTRY_T* s_ptCipAssemblyDir = &s_atCipAssemblyDir[0];

#else
/* Number of assembly instances */
uint32_t s_ulCipAssemblyDirEntries = 0;

/* Pointer to assembly list */
CIP_ASSEMBLY_DIR_T* s_ptCipAssemblyDir = NULL;

#endif

/***************************************************************************************************
*! AppDNS_RegisterAssembly.
*  This function is called to register one additional Assembly Object to the stack.
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
uint32_t AppDNS_RegisterAssembly(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
                                 uint32_t                       ulInstance,
                                 uint32_t                       ulSize,
                                 uint32_t                       ulOffset,
                                 uint32_t                       ulFlags)
{
  uint32_t ulRet = CIFX_NO_ERROR;
  DNS_PACKET_CREATE_ASSEMBLY_REQ_T* ptReq = AppDNS_GlobalPacket_Init(ptDnsRsc);

  /* prepare packet to create a new assembly */
  ptReq->tData.ulInstance = ulInstance;
  ptReq->tData.ulSize = ulSize;
  ptReq->tData.ulOffset = ulOffset;
  ptReq->tData.ulFlags = ulFlags;

  /* Issue Create Assembly Request */
  ptReq->tHead.ulCmd = DNS_CMD_CREATE_ASSEMBLY_REQ;
  ptReq->tHead.ulLen = DNS_CREATE_ASSEMBLY_REQ_SIZE;

  ulRet = AppDNS_GlobalPacket_SendReceive(ptDnsRsc);
  return ulRet;
}

/***************************************************************************************************
*! AppDNS_AsObject_Registration.
*  This function is called once at startup to register additional Assembly Objects.
*   \param ptAppData   pointer to APP_DATA_T structure
***************************************************************************************************/
uint32_t AppDNS_AsObject_Registration(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  uint32_t ulRet = CIFX_NO_ERROR;
  uint32_t ulIdx = 0;

  /* Optional: Register additional assemblies if configured */
  for (ulIdx = 0; (ulIdx < s_ulCipAssemblyDirEntries) && (ulRet == 0); ulIdx++)
  {
      ulRet = AppDNS_RegisterAssembly(ptDnsRsc,
                                      s_ptCipAssemblyDir[ulIdx].ulInstance,
                                      s_ptCipAssemblyDir[ulIdx].ulSize,
                                      s_ptCipAssemblyDir[ulIdx].ulOffset,
                                      s_ptCipAssemblyDir[ulIdx].ulFlags);
  }

  return ulRet;
}


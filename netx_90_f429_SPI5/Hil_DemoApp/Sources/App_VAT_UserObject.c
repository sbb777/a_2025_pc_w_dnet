/**************************************************************************************
 * VAT User Object Implementation
 *
 * Implements VAT-specific CIP objects for Explicit Message communication
 * Based on Hilscher's AppDNS_DemoApplication_UserObject.c pattern
 **************************************************************************************/

#include "App_VAT_UserObject.h"
#include "App_VAT_Parameters.h"
#include "App_VAT_Diagnostic.h"
#include "App_DemoApplication.h"
#include "AppDNS_DemoApplication.h"
#include "DNS_Includes.h"
#include <string.h>
#include <stdio.h>

/***************************************************************************************************
 * VAT Object Definitions
 ***************************************************************************************************/

/* VAT Parameter Object (Class 0x64) */
#define VAT_CLASS_PARAMETER                     0x64
#define VAT_CLASS_PARAMETER_MAX_INST            99      /* 99 parameters */

/* VAT Diagnostic Object (Class 0x65) */
#define VAT_CLASS_DIAGNOSTIC                    0x65
#define VAT_CLASS_DIAGNOSTIC_MAX_INST           1       /* Single instance */

/***************************************************************************************************
 * Object Registration Directory
 ***************************************************************************************************/
typedef struct CIP_REGISTER_OBJ_DIR_ENTRY_Ttag
{
  uint32_t ulClass;
  uint32_t ulOptionFlags;
  uint32_t ulMaxInstance;
} CIP_REGISTER_OBJ_DIR_ENTRY_T;

static CIP_REGISTER_OBJ_DIR_ENTRY_T s_atVatCipRegisterObjDir[] =
{
  /* Class,                      OptionFlags,   MaxInstance */
  { VAT_CLASS_PARAMETER,         0x00000000,    VAT_CLASS_PARAMETER_MAX_INST  },
  { VAT_CLASS_DIAGNOSTIC,        0x00000000,    VAT_CLASS_DIAGNOSTIC_MAX_INST },
};

/* Number of VAT object classes to register */
static uint32_t s_ulVatCipRegisterObjDirEntries = sizeof(s_atVatCipRegisterObjDir) / sizeof(s_atVatCipRegisterObjDir[0]);

/* External references */
extern VAT_PARAM_MANAGER_T g_tParamManager;
extern VAT_DIAGNOSTIC_DATA_T g_tVatDiagnostics;
extern APP_DATA_T g_tAppData;

/***************************************************************************************************
 * Register a single VAT object at the stack
 ***************************************************************************************************/
static uint32_t AppDNS_VAT_RegisterObject(APP_DATA_T* ptAppData, uint32_t ulClass, uint32_t ulOptionFlags)
{
  uint32_t ulRet = CIFX_NO_ERROR;
  DNS_PACKET_REGISTER_CLASS_REQ_T* ptReq =
      (DNS_PACKET_REGISTER_CLASS_REQ_T*)AppDNS_GlobalPacket_Init(ptAppData);

  /* Prepare packet to register object */
  ptReq->tData.ulClass = ulClass;
  ptReq->tData.ulServiceMask = ulOptionFlags;

  /* Issue Register Object Request */
  ptReq->tHead.ulCmd = DNS_CMD_REGISTER_CLASS_REQ;
  ptReq->tHead.ulLen = DNS_REGISTER_CLASS_REQ_SIZE;

  ulRet = AppDNS_GlobalPacket_SendReceive(ptAppData);

  if (ulRet == CIFX_NO_ERROR) {
    printf("[VAT] Registered Object Class 0x%02X (MaxInst=%d)\r\n",
           (unsigned int)ulClass,
           (ulClass == VAT_CLASS_PARAMETER) ? VAT_CLASS_PARAMETER_MAX_INST : VAT_CLASS_DIAGNOSTIC_MAX_INST);
  } else {
    printf("[VAT] ERROR: Failed to register Class 0x%02X: 0x%08X\r\n",
           (unsigned int)ulClass, (unsigned int)ulRet);
  }

  return ulRet;
}

/***************************************************************************************************
 * Register all VAT objects at the stack at startup
 ***************************************************************************************************/
uint32_t AppDNS_VAT_UserObject_Registration(APP_DATA_T* ptAppData)
{
  uint32_t ulIdx = 0;
  uint32_t ulRet = 0;

  printf("[VAT] Registering VAT User Objects...\r\n");

  for (ulIdx = 0; (ulIdx < s_ulVatCipRegisterObjDirEntries) && (ulRet == 0); ulIdx++)
  {
    ulRet = AppDNS_VAT_RegisterObject(ptAppData,
                                      s_atVatCipRegisterObjDir[ulIdx].ulClass,
                                      s_atVatCipRegisterObjDir[ulIdx].ulOptionFlags);
  }

  if (ulRet == 0) {
    printf("[VAT] All VAT User Objects registered successfully\r\n");
  }

  return ulRet;
}

/***************************************************************************************************
 * Check if a requested object is a registered VAT user object
 ***************************************************************************************************/
bool AppDNS_VAT_UserObject_IsRegistered(uint32_t ulClass, uint32_t ulInstance)
{
  uint32_t ulIdx = 0;

  for (ulIdx = 0; ulIdx < s_ulVatCipRegisterObjDirEntries; ulIdx++)
  {
    if (ulClass == s_atVatCipRegisterObjDir[ulIdx].ulClass)
    {
      /* Instance must be >= 1 and <= MaxInstance */
      if (ulInstance >= 1 && ulInstance <= s_atVatCipRegisterObjDir[ulIdx].ulMaxInstance)
      {
        return true;
      }
      else
      {
        printf("[VAT] Invalid instance %u for Class 0x%02X (Max=%u)\r\n",
               (unsigned int)ulInstance,
               (unsigned int)ulClass,
               (unsigned int)s_atVatCipRegisterObjDir[ulIdx].ulMaxInstance);
        return false;
      }
    }
  }

  return false;
}

/***************************************************************************************************
 * VAT Parameter Object Handler (Class 0x64)
 ***************************************************************************************************/
static void AppDNS_VAT_HandleParameterObject(DNS_PACKET_CIP_SERVICE_IND_T* ptInd,
                                               DNS_PACKET_CIP_SERVICE_RES_T* ptRes,
                                               uint32_t* pulResDataSize,
                                               uint32_t* pulGRC,
                                               uint32_t* pulERC)
{
  uint32_t ulGRC = CIP_GRC_SUCCESS;
  uint32_t ulERC = 0;
  uint32_t ulResDataSize = 0;

  uint8_t bParamId = (uint8_t)ptInd->tData.ulInstance;
  uint8_t bService = (uint8_t)ptInd->tData.ulService;

  switch (bService)
  {
    case CIP_SERVICE_GET_ATTRIBUTE_SINGLE:
    {
      /* Read parameter value */
      uint8_t abData[32] = {0};
      uint8_t bSize = 0;

      int32_t lRet = VAT_Param_Get(&g_tParamManager, bParamId, abData, &bSize);

      if (lRet == 0)
      {
        /* Success - copy data to response */
        memcpy(ptRes->tData.abData, abData, bSize);
        ulResDataSize = bSize;
        ulGRC = CIP_GRC_SUCCESS;

        printf("[VAT Param] GET Param%u: ", bParamId);
        for (uint8_t i = 0; i < bSize; i++) {
          printf("%02X ", abData[i]);
        }
        printf("\r\n");
      }
      else
      {
        ulGRC = CIP_GRC_ATTRIBUTE_NOT_SUPPORTED;
        printf("[VAT Param] ERROR: Param%u not found\r\n", bParamId);
      }
    }
    break;

    case CIP_SERVICE_SET_ATTRIBUTE_SINGLE:
    {
      /* Write parameter value */
      uint32_t ulDataLength = ptInd->tHead.ulLen - DNS_CIP_SERVICE_IND_SIZE;

      if (ulDataLength == 0)
      {
        /* Reset to default */
        ulGRC = CIP_GRC_SUCCESS;
        printf("[VAT Param] RESET Param%u to default\r\n", bParamId);
      }
      else
      {
        int32_t lRet = VAT_Param_Set(&g_tParamManager, bParamId,
                                     ptInd->tData.abData, (uint8_t)ulDataLength);

        if (lRet == 0)
        {
          ulGRC = CIP_GRC_SUCCESS;

          printf("[VAT Param] SET Param%u: ", bParamId);
          for (uint32_t i = 0; i < ulDataLength; i++) {
            printf("%02X ", ptInd->tData.abData[i]);
          }
          printf("\r\n");
        }
        else if (lRet == -2)
        {
          /* Read-only */
          ulGRC = CIP_GRC_TOO_MUCH_DATA;
          printf("[VAT Param] ERROR: Param%u is READ-ONLY\r\n", bParamId);
        }
        else if (lRet == -3)
        {
          /* Invalid size */
          ulGRC = CIP_GRC_INVALID_ATTRIBUTE_VALUE;
          printf("[VAT Param] ERROR: Invalid size for Param%u\r\n", bParamId);
        }
        else if (lRet == -4)
        {
          /* Out of range */
          ulGRC = CIP_GRC_INVALID_ATTRIBUTE_VALUE;
          printf("[VAT Param] ERROR: Value out of range for Param%u\r\n", bParamId);
        }
        else
        {
          ulGRC = CIP_GRC_ATTRIBUTE_NOT_SUPPORTED;
          printf("[VAT Param] ERROR: Param%u not supported\r\n", bParamId);
        }
      }
    }
    break;

    default:
      ulGRC = CIP_GRC_SERVICE_NOT_SUPPORTED;
      printf("[VAT Param] ERROR: Service 0x%02X not supported\r\n", bService);
      break;
  }

  *pulGRC = ulGRC;
  *pulERC = ulERC;
  *pulResDataSize = ulResDataSize;
}

/***************************************************************************************************
 * VAT Diagnostic Object Handler (Class 0x65)
 ***************************************************************************************************/
static void AppDNS_VAT_HandleDiagnosticObject(DNS_PACKET_CIP_SERVICE_IND_T* ptInd,
                                                DNS_PACKET_CIP_SERVICE_RES_T* ptRes,
                                                uint32_t* pulResDataSize,
                                                uint32_t* pulGRC,
                                                uint32_t* pulERC)
{
  uint32_t ulGRC = CIP_GRC_SUCCESS;
  uint32_t ulERC = 0;
  uint32_t ulResDataSize = 0;

  uint8_t bAttribute = (uint8_t)ptInd->tData.ulAttribute;
  uint8_t bService = (uint8_t)ptInd->tData.ulService;

  switch (bService)
  {
    case CIP_SERVICE_GET_ATTRIBUTE_SINGLE:
    {
      uint8_t* pbSrc = NULL;
      uint8_t bSize = 0;

      /* Map attribute to diagnostic data */
      switch (bAttribute)
      {
        case 1:  /* Uptime */
          pbSrc = (uint8_t*)&g_tVatDiagnostics.ulUptimeSeconds;
          bSize = 4;
          break;

        case 2:  /* Total Cycles */
          pbSrc = (uint8_t*)&g_tVatDiagnostics.ulTotalCycles;
          bSize = 4;
          break;

        case 3:  /* Error Count */
          pbSrc = (uint8_t*)&g_tVatDiagnostics.usErrorCount;
          bSize = 2;
          break;

        case 4:  /* Last Error Code */
          pbSrc = (uint8_t*)&g_tVatDiagnostics.usLastErrorCode;
          bSize = 2;
          break;

        case 6:  /* Pressure Min */
          pbSrc = (uint8_t*)&g_tVatDiagnostics.sPressureMin;
          bSize = 2;
          break;

        case 7:  /* Pressure Max */
          pbSrc = (uint8_t*)&g_tVatDiagnostics.sPressureMax;
          bSize = 2;
          break;

        case 8:  /* Pressure Avg */
          pbSrc = (uint8_t*)&g_tVatDiagnostics.sPressureAvg;
          bSize = 2;
          break;

        case 14:  /* Temperature */
          pbSrc = (uint8_t*)&g_tVatDiagnostics.sTemperature;
          bSize = 2;
          break;

        case 15:  /* Firmware Version */
          pbSrc = (uint8_t*)&g_tVatDiagnostics.ulFirmwareVersion;
          bSize = 4;
          break;

        default:
          ulGRC = CIP_GRC_ATTRIBUTE_NOT_SUPPORTED;
          printf("[VAT Diag] ERROR: Attribute %u not supported\r\n", bAttribute);
          break;
      }

      if (pbSrc != NULL)
      {
        memcpy(ptRes->tData.abData, pbSrc, bSize);
        ulResDataSize = bSize;

        printf("[VAT Diag] GET Attr%u: ", bAttribute);
        for (uint8_t i = 0; i < bSize; i++) {
          printf("%02X ", ptRes->tData.abData[i]);
        }
        printf("\r\n");
      }
    }
    break;

    case CIP_SERVICE_RESET:
    {
      /* Reset diagnostic statistics */
      VAT_Diagnostic_Reset();
      ulGRC = CIP_GRC_SUCCESS;
      printf("[VAT Diag] Statistics RESET\r\n");
    }
    break;

    default:
      ulGRC = CIP_GRC_SERVICE_NOT_SUPPORTED;
      printf("[VAT Diag] ERROR: Service 0x%02X not supported\r\n", bService);
      break;
  }

  *pulGRC = ulGRC;
  *pulERC = ulERC;
  *pulResDataSize = ulResDataSize;
}

/***************************************************************************************************
 * VAT User Object Indication Handler - Main Entry Point
 *
 * This function handles all incoming CIP requests directed to VAT objects
 * that have been registered by the host application.
 ***************************************************************************************************/
void AppDNS_VAT_UserObject_Indication(APP_DATA_T* ptAppData,
                                       DNS_PACKET_CIP_SERVICE_IND_T* ptInd,
                                       DNS_PACKET_CIP_SERVICE_RES_T* ptRes,
                                       uint32_t* pulResDataSize)
{
  uint32_t ulGRC = CIP_GRC_SUCCESS;
  uint32_t ulERC = 0;
  uint32_t ulResDataSize = 0;

  uint32_t ulClass = ptInd->tData.ulClass;
  uint32_t ulInstance = ptInd->tData.ulInstance;
  uint32_t ulService = ptInd->tData.ulService;

  printf("[VAT] CIP Request: Service=0x%02X, Class=0x%02X, Inst=%u\r\n",
         (unsigned int)ulService,
         (unsigned int)ulClass,
         (unsigned int)ulInstance);

  /* Route by Class */
  switch (ulClass)
  {
    case VAT_CLASS_PARAMETER:
      AppDNS_VAT_HandleParameterObject(ptInd, ptRes, &ulResDataSize, &ulGRC, &ulERC);
      break;

    case VAT_CLASS_DIAGNOSTIC:
      AppDNS_VAT_HandleDiagnosticObject(ptInd, ptRes, &ulResDataSize, &ulGRC, &ulERC);
      break;

    default:
      ulGRC = CIP_GRC_OBJECT_DOES_NOT_EXIST;
      printf("[VAT] ERROR: Unknown Class 0x%02X\r\n", (unsigned int)ulClass);
      break;
  }

  /* Fill response */
  ptRes->tData.ulGRC = ulGRC;
  ptRes->tData.ulERC = ulERC;
  *pulResDataSize = ulResDataSize;

  printf("[VAT] Response: GRC=0x%02X, DataSize=%u\r\n",
         (unsigned int)ulGRC, (unsigned int)ulResDataSize);
}

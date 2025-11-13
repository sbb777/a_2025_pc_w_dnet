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

***************************************************************************************/
/* Includes */

/* Includes */
#include "DNS_Includes.h"
#include "AppDNS_DemoApplication.h"


/*************************************************************************************************/
/* User object dirctory entry                                                                    */
/*************************************************************************************************/
typedef struct CIP_REGISTER_OBJ_DIR_ENTRY_Ttag
{
  uint32_t ulClass;
  uint32_t ulOptionFlags;
  uint32_t ulMaxInstance;
} CIP_REGISTER_OBJ_DIR_ENTRY_T;


/*************************************************************************************************/
/* User specic object definition                                                                 */
/*************************************************************************************************/
/* Configuration related definitions */

/* Simulate a imaginary Sensor Control object matching to the example frame work.
*  These object is also referenced as parameter object in the EDS delivered with this Demo.
*  Two instances of this object are supported according the example frame work.
*/
#define DNS_DEMO_OBJECT_SCTRL                       100
#define DNS_DEMO_OBJECT_SCTRL_MAX_INST                2
#define DNS_DEMO_OBJECT_SCTRL_ATT_SENSITY             1

#define DNS_DEMO_OBJECT_SCTRL_SENSITY_HIGH            0
#define DNS_DEMO_OBJECT_SCTRL_SENSITY_MIDDLE          1
#define DNS_DEMO_OBJECT_SCTRL_SENSITY_LOW             2

/* Simulate a imaginary Actuator Control object matching to the example frame work.
*  These object is also referenced as parameter object in the EDS delivered with this Demo.
*  Two instances of this object are supported according the example frame work.
*/
#define DNS_DEMO_OBJECT_ACTRL                        101
#define DNS_DEMO_OBJECT_ACTRL_MAX_INST                2
#define DNS_DEMO_OBJECT_ACTRL_ATT_FAILSAFE            1

#define DNS_DEMO_OBJECT_ACTRL_FAILSAFE_CLEAR          0
#define DNS_DEMO_OBJECT_ACTRL_FAILSAFE_HOLD           1

/* User demo object, free for testing  */
#define DNS_DEMO_OBJECT_TEST                        102
#define DNS_DEMO_OBJECT_TEST_MAX_INST                 1
#define DNS_DEMO_OBJECT_TEST_ATT_1                    1
#define DNS_DEMO_OBJECT_TEST_ATT_1_SIZE             255


#ifdef __HIL_PRAGMA_PACK_ENABLE
#pragma __HIL_PRAGMA_PACK_1(APP_DEMO_USER_OBJECT)
#endif

typedef __HIL_PACKED_PRE struct SENSOR_CONFIG_OBJECT_T
{
  uint8_t bSensity;
}__HIL_PACKED_POST SENSOR_CONFIG_OBJECT_T;

typedef __HIL_PACKED_PRE struct ACTUATOR_CONFIG_OBJECT_T
{
  uint8_t bFailSaveBehaviour;
}__HIL_PACKED_POST ACTUATOR_CONFIG_OBJECT_T;

#ifdef __HIL_PRAGMA_PACK_ENABLE
#pragma __HIL_PRAGMA_UNPACK_1(APP_DEMO_USER_OBJECT)
#endif

static SENSOR_CONFIG_OBJECT_T    s_tSensorConfigObject[DNS_DEMO_OBJECT_SCTRL_MAX_INST];
static ACTUATOR_CONFIG_OBJECT_T  s_tActuatorConfigObject[DNS_DEMO_OBJECT_ACTRL_MAX_INST];
static uint8_t                   s_abTestObject[DNS_DEMO_OBJECT_TEST_ATT_1_SIZE];


/*************************************************************************************************/
/* List of additional CIP objects that shall be supported                                        */
/*************************************************************************************************/
static CIP_REGISTER_OBJ_DIR_ENTRY_T s_atCipRegisterObjDir[] =
{
  /* Class,                   OptionFlags,*/
  { DNS_DEMO_OBJECT_SCTRL,    0x00000000, DNS_DEMO_OBJECT_SCTRL_MAX_INST },
  { DNS_DEMO_OBJECT_ACTRL,    0x00000000, DNS_DEMO_OBJECT_ACTRL_MAX_INST },
  { DNS_DEMO_OBJECT_TEST,     0x00000000, DNS_DEMO_OBJECT_TEST_MAX_INST  },
};

/* Number of additional object classes to register */
uint32_t s_ulCipRegisterObjDirEntries = sizeof(s_atCipRegisterObjDir) / sizeof(s_atCipRegisterObjDir[0]);

/* Pointer to the list of additional object classes to register */
CIP_REGISTER_OBJ_DIR_ENTRY_T* s_ptCipRegisterObjDir = &s_atCipRegisterObjDir[0];



/***************************************************************************************************
*! Register a user object at the stack at startup via packet.
*   \param ptAppData - pointer to APP_DATA_T structure
*   \param ulClass - object to be registered
*   \param ulOptionFlags - option flags
***************************************************************************************************/
static uint32_t AppDNS_RegisterObject(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc, uint32_t ulClass, uint32_t ulOptionFlags)
{
  uint32_t ulRet = CIFX_NO_ERROR;
  DNS_PACKET_REGISTER_CLASS_REQ_T* ptReq = AppDNS_GlobalPacket_Init(ptDnsRsc);

  /* prepare packet to register object */
  ptReq->tData.ulClass = ulClass;
  ptReq->tData.ulServiceMask = ulOptionFlags;

  /* Issue Register Object Request */
  ptReq->tHead.ulCmd = DNS_CMD_REGISTER_CLASS_REQ;
  ptReq->tHead.ulLen = DNS_REGISTER_CLASS_REQ_SIZE;

  ulRet = AppDNS_GlobalPacket_SendReceive(ptDnsRsc);

  return ulRet;
}

/***************************************************************************************************
*! Register all user object at the stack at startup.
*   \param ptAppData - pointer to APP_DATA_T structure
***************************************************************************************************/
uint32_t AppDNS_UserObject_Registration(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc)
{
  uint32_t ulIdx = 0;
  uint32_t ulRet = 0;

  for (ulIdx = 0; (ulIdx < s_ulCipRegisterObjDirEntries) && (ulRet == 0); ulIdx++)
  {
    ulRet = AppDNS_RegisterObject(ptDnsRsc,
                                  s_ptCipRegisterObjDir[ulIdx].ulClass,
                                  s_ptCipRegisterObjDir[ulIdx].ulOptionFlags);
  }

  return ulRet;
}


/***************************************************************************************************
*! Check if a requested object is registered as user object. This function is called from the
* packet handler before routing the cip request it to the user object handler.
*   \param ptAppData - pointer to APP_DATA_T structure
***************************************************************************************************/
bool AppDNS_UserObject_IsRegistered(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc, uint32_t ulClass, uint32_t ulInstance)
{
  uint32_t ulIdx = 0;
  for (ulIdx = 0; (ulIdx < s_ulCipRegisterObjDirEntries); ulIdx++)
  {
    if (ulClass == s_ptCipRegisterObjDir[ulIdx].ulClass)
    {
      if (ulInstance != 0 && ulInstance <= s_ptCipRegisterObjDir[ulIdx].ulMaxInstance)
      {
        return true;
      } else
      {
        return false;
      }
    }
  }
  return false;
}


/***************************************************************************************************
*! User object handler. This function handles all incomming cip requests which are direct to objects
* which have been registered by the host application.
*   \param ptAppData - pointer to APP_DATA_T structure
*   \param ptInd - pointer to the cip service indication packet
*   \param ptRes - pointer to the cip service response packet
*   \param pulResDataSize - number of service response data
***************************************************************************************************/
void AppDNS_UserObject_Indication(APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
                                  DNS_PACKET_CIP_SERVICE_IND_T*  ptInd,
                                  DNS_PACKET_CIP_SERVICE_RES_T*  ptRes,
                                  uint32_t*                      pulResDataSize)
{
  uint32_t ulGRC = 0;
  uint32_t ulERC = 0;
  uint32_t ulAttDataSize = 0;
  uint32_t ulMinAttDataSize = 0;
  uint32_t ulResDataSize = 0;
  uint8_t  *pbAttData = NULL;

  switch (ptInd->tData.ulService)
  {
    case CIP_SERVICE_GET_ATTRIBUTE_SINGLE:
    {
      switch (ptInd->tData.ulClass)
      {
        case DNS_DEMO_OBJECT_SCTRL:
        {
          if (ptInd->tData.ulAttribute == DNS_DEMO_OBJECT_SCTRL_ATT_SENSITY)
          {
            pbAttData = &s_tSensorConfigObject[ptInd->tData.ulInstance].bSensity;
            ulAttDataSize = sizeof(s_tSensorConfigObject[ptInd->tData.ulInstance].bSensity);
          }
        }
        break;

        case DNS_DEMO_OBJECT_ACTRL:
        {
          if (ptInd->tData.ulAttribute == DNS_DEMO_OBJECT_ACTRL_ATT_FAILSAFE)
          {
            pbAttData = &s_tActuatorConfigObject[ptInd->tData.ulInstance].bFailSaveBehaviour;
            ulAttDataSize = sizeof(s_tActuatorConfigObject[ptInd->tData.ulInstance].bFailSaveBehaviour);
          }
        }
        break;

        case DNS_DEMO_OBJECT_TEST:
        {
          if (ptInd->tData.ulAttribute == DNS_DEMO_OBJECT_TEST_ATT_1)
          {
            s_abTestObject[0] += 1;
            if (s_abTestObject[0] == DNS_DEMO_OBJECT_TEST_ATT_1_SIZE){
              s_abTestObject[0] = 1;
            }

            ulAttDataSize = s_abTestObject[0];
            pbAttData = &s_abTestObject[0];
          }
        }
        break;
      } /* switch (..ulClass) */

      /* If requested attribut exist return the data */
      if (pbAttData != NULL)
      {
        memcpy(&ptRes->tData.abData[0], pbAttData, ulAttDataSize);
        ulResDataSize = ulAttDataSize;
        ulGRC = CIP_GRC_SUCCESS;
        ulERC = 0;
      }
      else
      {
        ulGRC = CIP_GRC_ATTRIBUTE_NOT_SUPPORTED;
        ulERC = 0;
      }
    } /* case CIP_SERVICE_GET_ATTRIBUTE_SINGLE */
    break;

    case CIP_SERVICE_SET_ATTRIBUTE_SINGLE:
    {
      switch (ptInd->tData.ulClass)
      {
        case DNS_DEMO_OBJECT_SCTRL:
        {
          if (ptInd->tData.ulAttribute == DNS_DEMO_OBJECT_SCTRL_ATT_SENSITY)
          {
            pbAttData = &s_tSensorConfigObject[ptInd->tData.ulInstance].bSensity;
            ulAttDataSize = sizeof(s_tSensorConfigObject[ptInd->tData.ulInstance].bSensity);
            ulMinAttDataSize = ulAttDataSize; /* allow only excact size of att data to be set */
          }
        }
        break;

        case DNS_DEMO_OBJECT_ACTRL:
        {
          if (ptInd->tData.ulAttribute == DNS_DEMO_OBJECT_ACTRL_ATT_FAILSAFE)
          {
            pbAttData = &s_tActuatorConfigObject[ptInd->tData.ulInstance].bFailSaveBehaviour;
            ulAttDataSize = sizeof(s_tActuatorConfigObject[ptInd->tData.ulInstance].bFailSaveBehaviour);
            ulMinAttDataSize = ulAttDataSize; /* allow only excact size of att data to be set */
          }
        }
        break;

        case DNS_DEMO_OBJECT_TEST:
        {
          if (ptInd->tData.ulAttribute == DNS_DEMO_OBJECT_TEST_ATT_1)
          {
            pbAttData = &s_abTestObject[0];
            ulAttDataSize = DNS_DEMO_OBJECT_TEST_ATT_1_SIZE;
            ulMinAttDataSize = 0; /* allow a variant size of att data to be set*/
          }
        }
        break;
      }

      uint32_t ulDataLength = ptInd->tHead.ulLen - DNS_CIP_SERVICE_IND_SIZE;
      if (pbAttData == NULL)
      {
        ulGRC = CIP_GRC_ATTRIBUTE_NOT_SUPPORTED;
        ulERC = 0;
      }
      else if (0 == ulDataLength)
      {
        /* Set ATT size 0 is accept it as an option to reset the ATT value. */
        ulGRC = CIP_GRC_SUCCESS;
        ulERC = 0;
        memset(pbAttData, 0x00, ulAttDataSize);
      }
      else if (ulDataLength > ulAttDataSize)
      {
        ulGRC = CIP_GRC_TOO_MUCH_DATA;
      }
      else if (ulDataLength < ulMinAttDataSize)
      {
        ulGRC = CIP_GRC_NOT_ENOUGH_DATA;
      }
      else
      {
        /* Check content of user object data to be set:
         *
         * ... */
        if (DNS_DEMO_OBJECT_SCTRL == ptInd->tData.ulClass )
        {
          if (ptInd->tData.abData[0] > DNS_DEMO_OBJECT_SCTRL_SENSITY_LOW)
          {
            ulGRC = CIP_GRC_INVALID_ATTRIBUTE_VALUE;
          }
        }
        else if (DNS_DEMO_OBJECT_ACTRL == ptInd->tData.ulClass)
        {
          if (ptInd->tData.abData[0] > DNS_DEMO_OBJECT_ACTRL_FAILSAFE_HOLD)
          {
            ulGRC = CIP_GRC_INVALID_ATTRIBUTE_VALUE;
          }
        }
      }

      if (ulGRC == CIP_GRC_SUCCESS)
      {
        /* Store new configuration data */
        memcpy(pbAttData, &ptInd->tData.abData[0], ulDataLength);
      }
    } /* case CIP_SERVICE_SET_ATTRIBUTE_SINGLE */
    break;

    default:
    {
      ulGRC = CIP_GRC_SERVICE_NOT_SUPPORTED;
      ulERC = 0;
    }
    break;
  }

  ptRes->tData.ulGRC = ulGRC;
  ptRes->tData.ulERC = ulERC;
  *pulResDataSize = ulResDataSize;

  return;
}

/******************************************************************************* * VAT CIP Explicit Message 완전한 수정 코드
 *
 * 이 코드를 AppDNS_DemoApplicationFunctions.c에 추가/수정하세요
 ******************************************************************************/

/* =========================================================================
 * 1. 헤더 추가 (파일 상단에 추가)
 * ========================================================================= */

#include "DNS_packet_register_class.h"
#include "DNS_packet_cip_service.h"
#include "AppDNS_ExplicitMsg.h"  /* CIP 처리 함수 */


/* =========================================================================
 * 2. Class 등록 함수 (파일 끝부분에 추가)
 * ========================================================================= */

/******************************************************************************
 * Register CIP Class to DeviceNet Stack
 ******************************************************************************/
uint32_t AppDNS_RegisterClass(APP_DATA_T* ptAppData, uint32_t ulClass)
{
    uint32_t ulRet = CIFX_NO_ERROR;
    DNS_PACKET_REGISTER_CLASS_REQ_T* ptReq =
        (DNS_PACKET_REGISTER_CLASS_REQ_T*)AppDNS_GlobalPacket_Init(ptAppData);

    /* Set packet header */
    ptReq->tHead.ulCmd = DNS_CMD_REGISTER_CLASS_REQ;
    ptReq->tHead.ulLen = DNS_REGISTER_CLASS_REQ_SIZE;
    ptReq->tHead.ulSta = 0;

    /* Set class to register */
    ptReq->tData.ulClass = ulClass;
    ptReq->tData.ulServiceMask = 0xFFFFFFFF;  /* All services */

    /* Send and receive */
    ulRet = AppDNS_GlobalPacket_SendReceive(ptAppData);

    if (ulRet == CIFX_NO_ERROR) {
        printf("✅ Class 0x%02X registered successfully\n", (unsigned int)ulClass);
    } else {
        printf("❌ Failed to register Class 0x%02X, error: 0x%08X\n",
               (unsigned int)ulClass, (unsigned int)ulRet);
    }

    return ulRet;
}

/******************************************************************************
 * Register All VAT CIP Classes
 ******************************************************************************/
uint32_t AppDNS_RegisterAllVatClasses(APP_DATA_T* ptAppData)
{
    uint32_t ulRet = CIFX_NO_ERROR;

    printf("\n=== Registering VAT CIP Classes ===\n");

    /* Register Identity Object (Class 0x01) */
    ulRet = AppDNS_RegisterClass(ptAppData, 0x01);
    if (ulRet != CIFX_NO_ERROR) {
        return ulRet;
    }

    /* Register VAT Object (Class 0x30) */
    ulRet = AppDNS_RegisterClass(ptAppData, 0x30);
    if (ulRet != CIFX_NO_ERROR) {
        return ulRet;
    }

    printf("===================================\n\n");

    return ulRet;
}


/* =========================================================================
 * 3. CIP Service Indication 처리 함수 (파일 끝부분에 추가)
 * ========================================================================= */

/******************************************************************************
 * Handle CIP Service Indication (Explicit Message from Master)
 ******************************************************************************/
void AppDNS_HandleCipServiceIndication(APP_DATA_T* ptAppData)
{
    DNS_PACKET_CIP_SERVICE_IND_T* ptInd =
        (DNS_PACKET_CIP_SERVICE_IND_T*)AppDNS_GlobalPacket_Get(ptAppData);

    DNS_PACKET_CIP_SERVICE_RES_T* ptRes =
        (DNS_PACKET_CIP_SERVICE_RES_T*)AppDNS_GlobalPacket_Get(ptAppData);

    /* Extract CIP message information */
    uint8_t service = (uint8_t)ptInd->tData.ulService;
    uint8_t class_id = (uint8_t)ptInd->tData.ulClass;
    uint8_t instance_id = (uint8_t)ptInd->tData.ulInstance;
    uint8_t attribute_id = (uint8_t)ptInd->tData.ulAttribute;
    uint32_t dataLen = ptInd->tHead.ulLen - DNS_CIP_SERVICE_IND_SIZE;

    /* Debug output */
    printf("\n=== CIP Service Indication ===\n");
    printf("Service:   0x%02X ", service);
    if (service == 0x0E) printf("(Get Attribute Single)\n");
    else if (service == 0x10) printf("(Set Attribute Single)\n");
    else if (service == 0x01) printf("(Get Attribute All)\n");
    else if (service == 0x16) printf("(Save)\n");
    else if (service == 0x05) printf("(Reset)\n");
    else printf("(Unknown)\n");

    printf("Class:     0x%02X\n", class_id);
    printf("Instance:  0x%02X\n", instance_id);
    printf("Attribute: 0x%02X\n", attribute_id);
    printf("Data Len:  %d\n", (int)dataLen);

    /* Process CIP service */
    uint8_t error = 0;
    uint32_t respDataLen = 0;

    switch (service) {
        case 0x0E:  /* Get Attribute Single */
        {
            uint8_t respSize = 0;
            error = CIP_HandleGetAttributeSingle(class_id, instance_id, attribute_id,
                                                  ptRes->tData.abData, &respSize);
            respDataLen = respSize;

            if (error == 0) {
                printf("  -> Get: Success, Data=");
                for (uint32_t i = 0; i < respDataLen; i++) {
                    printf("%02X ", ptRes->tData.abData[i]);
                }
                printf("\n");
            } else {
                printf("  -> Get: Error 0x%02X\n", error);
            }
            break;
        }

        case 0x10:  /* Set Attribute Single */
        {
            printf("  Data: ");
            for (uint32_t i = 0; i < dataLen; i++) {
                printf("%02X ", ptInd->tData.abData[i]);
            }
            printf("\n");

            error = CIP_HandleSetAttributeSingle(class_id, instance_id, attribute_id,
                                                  ptInd->tData.abData, (uint8_t)dataLen);
            respDataLen = 0;

            if (error == 0) {
                printf("  -> Set: Success\n");
            } else {
                printf("  -> Set: Error 0x%02X\n", error);
            }
            break;
        }

        case 0x01:  /* Get Attribute All */
        {
            uint16_t respSize = 0;
            error = CIP_HandleGetAttributeAll(class_id, instance_id,
                                              ptRes->tData.abData, &respSize);
            respDataLen = respSize;
            printf("  -> Get All: %s, Size=%d\n",
                   (error == 0) ? "Success" : "Error", respSize);
            break;
        }

        case 0x16:  /* Save */
        {
            error = CIP_HandleSave();
            respDataLen = 0;
            printf("  -> Save: %s\n", (error == 0) ? "Success" : "Error");
            break;
        }

        case 0x05:  /* Reset */
        {
            error = CIP_HandleReset();
            respDataLen = 0;
            printf("  -> Reset: %s\n", (error == 0) ? "Success" : "Error");
            break;
        }

        default:
            error = 0x08;  /* Service not supported */
            respDataLen = 0;
            printf("  -> Unsupported service!\n");
            break;
    }

    /* Build response packet */
    ptRes->tHead.ulCmd = DNS_CMD_CIP_SERVICE_RES;
    ptRes->tHead.ulLen = DNS_CIP_SERVICE_RES_SIZE + respDataLen;
    ptRes->tHead.ulSta = 0;

    ptRes->tData.ulService = ptInd->tData.ulService;
    ptRes->tData.ulClass = ptInd->tData.ulClass;
    ptRes->tData.ulInstance = ptInd->tData.ulInstance;
    ptRes->tData.ulAttribute = ptInd->tData.ulAttribute;
    ptRes->tData.ulMember = ptInd->tData.ulMember;
    ptRes->tData.ulGRC = error;  /* General Error Code */
    ptRes->tData.ulERC = 0;      /* Extended Error Code */

    printf("Response:  Cmd=0x%08X, Len=%d, Error=0x%02X\n",
           (unsigned int)ptRes->tHead.ulCmd, (int)ptRes->tHead.ulLen, error);
    printf("==============================\n\n");

    /* Send response */
    AppDNS_GlobalPacket_Send(ptAppData);
}


/* =========================================================================
 * 4. PacketHandler 수정 (기존 함수 수정)
 * ========================================================================= */

/*
 기존 AppDNS_PacketHandler_callback() 함수의 switch 문에 다음을 추가:
 */

#if 0  /* 예시 코드 - 실제 적용 시 기존 switch에 추가하세요 */

bool AppDNS_PacketHandler_callback( CIFX_PACKET* ptPacket,
                           void*        pvUserData )
{
  bool        fPacketHandled = true;
  APP_DATA_T* ptAppData = (APP_DATA_T*)pvUserData;

  if( ptPacket != &ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket )
  {
    PRINTF("Unexpected packet resource received!!!" NEWLINE);
    return false;
  }

  switch( ptPacket->tHeader.ulCmd )
  {
    /* ⭐ 이 부분을 추가하세요 ⭐ */
    case DNS_CMD_CIP_SERVICE_IND:  /* 0x0000B104 */
      /* CIP Service Indication - Explicit Message from Master */
      AppDNS_HandleCipServiceIndication(ptAppData);
      fPacketHandled = true;
      break;

    /* 기존 default 케이스는 그대로 유지 */
    default:
    {
      if ((ptPacket->tHeader.ulCmd & 0x1) == 0)
      {
        PRINTF("Warning: Disregarded indication packet, Cmd=0x%08X\n",
               (unsigned int)ptPacket->tHeader.ulCmd);
        ptPacket->tHeader.ulState = ERR_HIL_NO_APPLICATION_REGISTERED;
        ptPacket->tHeader.ulCmd |= 0x01;
        AppDNS_GlobalPacket_Send(ptAppData);
      }
    }
    break;
  }

  return fPacketHandled;
}

#endif  /* 예시 코드 끝 */


/* =========================================================================
 * 5. 초기화에 Class 등록 추가
 * ========================================================================= */

/*
 AppDNS_DemoApplication.c의 AppDNS_Init() 함수에 다음을 추가:
 (Set Configuration 이후, Create Assembly 이전에 추가)
 */

#if 0  /* 예시 코드 */

uint32_t AppDNS_Init(APP_DATA_T* ptAppData)
{
  uint32_t ulRet = CIFX_NO_ERROR;

  /* ... 기존 초기화 코드 ... */

  /* Set Configuration */
  ulRet = AppDNS_SetConfiguration(ptAppData);
  if (ulRet != CIFX_NO_ERROR) {
    return ulRet;
  }

  /* ⭐ 여기에 추가하세요 ⭐ */
  /* Register VAT CIP Classes */
  ulRet = AppDNS_RegisterAllVatClasses(ptAppData);
  if (ulRet != CIFX_NO_ERROR) {
    PRINTF("Failed to register CIP classes!\n");
    return ulRet;
  }

  /* ... 나머지 초기화 코드 ... */

  return ulRet;
}

#endif  /* 예시 코드 끝 */

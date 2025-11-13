# VAT CIP Class 등록 및 Explicit Message 처리 가이드

## ❌ 현재 문제

### netHost 수신 패킷
```
State = 0xC0000004   ← ❗ 에러!
Len = 0              ← 응답 없음
```

### 에러 원인
- DeviceNet 스택에 **Class가 등록되지 않음**
- 스택이 Explicit Message를 애플리케이션으로 전달하지 않음
- 따라서 응답이 생성되지 않음

---

## ✅ 해결 방법

### 1단계: Class 등록 (초기화 시)
### 2단계: CIP Service Indication 처리
### 3단계: CIP Service Response 전송

---

## 🔧 1. Class 등록 함수 추가

### AppDNS_DemoApplicationFunctions.c에 추가:

```c
#include "DNS_packet_register_class.h"
#include "DNS_packet_cip_service.h"
#include "DNS_packet_commands.h"

/******************************************************************************
 * Register VAT CIP Classes to DeviceNet Stack
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
        PRINTF("Class 0x%02X registered successfully\n", ulClass);
    } else {
        PRINTF("Failed to register Class 0x%02X, error: 0x%08X\n", ulClass, ulRet);
    }

    return ulRet;
}

/******************************************************************************
 * Register All VAT Classes
 ******************************************************************************/

uint32_t AppDNS_RegisterAllVatClasses(APP_DATA_T* ptAppData)
{
    uint32_t ulRet = CIFX_NO_ERROR;

    PRINTF("\n=== Registering VAT CIP Classes ===\n");

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

    PRINTF("===================================\n\n");

    return ulRet;
}
```

---

## 🔧 2. CIP Service Indication 처리

### AppDNS_DemoApplicationFunctions.c 수정:

```c
#include "AppDNS_ExplicitMsg.h"  /* CIP_ProcessExplicitMessage() */

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
    PRINTF("\n=== CIP Service Indication ===\n");
    PRINTF("Service: 0x%02X\n", service);
    PRINTF("Class: 0x%02X\n", class_id);
    PRINTF("Instance: 0x%02X\n", instance_id);
    PRINTF("Attribute: 0x%02X\n", attribute_id);
    PRINTF("Data Length: %d\n", dataLen);

    /* Process CIP service */
    uint8_t error = 0;
    uint32_t respDataLen = 0;

    if (service == 0x0E) {
        /* Get Attribute Single */
        uint8_t respSize = 0;
        error = CIP_HandleGetAttributeSingle(class_id, instance_id, attribute_id,
                                              ptRes->tData.abData, &respSize);
        respDataLen = respSize;

    } else if (service == 0x10) {
        /* Set Attribute Single */
        error = CIP_HandleSetAttributeSingle(class_id, instance_id, attribute_id,
                                              ptInd->tData.abData, (uint8_t)dataLen);
        respDataLen = 0;

    } else if (service == 0x01) {
        /* Get Attribute All */
        uint16_t respSize = 0;
        error = CIP_HandleGetAttributeAll(class_id, instance_id,
                                          ptRes->tData.abData, &respSize);
        respDataLen = respSize;

    } else if (service == 0x16) {
        /* Save */
        error = CIP_HandleSave();
        respDataLen = 0;

    } else if (service == 0x05) {
        /* Reset */
        error = CIP_HandleReset();
        respDataLen = 0;

    } else {
        /* Unsupported service */
        error = 0x08;  /* Service not supported */
        respDataLen = 0;
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

    /* Debug output */
    PRINTF("Response Error Code: 0x%02X\n", error);
    PRINTF("Response Data Length: %d\n", respDataLen);
    PRINTF("==============================\n\n");

    /* Send response */
    AppDNS_GlobalPacket_Send(ptAppData);
}
```

---

## 🔧 3. Packet Handler 수정

### AppDNS_DemoApplicationFunctions.c의 AppDNS_PacketHandler_callback() 수정:

```c
bool AppDNS_PacketHandler_callback( CIFX_PACKET* ptPacket,
                           void*        pvUserData )
{
  bool        fPacketHandled = true;
  APP_DATA_T* ptAppData = (APP_DATA_T*)pvUserData;

  /* Check if the received packet is placed in the channel related global packet buffer */
  if( ptPacket != &ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket )
  {
    PRINTF("Unexpected packet resource received!!!" NEWLINE);
    return false;
  }

  switch( ptPacket->tHeader.ulCmd )
  {
    case DNS_CMD_CIP_SERVICE_IND:
      /* ⭐ CIP Service Indication - Explicit Message from Master */
      AppDNS_HandleCipServiceIndication(ptAppData);
      fPacketHandled = true;
      break;

    default:
    {
      if ((ptPacket->tHeader.ulCmd & 0x1) == 0)
      {
        PRINTF("Warning: Disregarded indication packet received! Cmd=0x%08X\n",
               ptPacket->tHeader.ulCmd);

        ptPacket->tHeader.ulState = ERR_HIL_NO_APPLICATION_REGISTERED;
        ptPacket->tHeader.ulCmd |= 0x01; /* Make it a response */
        AppDNS_GlobalPacket_Send(ptAppData);
      }
      else
      {
        PRINTF("Warning: Disregarded confirmation packet received! Cmd=0x%08X\n",
               ptPacket->tHeader.ulCmd);
      }
    }
    break;
  }

  return fPacketHandled;
}
```

---

## 🔧 4. 초기화에 Class 등록 추가

### AppDNS_DemoApplication.c의 AppDNS_Init() 함수 수정:

```c
uint32_t AppDNS_Init(APP_DATA_T* ptAppData)
{
  uint32_t ulRet = CIFX_NO_ERROR;

  /* ... 기존 초기화 코드 ... */

  /* Set Configuration */
  ulRet = AppDNS_SetConfiguration(ptAppData);
  if (ulRet != CIFX_NO_ERROR) {
    return ulRet;
  }

  /* ⭐ Register VAT CIP Classes */
  ulRet = AppDNS_RegisterAllVatClasses(ptAppData);
  if (ulRet != CIFX_NO_ERROR) {
    PRINTF("Failed to register CIP classes!\n");
    return ulRet;
  }

  /* ... 나머지 초기화 코드 ... */

  return ulRet;
}
```

---

## 📊 예상 동작

### 초기화 시 (Class 등록)
```
=== Registering VAT CIP Classes ===
Class 0x01 registered successfully
Class 0x30 registered successfully
===================================
```

### netHost에서 Set Param6 전송 시
```
=== CIP Service Indication ===
Service: 0x10
Class: 0x30
Instance: 0x01
Attribute: 0x0C
Data Length: 1
Response Error Code: 0x00
Response Data Length: 0
==============================
```

### netHost 수신 (정상)
```
State = 0x00000000   ✅ Success!
Cmd = 0x00008021     ✅ RCX_SEND_PACKET_CNF
Len = 0x00000004     ✅ 응답 있음!
Data = 90 00 00 00   ✅ CIP Success
```

---

## 🎯 요약

### 필수 작업
1. ✅ `AppDNS_RegisterClass()` 함수 구현
2. ✅ `AppDNS_RegisterAllVatClasses()` 함수 구현
3. ✅ `AppDNS_HandleCipServiceIndication()` 함수 구현
4. ✅ `AppDNS_PacketHandler_callback()` 수정 (DNS_CMD_CIP_SERVICE_IND 추가)
5. ✅ `AppDNS_Init()`에서 Class 등록 호출

### 등록할 Classes
- **Class 0x01**: Identity Object (Param1: Vendor ID)
- **Class 0x30**: VAT Object (Param5, Param6, ...)

### 핵심 포인트
- **Class 등록 없이는 Explicit Message 처리 불가**
- **Indication/Response 구조로 동작**
- **DeviceNet 스택이 자동으로 Indication 전달**

---

**작성일**: 2025-11-05
**버전**: 1.0

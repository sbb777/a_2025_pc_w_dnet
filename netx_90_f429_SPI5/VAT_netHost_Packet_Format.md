# netHost 패킷 포맷 상세 가이드

## 📋 RCX_PACKET 구조

### 완전한 패킷 구조
```c
typedef struct RCX_PACKET_HEADER_Ttag {
    uint32_t ulDest;      /* Destination Queue Handle */
    uint32_t ulSrc;       /* Source Queue Handle */
    uint32_t ulDestId;    /* Destination End Point Identifier */
    uint32_t ulSrcId;     /* Source End Point Identifier */
    uint32_t ulLen;       /* Packet Data Length (bytes) */
    uint32_t ulId;        /* Packet Identification */
    uint32_t ulState;     /* State */
    uint32_t ulCmd;       /* Command/Response */
    uint32_t ulExt;       /* Extension */
    uint32_t ulRout;      /* Routing Information */
} RCX_PACKET_HEADER_T;

typedef struct RCX_PACKET_Ttag {
    RCX_PACKET_HEADER_T tHeader;
    uint8_t             abData[1024];  /* Payload */
} RCX_PACKET_T;
```

---

## 🔧 1. netHost 전송 패킷 (전체)

### Test 1: Get Param1 (Vendor ID 읽기)

**전체 패킷**:
```
Dest=0x00000000      # Host Channel (PC → netX)
Src=0x00000000       # Response will use netX channel
DestID=0x00000000    # Default
SrcID=0x00000000     # Default
Len=0x00000008       # CIP 메시지 길이: 8 bytes
ID=0x00000000        # Packet ID (매칭용)
State=0x00000000     # Default
Cmd=0x00008020       # RCX_SEND_PACKET_REQ ⭐ 중요!
Ext=0x00000000       # No extension
Route=0x00000000     # No routing
Data=0E 03 20 01 24 01 30 01   # CIP 메시지
```

**CIP 메시지 상세** (Data 필드):
```
Byte  Hex   의미
----  ----  --------------------------------------------------
0     0x0E  Service: Get Attribute Single
1     0x03  Path Size: 3 words (6 bytes)
2     0x20  Logical Class ID (8-bit)
3     0x01  Class = 0x01 (Identity Object)
4     0x24  Logical Instance ID (8-bit)
5     0x01  Instance = 0x01
6     0x30  Logical Attribute ID (8-bit)
7     0x01  Attribute = 0x01 (Vendor ID)
```

---

### Test 2: Set Param6 (Controller Mode 쓰기)

**전체 패킷** (사용자가 전송한 것):
```
Dest=0x00000000
Src=0x00000000
DestID=0x00000000
SrcID=0x00000000
Len=0x00000009       # CIP 메시지 길이: 9 bytes
ID=0x00000000
State=0x00000000
Cmd=0x00008020       # RCX_SEND_PACKET_REQ ⭐
Ext=0x00000000
Route=0x00000000
Data=10 03 20 30 24 01 30 0C 05   # CIP 메시지
```

**CIP 메시지 상세** (Data 필드):
```
Byte  Hex   의미
----  ----  --------------------------------------------------
0     0x10  Service: Set Attribute Single
1     0x03  Path Size: 3 words
2     0x20  Logical Class ID
3     0x30  Class = 0x30 (VAT Object)
4     0x24  Logical Instance ID
5     0x01  Instance = 0x01
6     0x30  Logical Attribute ID
7     0x0C  Attribute = 0x0C (Controller Mode)
8     0x05  Data: Value = 5 (Pressure Control)
```

---

## 📊 2. netHost 수신 패킷 (전체)

### Test 1 응답: Vendor ID = 404

**전체 응답 패킷**:
```
Dest=0x00000000      # Host Channel (netX → PC)
Src=0x00000020       # netX DeviceNet Channel
DestID=0x00000000
SrcID=0x00000000
Len=0x00000006       # CIP 응답 길이: 6 bytes ⭐
ID=0x00000000        # 요청과 동일
State=0x00000000
Cmd=0x00008021       # RCX_SEND_PACKET_CNF (응답) ⭐
Ext=0x00000000
Route=0x00000000
Data=8E 00 00 00 94 01   # CIP 응답
```

**CIP 응답 상세** (Data 필드):
```
Byte  Hex   의미
----  ----  --------------------------------------------------
0     0x8E  Reply Service (0x0E | 0x80)
1     0x00  Reserved
2     0x00  General Status: Success ✅
3     0x00  Additional Status Size
4     0x94  Data: Vendor ID Low byte
5     0x01  Data: Vendor ID High byte
→ Vendor ID = 0x0194 = 404 ✅
```

---

### Test 2 응답: Set Success

**전체 응답 패킷**:
```
Dest=0x00000000
Src=0x00000020
DestID=0x00000000
SrcID=0x00000000
Len=0x00000004       # CIP 응답 길이: 4 bytes ⭐
ID=0x00000000
State=0x00000000
Cmd=0x00008021       # RCX_SEND_PACKET_CNF ⭐
Ext=0x00000000
Route=0x00000000
Data=90 00 00 00   # CIP 응답
```

**CIP 응답 상세** (Data 필드):
```
Byte  Hex   의미
----  ----  --------------------------------------------------
0     0x90  Reply Service (0x10 | 0x80)
1     0x00  Reserved
2     0x00  General Status: Success ✅
3     0x00  Additional Status Size
```

---

## 🔍 3. 현재 문제 분석

### 사용자의 전송 패킷 (정상)
```
Cmd=0x00008020      # ✅ 정상
Len=0x00000009      # ✅ 정상
Data=10 03 20 30 24 01 30 0C 05   # ✅ 정상
```

### 현재 수신 패킷 (문제)
```
Len=0x00000000      # ❌ 0 bytes - 응답 없음!
```

### ❌ 문제 원인

**정상 응답이라면**:
```
Cmd=0x00008021      # RCX_SEND_PACKET_CNF (응답)
Len=0x00000004      # 4 bytes
Data=90 00 00 00    # Success 응답
```

**현재 상태**:
- `Len=0`: 응답 데이터가 생성되지 않음
- **원인**: `pResponse->tHeader.ulLen`이 설정되지 않음
- **원인**: CIP 응답 생성 함수 누락

---

## 🔧 4. 해결 방법

### AppDNS_DemoApplicationFunctions.c

```c
#include "AppDNS_ExplicitMsg.h"
#include <stdio.h>

/******************************************************************************
 * Explicit Message Handler
 ******************************************************************************/

void AppDNS_HandleExplicitMessage(RCX_PACKET* pRequest, RCX_PACKET* pResponse)
{
    /* 요청 패킷 정보 */
    uint32_t reqLen = pRequest->tHeader.ulLen;
    const uint8_t* pReqData = pRequest->abData;

    /* 응답 패킷 준비 */
    uint8_t* pRespData = pResponse->abData;
    uint32_t respLen = 0;

    /* 디버그 출력 */
    printf("\n=== RCX Packet Received ===\n");
    printf("Cmd:    0x%08X\n", pRequest->tHeader.ulCmd);
    printf("Len:    0x%08X (%d bytes)\n", reqLen, reqLen);
    printf("Data:   ");
    for (uint32_t i = 0; i < reqLen; i++) {
        printf("%02X ", pReqData[i]);
    }
    printf("\n");

    /* CIP 메시지 처리 */
    uint8_t error = CIP_ProcessExplicitMessage(pReqData, reqLen,
                                                pRespData, &respLen);

    /* ⭐ 중요: 응답 패킷 헤더 설정 */
    pResponse->tHeader.ulLen = respLen;
    pResponse->tHeader.ulCmd = 0x00008021;  /* RCX_SEND_PACKET_CNF */
    pResponse->tHeader.ulState = 0;
    pResponse->tHeader.ulId = pRequest->tHeader.ulId;  /* ID 매칭 */

    /* 디버그 출력 */
    printf("Response:\n");
    printf("Cmd:    0x%08X\n", pResponse->tHeader.ulCmd);
    printf("Len:    0x%08X (%d bytes)\n", respLen, respLen);
    printf("Data:   ");
    for (uint32_t i = 0; i < respLen; i++) {
        printf("%02X ", pRespData[i]);
    }
    printf("\n");
    printf("CIP Error: 0x%02X\n", error);
    printf("===========================\n\n");
}
```

---

## 📊 5. 전체 테스트 데이터 (CMD 포함)

### Test 1: Get Param1 (Vendor ID)

**전송**:
| 필드 | 값 | 의미 |
|------|------|------|
| **Cmd** | `0x00008020` | RCX_SEND_PACKET_REQ |
| **Len** | `0x00000008` | 8 bytes |
| **Data** | `0E 03 20 01 24 01 30 01` | Get Attr Single |

**예상 수신**:
| 필드 | 값 | 의미 |
|------|------|------|
| **Cmd** | `0x00008021` | RCX_SEND_PACKET_CNF |
| **Len** | `0x00000006` | 6 bytes |
| **Data** | `8E 00 00 00 94 01` | Vendor ID = 404 |

---

### Test 2: Set Param6 (Controller Mode = 5)

**전송**:
| 필드 | 값 | 의미 |
|------|------|------|
| **Cmd** | `0x00008020` | RCX_SEND_PACKET_REQ |
| **Len** | `0x00000009` | 9 bytes |
| **Data** | `10 03 20 30 24 01 30 0C 05` | Set Attr Single |

**예상 수신**:
| 필드 | 값 | 의미 |
|------|------|------|
| **Cmd** | `0x00008021` | RCX_SEND_PACKET_CNF |
| **Len** | `0x00000004` | 4 bytes |
| **Data** | `90 00 00 00` | Success |

---

### Test 3: Get Param5 (Device Status)

**전송**:
| 필드 | 값 | 의미 |
|------|------|------|
| **Cmd** | `0x00008020` | RCX_SEND_PACKET_REQ |
| **Len** | `0x00000008` | 8 bytes |
| **Data** | `0E 03 20 30 24 01 30 0B` | Get Attr Single |

**예상 수신**:
| 필드 | 값 | 의미 |
|------|------|------|
| **Cmd** | `0x00008021` | RCX_SEND_PACKET_CNF |
| **Len** | `0x00000005` | 5 bytes |
| **Data** | `8E 00 00 00 02` | Status = 2 (Idle) |

---

### Test 4: Set Read-Only (에러)

**전송**:
| 필드 | 값 | 의미 |
|------|------|------|
| **Cmd** | `0x00008020` | RCX_SEND_PACKET_REQ |
| **Len** | `0x0000000A` | 10 bytes |
| **Data** | `10 03 20 01 24 01 30 01 99 99` | Set Read-Only |

**예상 수신**:
| 필드 | 값 | 의미 |
|------|------|------|
| **Cmd** | `0x00008021` | RCX_SEND_PACKET_CNF |
| **Len** | `0x00000004` | 4 bytes |
| **Data** | `90 00 0E 00` | Error: Not Settable |

---

## 🎯 요약

### CMD 필드
- **요청**: `0x00008020` (RCX_SEND_PACKET_REQ)
- **응답**: `0x00008021` (RCX_SEND_PACKET_CNF)

### 전체 패킷 = 헤더 (40 bytes) + 데이터 (가변)
```
RCX_PACKET_HEADER_T (40 bytes):
  - ulDest, ulSrc, ulDestId, ulSrcId
  - ulLen ⭐ (CIP 메시지 길이)
  - ulId, ulState
  - ulCmd ⭐ (0x8020 or 0x8021)
  - ulExt, ulRout

abData[] (가변):
  - CIP 메시지 (Service + Path + Data)
```

### 핵심 포인트
1. ✅ **Cmd** 필드는 필수 (0x00008020)
2. ✅ **Len** 필드는 CIP 메시지 길이
3. ✅ **Data** 필드는 CIP 메시지 전체
4. ⭐ **응답 시**: `pResponse->tHeader.ulLen` 반드시 설정!

---

**작성일**: 2025-11-05
**버전**: 1.0

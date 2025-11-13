# DeviceNet Explicit 메시지 통신 에러 분석 보고서

**작성일**: 2025-11-07
**프로젝트**: netX90 F429 SPI5 DeviceNet Implementation
**분석 대상**: Explicit 메시지 통신 에러

---

## 1. 문제 상황 요약

### 1.1 현재 상태
- ✅ **I/O 메시징**: Hilscher Stack + STM32 디바이스와 netHOST 간 정상 통신 확인
- ❌ **Explicit 메시징**: 수신 패킷이 없음 (UCMM 통신 실패)

### 1.2 수신된 에러 패킷 분석

| 필드 | 값 | 의미 |
|------|-----|------|
| **Dest** | `0x00000000` | Host Channel (PC → netX) |
| **Src** | `0x00000000` | netX Channel (Slave 응답) |
| **Len** | `0` | ❗ 수신 데이터 없음 (CIP 응답 payload 비어있음) |
| **State** | `0xC0000004` | ❗❗ **오류 상태 코드** |
| **Cmd** | `0x00008021` | `RCX_RECEIVE_PACKET_CNF` (수신 확인 응답) |

---

## 2. 에러 코드 해석

### 2.1 State = 0xC0000004 의미

```
0xC0000004 = 0xC0000000 (Critical error + confirm) + 0x00000004 (Invalid Parameter/Channel/Route)
```

**해석**:
> "RCX_RECEIVE_PACKET_CNF 명령은 정상 반환되었지만, 내부적으로 패킷 처리 실패"
> (대상 채널, 라우트, 또는 데이터 구조 오류)

### 2.2 통신 구간별 상태

| 구간 | 상태 | 설명 |
|------|------|------|
| netHOST → RCX_SEND_PACKET_REQ | ✅ 정상 | 패킷이 netX로 전달됨 |
| netX90 DeviceNet 스택 | ❌ 실패 | CIP Explicit 메시지로 인식 안 됨 (라우팅/데이터 오류) |
| STM32 앱 (App_AllChannels_PacketHandler) | 🚫 미수신 | CIP 요청이 DPM에 도달하지 않음 |
| RCX 응답 (RCX_RECEIVE_PACKET_CNF) | ⚠️ 형식만 정상 | "수신 확인했지만 처리 실패" 의미 |

---

## 3. 원인 분석

### 3.1 가능한 원인 5가지

| 순위 | 원인 | 상세 설명 | 확인 방법 |
|------|------|----------|----------|
| **1** | **UCMM 비활성화** | `AppDNS_ConfigureStack()` 또는 EDS에서 UCMM 기능 OFF | `bUCMMCapable = 1` 확인 필요 |
| **2** | **채널 ID 오류** | RCX Packet `Dest`/`Route`가 I/O용(0)으로 설정 | Explicit용 Channel ID 별도 필요 여부 확인 |
| **3** | **CIP 요청 구조 오류** | Path size, segment type, length mismatch | PathSize 계산 (바이트/2) 검증 |
| **4** | **CIP Fragmentation 오류** | Len>8바이트, 스택 분할처리 실패 | 8바이트 이하 짧은 명령으로 테스트 |
| **5** | **BUS 상태 문제** | BUS OFF / Duplicate MAC | BUS ON 상태 확인 |

### 3.2 CIP 요청 패킷 구조 검증

현재 보낸 데이터 예시:
```
10 03 20 30 24 01 30 0C 05
```

- `0x10` = Set_Attribute_Single ✅
- `0x03` = Path size in words → (0x03 × 2 = 6 bytes) ✅
- Path: `20 30 24 01 30 0C` (Class 0x30, Instance 0x01, Attribute 0x0C) ✅
- Data: `05` (1 byte) ✅

**구조적으로는 정상**이나, netX CIP 스택이 "경로가 너무 짧다"로 처리했을 가능성 있음.

---

## 4. EDS 파일 분석 시도

### 4.1 분석 대상
- 파일명: `476297.eds`
- 목적: UCMM/Explicit 메시징 지원 여부 확인

### 4.2 분석 결과
❌ **EDS 파일 파싱 실패**
- Python configparser로 파싱 시도했으나 여러 오류 발생:
  1. Missing Section Header Error
  2. Parsing Error ($ 주석 라인 처리 문제)

### 4.3 필요한 EDS 확인 사항

EDS 파일에서 다음 항목 수동 확인 필요:

```ini
[Device]
UCMMCapable = ?        # 1이어야 Explicit 지원
Group2OnlyServer = ?   # 0이어야 UCMM 허용
```

---

## 5. 권장 조치 사항

### 5.1 즉시 확인 사항 (우선순위 순)

#### 1단계: UCMM 활성화 확인
```c
// AppDNS_ConfigureStack() 내부 확인
tDeviceNetConfig tCfg;
tCfg.bUCMMCapable = 1;      // ← 반드시 1이어야 함
tCfg.bGroup2OnlyServer = 0; // ← 반드시 0이어야 함
```

#### 2단계: Channel 및 Route 수정
```c
// netHOST 측 RCX 패킷 송신 시
tRCX_PACKET tPkt = {0};
tPkt.tHead.ulDest = 0x00000001;  // DeviceNet channel (slave) - 0이 아닌 값 필요
tPkt.tHead.ulCmd = RCX_SEND_PACKET_REQ;
tPkt.tHead.ulLen = cip_data_len;
memcpy(tPkt.abData, cip_payload, cip_data_len);
```

#### 3단계: 간단한 테스트 패킷으로 검증
GET 명령으로 변경하여 테스트:
```
0E 03 20 30 24 01 30 0C
```
- `0x0E` = Get_Attribute_Single
- Set보다 간단하여 디버깅에 유리

#### 4단계: BUS 상태 확인
```c
// BUS ON 상태 확인
// Duplicate MAC 없는지 확인
```

### 5.2 디버깅 로그 활성화

Hilscher DeviceNet Stack 내부 디버그:
- netX90 UART 디버그 활성화
- CIFX Diagnostic Tool 사용
- "Received UCMM Request" 로그 확인

---

## 6. 미해결 사항

1. **EDS 파일 정확한 UCMM 설정 값 미확인**
   - 476297.eds 파일을 수동으로 텍스트 에디터로 열어 확인 필요
   - `UCMMCapable`과 `Group2OnlyServer` 값 확인

2. **실제 사용 중인 코드 미확인**
   - `AppDNS_ConfigureStack()` 함수 내용
   - `RCX_SEND_PACKET_REQ` 생성 코드 (netHOST 측)

3. **netX 로그 미확인**
   - 스택 레벨에서 패킷을 어떻게 처리/거부했는지 확인 필요

---

## 7. 결론

### 핵심 문제
**State 0xC0000004 = UCMM 요청이 netX에서 "처리 불가"로 거부됨**

### 가장 가능성 높은 원인
1. **UCMM 기능 비활성화** (bUCMMCapable = 0 또는 Group2OnlyServer = 1)
2. **Route/Dest ID 설정 오류** (0x00000000으로 설정되어 있음)

### 다음 단계
1. `AppDNS_ConfigureStack()` 코드 확인 → UCMM 활성화
2. netHOST RCX 패킷 Dest/Route 수정
3. 짧은 GET 명령으로 재테스트
4. EDS 파일 수동 확인

---

## 8. 참고 자료

### 8.1 CIP Explicit Request 구조
```
Service(1B) | PathSizeInWords(1B) | CIP Path(2*PathSize bytes) | Request Data(N bytes)
```

### 8.2 CIP Path 예시 (8-bit 세그먼트)
```
0x20 0x30   # Class segment (8-bit class) = 0x30
0x24 0x01   # Instance segment (8-bit instance) = 0x01
0x30 0x0C   # Attribute segment (8-bit attribute) = 0x0C
```

### 8.3 STM32 Explicit 핸들러 위치
```c
while (g_tAppData.fRunning && lRet == CIFX_NO_ERROR) {
    App_IODataHandler(&g_tAppData);                      // I/O(사이클릭) 처리
    lRet = App_AllChannels_PacketHandler(&g_tAppData);   // Mailbox(Explicit 등) 처리
    OS_Sleep(1);
}
```

---

**작성자 주석**:
이 문서는 ChatGPT와의 대화 내용(`chagpt_explicit_msg_error.md`)을 기반으로 작성되었으며,
실제 코드 확인 및 EDS 파일 정확한 분석 후 업데이트가 필요합니다.

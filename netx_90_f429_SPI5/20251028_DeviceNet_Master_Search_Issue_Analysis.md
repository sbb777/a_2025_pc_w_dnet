# DeviceNet Master 검색 실패 원인 분석

**작성일시**: 2025-10-28
**작업자**: Claude
**문제**: DeviceNet Master Identity 조회 시 검색 실패

---

## 1. 문제 상황

### 1.1 증상
- `DeviceNet_ScanForMaster()` 실행 시 Master 검색 실패
- `DeviceNet_GetMasterIdentity()` 호출 시 응답 없음 또는 타임아웃
- "No Master Found on Network" 메시지 출력

### 1.2 사용자 보고
> "최초 작업전 코드에서는 master가 network scan 하면 검출이 되었는데 현재는 검색이 되지 않는다"

---

## 2. 근본 원인 분석

### 2.1 아키텍처 미스매치

**현재 구현의 전제 조건**:
```
STM32 (Host)
    ↓ SPI
netX90 (DeviceNet Slave)  ← 우리의 위치
    ↓ DeviceNet Network
Master (Scanner/PLC)      ← 조회 대상
```

**문제점**:
- **netX90은 DeviceNet Slave로 동작**
- **Slave는 Master를 직접 조회할 수 없음**
- DeviceNet 프로토콜에서 정보는 Master → Slave 방향으로 흐름

### 2.2 DNS_CMD_CIP_SERVICE_REQ의 제한사항

#### 헤더 파일 명세 (DNS_packet_cip_service.h:27-28)
```c
/**
 * This packet is used to perform a CIP service from the host application
 * to any object of the local DeviceNet Slave stack.
 */
```

**핵심**: "**local DeviceNet Slave stack**"

#### 의미
- DNS_CMD_CIP_SERVICE_REQ는 **로컬 객체만** 조회 가능
- 원격 Master 조회 불가능
- Instance로 Node ID를 사용하는 것은 잘못된 접근

### 2.3 CIP 객체 모델의 이해

```
┌─────────────────────────────────────┐
│  Master (Node 0)                    │
│  ├─ Identity Object (Instance 1)    │  ← 이것을 조회하려 함
│  ├─ Assembly Object                 │
│  └─ Connection Manager              │
└─────────────────────────────────────┘
            ↓ DeviceNet Network
┌─────────────────────────────────────┐
│  netX90 Slave (Node 10)             │  ← 우리의 위치
│  ├─ Identity Object (Instance 1)    │  ← 이것만 조회 가능
│  ├─ Assembly Object                 │
│  └─ Connection Manager              │
└─────────────────────────────────────┘
```

**DNS_CMD_CIP_SERVICE_REQ로 조회 가능**:
- netX90 Slave의 로컬 객체만

**DNS_CMD_CIP_SERVICE_REQ로 조회 불가능**:
- 원격 Master의 객체
- 다른 Slave의 객체

---

## 3. DeviceNet 통신 방식 비교

### 3.1 I/O Messaging (현재 VAT 테스트에서 사용 중)

```
Master → Assembly Object (Producing) → Slave
Slave → Assembly Object (Consuming) → Master
```

**특징**:
- 주기적 데이터 교환
- Pressure, Setpoint 등 실시간 데이터
- **정상 작동 중** (VAT 테스트 성공)

### 3.2 Explicit Messaging (Master Identity 조회 시도)

#### 3.2.1 Local Explicit Messaging (현재 구현)
```c
Host (STM32) → DNS_CMD_CIP_SERVICE_REQ → netX90 Local Stack
```
- **용도**: netX90 Slave 자신의 객체 조회
- **결과**: 성공 (로컬 객체 접근)

#### 3.2.2 Remote Explicit Messaging (필요한 기능)
```
Host (STM32) → netX90 → DeviceNet Network → Master
```
- **필요 조건**: Explicit Message Forwarding 기능
- **현재 상태**: 미구현
- **결과**: 실패 (원격 접근 불가)

---

## 4. 왜 이전에는 작동했는가?

### 4.1 가능한 시나리오

#### 시나리오 1: 착오
- **이전 코드 분석 결과**: Master 조회 기능 없었음
- **git commit 442425a**: slave 정보만 출력
- **결론**: 이전에도 Master를 조회한 적 없음

#### 시나리오 2: 다른 도구 사용
- RSNetWorx for DeviceNet 등 네트워크 설정 도구
- 이 도구들은 Master에서 실행되어 네트워크 스캔
- **차이점**: Master → Slave 방향 조회 (정상)

#### 시나리오 3: Master 설정
- Master가 자신의 Identity를 Slave에게 전송하도록 설정
- Slave는 수신한 데이터를 표시만 함
- **방향**: Master → Slave (정상)

### 4.2 현재 구현의 시도
```
Slave → Master 방향으로 Identity 조회
```
- **문제**: 프로토콜 위반
- **결과**: 실패

---

## 5. DeviceNet Slave API 한계

### 5.1 사용 가능한 패킷 명령

| 명령 | 용도 | 대상 |
|------|------|------|
| DNS_CMD_CIP_SERVICE_REQ | CIP Service 요청 | **로컬 객체** |
| DNS_CMD_REGISTER_CLASS_REQ | 클래스 등록 | 로컬 스택 |
| DNS_CMD_CREATE_ASSEMBLY_REQ | Assembly 생성 | 로컬 스택 |
| DNS_CMD_SET_CONFIGURATION_REQ | 설정 변경 | 로컬 스택 |

### 5.2 없는 기능
- ❌ Remote Explicit Message 전송
- ❌ Master 정보 조회
- ❌ 네트워크 스캔
- ❌ 다른 노드 검색

### 5.3 이유
DeviceNet Slave는:
- Master의 명령에 **응답**하는 역할
- 스스로 네트워크를 **조회하지 않음**
- 정보는 Master → Slave 방향으로만 흐름

---

## 6. 올바른 접근 방법

### 6.1 방법 1: Master 측에서 정보 제공 (권장)

#### Master 설정
```
Master Device Profile
  └─ Identity Broadcasting
      - Vendor ID
      - Product Name
      - Firmware Version
```

#### Slave 수신
```c
// Master가 보낸 데이터를 Assembly Object로 수신
// 또는 Configuration 시점에 전달받음
```

**장점**:
- 프로토콜 준수
- 구현 간단
- 신뢰성 높음

**단점**:
- Master 설정 필요
- 표준화되지 않을 수 있음

### 6.2 방법 2: 네트워크 설정 도구 사용

#### RSNetWorx for DeviceNet
```
PC (RSNetWorx)
  ↓ DeviceNet Network
Master → 정보 조회 및 표시
Slave → 정보 조회 및 표시
```

**장점**:
- 모든 노드 정보 확인 가능
- 표준 도구
- 네트워크 진단 기능

**단점**:
- 별도 PC 및 소프트웨어 필요
- 런타임 자동화 불가

### 6.3 방법 3: Master 펌웨어 수정

#### Master에 커스텀 기능 추가
```c
// Master 측 코드
void SendIdentityToSlave(uint8_t slaveNodeId)
{
    // Identity 정보를 Slave에게 전송
    SendExplicitMessage(slaveNodeId, IDENTITY_DATA);
}
```

**장점**:
- 완전한 제어
- 자동화 가능

**단점**:
- Master 펌웨어 수정 필요
- Master 접근 권한 필요
- 유지보수 복잡

### 6.4 방법 4: Proxy/Gateway 사용

#### 중간 Gateway 노드
```
Master
  ↓
Gateway (Master 정보 수집 및 재전송)
  ↓
netX90 Slave → STM32
```

**장점**:
- 기존 시스템 수정 불필요
- 유연성

**단점**:
- 추가 하드웨어 필요
- 복잡도 증가

---

## 7. 현재 구현 코드의 문제점

### 7.1 잘못된 가정

```c
// devicenet_master_info.c:63
tSendPkt.tData.ulInstance = masterNodeId;  // ❌ 잘못된 사용
```

**문제**:
- Instance는 "로컬 객체 인스턴스"를 의미
- Node ID를 Instance로 사용할 수 없음
- Master의 객체에 접근할 수 없음

### 7.2 API 한계 무시

```c
// DNS_CMD_CIP_SERVICE_REQ는 로컬 객체 전용
lRet = xChannelPutPacket(hChannel, &tSendPkt, TIMEOUT);
```

**문제**:
- 원격 노드 조회 시도
- 프로토콜 위반
- 응답 없음 또는 에러

### 7.3 프로토콜 미스매치

```
의도: Slave → Master 정보 조회
실제: Slave → 로컬 스택 조회
결과: Master 정보를 얻을 수 없음
```

---

## 8. 테스트 결과 해석

### 8.1 예상 출력

```
========================================
 DeviceNet Master Identity
========================================

=== Scanning DeviceNet Network for Master ===
Trying Node 0...
Error: Failed to receive CIP service response (0x80000009)
Trying Node 63...
Error: Failed to receive CIP service response (0x80000009)
...
=== No Master Found on Network ===

ERROR: Could not retrieve Master identity
Please verify:
  1. DeviceNet Master is powered on
  2. Network cables are connected
  3. Network termination is correct
  4. Master node address configuration
```

### 8.2 에러 원인

**CIFX_DEV_GET_TIMEOUT (0x80000009)**:
- 패킷 응답 타임아웃
- **근본 원인**: 잘못된 패킷 전송
- netX90 스택이 요청을 처리할 수 없음
- 원격 Master 조회 불가능

### 8.3 올바른 동작

현재 구현으로는:
```c
// 로컬 slave의 Identity는 조회 가능
DeviceNet_GetMasterIdentity(hChannel, 1, &identity);
// Instance 1 = 로컬 Identity Object
```

하지만 이것은:
- **netX90 Slave 자신의 정보**
- Master 정보 아님

---

## 9. 실제 통신 확인

### 9.1 VAT 테스트 로그 재확인

```
COS Flags: 0x00000001 (HIL_COMM_COS_READY)
Error Count: 0
Pressure: 0
```

**해석**:
- **COS_READY**: Master와 I/O 통신 **정상**
- **Error Count 0**: 통신 에러 없음
- **Pressure 0**: Mock 함수 또는 실제 VAT 미연결

### 9.2 I/O Messaging vs Explicit Messaging

| 구분 | I/O Messaging | Explicit Messaging |
|------|---------------|-------------------|
| 상태 | ✅ 정상 작동 | ❌ 불가능 |
| 방향 | Master ↔ Slave | Slave → Master (불가) |
| 용도 | 실시간 데이터 | 설정/진단 |
| 구현 | VAT 테스트 | 현재 시도 |

---

## 10. 해결 방안 권장사항

### 10.1 즉시 적용 가능 (권장)

#### Option A: 네트워크 설정 도구 사용
1. RSNetWorx for DeviceNet 설치
2. DeviceNet 네트워크 스캔
3. Master 정보 확인 및 기록
4. 문서화

**장점**:
- 코드 수정 불필요
- 즉시 확인 가능
- 표준 방법

#### Option B: Master 문서 참조
1. Master 장치 매뉴얼 확인
2. Vendor ID, Product Name, Firmware Version 기록
3. STM32 코드에 상수로 정의

```c
// main.c
#define MASTER_VENDOR_ID      0x????
#define MASTER_PRODUCT_CODE   0x????
#define MASTER_PRODUCT_NAME   "Master Device Name"
#define MASTER_FW_VERSION     "1.2.3"

void PrintMasterInformation(void)
{
    printf("Master Information (from documentation):\r\n");
    printf("  Vendor ID: 0x%04X\r\n", MASTER_VENDOR_ID);
    printf("  Product: %s\r\n", MASTER_PRODUCT_NAME);
    printf("  Firmware: %s\r\n", MASTER_FW_VERSION);
}
```

### 10.2 중기 솔루션

#### Master 측 Identity Broadcasting 설정
1. Master 설정 도구 접속
2. Identity Broadcasting 활성화
3. Slave에서 수신 코드 작성

```c
// 새로운 함수
int32_t DeviceNet_ReceiveMasterIdentity(CIFXHANDLE hChannel,
                                         MASTER_IDENTITY_T* pIdentity)
{
    // Master가 보낸 데이터를 특정 Assembly Object로 수신
    // (Master 설정에 따라 구현)
}
```

### 10.3 장기 솔루션

#### Custom Explicit Message Forwarding
1. netX90 펌웨어 확장 (Hilscher 지원 필요)
2. Remote Explicit Messaging 지원
3. Master 조회 기능 구현

**참고**: Hilscher 기술 지원 문의 필요

---

## 11. 코드 수정 권장사항

### 11.1 현재 코드 제거 또는 수정

```c
// main.c:347-384
// 현재 Master 조회 코드 제거 또는 주석 처리
#if 0  // Master 직접 조회 불가능으로 비활성화
    /* 4. DeviceNet Master Identity Information */
    printf("========================================\r\n");
    printf(" DeviceNet Master Identity\r\n");
    printf("========================================\r\n");
    ...
#endif
```

### 11.2 대체 구현

```c
// main.c에 추가
static void PrintMasterInformationFromDoc(void)
{
    printf("========================================\r\n");
    printf(" DeviceNet Master Information\r\n");
    printf(" (From Device Documentation)\r\n");
    printf("========================================\r\n");

    printf("\r\n");
    printf("Note: DeviceNet Slave cannot directly query\r\n");
    printf("      Master information via Explicit Messaging.\r\n");
    printf("\r\n");
    printf("To obtain Master information:\r\n");
    printf("  1. Use RSNetWorx or similar network tools\r\n");
    printf("  2. Refer to Master device documentation\r\n");
    printf("  3. Configure Master to broadcast Identity\r\n");
    printf("\r\n");

    // 문서화된 정보 출력
    printf("[Known Master Information]\r\n");
    printf("  Vendor ID:    0x???? (Check documentation)\r\n");
    printf("  Product Name: ????? (Check documentation)\r\n");
    printf("  Firmware:     ?.?.? (Check documentation)\r\n");

    printf("========================================\r\n\r\n");
}
```

---

## 12. 결론

### 12.1 핵심 요약

1. **근본 원인**: DeviceNet Slave는 Master를 직접 조회할 수 없음
2. **API 한계**: DNS_CMD_CIP_SERVICE_REQ는 로컬 객체 전용
3. **프로토콜 제약**: 정보는 Master → Slave 방향으로만 흐름
4. **현재 상태**: I/O Messaging은 정상, Explicit Messaging 불가

### 12.2 액션 아이템

#### 즉시 조치
- [ ] 현재 Master 조회 코드 비활성화
- [ ] 네트워크 설정 도구로 Master 정보 확인
- [ ] Master 문서 참조하여 정보 기록

#### 추가 검토
- [ ] Master Identity Broadcasting 설정 가능 여부 확인
- [ ] Master 측 커스텀 기능 추가 가능성 검토
- [ ] Hilscher 기술 지원 문의 (Remote Explicit Messaging)

### 12.3 현실적 해결책

**가장 실용적인 방법**:
1. RSNetWorx 등 네트워크 도구로 Master 정보 확인
2. 확인한 정보를 STM32 코드에 상수로 정의
3. 런타임에 정적 정보 출력

**이유**:
- 즉시 적용 가능
- 코드 수정 최소화
- 추가 비용 없음
- DeviceNet 표준 준수

---

## 13. 참고 자료

### 13.1 DeviceNet 사양
- ODVA DeviceNet Specification Volume I
- CIP Networks Library: DeviceNet Adaptation of CIP

### 13.2 관련 문서
- `20251028_DeviceNet_Master_vs_Slave_Information_Analysis.md`
- `20251028_DeviceNet_Master_Identity_CIP_Explicit_Messaging.md`

### 13.3 Hilscher 문서
- DeviceNet Slave Stack Manual
- cifX API Reference
- DNS Packet Interface Specification

---

**작성 완료일시**: 2025-10-28
**문서 버전**: 1.0
**권장 조치**: 네트워크 설정 도구 사용 또는 Master 문서 참조

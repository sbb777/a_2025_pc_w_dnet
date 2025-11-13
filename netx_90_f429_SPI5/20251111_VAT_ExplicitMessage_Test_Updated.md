# VAT Explicit Message 테스트 가이드 (업데이트)

**작성일**: 2025-11-11
**목적**: VAT Valve 특성 파라미터 Explicit Message 테스트

---

## VAT Valve 파라미터 (Class 0x64)

### 구현된 파라미터 목록

| ID | 이름 | 타입 | 단위 | 접근 | 기본값 | 범위 | 설명 |
|----|------|------|------|------|--------|------|------|
| 1 | Pressure Setpoint | INT16 | 0.01 mbar | RW | 5000 (50.00 mbar) | 0-10000 | 목표 압력 설정값 |
| 2 | Position Setpoint | INT16 | 0.01% | RW | 5000 (50.00%) | 0-10000 | 목표 위치 설정값 |
| 3 | Control Mode | UINT8 | enum | RW | 0 (Pressure) | 0-5 | 제어 모드 |
| 4 | Actual Pressure | INT16 | 0.01 mbar | RO | 0 | 0-10000 | 현재 압력 값 |
| 5 | Actual Position | INT16 | 0.01% | RO | 0 | 0-10000 | 현재 위치 값 |
| 6 | Device Status | UINT8 | enum | RO | 1 (Idle) | 0-4 | 장치 상태 |
| 7 | Exception Status | UINT8 | bitmap | RO | 0 | 0-255 | 에러 상태 |
| 8 | Access Mode | UINT8 | enum | RO | 1 (Remote) | 0-1 | 제어 모드 |
| 9 | Pressure Upper Limit | INT16 | 0.01 mbar | RW | 10000 | 0-10000 | 압력 상한 |
| 10 | Pressure Lower Limit | INT16 | 0.01 mbar | RW | 0 | 0-10000 | 압력 하한 |

### Control Mode (Param 3) 값

| 값 | 이름 | 설명 |
|----|------|------|
| 0 | Pressure Control | 압력 제어 모드 |
| 1 | Position Control | 위치 제어 모드 |
| 2 | Open | 밸브 전개 |
| 3 | Close | 밸브 폐쇄 |
| 4 | Hold | 현재 상태 유지 |
| 5 | Learn | 학습 모드 |

### Device Status (Param 6) 값

| 값 | 이름 | 설명 |
|----|------|------|
| 0 | Init | 초기화 중 |
| 1 | Idle | 대기 상태 |
| 2 | Controlling | 제어 중 |
| 3 | Error | 에러 상태 |
| 4 | Learn | 학습 중 |

---

## VAT Diagnostic Object (Class 0x65)

### 구현된 Attributes

| Attr | 이름 | 타입 | 설명 |
|------|------|------|------|
| 1 | Uptime | UINT32 | 가동 시간 (초) |
| 2 | Total Cycles | UINT32 | 총 제어 사이클 수 |
| 3 | Error Count | UINT16 | 총 에러 발생 횟수 |
| 4 | Last Error Code | UINT16 | 마지막 에러 코드 |
| 5 | Last Error Timestamp | UINT32 | 마지막 에러 시각 (초) |
| 6 | Pressure Min | INT16 | 최소 압력 기록 |
| 7 | Pressure Max | INT16 | 최대 압력 기록 |
| 8 | Pressure Avg | INT16 | 평균 압력 (100 샘플) |
| 9 | Position Min | INT16 | 최소 위치 기록 |
| 10 | Position Max | INT16 | 최대 위치 기록 |
| 11 | Network RX Count | UINT32 | 수신 패킷 수 |
| 12 | Network TX Count | UINT32 | 송신 패킷 수 |
| 13 | Network Error Count | UINT16 | 통신 에러 수 |
| 14 | Temperature | INT16 | 시스템 온도 (°C) |
| 15 | Firmware Version | UINT32 | 펌웨어 버전 |

---

## netHOST 테스트 시나리오

## 테스트 1: Pressure Setpoint 읽기 (Param 1)

### 목적
현재 압력 설정값 읽기

### 패킷 설정
```
Cmd:         0x0000308A
Len:         20
Data (HEX):
0E 00 00 00    // Service: 0x0E (Get_Attribute_Single)
64 00 00 00    // Class: 0x64 (VAT Parameter)
01 00 00 00    // Instance: 1 (Pressure Setpoint)
01 00 00 00    // Attribute: 1
01 00 00 00    // Member: 1 (Device/Node ID)
```

**중요**: DNS_CIP_SERVICE_REQ_T 구조체는 5개 필드 (Service, Class, Instance, Attribute, **Member**)를 포함합니다.
Member 필드는 일반적으로 Device ID 또는 배열 인덱스로 사용됩니다.

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
00 00 00 00    // GRC: 0x00 (Success)
00 00 00 00    // ERC: 0x00
88 13          // Value: 5000 = 0x1388 (little-endian, 50.00 mbar)
```

### 예상 시리얼 출력
```
=== CIP Service Indication ===
Service:   0x0E (Get Attribute Single)
Class:     0x64
Instance:  0x01
Attribute: 0x01
  -> Routing to VAT Handler
[VAT] CIP Service: 0x0E, Class: 0x64, Inst: 1, Attr: 1
[VAT] GET Param1: 88 13
[VAT] Response sent: GRC=0x00, DataSize=2
```

---

## 테스트 2: Pressure Setpoint 쓰기 (Param 1)

### 목적
압력 설정값을 70.00 mbar (7000)로 변경

### 패킷 설정
```
Cmd:         0x0000308A
Len:         22
Data (HEX):
10 00 00 00    // Service: 0x10 (Set_Attribute_Single)
64 00 00 00    // Class: 0x64
01 00 00 00    // Instance: 1
01 00 00 00    // Attribute: 1
01 00 00 00    // Member: 1 (Device/Node ID)
58 1B          // Data: 7000 = 0x1B58 (little-endian)
```

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
00 00 00 00    // GRC: 0x00 (Success)
00 00 00 00    // ERC: 0x00
```

### 예상 시리얼 출력
```
[VAT] CIP Service: 0x10, Class: 0x64, Inst: 1, Attr: 1
[VAT] SET Param1: 58 1B
[VAT] Response sent: GRC=0x00, DataSize=0
```

---

## 테스트 3: Control Mode 읽기 (Param 3)

### 목적
현재 제어 모드 확인

### 패킷 설정
```
Cmd:         0x0000308A
Len:         16
Data (HEX):
0E 00 00 00    // Service: 0x0E
64 00 00 00    // Class: 0x64
03 00 00 00    // Instance: 3 (Control Mode)
01 00 00 00    // Attribute: 1
```

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
00 00 00 00    // GRC: 0x00
00 00 00 00    // ERC: 0x00
00             // Value: 0 (Pressure Control)
```

---

## 테스트 4: Control Mode 변경 (Param 3)

### 목적
Position Control 모드로 변경

### 패킷 설정
```
Cmd:         0x0000308A
Len:         17
Data (HEX):
10 00 00 00    // Service: 0x10
64 00 00 00    // Class: 0x64
03 00 00 00    // Instance: 3
01 00 00 00    // Attribute: 1
01             // Data: 1 (Position Control)
```

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
00 00 00 00    // GRC: 0x00
00 00 00 00    // ERC: 0x00
```

---

## 테스트 5: Actual Pressure 읽기 (Param 4 - Read-Only)

### 목적
현재 실제 압력 값 읽기

### 패킷 설정
```
Cmd:         0x0000308A
Len:         16
Data (HEX):
0E 00 00 00    // Service: 0x0E
64 00 00 00    // Class: 0x64
04 00 00 00    // Instance: 4 (Actual Pressure)
01 00 00 00    // Attribute: 1
```

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
00 00 00 00    // GRC: 0x00
00 00 00 00    // ERC: 0x00
00 00          // Value: 0 (초기값, I/O에서 업데이트됨)
```

---

## 테스트 6: Read-Only Parameter 쓰기 시도 (에러 테스트)

### 목적
Read-Only 파라미터 쓰기 시도하여 에러 확인

### 패킷 설정
```
Cmd:         0x0000308A
Len:         20
Data (HEX):
10 00 00 00    // Service: 0x10 (Set)
64 00 00 00    // Class: 0x64
04 00 00 00    // Instance: 4 (Actual Pressure - READ ONLY)
01 00 00 00    // Attribute: 1
64 00          // Data: 100
```

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
02 00 00 00    // GRC: 0x02 (Too Much Data - Read-Only Error)
00 00 00 00    // ERC: 0x00
```

---

## 테스트 7: 범위 초과 값 쓰기 (에러 테스트)

### 목적
Pressure Setpoint에 범위 초과 값 쓰기

### 패킷 설정
```
Cmd:         0x0000308A
Len:         20
Data (HEX):
10 00 00 00    // Service: 0x10
64 00 00 00    // Class: 0x64
01 00 00 00    // Instance: 1
01 00 00 00    // Attribute: 1
10 27          // Data: 10000 = 0x2710 (OK)
or
11 27          // Data: 10001 = 0x2711 (Out of range)
```

### 예상 응답 (범위 초과)
```
Cmd:         0x0000308B
State:       0x00000000
Data:
09 00 00 00    // GRC: 0x09 (Invalid Attribute Value)
00 00 00 00    // ERC: 0x00
```

---

## 테스트 8: Device Status 읽기 (Param 6)

### 패킷 설정
```
Cmd:         0x0000308A
Len:         16
Data (HEX):
0E 00 00 00    // Service: 0x0E
64 00 00 00    // Class: 0x64
06 00 00 00    // Instance: 6 (Device Status)
01 00 00 00    // Attribute: 1
```

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
00 00 00 00    // GRC: 0x00
00 00 00 00    // ERC: 0x00
01             // Value: 1 (Idle)
```

---

## 테스트 9: Diagnostic Object - Uptime 읽기

### 패킷 설정
```
Cmd:         0x0000308A
Len:         16
Data (HEX):
0E 00 00 00    // Service: 0x0E
65 00 00 00    // Class: 0x65 (VAT Diagnostic)
01 00 00 00    // Instance: 1
01 00 00 00    // Attribute: 1 (Uptime)
```

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
00 00 00 00    // GRC: 0x00
00 00 00 00    // ERC: 0x00
[4 bytes]      // UINT32 Uptime (seconds since boot)
```

---

## 테스트 10: Diagnostic Object - Get All

### 패킷 설정
```
Cmd:         0x0000308A
Len:         12
Data (HEX):
01 00 00 00    // Service: 0x01 (Get_Attributes_All)
65 00 00 00    // Class: 0x65
01 00 00 00    // Instance: 1
```

### 예상 응답
```
Cmd:         0x0000308B
State:       0x00000000
Data:
00 00 00 00    // GRC: 0x00
00 00 00 00    // ERC: 0x00
[30 bytes]     // All diagnostic data
```

---

## 성공 기준

### 기본 기능
- [x] Parameter 1-10 구현 완료
- [ ] **Pressure Setpoint 읽기 성공** (테스트 1)
- [ ] **Pressure Setpoint 쓰기 성공** (테스트 2)
- [ ] **Control Mode 읽기/쓰기 성공** (테스트 3, 4)
- [ ] **Actual Pressure 읽기 성공** (테스트 5)

### 에러 처리
- [ ] **Read-Only 쓰기 에러 반환** (테스트 6)
- [ ] **범위 초과 에러 반환** (테스트 7)

### 진단 기능
- [ ] **Device Status 읽기** (테스트 8)
- [ ] **Diagnostic Uptime 읽기** (테스트 9)
- [ ] **Diagnostic Get All** (테스트 10)

---

## 파라미터 값 해석 방법

### Pressure/Position 값 (INT16)
```
실제값 = 읽은값 / 100
예: 0x1388 (5000) = 50.00 mbar
예: 0x1B58 (7000) = 70.00 mbar
```

### Little-Endian 변환
```
netHOST에서 보내는 값: 7000 (0x1B58)
바이트 순서: 58 1B (low byte first)
```

---

**작성 완료**: 2025-11-11

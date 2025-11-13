# DNS_V5_SIMPLE_CONFIG_DEMO.EDS 파일 분석 및 IO 데이터 통신 설명
**작성일시**: 2025-10-27

---

## 1. EDS 파일 개요

### 📋 Electronic Data Sheet (EDS) 란?

**EDS 파일**은 DeviceNet, EtherNet/IP 등의 산업용 네트워크에서 사용되는 장치 설명 파일로, 다음 정보를 포함합니다:
- 장치 식별 정보 (제조사, 제품명, 버전)
- 지원하는 통신 모드 (Poll, Strobe, COS, Cyclic)
- 입출력 데이터 구조 및 크기
- 네트워크 설정 파라미터

### 📄 파일 정보

| 항목 | 값 |
|------|-----|
| **파일명** | DNS_V5_SIMPLE_CONFIG_DEMO.EDS |
| **프로토콜** | DeviceNet Slave (DNS) V5 |
| **버전** | 1.1 |
| **생성도구** | EZ-EDS Version 3.27.1.20191218 |
| **작성일** | 2021-01-01 |
| **제조사** | Hilscher Gesellschaft fuer Systemautomation mbH |

---

## 2. 파일 구조 분석

### 📦 [File] 섹션

장치 설명 파일의 메타데이터를 정의합니다.

```ini
[File]
        DescText = "DNS-File for DeviceNet Slave Simple Config Demo Application";
        CreateDate = 01-01-2021;
        CreateTime = 00:00:00;
        ModDate = 01-01-2021;
        ModTime = 00:00:00;
        Revision = 1.1;
        HomeURL = "www.hilscher.com";
```

| 필드 | 설명 | 값 |
|------|------|-----|
| **DescText** | 파일 설명 | DeviceNet Slave 단순 설정 데모 |
| **CreateDate** | 생성 날짜 | 2021-01-01 |
| **Revision** | 파일 버전 | 1.1 |
| **HomeURL** | 제조사 웹사이트 | www.hilscher.com |

---

### 🏭 [Device] 섹션

DeviceNet 장치의 식별 정보를 정의합니다.

```ini
[Device]
        VendCode = 283;
        VendName = "Hilscher GmbH";
        ProdType = 12;
        ProdTypeStr = "Communications Adapter";
        ProdCode = 34;
        MajRev = 5;
        MinRev = 1;
        ProdName = "DNS_V5_SIMPLE_CONFIG_DEMO";
        Catalog = "0";
        Icon = "NETX.ICO";
```

#### 장치 식별 정보

| 필드 | 의미 | 값 | 설명 |
|------|------|-----|------|
| **VendCode** | 제조사 코드 | 283 | DeviceNet 표준 제조사 ID (Hilscher) |
| **VendName** | 제조사 이름 | Hilscher GmbH | 독일 산업 자동화 솔루션 제조사 |
| **ProdType** | 제품 타입 | 12 | Communications Adapter 카테고리 |
| **ProdTypeStr** | 제품 타입 문자열 | Communications Adapter | 통신 어댑터 장치 |
| **ProdCode** | 제품 코드 | 34 | Hilscher 내부 제품 식별 코드 |
| **MajRev** | 주 버전 | 5 | DNS V5 프로토콜 버전 |
| **MinRev** | 부 버전 | 1 | 마이너 업데이트 버전 |
| **ProdName** | 제품명 | DNS_V5_SIMPLE_CONFIG_DEMO | 데모 애플리케이션 |

#### DeviceNet Vendor Code 참고
- **283 (0x11B)**: Hilscher Gesellschaft fuer Systemautomation mbH
- ODVA (Open DeviceNet Vendor Association)에서 할당한 공식 제조사 코드

---

## 3. I/O 통신 특성 분석

### 🔄 [IO_Info] 섹션

DeviceNet의 4가지 통신 모드와 입출력 데이터 매핑을 정의합니다.

#### 3.1 기본 설정

```ini
Default = 0x0001;       $ poll as default
```

| 값 | 의미 |
|-----|------|
| **0x0001** | Poll 모드를 기본 통신 방식으로 설정 |

---

#### 3.2 Poll 모드

**마스터가 주기적으로 슬레이브를 폴링하여 데이터 교환**

```ini
PollInfo =
        0x000F,         $ OK to Combine w/Poll/Strobe/COS/Cyclic
        1,              $ Default Input  = Input1
        1;              $ Default Output = Output1
```

| 필드 | 값 | 설명 |
|------|-----|------|
| **Combination Flag** | 0x000F | 모든 모드와 조합 가능 (Poll/Strobe/COS/Cyclic) |
| **Default Input** | 1 | Input1 구조 사용 (10바이트) |
| **Default Output** | 1 | Output1 구조 사용 (6바이트) |

**동작 방식**:
1. 마스터 → `Polling Request` → 슬레이브
2. 슬레이브 → `Input Data (10 bytes)` + `Output Data (6 bytes)` → 마스터
3. 주기적으로 반복 (일반적으로 10~100ms)

---

#### 3.3 Strobe 모드

**마스터가 트리거 신호를 보내면 슬레이브가 응답**

```ini
StrobeInfo =
        0x000F,         $ OK to Combine w/Poll/Strobe/COS/Cyclic
        1,              $ Default Input  = Input1
        0;              $ Default Output = none
```

| 필드 | 값 | 설명 |
|------|-----|------|
| **Combination Flag** | 0x000F | 모든 모드와 조합 가능 |
| **Default Input** | 1 | Input1 구조 사용 (10바이트) |
| **Default Output** | 0 | 출력 없음 (입력만 스트로브) |

**동작 방식**:
1. 마스터 → `Strobe Signal` → 슬레이브
2. 슬레이브 → `Input Data (10 bytes)` → 마스터
3. 이벤트 기반으로 동작

---

#### 3.4 COS (Change of State) 모드

**데이터가 변경될 때만 슬레이브가 자동으로 전송**

```ini
COSInfo =
        0x0007,         $ OK to Combine w/Poll/Strobe/COS
        1,              $ Default Input  = Input1
        1;              $ Default Output = Output1
```

| 필드 | 값 | 설명 |
|------|-----|------|
| **Combination Flag** | 0x0007 | Poll/Strobe/COS와 조합 가능 (Cyclic 제외) |
| **Default Input** | 1 | Input1 구조 사용 (10바이트) |
| **Default Output** | 1 | Output1 구조 사용 (6바이트) |

**동작 방식**:
1. 슬레이브 내부에서 입력 데이터 변경 감지
2. 변경 시 → `Unsolicited Message` → 마스터
3. 네트워크 트래픽 최소화 (변경 시에만 전송)

---

#### 3.5 Cyclic 모드

**고정된 주기로 자동 데이터 교환**

```ini
CyclicInfo =
        0x000B,         $ OK to Combine w/Poll/Strobe/Cyclic
        1,              $ Default Input  = Input1
        1;              $ Default Output = Output1
```

| 필드 | 값 | 설명 |
|------|-----|------|
| **Combination Flag** | 0x000B | Poll/Strobe/Cyclic과 조합 가능 (COS 제외) |
| **Default Input** | 1 | Input1 구조 사용 (10바이트) |
| **Default Output** | 1 | Output1 구조 사용 (6바이트) |

**동작 방식**:
1. 설정된 주기마다 자동으로 데이터 교환
2. 마스터 개입 없이 슬레이브가 자율적으로 전송
3. 실시간성이 중요한 애플리케이션에 적합

---

## 4. 입력 데이터 구조 분석 (Input1)

### 📥 슬레이브 → 마스터 전송 데이터

```ini
Input1 =
        10,                                      $ 10 bytes
        80,                                      $ 80 bits
        0x000F,                                  $ 모든 모드 지원
        "Counter + Sensor + Actuator data",     $ 설명
        6,                                       $ 6개 데이터 항목
        "20 04 24 65 30 03",                    $ Path (Assembly Instance)
        "Counter, S1(Value), S1(Status), S2(Value), S2(Status), A1(Status) A1(Status)";
```

### 📊 Input1 필드 상세

| 필드 | 값 | 설명 |
|------|-----|------|
| **크기 (바이트)** | 10 | 총 10바이트 데이터 |
| **크기 (비트)** | 80 | 10 × 8 = 80 비트 |
| **지원 모드** | 0x000F | Poll/Strobe/COS/Cyclic 모두 지원 |
| **설명** | Counter + Sensor + Actuator data | 카운터, 센서, 액추에이터 데이터 |
| **항목 수** | 6 | 6개의 데이터 필드 |
| **Assembly Path** | 20 04 24 65 30 03 | CIP Object Path |

### 🗂️ Input1 데이터 맵 (10바이트)

| 바이트 인덱스 | 이름 | 데이터 타입 | 설명 |
|--------------|------|------------|------|
| **0** | Counter | uint8_t | 카운터 값 |
| **1** | S1(Value) | uint8_t | 센서 1 측정값 |
| **2** | S1(Status) | uint8_t | 센서 1 상태 플래그 |
| **3** | S2(Value) | uint8_t | 센서 2 측정값 |
| **4** | S2(Status) | uint8_t | 센서 2 상태 플래그 |
| **5** | A1(Status) | uint8_t | 액추에이터 1 상태 (첫 바이트) |
| **6** | A1(Status) | uint8_t | 액추에이터 1 상태 (두 번째 바이트) |
| **7** | - | uint8_t | 예약/미사용 |
| **8** | - | uint8_t | 예약/미사용 |
| **9** | - | uint8_t | 예약/미사용 |

### 📌 현재 코드와의 연관성

**`App_DemoApplication.h:65-68`**:
```c
typedef __HIL_PACKED_PRE struct APP_PROCESS_DATA_INPUT_Ttag
{
  uint8_t input[6];  // ⚠️ EDS는 10바이트인데 코드는 6바이트만 정의
} __HIL_PACKED_POST APP_PROCESS_DATA_INPUT_T;
```

**⚠️ 불일치 발견**:
- **EDS 정의**: 10바이트 (Input1 = 10)
- **코드 정의**: 6바이트 (input[6])
- **영향**: 나머지 4바이트(7~9번)는 현재 코드에서 접근 불가

---

## 5. 출력 데이터 구조 분석 (Output1)

### 📤 마스터 → 슬레이브 전송 데이터

```ini
Output1 =
        6,                                       $ 6 bytes
        48,                                      $ 48 bits
        0x000D,                                  $ Poll/COS/Cyclic 지원
        "Actuator data + Counter control",      $ 설명
        6,                                       $ 6개 데이터 항목
        "20 04 24 64 30 03",                    $ Path (Assembly Instance)
        "A1(Value), A1(Value), Counter Direction, Counter Speed";
```

### 📊 Output1 필드 상세

| 필드 | 값 | 설명 |
|------|-----|------|
| **크기 (바이트)** | 6 | 총 6바이트 데이터 |
| **크기 (비트)** | 48 | 6 × 8 = 48 비트 |
| **지원 모드** | 0x000D | Poll/COS/Cyclic 지원 (Strobe 제외) |
| **설명** | Actuator data + Counter control | 액추에이터 데이터 + 카운터 제어 |
| **항목 수** | 6 | 6개의 데이터 필드 |
| **Assembly Path** | 20 04 24 64 30 03 | CIP Object Path |

### 🗂️ Output1 데이터 맵 (6바이트)

| 바이트 인덱스 | 이름 | 데이터 타입 | 설명 |
|--------------|------|------------|------|
| **0** | A1(Value) | uint8_t | 액추에이터 1 제어값 (첫 바이트) |
| **1** | A1(Value) | uint8_t | 액추에이터 1 제어값 (두 번째 바이트) |
| **2** | Counter Direction | uint8_t | 카운터 방향 (0=Down, 1=Up) |
| **3** | Counter Speed | uint8_t | 카운터 속도 |
| **4** | - | uint8_t | 예약/미사용 |
| **5** | - | uint8_t | 예약/미사용 |

### 📌 현재 코드와의 연관성

**`App_DemoApplication.h:70-73`**:
```c
typedef __HIL_PACKED_PRE struct APP_PROCESS_DATA_OUTPUT_Ttag
{
  uint8_t output[10];  // ✅ EDS는 6바이트인데 코드는 10바이트 정의
} __HIL_PACKED_POST APP_PROCESS_DATA_OUTPUT_T;
```

**✅ 크기 비교**:
- **EDS 정의**: 6바이트 (Output1 = 6)
- **코드 정의**: 10바이트 (output[10])
- **영향**: 코드가 더 큰 버퍼를 제공하여 안전성 확보

---

## 6. Assembly Instance Path 분석

### 🛤️ CIP (Common Industrial Protocol) Object Path

DeviceNet은 CIP 프로토콜을 사용하며, Assembly Object를 통해 데이터를 그룹화합니다.

#### Input1 Path: `20 04 24 65 30 03`

| 세그먼트 | 값 | 의미 |
|---------|-----|------|
| **20 04** | Class ID Segment | Class 0x04 = Assembly Object |
| **24 65** | Instance ID Segment | Instance 0x65 (101) = Input Assembly |
| **30 03** | Attribute ID Segment | Attribute 0x03 = Data Attribute |

**해석**: Assembly Object의 101번 인스턴스, 데이터 속성 접근

#### Output1 Path: `20 04 24 64 30 03`

| 세그먼트 | 값 | 의미 |
|---------|-----|------|
| **20 04** | Class ID Segment | Class 0x04 = Assembly Object |
| **24 64** | Instance ID Segment | Instance 0x64 (100) = Output Assembly |
| **30 03** | Attribute ID Segment | Attribute 0x03 = Data Attribute |

**해석**: Assembly Object의 100번 인스턴스, 데이터 속성 접근

---

## 7. 통신 모드별 동작 비교

### 📊 모드 특성 비교표

| 특성 | Poll | Strobe | COS | Cyclic |
|------|------|--------|-----|--------|
| **트리거** | 마스터 요청 | 마스터 트리거 | 데이터 변경 | 고정 주기 |
| **실시간성** | 중간 | 높음 | 낮음 | 매우 높음 |
| **네트워크 부하** | 중간 | 낮음 | 매우 낮음 | 높음 |
| **입력 데이터** | 10바이트 | 10바이트 | 10바이트 | 10바이트 |
| **출력 데이터** | 6바이트 | 없음 | 6바이트 | 6바이트 |
| **조합 가능 모드** | 모두 | 모두 | Poll/Strobe/COS | Poll/Strobe/Cyclic |
| **응답 시간** | 10-100ms | <10ms | 변동적 | 고정 (1-50ms) |
| **적합한 용도** | 일반 제어 | 이벤트 감지 | 상태 모니터링 | 고속 제어 |

### 🎯 권장 사용 시나리오

#### Poll 모드
- **적합**: 일반적인 PLC 제어, 주기적 상태 확인
- **예시**: 온도 센서 모니터링, 밸브 제어

#### Strobe 모드
- **적합**: 트리거 기반 데이터 수집
- **예시**: 바코드 스캐너, 광센서 트리거

#### COS 모드
- **적합**: 상태 변화 감지, 알람 시스템
- **예시**: 비상 정지 버튼, 도어 센서

#### Cyclic 모드
- **적합**: 고속 실시간 제어, 모션 제어
- **예시**: 서보 모터 제어, 로봇 제어

---

## 8. 현재 코드와 EDS 파일 매핑

### 🔗 데이터 구조 비교

#### 입력 데이터 (슬레이브 → 마스터)

| 측면 | EDS 정의 | 코드 정의 | 상태 |
|------|---------|----------|------|
| **크기** | 10바이트 | 6바이트 | ⚠️ 불일치 |
| **변수명** | Input1 | tInputData.input[6] | - |
| **읽기 함수** | - | `xChannelIORead()` | ✅ |
| **업데이트 주기** | 모드 의존 | 500ms | ✅ |

**⚠️ 권장사항**: 코드를 `input[10]`으로 확장하여 EDS와 일치시킬 것

#### 출력 데이터 (마스터 → 슬레이브)

| 측면 | EDS 정의 | 코드 정의 | 상태 |
|------|---------|----------|------|
| **크기** | 6바이트 | 10바이트 | ✅ 안전 |
| **변수명** | Output1 | tOutputData.output[10] | - |
| **쓰기 함수** | - | `xChannelIOWrite()` | ✅ |
| **업데이트 주기** | 모드 의존 | 500ms | ✅ |

**✅ 상태**: 코드가 더 큰 버퍼를 제공하여 확장성 확보

---

### 🔧 코드 수정 권장사항

#### 1. 입력 데이터 구조 확장

**현재 (App_DemoApplication.h:65-68)**:
```c
typedef __HIL_PACKED_PRE struct APP_PROCESS_DATA_INPUT_Ttag
{
  uint8_t input[6];  // ⚠️ 6바이트만 정의
} __HIL_PACKED_POST APP_PROCESS_DATA_INPUT_T;
```

**권장 수정**:
```c
typedef __HIL_PACKED_PRE struct APP_PROCESS_DATA_INPUT_Ttag
{
  uint8_t counter;          // [0] 카운터 값
  uint8_t sensor1_value;    // [1] 센서 1 측정값
  uint8_t sensor1_status;   // [2] 센서 1 상태
  uint8_t sensor2_value;    // [3] 센서 2 측정값
  uint8_t sensor2_status;   // [4] 센서 2 상태
  uint8_t actuator1_status; // [5] 액추에이터 1 상태 (바이트 1)
  uint8_t actuator1_status2;// [6] 액추에이터 1 상태 (바이트 2)
  uint8_t reserved1;        // [7] 예약
  uint8_t reserved2;        // [8] 예약
  uint8_t reserved3;        // [9] 예약
} __HIL_PACKED_POST APP_PROCESS_DATA_INPUT_T;
```

#### 2. 출력 데이터 구조 명확화

**현재 (App_DemoApplication.h:70-73)**:
```c
typedef __HIL_PACKED_PRE struct APP_PROCESS_DATA_OUTPUT_Ttag
{
  uint8_t output[10];  // 10바이트 정의 (EDS는 6바이트만 사용)
} __HIL_PACKED_POST APP_PROCESS_DATA_OUTPUT_T;
```

**권장 수정**:
```c
typedef __HIL_PACKED_PRE struct APP_PROCESS_DATA_OUTPUT_Ttag
{
  uint8_t actuator1_value_low;   // [0] 액추에이터 1 제어값 (하위)
  uint8_t actuator1_value_high;  // [1] 액추에이터 1 제어값 (상위)
  uint8_t counter_direction;     // [2] 카운터 방향 (0=Down, 1=Up)
  uint8_t counter_speed;         // [3] 카운터 속도
  uint8_t reserved1;             // [4] 예약 (확장용)
  uint8_t reserved2;             // [5] 예약 (확장용)
  uint8_t reserved3;             // [6] 예약
  uint8_t reserved4;             // [7] 예약
  uint8_t reserved5;             // [8] 예약
  uint8_t reserved6;             // [9] 예약
} __HIL_PACKED_POST APP_PROCESS_DATA_OUTPUT_T;
```

---

## 9. 실제 사용 예시

### 📝 시나리오: 온도 센서와 냉각 팬 제어

#### 입력 데이터 (슬레이브 → PLC)
```c
// 슬레이브에서 PLC로 전송
tInputData.counter = cycle_count;        // 주기 카운터
tInputData.sensor1_value = temperature;  // 온도 센서 (0-100°C)
tInputData.sensor1_status = temp_valid ? 0x01 : 0x00;
tInputData.sensor2_value = humidity;     // 습도 센서 (0-100%)
tInputData.sensor2_status = humid_valid ? 0x01 : 0x00;
tInputData.actuator1_status = fan_running ? 0x01 : 0x00;
tInputData.actuator1_status2 = fan_speed; // 실제 팬 속도
```

#### 출력 데이터 (PLC → 슬레이브)
```c
// PLC에서 슬레이브로 제어 명령
tOutputData.actuator1_value_low = fan_pwm_low;   // PWM 듀티 하위
tOutputData.actuator1_value_high = fan_pwm_high; // PWM 듀티 상위
tOutputData.counter_direction = 1;               // 카운터 증가
tOutputData.counter_speed = 10;                  // 10ms 간격
```

#### 제어 로직 예시
```c
void App_IODataHandler(void* ptAppResources)
{
  APP_DATA_T* ptAppData = (APP_DATA_T*)ptAppResources;
  long lRet;

  if(ptAppData->aptChannels[0]->hChannel != NULL)
  {
    /** 입력 데이터 읽기 (센서 → PLC) **************/
    lRet = xChannelIORead(ptAppData->aptChannels[0]->hChannel, 0, 0,
                          sizeof(ptAppData->tInputData),
                          &ptAppData->tInputData, 0);

    if(lRet == CIFX_NO_ERROR)
    {
      // 센서 데이터 처리
      uint8_t temperature = ptAppData->tInputData.input[1];
      uint8_t temp_status = ptAppData->tInputData.input[2];

      if(temp_status == 0x01)  // 센서 정상
      {
        // 온도에 따른 팬 제어
        if(temperature > 60)
        {
          ptAppData->tOutputData.output[0] = 255; // 최대 속도
          ptAppData->tOutputData.output[1] = 0;
        }
        else if(temperature > 40)
        {
          ptAppData->tOutputData.output[0] = 128; // 중간 속도
          ptAppData->tOutputData.output[1] = 0;
        }
        else
        {
          ptAppData->tOutputData.output[0] = 0;   // 정지
          ptAppData->tOutputData.output[1] = 0;
        }
      }
    }

    /** 출력 데이터 쓰기 (PLC → 액추에이터) ********/
    lRet = xChannelIOWrite(ptAppData->aptChannels[0]->hChannel, 0, 0,
                           sizeof(ptAppData->tOutputData),
                           &ptAppData->tOutputData, 0);
  }
}
```

---

## 10. DeviceNet 통신 파라미터

### 📡 네트워크 설정

| 파라미터 | 일반 값 | 설명 |
|---------|---------|------|
| **Baud Rate** | 125/250/500 kbps | 네트워크 속도 |
| **Node Address** | 0-63 | 슬레이브 주소 (MAC ID) |
| **EPR (Expected Packet Rate)** | 10-5000ms | 예상 패킷 주기 |
| **Message Timeout** | EPR × 4 | 통신 타임아웃 |
| **Idle Timeout** | 30s | 유휴 연결 종료 |

### 🔧 통신 파라미터 설정 예시

```c
// DeviceNet 설정 (예시)
#define DNS_NODE_ADDRESS        10      // 슬레이브 주소
#define DNS_BAUD_RATE           250000  // 250 kbps
#define DNS_EPR_MS              100     // 100ms 주기
#define DNS_CONNECTION_TIMEOUT  400     // 400ms 타임아웃
```

---

## 11. EDS 파일 사용 방법

### 🛠️ DeviceNet 설정 도구

#### RSNetWorx for DeviceNet (Rockwell Automation)
1. **EDS 파일 설치**: RSNetWorx → `Tools` → `EDS Wizard` → `.EDS` 파일 선택
2. **장치 추가**: Network Browser에서 DNS_V5_SIMPLE_CONFIG_DEMO 선택
3. **I/O 설정**: Connection Configuration에서 Poll/Strobe/COS/Cyclic 선택
4. **다운로드**: Upload/Download → Device로 설정 전송

#### Hilscher netANALYZER
1. **네트워크 스캔**: 자동으로 DeviceNet 노드 검색
2. **EDS 매칭**: VendCode=283, ProdCode=34로 장치 식별
3. **I/O 모니터링**: 실시간 입출력 데이터 확인

---

## 12. 트러블슈팅 가이드

### ⚠️ 일반적인 문제와 해결방법

#### 문제 1: 입출력 데이터 크기 불일치

**증상**:
- `xChannelIORead()` 또는 `xChannelIOWrite()` 실패
- 에러 코드: `CIFX_INVALID_BUFFERSIZE`

**원인**:
- 코드의 구조체 크기와 EDS 정의가 불일치

**해결**:
```c
// 입력 데이터: 10바이트로 수정
typedef struct {
  uint8_t input[10];  // EDS Input1 크기와 일치
} APP_PROCESS_DATA_INPUT_T;

// 출력 데이터: 6바이트 사용 (10바이트 버퍼는 유지)
// EDS Output1에 정의된 6바이트만 유효
```

#### 문제 2: 통신 모드 미지원

**증상**:
- 특정 통신 모드에서 연결 실패

**원인**:
- PLC가 요청한 모드를 EDS에서 지원하지 않음

**해결**:
- EDS 파일에서 지원 모드 확인
  - Poll: 0x000F (모두 지원)
  - Strobe: 0x000F (모두 지원)
  - COS: 0x0007 (Cyclic 제외)
  - Cyclic: 0x000B (COS 제외)

#### 문제 3: Assembly Instance 접근 오류

**증상**:
- `Connection Manager` 에러
- 에러 코드: "Invalid Assembly Instance"

**원인**:
- 잘못된 Assembly Instance 번호

**해결**:
- EDS에 정의된 경로 사용
  - Input: Instance 101 (0x65)
  - Output: Instance 100 (0x64)

---

## 13. 요약 및 권장사항

### 📋 EDS 파일 핵심 정보

| 항목 | 값 |
|------|-----|
| **프로토콜** | DeviceNet Slave V5 |
| **제조사** | Hilscher (VendCode=283) |
| **제품** | DNS_V5_SIMPLE_CONFIG_DEMO (ProdCode=34) |
| **입력 크기** | 10바이트 (Counter + Sensors + Actuator) |
| **출력 크기** | 6바이트 (Actuator + Counter Control) |
| **지원 모드** | Poll, Strobe, COS, Cyclic |
| **기본 모드** | Poll |

### ✅ 현재 코드 개선 권장사항

#### 1. 입력 데이터 구조 확장
```c
// App_DemoApplication.h 수정
typedef struct {
  uint8_t input[10];  // 6 → 10바이트로 확장
} APP_PROCESS_DATA_INPUT_T;
```

#### 2. 명확한 필드 정의
```c
// 구조화된 접근을 위한 union 사용
typedef union {
  struct {
    uint8_t counter;
    uint8_t sensor1_value;
    uint8_t sensor1_status;
    uint8_t sensor2_value;
    uint8_t sensor2_status;
    uint8_t actuator1_status[2];
    uint8_t reserved[3];
  } fields;
  uint8_t raw[10];
} APP_PROCESS_DATA_INPUT_T;
```

#### 3. EDS 파일 활용
- RSNetWorx 또는 Hilscher 도구에 EDS 파일 등록
- 자동 I/O 설정 및 검증

### 🎯 다음 단계

1. **코드 수정**: 입력 데이터 구조를 10바이트로 확장
2. **테스트**: 확장된 데이터로 통신 검증
3. **문서화**: 각 바이트의 의미를 코드 주석에 추가
4. **최적화**: 실제 사용하는 필드만 처리하도록 로직 개선

---

## 14. 참고 자료

### 📚 관련 문서
- **DeviceNet Specification**: ODVA DeviceNet Volume I & II
- **CIP Specification**: Common Industrial Protocol (ODVA)
- **Hilscher DeviceNet Manual**: netX DeviceNet Slave Protocol API
- **EDS File Format**: ODVA EDS File Specification

### 🔗 유용한 링크
- ODVA 공식 사이트: https://www.odva.org
- Hilscher 기술 지원: https://www.hilscher.com/support
- DeviceNet EDS 검증 도구: EZ-EDS (ODVA 제공)

---

**문서 끝**

# DeviceNet 통합 가이드

이 문서는 Dasan APC RTOS 프로젝트에 DeviceNet을 통합하는 구조를 설명합니다.

## 전체 디렉토리 구조

```
Dasan_APC_rtos_20250110/
├── App/
│   ├── backup/                      # 기존 백업 파일
│   ├── common/                      # 기존 공통 파일
│   ├── logic/                       # 🔄 기존 애플리케이션 로직 (보존됨)
│   │   └── cmd/
│   ├── model/                       # 🔄 기존 애플리케이션 모델 (보존됨)
│   │   └── driver/
│   │
│   ├── DeviceNet/                   # 🆕 DeviceNet 통합 모듈
│   │   ├── includes/                # 헤더 파일
│   │   │   ├── DNS_API/             # DNS 패킷 API (SDK에서 복사)
│   │   │   │   └── README.md
│   │   │   ├── AppDNS_DemoApplication.h
│   │   │   └── AppDNS_DeviceNetTask.h   # FreeRTOS 태스크 헤더
│   │   ├── Sources/                 # 소스 파일
│   │   │   ├── AppDNS_DemoApplication.c
│   │   │   ├── AppDNS_DemoApplicationFunctions.c
│   │   │   ├── AppDNS_ExplicitMsg.c
│   │   │   └── AppDNS_DeviceNetTask.c   # FreeRTOS 태스크 구현
│   │   └── README_KR.md             # DeviceNet 통합 가이드
│   │
│   └── cifXToolkit/                 # 🆕 cifXToolkit 라이브러리
│       ├── Source/                  # cifX 핵심 소스 (복사 필요)
│       │   └── README.md
│       ├── SerialDPM/               # SPI DPM 인터페이스 (복사 필요)
│       │   └── README.md
│       ├── OSAbstraction/           # OS 추상화 레이어 (FreeRTOS 적용)
│       │   ├── README_KR.md
│       │   ├── OS_Custom.c          # ✅ FreeRTOS 구현 (완료)
│       │   └── OS_SPICustom.c       # ✅ SPI5 HAL 구현 (완료)
│       └── README_KR.md             # cifXToolkit 통합 가이드
│
├── Core/                            # STM32 핵심 파일
├── Drivers/                         # STM32 HAL 드라이버
└── Middlewares/                     # FreeRTOS 및 기타 미들웨어
```

## 기존 구조 vs 새로 추가된 구조

### ✅ 보존됨 (기존 구조)
- `App/backup/` - 기존 백업 파일
- `App/common/` - 기존 공통 파일
- `App/logic/` - **기존 애플리케이션 로직 (보존됨)**
- `App/model/` - **기존 애플리케이션 모델 (보존됨)**
- 기타 모든 기존 디렉토리

### 🆕 새로 추가됨
- `App/DeviceNet/` - DeviceNet 프로토콜 통합
- `App/cifXToolkit/` - cifX 통신 툴킷

새로운 DeviceNet 및 cifXToolkit 디렉토리는 App 디렉토리 아래의 기존 logic 및 model 폴더와 **공존**합니다.

## 통합 개요

### 계층 아키텍처

```
┌─────────────────────────────────────────────────┐
│  애플리케이션 계층 (App/logic, App/model)       │
│  - 기존 비즈니스 로직                           │
│  - 센서 관리                                    │
│  - 제어 알고리즘                                │
└────────────────┬────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────┐
│  DeviceNet 계층 (App/DeviceNet)                 │
│  - DeviceNet 프로토콜 스택                      │
│  - I/O 어셈블리 처리                            │
│  - Explicit 메시징                              │
│  - FreeRTOS 태스크 래퍼                         │
└────────────────┬────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────┐
│  cifXToolkit 계층 (App/cifXToolkit)             │
│  - DPM 액세스 API                               │
│  - 패킷 처리                                    │
│  - FreeRTOS 추상화                              │
└────────────────┬────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────┐
│  SerialDPM 계층 (App/cifXToolkit/SerialDPM)    │
│  - SPI DPM 프로토콜                             │
│  - 핸드셰이킹 (SRDY/CS)                         │
│  - netX 90 전용 타이밍                          │
└────────────────┬────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────┐
│  STM32 HAL 계층 (OS_SPICustom.c)               │
│  - SPI5 인터페이스                              │
│  - GPIO 제어                                    │
│  - 하드웨어 추상화                              │
└────────────────┬────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────┐
│  하드웨어 (STM32F429 ←SPI5→ netX 90)          │
└─────────────────────────────────────────────────┘
```

## 상세 FW 작동 순서도

### 1. 시스템 초기화 순서

```
시작
  │
  ├─→ [1] STM32 HAL 초기화
  │     ├─ SystemClock_Config()
  │     ├─ MX_GPIO_Init()
  │     ├─ MX_SPI5_Init()          ◀── SPI5 초기화
  │     └─ MX_FREERTOS_Init()
  │
  ├─→ [2] FreeRTOS 커널 시작
  │     └─ osKernelStart()
  │
  ├─→ [3] DeviceNet 태스크 생성
  │     ├─ AppDNS_DeviceNetTask_Create()
  │     │   ├─ xTaskCreate()
  │     │   │   ├─ 태스크 이름: "DeviceNetTask"
  │     │   │   ├─ 스택 크기: 4KB
  │     │   │   ├─ 우선순위: tskIDLE_PRIORITY + 3
  │     │   │   └─ 함수: AppDNS_DeviceNetTask()
  │     │   └─ 태스크 핸들 저장
  │     └─ 성공/실패 반환
  │
  ├─→ [4] 기타 애플리케이션 태스크 생성
  │     ├─ 센서 태스크
  │     ├─ 제어 태스크
  │     └─ UI 태스크
  │
  └─→ [5] 스케줄러 실행
        └─ 각 태스크 병렬 실행 시작
```

### 2. DeviceNet 태스크 초기화 흐름

```
AppDNS_DeviceNetTask() 진입
  │
  ├─→ [1] SPI 하드웨어 초기화
  │     └─ SPI_Init()                           [OS_SPICustom.c]
  │         ├─ GPIO 초기화
  │         │   ├─ CS 핀: 출력, 초기값 HIGH
  │         │   └─ SRDY 핀: 입력, 풀업
  │         └─ SPI5 설정 확인
  │             ├─ 모드: Master
  │             ├─ 속도: ~20 MHz
  │             └─ 데이터 크기: 8 bit
  │
  ├─→ [2] cifX 드라이버 초기화
  │     └─ xDriverOpen(&hDriver)                [cifXToolkit]
  │         ├─ OS 추상화 레이어 초기화
  │         │   ├─ Mutex 생성 (OS_CreateMutex)
  │         │   └─ 메모리 할당 (OS_Memalloc)
  │         ├─ SerialDPM 인터페이스 초기화
  │         │   ├─ netX 90 SPI DPM 설정
  │         │   └─ 핸드셰이크 프로토콜 준비
  │         └─ 드라이버 핸들 반환
  │
  ├─→ [3] cifX 디바이스 열기
  │     └─ xChannelOpen(hDriver, "cifX0", 0, &hDevice)
  │         ├─ DPM 구조 검증
  │         │   ├─ DPM 헤더 읽기 (SPI)
  │         │   ├─ 매직 넘버 확인
  │         │   └─ DPM 버전 확인
  │         ├─ 통신 플래그 설정
  │         │   ├─ COM_FLAG 초기화
  │         │   └─ 핸드셰이크 비트 설정
  │         └─ 디바이스 핸들 반환
  │
  ├─→ [4] DeviceNet 스택 초기화
  │     └─ AppDNS_DemoApplication_Init()        [DeviceNet SDK]
  │         ├─ DeviceNet 설정 패킷 전송
  │         │   ├─ MAC ID 설정 (0-63)
  │         │   ├─ Baud Rate 설정 (125/250/500 kbps)
  │         │   └─ I/O Assembly 설정
  │         ├─ Explicit Messaging 설정
  │         │   ├─ Object Dictionary 등록
  │         │   └─ 서비스 핸들러 등록
  │         ├─ 네트워크 시작 명령 전송
  │         └─ 초기화 완료 대기
  │
  ├─→ [5] 초기화 완료 확인
  │     ├─ 성공 → 메인 루프 진입
  │     └─ 실패 → vTaskSuspend(NULL)
  │
  └─→ [6] 메인 루프 시작 (무한 루프)
```

### 3. DeviceNet 메인 루프 (주기적 실행)

```
무한 루프 {
  │
  ├─→ [1] DeviceNet 애플리케이션 실행
  │     └─ AppDNS_DemoApplication_Run()         [DeviceNet SDK]
  │         │
  │         ├─→ [A] I/O 데이터 처리
  │         │     ├─ 입력 데이터 읽기
  │         │     │   └─ xChannelIORead()
  │         │     │       ├─ DPM 입력 영역 액세스
  │         │     │       ├─ SPI 읽기 수행
  │         │     │       └─ 데이터 반환
  │         │     │
  │         │     ├─ 애플리케이션 로직 수행
  │         │     │   ├─ 센서 데이터 → DeviceNet 입력
  │         │     │   └─ DeviceNet 출력 → 액추에이터
  │         │     │
  │         │     └─ 출력 데이터 쓰기
  │         │         └─ xChannelIOWrite()
  │         │             ├─ DPM 출력 영역 액세스
  │         │             ├─ SPI 쓰기 수행
  │         │             └─ 완료
  │         │
  │         ├─→ [B] Explicit 메시지 처리
  │         │     ├─ 수신 메시지 확인
  │         │     │   └─ xChannelGetPacket()
  │         │     │       ├─ DPM 메일박스 확인
  │         │     │       ├─ 패킷 읽기 (SPI)
  │         │     │       └─ 패킷 반환
  │         │     │
  │         │     ├─ 메시지 처리
  │         │     │   ├─ 서비스 코드 파싱
  │         │     │   ├─ Object Dictionary 액세스
  │         │     │   └─ 응답 생성
  │         │     │
  │         │     └─ 응답 전송
  │         │         └─ xChannelPutPacket()
  │         │             ├─ 응답 패킷 준비
  │         │             ├─ DPM 메일박스 쓰기
  │         │             └─ SPI 전송
  │         │
  │         └─→ [C] 네트워크 상태 모니터링
  │               ├─ 연결 상태 확인
  │               ├─ 에러 플래그 확인
  │               └─ 타임아웃 처리
  │
  └─→ [2] 태스크 지연
        └─ vTaskDelay(pdMS_TO_TICKS(10))        ◀── 10ms 주기
}
```

### 4. SPI DPM 액세스 상세 흐름

```
DPM 읽기/쓰기 요청
  │
  ├─→ [1] Mutex 획득
  │     └─ OS_WaitMutex(hMutex, timeout)
  │         └─ xSemaphoreTake()
  │
  ├─→ [2] SRDY 대기
  │     └─ SPI_WaitSRDY(timeout)                [OS_SPICustom.c]
  │         │
  │         └─ while (timeout) {
  │               if (HAL_GPIO_ReadPin(SRDY_PORT, SRDY_PIN) == HIGH)
  │                   break;  // SRDY 활성
  │               delay(1ms);
  │             }
  │
  ├─→ [3] CS 활성화
  │     └─ SPI_SetCS(1)
  │         └─ HAL_GPIO_WritePin(CS_PORT, CS_PIN, LOW)
  │
  ├─→ [4] SPI 전송 수행
  │     └─ SPI_Transfer(pbSend, pbRecv, ulLen)
  │         └─ HAL_SPI_TransmitReceive(&hspi5, pbSend, pbRecv, ulLen, timeout)
  │             │
  │             ├─→ [A] 명령 바이트 전송
  │             │     ├─ 읽기: 0x00 + 주소
  │             │     └─ 쓰기: 0x80 + 주소
  │             │
  │             ├─→ [B] 주소 전송 (2 바이트)
  │             │     ├─ High byte
  │             │     └─ Low byte
  │             │
  │             └─→ [C] 데이터 전송/수신
  │                   └─ N 바이트 (요청 크기만큼)
  │
  ├─→ [5] CS 비활성화
  │     └─ SPI_SetCS(0)
  │         └─ HAL_GPIO_WritePin(CS_PORT, CS_PIN, HIGH)
  │
  ├─→ [6] SRDY 해제 대기
  │     └─ while (HAL_GPIO_ReadPin(SRDY_PORT, SRDY_PIN) == HIGH)
  │           delay(10us);
  │
  ├─→ [7] Mutex 해제
  │     └─ OS_ReleaseMutex(hMutex)
  │         └─ xSemaphoreGive()
  │
  └─→ [8] 결과 반환
        ├─ 성공: 0
        └─ 실패: 에러 코드
```

### 5. 에러 처리 흐름

```
에러 발생
  │
  ├─→ [1] 에러 타입 분류
  │     │
  │     ├─→ [A] SPI 통신 에러
  │     │     ├─ HAL_TIMEOUT
  │     │     │   └─ SPI 버스 리셋 시도
  │     │     ├─ HAL_ERROR
  │     │     │   └─ SPI 재초기화
  │     │     └─ SRDY 타임아웃
  │     │         └─ netX 리셋 시도
  │     │
  │     ├─→ [B] cifX 드라이버 에러
  │     │     ├─ CIFX_DEV_NOT_READY
  │     │     │   └─ 디바이스 재초기화
  │     │     ├─ CIFX_DEV_NO_COM_FLAG
  │     │     │   └─ DPM 구조 재검증
  │     │     └─ CIFX_DEV_EXCHANGE_FAILED
  │     │         └─ 통신 재시도
  │     │
  │     └─→ [C] DeviceNet 프로토콜 에러
  │           ├─ 네트워크 타임아웃
  │           │   └─ 네트워크 재시작
  │           ├─ Duplicate MAC ID
  │           │   └─ MAC ID 재설정
  │           └─ Bus Off
  │               └─ 복구 절차 수행
  │
  ├─→ [2] 에러 로깅
  │     ├─ 에러 카운터 증가
  │     ├─ 에러 히스토리 저장
  │     └─ 디버그 출력 (UART/ITM)
  │
  ├─→ [3] 복구 시도
  │     ├─ 재시도 카운터 확인
  │     │   ├─ 재시도 가능 → 복구 수행
  │     │   └─ 최대 재시도 초과 → [4]로 이동
  │     └─ 복구 절차 수행
  │         ├─ 소프트 리셋
  │         ├─ 하드 리셋 (netX)
  │         └─ 전체 재초기화
  │
  ├─→ [4] 실패 처리
  │     ├─ 에러 플래그 설정
  │     ├─ 안전 모드 진입
  │     │   ├─ 출력 비활성화
  │     │   └─ 경고 표시
  │     └─ 사용자 알림
  │         ├─ LED 표시
  │         └─ 디스플레이 메시지
  │
  └─→ [5] 계속 또는 중단
        ├─ 치명적 에러 → 태스크 일시 중단
        └─ 비치명적 에러 → 정상 동작 계속
```

### 6. FreeRTOS 태스크 상호작용

```
┌──────────────────────┐
│  DeviceNet Task      │  Priority: 5
│  (10ms 주기)         │
└──────┬───────────────┘
       │
       ├─→ Mutex 공유 ◀────────────┐
       │                            │
       ├─→ I/O 데이터 Queue ◀──────┼────────────┐
       │                            │            │
       │                            │            │
┌──────▼───────────────┐  ┌────────┴─────────┐  │
│  센서 Task           │  │  제어 Task       │  │
│  (5ms 주기)          │  │  (20ms 주기)     │  │
│  Priority: 6         │  │  Priority: 4     │  │
└──────┬───────────────┘  └────────┬─────────┘  │
       │                            │            │
       ├─→ 센서 데이터 Queue ───────┤            │
       │                            │            │
       └─→ 제어 명령 Queue ◀────────┴────────────┘

데이터 흐름:
1. 센서 Task → 센서 데이터 Queue → DeviceNet Task
2. DeviceNet Task → I/O 데이터 Queue → 제어 Task
3. 제어 Task → 제어 명령 Queue → DeviceNet Task
4. DeviceNet Task → DeviceNet Network

동기화:
- Mutex: SPI 버스 액세스 보호
- Semaphore: DPM 액세스 동기화
- Queue: 태스크 간 데이터 전달
```

### 7. 타이밍 다이어그램

```
시간축 (ms) →
0    5    10   15   20   25   30   35   40
│    │    │    │    │    │    │    │    │
DeviceNet Task (10ms 주기)
├────┤    ├────┤    ├────┤    ├────┤
│I/O │    │I/O │    │I/O │    │I/O │
└────┘    └────┘    └────┘    └────┘

센서 Task (5ms 주기)
├──┤  ├──┤  ├──┤  ├──┤  ├──┤  ├──┤  ├──┤
│S │  │S │  │S │  │S │  │S │  │S │  │S │
└──┘  └──┘  └──┘  └──┘  └──┘  └──┘  └──┘

제어 Task (20ms 주기)
├────────┤         ├────────┤
│  제어   │         │  제어   │
└────────┘         └────────┘

SPI 버스 사용
├┤ ├┤ ├┤  ├┤ ├┤ ├┤  ├┤ ├┤ ├┤  ├┤ ├┤
DN Sn DN  Sn DN Sn  DN Sn DN  Sn DN

DN: DeviceNet 액세스
Sn: 센서 액세스

주의사항:
- SPI 버스는 한 번에 하나의 태스크만 사용
- Mutex로 배타적 액세스 보장
- 우선순위: 센서(6) > DeviceNet(5) > 제어(4)
```

## 통합 단계

### Phase 1: 디렉토리 구조 설정 ✅ 완료

다음이 포함된 디렉토리 구조가 생성되었습니다:
- DeviceNet 스켈레톤 파일 및 헤더
- cifXToolkit OS 추상화 구현
- 상세한 지침이 포함된 README 파일

### Phase 2: 외부 라이브러리 복사 (수행 필요)

1. **DeviceNet SDK 파일 복사**:
   ```bash
   # DNS API 헤더 복사
   cp -r <DeviceNet_SDK>/DNS_API/* \
         Dasan_APC_rtos_20250110/App/DeviceNet/includes/DNS_API/

   # 애플리케이션 소스 복사 (플레이스홀더 교체)
   cp <DeviceNet_SDK>/AppDNS_DemoApplication.c \
      Dasan_APC_rtos_20250110/App/DeviceNet/Sources/
   ```

2. **cifXToolkit 파일 복사**:
   ```bash
   # 핵심 툴킷 소스 복사
   cp -r <cifXToolkit>/Source/* \
         Dasan_APC_rtos_20250110/App/cifXToolkit/Source/

   # SerialDPM 복사 (netX 90 버전 사용)
   cp -r <cifXToolkit>/SerialDPM/NetX90_* \
         Dasan_APC_rtos_20250110/App/cifXToolkit/SerialDPM/
   ```

### Phase 3: STM32CubeMX 설정 (수행 필요)

1. **SPI5 설정**:
   - 모드: Full Duplex Master
   - 데이터 크기: 8 bits
   - 클럭: 최대 20 MHz
   - CPOL: Low, CPHA: 1 Edge

2. **GPIO 설정**:
   - CS (Chip Select): 출력
   - SRDY (Service Ready): 풀업이 있는 입력

3. **`App/cifXToolkit/OSAbstraction/OS_SPICustom.c`의 핀 정의 업데이트**

### Phase 4: 빌드 시스템 통합 (수행 필요)

1. **.cproject 업데이트** (Eclipse/STM32CubeIDE):
   - include 경로 추가:
     ```
     App/DeviceNet/includes
     App/DeviceNet/includes/DNS_API
     App/cifXToolkit/Source
     App/cifXToolkit/SerialDPM
     App/cifXToolkit/OSAbstraction
     ```

   - 소스 경로 추가:
     ```
     App/DeviceNet/Sources
     App/cifXToolkit/Source
     App/cifXToolkit/SerialDPM
     App/cifXToolkit/OSAbstraction
     ```

2. **Makefile 업데이트** (make 사용 시):
   - `C_INCLUDES`에 디렉토리 추가
   - `C_SOURCES`에 `.c` 파일 추가

### Phase 5: FreeRTOS 통합 (수행 필요)

`main.c` 또는 `freertos.c`에 추가:

```c
#include "AppDNS_DeviceNetTask.h"

void MX_FREERTOS_Init(void)
{
    /* 기존 태스크 생성... */

    /* DeviceNet 태스크 생성 */
    if (AppDNS_DeviceNetTask_Create() != pdPASS)
    {
        Error_Handler();
    }
}
```

### Phase 6: 테스트 및 검증 (수행 필요)

1. **단위 테스트**:
   - SPI 통신
   - GPIO 핸드셰이킹
   - FreeRTOS 태스크 생성

2. **통합 테스트**:
   - cifX 디바이스 초기화
   - DeviceNet 네트워크 시작
   - I/O 데이터 교환
   - Explicit 메시징

3. **시스템 테스트**:
   - 네트워크 통신
   - 실시간 성능
   - 에러 복구

## 하드웨어 요구사항

### STM32F429 설정
- SPI5 주변장치 활성화
- CS 및 SRDY용 GPIO 핀
- cifX 버퍼용 최소 64KB RAM
- 약 16KB 힙이 있는 FreeRTOS

### netX 90 설정
- DeviceNet 펌웨어 로드됨
- SPI DPM 인터페이스 활성화
- 네트워크 설정 (MAC ID, baud rate)
- I/O 어셈블리 구성됨

### 연결
| STM32F429 | netX 90 | 기능 |
|-----------|---------|----------|
| SPI5_SCK  | SPI_CLK | SPI 클럭 |
| SPI5_MOSI | SPI_MOSI| SPI 데이터 출력 |
| SPI5_MISO | SPI_MISO| SPI 데이터 입력 |
| GPIO (CS) | SPI_CS  | 칩 선택 |
| GPIO      | SRDY    | 서비스 준비 |
| GND       | GND     | 그라운드 |

## 소프트웨어 종속성

- **STM32 HAL**: SPI, GPIO, 시스템 함수
- **FreeRTOS**: 태스크 관리, 세마포어, 힙
- **cifXToolkit**: Hilscher 통신 라이브러리
- **DeviceNet SDK**: 프로토콜 스택 구현
- **표준 C 라이브러리**: string.h, stdint.h

## 파일 요약

### 생성된 파일 (즉시 사용 가능)
- ✅ `App/DeviceNet/includes/AppDNS_DemoApplication.h` - DeviceNet API 헤더
- ✅ `App/DeviceNet/includes/AppDNS_DeviceNetTask.h` - FreeRTOS 태스크 헤더
- ✅ `App/DeviceNet/Sources/AppDNS_DeviceNetTask.c` - FreeRTOS 태스크 구현
- ✅ `App/cifXToolkit/OSAbstraction/OS_Custom.c` - FreeRTOS OS 레이어 (완료)
- ✅ `App/cifXToolkit/OSAbstraction/OS_SPICustom.c` - SPI HAL 레이어 (완료)

### 플레이스홀더 파일 (SDK 파일로 교체 필요)
- ⏳ `App/DeviceNet/Sources/AppDNS_DemoApplication.c`
- ⏳ `App/DeviceNet/Sources/AppDNS_DemoApplicationFunctions.c`
- ⏳ `App/DeviceNet/Sources/AppDNS_ExplicitMsg.c`

### 외부 파일용 디렉토리
- 📁 `App/DeviceNet/includes/DNS_API/` - DNS API 헤더 복사 위치
- 📁 `App/cifXToolkit/Source/` - cifX 핵심 소스 복사 위치
- 📁 `App/cifXToolkit/SerialDPM/` - SPI DPM 구현 복사 위치

## 다음 단계

1. ✅ **디렉토리 구조 생성됨** - 완료
2. ⏳ **DeviceNet SDK 파일 복사** - `App/DeviceNet/includes/DNS_API/README.md` 참조
3. ⏳ **cifXToolkit 파일 복사** - `App/cifXToolkit/Source/README.md` 참조
4. ⏳ **STM32CubeMX 설정** - SPI5 및 GPIO 설정
5. ⏳ **빌드 시스템 업데이트** - include/source 경로 추가
6. ⏳ **SPI 통신 테스트** - 하드웨어 연결 확인
7. ⏳ **cifX 초기화** - 디바이스 초기화 테스트
8. ⏳ **DeviceNet 테스트** - 네트워크 통신 확인

## 참조

### 문서
- `App/DeviceNet/README_KR.md` - DeviceNet 통합 상세
- `App/cifXToolkit/README_KR.md` - cifXToolkit 통합 상세
- `App/cifXToolkit/OSAbstraction/README_KR.md` - OS 추상화 가이드라인

### 외부 리소스
- Hilscher cifXToolkit API 레퍼런스
- DeviceNet 사양 (ODVA)
- netX 90 사용자 매뉴얼
- STM32F429 레퍼런스 매뉴얼

## 지원 및 문제 해결

### 일반적인 문제

1. **SPI 통신 실패**
   - GPIO 핀 할당 확인
   - SPI 클럭 설정 확인
   - 오실로스코프로 SRDY 신호 모니터링

2. **DeviceNet 네트워크 타임아웃**
   - netX 90 펌웨어 로드 확인
   - 네트워크 설정 확인 (MAC ID, baud rate)
   - 적절한 네트워크 종단 확인

3. **FreeRTOS 태스크 문제**
   - 할당 실패 시 힙 크기 증가
   - 타이밍 문제 발생 시 태스크 우선순위 조정
   - 스택 크기 확인 (스택 오버플로 감지 사용)

### 디버그 팁

- cifX 디버그 출력 활성화
- SPI 신호에 로직 분석기 사용
- SRDY 신호 타이밍 모니터링
- DPM 구조 정렬 확인
- netX 펌웨어 버전 호환성 확인

## 결론

이 통합 구조는 **기존 애플리케이션 로직 및 모델 코드를 모두 보존하면서** 기존 Dasan APC RTOS 프로젝트에 DeviceNet 지원을 추가하는 깔끔하고 모듈식 접근 방식을 제공합니다.

이 구조의 주요 이점:
- **관심사의 분리**: DeviceNet/cifX 코드가 애플리케이션에서 격리됨
- **재사용성**: OS 추상화로 쉬운 이식 가능
- **유지보수성**: 명확한 디렉토리 구조 및 문서화
- **하위 호환성**: 기존 코드는 변경되지 않음

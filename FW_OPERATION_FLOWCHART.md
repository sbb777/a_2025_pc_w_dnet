# DeviceNet 펌웨어 작동 순서도

이 문서는 DeviceNet 통합 펌웨어의 상세한 작동 순서도를 제공합니다.

## 목차

1. [시스템 전체 초기화 흐름](#1-시스템-전체-초기화-흐름)
2. [DeviceNet 태스크 상세 흐름](#2-devicenet-태스크-상세-흐름)
3. [SPI DPM 통신 프로토콜](#3-spi-dpm-통신-프로토콜)
4. [I/O 데이터 교환 흐름](#4-io-데이터-교환-흐름)
5. [Explicit 메시징 흐름](#5-explicit-메시징-흐름)
6. [에러 처리 및 복구 흐름](#6-에러-처리-및-복구-흐름)
7. [태스크 간 상호작용](#7-태스크-간-상호작용)

---

## 1. 시스템 전체 초기화 흐름

### 1.1 부팅 시퀀스

```
전원 ON
  │
  ├─→ [Hardware Reset]
  │     ├─ STM32 Reset Handler
  │     ├─ Stack Pointer 설정
  │     └─ Program Counter 설정
  │
  ├─→ [SystemInit]
  │     ├─ FPU 설정 (Cortex-M4)
  │     ├─ Vector Table 설정
  │     └─ 클럭 사전 설정
  │
  ├─→ [main() 진입]
  │     │
  │     ├─→ HAL_Init()
  │     │     ├─ HAL 라이브러리 초기화
  │     │     ├─ SysTick 설정
  │     │     └─ 우선순위 그룹 설정
  │     │
  │     ├─→ SystemClock_Config()
  │     │     ├─ HSE 클럭 활성화 (8 MHz)
  │     │     ├─ PLL 설정 (180 MHz)
  │     │     ├─ AHB/APB1/APB2 Prescaler 설정
  │     │     └─ Flash Latency 설정
  │     │
  │     ├─→ MX_GPIO_Init()
  │     │     ├─ 모든 GPIO 포트 클럭 활성화
  │     │     ├─ LED 핀 설정
  │     │     ├─ 버튼 핀 설정
  │     │     └─ 기타 GPIO 설정
  │     │
  │     ├─→ MX_SPI5_Init()  ◀── netX 90 통신용
  │     │     ├─ SPI5 클럭 활성화
  │     │     ├─ GPIO AF 설정 (SCK, MISO, MOSI)
  │     │     ├─ SPI 파라미터 설정
  │     │     │   ├─ Mode: Master
  │     │     │   ├─ Direction: Full Duplex
  │     │     │   ├─ Data Size: 8 bits
  │     │     │   ├─ CPOL: Low
  │     │     │   ├─ CPHA: 1 Edge
  │     │     │   └─ Baud Rate: ~11.25 MHz
  │     │     └─ DMA 설정 (선택 사항)
  │     │
  │     ├─→ MX_USART_Init()
  │     │     └─ 디버그/통신용 UART 설정
  │     │
  │     ├─→ MX_TIM_Init()
  │     │     └─ 타이머 초기화
  │     │
  │     └─→ MX_FREERTOS_Init()
  │           │
  │           └─→ osKernelInitialize()
  │                 └─ FreeRTOS 커널 초기화
  │
  └─→ [osKernelStart()]
        │
        └─→ FreeRTOS 스케줄러 시작
              ├─ Idle Task 생성
              ├─ Timer Task 생성 (설정된 경우)
              └─ 사용자 태스크 실행 시작
```

### 1.2 FreeRTOS 태스크 생성 순서

```
MX_FREERTOS_Init()
  │
  ├─→ [1] 기본 애플리케이션 태스크 생성
  │     ├─ DefaultTask (우선순위: 1)
  │     │   └─ 기본 애플리케이션 로직
  │     │
  │     ├─ SensorTask (우선순위: 6)
  │     │   ├─ 스택: 2KB
  │     │   └─ 센서 데이터 수집
  │     │
  │     ├─ ControlTask (우선순위: 4)
  │     │   ├─ 스택: 3KB
  │     │   └─ 제어 알고리즘
  │     │
  │     └─ UITask (우선순위: 2)
  │           ├─ 스택: 2KB
  │           └─ 사용자 인터페이스
  │
  ├─→ [2] DeviceNet 태스크 생성
  │     └─ AppDNS_DeviceNetTask_Create()
  │           │
  │           ├─ xTaskCreate()
  │           │   ├─ 함수: AppDNS_DeviceNetTask
  │           │   ├─ 이름: "DeviceNetTask"
  │           │   ├─ 스택: 4KB (1024 * 4)
  │           │   ├─ 파라미터: NULL
  │           │   ├─ 우선순위: 5 (tskIDLE_PRIORITY + 3)
  │           │   └─ 핸들: &xDeviceNetTaskHandle
  │           │
  │           └─ 반환: pdPASS/pdFAIL
  │
  ├─→ [3] 큐 생성
  │     ├─ xQueueCreate() - 센서 데이터 큐
  │     ├─ xQueueCreate() - 제어 명령 큐
  │     └─ xQueueCreate() - DeviceNet I/O 큐
  │
  ├─→ [4] 세마포어 생성
  │     ├─ xSemaphoreCreateBinary() - SPI 완료 신호
  │     └─ xSemaphoreCreateMutex() - SPI 버스 보호
  │
  └─→ [5] 타이머 생성 (선택 사항)
        └─ xTimerCreate() - 주기적 작업용
```

---

## 2. DeviceNet 태스크 상세 흐름

### 2.1 태스크 시작 및 초기화

```
AppDNS_DeviceNetTask(void *argument)
  │
  ├─→ [Phase 1: 하드웨어 초기화]
  │     │
  │     └─→ SPI_Init()
  │           ├─ GPIO 초기화
  │           │   ├─ CS 핀: GPIOF Pin 6
  │           │   │   ├─ 모드: Output PP
  │           │   │   ├─ 초기값: HIGH (비활성)
  │           │   │   └─ 속도: High
  │           │   │
  │           │   └─ SRDY 핀: GPIOF Pin 7
  │           │       ├─ 모드: Input
  │           │       ├─ Pull: Pull-up
  │           │       └─ 속도: High
  │           │
  │           ├─ SPI5 파라미터 확인
  │           │   ├─ 클럭: 11.25 MHz
  │           │   ├─ 모드: Full Duplex Master
  │           │   └─ 데이터 크기: 8 bits
  │           │
  │           └─ 초기화 완료 확인
  │                 ├─ 성공 → 다음 단계
  │                 └─ 실패 → 에러 처리
  │
  ├─→ [Phase 2: cifX 드라이버 초기화]
  │     │
  │     └─→ xDriverOpen(&hDriver)
  │           │
  │           ├─→ OS 추상화 레이어 초기화
  │           │     ├─ OS_CreateMutex() × N
  │           │     │   └─ FreeRTOS: xSemaphoreCreateMutex()
  │           │     │
  │           │     ├─ OS_Memalloc() - 내부 버퍼
  │           │     │   └─ FreeRTOS: pvPortMalloc()
  │           │     │       ├─ 크기: ~16KB
  │           │     │       └─ 힙에서 할당
  │           │     │
  │           │     └─ OS_CreateEvent() (선택 사항)
  │           │
  │           ├─→ SerialDPM 인터페이스 초기화
  │           │     ├─ netX 90 SPI DPM 설정
  │           │     ├─ 핸드셰이크 프로토콜 준비
  │           │     └─ 버퍼 할당
  │           │
  │           ├─→ 드라이버 구조 초기화
  │           │     ├─ 디바이스 리스트 생성
  │           │     ├─ 핸들 테이블 초기화
  │           │     └─ 상태 플래그 설정
  │           │
  │           └─ 반환
  │                 ├─ CIFX_NO_ERROR → 성공
  │                 └─ 에러 코드 → 실패
  │
  ├─→ [Phase 3: cifX 채널 열기]
  │     │
  │     └─→ xChannelOpen(hDriver, "cifX0", 0, &hDevice)
  │           │
  │           ├─→ [Step 1] DPM 구조 읽기
  │           │     │
  │           │     ├─ DPM_Read(0x0000, buffer, 64)
  │           │     │   ├─ CS 활성화
  │           │     │   ├─ SRDY 대기 (타임아웃: 1000ms)
  │           │     │   ├─ 명령: 0x00, 0x00, 0x00
  │           │     │   ├─ 64바이트 읽기
  │           │     │   └─ CS 비활성화
  │           │     │
  │           │     └─ DPM 헤더 검증
  │           │           ├─ 매직 넘버 확인: 0x4E657458
  │           │           ├─ DPM 버전 확인
  │           │           ├─ 채널 정보 읽기
  │           │           └─ 통신 플래그 위치 확인
  │           │
  │           ├─→ [Step 2] 통신 플래그 설정
  │           │     │
  │           │     ├─ HOST_COM_FLAG 읽기
  │           │     ├─ 핸드셰이크 비트 설정
  │           │     │   ├─ HOST_COS_FLAG_SET
  │           │     │   └─ NETX_TO_HOST_FLAG_CLEAR
  │           │     └─ HOST_COM_FLAG 쓰기
  │           │
  │           ├─→ [Step 3] 채널 정보 읽기
  │           │     ├─ 채널 0 구조 읽기
  │           │     ├─ I/O 영역 주소 확인
  │           │     │   ├─ 입력 영역: 오프셋, 크기
  │           │     │   └─ 출력 영역: 오프셋, 크기
  │           │     └─ 메일박스 영역 주소 확인
  │           │
  │           ├─→ [Step 4] 채널 핸들 생성
  │           │     ├─ 채널 구조 할당
  │           │     ├─ 뮤텍스 생성
  │           │     └─ 핸들 반환
  │           │
  │           └─ 반환
  │                 ├─ CIFX_NO_ERROR → 성공
  │                 └─ 에러 코드 → 실패
  │
  ├─→ [Phase 4: DeviceNet 스택 초기화]
  │     │
  │     └─→ AppDNS_DemoApplication_Init()
  │           │
  │           ├─→ [Step 1] 네트워크 파라미터 설정
  │           │     │
  │           │     ├─ DNS_SET_CONFIGURATION 패킷 준비
  │           │     │   ├─ MAC ID: 0-63 (예: 5)
  │           │     │   ├─ Baud Rate:
  │           │     │   │   ├─ 125 kbps (0)
  │           │     │   │   ├─ 250 kbps (1)
  │           │     │   │   └─ 500 kbps (2)
  │           │     │   └─ Node Address: MAC ID
  │           │     │
  │           │     └─ xChannelPutPacket()
  │           │           ├─ 패킷 전송 (타임아웃: 5000ms)
  │           │           └─ 응답 대기
  │           │
  │           ├─→ [Step 2] I/O Assembly 설정
  │           │     │
  │           │     ├─ DNS_SET_IO_ASSEMBLY 패킷 준비
  │           │     │   ├─ 입력 Assembly:
  │           │     │   │   ├─ Instance: 100
  │           │     │   │   ├─ 크기: 8 바이트
  │           │     │   │   └─ 타입: Polled
  │           │     │   │
  │           │     │   └─ 출력 Assembly:
  │           │     │       ├─ Instance: 150
  │           │     │       ├─ 크기: 8 바이트
  │           │     │       └─ 타입: Polled
  │           │     │
  │           │     └─ xChannelPutPacket()
  │           │           └─ 응답 확인
  │           │
  │           ├─→ [Step 3] Object Dictionary 등록
  │           │     │
  │           │     ├─ Identity Object (Class 0x01)
  │           │     │   ├─ Vendor ID
  │           │     │   ├─ Device Type
  │           │     │   ├─ Product Code
  │           │     │   └─ Revision
  │           │     │
  │           │     ├─ Assembly Object (Class 0x04)
  │           │     │   ├─ 입력 Assembly 데이터
  │           │     │   └─ 출력 Assembly 데이터
  │           │     │
  │           │     └─ Connection Object (Class 0x05)
  │           │         └─ Connection 파라미터
  │           │
  │           ├─→ [Step 4] 네트워크 시작
  │           │     │
  │           │     ├─ DNS_START_NETWORK 패킷 전송
  │           │     ├─ 온라인 상태 대기
  │           │     │   ├─ 폴링 간격: 100ms
  │           │     │   └─ 최대 대기: 10초
  │           │     │
  │           │     └─ 상태 확인
  │           │           ├─ DNS_STATE_ONLINE → 성공
  │           │           └─ 기타 → 에러
  │           │
  │           └─ 반환
  │                 ├─ 0 → 성공
  │                 └─ -1 → 실패
  │
  ├─→ [Phase 5: 초기화 결과 확인]
  │     │
  │     ├─ if (초기화 실패)
  │     │   ├─ 에러 로깅
  │     │   ├─ LED 에러 표시
  │     │   └─ vTaskSuspend(NULL)  ← 태스크 일시 중단
  │     │
  │     └─ if (초기화 성공)
  │           ├─ 성공 로깅
  │           ├─ LED 정상 표시
  │           └─ 메인 루프 진입
  │
  └─→ [Phase 6: 메인 루프]
        │
        └─→ 무한 루프 (다음 섹션 참조)
```

### 2.2 메인 루프 (주기적 실행)

```
for (;;) {  // 무한 루프
  │
  ├─→ [1] DeviceNet 애플리케이션 실행
  │     │
  │     └─→ AppDNS_DemoApplication_Run()
  │           │
  │           ├─→ [A] I/O 데이터 처리
  │           │     │
  │           │     ├─→ 입력 데이터 읽기
  │           │     │     │
  │           │     │     ├─ 큐에서 센서 데이터 가져오기
  │           │     │     │   └─ xQueueReceive(sensorQueue, &data, 0)
  │           │     │     │
  │           │     │     ├─ 데이터 포맷 변환
  │           │     │     │   ├─ 압력: bar → 0.01 bar 단위
  │           │     │     │   ├─ 온도: °C → 0.1°C 단위
  │           │     │     │   └─ 상태: 비트 필드로 변환
  │           │     │     │
  │           │     │     └─ xChannelIOWrite()
  │           │     │           │
  │           │     │           ├─ Mutex 획득 (100ms)
  │           │     │           ├─ DPM 입력 영역 주소 계산
  │           │     │           ├─ DPM_Write(addr, data, len)
  │           │     │           │   ├─ SRDY 대기
  │           │     │           │   ├─ SPI 전송
  │           │     │           │   └─ 완료 확인
  │           │     │           └─ Mutex 해제
  │           │     │
  │           │     ├─→ 출력 데이터 쓰기
  │           │     │     │
  │           │     │     ├─ xChannelIORead()
  │           │     │     │   │
  │           │     │     │   ├─ Mutex 획득 (100ms)
  │           │     │     │   ├─ DPM 출력 영역 주소 계산
  │           │     │     │   ├─ DPM_Read(addr, data, len)
  │           │     │     │   │   ├─ SRDY 대기
  │           │     │     │   │   ├─ SPI 수신
  │           │     │     │   │   └─ 데이터 확인
  │           │     │     │   └─ Mutex 해제
  │           │     │     │
  │           │     │     ├─ 데이터 포맷 변환
  │           │     │     │   ├─ 밸브 제어: 0-10000 → 0-100%
  │           │     │     │   ├─ 디지털 출력: 비트 필드
  │           │     │     │   └─ 제어 모드: 열거형
  │           │     │     │
  │           │     │     └─ 큐로 제어 데이터 전송
  │           │     │           └─ xQueueSend(controlQueue, &data, 0)
  │           │     │
  │           │     └─→ 데이터 동기화
  │           │           ├─ 입력 버퍼 교환 플래그 설정
  │           │           └─ 출력 버퍼 교환 플래그 설정
  │           │
  │           ├─→ [B] Explicit 메시지 처리
  │           │     │
  │           │     ├─→ 수신 메시지 확인
  │           │     │     │
  │           │     │     └─ xChannelGetPacket()
  │           │     │           │
  │           │     │           ├─ 메일박스 확인
  │           │     │           │   ├─ DPM_Read(mailbox_addr, &header, 8)
  │           │     │           │   ├─ 패킷 존재 확인
  │           │     │           │   └─ 패킷 길이 확인
  │           │     │           │
  │           │     │           ├─ 패킷 데이터 읽기
  │           │     │           │   └─ DPM_Read(data_addr, packet, len)
  │           │     │           │
  │           │     │           └─ 메일박스 플래그 클리어
  │           │     │
  │           │     ├─→ 메시지 타입 확인
  │           │     │     │
  │           │     │     ├─ if (Explicit Request)
  │           │     │     │   │
  │           │     │     │   ├─ 서비스 코드 파싱
  │           │     │     │   │   ├─ GET_ATTRIBUTE_SINGLE (0x0E)
  │           │     │     │   │   ├─ SET_ATTRIBUTE_SINGLE (0x10)
  │           │     │     │   │   ├─ GET_ATTRIBUTE_ALL (0x01)
  │           │     │     │   │   └─ RESET (0x05)
  │           │     │     │   │
  │           │     │     │   ├─ Object Dictionary 액세스
  │           │     │     │   │   ├─ Class ID 확인
  │           │     │     │   │   ├─ Instance ID 확인
  │           │     │     │   │   ├─ Attribute ID 확인
  │           │     │     │   │   └─ 데이터 읽기/쓰기
  │           │     │     │   │
  │           │     │     │   └─ 응답 생성
  │           │     │     │
  │           │     │     └─ if (Indication)
  │           │     │           └─ 상태 업데이트
  │           │     │
  │           │     └─→ 응답 전송
  │           │           │
  │           │           └─ xChannelPutPacket()
  │           │                 │
  │           │                 ├─ 응답 패킷 준비
  │           │                 ├─ DPM_Write(mailbox_addr, packet, len)
  │           │                 └─ 전송 플래그 설정
  │           │
  │           └─→ [C] 네트워크 상태 모니터링
  │                 │
  │                 ├─ 연결 상태 확인
  │                 │   ├─ DPM_Read(status_addr, &status, 4)
  │                 │   └─ DNS_STATE_ONLINE 확인
  │                 │
  │                 ├─ 에러 플래그 확인
  │                 │   ├─ BUS_OFF
  │                 │   ├─ TIMEOUT
  │                 │   ├─ DUPLICATE_MAC
  │                 │   └─ CAN_ERROR
  │                 │
  │                 └─ 타임아웃 처리
  │                     ├─ 마지막 통신 시간 확인
  │                     └─ 워치독 갱신
  │
  └─→ [2] 태스크 지연
        │
        └─→ vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10))
              │
              ├─ 정확한 10ms 주기 보장
              └─ 다른 태스크에 CPU 양보
}
```

---

## 3. SPI DPM 통신 프로토콜

### 3.1 DPM 읽기 상세 프로토콜

```
DPM_Read(address, buffer, length)
  │
  ├─→ [1] 사전 조건 확인
  │     ├─ address < DPM_SIZE
  │     ├─ buffer != NULL
  │     └─ length > 0
  │
  ├─→ [2] Mutex 획득
  │     │
  │     └─→ OS_WaitMutex(g_hSpiMutex, 1000)
  │           │
  │           ├─ xSemaphoreTake(hMutex, pdMS_TO_TICKS(1000))
  │           │   │
  │           │   ├─ if (획득 성공)
  │           │   │   └─ 계속 진행
  │           │   │
  │           │   └─ if (타임아웃)
  │           │         ├─ 에러 로깅
  │           │         └─ return -1
  │           │
  │           └─ SPI 버스 배타적 액세스 확보
  │
  ├─→ [3] SRDY 대기
  │     │
  │     └─→ SPI_WaitSRDY(1000)
  │           │
  │           ├─ startTime = HAL_GetTick()
  │           │
  │           └─ while ((HAL_GetTick() - startTime) < 1000)
  │                 │
  │                 ├─ state = HAL_GPIO_ReadPin(SRDY_PORT, SRDY_PIN)
  │                 │
  │                 ├─ if (state == GPIO_PIN_SET)
  │                 │   └─ return 0  // SRDY 활성
  │                 │
  │                 ├─ HAL_Delay(1)  // 1ms 대기
  │                 │
  │                 └─ if (타임아웃)
  │                       ├─ 에러 로깅: "SRDY timeout"
  │                       ├─ Mutex 해제
  │                       └─ return -2
  │
  ├─→ [4] CS 활성화
  │     │
  │     └─→ SPI_SetCS(1)
  │           │
  │           ├─ HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET)
  │           │   └─ CS를 LOW로 설정 (활성)
  │           │
  │           └─ 짧은 지연 (10ns 최소, 실제로는 명령어 지연으로 충분)
  │
  ├─→ [5] 읽기 명령 전송
  │     │
  │     ├─ 명령 바이트 준비
  │     │   ├─ cmd[0] = 0x00        // 읽기 명령
  │     │   ├─ cmd[1] = address >> 8   // 주소 High byte
  │     │   └─ cmd[2] = address & 0xFF // 주소 Low byte
  │     │
  │     └─→ SPI_Transfer(cmd, dummy, 3)
  │           │
  │           └─→ HAL_SPI_TransmitReceive(&hspi5, cmd, dummy, 3, 1000)
  │                 │
  │                 ├─ SPI 하드웨어로 3바이트 송신
  │                 ├─ 동시에 3바이트 수신 (무시)
  │                 │
  │                 └─ if (HAL_OK)
  │                       └─ 계속 진행
  │                     else
  │                       ├─ CS 비활성화
  │                       ├─ Mutex 해제
  │                       └─ return -3
  │
  ├─→ [6] 데이터 수신
  │     │
  │     ├─ 더미 송신 버퍼 준비 (0xFF × length)
  │     │
  │     └─→ SPI_Transfer(dummy_tx, buffer, length)
  │           │
  │           └─→ HAL_SPI_TransmitReceive(&hspi5, dummy_tx, buffer, length, 1000)
  │                 │
  │                 ├─ 더미 데이터 송신 (클럭 생성용)
  │                 ├─ 실제 데이터 수신 → buffer
  │                 │
  │                 └─ 데이터 확인 (CRC 등, 구현 시)
  │
  ├─→ [7] CS 비활성화
  │     │
  │     └─→ SPI_SetCS(0)
  │           │
  │           ├─ HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET)
  │           │   └─ CS를 HIGH로 설정 (비활성)
  │           │
  │           └─ 짧은 지연 (10ns 최소)
  │
  ├─→ [8] SRDY 해제 대기
  │     │
  │     └─→ while (SPI_GetSRDY() == 1)
  │           │
  │           ├─ if (timeout > 100ms)
  │           │   └─ break  // 타임아웃은 심각하지 않음
  │           │
  │           └─ delay(10us)
  │
  ├─→ [9] Mutex 해제
  │     │
  │     └─→ OS_ReleaseMutex(g_hSpiMutex)
  │           │
  │           └─ xSemaphoreGive(hMutex)
  │                 └─ SPI 버스 해제, 다른 태스크 액세스 가능
  │
  └─→ [10] 결과 반환
        │
        ├─ return 0  // 성공
        └─ return -n  // 에러 코드
```

### 3.2 DPM 쓰기 상세 프로토콜

```
DPM_Write(address, data, length)
  │
  ├─→ [1-3] 동일 (사전 조건, Mutex, SRDY)
  │
  ├─→ [4] CS 활성화
  │     └─ (읽기와 동일)
  │
  ├─→ [5] 쓰기 명령 전송
  │     │
  │     ├─ 명령 바이트 준비
  │     │   ├─ cmd[0] = 0x80        // 쓰기 명령 (0x80 = write bit)
  │     │   ├─ cmd[1] = address >> 8
  │     │   └─ cmd[2] = address & 0xFF
  │     │
  │     └─→ SPI_Transfer(cmd, dummy, 3)
  │
  ├─→ [6] 데이터 송신
  │     │
  │     └─→ SPI_Transfer(data, dummy_rx, length)
  │           │
  │           └─→ HAL_SPI_TransmitReceive(&hspi5, data, dummy_rx, length, 1000)
  │                 │
  │                 ├─ data 송신 → DPM
  │                 └─ 수신 데이터는 무시
  │
  ├─→ [7-10] 동일 (CS 비활성화, SRDY 대기, Mutex 해제, 반환)
  │
  └─→ return 0 / -n
```

### 3.3 SPI 타이밍 다이어그램

```
시간 →

CS    ────┐                                      ┌────
          └──────────────────────────────────────┘
          ↑                                      ↑
        Active                                Inactive

SRDY  ─────────┐                          ┌──────────
               └──────────────────────────┘
               ↑                          ↑
            Ready                      Released
            (Host가 확인)               (완료 신호)

CLK   ────────┐┌┐┌┐┌┐┌┐┌┐┌┐┌┐┌┐┌┐┌┐┌┐┌┐┌┐┌┐┌┐┌────
              └┘└┘└┘└┘└┘└┘└┘└┘└┘└┘└┘└┘└┘└┘└┘└
              ↑                              ↑
           11.25 MHz (88.9ns/cycle)

MOSI  ────────┤C0├─┤AH├─┤AL├─┤D0├─┤D1├─┤D2├─────
              └───┘ └───┘ └───┘ └───┘ └───┘
              ↑    ↑    ↑    ↑    ↑    ↑
            CMD  Addr  Addr  Data Data Data
           (0x00) (Hi)  (Lo) Byte0 Byte1 Byte2

MISO  ────────┤XX├─┤XX├─┤XX├─┤D0├─┤D1├─┤D2├─────
              └───┘ └───┘ └───┘ └───┘ └───┘
                                ↑    ↑    ↑
                              받은 데이터

타이밍 파라미터:
- CS setup time: 10ns (최소)
- CS hold time: 10ns (최소)
- SRDY response: < 1ms (전형적)
- 바이트 간 간격: 0 (연속 전송)
- SPI 클럭: 11.25 MHz (88.9ns/cycle)
```

---

## 4. I/O 데이터 교환 흐름

### 4.1 센서 → DeviceNet 입력 데이터 흐름

```
[센서 Task] ─────→ [센서 큐] ─────→ [DeviceNet Task] ─────→ [DPM] ─────→ [netX 90] ─────→ [DeviceNet 네트워크]
   5ms 주기           버퍼링              10ms 주기          SPI 전송     프로토콜 처리       마스터로 전송

상세 흐름:

센서 Task (5ms 주기)
  │
  ├─→ 센서 데이터 읽기
  │     ├─ ADC 읽기 (압력 센서)
  │     ├─ SPI 읽기 (온도 센서)
  │     └─ GPIO 읽기 (디지털 입력)
  │
  ├─→ 데이터 처리
  │     ├─ 필터링 (이동 평균)
  │     ├─ 스케일링 (물리값 변환)
  │     └─ 상태 플래그 생성
  │
  └─→ 큐로 전송
        │
        └─→ xQueueSend(sensorQueue, &sensorData, 0)
              ├─ if (큐 가득참)
              │   └─ 오래된 데이터 덮어쓰기
              └─ 성공

                    ▼

DeviceNet Task (10ms 주기)
  │
  ├─→ 큐에서 데이터 수신
  │     │
  │     └─→ xQueueReceive(sensorQueue, &data, 0)
  │           ├─ if (데이터 있음)
  │           │   └─ 최신 데이터 가져오기
  │           └─ if (데이터 없음)
  │                 └─ 이전 데이터 유지
  │
  ├─→ DeviceNet 형식으로 변환
  │     │
  │     ├─ typedef struct {
  │     │     uint16_t pressure1;    // 0.01 bar 단위
  │     │     uint16_t pressure2;    // 0.01 bar 단위
  │     │     uint16_t temperature;  // 0.1°C 단위
  │     │     uint8_t  digital_in;   // 비트 필드
  │     │     uint8_t  status;       // 상태 플래그
  │     │   } DN_InputAssembly_t;
  │     │
  │     └─ 변환 수행
  │           ├─ pressure1 = sensor.pressure1 * 100
  │           ├─ pressure2 = sensor.pressure2 * 100
  │           ├─ temperature = sensor.temp * 10
  │           ├─ digital_in = sensor.digital_bits
  │           └─ status = sensor.status_flags
  │
  └─→ DPM에 쓰기
        │
        └─→ xChannelIOWrite(hDevice, 0, 0, sizeof(data), &data, 100)
              │
              ├─→ Mutex 획득
              ├─→ DPM 입력 영역 주소 계산
              │     └─ addr = DPM_INPUT_BASE + offset
              │
              ├─→ DPM_Write(addr, &data, sizeof(data))
              │     ├─ SRDY 대기
              │     ├─ CS 활성화
              │     ├─ 명령 전송: 0x80, addr_hi, addr_lo
              │     ├─ 데이터 전송: 8 바이트
              │     └─ CS 비활성화
              │
              └─→ Mutex 해제

                    ▼

netX 90 (펌웨어)
  │
  ├─→ DPM 입력 데이터 감지
  ├─→ DeviceNet 프로토콜 처리
  │     ├─ Polled I/O Connection
  │     └─ CAN 프레임 생성
  │
  └─→ DeviceNet 네트워크로 전송
        └─→ 마스터 PLC가 데이터 수신
```

### 4.2 DeviceNet 출력 → 액추에이터 데이터 흐름

```
[DeviceNet 네트워크] ─────→ [netX 90] ─────→ [DPM] ─────→ [DeviceNet Task] ─────→ [제어 큐] ─────→ [제어 Task]
    마스터 PLC 전송         프로토콜 수신    SPI 수신         10ms 주기            버퍼링            20ms 주기

상세 흐름:

DeviceNet 네트워크
  │
  └─→ 마스터 PLC가 출력 데이터 전송
        ├─ 밸브 제어값
        ├─ 디지털 출력
        └─ 제어 모드

                    ▼

netX 90 (펌웨어)
  │
  ├─→ CAN 프레임 수신
  ├─→ DeviceNet 프로토콜 처리
  │     └─ Polled I/O Connection
  │
  └─→ DPM 출력 영역에 쓰기
        └─ 출력 Assembly 데이터 업데이트

                    ▼

DeviceNet Task (10ms 주기)
  │
  ├─→ DPM에서 읽기
  │     │
  │     └─→ xChannelIORead(hDevice, 0, 0, sizeof(data), &data, &len, 100)
  │           │
  │           ├─→ Mutex 획득
  │           ├─→ DPM 출력 영역 주소 계산
  │           │     └─ addr = DPM_OUTPUT_BASE + offset
  │           │
  │           ├─→ DPM_Read(addr, &data, sizeof(data))
  │           │     ├─ SRDY 대기
  │           │     ├─ CS 활성화
  │           │     ├─ 명령 전송: 0x00, addr_hi, addr_lo
  │           │     ├─ 데이터 수신: 8 바이트
  │           │     └─ CS 비활성화
  │           │
  │           └─→ Mutex 해제
  │
  ├─→ 애플리케이션 형식으로 변환
  │     │
  │     ├─ typedef struct {
  │     │     uint16_t valve_control;  // 0-10000 (0-100%)
  │     │     uint8_t  digital_out;    // 비트 필드
  │     │     uint8_t  control_mode;   // 모드
  │     │   } DN_OutputAssembly_t;
  │     │
  │     └─ 변환 수행
  │           ├─ control.valve = dn_output.valve_control / 100.0
  │           ├─ control.digital = dn_output.digital_out
  │           └─ control.mode = dn_output.control_mode
  │
  └─→ 큐로 전송
        │
        └─→ xQueueSend(controlQueue, &controlData, 0)
              └─ 제어 Task가 처리

                    ▼

제어 Task (20ms 주기)
  │
  ├─→ 큐에서 제어 명령 수신
  │     └─→ xQueueReceive(controlQueue, &cmd, 0)
  │
  ├─→ 제어 알고리즘 실행
  │     ├─ PID 제어
  │     ├─ 밸브 위치 계산
  │     └─ 안전 검사
  │
  └─→ 액추에이터 제어
        ├─ DAC 출력 (밸브 제어)
        ├─ GPIO 출력 (디지털 출력)
        └─ 모터 제어
```

---

## 5. Explicit 메시징 흐름

### 5.1 읽기 요청 처리

```
[마스터 PLC] ─────→ [DeviceNet] ─────→ [netX 90] ─────→ [DPM Mailbox] ─────→ [DeviceNet Task]
   읽기 요청            CAN 전송         프로토콜 처리      메시지 저장           요청 처리

상세 흐름:

마스터 PLC
  │
  └─→ Explicit 읽기 요청 전송
        ├─ 서비스: GET_ATTRIBUTE_SINGLE (0x0E)
        ├─ Class: 0x01 (Identity)
        ├─ Instance: 0x01
        ├─ Attribute: 0x01 (Vendor ID)
        └─ DeviceNet 네트워크로 전송

                    ▼

netX 90
  │
  ├─→ CAN 메시지 수신
  ├─→ Explicit 메시지 인식
  ├─→ DPM 메일박스에 저장
  │     ├─ 메일박스 헤더 업데이트
  │     │   ├─ 패킷 길이
  │     │   ├─ 유효 플래그
  │     │   └─ 시퀀스 번호
  │     │
  │     └─ 메시지 데이터 쓰기
  │
  └─→ Host 인터럽트 (선택 사항)

                    ▼

DeviceNet Task (10ms 주기)
  │
  ├─→ [1] 메일박스 확인
  │     │
  │     └─→ xChannelGetPacket(hDevice, sizeof(packet), &packet, &len, 10)
  │           │
  │           ├─→ Mutex 획득
  │           │
  │           ├─→ 메일박스 헤더 읽기
  │           │     │
  │           │     └─→ DPM_Read(MAILBOX_HEADER_ADDR, &header, 8)
  │           │           ├─ 유효 플래그 확인
  │           │           ├─ 패킷 길이 확인
  │           │           └─ if (패킷 없음) return TIMEOUT
  │           │
  │           ├─→ 패킷 데이터 읽기
  │           │     │
  │           │     └─→ DPM_Read(MAILBOX_DATA_ADDR, &packet, len)
  │           │           └─ Explicit 메시지 수신
  │           │
  │           ├─→ 메일박스 플래그 클리어
  │           │     └─→ DPM_Write(MAILBOX_FLAGS_ADDR, &clear, 4)
  │           │
  │           └─→ Mutex 해제
  │
  ├─→ [2] 메시지 파싱
  │     │
  │     ├─ 서비스 코드 확인: 0x0E
  │     ├─ Class ID: 0x01
  │     ├─ Instance ID: 0x01
  │     └─ Attribute ID: 0x01
  │
  ├─→ [3] Object Dictionary 액세스
  │     │
  │     └─→ HandleExplicitReadRequest(class, instance, attribute, &data, &len)
  │           │
  │           ├─ switch (class)
  │           │   │
  │           │   └─ case 0x01:  // Identity Object
  │           │         │
  │           │         └─ switch (attribute)
  │           │               │
  │           │               ├─ case 0x01:  // Vendor ID
  │           │               │   ├─ data = VENDOR_ID (0x1234)
  │           │               │   ├─ len = 2
  │           │               │   └─ return SUCCESS
  │           │               │
  │           │               ├─ case 0x02:  // Device Type
  │           │               │   └─ ...
  │           │               │
  │           │               └─ default:
  │           │                     └─ return ERROR_ATTRIBUTE_NOT_SUPPORTED
  │           │
  │           └─ 데이터 반환
  │
  ├─→ [4] 응답 패킷 생성
  │     │
  │     ├─ response.service = 0x8E  // Response bit set
  │     ├─ response.status = 0x00   // Success
  │     ├─ response.data = vendor_id
  │     └─ response.data_len = 2
  │
  └─→ [5] 응답 전송
        │
        └─→ xChannelPutPacket(hDevice, &response, 1000)
              │
              ├─→ Mutex 획득
              │
              ├─→ 메일박스에 응답 쓰기
              │     │
              │     └─→ DPM_Write(MAILBOX_DATA_ADDR, &response, len)
              │           └─ 응답 패킷 저장
              │
              ├─→ 메일박스 플래그 설정
              │     └─→ DPM_Write(MAILBOX_FLAGS_ADDR, &flags, 4)
              │           └─ HOST_PACKET_READY 플래그
              │
              └─→ Mutex 해제

                    ▼

netX 90
  │
  ├─→ 메일박스 응답 감지
  ├─→ DeviceNet 응답 프로토콜 처리
  └─→ CAN 메시지로 전송

                    ▼

마스터 PLC
  │
  └─→ 응답 수신
        ├─ Vendor ID = 0x1234
        └─ 처리 완료
```

### 5.2 쓰기 요청 처리

```
쓰기 요청 (SET_ATTRIBUTE_SINGLE, 0x10)

마스터 PLC → netX 90 → DPM → DeviceNet Task
  │
  ├─→ [1-2] 읽기와 동일 (메일박스 확인, 메시지 파싱)
  │
  ├─→ [3] Object Dictionary 업데이트
  │     │
  │     └─→ HandleExplicitWriteRequest(class, instance, attribute, data, len)
  │           │
  │           ├─ switch (class)
  │           │   │
  │           │   └─ case 0x64:  // Application Object
  │           │         │
  │           │         └─ switch (attribute)
  │           │               │
  │           │               ├─ case 0x01:  // Setpoint
  │           │               │   ├─ if (len == 2)
  │           │               │   │   ├─ setpoint = *(uint16_t*)data
  │           │               │   │   ├─ SetControlSetpoint(setpoint)
  │           │               │   │   └─ return SUCCESS
  │           │               │   └─ else
  │           │               │         └─ return ERROR_INVALID_SIZE
  │           │               │
  │           │               └─ case 0x02:  // Control Mode
  │           │                     └─ ...
  │           │
  │           └─ 결과 반환
  │
  ├─→ [4] 응답 패킷 생성
  │     ├─ response.service = 0x90  // Write response
  │     └─ response.status = 0x00   // Success
  │
  └─→ [5] 응답 전송 (읽기와 동일)
```

---

## 6. 에러 처리 및 복구 흐름

### 6.1 SPI 통신 에러

```
SPI 전송 실패 감지
  │
  ├─→ [1] 에러 타입 분류
  │     │
  │     ├─→ HAL_TIMEOUT
  │     │     │
  │     │     ├─ 원인 분석
  │     │     │   ├─ SPI 버스 응답 없음
  │     │     │   └─ SRDY 타임아웃
  │     │     │
  │     │     └─ 복구 절차
  │     │           ├─ SPI 하드웨어 리셋
  │     │           │   ├─ __HAL_SPI_DISABLE(&hspi5)
  │     │           │   ├─ HAL_SPI_DeInit(&hspi5)
  │     │           │   ├─ HAL_Delay(10)
  │     │           │   └─ MX_SPI5_Init()
  │     │           │
  │     │           ├─ GPIO 리셋
  │     │           │   ├─ CS = HIGH
  │     │           │   └─ 대기 100ms
  │     │           │
  │     │           └─ 재시도
  │     │                 ├─ 재시도 카운터++
  │     │                 └─ if (재시도 < 3)
  │     │                       └─ 전송 재시도
  │     │                     else
  │     │                       └─ 에러 반환
  │     │
  │     ├─→ HAL_ERROR
  │     │     │
  │     │     └─ 복구 절차
  │     │           ├─ 에러 플래그 확인
  │     │           │   ├─ OVERRUN
  │     │           │   ├─ MODE_FAULT
  │     │           │   └─ CRC_ERROR
  │     │           │
  │     │           ├─ 플래그 클리어
  │     │           │   └─ __HAL_SPI_CLEAR_*_FLAG()
  │     │           │
  │     │           └─ SPI 재초기화
  │     │
  │     └─→ SRDY_TIMEOUT
  │           │
  │           └─ 복구 절차
  │                 ├─ netX 상태 확인
  │                 │   └─ DPM 헤더 읽기 시도
  │                 │
  │                 ├─ if (DPM 액세스 불가)
  │                 │   ├─ netX 하드 리셋 필요
  │                 │   └─ Error_Handler()
  │                 │
  │                 └─ if (DPM 액세스 가능)
  │                       └─ SRDY 신호 문제
  │                             ├─ SRDY 핀 상태 확인
  │                             └─ 하드웨어 점검 필요
  │
  ├─→ [2] 에러 로깅
  │     │
  │     ├─ 에러 카운터 증가
  │     │   ├─ g_spiErrorCount++
  │     │   └─ g_spiErrorHistory[index] = error_code
  │     │
  │     ├─ 타임스탬프 기록
  │     │   └─ error_time = xTaskGetTickCount()
  │     │
  │     └─ 디버그 출력
  │           └─ printf("SPI Error: code=0x%X, time=%u\r\n", code, time)
  │
  └─→ [3] 상위 레벨 알림
        │
        ├─ 이벤트 플래그 설정
        │   └─ xEventGroupSetBits(g_errorEvents, SPI_ERROR_BIT)
        │
        └─ LED 표시
              └─ Set_Error_LED()
```

### 6.2 DeviceNet 네트워크 에러

```
네트워크 에러 감지
  │
  ├─→ [1] 에러 타입 분류
  │     │
  │     ├─→ BUS_OFF
  │     │     │
  │     │     ├─ 원인
  │     │     │   ├─ 과도한 CAN 에러
  │     │     │   ├─ 버스 단선
  │     │     │   └─ 터미네이션 문제
  │     │     │
  │     │     └─ 복구 절차
  │     │           │
  │     │           ├─ [Step 1] 안전 상태 진입
  │     │           │     ├─ 모든 출력 OFF
  │     │           │     │   └─ SetSafeOutputState()
  │     │           │     │
  │     │           │     └─ 에러 상태 표시
  │     │           │
  │     │           ├─ [Step 2] 에러 카운터 리셋
  │     │           │     │
  │     │           │     └─→ DNS_RESET_ERROR_COUNTERS 패킷 전송
  │     │           │           ├─ xChannelPutPacket()
  │     │           │           └─ 응답 대기 (1초)
  │     │           │
  │     │           ├─ [Step 3] 네트워크 재시작
  │     │           │     │
  │     │           │     └─→ DNS_RESTART_NETWORK 패킷 전송
  │     │           │           ├─ xChannelPutPacket()
  │     │           │           └─ 응답 대기 (1초)
  │     │           │
  │     │           ├─ [Step 4] 온라인 대기
  │     │           │     │
  │     │           │     └─→ WaitForOnline(5000)
  │     │           │           │
  │     │           │           └─ for (i = 0; i < 50; i++)
  │     │           │                 ├─ 상태 읽기
  │     │           │                 ├─ if (DNS_STATE_ONLINE)
  │     │           │                 │   └─ return SUCCESS
  │     │           │                 └─ vTaskDelay(100)
  │     │           │
  │     │           └─ [Step 5] 복구 결과
  │     │                 │
  │     │                 ├─ if (복구 성공)
  │     │                 │   ├─ 로그: "Recovered from Bus-Off"
  │     │                 │   ├─ LED 정상 표시
  │     │                 │   └─ 정상 동작 재개
  │     │                 │
  │     │                 └─ if (복구 실패)
  │     │                       ├─ 로그: "Bus-Off recovery failed"
  │     │                       ├─ LED 에러 표시
  │     │                       └─ 수동 개입 필요
  │     │
  │     ├─→ DUPLICATE_MAC_ID
  │     │     │
  │     │     └─ 복구 절차
  │     │           ├─ 충돌 MAC ID 확인
  │     │           ├─ 새 MAC ID 할당
  │     │           │   └─ mac_id = (mac_id + 1) % 64
  │     │           │
  │     │           └─ 네트워크 재시작
  │     │                 └─ (BUS_OFF 복구와 유사)
  │     │
  │     └─→ TIMEOUT
  │           │
  │           └─ 복구 절차
  │                 ├─ 통신 상태 확인
  │                 │   ├─ 마지막 통신 시간
  │                 │   └─ 네트워크 활성 확인
  │                 │
  │                 ├─ if (타임아웃 > 5초)
  │                 │   └─ 네트워크 재시작
  │                 │
  │                 └─ else
  │                       └─ 계속 모니터링
  │
  └─→ [2-3] 에러 로깅 및 알림 (SPI와 동일)
```

---

## 7. 태스크 간 상호작용

### 7.1 태스크 우선순위 및 스케줄링

```
FreeRTOS 스케줄러
  │
  ├─→ 우선순위 6: 센서 Task (5ms 주기)
  │     │
  │     ├─ 최고 우선순위 - 실시간 데이터 수집
  │     ├─ 짧은 실행 시간 (~1ms)
  │     └─ 센서 데이터 큐로 전송
  │
  ├─→ 우선순위 5: DeviceNet Task (10ms 주기)
  │     │
  │     ├─ 높은 우선순위 - 네트워크 통신
  │     ├─ 중간 실행 시간 (~2-3ms)
  │     └─ I/O 큐 및 메시지 처리
  │
  ├─→ 우선순위 4: 제어 Task (20ms 주기)
  │     │
  │     ├─ 중간 우선순위 - 제어 알고리즘
  │     ├─ 긴 실행 시간 (~5ms)
  │     └─ 액추에이터 제어
  │
  ├─→ 우선순위 2: UI Task (100ms 주기)
  │     │
  │     ├─ 낮은 우선순위 - 사용자 인터페이스
  │     ├─ 가변 실행 시간
  │     └─ 디스플레이 업데이트
  │
  └─→ 우선순위 0: Idle Task
        │
        └─ CPU 유휴 시간 처리
```

### 7.2 데이터 흐름 및 동기화

```
                센서 큐           DeviceNet I/O          제어 큐
                (10 items)         DPM 영역            (5 items)
                    ↓                  ↓                   ↓

센서 Task  ─────→ │ │ ─────→ DeviceNet Task ─────→ │ │ ─────→ 제어 Task
 (5ms)           push           (10ms)              push         (20ms)
   ↓              ↓               ↓                  ↓             ↓
[ADC 읽기]    [큐 쓰기]      [큐 읽기]          [DPM 읽기]    [큐 읽기]
[처리]        [블로킹 안함]   [DPM 쓰기]         [큐 쓰기]     [제어 출력]
              [덮어쓰기 OK]   [SPI 전송]         [블로킹 안함]  [액추에이터]

동기화 메커니즘:
  │
  ├─→ Mutex: g_spiMutex
  │     ├─ SPI 버스 배타적 액세스
  │     ├─ DeviceNet Task ↔ 센서 Task (SPI ADC)
  │     └─ 타임아웃: 100ms
  │
  ├─→ Queue: sensorQueue
  │     ├─ 크기: 10 items × sizeof(SensorData_t)
  │     ├─ 타임아웃: 0 (non-blocking)
  │     └─ 오버플로 정책: 덮어쓰기
  │
  ├─→ Queue: controlQueue
  │     ├─ 크기: 5 items × sizeof(ControlData_t)
  │     ├─ 타임아웃: 0 (non-blocking)
  │     └─ 오버플로 정책: 드롭
  │
  └─→ Event Group: g_errorEvents
        ├─ SPI_ERROR_BIT
        ├─ DEVICENET_ERROR_BIT
        └─ SYSTEM_ERROR_BIT
```

### 7.3 타이밍 분석

```
시간축 (ms) →
0    5    10   15   20   25   30   35   40   45   50
│    │    │    │    │    │    │    │    │    │    │

센서 Task (5ms)
├──┤ ├──┤ ├──┤ ├──┤ ├──┤ ├──┤ ├──┤ ├──┤ ├──┤ ├──┤
S1   S2   S3   S4   S5   S6   S7   S8   S9   S10
↓    ↓    ↓    ↓    ↓    ↓    ↓    ↓    ↓    ↓
큐   큐   큐   큐   큐   큐   큐   큐   큐   큐

DeviceNet Task (10ms)
├────┤    ├────┤    ├────┤    ├────┤    ├────┤
DN1        DN2        DN3        DN4        DN5
↓          ↓          ↓          ↓          ↓
DPM        DPM        DPM        DPM        DPM

제어 Task (20ms)
├────────┤         ├────────┤         ├────────┤
  C1                  C2                  C3
  ↓                   ↓                   ↓
액추에이터          액추에이터          액추에이터

SPI 버스 사용 (Mutex 보호)
├┤ ├┤ ├──┤ ├┤ ├──┤ ├┤ ├──┤ ├┤ ├──┤ ├┤ ├──┤
S1 S2 DN1  S3 DN2  S4 DN3  S5 DN4  S6 DN5

레전드:
S = 센서 SPI 액세스 (~0.5ms)
DN = DeviceNet DPM 액세스 (~2ms)
C = 제어 Task 실행 (~5ms)

CPU 사용률 추정:
- 센서 Task: 0.5ms / 5ms = 10%
- DeviceNet Task: 2ms / 10ms = 20%
- 제어 Task: 5ms / 20ms = 25%
- 기타 (UI, Idle): ~45%
- 총 CPU 사용률: ~55% (충분한 여유)
```

---

## 결론

이 문서는 DeviceNet 통합 펌웨어의 상세한 작동 순서도를 제공합니다. 각 섹션은 초기화부터 실시간 데이터 교환, 에러 처리까지 전체 시스템 동작을 포괄합니다.

### 주요 포인트

1. **계층적 초기화**: 하드웨어 → 드라이버 → 프로토콜 → 애플리케이션
2. **실시간 성능**: 정확한 주기 실행 (10ms DeviceNet, 5ms 센서)
3. **스레드 안전**: Mutex 및 Queue를 통한 동기화
4. **에러 복구**: 체계적인 에러 감지 및 자동 복구
5. **효율적인 통신**: SPI DPM 프로토콜 최적화

### 참고 자료

- 각 섹션의 코드 예제는 해당 README 파일 참조
- 타이밍 요구사항은 netX 90 데이터시트 참조
- DeviceNet 프로토콜은 ODVA 사양 참조

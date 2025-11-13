/* USER CODE BEGIN Header */
/**
 * Louis 2025-07-22 15:54:47 -> 25-10-27 122009 -> 25-11-12 
 * VAT Explicit custom message 통신 - OK 
 * Version : stm32f429_disco_VAT_EDS_0.2.0
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "cifXToolkit.h"
#include "SerialDPMInterface.h"
#include "cifXErrors.h"
#include "App_DemoApplication.h"
#include "Hil_DualPortMemory.h"      // COS flags 정의

/* ========== VAT TEST INTEGRATION - 추가 시작 ========== */
#include "vat_devicenet_test.h"
#include "devicenet_master_info.h"
#include "AppDNS_DemoApplication.h"  // DeviceNet 스택 초기화 함수
#include "App_VAT_Parameters.h"      // VAT 파라미터 매니저
#include "App_VAT_Diagnostic.h"      // VAT 진단 매니저
#include "App_VAT_ExplicitHandler.h" // VAT Explicit 핸들러
/* ========== VAT TEST INTEGRATION - 추가 끝 ========== */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//#define UART_RX_BUFFER_SIZE 256
//typedef struct {
//    uint8_t buffer[UART_RX_BUFFER_SIZE];
//    volatile uint16_t head;
//    volatile uint16_t tail;
//} RingBuffer;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi4;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart5;
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
// 전역 영역에 선언
volatile char g_szLastCookie[5] = {0};   // 외부 모니터링 가능

/* ========== DeviceNet 스택 초기화 - extern 선언 ========== */
extern PROTOCOL_DESCRIPTOR_T g_tRealtimeProtocolHandlers;  // AppDNS_DemoApplication.c에서 정의
extern APP_DATA_T g_tAppData;                              // App_SystemPackets.c에서 정의
extern APP_DNS_DATA_T g_tAppDnsData;                       // AppDNS_DemoApplicationFunctions.c에서 정의
extern uint32_t App_AllChannels_PacketHandler(APP_DATA_T* ptAppData);  // App_DemoApplication.c
/* ========== extern 선언 끝 ========== */

uint8_t rx_data[10] = {0};
uint32_t delayCount =0;

uint8_t rxBuffer[64];
uint8_t rxByte;
uint8_t txMessage[] = "STM32 UART5 says hello to netX90!\r\n";
volatile uint8_t rxReady = 0;

static UART_HandleTypeDef *huart_ring;
static RingBuffer rx_ring;

static uint8_t rx_temp; // 수신용 임시 바이트 저장

uint8_t line[100];

/** DIP switch validation variables */
volatile uint8_t g_DipSwitchData[5] = {0};  // Validated DIP switch data: "1010" + null terminator
volatile uint8_t g_bDipDataValid = 0;       // Flag indicating valid DIP data received

/* VAT 시뮬레이션 상태 */
typedef struct {
    int16_t current_pressure;
    int16_t current_position;
    uint8_t device_status;
    uint8_t exception_status;
} VAT_SIM_STATE_T;

static VAT_SIM_STATE_T g_tVatSimState = {0};

/* ========== VAT TEST INTEGRATION - 추가 시작 ========== */
/**
 * VAT 테스트 컨텍스트 전역 변수
 * - Live Expression으로 모니터링 가능
 * - 디버거에서 실시간 데이터 확인
 */
VAT_TEST_CONTEXT_T g_tVatContext;

/**
 * VAT 테스트 모드 선택
 * 0 = 기본 App_CifXApplicationDemo 실행 (기존 동작)
 * 1 = VAT Basic Pressure Control Test
 * 2 = VAT Full Calibration Test
 * 3 = VAT Continuous Monitoring (100 cycles)
 */
//volatile uint8_t g_VatTestMode = 1;  // 기본값: Basic Pressure Control
// volatile uint8_t g_VatTestMode = 3;  // 기본값: Basic Pressure Control
volatile uint8_t g_VatTestMode = 4;  // 기본값: Basic Pressure Control

/**
 * VAT 테스트 활성화 플래그
 * true = VAT 테스트 실행
 * false = 기존 App_CifXApplicationDemo 실행
 */
volatile bool g_bEnableVatTest = true; // false; //
/* ========== VAT TEST INTEGRATION - 추가 끝 ========== */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_SPI4_Init(void);
static void MX_UART5_Init(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
static int32_t PreCommunicateWithSPI0(void);
static uint8_t ExtractDipSwitchData(const uint8_t *line, uint8_t *dipData);
static uint8_t ValidateDipSwitchData(const uint8_t *currentLine);

/* ========== VAT TEST INTEGRATION - 추가 시작 ========== */
static void PrintDeviceInformation(CIFXHANDLE hDriver, CIFXHANDLE hChannel);
static void PrintDeviceNetStatus(APP_DATA_T* ptAppData);  // ⭐ DeviceNet 상태 디버깅
static void VAT_InitializeTest(void);
static void VAT_RunTest(CIFXHANDLE hChannel);
/* ========== VAT TEST INTEGRATION - 추가 끝 ========== */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief Redirect printf to UART5
 * @param ch Character to send
 * @return Character sent
 *
 * 최적화: 짧은 타임아웃으로 속도 개선
 * 115200 baud: 1 byte = ~87 μs, 10ms면 충분
 */
int __io_putchar(int ch)
{
    /* Send character via UART5 with 10ms timeout */
    // HAL_UART_Transmit(&huart5, (uint8_t *)&ch, 1, 10);
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 10);
    return ch;
}

static bool isCookieAvailable(PDEVICEINSTANCE ptDevInstance, uint32_t ulTimeoutInMs)
{
  bool fCookieAvailable = false;
  char szCookie[5];
  uint32_t starttime;
  uint32_t difftime = 0;

  starttime = OS_GetMilliSecCounter();

  while(false == fCookieAvailable && difftime < ulTimeoutInMs)
  {
    OS_Memset(szCookie, 0, sizeof(szCookie));

    HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);

    // 전역 변수로 복사 (null termination 포함) Louis
//    szCookie[4] = '\0';  // 안전하게 null-terminated 보장
    OS_Memcpy((void*)g_szLastCookie, szCookie, sizeof(g_szLastCookie));

    /** on DPM cards we need to check the for a valid cookie */
    if((0 == OS_Strcmp(szCookie, CIFX_DPMSIGNATURE_BSL_STR)) || (0 == OS_Strcmp(szCookie, CIFX_DPMSIGNATURE_FW_STR)))
    {
      /** We have a firmware or bootloader running, so we assume it is a flash based device */
      /** NOTE: If the driver is restarted and a RAM based FW was downloaded before this
       will result in the device being handled as flash based.
       Currently there is no way to detect this */
      fCookieAvailable = true;
    }
    else
    {
      fCookieAvailable = false;
      difftime = OS_GetMilliSecCounter() - starttime;
    }
  }

  if(false == fCookieAvailable)
  {
    printf("DPM cookie not available since %u milliseconds\r\n", (unsigned int) ulTimeoutInMs);
  }

  return fCookieAvailable;
}

static int32_t InitializeToolkit()
{
  int32_t lRet = CIFX_NO_ERROR;

  lRet = cifXTKitInit();

  if(CIFX_NO_ERROR == lRet)
  {
    PDEVICEINSTANCE ptDevInstance = (PDEVICEINSTANCE) OS_Memalloc(sizeof(*ptDevInstance));
    OS_Memset(ptDevInstance, 0, sizeof(*ptDevInstance));

    /** Set trace level of toolkit */
    g_ulTraceLevel = TRACE_LEVEL_ERROR |
    TRACE_LEVEL_WARNING |
    TRACE_LEVEL_INFO |
    TRACE_LEVEL_DEBUG;

    /** Insert the basic device information into the DeviceInstance structure
     for the toolkit.
     NOTE: The irq number are for information use only, so we skip them here.
     Interrupt is currently not supported and ignored, so we don't need to set it */
    ptDevInstance->fPCICard = 0;
    ptDevInstance->pvOSDependent = ptDevInstance;
    ptDevInstance->ulDPMSize = 0x8000;           // 32K
    OS_Strncpy(ptDevInstance->szName, "cifX0", sizeof(ptDevInstance->szName));

    /** we know here that netX firmware is flash based, therefore we check if it starts up
     ** by comparing cookie at DPM address 0x00 is valid.*/

    do
    {
      SerialDPM_Init(ptDevInstance);
    } while(false == isCookieAvailable(ptDevInstance, 100));

    /* Add the device to the toolkits handled device list */
    lRet = cifXTKitAddDevice(ptDevInstance);

    /** If it succeeded do device tests */
    if(CIFX_NO_ERROR != lRet)
    {
      /** Uninitialize Toolkit, this will remove all handled boards from the toolkit and
       deallocate the device instance */
      cifXTKitDeinit();
    }
  }

  return lRet;
}

/* ========== VAT TEST INTEGRATION - 함수 구현 시작 ========== */

/**
 * @brief DeviceNet Stack 상태 출력 (디버깅용)
 *
 * @param ptAppData  Application data structure
 *
 * Stack 초기화 후 상태를 확인하여 Explicit 메시징 준비 상태를 점검
 */
static void PrintDeviceNetStatus(APP_DATA_T* ptAppData)
{
    CHANNEL_INFORMATION tInfo;
    int32_t lRet;

    // Get channel information
    lRet = xChannelInfo(ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->hChannel,
                        sizeof(CHANNEL_INFORMATION),
                        &tInfo);

    if (lRet != CIFX_NO_ERROR) {
        printf("[ERROR] Failed to get channel info: 0x%08X\r\n", (unsigned int)lRet);
        return;
    }

    // Print status banner
    printf("\r\n");
    printf("=============================================\r\n");
    printf("  DeviceNet Stack Status\r\n");
    printf("=============================================\r\n");

    // Channel READY status
    printf("Channel READY:     %s\r\n",
        (tInfo.ulDeviceCOSFlags & HIL_COMM_COS_READY) ? "[OK] YES" : "[FAIL] NO");

    // BUS ON status
    printf("BUS ON:            %s\r\n",
        (tInfo.ulDeviceCOSFlags & HIL_COMM_COS_BUS_ON) ? "[OK] YES" : "[FAIL] NO");

    // Communication state
    printf("Comm State:        %s\r\n",
        (tInfo.ulDeviceCOSFlags & HIL_COMM_COS_RUN) ? "[OK] RUN" : "[WARN] STOP");

    // Node configuration
    printf("\r\n--- Configuration ---\r\n");
    printf("Node ID:           %d\r\n", g_tAppDnsData.bNodeIdValue);
    printf("Baud Rate:         ");
    switch (g_tAppDnsData.bBaudRateValue) {
        case 0: printf("125 kB/s\r\n"); break;
        case 1: printf("250 kB/s\r\n"); break;
        case 2: printf("500 kB/s\r\n"); break;
        default: printf("Unknown (%d)\r\n", g_tAppDnsData.bBaudRateValue); break;
    }

    // Device information
    printf("\r\n--- Device Info ---\r\n");
    printf("Device Number:     %u\r\n", (unsigned int)tInfo.ulDeviceNumber);
    printf("Serial Number:     %u\r\n", (unsigned int)tInfo.ulSerialNumber);
    printf("Firmware:          %u.%u.%u build %u\r\n",
        (unsigned int)tInfo.usFWMajor,
        (unsigned int)tInfo.usFWMinor,
        (unsigned int)tInfo.usFWRevision,
        (unsigned int)tInfo.usFWBuild);

    // Error information
    printf("\r\n--- Error Status ---\r\n");
    if (tInfo.ulDeviceCOSFlags & HIL_COMM_COS_CONFIG_LOCKED) {
        printf("Config Locked:     [WARN] YES (cannot reconfigure)\r\n");
    } else {
        printf("Config Locked:     [OK] NO\r\n");
    }

    printf("=============================================\r\n\r\n");

    // Explicit message readiness check
    if ((tInfo.ulDeviceCOSFlags & HIL_COMM_COS_READY) &&
        (tInfo.ulDeviceCOSFlags & HIL_COMM_COS_BUS_ON)) {
        printf("✅ Stack is READY for Explicit Messaging\r\n\r\n");
    } else {
        printf("❌ Stack is NOT READY for Explicit Messaging\r\n");
        printf("   Please check Channel READY and BUS ON status\r\n\r\n");
    }
}

/**
 * @brief 디바이스 정보 출력
 *
 * @param hDriver   cifX 드라이버 핸들
 * @param hChannel  cifX 채널 핸들
 *
 * 드라이버, 보드, 채널 정보를 조회하여 출력
 */
static void PrintDeviceInformation(CIFXHANDLE hDriver, CIFXHANDLE hChannel)
{
    int32_t lRet = CIFX_NO_ERROR;

    printf("\r\n");
    printf("========================================\r\n");
    printf(" Device Information\r\n");
    printf("========================================\r\n");

    /* 1. 드라이버 정보 */
    DRIVER_INFORMATION tDriverInfo;
    lRet = xDriverGetInformation(hDriver, sizeof(DRIVER_INFORMATION), &tDriverInfo);
    if(lRet == CIFX_NO_ERROR)
    {
        printf("\r\n[Driver Information]\r\n");
        printf("  Driver Version:  %s\r\n", tDriverInfo.abDriverVersion);
        printf("  Board Count:     %lu\r\n", tDriverInfo.ulBoardCnt);
    }
    else
    {
        printf("\r\n[Driver Information] ERROR: 0x%08X\r\n", (unsigned int)lRet);
    }

    /* 2. 보드 정보 */
    BOARD_INFORMATION tBoardInfo;
    lRet = xDriverEnumBoards(hDriver, 0, sizeof(BOARD_INFORMATION), &tBoardInfo);
    if(lRet == CIFX_NO_ERROR)
    {
        printf("\r\n[Board Information]\r\n");
        printf("  Board Name:      %s\r\n", tBoardInfo.abBoardName);
        printf("  Board Alias:     %s\r\n", tBoardInfo.abBoardAlias);
        printf("  Board ID:        0x%08lX\r\n", tBoardInfo.ulBoardID);
        printf("  Device Number:   %lu\r\n", tBoardInfo.tSystemInfo.ulDeviceNumber);
        printf("  Serial Number:   %lu\r\n", tBoardInfo.tSystemInfo.ulSerialNumber);
        printf("  DPM Size:        %lu bytes\r\n", tBoardInfo.ulDpmTotalSize);
        printf("  Channel Count:   %lu\r\n", tBoardInfo.ulChannelCnt);
        printf("  HW Revision:     %u\r\n", tBoardInfo.tSystemInfo.bHwRevision);
        printf("  Device Class:    0x%04X\r\n", tBoardInfo.tSystemInfo.usDeviceClass);
    }
    else
    {
        printf("\r\n[Board Information] ERROR: 0x%08X\r\n", (unsigned int)lRet);
    }

    /* 3. 채널 정보 */
    CHANNEL_INFORMATION tChannelInfo;
    lRet = xChannelInfo(hChannel, sizeof(CHANNEL_INFORMATION), &tChannelInfo);
    if(lRet == CIFX_NO_ERROR)
    {
        printf("\r\n[Channel Information]\r\n");
        printf("  Board Name:      %s\r\n", tChannelInfo.abBoardName);
        printf("  Device Number:   %lu\r\n", tChannelInfo.ulDeviceNumber);
        printf("  Serial Number:   %lu\r\n", tChannelInfo.ulSerialNumber);

        printf("\r\n[Firmware Information]\r\n");
        printf("  Firmware Name:   %.*s\r\n",
               tChannelInfo.bFWNameLength,
               tChannelInfo.abFWName);
        printf("  FW Version:      %u.%u.%u.%u\r\n",
               tChannelInfo.usFWMajor,
               tChannelInfo.usFWMinor,
               tChannelInfo.usFWBuild,
               tChannelInfo.usFWRevision);
        printf("  FW Build Date:   %04u-%02u-%02u\r\n",
               tChannelInfo.usFWYear,
               tChannelInfo.bFWMonth,
               tChannelInfo.bFWDay);

        printf("\r\n[Channel Status]\r\n");
        printf("  Open Count:      %lu\r\n", tChannelInfo.ulOpenCnt);
        printf("  Mailbox Size:    %lu bytes\r\n", tChannelInfo.ulMailboxSize);
        printf("  IO Input Areas:  %lu\r\n", tChannelInfo.ulIOInAreaCnt);
        printf("  IO Output Areas: %lu\r\n", tChannelInfo.ulIOOutAreaCnt);
        printf("  netX Flags:      0x%08lX\r\n", tChannelInfo.ulNetxFlags);
        printf("  Host Flags:      0x%08lX\r\n", tChannelInfo.ulHostFlags);
        printf("  Device COS:      0x%08lX\r\n", tChannelInfo.ulDeviceCOSFlags);
    }
    else
    {
        printf("\r\n[Channel Information] ERROR: 0x%08X\r\n", (unsigned int)lRet);
    }

    printf("========================================\r\n\r\n");

    /* 4. DeviceNet Master Identity Information */
    printf("========================================\r\n");
    printf(" DeviceNet Master Identity\r\n");
    printf("========================================\r\n");

    DEVICENET_MASTER_IDENTITY_T tMasterIdentity;
    uint8_t masterNodeId = 0;  /* Try common master address first */

    /* Try to scan for master on network */
    lRet = DeviceNet_ScanForMaster(hChannel, &masterNodeId);

    if(lRet == CIFX_NO_ERROR)
    {
        /* Print detailed master information */
        DeviceNet_PrintMasterIdentity(&tMasterIdentity);
    }
    else
    {
        /* Scanning failed, try direct query at node 0 */
        printf("Scanning failed, trying direct query at Node 0...\r\n");
        lRet = DeviceNet_GetMasterIdentity(hChannel, 0, &tMasterIdentity);

        if(lRet == CIFX_NO_ERROR && tMasterIdentity.bDataValid)
        {
            DeviceNet_PrintMasterIdentity(&tMasterIdentity);
        }
        else
        {
            printf("ERROR: Could not retrieve Master identity\r\n");
            printf("Please verify:\r\n");
            printf("  1. DeviceNet Master is powered on\r\n");
            printf("  2. Network cables are connected\r\n");
            printf("  3. Network termination is correct\r\n");
            printf("  4. Master node address configuration\r\n");
        }
    }

    printf("========================================\r\n\r\n");
}

/**
 * @brief VAT 테스트 초기화
 *
 * VAT Adaptive Pressure Controller 테스트를 위한 초기 설정
 */
static void VAT_InitializeTest(void)
{
    printf("\r\n========================================\r\n");
    printf(" VAT DeviceNet Test Initialization\r\n");
    printf("========================================\r\n");

    /* VAT Diagnostic 초기화 */
    VAT_Diagnostic_Init();

    /* 테스트 컨텍스트 초기화 */
    VAT_Test_Init(&g_tVatContext);

    /* 테스트 설정 */
    VAT_TEST_CONFIG_T tConfig = {
        .node_address = 10,                             /* DeviceNet 노드 주소 */
        .baud_rate = 125000,                            /* 250 kbps */
        .epr_ms = 100,                                  /* 100ms 패킷 주기 */
        .input_assembly = VAT_INPUT_ASSEMBLY_100,       /* 7바이트 전체 상태 */
        .output_assembly = VAT_OUTPUT_ASSEMBLY_8,       /* 5바이트 제어+모드 */
        .enable_logging = true,                         /* 로깅 활성화 */
        .enable_validation = true                       /* 검증 활성화 */
    };

    VAT_Test_Configure(&g_tVatContext, &tConfig);

    printf("VAT Test Configuration:\r\n");
    printf("  Mode: %u (1=Basic, 2=Calibration, 3=Monitoring)\r\n", g_VatTestMode);
    printf("  Node Address: %u\r\n", tConfig.node_address);
    printf("  Baud Rate: %lu bps\r\n", tConfig.baud_rate);
    printf("  Input Assembly: 0x%02X (%u bytes)\r\n",
           tConfig.input_assembly,
           sizeof(VAT_INPUT_ASSEMBLY_100_T));
    printf("  Output Assembly: 0x%02X (%u bytes)\r\n",
           tConfig.output_assembly,
           sizeof(VAT_OUTPUT_ASSEMBLY_8_T));
    printf("========================================\r\n\r\n");
}

/**
 * @brief VAT 테스트 실행
 *
 * @param hChannel  cifX 채널 핸들
 *
 * g_VatTestMode 값에 따라 다른 테스트 시나리오 실행:
 * - 1: Basic Pressure Control (10 cycles)
 * - 2: Full Calibration (5 cycles with auto-learn)
 * - 3: Continuous Monitoring (100 cycles)
 */
static void VAT_RunTest(CIFXHANDLE hChannel)
{
    printf("\r\n========================================\r\n");
    printf(" Running VAT Test Mode: %u\r\n", g_VatTestMode);
    printf("========================================\r\n\r\n");

    switch(g_VatTestMode)
    {
        case 1:  /* Basic Pressure Control Test */
        {
            printf("Test: Basic Pressure Control\r\n");
            printf("Cycles: 10\r\n");
            printf("Setpoint: 500\r\n\r\n");

            VAT_Test_BasicPressureControl(&g_tVatContext, hChannel);
            break;
        }

        case 2:  /* Full Calibration Test */
        {
            printf("Test: Full Calibration with Auto-Learn\r\n");
            printf("Cycles: 5\r\n\r\n");

            VAT_Test_FullCalibration(&g_tVatContext, hChannel);
            break;
        }

        case 3:  /* Continuous Monitoring */
        {
            // printf("Test: Continuous Monitoring\r\n");
            // printf("Cycles: 100 (10 seconds @ 100ms)\r\n");
            // printf("Setpoint: 750\r\n\r\n");

            printf("Test: INFINITE Continuous Monitoring\r\n");
            printf("Cycles: INFINITE (until reset)\r\n");
            printf("Update Rate: 100ms\r\n");
            printf("Press RESET button to stop.\r\n\r\n");

            /* 압력 제어 모드 설정 */
            VAT_Test_SetControlMode(&g_tVatContext, VAT_CONTROL_MODE_PRESSURE);
            // VAT_Test_SetPressureSetpoint(&g_tVatContext, 750);

           /* 초기 Setpoint */
            int16_t setpoint = 500;
            VAT_Test_SetPressureSetpoint(&g_tVatContext, setpoint);



            
            /* 무한 연속 모니터링 루프 */
            uint32_t cycle_count = 0;

while(1)
{
    /* Setpoint 증가 */
    setpoint++;
    if(setpoint > 1000) setpoint = 0;

    /* ✅ g_tAppData에 직접 데이터 설정 (5 bytes, Master → Slave) */
    g_tAppData.tInputData.input[0] = 2;  // Control Mode = Pressure
    g_tAppData.tInputData.input[1] = (uint8_t)(setpoint & 0xFF);
    g_tAppData.tInputData.input[2] = (uint8_t)((setpoint >> 8) & 0xFF);
    g_tAppData.tInputData.input[3] = 0;  // Control Instance
    g_tAppData.tInputData.input[4] = 0;  // Reserved

    /* ✅ 출력 데이터 증가 (7 bytes, Slave → Master) */
    for(int i = 0; i < 7; i++) {
        g_tAppData.tOutputData.output[i]++;
    }

    /* ✅ 실제 DPM에 쓰기 */
    int32_t lRet = xChannelIOWrite(hChannel, 0, 0,
                                    sizeof(g_tAppData.tOutputData),
                                    &g_tAppData.tOutputData, 0);
    if(lRet != CIFX_NO_ERROR)
    {
        printf("[%lu] Write Error: 0x%08X\r\n", cycle_count, (unsigned int)lRet);
    }

    /* ✅ 실제 DPM에서 읽기 */
    lRet = xChannelIORead(hChannel, 0, 0,
                          sizeof(g_tAppData.tInputData),
                          &g_tAppData.tInputData, 0);
    if(lRet != CIFX_NO_ERROR)
    {
        printf("[%lu] Read Error: 0x%08X\r\n", cycle_count, (unsigned int)lRet);
    }

    /* ✅ VAT 진단 업데이트 (100ms마다) */
    VAT_Diagnostic_Update();

    /* ✅ Parameters ↔ I/O Data 동기화 */
    VAT_Sync_IoDataToParameters();  /* I/O → Parameters */
    VAT_Sync_ParametersToIoData();  /* Parameters → I/O */

    /* 매 10회마다 상태 출력 */
    if((cycle_count % 10) == 0)
    {
        printf("[%lu] Setpoint=%d, Output=[%02X %02X %02X %02X %02X %02X %02X]\r\n",
               cycle_count,
               setpoint,
               g_tAppData.tOutputData.output[0],
               g_tAppData.tOutputData.output[1],
               g_tAppData.tOutputData.output[2],
               g_tAppData.tOutputData.output[3],
               g_tAppData.tOutputData.output[4],
               g_tAppData.tOutputData.output[5],
               g_tAppData.tOutputData.output[6]);
    }

    HAL_Delay(100);
    cycle_count++;
}

            // while(1)
            // {
            //     /* Setpoint 증가 */
            //     setpoint++;
            //     if(setpoint > 1000) setpoint = 0;
            //     VAT_Test_SetPressureSetpoint(&g_tVatContext, setpoint);

            //     /* 제어 데이터 전송 */
            //     int32_t lRet = VAT_Test_WriteOutput(&g_tVatContext, hChannel);
            //     if(lRet != CIFX_NO_ERROR)
            //     {
            //         printf("[%lu] Write Error: 0x%08X\r\n", cycle_count, (unsigned int)lRet);
            //     }

            //     /* 센서 데이터 읽기 */
            //     lRet = VAT_Test_ReadInput(&g_tVatContext, hChannel);
            //     if(lRet != CIFX_NO_ERROR)
            //     {
            //         printf("[%lu] Read Error: 0x%08X\r\n", cycle_count, (unsigned int)lRet);
            //     }

            //     /* 예외 상태 확인 */
            //     if(VAT_Test_HasException(&g_tVatContext))
            //     {
            //         printf("[%lu] WARNING: Exception 0x%02X\r\n",
            //                cycle_count, VAT_Test_GetExceptionStatus(&g_tVatContext));
            //     }

            //     /* 매 10회마다 상태 출력 */
            //     if((cycle_count % 10) == 0 && g_tVatContext.input_data_valid)
            //     {
            //         printf("[%lu] Setpoint=%d, Pressure=%d, Position=%d, Status=0x%02X\r\n",
            //                cycle_count,
            //                setpoint,
            //                g_tVatContext.input_asm100.pressure,
            //                g_tVatContext.input_asm100.position,
            //                g_tVatContext.input_asm100.device_status);
            //     }

            //     /* 100ms 대기 */
            //     HAL_Delay(100);

            //     cycle_count++;

            //     /* 1000회마다 진행 상황 출력 */
            //     if((cycle_count % 1000) == 0)
            //     {
            //         printf("\r\n*** Running for %lu cycles (%.1f minutes) ***\r\n",
            //                cycle_count, (float)cycle_count / 600.0f);
            //     }
            // }


     /*
  
            for(int i = 0; i < 100; i++)
            {
                
                int32_t lRet = VAT_Test_WriteOutput(&g_tVatContext, hChannel);
                if(lRet != CIFX_NO_ERROR)
                {
                    printf("[%03d] Write Error: 0x%08X\r\n", i, (unsigned int)lRet);
                }

                
                lRet = VAT_Test_ReadInput(&g_tVatContext, hChannel);
                if(lRet != CIFX_NO_ERROR)
                {
                    printf("[%03d] Read Error: 0x%08X\r\n", i, (unsigned int)lRet);
                }

                 예외 상태 확인 
                if(VAT_Test_HasException(&g_tVatContext))
                {
                    printf("[%03d] WARNING: Exception 0x%02X\r\n",
                           i, VAT_Test_GetExceptionStatus(&g_tVatContext));
                }

                // 매 10회마다 상태 출력 
                if((i % 10) == 0 && g_tVatContext.input_data_valid)
                {
                    printf("[%03d] Pressure=%d, Position=%d, Status=0x%02X\r\n",
                           i,
                           g_tVatContext.input_asm100.pressure,
                           g_tVatContext.input_asm100.position,
                           g_tVatContext.input_asm100.device_status);
                }

                // 100ms 대기 
                HAL_Delay(100);
            }

            printf("\r\nContinuous monitoring completed.\r\n");
            break;
        } */
          }

case 4:  /* VAT Real Data Simulation - INFINITE */
{
    printf("Test: VAT Real Data Simulation (INFINITE)\r\n");
    printf("Simulating VAT Adaptive Pressure Controller\r\n");
    printf("Press RESET to stop.\r\n\r\n");

    /* 시뮬레이션 상태 초기화 */
    memset(&g_tVatSimState, 0, sizeof(g_tVatSimState));
    g_tVatSimState.current_pressure = 0;
    g_tVatSimState.current_position = 0;
    g_tVatSimState.device_status = 0x02;  // VALVE_CLOSED
    g_tVatSimState.exception_status = 0x00;

    uint32_t cycle_count = 0;

    /* 무한 제어 루프 */
    while(1)
    {
        /* === Master → Slave: 제어 명령 읽기 === */
        int32_t lRet = xChannelIORead(hChannel, 0, 0,
                                       sizeof(g_tAppData.tInputData),
                                       &g_tAppData.tInputData, 0);
        if(lRet != CIFX_NO_ERROR) {
            printf("[%lu] Read Error: 0x%08X\r\n", cycle_count, (unsigned int)lRet);
        }

        /* Master로부터 받은 제어 명령 파싱 */
        uint8_t control_mode = g_tAppData.tInputData.input[0];
        int16_t control_setpoint = (int16_t)(g_tAppData.tInputData.input[1] |
                                             (g_tAppData.tInputData.input[2] << 8));

        /* === 제어 모드에 따른 시뮬레이션 === */
        switch(control_mode)
        {
            case 0x00:  /* CLOSED */
                if(g_tVatSimState.current_position > 0) {
                    g_tVatSimState.current_position -= 10;
                    if(g_tVatSimState.current_position < 0)
                        g_tVatSimState.current_position = 0;
                    g_tVatSimState.device_status |= 0x04;  // MOVING
                } else {
                    g_tVatSimState.device_status &= ~0x04;
                    g_tVatSimState.device_status |= 0x02;  // VALVE_CLOSED
                }
                g_tVatSimState.current_pressure = 869;
                break;

            case 0x01:  /* OPEN */
                if(g_tVatSimState.current_position < 1000) {
                    g_tVatSimState.current_position += 10;
                    if(g_tVatSimState.current_position > 1000)
                        g_tVatSimState.current_position = 1000;
                    g_tVatSimState.device_status |= 0x04;  // MOVING
                } else {
                    g_tVatSimState.device_status &= ~0x04;
                    g_tVatSimState.device_status |= 0x01;  // VALVE_OPEN
                }
                g_tVatSimState.current_pressure = 10;
                break;

            case 0x02:  /* PRESSURE */
            {
                int16_t pressure_error = control_setpoint - g_tVatSimState.current_pressure;

                if(abs(pressure_error) <= 10) {
                    /* 설정값 도달 */
                    g_tVatSimState.device_status |= 0x08;   // AT_SETPOINT
                    g_tVatSimState.device_status &= ~0x04;  // MOVING 해제
                } else {
                    /* 압력 조정 중 */
                    g_tVatSimState.device_status |= 0x04;   // MOVING
                    g_tVatSimState.device_status &= ~0x08;  // AT_SETPOINT 해제

                    /* 밸브 위치 조정 */
                    if(pressure_error > 0) {
                        g_tVatSimState.current_position += 5;
                        if(g_tVatSimState.current_position > 1000)
                            g_tVatSimState.current_position = 1000;
                    } else {
                        g_tVatSimState.current_position -= 5;
                        if(g_tVatSimState.current_position < 0)
                            g_tVatSimState.current_position = 0;
                    }

                    /* 압력 업데이트 (간단한 모델) */
                    g_tVatSimState.current_pressure =
                        (g_tVatSimState.current_position * control_setpoint) / 1000;
                }
                break;
            }

            case 0x03:  /* POSITION */
            {
                int16_t position_error = control_setpoint - g_tVatSimState.current_position;

                if(abs(position_error) <= 5) {
                    g_tVatSimState.device_status |= 0x08;
                    g_tVatSimState.device_status &= ~0x04;
                } else {
                    g_tVatSimState.device_status |= 0x04;
                    g_tVatSimState.device_status &= ~0x08;

                    if(position_error > 0) {
                        g_tVatSimState.current_position += 10;
                        if(g_tVatSimState.current_position > 1000)
                            g_tVatSimState.current_position = 1000;
                    } else {
                        g_tVatSimState.current_position -= 10;
                        if(g_tVatSimState.current_position < 0)
                            g_tVatSimState.current_position = 0;
                    }
                }

                /* 위치에 따른 압력 (최대 1000 mbar) */
                // g_tVatSimState.current_pressure = g_tVatSimState.current_position;
                break;
            }

            default:
                /* 알 수 없는 모드 - 안전을 위해 닫힘 */
                g_tVatSimState.current_position = 0;
                g_tVatSimState.current_pressure = 0;
                g_tVatSimState.device_status = 0x02;  // VALVE_CLOSED
                break;
        }

        /* 밸브 완전 열림/닫힘 플래그 갱신 */
        if(g_tVatSimState.current_position >= 1000) {
            g_tVatSimState.device_status |= 0x01;   // VALVE_OPEN
            g_tVatSimState.device_status &= ~0x02;
        } else if(g_tVatSimState.current_position <= 0) {
            g_tVatSimState.device_status |= 0x02;   // VALVE_CLOSED
            g_tVatSimState.device_status &= ~0x01;
        } else {
            g_tVatSimState.device_status &= ~0x03;  // 중간 상태
        }

        /* === Slave → Master: 센서 데이터 전송 === */
        g_tAppData.tOutputData.output[0] = g_tVatSimState.exception_status;
        g_tAppData.tOutputData.output[1] = (uint8_t)(g_tVatSimState.current_pressure & 0xFF);
        g_tAppData.tOutputData.output[2] = (uint8_t)((g_tVatSimState.current_pressure >> 8) & 0xFF);
        g_tAppData.tOutputData.output[3] = (uint8_t)(g_tVatSimState.current_position & 0xFF);
        g_tAppData.tOutputData.output[4] = (uint8_t)((g_tVatSimState.current_position >> 8) & 0xFF);
        g_tAppData.tOutputData.output[5] = g_tVatSimState.device_status;
        g_tAppData.tOutputData.output[6] = 0x00;  // access_mode (reserved)

        /* DPM에 쓰기 */
        lRet = xChannelIOWrite(hChannel, 0, 0,
                               sizeof(g_tAppData.tOutputData),
                               &g_tAppData.tOutputData, 0);
        if(lRet != CIFX_NO_ERROR) {
            printf("[%lu] Write Error: 0x%08X\r\n", cycle_count, (unsigned int)lRet);
        }

        /* 매 10회마다 상태 출력 */
        if((cycle_count % 10) == 0) {
            printf("[%lu] Mode=%d, Setpoint=%d, Pressure=%d, Position=%d, Status=0x%02X\r\n",
                   cycle_count,
                   control_mode,
                   control_setpoint,
                   g_tVatSimState.current_pressure,
                   g_tVatSimState.current_position,
                   g_tVatSimState.device_status);
        }

        /* 100ms 대기 */
        HAL_Delay(100);
        cycle_count++;

        /* 1000회마다 진행 상황 */
        if((cycle_count % 1000) == 0) {
            printf("\r\n*** VAT Simulation running for %lu cycles (%.1f min) ***\r\n",
                   cycle_count, (float)cycle_count / 600.0f);
        }
    }

    break;
}

        default:
        {
            printf("ERROR: Invalid test mode %u\r\n", g_VatTestMode);
            printf("Valid modes: 1 (Basic), 2 (Calibration), 3 (Monitoring)\r\n");
            break;
        }
    }

    printf("\r\n========================================\r\n");
    printf(" VAT Test Completed\r\n");
    printf("========================================\r\n");

    /* 최종 통계 출력 */
    VAT_Test_PrintStats(&g_tVatContext);
}

/* ========== VAT TEST INTEGRATION - 함수 구현 끝 ========== */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	 int32_t lRet=CIFX_NO_ERROR;    /* Return value for common error codes      */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_USB_HOST_Init();
  MX_TIM3_Init();
  MX_SPI4_Init();
  MX_UART5_Init();
  /* USER CODE BEGIN 2 */


  UART_Ring_Init(&huart5);  // 인터럽트 수신 초기화

//  for (int i = 0; i<10; i++){
//	  HAL_UART_Transmit(&huart1, (uint8_t *)"RX: ", 4, HAL_MAX_DELAY);
//	  HAL_Delay(1000);
//
//  }

//  HAL_UART_Transmit(&huart1, (uint8_t *)"RX: ", 4, HAL_MAX_DELAY);
//            HAL_UART_Transmit(&huart1, line, len, HAL_MAX_DELAY);
//            HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);

  /* UART printf 테스트 */
  printf("\r\n\r\n");
  printf("========================================\r\n");
  printf(" STM32F429 + netX90 System Starting\r\n");
  printf("========================================\r\n");
  printf("UART5: 115200 baud, 8N1\r\n");
  printf("Date: 2025-10-28\r\n");
  printf("========================================\r\n\r\n");

  // D switch validation loop - simple single reception
  // HAL_UART_Transmit(&huart5, (uint8_t *)"Starting D validation loop (waiting for D:[xxxx] + 0x0D)...\r\n", 62, HAL_MAX_DELAY);

  /*
  // 수정된 부분: 복잡한 누적 로직을 제거하고, 라인 단위로 수신
  while (!g_bDipDataValid)
  {
      int len = UART_Ring_ReadLine(line, sizeof(line));

      if (len > 0)
      {
          // A full line was received. Echo it back.
          HAL_UART_Transmit(&huart5, (uint8_t *)"RX: ", 4, HAL_MAX_DELAY);
          HAL_UART_Transmit(&huart5, line, len, HAL_MAX_DELAY);
          HAL_UART_Transmit(&huart5, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);

          // Now validate the received line
          if (ValidateDipSwitchData(line) == 1)
          {
              HAL_UART_Transmit(&huart5, (uint8_t *)"D data validated!\r\n", 19, HAL_MAX_DELAY);
              break; // Exit validation loop
          }
          else
          {
              // Validation failed, continue waiting for a new line
              HAL_UART_Transmit(&huart5, (uint8_t *)"Validation FAILED, waiting for new data.\r\n", 42, HAL_MAX_DELAY);
          }
      }
      
      HAL_Delay(100); // Short delay to prevent busy-waiting
  }
  */
  
  lRet = InitializeToolkit();

  if(CIFX_NO_ERROR == lRet)
  {
      /* ========== VAT TEST INTEGRATION - 메인 로직 수정 시작 ========== */

      if(g_bEnableVatTest)
      {
          /* VAT 테스트 모드 */
          CIFXHANDLE hDriver = NULL;
          CIFXHANDLE hChannel = NULL;

          /* VAT 테스트 초기화 */
          VAT_InitializeTest();

          /* cifX 드라이버 열기 */
          lRet = xDriverOpen(&hDriver);
          if(CIFX_NO_ERROR == lRet)
          {
              /* 채널 0 열기 */
              lRet = xChannelOpen(hDriver, "cifX0", 0, &hChannel);
              if(CIFX_NO_ERROR == lRet)
              {
                  /* ========== DeviceNet 스택 초기화 시작 ========== */
                  printf("\r\n");
                  printf("========================================\r\n");
                  printf(" DeviceNet Stack Initialization\r\n");
                  printf("========================================\r\n");

                  /* g_tAppData 채널 0 설정 */
                  if(g_tAppData.aptChannels[0] == NULL)
                  {
                      g_tAppData.aptChannels[0] = (APP_COMM_CHANNEL_T*)calloc(1, sizeof(APP_COMM_CHANNEL_T));
                      if(g_tAppData.aptChannels[0] == NULL)
                      {
                          printf("ERROR: Failed to allocate memory for channel 0\r\n");
                          lRet = CIFX_DRV_INIT_ERROR;
                      }
                  }

                  if(lRet == CIFX_NO_ERROR)
                  {
                      /* 채널 핸들 및 프로토콜 핸들러 설정 */
                      g_tAppData.aptChannels[0]->hChannel = hChannel;
                      g_tAppData.aptChannels[0]->tProtocol = g_tRealtimeProtocolHandlers;

                      /* ========== CRITICAL: Wait for channel ready BEFORE configuration! ========== */
                      printf("\r\nStep 1: Waiting for channel to be ready...\r\n");
                      CHANNEL_INFORMATION tChannelInfo;
                      uint32_t timeout_ms = 0;
                      const uint32_t MAX_WAIT_MS = 5000;

                      do {
                          xChannelInfo(hChannel, sizeof(CHANNEL_INFORMATION), &tChannelInfo);

                          if(tChannelInfo.ulDeviceCOSFlags & HIL_COMM_COS_READY)
                              break;

                          HAL_Delay(100);
                          timeout_ms += 100;

                          if((timeout_ms % 1000) == 0) {
                              printf("  [%lu s] COS Flags: 0x%08X\r\n",
                                     timeout_ms / 1000,
                                     (unsigned int)tChannelInfo.ulDeviceCOSFlags);
                          }

                          if(timeout_ms >= MAX_WAIT_MS) {
                              printf("ERROR: Channel ready timeout!\r\n");
                              lRet = CIFX_DEV_NOT_READY;
                              break;
                          }
                      } while(1);

                      if(lRet == CIFX_NO_ERROR) {
                          printf("  Channel ready! COS Flags: 0x%08X\r\n",
                                 (unsigned int)tChannelInfo.ulDeviceCOSFlags);

                          /* Set Host State to READY */
                          printf("\r\nStep 2: Setting Host State to READY...\r\n");
                          uint32_t ulState = 0;
                          lRet = xChannelHostState(hChannel, CIFX_HOST_STATE_READY, &ulState, 1000);
                          if(lRet == CIFX_NO_ERROR) {
                              printf("  Host State set to READY (State: 0x%08X)\r\n", (unsigned int)ulState);
                          } else {
                              printf("ERROR: xChannelHostState failed: 0x%08X\r\n", (unsigned int)lRet);
                          }
                      }

                      if(lRet == CIFX_NO_ERROR)
                      {
                          printf("\r\nStep 3: Calling Protocol_StartConfiguration...\r\n");

                          /* Protocol Start Configuration (includes Pkt_Init and RegisterIndicationHandler) */
                          if(g_tAppData.aptChannels[0]->tProtocol.pfStartChannelConfiguration != NULL)
                          {
                              lRet = g_tAppData.aptChannels[0]->tProtocol.pfStartChannelConfiguration(&g_tAppData);
                              if(lRet != CIFX_NO_ERROR)
                              {
                                  printf("ERROR: Protocol_StartConfiguration failed: 0x%08X\r\n", (unsigned int)lRet);
                              }
                          }
                          else
                          {
                              printf("ERROR: pfStartChannelConfiguration is NULL!\r\n");
                              lRet = CIFX_DRV_INIT_ERROR;
                          }
                      }

                      if(lRet == CIFX_NO_ERROR)
                      {
                          printf("  [OK] Protocol_StartConfiguration completed\r\n");
                          printf("    - Pkt_Init() done\r\n");
                          printf("    - Pkt_RegisterIndicationHandler() done\r\n");
                          printf("    - Application registered (if DNS_HOST_APP_REGISTRATION defined)\r\n");
                          printf("    - Stack configuration (SetConfig, ChannelInit, StartComm) done\r\n");

                          /* Initialize VAT Parameter Manager */
                          printf("\r\nStep 4: Initializing VAT Parameters...\r\n");
                          VAT_Param_Init(&g_tParamManager);
                          printf("  VAT Parameter Manager initialized (%d parameters)\r\n",
                                 g_tParamManager.param_count);

                          printf("\r\n========================================\r\n");
                          printf("DeviceNet Stack initialized successfully!\r\n");
                          printf("Waiting for Explicit Messages from master...\r\n");
                          printf("\r\nNOTE: CIP Service Indications will be logged below\r\n");
                          printf("========================================\r\n\r\n");

                          /* ⭐ Print stack status for debugging Explicit messaging */
                          PrintDeviceNetStatus(&g_tAppData);
                      }
                      else
                      {
                          printf("Step 1: ❌ AppDNS_ConfigureStack() FAILED\r\n");
                          printf("  Error Code: 0x%08X\r\n", (unsigned int)lRet);
                          printf("\r\nDeviceNet Stack initialization failed!\r\n");
                          printf("Master scan will NOT detect this slave.\r\n");
                          printf("========================================\r\n\r\n");
                      }
                  }
                  /* ========== DeviceNet 스택 초기화 끝 ========== */

                  /* Continue with testing if initialization succeeded */
                  if(lRet == CIFX_NO_ERROR)
                  {
                      /* 디바이스 정보 출력 */
                      PrintDeviceInformation(hDriver, hChannel);

                      /* ========== Main Communication Loop ========== */
                      printf("\r\n========================================\r\n");
                      printf(" Entering Main Communication Loop\r\n");
                      printf(" - Processing Explicit Messages\r\n");
                      printf(" - Handling IO Data\r\n");
                      printf(" Press RESET to stop\r\n");
                      printf("========================================\r\n\r\n");

                      uint32_t loop_count = 0;
                      while(1)
                      {
                          /* Process incoming packets (Explicit Messages) - CRITICAL! */
                          lRet = App_AllChannels_PacketHandler(&g_tAppData);
                          if(lRet != CIFX_NO_ERROR)
                          {
                              printf("ERROR: PacketHandler failed: 0x%08X\r\n", (unsigned int)lRet);
                          }

                          /* TODO: Process IO Data here if needed */
                          /* For now, just focus on Explicit Messages */

                          /* Periodic status output every 10 seconds */
                          if((loop_count % 1000) == 0)
                          {
                              printf("[%lu] Communication active, waiting for Explicit Messages...\r\n",
                                     loop_count / 100);
                          }

                          HAL_Delay(10);  /* 10ms cycle time */
                          loop_count++;
                      }
                  }
                  else
                  {
                      printf("\r\n*** Skipping communication loop due to initialization failure ***\r\n");
                  }

                  /* ✅ 수정: 채널은 닫지 않고 유지 - 계속 통신 가능하도록 */
                  printf("\r\n*** VAT Test completed - keeping channel open for continuous operation ***\r\n");
                  // xChannelClose(hChannel);  /* 주석 처리: 통신 유지 */
              }
              else
              {
                  printf("ERROR: xChannelOpen failed: 0x%08X\r\n", (unsigned int)lRet);
              }

              /* ✅ 수정: 드라이버도 닫지 않고 유지 - 계속 통신 가능하도록 */
              // xDriverClose(hDriver);  /* 주석 처리: 통신 유지 */
          }
          else
          {
              printf("ERROR: xDriverOpen failed: 0x%08X\r\n", (unsigned int)lRet);
          }

          /* ✅ 수정: 통계 출력만 수행하고 채널은 유지 */
          printf("\r\n========================================\r\n");
          printf(" VAT Test Statistics\r\n");
          printf("========================================\r\n");
          VAT_Test_PrintStats(&g_tVatContext);
          printf("Communication channel remains active.\r\n");
          printf("========================================\r\n\r\n");
          // VAT_Test_Deinit(&g_tVatContext);  /* 주석 처리: 통신 유지 */
      }
      else
      {
          /* 기존 App_CifXApplicationDemo 실행 */
          printf("Running original App_CifXApplicationDemo...\r\n");
          lRet = App_CifXApplicationDemo("cifX0");
      }

      /* ========== VAT TEST INTEGRATION - 메인 로직 수정 끝 ========== */
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI4_Init(void)
{

  /* USER CODE BEGIN SPI4_Init 0 */

  /* USER CODE END SPI4_Init 0 */

  /* USER CODE BEGIN SPI4_Init 1 */

  /* USER CODE END SPI4_Init 1 */
  /* SPI4 parameter configuration*/
  hspi4.Instance = SPI4;
  hspi4.Init.Mode = SPI_MODE_MASTER;
  hspi4.Init.Direction = SPI_DIRECTION_2LINES;
  hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi4.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi4.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi4.Init.NSS = SPI_NSS_SOFT;
  hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi4.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi4.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI4_Init 2 */

  /* USER CODE END SPI4_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 200-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, NCS_MEMS_SPI_Pin|CSX_Pin|OTG_FS_PSO_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SPI5_CS_Pin|GPIO_PIN_6|ACP_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, RDX_Pin|WRX_DCX_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, LD3_Pin|LD4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : A0_Pin A1_Pin A2_Pin A3_Pin
                           A4_Pin A5_Pin SDNRAS_Pin A6_Pin
                           A7_Pin A8_Pin A9_Pin */
  GPIO_InitStruct.Pin = A0_Pin|A1_Pin|A2_Pin|A3_Pin
                          |A4_Pin|A5_Pin|SDNRAS_Pin|A6_Pin
                          |A7_Pin|A8_Pin|A9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI5_SCK_Pin SPI5_MISO_Pin SPI5_MOSI_Pin */
  GPIO_InitStruct.Pin = SPI5_SCK_Pin|SPI5_MISO_Pin|SPI5_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : ENABLE_Pin */
  GPIO_InitStruct.Pin = ENABLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(ENABLE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SDNWE_Pin */
  GPIO_InitStruct.Pin = SDNWE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(SDNWE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : NCS_MEMS_SPI_Pin CSX_Pin OTG_FS_PSO_Pin */
  GPIO_InitStruct.Pin = NCS_MEMS_SPI_Pin|CSX_Pin|OTG_FS_PSO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : B1_Pin MEMS_INT1_Pin MEMS_INT2_Pin TP_INT1_Pin */
  GPIO_InitStruct.Pin = B1_Pin|MEMS_INT1_Pin|MEMS_INT2_Pin|TP_INT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : B5_Pin VSYNC_Pin R4_Pin R5_Pin */
  GPIO_InitStruct.Pin = B5_Pin|VSYNC_Pin|R4_Pin|R5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI5_CS_Pin PA6 ACP_RST_Pin */
  GPIO_InitStruct.Pin = SPI5_CS_Pin|GPIO_PIN_6|ACP_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OC_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : R3_Pin R6_Pin */
  GPIO_InitStruct.Pin = R3_Pin|R6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : A10_Pin A11_Pin BA0_Pin BA1_Pin
                           SDCLK_Pin SDNCAS_Pin */
  GPIO_InitStruct.Pin = A10_Pin|A11_Pin|BA0_Pin|BA1_Pin
                          |SDCLK_Pin|SDNCAS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : D4_Pin D5_Pin D6_Pin D7_Pin
                           D8_Pin D9_Pin D10_Pin D11_Pin
                           D12_Pin NBL0_Pin NBL1_Pin */
  GPIO_InitStruct.Pin = D4_Pin|D5_Pin|D6_Pin|D7_Pin
                          |D8_Pin|D9_Pin|D10_Pin|D11_Pin
                          |D12_Pin|NBL0_Pin|NBL1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : G4_Pin G5_Pin B6_Pin B7_Pin */
  GPIO_InitStruct.Pin = G4_Pin|G5_Pin|B6_Pin|B7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : D13_Pin D14_Pin D15_Pin D0_Pin
                           D1_Pin D2_Pin D3_Pin */
  GPIO_InitStruct.Pin = D13_Pin|D14_Pin|D15_Pin|D0_Pin
                          |D1_Pin|D2_Pin|D3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : TE_Pin */
  GPIO_InitStruct.Pin = TE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RDX_Pin WRX_DCX_Pin */
  GPIO_InitStruct.Pin = RDX_Pin|WRX_DCX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : R7_Pin DOTCLK_Pin B3_Pin */
  GPIO_InitStruct.Pin = R7_Pin|DOTCLK_Pin|B3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : HSYNC_Pin G6_Pin R2_Pin */
  GPIO_InitStruct.Pin = HSYNC_Pin|G6_Pin|R2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : I2C3_SDA_Pin */
  GPIO_InitStruct.Pin = I2C3_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
  HAL_GPIO_Init(I2C3_SDA_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : I2C3_SCL_Pin */
  GPIO_InitStruct.Pin = I2C3_SCL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
  HAL_GPIO_Init(I2C3_SCL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : G7_Pin B2_Pin */
  GPIO_InitStruct.Pin = G7_Pin|B2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : G3_Pin B4_Pin */
  GPIO_InitStruct.Pin = G3_Pin|B4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : LD3_Pin LD4_Pin */
  GPIO_InitStruct.Pin = LD3_Pin|LD4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : SDCKE1_Pin SDNE1_Pin */
  GPIO_InitStruct.Pin = SDCKE1_Pin|SDNE1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//
//static int32_t PreCommunicateWithSPI0()
//
//{
//
//int32_t lRet = CIFX_NO_ERROR;
//
//uint8_t tx_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
//
//uint8_t rx_data[10] = {0};
//
//printf("Starting SPI0 pre-communication with netX90...\r\n");
//
//// SPI0 통신 수행 (예시)
//
//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); //
//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // CS Low
//
//if(HAL_SPI_TransmitReceive(&hspi4, tx_data, rx_data, 10, 1000) != HAL_OK)
//
//{
//
//lRet = CIFX_FUNCTION_FAILED;
//
//printf("SPI0 communication failed\r\n");
//
//}
//
//else
//
//{
//
//printf("SPI0 communication successful\r\n");
//
//// 수신된 데이터 처리
//
//for(int i = 0; i < 10; i++)
//
//{
//
//printf("RX[%d]: 0x%02X\r\n", i, rx_data[i]);
//
//}
//
//}
//
//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); // CS High
//
//return lRet;
//
//}



// 인터럽트 콜백
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//  if (huart->Instance == UART5)
//  {
//    rxReady = 1;
//  }
//}

// 에러 콜백
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == UART5)
  {
    // 에러 종류 확인 및 처리
    uint32_t error = HAL_UART_GetError(huart);
    
    HAL_UART_Transmit(&huart5, (uint8_t*)"UART Error Code: 0x", 19, HAL_MAX_DELAY);
    
    // 에러 코드를 16진수로 출력
    char errorStr[9];
    sprintf(errorStr, "%08X", (unsigned int)error);
    HAL_UART_Transmit(&huart5, (uint8_t*)errorStr, 8, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
    
    // UART 상태 재설정 및 수신 재시작
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE)) {
        __HAL_UART_CLEAR_OREFLAG(huart);
    }
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_NE)) {
        __HAL_UART_CLEAR_NEFLAG(huart);
    }
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_FE)) {
        __HAL_UART_CLEAR_FEFLAG(huart);
    }
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_PE)) {
        __HAL_UART_CLEAR_PEFLAG(huart);
    }
    
    // UART 수신 재시작
    HAL_UART_Receive_IT(huart, &rx_temp, 1);
  }
}


//
//static int32_t PreCommunicateWithSPI0()
//{
//    int32_t lRet = CIFX_NO_ERROR;
//    uint8_t tx_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
////    uint8_t rx_data[10] = {0};
//    uint32_t success_count = 0;
//
//    printf("Starting SPI0 pre-communication with netX90 (20 attempts)...\r\n");
//
//    // 20회 반복
//    for(int attempt = 1; attempt <= 20; attempt++)
//    {
//        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // CS Low
////        HAL_Delay(1);
//        for (volatile int i = 0; i < 1000; i++) __NOP();  // 약 100~150μs
//
//        if(HAL_SPI_TransmitReceive(&hspi4, tx_data, rx_data, 10, 1000) != HAL_OK)
//        {
//            printf("Attempt %d: FAILED\r\n", attempt);
//        }
//        else
//        {
//            printf("Attempt %d: SUCCESS\r\n", attempt);
//            success_count++;
//        }
//
//        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); // CS High
//        HAL_Delay(100); // 100ms 대기
//    }
//
//    printf("Success: %lu/20 attempts\r\n", success_count);
//
//    if(success_count > 0)
//        lRet = CIFX_NO_ERROR;
//    else
//        lRet = CIFX_FUNCTION_FAILED;
//
//    return lRet;
//}




void UART_Ring_Init(UART_HandleTypeDef *huart)
{
    huart_ring = huart;
    rx_ring.head = 0;
    rx_ring.tail = 0;

    // 첫 1바이트 수신 시작
    HAL_UART_Receive_IT(huart_ring, &rx_temp, 1);
}

// 인터럽트 콜백: 수신 완료 시 호출됨
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	  if (huart->Instance == UART5)
	  {
	    rxReady = 1;
	  }


    if (huart == huart_ring)
    {
        uint16_t next_head = (rx_ring.head + 1) % UART_RX_BUFFER_SIZE;

        // 버퍼가 가득 차면 덮어쓰지 않음 (오버플로우 방지)
        if (next_head != rx_ring.tail)
        {
            rx_ring.buffer[rx_ring.head] = rx_temp;
            rx_ring.head = next_head;
        }

        // 다음 수신 요청
        HAL_UART_Receive_IT(huart_ring, &rx_temp, 1);
    }
}

// Ring buffer에서 1바이트 읽기
int UART_Ring_Read(uint8_t *data)
{
    if (rx_ring.head == rx_ring.tail)
    {
        return 0; // 데이터 없음
    }

    *data = rx_ring.buffer[rx_ring.tail];
    rx_ring.tail = (rx_ring.tail + 1) % UART_RX_BUFFER_SIZE;
    return 1;
}


// 수정된 부분: 라인 종결 문자를 '\r' (0x0D)로 변경
// 줄 단위 수신 (0x0D로 종료되는 문자열)
int UART_Ring_ReadLine(uint8_t *lineBuf, uint16_t maxLen)
{
    uint16_t i = 0;
    uint8_t ch;

    // Read from the ring buffer until a line is complete or buffer is empty
    while (i < maxLen - 1)
    {
        if (!UART_Ring_Read(&ch))
        {
            // No new character available yet, return 0 to signal no complete line
            return 0;
        }

        if (ch == '\r')
        {
            // Found the carriage return terminator
            lineBuf[i] = '\0'; // Replace \r with null terminator
            return i;
        }
        else
        {
            lineBuf[i++] = ch;
        }
    }
    
    // Buffer overflow or no terminator found within maxLen.
    lineBuf[i] = '\0';
    return i;
}



/*!
 * \brief Extract DIP switch data from received UART line
 * \param[in] line - Received UART line buffer
 * \param[out] dipData - Extracted 4-bit DIP switch data (as string)
 * \return 1 if valid DIP data found, 0 otherwise
 */
static uint8_t ExtractDipSwitchData(const uint8_t *line, uint8_t *dipData)
{
    // Look for pattern "D:[XXXX]" where XXXX is 4 binary digits
    const char *dipStart = strstr((const char*)line, "D:[");
    
    if (dipStart == NULL) {
        HAL_UART_Transmit(&huart5, (uint8_t *)"No D pattern found\r\n", 20, HAL_MAX_DELAY);
        return 0;
    }
    
    dipStart += 3; // Move to start of binary digits
    
    // Validate 4 binary digits
    for (int i = 0; i < 4; i++)
    {
        if (dipStart[i] != '0' && dipStart[i] != '1') {
            HAL_UART_Transmit(&huart5, (uint8_t *)"Invalid D digit at pos ", 23, HAL_MAX_DELAY);
            char posStr[2] = {'0' + i, '\0'};
            HAL_UART_Transmit(&huart5, (uint8_t *)posStr, 1, HAL_MAX_DELAY);
            HAL_UART_Transmit(&huart5, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
            return 0;
        }
        dipData[i] = dipStart[i];
    }
    
    // Check for closing bracket
    if (dipStart[4] != ']') {
        HAL_UART_Transmit(&huart5, (uint8_t *)"Missing closing bracket\r\n", 25, HAL_MAX_DELAY);
        return 0;
    }
    
    dipData[4] = '\0'; // Null terminate
    
    HAL_UART_Transmit(&huart5, (uint8_t *)"D extracted: ", 13, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, dipData, 4, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
    
    return 1;
}

/*!
 * \brief Validate D switch data with single reception
 * \param[in] currentLine - Current received UART line
 * \return 1 if validation successful, 0 if extraction failed
 */
static uint8_t ValidateDipSwitchData(const uint8_t *currentLine)
{
    uint8_t currentDipData[5] = {0};
    
    HAL_UART_Transmit(&huart5, (uint8_t *)"Validating line: ", 17, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, (uint8_t *)currentLine, strlen((const char*)currentLine), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
    
    // Extract D data from current line
    if (ExtractDipSwitchData(currentLine, currentDipData) == 0) {
        HAL_UART_Transmit(&huart5, (uint8_t *)"D extraction failed\r\n", 21, HAL_MAX_DELAY);
        return 0; // No valid D data in current line
    }
    
    // Single reception validation - no comparison needed
    for(int i = 0; i < 5; i++) g_DipSwitchData[i] = currentDipData[i];
    g_bDipDataValid = 1;
    
    HAL_UART_Transmit(&huart5, (uint8_t *)"D validation SUCCESS: ", 22, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, (uint8_t*)g_DipSwitchData, 4, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
    
    return 1; // Validation successful immediately
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  * where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

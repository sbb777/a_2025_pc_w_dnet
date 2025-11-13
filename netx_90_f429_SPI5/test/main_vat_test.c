/* USER CODE BEGIN Header */
/**
 * VAT DeviceNet Test Integration - Modified Main
 *
 * 최소한의 변경으로 VAT Adaptive Pressure Controller 테스트 통합
 *
 * 변경 사항:
 * 1. vat_devicenet_test.h 추가
 * 2. 전역 VAT 테스트 컨텍스트 추가
 * 3. VAT 초기화 및 설정 추가
 * 4. App_CifXApplicationDemo 대신 VAT 테스트 실행
 *
 * Original: Louis 2025-07-22 15:54:47 -> 25-10-27 122009
 * Modified for VAT Test: 2025-10-27
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

/* ========== VAT TEST INTEGRATION - 추가 시작 ========== */
#include "vat_devicenet_test.h"
/* ========== VAT TEST INTEGRATION - 추가 끝 ========== */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
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
// 기존 전역 변수
volatile char g_szLastCookie[5] = {0};
uint8_t rx_data[10] = {0};
uint32_t delayCount = 0;
uint8_t rxBuffer[64];
uint8_t rxByte;
uint8_t txMessage[] = "STM32 UART5 says hello to netX90!\r\n";
volatile uint8_t rxReady = 0;
static UART_HandleTypeDef *huart_ring;
static RingBuffer rx_ring;
static uint8_t rx_temp;
uint8_t line[100];
volatile uint8_t g_DipSwitchData[5] = {0};
volatile uint8_t g_bDipDataValid = 0;

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
volatile uint8_t g_VatTestMode = 1;  // 기본값: Basic Pressure Control

/**
 * VAT 테스트 활성화 플래그
 * true = VAT 테스트 실행
 * false = 기존 App_CifXApplicationDemo 실행
 */
volatile bool g_bEnableVatTest = true;
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
static void VAT_InitializeTest(void);
static void VAT_RunTest(CIFXHANDLE hChannel);
/* ========== VAT TEST INTEGRATION - 추가 끝 ========== */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
    OS_Memcpy((void*)g_szLastCookie, szCookie, sizeof(g_szLastCookie));

    if((0 == OS_Strcmp(szCookie, CIFX_DPMSIGNATURE_BSL_STR)) ||
       (0 == OS_Strcmp(szCookie, CIFX_DPMSIGNATURE_FW_STR)))
    {
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

    g_ulTraceLevel = TRACE_LEVEL_ERROR |
                     TRACE_LEVEL_WARNING |
                     TRACE_LEVEL_INFO |
                     TRACE_LEVEL_DEBUG;

    ptDevInstance->fPCICard = 0;
    ptDevInstance->pvOSDependent = ptDevInstance;
    ptDevInstance->ulDPMSize = 0x8000;
    OS_Strncpy(ptDevInstance->szName, "cifX0", sizeof(ptDevInstance->szName));

    do
    {
      SerialDPM_Init(ptDevInstance);
    } while(false == isCookieAvailable(ptDevInstance, 100));

    lRet = cifXTKitAddDevice(ptDevInstance);

    if(CIFX_NO_ERROR != lRet)
    {
      cifXTKitDeinit();
    }
  }

  return lRet;
}

/* ========== VAT TEST INTEGRATION - 함수 구현 시작 ========== */

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

    /* 테스트 컨텍스트 초기화 */
    VAT_Test_Init(&g_tVatContext);

    /* 테스트 설정 */
    VAT_TEST_CONFIG_T tConfig = {
        .node_address = 10,                             /* DeviceNet 노드 주소 */
        .baud_rate = 250000,                            /* 250 kbps */
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
            printf("Test: Continuous Monitoring\r\n");
            printf("Cycles: 100 (10 seconds @ 100ms)\r\n");
            printf("Setpoint: 750\r\n\r\n");

            /* 압력 제어 모드 설정 */
            VAT_Test_SetControlMode(&g_tVatContext, VAT_CONTROL_MODE_PRESSURE);
            VAT_Test_SetPressureSetpoint(&g_tVatContext, 750);

            /* 연속 모니터링 루프 */
            for(int i = 0; i < 100; i++)
            {
                /* 제어 데이터 전송 */
                int32_t lRet = VAT_Test_WriteOutput(&g_tVatContext, hChannel);
                if(lRet != CIFX_NO_ERROR)
                {
                    printf("[%03d] Write Error: 0x%08X\r\n", i, (unsigned int)lRet);
                }

                /* 센서 데이터 읽기 */
                lRet = VAT_Test_ReadInput(&g_tVatContext, hChannel);
                if(lRet != CIFX_NO_ERROR)
                {
                    printf("[%03d] Read Error: 0x%08X\r\n", i, (unsigned int)lRet);
                }

                /* 예외 상태 확인 */
                if(VAT_Test_HasException(&g_tVatContext))
                {
                    printf("[%03d] WARNING: Exception 0x%02X\r\n",
                           i, VAT_Test_GetExceptionStatus(&g_tVatContext));
                }

                /* 매 10회마다 상태 출력 */
                if((i % 10) == 0 && g_tVatContext.input_data_valid)
                {
                    printf("[%03d] Pressure=%d, Position=%d, Status=0x%02X\r\n",
                           i,
                           g_tVatContext.input_asm100.pressure,
                           g_tVatContext.input_asm100.position,
                           g_tVatContext.input_asm100.device_status);
                }

                /* 100ms 대기 */
                HAL_Delay(100);
            }

            printf("\r\nContinuous monitoring completed.\r\n");
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
  int32_t lRet = CIFX_NO_ERROR;
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

  UART_Ring_Init(&huart5);

  /* DIP switch validation loop */
  HAL_UART_Transmit(&huart5, (uint8_t *)"Starting DIP validation loop...\r\n", 33, HAL_MAX_DELAY);

  while (!g_bDipDataValid)
  {
      int len = UART_Ring_ReadLine(line, sizeof(line));

      if (len > 0)
      {
          HAL_UART_Transmit(&huart5, (uint8_t *)"RX: ", 4, HAL_MAX_DELAY);
          HAL_UART_Transmit(&huart5, line, len, HAL_MAX_DELAY);
          HAL_UART_Transmit(&huart5, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);

          if (ValidateDipSwitchData(line) == 1)
          {
              HAL_UART_Transmit(&huart5, (uint8_t *)"DIP data validated!\r\n", 21, HAL_MAX_DELAY);
              break;
          }
          else
          {
              HAL_UART_Transmit(&huart5, (uint8_t *)"Validation FAILED, waiting...\r\n", 31, HAL_MAX_DELAY);
          }
      }

      HAL_Delay(100);
  }

  /* cifX Toolkit 초기화 */
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
                  /* 채널 준비 대기 */
                  CHANNEL_INFORMATION tChannelInfo;
                  printf("Waiting for channel ready...\r\n");

                  do
                  {
                      xChannelInfo(hChannel, sizeof(CHANNEL_INFORMATION), &tChannelInfo);
                  }
                  while(!(tChannelInfo.ulDeviceCOSFlags & HIL_COMM_COS_READY));

                  printf("Channel ready!\r\n");

                  /* VAT 테스트 실행 */
                  VAT_RunTest(hChannel);

                  /* 채널 닫기 */
                  xChannelClose(hChannel);
              }
              else
              {
                  printf("ERROR: xChannelOpen failed: 0x%08X\r\n", (unsigned int)lRet);
              }

              /* 드라이버 닫기 */
              xDriverClose(hDriver);
          }
          else
          {
              printf("ERROR: xDriverOpen failed: 0x%08X\r\n", (unsigned int)lRet);
          }

          /* 테스트 종료 */
          VAT_Test_Deinit(&g_tVatContext);
      }
      else
      {
          /* 기존 App_CifXApplicationDemo 실행 */
          printf("Running original App_CifXApplicationDemo...\r\n");
          lRet = App_CifXApplicationDemo("cifX0");
      }

      /* ========== VAT TEST INTEGRATION - 메인 로직 수정 끝 ========== */
  }
  else
  {
      printf("ERROR: InitializeToolkit failed: 0x%08X\r\n", (unsigned int)lRet);
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

/* 나머지 함수들은 원본과 동일 (SystemClock_Config, MX_* 등) */
/* 생략... */

/*****************************************************************************/
/*! UART Ring Buffer Functions (원본과 동일)                               */
/*****************************************************************************/

void UART_Ring_Init(UART_HandleTypeDef *huart)
{
    huart_ring = huart;
    rx_ring.head = 0;
    rx_ring.tail = 0;
    HAL_UART_Receive_IT(huart_ring, &rx_temp, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART5)
    {
        rxReady = 1;
    }

    if (huart == huart_ring)
    {
        uint16_t next_head = (rx_ring.head + 1) % UART_RX_BUFFER_SIZE;

        if (next_head != rx_ring.tail)
        {
            rx_ring.buffer[rx_ring.head] = rx_temp;
            rx_ring.head = next_head;
        }

        HAL_UART_Receive_IT(huart_ring, &rx_temp, 1);
    }
}

int UART_Ring_Read(uint8_t *data)
{
    if (rx_ring.head == rx_ring.tail)
        return 0;

    *data = rx_ring.buffer[rx_ring.tail];
    rx_ring.tail = (rx_ring.tail + 1) % UART_RX_BUFFER_SIZE;
    return 1;
}

int UART_Ring_ReadLine(uint8_t *lineBuf, uint16_t maxLen)
{
    uint16_t i = 0;
    uint8_t ch;

    while (i < maxLen - 1)
    {
        if (!UART_Ring_Read(&ch))
            return 0;

        if (ch == '\r')
        {
            lineBuf[i] = '\0';
            return i;
        }
        else
        {
            lineBuf[i++] = ch;
        }
    }

    lineBuf[i] = '\0';
    return i;
}

static uint8_t ExtractDipSwitchData(const uint8_t *line, uint8_t *dipData)
{
    const char *dipStart = strstr((const char*)line, "D:[");

    if (dipStart == NULL)
        return 0;

    dipStart += 3;

    for (int i = 0; i < 4; i++)
    {
        if (dipStart[i] != '0' && dipStart[i] != '1')
            return 0;
        dipData[i] = dipStart[i];
    }

    if (dipStart[4] != ']')
        return 0;

    dipData[4] = '\0';
    return 1;
}

static uint8_t ValidateDipSwitchData(const uint8_t *currentLine)
{
    uint8_t currentDipData[5] = {0};

    if (ExtractDipSwitchData(currentLine, currentDipData) == 0)
        return 0;

    for(int i = 0; i < 5; i++)
        g_DipSwitchData[i] = currentDipData[i];
    g_bDipDataValid = 1;

    return 1;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART5)
    {
        uint32_t error = HAL_UART_GetError(huart);

        if(__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE))
            __HAL_UART_CLEAR_OREFLAG(huart);
        if(__HAL_UART_GET_FLAG(huart, UART_FLAG_NE))
            __HAL_UART_CLEAR_NEFLAG(huart);
        if(__HAL_UART_GET_FLAG(huart, UART_FLAG_FE))
            __HAL_UART_CLEAR_FEFLAG(huart);
        if(__HAL_UART_GET_FLAG(huart, UART_FLAG_PE))
            __HAL_UART_CLEAR_PEFLAG(huart);

        HAL_UART_Receive_IT(huart, &rx_temp, 1);
    }
}

/* 나머지 HAL 초기화 함수들 생략 (원본과 동일) */

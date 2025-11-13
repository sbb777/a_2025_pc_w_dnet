/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "APC_Main.h"
#include "APC_AsyncHandler.h"
#include "APC_PidHandler.h"
#include "APC_Scheduler.h"
#include "APC_Synch.h"
#include "APC_ControlMode.h"
#include "APC_CompAir.h"
#include "APC_Power.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
int stopPidRun_ = 0; //louis
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for asyncTask */
osThreadId_t asyncTaskHandle;
const osThreadAttr_t asyncTask_attributes = {
  .name = "asyncTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for pidTask */
osThreadId_t pidTaskHandle;
const osThreadAttr_t pidTask_attributes = {
  .name = "pidTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for pollTask */
osThreadId_t pollTaskHandle;
const osThreadAttr_t pollTask_attributes = {
  .name = "pollTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for CheckInputTask */
osThreadId_t CheckInputTaskHandle;
const osThreadAttr_t CheckInputTask_attributes = {
        .name = "CheckInputTask",
        .stack_size = 128 * 4,
        .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for jobQueue */
osMessageQueueId_t jobQueueHandle;
const osMessageQueueAttr_t jobQueue_attributes = {
  .name = "jobQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartAsyncTask(void *argument);
void StartPidTask(void *argument);
void StartPollTask(void *argument);
void StartCheckInputTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of jobQueue */
  jobQueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &jobQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  //registerMQ(&jobQueueHandle);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of asyncTask */
  asyncTaskHandle = osThreadNew(StartAsyncTask, NULL, &asyncTask_attributes);

  /* creation of pidTask */
  pidTaskHandle = osThreadNew(StartPidTask, NULL, &pidTask_attributes);

  /* creation of pollTask */
  pollTaskHandle = osThreadNew(StartPollTask, NULL, &pollTask_attributes);

  /* creation of CheckInputTask */
  //CheckInputTaskHandle = osThreadNew(StartCheckInputTask, NULL, &CheckInputTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
#define ADDR_CS_IF1_     (1 << 17)
uint16_t readc = 0;
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  initMain();

  uint32_t counter = 0;
  /* Infinite loop */
  for(;;)
  {
    osDelay(2);

#ifdef __REV_MTECH__
    // Check Thread Running
    HAL_GPIO_TogglePin(GPIOD, LD1_LED_Pin);
    //
#endif

    readc = readSRAM4(ADDR_CS_IF1_);
    printLocalRS232(APC_PORT_LOCAL, "[LOCAL INPUT] %02x\r\n", readc);
    // Test Relay
#if 1
    //writeRELAY_OPEN(APC_HIGH);
    //osDelay(1000);
    //writeRELAY_OPEN(APC_LOW);
    //osDelay(1000);

    writeRELAY_CLOSE(APC_HIGH);
    osDelay(1000);
    writeRELAY_CLOSE(APC_LOW);
    osDelay(1000);
#endif

    // Test DAC7612
#if 1
    writeAOUT_POSITION(0);
    writeAOUT_PRESSURE(30000);
    osDelay(500);
    //writeAOUT_POSITION(1000);
    writeAOUT_PRESSURE(20000);
    osDelay(500);
    //osDelay(1000);
    //writeAOUT_POSITION(2000);
    writeAOUT_PRESSURE(10000);
    osDelay(500);
    //osDelay(1000);
    writeAOUT_POSITION(100000);
    writeAOUT_PRESSURE(0);
    osDelay(500);
    //osDelay(1000);
#endif

    // Test MAX1116
#if 1
    osDelay(500);
    // in CONNECTOR
    refreshPower();

    // in MASTER
    transfer_VDRUCK_Spi();
#endif

#ifdef __REV_MTECH__
    uint8_t eeprom = 0;
    if(true == getE2promData(100, eeprom))	// forced true ???
    {
      printLocalRS232(APC_PORT_LOCAL, "[LOCAL DBG] %02x\r\n", eeprom);
    }

    eeprom = 0x55;
    if(true == setE2promData(100, eeprom))	// forced true ???
    {
      printLocalRS232(APC_PORT_LOCAL, "[LOCAL DBG] %02x\r\n", eeprom);
    }

    eeprom = 0;
    if(true == getE2promData(100, eeprom))	// forced true ???
    {
      printLocalRS232(APC_PORT_LOCAL, "[LOCAL DBG] %02x\r\n", eeprom);
    }

    //writeRemoteRS232Data()
    writeRemoteRS232Data((uint8_t *) "ABCDEFGH", strlen("ABCDEFGH"));
#endif

    procMain(counter);

    if (getInitStatus() == true) {
        scheduleSensor(counter);
        scheduleValve(counter);
        scheduleDisplay(counter);
        counter++;
    }
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartAsyncTask */
/**
* @brief Function implementing the asyncTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartAsyncTask */
void StartAsyncTask(void *argument)
{
  /* USER CODE BEGIN StartAsyncTask */
  uint32_t counter = 0;
  /* Infinite loop */
  for(;;)
  {
    osDelay(5);
    procAsyncHandler(counter);
    counter++;
  }
  /* USER CODE END StartAsyncTask */
}

/* USER CODE BEGIN Header_StartPidTask */
/**
* @brief Function implementing the pidTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartPidTask */
void StartPidTask(void *argument)
{
  /* USER CODE BEGIN StartPidTask */
  static uint32_t counter = 0; //louis
  static bool initJobExecuted = false;//l_0701_16
  extern double Ioutput_sum;
  extern double Prev_Error;
  /* Infinite loop */
  for(;;)
  {
#if 1
    osDelay(5);
#else
    if(getControlMode() != ControlMode_PRESSURE){
        setPidRunState(false);
       Ioutput_sum =0.0;
       Prev_Error = 0.0;
    }
    osDelay(5);

		if (stopPidRun_ == 1)  {
			setPidRunState(false);
			if (!initJobExecuted) {
				initJob();
				initJobExecuted = true; // Set the flag to true after executing initJob()
			}
		}

		if(getPidRunState() && (getControlMode() != ControlMode_PRESSURE)){
			setPidRunState(false);
			Ioutput_sum =0.0;
			Prev_Error = 0.0;
		}
		if (getSynchStatus() == true) {
			if(getPidRunState()){
				procPid();
				//            osDelay(30);//l,0708
				osDelay(5);//l,0708
				//            osDelay(1);//l,0708
				counter++;
			}
		}
#endif
  }
  /* USER CODE END StartPidTask */
}

/* USER CODE BEGIN Header_StartPollTask */
/**
* @brief Function implementing the pollTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartPollTask */
void StartPollTask(void *argument)
{
  /* USER CODE BEGIN StartPollTask */
  uint32_t counter = 0;
  /* Infinite loop */
  for(;;)
  {
#if 1
  	osDelay(3);         // osDelay(1);
#else
  	osDelay(3);         // osDelay(1);

    if (getInitStatus() == true) {
        scheduleInterrupt(counter);
        //scheduleSensor(counter);
        //scheduleValve(counter);
        //scheduleDisplay(counter);
        schedulePower(counter);
        scheduleAir(counter);
        scheduleRemotePort(counter);
        counter++;
    }
#endif
  }
  /* USER CODE END StartPollTask */
}

/* USER CODE BEGIN Header_StartCheckInputTask */
/**
* @brief Function implementing the CheckInputTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCheckInputTask */
void StartCheckInputTask(void *argument)
{
    /* USER CODE BEGIN StartCheckInputTask */
    uint32_t counter = 0;
    /* Infinite loop */
    while(true){
        if(getInitStatus() == true){
            scheduleInterrupt(counter);
            _cur_volt = getMax1116Value();
            _comp_air_val = getCompAir(APC_PORT_INTERNAL);      // Pendulum Isolation Type에서만 호출하도록.
            CheckRemote();
        }
        counter++;
        osDelay(10);        // 10ms delay
    }
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */


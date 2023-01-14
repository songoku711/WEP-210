/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include <stdio.h>
#include "usart.h"

#include "ioManager.h"
#include "sensorManager.h"
#include "mb.h"

#include "max6675.h"
#include "hc595.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Event flag for systemInitEvent */
#define SYSTEM_INIT_EVENT_MB_READY                (uint32_t)(0x0001)
#define SYSTEM_INIT_EVENT_IO_READY                (uint32_t)(0x0002)
#define SYSTEM_INIT_EVENT_SENS_READY              (uint32_t)(0x0004)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
/* Serial Tx task */
osThreadId_t eMB_PortSerialTxTaskHandle;

const osThreadAttr_t eMB_PortSerialTxTask_attributes = {
  .name = "eMBTxTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal7,
};

osEventFlagsId_t eMB_PortSerialTxEvent;
/* USER CODE END Variables */
/* Definitions for mainTask */
osThreadId_t mainTaskHandle;
const osThreadAttr_t mainTask_attributes = {
  .name = "mainTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for eMBTask */
osThreadId_t eMBTaskHandle;
const osThreadAttr_t eMBTask_attributes = {
  .name = "eMBTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for ioManagerTask */
osThreadId_t ioManagerTaskHandle;
const osThreadAttr_t ioManagerTask_attributes = {
  .name = "ioManagerTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for sensorTask */
osThreadId_t sensorTaskHandle;
const osThreadAttr_t sensorTask_attributes = {
  .name = "sensorTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ioExtIrqCbkTask */
osThreadId_t ioExtIrqCbkTaskHandle;
const osThreadAttr_t ioExtIrqCbkTask_attributes = {
  .name = "ioExtIrqCbkTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal7,
};
/* Definitions for adcConvCbkTask */
osThreadId_t adcConvCbkTaskHandle;
const osThreadAttr_t adcConvCbkTask_attributes = {
  .name = "adcConvCbkTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal7,
};
/* Definitions for ioInputStateQueue */
osMessageQueueId_t ioInputStateQueueHandle;
const osMessageQueueAttr_t ioInputStateQueue_attributes = {
  .name = "ioInputStateQueue"
};
/* Definitions for mainFunctionTimer */
osTimerId_t mainFunctionTimerHandle;
const osTimerAttr_t mainFunctionTimer_attributes = {
  .name = "mainFunctionTimer"
};
/* Definitions for systemInitEvent */
osEventFlagsId_t systemInitEventHandle;
const osEventFlagsAttr_t systemInitEvent_attributes = {
  .name = "systemInitEvent"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
 
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}
/* USER CODE END FunctionPrototypes */

void startMainTask(void *argument);
void starteMBTask(void *argument);
void startIoManagerTask(void *argument);
void startSensorTask(void *argument);
void startIoExtIrqCbkTask(void *argument);
void startAdcConvCbkTask(void *argument);
void mainFunctionTimerHandler(void *argument);

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

  /* Create the timer(s) */
  /* creation of mainFunctionTimer */
  mainFunctionTimerHandle = osTimerNew(mainFunctionTimerHandler, osTimerPeriodic, NULL, &mainFunctionTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of ioInputStateQueue */
  ioInputStateQueueHandle = osMessageQueueNew (10, sizeof(uint32_t), &ioInputStateQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of mainTask */
  mainTaskHandle = osThreadNew(startMainTask, NULL, &mainTask_attributes);

  /* creation of eMBTask */
  eMBTaskHandle = osThreadNew(starteMBTask, NULL, &eMBTask_attributes);

  /* creation of ioManagerTask */
  ioManagerTaskHandle = osThreadNew(startIoManagerTask, NULL, &ioManagerTask_attributes);

  /* creation of sensorTask */
  sensorTaskHandle = osThreadNew(startSensorTask, NULL, &sensorTask_attributes);

  /* creation of ioExtIrqCbkTask */
  ioExtIrqCbkTaskHandle = osThreadNew(startIoExtIrqCbkTask, NULL, &ioExtIrqCbkTask_attributes);

  /* creation of adcConvCbkTask */
  adcConvCbkTaskHandle = osThreadNew(startAdcConvCbkTask, NULL, &adcConvCbkTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  eMB_PortSerialTxTaskHandle = osThreadNew(eMB_PortSerialTxTask, NULL, &eMB_PortSerialTxTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* Create the event(s) */
  /* creation of systemInitEvent */
  systemInitEventHandle = osEventFlagsNew(&systemInitEvent_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  eMB_PortSerialTxEvent = osEventFlagsNew(NULL);
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_startMainTask */
/**
  * @brief  Function implementing the mainTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_startMainTask */
void startMainTask(void *argument)
{
  /* USER CODE BEGIN startMainTask */
  /* Wait for all initializations done */
  (void)osEventFlagsWait( systemInitEventHandle, \
                          SYSTEM_INIT_EVENT_MB_READY | SYSTEM_INIT_EVENT_IO_READY | SYSTEM_INIT_EVENT_SENS_READY, \
                          osFlagsWaitAny | osFlagsNoClear, \
                          osWaitForever);
  
  while (1)
  {
    osDelay(1000);
  }
  /* USER CODE END startMainTask */
}

/* USER CODE BEGIN Header_starteMBTask */
/**
* @brief Function implementing the eMBTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_starteMBTask */
void starteMBTask(void *argument)
{
  /* USER CODE BEGIN starteMBTask */
  eMBInit(MB_RTU, 0x01, 2, 38400, MB_PAR_NONE);
  eMBEnable();
  
  osEventFlagsSet(systemInitEventHandle, SYSTEM_INIT_EVENT_MB_READY);
  
  /* Infinite loop */
  while (1)
  {
    eMBPoll();
  }
  /* USER CODE END starteMBTask */
}

/* USER CODE BEGIN Header_startIoManagerTask */
/**
* @brief Function implementing the ioManagerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startIoManagerTask */
void startIoManagerTask(void *argument)
{
  /* USER CODE BEGIN startIoManagerTask */
  /* Wait for Modbus initialization done */
  (void)osEventFlagsWait( systemInitEventHandle, \
                          SYSTEM_INIT_EVENT_MB_READY, \
                          osFlagsWaitAny | osFlagsNoClear, \
                          osWaitForever);
  
  IoManager_Init();
  
  osEventFlagsSet(systemInitEventHandle, SYSTEM_INIT_EVENT_IO_READY);
  
  /* Infinite loop */
  while (1)
  {
    IoManager_MainFunction();
    osDelay(5);
  }
  /* USER CODE END startIoManagerTask */
}

/* USER CODE BEGIN Header_startSensorTask */
/**
* @brief Function implementing the sensorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startSensorTask */
void startSensorTask(void *argument)
{
  /* USER CODE BEGIN startSensorTask */
  /* Wait for Modbus initialization done */
  (void)osEventFlagsWait( systemInitEventHandle, \
                          SYSTEM_INIT_EVENT_MB_READY, \
                          osFlagsWaitAny | osFlagsNoClear, \
                          osWaitForever);
  
  SensorManager_Init();
  
  osEventFlagsSet(systemInitEventHandle, SYSTEM_INIT_EVENT_SENS_READY);
  
  /* Infinite loop */
  while (1)
  {
    SensorManager_MainFunction();
    osDelay(5);
  }
  /* USER CODE END startSensorTask */
}

/* USER CODE BEGIN Header_startIoExtIrqCbkTask */
/**
* @brief Function implementing the ioExtIrqCbkTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startIoExtIrqCbkTask */
void startIoExtIrqCbkTask(void *argument)
{
  /* USER CODE BEGIN startIoExtIrqCbkTask */
  /* Wait for I/O initialization done */
  (void)osEventFlagsWait( systemInitEventHandle, \
                          SYSTEM_INIT_EVENT_IO_READY, \
                          osFlagsWaitAny | osFlagsNoClear, \
                          osWaitForever);
  
  /* Infinite loop */
  while (1)
  {
    IoManager_ExtIrptMainFunction();
  }
  /* USER CODE END startIoExtIrqCbkTask */
}

/* USER CODE BEGIN Header_startAdcConvCbkTask */
/**
* @brief Function implementing the adcConvCbkTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startAdcConvCbkTask */
void startAdcConvCbkTask(void *argument)
{
  /* USER CODE BEGIN startAdcConvCbkTask */
  /* Wait for sensor initialization done */
  (void)osEventFlagsWait( systemInitEventHandle, \
                          SYSTEM_INIT_EVENT_SENS_READY, \
                          osFlagsWaitAny | osFlagsNoClear, \
                          osWaitForever);
  
  /* Infinite loop */
  while (1)
  {
    SensorManager_AdcConvMainFunction();
  }
  /* USER CODE END startAdcConvCbkTask */
}

/* mainFunctionTimerHandler function */
void mainFunctionTimerHandler(void *argument)
{
  /* USER CODE BEGIN mainFunctionTimerHandler */

  /* USER CODE END mainFunctionTimerHandler */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */


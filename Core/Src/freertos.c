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
#include "mb.h"
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
/* USER CODE BEGIN Variables */
USHORT inputData[8];
/* USER CODE END Variables */
/* Definitions for mainTask */
osThreadId_t mainTaskHandle;
const osThreadAttr_t mainTask_attributes = {
  .name = "mainTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for modbusSlaveTask */
osThreadId_t modbusSlaveTaskHandle;
const osThreadAttr_t modbusSlaveTask_attributes = {
  .name = "modbusSlaveTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void startMainTask(void *argument);
void startModbusSlaveTask(void *argument);

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

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of mainTask */
  mainTaskHandle = osThreadNew(startMainTask, NULL, &mainTask_attributes);

  /* creation of modbusSlaveTask */
  modbusSlaveTaskHandle = osThreadNew(startModbusSlaveTask, NULL, &modbusSlaveTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
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
  
  inputData[0] = 'A';
  inputData[1] = 'a';
  
  /* Infinite loop */
  for(;;)
  {
    eMBSlaveSetDataU16(MB_SLAVE_REG_TYPE_INPUT, 1, inputData[0]);
    eMBSlaveSetDataU16(MB_SLAVE_REG_TYPE_INPUT, 2, inputData[1]);
    
    if (++inputData[0] > 'Z')
    {
      inputData[0] = 'A';
    }
    
    if (++inputData[1] > 'z')
    {
      inputData[1] = 'a';
    }
    
    osDelay(1000);
  }
  /* USER CODE END startMainTask */
}

/* USER CODE BEGIN Header_startModbusSlaveTask */
/**
* @brief Function implementing the modbusSlaveTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startModbusSlaveTask */
void startModbusSlaveTask(void *argument)
{
  /* USER CODE BEGIN startModbusSlaveTask */
  eMBInit(MB_RTU, 0x01, 2, 38400, MB_PAR_NONE);
  eMBEnable();
  
  /* Infinite loop */
  for(;;)
  {
    eMBPoll();
  }
  /* USER CODE END startModbusSlaveTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */


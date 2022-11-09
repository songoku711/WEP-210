/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC_EXT_CS_Pin GPIO_PIN_4
#define ADC_EXT_CS_GPIO_Port GPIOA
#define ADC_EXT_SCK_Pin GPIO_PIN_5
#define ADC_EXT_SCK_GPIO_Port GPIOA
#define ADC_EXT_MISO_Pin GPIO_PIN_6
#define ADC_EXT_MISO_GPIO_Port GPIOA
#define ADC_EXT_MOSI_Pin GPIO_PIN_7
#define ADC_EXT_MOSI_GPIO_Port GPIOA
#define ADC_EXT_IRQ_Pin GPIO_PIN_0
#define ADC_EXT_IRQ_GPIO_Port GPIOB
#define ADC_EXT_IRQ_EXTI_IRQn EXTI0_IRQn
#define ADC_EXT_MCLK_Pin GPIO_PIN_1
#define ADC_EXT_MCLK_GPIO_Port GPIOB
#define DBG_TX_Pin GPIO_PIN_10
#define DBG_TX_GPIO_Port GPIOB
#define DBG_RX_Pin GPIO_PIN_11
#define DBG_RX_GPIO_Port GPIOB
#define HC595_SCK_Pin GPIO_PIN_13
#define HC595_SCK_GPIO_Port GPIOB
#define HC595_MOSI_Pin GPIO_PIN_15
#define HC595_MOSI_GPIO_Port GPIOB
#define HC595_CS_Pin GPIO_PIN_8
#define HC595_CS_GPIO_Port GPIOA
#define RS485_TX_Pin GPIO_PIN_9
#define RS485_TX_GPIO_Port GPIOA
#define RS485_RX_Pin GPIO_PIN_10
#define RS485_RX_GPIO_Port GPIOA
#define RS485_CS_Pin GPIO_PIN_12
#define RS485_CS_GPIO_Port GPIOA
#define SENS_OUT_1_Pin GPIO_PIN_4
#define SENS_OUT_1_GPIO_Port GPIOB
#define SENS_OUT_1_EXTI_IRQn EXTI4_IRQn
#define SENS_OUT_3_Pin GPIO_PIN_5
#define SENS_OUT_3_GPIO_Port GPIOB
#define SENS_OUT_3_EXTI_IRQn EXTI9_5_IRQn
#define SENS_OUT_2_Pin GPIO_PIN_6
#define SENS_OUT_2_GPIO_Port GPIOB
#define SENS_OUT_2_EXTI_IRQn EXTI9_5_IRQn
#define SENS_OUT_4_Pin GPIO_PIN_7
#define SENS_OUT_4_GPIO_Port GPIOB
#define SENS_OUT_4_EXTI_IRQn EXTI9_5_IRQn
#define MEM_SCL_Pin GPIO_PIN_8
#define MEM_SCL_GPIO_Port GPIOB
#define MEM_SDA_Pin GPIO_PIN_9
#define MEM_SDA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

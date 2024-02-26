/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

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
#define EtherCAT_IRQ_Pin GPIO_PIN_2
#define EtherCAT_IRQ_GPIO_Port GPIOE
#define EtherCAT_IRQ_EXTI_IRQn EXTI2_IRQn
#define EtherCAT_SYNC0_Pin GPIO_PIN_4
#define EtherCAT_SYNC0_GPIO_Port GPIOE
#define EtherCAT_SYNC0_EXTI_IRQn EXTI4_IRQn
#define EtherCAT_SYNC1_Pin GPIO_PIN_5
#define EtherCAT_SYNC1_GPIO_Port GPIOE
#define EtherCAT_SYNC1_EXTI_IRQn EXTI9_5_IRQn
#define HC595_RCLK_Pin GPIO_PIN_6
#define HC595_RCLK_GPIO_Port GPIOE
#define FPGA_RST_Pin GPIO_PIN_13
#define FPGA_RST_GPIO_Port GPIOC
#define Power_12V_Pin GPIO_PIN_2
#define Power_12V_GPIO_Port GPIOC
#define ADC_CS_Pin GPIO_PIN_3
#define ADC_CS_GPIO_Port GPIOA
#define Bluetooh_Enable_Pin GPIO_PIN_4
#define Bluetooh_Enable_GPIO_Port GPIOC
#define RUN_LED_Pin GPIO_PIN_2
#define RUN_LED_GPIO_Port GPIOB
#define HC595_SER_0_Pin GPIO_PIN_13
#define HC595_SER_0_GPIO_Port GPIOB
#define HC595_OE_0_Pin GPIO_PIN_14
#define HC595_OE_0_GPIO_Port GPIOB
#define HC595_OE_1_Pin GPIO_PIN_11
#define HC595_OE_1_GPIO_Port GPIOD
#define HC595_SRCLK_Pin GPIO_PIN_13
#define HC595_SRCLK_GPIO_Port GPIOD
#define Test_Mode_Pin GPIO_PIN_8
#define Test_Mode_GPIO_Port GPIOG
#define HC595_SER_1_Pin GPIO_PIN_2
#define HC595_SER_1_GPIO_Port GPIOD
#define HC595_SRCLR_Pin GPIO_PIN_3
#define HC595_SRCLR_GPIO_Port GPIOD
#define EEPROM_SDA_Pin GPIO_PIN_7
#define EEPROM_SDA_GPIO_Port GPIOB
#define EEPROM_SCL_Pin GPIO_PIN_8
#define EEPROM_SCL_GPIO_Port GPIOB
#define ADC_RST_Pin GPIO_PIN_9
#define ADC_RST_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

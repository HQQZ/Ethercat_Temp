/**
  ******************************************************************************
  * @file           : ethercat_led.h
  * @author         : h
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef __ETHERCAT_LED_H__
#define __ETHERCAT_LED_H__
/* Includes ------------------------------------------------------------------*/
#include "gpio.h"


/* USER CODE BEGIN Private defines */
#define EtherCAT_RUN_LED_ON        digitalHi(EtherCAT_RUN_LED_GPIO_Port, EtherCAT_RUN_LED_Pin)
#define EtherCAT_RUN_LED_OFF       digitalLo(EtherCAT_RUN_LED_GPIO_Port, EtherCAT_RUN_LED_Pin)
#define EtherCAT_RUN_LED_TOGGLE    digitalToggle(EtherCAT_RUN_LED_GPIO_Port, EtherCAT_RUN_LED_Pin)

#define EtherCAT_ERR_LED_ON        digitalHi(EtherCAT_ERR_LED_GPIO_Port, EtherCAT_ERR_LED_Pin)
#define EtherCAT_ERR_LED_OFF       digitalLo(EtherCAT_ERR_LED_GPIO_Port, EtherCAT_ERR_LED_Pin)
#define EtherCAT_ERR_LED_TOGGLE    digitalToggle(EtherCAT_ERR_LED_GPIO_Port, EtherCAT_ERR_LED_Pin)
/* USER CODE END Private defines */

#endif /* __ETHERCAT_LED_H__ */


/**
  ******************************************************************************
  * File Name          : freertos.h
  * Description        : Code for freertos applications
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
 
 
 #ifndef __FREERTOS_H__
 #define __FREERTOS_H__
 
 
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "semphr.h"
#include "queue.h"

extern SemaphoreHandle_t ADC_Semaphore;
extern SemaphoreHandle_t TIM_Semaphore;
extern QueueHandle_t ADsample_Queue;
 
 
 #endif /* __FREERTOS_H__ */
 


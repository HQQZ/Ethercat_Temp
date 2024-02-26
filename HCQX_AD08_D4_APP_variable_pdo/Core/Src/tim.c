/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
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
/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_adc.h"
/* USER CODE END 0 */

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim13;
TIM_HandleTypeDef htim14;

/* TIM6 init function */
void MX_TIM6_Init(void)
{

    /* USER CODE BEGIN TIM6_Init 0 */

    /* USER CODE END TIM6_Init 0 */

    TIM_MasterConfigTypeDef sMasterConfig = {0};

    /* USER CODE BEGIN TIM6_Init 1 */

    /* USER CODE END TIM6_Init 1 */
    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 839;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 99;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM6_Init 2 */

    /* USER CODE END TIM6_Init 2 */

}
/* TIM13 init function */
void MX_TIM13_Init(void)
{

    /* USER CODE BEGIN TIM13_Init 0 */

    /* USER CODE END TIM13_Init 0 */

    /* USER CODE BEGIN TIM13_Init 1 */

    /* USER CODE END TIM13_Init 1 */
    htim13.Instance = TIM13;
    htim13.Init.Prescaler = 83;
    htim13.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim13.Init.Period = 249;
    htim13.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim13.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim13) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM13_Init 2 */
	__HAL_TIM_SET_COUNTER(&htim13, 0);
    HAL_TIM_Base_Stop_IT(&htim13);
    /* USER CODE END TIM13_Init 2 */

}
/* TIM14 init function */
void MX_TIM14_Init(void)
{

    /* USER CODE BEGIN TIM14_Init 0 */

    /* USER CODE END TIM14_Init 0 */

    /* USER CODE BEGIN TIM14_Init 1 */

    /* USER CODE END TIM14_Init 1 */
    htim14.Instance = TIM14;
    htim14.Init.Prescaler = 83;
    htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim14.Init.Period = 249;
    htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM14_Init 2 */
	__HAL_TIM_SET_COUNTER(&htim14, 0);
    HAL_TIM_Base_Stop_IT(&htim14);
    /* USER CODE END TIM14_Init 2 */

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

    if(tim_baseHandle->Instance==TIM6)
    {
        /* USER CODE BEGIN TIM6_MspInit 0 */

        /* USER CODE END TIM6_MspInit 0 */
        /* TIM6 clock enable */
        __HAL_RCC_TIM6_CLK_ENABLE();

        /* TIM6 interrupt Init */
        HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
        /* USER CODE BEGIN TIM6_MspInit 1 */

        /* USER CODE END TIM6_MspInit 1 */
    }
    else if(tim_baseHandle->Instance==TIM13)
    {
        /* USER CODE BEGIN TIM13_MspInit 0 */

        /* USER CODE END TIM13_MspInit 0 */
        /* TIM13 clock enable */
        __HAL_RCC_TIM13_CLK_ENABLE();

        /* TIM13 interrupt Init */
        HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 7, 0);
        HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
        /* USER CODE BEGIN TIM13_MspInit 1 */

        /* USER CODE END TIM13_MspInit 1 */
    }
    else if(tim_baseHandle->Instance==TIM14)
    {
        /* USER CODE BEGIN TIM14_MspInit 0 */

        /* USER CODE END TIM14_MspInit 0 */
        /* TIM14 clock enable */
        __HAL_RCC_TIM14_CLK_ENABLE();

        /* TIM14 interrupt Init */
        HAL_NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, 7, 0);
        HAL_NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn);
        /* USER CODE BEGIN TIM14_MspInit 1 */

        /* USER CODE END TIM14_MspInit 1 */
    }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

    if(tim_baseHandle->Instance==TIM6)
    {
        /* USER CODE BEGIN TIM6_MspDeInit 0 */

        /* USER CODE END TIM6_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM6_CLK_DISABLE();

        /* TIM6 interrupt Deinit */
        HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
        /* USER CODE BEGIN TIM6_MspDeInit 1 */

        /* USER CODE END TIM6_MspDeInit 1 */
    }
    else if(tim_baseHandle->Instance==TIM13)
    {
        /* USER CODE BEGIN TIM13_MspDeInit 0 */

        /* USER CODE END TIM13_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM13_CLK_DISABLE();

        /* TIM13 interrupt Deinit */
        HAL_NVIC_DisableIRQ(TIM8_UP_TIM13_IRQn);
        /* USER CODE BEGIN TIM13_MspDeInit 1 */

        /* USER CODE END TIM13_MspDeInit 1 */
    }
    else if(tim_baseHandle->Instance==TIM14)
    {
        /* USER CODE BEGIN TIM14_MspDeInit 0 */

        /* USER CODE END TIM14_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM14_CLK_DISABLE();

        /* TIM14 interrupt Deinit */
        HAL_NVIC_DisableIRQ(TIM8_TRG_COM_TIM14_IRQn);
        /* USER CODE BEGIN TIM14_MspDeInit 1 */

        /* USER CODE END TIM14_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */
/**
 * @brief  设置定时器重装载寄存器并启动定时中断
 * @param  htim:定时器x指针
 * @param  tims:定时时间，单位us
 * @retval none
 */
void set_enable_timer_updata(TIM_HandleTypeDef *htim, uint16_t tims)
{
    HAL_TIM_Base_Stop_IT(htim);
    /* 清除定时器溢出标志 */
    htim->Instance->SR = ~(TIM_IT_UPDATE);

    /* 清除计数器 */
    __HAL_TIM_SET_COUNTER(htim, 0);

    /* 置位URS */
    htim->Instance->CR1 |= 0x0004;

    /* 更改重装载值 */
    if (tims > 0xffff)
        tims = 0xffff;
    //__HAL_TIM_SET_AUTORELOAD(htim, reload_time);
    htim->Instance->ARR = tims;

    /* 置位UG */
    htim->Instance->EGR |= 0x0001;

    /* 清除URS和UG */
    htim->Instance->CR1 &= ~0x0004;
    htim->Instance->EGR &= ~0x0001;

    /* 启动定时中断 */
    HAL_TIM_Base_Start_IT(htim);
}

/**
 * @brief  清零定时器计数器并关闭定时中断
 * @param  htim:定时器x指针
 * @retval none
 */
void stop_timer(TIM_HandleTypeDef *htim)
{
    __HAL_TIM_SET_COUNTER(htim, 0);
    HAL_TIM_Base_Stop_IT(htim);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    uint32_t ulReturn;
    BaseType_t pxHigherPriorityTaskWoken;
	
    ulReturn = taskENTER_CRITICAL_FROM_ISR();
	
    if (htim->Instance == TIM7) {
        HAL_IncTick();
//		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
    }
	else if(htim->Instance == TIM13)
	{
		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
	}
    
	 taskEXIT_CRITICAL_FROM_ISR(ulReturn);
}

/* USER CODE END 1 */

/**
 * @file app_adc.c
 * @author h
 * @brief adc应用层
 * @version 0.1
 * @date 2023-08-04
 *
 * @copyright Copyright (c) 2023
 *
 */
 
#include "app_adc.h"
#include "tim.h"
#include "app_freertos.h"

static uint16_t ad_sample_buf[8] = {0};
static float ad_convert_buf[8] = {0};

static KFP KFP_ADC[8]= {{0.02,0,0,0,0.001,0.005},
						 {0.02,0,0,0,0.001,0.005},
						 {0.02,0,0,0,0.001,0.005},
						 {0.02,0,0,0,0.001,0.005},
						 {0.02,0,0,0,0.001,0.005},
						 {0.02,0,0,0,0.001,0.005},
						 {0.02,0,0,0,0.001,0.005},
						 {0.02,0,0,0,0.001,0.005}};

static ad_sample_t ad_sample_queue[8] = {0};
static ad_sample_t ad_sample_queue_receive;

static float kalmanFilter(KFP *kfp,float input)
{
	//预测协方差方程：k时刻系统估算协方差 = k-1时刻的系统协方差 + 过程噪声协方差
	kfp->Now_P = kfp->LastP + kfp->Q;
	//卡尔曼增益方程：卡尔曼增益 = k时刻系统估算协方差 / （k时刻系统估算协方差 + 观测噪声协方差）
	kfp->Kg = kfp->Now_P / (kfp->Now_P + kfp->R);
	//更新最优值方程：k时刻状态变量的最优值 = 状态变量的预测值 + 卡尔曼增益 * （测量值 - 状态变量的预测值）
	kfp->out = kfp->out + kfp->Kg * (input -kfp->out);//因为这一次的预测值就是上一次的输出值
	//更新协方差方程: 本次的系统协方差付给 kfp->LastP 威下一次运算准备。
	kfp->LastP = (1-kfp->Kg) * kfp->Now_P;
	return kfp->out;
}

void adc_sw_ch_delay_over(void)
{
	BaseType_t pxHigherPriorityTaskWoken;
	stop_timer(&htim13);
	xSemaphoreGiveFromISR(ADC_Semaphore, &pxHigherPriorityTaskWoken);
}

void ad_sample(void)
{
	for(int i=0; i<8; i++)
	{
		sgm51652hx_SignalChannelRead(&sgm5162h8, i, 1, &ad_sample_buf[i]);
		set_enable_timer_updata(&htim13, 125);
		ad_sample_queue[i].ch = i;
		ad_sample_queue[i].ad_sample_value = ad_sample_buf[i];
		xQueueSend(ADsample_Queue,&ad_sample_queue[i], (TickType_t)10);
		xSemaphoreTake(ADC_Semaphore, portMAX_DELAY);
	}
}

void ad_convert(void)
{
	float ad_convert = 0;
	
	if(pdTRUE == xQueueReceive(ADsample_Queue, &ad_sample_queue_receive, portMAX_DELAY))
	{
		ad_convert_buf[ad_sample_queue_receive.ch] = 
		kalmanFilter(&KFP_ADC[ad_sample_queue_receive.ch], ad_sample_queue_receive.ad_sample_value * 312.5f/1000);
	}
}

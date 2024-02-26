/**
 * @file app_adc.c
 * @author h
 * @brief adcӦ�ò�
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
	//Ԥ��Э����̣�kʱ��ϵͳ����Э���� = k-1ʱ�̵�ϵͳЭ���� + ��������Э����
	kfp->Now_P = kfp->LastP + kfp->Q;
	//���������淽�̣����������� = kʱ��ϵͳ����Э���� / ��kʱ��ϵͳ����Э���� + �۲�����Э���
	kfp->Kg = kfp->Now_P / (kfp->Now_P + kfp->R);
	//��������ֵ���̣�kʱ��״̬����������ֵ = ״̬������Ԥ��ֵ + ���������� * ������ֵ - ״̬������Ԥ��ֵ��
	kfp->out = kfp->out + kfp->Kg * (input -kfp->out);//��Ϊ��һ�ε�Ԥ��ֵ������һ�ε����ֵ
	//����Э�����: ���ε�ϵͳЭ����� kfp->LastP ����һ������׼����
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

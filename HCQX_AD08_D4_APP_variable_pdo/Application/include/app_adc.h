/**
 * @file app_adc.h
 * @author h
 * @brief adcӦ�ò�ͷ�ļ�
 * @version 0.1
 * @date 2023-08-04
 *
 * @copyright Copyright (c) 2023
 *
 */
 
 #ifndef __APP_ADC_H__
 #define __APP_ADC_H__
 
 #include "sgm51652hx.h"
 
 typedef struct 
{
    float LastP;//�ϴι���Э���� ��ʼ��ֵΪ0.02
    float Now_P;//��ǰ����Э���� ��ʼ��ֵΪ0
    float out;//�������˲������ ��ʼ��ֵΪ0
    float Kg;//���������� ��ʼ��ֵΪ0
    float Q;//��������Э���� ��ʼ��ֵΪ0.001
    float R;//�۲�����Э���� ��ʼ��ֵΪ0.005
}KFP;

typedef struct
{
	uint8_t ch;
	uint16_t ad_sample_value;
}ad_sample_t;
 
void adc_sw_ch_delay_over(void);
void ad_sample(void);
void ad_convert(void);
 
 #endif /* __APP_ADC_H__ */


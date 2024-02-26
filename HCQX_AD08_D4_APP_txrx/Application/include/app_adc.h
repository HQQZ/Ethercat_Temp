/**
 * @file app_adc.h
 * @author h
 * @brief adc应用层头文件
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
    float LastP;//上次估算协方差 初始化值为0.02
    float Now_P;//当前估算协方差 初始化值为0
    float out;//卡尔曼滤波器输出 初始化值为0
    float Kg;//卡尔曼增益 初始化值为0
    float Q;//过程噪声协方差 初始化值为0.001
    float R;//观测噪声协方差 初始化值为0.005
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


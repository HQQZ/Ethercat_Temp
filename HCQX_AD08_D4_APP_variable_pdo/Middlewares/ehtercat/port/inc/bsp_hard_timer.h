/*
*********************************************************************************************************
*
*	ģ������ : Ӳ����ʱ��ģ��
*	�ļ����� : bsp_hard_timer.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*
*
*********************************************************************************************************
*/

#ifndef __BSP_HARD_TIMER_H
#define __BSP_HARD_TIMER_H


/* �ṩ������C�ļ����õĺ��� */
void bsp_InitHardTimer(void);


void bsp_InitSingleTimer(void);
void bsp_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void * _pCallBack);
#endif


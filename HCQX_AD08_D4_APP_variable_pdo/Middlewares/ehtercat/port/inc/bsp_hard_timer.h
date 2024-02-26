/*
*********************************************************************************************************
*
*	模块名称 : 硬件定时器模块
*	文件名称 : bsp_hard_timer.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*
*********************************************************************************************************
*/

#ifndef __BSP_HARD_TIMER_H
#define __BSP_HARD_TIMER_H


/* 提供给其他C文件调用的函数 */
void bsp_InitHardTimer(void);


void bsp_InitSingleTimer(void);
void bsp_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void * _pCallBack);
#endif


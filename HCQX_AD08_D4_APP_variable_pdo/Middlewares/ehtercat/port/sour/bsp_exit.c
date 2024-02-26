/*
*********************************************************************************************************
*
*	模块名称 : 外部中断模块
*	文件名称 : bsp_exit.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期       作者    说明
*
*
*********************************************************************************************************
*/


#include "bsp_exit.h"
#include "stm32f4xx.h"


/* 
引脚定义：
IRQ   :PE2
SYNC0 :PE4
SYNC1 :PE5
*/

/*
*********************************************************************************************************
*	函 数 名: IRQ_EXTI_Configuration
*	功能说明: 配置外部中断引脚 IRQ中断.
*				PE2 ：下降沿触发
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void IRQ_EXTI_Configuration(void)
{

	EXTI_InitTypeDef   EXTI_InitStructure;
	GPIO_InitTypeDef   GPIO_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

	/* Enable GPIOE clock */
    RCC_AHB1PeriphClockCmd(IRQ_RCC, ENABLE); 
	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Configure  pin as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd =GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = IRQ_Pinx;
	GPIO_Init(IRQ_GPIOx, &GPIO_InitStructure);

	/* Connect EXTI Line0 to  pin */
	SYSCFG_EXTILineConfig(IRQ_EXTI_PortSource, IRQ_EXTI_PinSource);

	/* Configure EXTI Line0 */
	EXTI_InitStructure.EXTI_Line = IRQ_EXTI_Line;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI Line0 Interrupt to the lowest priority */
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  				
	NVIC_InitStructure.NVIC_IRQChannel = IRQ_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}
/*
*********************************************************************************************************
*	函 数 名: SYNC0_EXTI_Configuration
*	功能说明: 配置外部中断引脚 SYNC0中断.
*				PE4 ：下降沿触发
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void SYNC0_EXTI_Configuration(void)
{
    EXTI_InitTypeDef   EXTI_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

	/* Enable GPIOE clock */
    RCC_AHB1PeriphClockCmd(SYNC0_RCC, ENABLE);  
    /* Enable SYSCFG clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Configure PC3 pin as input floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = SYNC0_Pinx;
    GPIO_Init(SYNC0_GPIOx, &GPIO_InitStructure);

    /* Connect EXTI Line3 to PC3 pin */
    SYSCFG_EXTILineConfig(SYNC0_EXTI_PortSource, SYNC0_EXTI_PinSource);

    /* Configure EXTI Line3 */
    EXTI_InitStructure.EXTI_Line = SYNC0_EXTI_Line;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//EXTI_Trigger_Risingz/;  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI Line3 Interrupt to the lowest priority */
//    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  				
    NVIC_InitStructure.NVIC_IRQChannel = SYNC0_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: SYNC1_EXTI_Configuration
*	功能说明: 配置外部中断引脚 SYNC1中断.
*				PE5 ：下降沿触发
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void SYNC1_EXTI_Configuration(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
	GPIO_InitTypeDef   GPIO_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

	/* Enable GPIOC clock */
	RCC_AHB1PeriphClockCmd(SYNC1_RCC, ENABLE);
	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Configure PC1 pin as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = SYNC1_Pinx;
	GPIO_Init(SYNC1_GPIOx, &GPIO_InitStructure);

	/* Connect EXTI Line1 to PC1 pin */
	SYSCFG_EXTILineConfig(SYNC1_EXTI_PortSource, SYNC1_EXTI_PinSource);

	/* Configure EXTI 1*/
	EXTI_InitStructure.EXTI_Line = SYNC1_EXTI_Line;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI Line1 Interrupt to the lowest priority */
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  				
	NVIC_InitStructure.NVIC_IRQChannel = SYNC1_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

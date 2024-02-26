/*
*********************************************************************************************************
*
*	ģ������ : �ⲿ�ж�ģ��
*	�ļ����� : bsp_exit.c
*	��    �� : V1.0
*	˵    �� : 
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*
*
*********************************************************************************************************
*/


#include "bsp_exit.h"
#include "stm32f4xx.h"


/* 
���Ŷ��壺
IRQ   :PE2
SYNC0 :PE4
SYNC1 :PE5
*/

/*
*********************************************************************************************************
*	�� �� ��: IRQ_EXTI_Configuration
*	����˵��: �����ⲿ�ж����� IRQ�ж�.
*				PE2 ���½��ش���
*	��    �Σ���
*	�� �� ֵ: ��
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
*	�� �� ��: SYNC0_EXTI_Configuration
*	����˵��: �����ⲿ�ж����� SYNC0�ж�.
*				PE4 ���½��ش���
*	��    �Σ���
*	�� �� ֵ: ��
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
*	�� �� ��: SYNC1_EXTI_Configuration
*	����˵��: �����ⲿ�ж����� SYNC1�ж�.
*				PE5 ���½��ش���
*	��    �Σ���
*	�� �� ֵ: ��
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

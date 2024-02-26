#ifndef __EXIT_H__
#define __EXIT_H__

/*  IRQÒý½Å PE2 */
#define IRQ_GPIOx                     GPIOE
#define IRQ_Pinx                      GPIO_Pin_2
#define IRQ_RCC                       RCC_AHB1Periph_GPIOE
#define IRQ_EXTI_PortSource           EXTI_PortSourceGPIOE
#define IRQ_EXTI_PinSource            EXTI_PinSource2
#define IRQ_EXTI_Line                 EXTI_Line2
#define IRQ_IRQChannel                EXTI2_IRQn
#define IRQ_EXTI_IRQHandler           EXTI2_IRQHandler

/*  SYNC0Òý½Å PE4 */
#define SYNC0_GPIOx                   GPIOE
#define SYNC0_Pinx                    GPIO_Pin_4
#define SYNC0_RCC                     RCC_AHB1Periph_GPIOE
#define SYNC0_EXTI_PortSource         EXTI_PortSourceGPIOE
#define SYNC0_EXTI_PinSource          EXTI_PinSource4
#define SYNC0_EXTI_Line               EXTI_Line4
#define SYNC0_IRQChannel              EXTI4_IRQn
#define SYNC0_EXTI_IRQHandler         EXTI4_IRQHandler

/*  SYNC1Òý½Å PE5 */
#define SYNC1_GPIOx                  GPIOE
#define SYNC1_Pinx                   GPIO_Pin_5
#define SYNC1_RCC                     RCC_AHB1Periph_GPIOE
#define SYNC1_EXTI_PortSource        EXTI_PortSourceGPIOE
#define SYNC1_EXTI_PinSource         EXTI_PinSource5
#define SYNC1_EXTI_Line              EXTI_Line5
#define SYNC1_IRQChannel             EXTI9_5_IRQn
#define SYNC1_EXTI_IRQHandler        EXTI9_5_IRQHandler



void IRQ_EXTI_Configuration(void);
void SYNC0_EXTI_Configuration(void);
void SYNC1_EXTI_Configuration(void);
#endif

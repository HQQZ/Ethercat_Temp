
/*--------------------------------------------------------------------------------------
------
------    Includes
------
--------------------------------------------------------------------------------------*/
#include "ecat_def.h"
#if EL9800_HW
#include "ecatslv.h"

#define    _EL9800HW_ 1
#include "el9800hw.h"
#undef    _EL9800HW_
/* ECATCHANGE_START(V5.11) ECAT10*/
/*remove definition of _EL9800HW_ (#ifdef is used in el9800hw.h)*/
/* ECATCHANGE_END(V5.11) ECAT10*/

#include "ecatappl.h"

void mem_test(void);
/*--------------------------------------------------------------------------------------
------
------    internal Types and Defines
------
--------------------------------------------------------------------------------------*/

typedef union
{
    unsigned short    Word;
    unsigned char    Byte[2];
} UBYTETOWORD;

typedef union 
{
    UINT8           Byte[2];
    UINT16          Word;
}
UALEVENT;

volatile uint16_t PdiIsrDogCount=0;
/*-----------------------------------------------------------------------------------------
------
------    SPI defines/macros
------
-----------------------------------------------------------------------------------------*/
#define SPI_DEACTIVE                      1
#define SPI_ACTIVE                        0


#if INTERRUPTS_SUPPORTED
/*-----------------------------------------------------------------------------------------
------
------    Global Interrupt setting
------
-----------------------------------------------------------------------------------------*/

#define    DISABLE_GLOBAL_INT                  __disable_irq()					
#define    ENABLE_GLOBAL_INT                   __enable_irq()				
#define    DISABLE_AL_EVENT_INT                DISABLE_GLOBAL_INT
#define    ENABLE_AL_EVENT_INT                 ENABLE_GLOBAL_INT


/*-----------------------------------------------------------------------------------------
------
------    ESC Interrupt
------
-----------------------------------------------------------------------------------------*/
#if AL_EVENT_ENABLED
//	#define    ESC_INT_STATE                  EXTI_GetITStatus(IRQ_EXTI_Line)
	#define    INIT_ESC_INT                   {HAL_NVIC_EnableIRQ(EtherCAT_IRQ_EXTI_IRQn);}					
	#define    EcatIsr                        EtherCAT_IRQ_EXTI_Callback
//	#define    ACK_ESC_INT         	          EXTI_ClearITPendingBit(IRQ_EXTI_Line);  
	#define    IS_ESC_INT_ACTIVE		
#endif //#if AL_EVENT_ENABLED

/*-----------------------------------------------------------------------------------------
------
------    SYNC0 Interrupt
------
-----------------------------------------------------------------------------------------*/
#if DC_SUPPORTED && _STM32_IO8
//	#define     SYNC0_INT_STATE                EXTI_GetITStatus(SYNC0_EXTI_Line)
	#define     INIT_SYNC0_INT                 {HAL_NVIC_EnableIRQ(EtherCAT_SYNC0_EXTI_IRQn);}			
	#define     Sync0Isr                       EtherCAT_SYNC0_EXTI_Callback 
	#define     DISABLE_SYNC0_INT              //NVIC_DisableIRQ(SYNC0_IRQChannel);	 
	#define     ENABLE_SYNC0_INT               //NVIC_EnableIRQ(SYNC0_IRQChannel);	
//	#define     ACK_SYNC0_INT                  EXTI_ClearITPendingBit(SYNC0_EXTI_Line);
	#define     IS_SYNC0_INT_ACTIVE             

																						
	/*ECATCHANGE_START(V5.10) HW3*/
//	#define     SYNC1_INT_STATE                EXTI_GetITStatus(SYNC1_EXTI_Line)
	#define    INIT_SYNC1_INT                  {HAL_NVIC_EnableIRQ(EtherCAT_SYNC1_EXTI_IRQn);}	
	#define    Sync1Isr                        EtherCAT_SYNC1_EXTI_Callback 
	#define    DISABLE_SYNC1_INT               //NVIC_DisableIRQ(SYNC1_IRQChannel);
	#define    ENABLE_SYNC1_INT                //NVIC_EnableIRQ(SYNC1_IRQChannel); 
//	#define    ACK_SYNC1_INT                   EXTI_ClearITPendingBit(SYNC1_EXTI_Line);
	#define    IS_SYNC1_INT_ACTIVE              

	/*ECATCHANGE_END(V5.10) HW3*/

#endif //#if DC_SUPPORTED && _STM32_IO8

#endif	//#if INTERRUPTS_SUPPORTED
/*-----------------------------------------------------------------------------------------
------
------    Hardware timer
------
-----------------------------------------------------------------------------------------*/
#if _STM32_IO8
#if ECAT_TIMER_INT

//	#define TIM                                   TIM6
//	#define ECAT_TIMER_INT_STATE                  TIM_GetITStatus(TIM, TIM_IT_Update)
//	#define ECAT_TIMER_ACK_INT                    TIM_ClearITPendingBit(TIM , TIM_FLAG_Update);//#define ECAT_TIMER_ACK_INT           TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);	

	#define TimerIsr                              EtherCAT_TIM_ISR_Callback					
	#define ENABLE_ECAT_TIMER_INT                 {HAL_TIM_Base_Start_IT(&EtherCAT_TIM_Handle);}	
	#define DISABLE_ECAT_TIMER_INT                //NVIC_DisableIRQ(TIM6_DAC_IRQn) ;

	#define INIT_ECAT_TIMER                       {EtherCAT_TIMX_Init();}

	#define STOP_ECAT_TIMER                       DISABLE_ECAT_TIMER_INT;/*disable timer interrupt*/
	#define START_ECAT_TIMER          	          ENABLE_ECAT_TIMER_INT;
#else    //#if ECAT_TIMER_INT
	#define INIT_ECAT_TIMER      				  TIM_Configuration(10);

	#define STOP_ECAT_TIMER                       TIM_Cmd(TIM2, DISABLE);		 

	#define START_ECAT_TIMER                      TIM_Cmd(TIM2, ENABLE);   			
#endif //#else #if ECAT_TIMER_INT

#elif _STM32_IO4

	#if !ECAT_TIMER_INT
	#define ENABLE_ECAT_TIMER_INT                 NVIC_EnableIRQ(TIM2_IRQn) ;	
	#define DISABLE_ECAT_TIMER_INT                NVIC_DisableIRQ(TIM2_IRQn) ;
	#define INIT_ECAT_TIMER                       TIM_Configuration(10) ;	
	#define STOP_ECAT_TIMER              	      TIM_Cmd(TIM2, DISABLE);	
	#define START_ECAT_TIMER           		      TIM_Cmd(TIM2, ENABLE);			

#else    //#if !ECAT_TIMER_INT

	#warning "define Timer Interrupt Macros"

#endif //#else #if !ECAT_TIMER_INT
#endif //#elif _STM32_IO4

/*-----------------------------------------------------------------------------------------
------
------    Configuration Bits
------
-----------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
------
------    LED defines
------
-----------------------------------------------------------------------------------------*/
#if _STM32_IO8
// EtherCAT Status LEDs -> StateMachine
#define ETHERCAT_PDI_ISR_LED_TOGGLE()             EtherCAT_RUN_LED_TOGGLE
#define ETHERCAT_RUN_LED_ON()                     EtherCAT_RUN_LED_ON
#define ETHERCAT_RUN_LED_OFF()                    EtherCAT_RUN_LED_OFF
#define ETHERCAT_ERR_LED_ON()                     EtherCAT_ERR_LED_ON
#define ETHERCAT_ERR_LED_OFF()                    EtherCAT_ERR_LED_OFF
#endif //_STM32_IO8


/*--------------------------------------------------------------------------------------
------
------    internal Variables
------
--------------------------------------------------------------------------------------*/
UALEVENT         EscALEvent;            //contains the content of the ALEvent register (0x220), this variable is updated on each Access to the Esc

/*--------------------------------------------------------------------------------------
------
------    internal functions
------
--------------------------------------------------------------------------------------*/


/*******************************************************************************
  Function:
    void GetInterruptRegister(void)

  Summary:
    The function operates a SPI access without addressing.

  Description:
    The first two bytes of an access to the EtherCAT ASIC always deliver the AL_Event register (0x220).
    It will be saved in the global "EscALEvent"
  *****************************************************************************/
static void GetInterruptRegister(void)
{
    DISABLE_AL_EVENT_INT;
    HW_EscReadIsr((MEM_ADDR *)&EscALEvent.Word, 0x220, 2);
    ENABLE_AL_EVENT_INT;
}


/*******************************************************************************
  Function:
    void ISR_GetInterruptRegister(void)

  Summary:
    The function operates a SPI access without addressing.
        Shall be implemented if interrupts are supported else this function is equal to "GetInterruptRegsiter()"

  Description:
    The first two bytes of an access to the EtherCAT ASIC always deliver the AL_Event register (0x220).
        It will be saved in the global "EscALEvent"
  *****************************************************************************/

static void ISR_GetInterruptRegister(void)
{
    HW_EscReadIsr((MEM_ADDR *)&EscALEvent.Word, 0x220, 2);
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\return     0 if initialization was successful

 \brief    This function intialize the Process Data Interface (PDI) and the host controller.
*////////////////////////////////////////////////////////////////////////////////////////
UINT8 HW_Init(void)
{
	const uint32_t MaxOutTime=2000000;
	uint32_t OutTimes=0;
	UINT16 intMask;
	
	do
	{
//        IWDG_Feed();
        intMask = 0x0093;
        HW_EscWriteWord(intMask, ESC_AL_EVENTMASK_OFFSET);
        intMask = 0;
        HW_EscReadWord(intMask, ESC_AL_EVENTMASK_OFFSET);
		OutTimes++;
	} while ((intMask!= 0x0093)&&(OutTimes<MaxOutTime));
    
    HW_EscReadWord(intMask, 0x0110);//isEEPROM_LOADED
    
	OutTimes=0;
    do
    {
        HW_EscReadWord(intMask, 0x0110);//isEEPROM_LOADED
		OutTimes++;
    }while(((intMask&0x0001) == 0) && (OutTimes<MaxOutTime) );
    HW_EscReadWord(intMask, 0x0110);//isEEPROM_LOADED
    
	OutTimes=0;
	do
	{
//        IWDG_Feed();
        intMask = 0x0093;
        HW_EscWriteWord(intMask, ESC_AL_EVENTMASK_OFFSET);
        intMask = 0;
        HW_EscReadWord(intMask, ESC_AL_EVENTMASK_OFFSET);
		OutTimes++;
	} while ((intMask!= 0x0093) && (OutTimes<MaxOutTime) );
	intMask = 0x00;
	  
    HW_EscWriteWord(intMask, ESC_AL_EVENTMASK_OFFSET);

#if AL_EVENT_ENABLED
    INIT_ESC_INT;
//    ENABLE_ESC_INT();
#endif

#if DC_SUPPORTED&& _STM32_IO8
//    INIT_SYNC0_INT
//    INIT_SYNC1_INT

//    ENABLE_SYNC0_INT;
//    ENABLE_SYNC1_INT;
#endif

#if ECAT_TIMER_INT
    INIT_ECAT_TIMER;
    START_ECAT_TIMER;
#endif

#if INTERRUPTS_SUPPORTED
    /* enable all interrupts */
    ENABLE_GLOBAL_INT;
#endif

    return 0;
}



/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief    This function shall be implemented if hardware resources need to be release
        when the sample application stops
*////////////////////////////////////////////////////////////////////////////////////////
void HW_Release(void)
{

}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    first two Bytes of ALEvent register (0x220)

 \brief  This function gets the current content of ALEvent register
*////////////////////////////////////////////////////////////////////////////////////////
UINT16 HW_GetALEventRegister(void)
{
    GetInterruptRegister();
    return EscALEvent.Word;
}
#if INTERRUPTS_SUPPORTED
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    first two Bytes of ALEvent register (0x220)

 \brief  The SPI PDI requires an extra ESC read access functions from interrupts service routines.
        The behaviour is equal to "HW_GetALEventRegister()"
*////////////////////////////////////////////////////////////////////////////////////////
#if _STM32_IO4  && AL_EVENT_ENABLED
/* the pragma interrupt_level is used to tell the compiler that these functions will not
   be called at the same time from the main function and the interrupt routine */
//#pragma interrupt_level 1
#endif
UINT16 HW_GetALEventRegister_Isr(void)
{
     ISR_GetInterruptRegister();
    return EscALEvent.Word;
}
#endif


#if UC_SET_ECAT_LED
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param RunLed            desired EtherCAT Run led state
 \param ErrLed            desired EtherCAT Error led state

  \brief    This function updates the EtherCAT run and error led
*////////////////////////////////////////////////////////////////////////////////////////
void HW_SetLed(UINT8 RunLed,UINT8 ErrLed)
{
#if _STM32_IO8
    if(RunLed)
	{	
//        ETHERCAT_RUN_LED_ON();
	}	
    else
	{	
//        ETHERCAT_RUN_LED_OFF();
	}	
    
    if(ErrLed)
	{	
        //ETHERCAT_ERR_LED_ON();
	}	
    else
	{	
//        ETHERCAT_ERR_LED_OFF();
	}	
#endif
}
#endif //#if UC_SET_ECAT_LED
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param pData        Pointer to a byte array which holds data to write or saves read data.
 \param Address     EtherCAT ASIC address ( upper limit is 0x1FFF )    for access.
 \param Len            Access size in Bytes.

 \brief  This function operates the SPI read access to the EtherCAT ASIC.
*////////////////////////////////////////////////////////////////////////////////////////
void HW_EscRead( MEM_ADDR *pData, UINT16 Address, UINT16 Len )
{
    /* HBu 24.01.06: if the SPI will be read by an interrupt routine too the
                     mailbox reading may be interrupted but an interrupted
                     reading will remain in a SPI transmission fault that will
                     reset the internal Sync Manager status. Therefore the reading
                     will be divided in 1-byte reads with disabled interrupt */
    UINT16 i;
    UINT8 *pTmpData = (UINT8 *)pData;

    /* loop for all bytes to be read */
    while ( Len > 0 )
    {
        if (Address >= 0x1000)
        {
            i = Len;
        }
        else
        {
            i= (Len > 4) ? 4 : Len;//这段代码适用于LAN9252

            if(Address & 01)
            {
               i=1;
            }
            else if (Address & 02)
            {
               i= (i&1) ? 1:2;
            }
            else if (i == 03)
            {
                i=1;
            }
        }

        DISABLE_AL_EVENT_INT;

        #if USE_PDI == 0x08
        PMPReadDRegister(pTmpData, Address, i);
        #else
        SPIReadDRegister(pTmpData, Address, i);
        #endif

        ENABLE_AL_EVENT_INT;

        Len -= i;
        pTmpData += i;
        Address += i;
    }


}
#if INTERRUPTS_SUPPORTED
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param pData        Pointer to a byte array which holds data to write or saves read data.
 \param Address     EtherCAT ASIC address ( upper limit is 0x1FFF )    for access.
 \param Len            Access size in Bytes.

\brief  The SPI PDI requires an extra ESC read access functions from interrupts service routines.
        The behaviour is equal to "HW_EscRead()"
*////////////////////////////////////////////////////////////////////////////////////////
#if _STM32_IO4  && AL_EVENT_ENABLED
/* the pragma interrupt_level is used to tell the compiler that these functions will not
   be called at the same time from the main function and the interrupt routine */
//#pragma interrupt_level 1
#endif
 void HW_EscReadIsr( MEM_ADDR *pData, UINT16 Address, UINT16 Len )
{

   UINT16 i;
   UINT8 *pTmpData = (UINT8 *)pData;

    /* send the address and command to the ESC */

    /* loop for all bytes to be read */
   while ( Len > 0 )
   {

        if (Address >= 0x1000)
        {
            i = Len;
        }
        else
        {
            i= (Len > 4) ? 4 : Len;

            if(Address & 01)
            {
               i=1;
            }
            else if (Address & 02)
            {
               i= (i&1) ? 1:2;
            }
            else if (i == 03)
            {
                i=1;
            }
        }

        #if USE_PDI == 0x08
        PMPReadDRegister(pTmpData, Address, i);
        #else
        SPIReadDRegister(pTmpData, Address, i);
        #endif

        Len -= i;
        pTmpData += i;
        Address += i;
    }
   
}
#endif //#if INTERRUPTS_SUPPORTED
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param pData        Pointer to a byte array which holds data to write or saves write data.
 \param Address     EtherCAT ASIC address ( upper limit is 0x1FFF )    for access.
 \param Len            Access size in Bytes.

  \brief  This function operates the SPI write access to the EtherCAT ASIC.
*////////////////////////////////////////////////////////////////////////////////////////
void HW_EscWrite( MEM_ADDR *pData, UINT16 Address, UINT16 Len )
{

    UINT16 i;
    UINT8 *pTmpData = (UINT8 *)pData;

    /* loop for all bytes to be written */
    while ( Len )
    {

        if (Address >= 0x1000)
        {
            i = Len;
        }
        else
        {
            i= (Len > 4) ? 4 : Len;

            if(Address & 01)
            {
               i=1;
            }
            else if (Address & 02)
            {
               i= (i&1) ? 1:2;
            }
            else if (i == 03)
            {
                i=1;
            }
        }

        DISABLE_AL_EVENT_INT;
       
        /* start transmission */
        #if USE_PDI == 0x08
        PMPWriteRegister(pTmpData, Address, i);
        #else
        SPIWriteRegister(pTmpData, Address, i);
        #endif


        ENABLE_AL_EVENT_INT;

       
   
        /* next address */
        Len -= i;
        pTmpData += i;
        Address += i;

    }

}
#if INTERRUPTS_SUPPORTED
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param pData        Pointer to a byte array which holds data to write or saves write data.
 \param Address     EtherCAT ASIC address ( upper limit is 0x1FFF )    for access.
 \param Len            Access size in Bytes.

 \brief  The SPI PDI requires an extra ESC write access functions from interrupts service routines.
        The behaviour is equal to "HW_EscWrite()"
*////////////////////////////////////////////////////////////////////////////////////////
void HW_EscWriteIsr( MEM_ADDR *pData, UINT16 Address, UINT16 Len )
{

    UINT16 i ;
    UINT8 *pTmpData = (UINT8 *)pData;

  
    /* loop for all bytes to be written */
    while ( Len )
    {

        if (Address >= 0x1000)
        {
            i = Len;
        }
        else
        {
            i= (Len > 4) ? 4 : Len;

            if(Address & 01)
            {
               i=1;
            }
            else if (Address & 02)
            {
               i= (i&1) ? 1:2;
            }
            else if (i == 03)
            {
                i=1;
            }
        }
        
       /* start transmission */

        #if USE_PDI == 0x08
        PMPWriteRegister(pTmpData, Address, i);
        #else
        SPIWriteRegister(pTmpData, Address, i);
        #endif
       
       /* next address */
        Len -= i;
        pTmpData += i;
        Address += i;
    }

}

#endif


/*
*********************************************************************************************************
*	函 数 名: EcatIsrFeed
*	功能说明: PDI中断通信检测
*	形    参:  无
*	返 回 值: 
*********************************************************************************************************
*/
uint8_t EcatIsrOnlineCheck(void)
{
    if(PdiIsrDogCount<1000)
	{
	    PdiIsrDogCount++;
	}	
	if(PdiIsrDogCount>1000)return 1;
	else return 0;
}
void EcatIsrFeed(void)
{
    PdiIsrDogCount=0;
}
/*
*********************************************************************************************************
*	函 数 名: EcatIsr
*	功能说明: PDI中断服务函数
*	形    参:  无
*	返 回 值: 
*********************************************************************************************************
*/
//uint32_t pidCount;
//uint32_t pidClear;
//uint8_t g_PDI_Flash=0;
void  EcatIsr(void)
{
//    if(ESC_INT_STATE != RESET)
//    {
//		if(pidClear>0)pidClear--;
//		pidCount++;
        PDI_Isr();
		EcatIsrFeed();
		//g_PDI_Flash++;
		
//        ACK_ESC_INT;
//    }
}
#endif     // AL_EVENT_ENABLED



#if DC_SUPPORTED&& _STM32_IO8/*
*********************************************************************************************************
*	函 数 名: Sync0Isr
*	功能说明:SYNC0中断服务函数 
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
uint32_t sync0Count;
void Sync0Isr(void)
{
//    if(SYNC0_INT_STATE != RESET)
//    {
//		pidClear++;
		sync0Count++;
        Sync0_Isr();
//        ACK_SYNC0_INT;
//    }
}
/*
*********************************************************************************************************
*	函 数 名: Sync1Isr
*	功能说明:SYNC1中断服务函数 
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void Sync1Isr(void)
{	
//    if(SYNC1_INT_STATE != RESET)
//    {
        Sync1_Isr();
//        ACK_SYNC1_INT;
//    }
}
/*ECATCHANGE_END(V5.10) HW3*/
#endif



/*
*********************************************************************************************************
*	函 数 名: TimerIsr
*	功能说明:SYNC1中断服务函数 
*	形    参:  timer中断服务函数---定时1ms---2000个ticks
*	返 回 值: 无
*********************************************************************************************************
*/
#if _STM32_IO8 && ECAT_TIMER_INT
uint32_t hardTime;
void TimerIsr(void)
{		
    //if(ECAT_TIMER_INT_STATE != RESET )//中断函数中已经判断并清零
    {
        
        
        ECAT_CheckTimer();
        hardTime++;
//        ECAT_TIMER_ACK_INT;
    }
}
#endif

//#endif //#if EL9800_HW














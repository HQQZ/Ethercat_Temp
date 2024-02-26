/* Includes ------------------------------------------------------------------*/
#include "FSMC.h"
#include "ecat_def.h"
#include "el9800hw.h"
#include "stm32f4xx.h"
#include "stm32f4xx_fsmc.h"
#include "stm32f4xx_rcc.h"

#define Bank1_SRAM1_ADDR    ((uint32_t)( 0x60000000))   //FSMC_NE1
#define Bank1_SRAM2_ADDR    ((uint32_t)( 0x64000000))   //FSMC_NE2
#define Bank1_SRAM3_ADDR    ((uint32_t)( 0x68000000))   //FSMC_NE3
#define Bank1_SRAM4_ADDR    ((uint32_t)( 0x6c000000))	//FSMC_NE4

#define HBI_SRAM_ADDR Bank1_SRAM1_ADDR
#define HBI_Bank_NORSRAM   FSMC_Bank1_NORSRAM1


/*
*********************************************************************************************************
*	函 数 名: FSMC_GPIO_init
*	功能说明:FSMC引脚初始化 
*	形    参:  无
*	返 回 值: 
*********************************************************************************************************
*/
void FSMC_GPIO_init(void)
{
    //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | 
                            RCC_AHB1Periph_GPIOB | 
                            RCC_AHB1Periph_GPIOC | 
                            RCC_AHB1Periph_GPIOD | 
                            RCC_AHB1Periph_GPIOE | 
                            RCC_AHB1Periph_GPIOF | 
                            RCC_AHB1Periph_GPIOG, ENABLE);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
	

    
    /*--FSMC GPIOs Configuration -----------------------------------------------------*/
    /*PD14 <-> HBI_D0
    PD15 <-> HBI_D1
    PD0  <-> HBI_D2
    PD1  <-> HBI_D3
    PE7  <-> HBI_D4
    PE8  <-> HBI_D5
    PE9  <-> HBI_D6
    PE10 <-> HBI_D7
    PE11 <-> HBI_D8
    PE12 <-> HBI_D9
    PE13 <-> HBI_D10
    PE14 <-> HBI_D11
    PE15 <-> HBI_D12
    PD8  <-> HBI_D13
    PD9  <-> HBI_D14
    PD10 <-> HBI_D15
    
    PF0  <-> HBI_A0
    PF1  <-> HBI_A1
    PF2  <-> HBI_A2
    PF3  <-> HBI_A3
    PF4  <-> HBI_A4
    PF5  <-> HBI_A5
    PF12 <-> HBI_A6
    PF13 <-> HBI_A7
    PF14 <-> HBI_A8
    PF15 <-> HBI_A9
    PG0  <-> HBI_A10
    PG1  <-> HBI_A11
    PG2  <-> HBI_A12
    PG3  <-> HBI_A13
    PG4  <-> HBI_A14
    PG5  <-> HBI_A15
    
    PD4  <-> HBI_RD,FSMC_NOE,
    PD5  <-> HBI_WR,FSMC_NWE,
    PE1  <-> HBI_BHE,FSMC_NBL1,
    PD6  <-> HBI_BUSY,FSMC_NWAIT
    PD7  <-> HBI_CS,FSMC_NE1
    */
    
    /* GPIOG configuration */
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource7 , GPIO_AF_FSMC);//CS  FSMC_NE1 PD7
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 ;  
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    //  GPIO_ResetBits(GPIOG, GPIO_Pin_12);
    
    /* GPIOD configuration */
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);//RD NOE PD4
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);//WR NWE PD5
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_FSMC);//BUSY NWAIT PD6
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC);//CS  FSMC_NE1 PD7
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_4  | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |
                                  GPIO_Pin_8  | GPIO_Pin_9  |  GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* GPIOE configuration */
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource1 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FSMC);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_7 | GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11|
                                    GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

//    /* GPIOB configuration */
//    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_FSMC);//NADV PB7
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
//    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*地址总线的GPIO初始化*/
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource0 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource1 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource2 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource3 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource4 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource5 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource12 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource13 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource14 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource15 , GPIO_AF_FSMC);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3 |
                                    GPIO_Pin_4  | GPIO_Pin_5  |  GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14| GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOG, GPIO_PinSource0 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource1 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource2 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource3 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource4 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource5 , GPIO_AF_FSMC);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3 |
                                    GPIO_Pin_4  | GPIO_Pin_5  ; 
    GPIO_Init(GPIOG, &GPIO_InitStructure);
   
}
/*******************************************************************************
* Function Name  : FSMC_Init
* Description    : 
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void FSMC_Init(void)
{
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  p;
//	FSMC_NORSRAMTimingInitTypeDef  WriteTiming; 
	
	FSMC_GPIO_init();/* GPIO引脚配置 */
	
  
  /* Enable FSMC clock */
    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);
    
    FSMC_NORSRAMDeInit(HBI_Bank_NORSRAM);
  /*--------------FSMC 总线 存储器参数配置------------------------------*/
    p.FSMC_AddressSetupTime = 10;           //地址建立时间   为1个HCLK 1/168M=6ns 
    p.FSMC_AddressHoldTime = 0;            //地址保持时间  模式A未用到
    p.FSMC_DataSetupTime = 18;              //数据建立时间
    p.FSMC_BusTurnAroundDuration = 0x0f;      //总线恢复时间
    p.FSMC_CLKDivision = 2;                // 时钟分频因子 
    p.FSMC_DataLatency = 3;                     //数据产生时间
    p.FSMC_AccessMode =  FSMC_AccessMode_A; //FSMC NOR控制器时序
    
    /*--------------FSMC 总线 参数配置------------------------------*/
    FSMC_NORSRAMInitStructure.FSMC_Bank = HBI_Bank_NORSRAM;                  //使用了FSMC的BANK1的子板块2             
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;//禁止地址数据线复用
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;           //存储器类型为SRAM
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;  //存储器数据宽度为16位
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable; //关闭突发模式访问
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low; //等待信号优先级，只有在使能突发访问模式才有效
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;  //关闭Wrapped burst access mode，只有在使能突发访问模式才有效       
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;//等待信号设置，只有在使能突发访问模式才有效
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;     //使能这个BANK的写操作
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;     //使能/关闭等待信息设置，只在使能突发访问模式才有效
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable; //关闭Extend Mode
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;     //关闭Write Burst Mode   
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;               //读操作时序参数
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;                   //写操作时序参数
    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 


    /*!< Enable FSMC Bank1_SRAM2 Bank */
    FSMC_NORSRAMCmd(HBI_Bank_NORSRAM, ENABLE);  
}



void SRAM_WriteBuffer(uint16_t* pBuffer, uint32_t WriteAddr, uint32_t NumHalfwordToWrite)
{
  for (; NumHalfwordToWrite != 0; NumHalfwordToWrite--) /* while there is data to write */
  {
    /* Transfer data to the memory */
    *(uint16_t *) (HBI_SRAM_ADDR + WriteAddr) = *pBuffer++;

    /* Increment the address*/
    WriteAddr += 2;
  }
}

/**
  * @brief  Reads a block of data from the FSMC SRAM memory.
  * @param  pBuffer : pointer to the buffer that receives the data read from the
  *         SRAM memory.
  * @param  ReadAddr : SRAM memory internal address to read from.
  * @param  NumHalfwordToRead : number of half-words to read.
  * @retval None
  */
void SRAM_ReadBuffer(uint16_t* pBuffer, uint32_t ReadAddr, uint32_t NumHalfwordToRead)
{
  for (; NumHalfwordToRead != 0; NumHalfwordToRead--) /* while there is data to read */
  {
    /* Read a half-word from the memory */
    *pBuffer++ = *(__IO uint16_t*) (HBI_SRAM_ADDR + ReadAddr);

    /* Increment the address*/
    ReadAddr += 2;
  }
}

void PMPReadWord (uint16_t Address, uint16_t *Buffer, uint16_t Count)
{
   //for (; Count != 0; Count--) /* while there is data to read */
  //{
    /* Read a half-word from the memory */
    *Buffer++ = *(__IO uint16_t*) (HBI_SRAM_ADDR + Address);

    /* Increment the address*/
  //  Address += 2;
 // }
}

uint32_t PMPReadDWord (uint16_t Address)
{
    
    UINT32_VAL res;
	  
		//res.Val = (uint32_t)(*(volatile uint32_t *)(HBI_SRAM_ADDR+(Address)));
	  res.w[0]= *(__IO uint16_t*) (HBI_SRAM_ADDR + Address);
    /* Increment the address*/
    Address += 2;
	 res.w[1]= *(__IO uint16_t*) (HBI_SRAM_ADDR + Address);
	

    return res.Val;
}





void PMPWriteWord (uint16_t Address, uint16_t *Buffer, uint16_t Count)
{
    
	//for (; Count != 0; Count--) /* while there is data to write */
	//	{
       /* Transfer data to the memory */
        *(uint16_t *) (HBI_SRAM_ADDR + Address) = *Buffer++;

      /* Increment the address*/
 //      Address += 2;
	//	}
    
}

void PMPWriteDWord (uint16_t Address, uint32_t Val)
{
   
	UINT32_VAL res;
	res.Val=Val;
	
	/* Transfer data to the memory */
	
   // *(uint32_t *) (HBI_SRAM_ADDR + Address) = Val;
	*(uint16_t *) (HBI_SRAM_ADDR + Address) = res.w[0];

    /* Increment the address*/
    Address += 2;
	*(uint16_t *) (HBI_SRAM_ADDR + Address) = res.w[1];

  
}


void PMPReadRegUsingCSR(uint8_t *ReadBuffer, uint16_t Address, uint8_t Count)
{
	UINT32_VAL param32_1 = {0};
    uint8_t i = 0;
	UINT16_VAL wAddr;
    wAddr.Val = Address;

    param32_1.v[0] = wAddr.byte.LB;
    param32_1.v[1] = wAddr.byte.HB;
    param32_1.v[2] = Count;
    param32_1.v[3] = ESC_READ_BYTE;

    PMPWriteDWord (CSR_CMD_REG, param32_1.Val);

    do
    {
        param32_1.Val = PMPReadDWord (ESC_CSR_CMD_REG);
	

    }while(param32_1.v[3] & ESC_CSR_BUSY);


    param32_1.Val = PMPReadDWord (ESC_CSR_DATA_REG);

    
     for(i=0;i<Count;i++)
         ReadBuffer[i] = param32_1.v[i];
   
    return;
}

void PMPWriteRegUsingCSR( uint8_t *WriteBuffer, uint16_t Address, uint8_t Count)
{
    UINT32_VAL param32_1 = {0};
	  UINT16_VAL wAddr;
    uint8_t i = 0;
		
    for(i=0;i<Count;i++)
         param32_1.v[i] = WriteBuffer[i];
		
    PMPWriteDWord (ESC_CSR_DATA_REG, param32_1.Val);
		
		wAddr.Val = Address;

    param32_1.v[0] = wAddr.byte.LB;
    param32_1.v[1] = wAddr.byte.HB;
    param32_1.v[2] = Count;
    param32_1.v[3] = ESC_WRITE_BYTE;

    PMPWriteDWord (0x304, param32_1.Val);
   do
    {
       
			param32_1.Val = PMPReadDWord (0x304);
			
    }while(param32_1.v[3] & ESC_CSR_BUSY);

    return;
}

void PMPReadPDRamRegister(uint8_t *ReadBuffer, uint16_t Address, uint16_t Count)
{
    UINT32_VAL param32_1 = {0};
    uint8_t i = 0,nlength, nBytePosition;
    uint8_t nReadSpaceAvblCount;
//    uint16_t RefAddr = Address;

    /*Reset/Abort any previous commands.*/
    param32_1.Val = PRAM_RW_ABORT_MASK;                                                 


        PMPWriteDWord (PRAM_READ_CMD_REG, param32_1.Val);
   
        /*The host should not modify this field unless the PRAM Read Busy
        (PRAM_READ_BUSY) bit is a 0.*/
        do
        {

  
            param32_1.Val = PMPReadDWord (PRAM_READ_CMD_REG);
  
        }while((param32_1.v[3] & PRAM_RW_BUSY_8B));

    /*Write address and length in the EtherCAT Process RAM Read Address and
     * Length Register (ECAT_PRAM_RD_ADDR_LEN)*/
    param32_1.w[0] = Address;
    param32_1.w[1] = Count;



        PMPWriteDWord (PRAM_READ_ADDR_LEN_REG, param32_1.Val);
   

        param32_1.Val = PMPReadDWord (HBI_INDEXED_DATA2_REG );
        /*Set PRAM Read Busy (PRAM_READ_BUSY) bit(-EtherCAT Process RAM Read Command Register)
         *  to start read operatrion*/

        param32_1.Val = PRAM_RW_BUSY_32B; /*TODO:replace with #defines*/

        PMPWriteDWord (PRAM_READ_CMD_REG, param32_1.Val);
 
        /*Read PRAM Read Data Available (PRAM_READ_AVAIL) bit is set*/
        do
        {
            
                    param32_1.Val = PMPReadDWord (PRAM_READ_CMD_REG);
          
        }while(!(param32_1.v[0] & IS_PRAM_SPACE_AVBL_MASK));

    nReadSpaceAvblCount = param32_1.v[1] & PRAM_SPACE_AVBL_COUNT_MASK;

    /*Fifo registers are aliased address. In indexed it will read indexed data reg 0x04, but it will point to reg 0
     In other modes read 0x04 FIFO register since all registers are aliased*/

    
        param32_1.Val = PMPReadDWord (PRAM_READ_FIFO_REG);
  
    nReadSpaceAvblCount--;
    nBytePosition = (Address & 0x03);
    nlength = (4-nBytePosition) > Count ? Count:(4-nBytePosition);
    memcpy(ReadBuffer+i ,&param32_1.v[nBytePosition],nlength);
    Count-=nlength;
    i+=nlength;

    while(Count && nReadSpaceAvblCount)
    {
       
           param32_1.Val = PMPReadDWord (PRAM_READ_FIFO_REG);
    

        nlength = Count > 4 ? 4: Count;
        memcpy((ReadBuffer+i) ,&param32_1,nlength);

        i+=nlength;
        Count-=nlength;
        nReadSpaceAvblCount --;

        if (!nReadSpaceAvblCount)
        {
           
                    param32_1.Val = PMPReadDWord (PRAM_READ_CMD_REG);
          
            nReadSpaceAvblCount = param32_1.v[1] & PRAM_SPACE_AVBL_COUNT_MASK;
        }

    }
   
    return;
}
        

void PMPWritePDRamRegister(uint8_t *WriteBuffer, uint16_t Address, uint16_t Count)
{
    UINT32_VAL param32_1 = {0};
    uint8_t i = 0,nlength, nBytePosition,nWrtSpcAvlCount;
//    uint16_t RefAddr = Address;
    
    /*Reset or Abort any previous commands.*/
    param32_1.Val = PRAM_RW_ABORT_MASK;                                                 /*TODO:replace with #defines*/
  
        PMPWriteDWord (PRAM_WRITE_CMD_REG, param32_1.Val);
   

        /*Make sure there is no previous write is pending
        (PRAM Write Busy) bit is a 0 */
        do
        {
            param32_1.Val = PMPReadDWord (PRAM_WRITE_CMD_REG);
 
        }while((param32_1.v[3] & PRAM_RW_BUSY_8B));

        /*Write Address and Length Register (ECAT_PRAM_WR_ADDR_LEN) with the
        starting byte address and length)*/
        param32_1.w[0] = Address;
        param32_1.w[1] = Count;

  
        PMPWriteDWord (PRAM_WRITE_ADDR_LEN_REG, param32_1.Val);
   

    /*write to the EtherCAT Process RAM Write Command Register (ECAT_PRAM_WR_CMD) with the  PRAM Write Busy
    (PRAM_WRITE_BUSY) bit set*/

    param32_1.Val = PRAM_RW_BUSY_32B; 

   
        PMPWriteDWord (PRAM_WRITE_CMD_REG, param32_1.Val);
  

        /*Read PRAM write Data Available (PRAM_READ_AVAIL) bit is set*/
        do
        {
   
            param32_1.Val = PMPReadDWord (PRAM_WRITE_CMD_REG);

        }while(!(param32_1.v[0] & IS_PRAM_SPACE_AVBL_MASK));

    /*Check write data available count*/
    nWrtSpcAvlCount = param32_1.v[1] & PRAM_SPACE_AVBL_COUNT_MASK;

    /*Write data to Write FIFO) */ 
    /*get the byte lenth for first read*/
    nBytePosition = (Address & 0x03);

    nlength = (4-nBytePosition) > Count ? Count:(4-nBytePosition);

    param32_1.Val = 0;
    memcpy(&param32_1.v[nBytePosition],WriteBuffer+i, nlength);

  
        PMPWriteDWord (PRAM_WRITE_FIFO_REG,param32_1.Val);

    nWrtSpcAvlCount--;
    Count-=nlength;
    i+=nlength;


    while(nWrtSpcAvlCount && Count)
    {
        nlength = Count > 4 ? 4: Count;
        param32_1.Val = 0;
        memcpy(&param32_1, (WriteBuffer+i), nlength);

   
        PMPWriteDWord (PRAM_WRITE_FIFO_REG,param32_1.Val);
   
			i+=nlength;
        Count-=nlength;
        nWrtSpcAvlCount--;

        if (!nWrtSpcAvlCount)
        {
            
                    param32_1.Val = PMPReadDWord (PRAM_WRITE_CMD_REG);
           
            /*Check write data available count*/
            nWrtSpcAvlCount = param32_1.v[1] & PRAM_SPACE_AVBL_COUNT_MASK;
        }
    }
    return;
}
void PMPReadDRegister(uint8_t *ReadBuffer, uint16_t Address, uint16_t Count)
{
    uint16_t i;
    UINT16_VAL val;
    uint8_t *p = ReadBuffer;
    uint16_t addr;
    uint8_t end_Flag = 0;
    
    addr = Address;
    if(addr%2 != 0)
    {
        *(p++) = *(uint16_t *) ((uint32_t)( HBI_SRAM_ADDR+(addr<<1)));
        addr++;
        Count--;
    }
    if(Count%2 != 0)
    {
        end_Flag = 1;
        Count--;
    }
    for(i=0;i<Count/2;i++)
    {
        val.Val = *(uint16_t *) ((uint32_t)( HBI_SRAM_ADDR+(addr<<1)));
        *(p++) = val.byte.LB;
        *(p++) = val.byte.HB;
        addr += 2;
    }
    if(end_Flag != 0)
        *(p++) = *(uint16_t *) ((uint32_t)( HBI_SRAM_ADDR+(addr<<1)));
}
void PMPWriteRegister( uint8_t *WriteBuffer, uint16_t Address, uint16_t Count)
{
    uint16_t i;
    UINT16_VAL val;
    uint8_t *p = WriteBuffer;
    uint16_t addr;
    uint8_t end_Flag = 0;
    
    addr = Address;
    if(addr%2 != 0)
    {
        val.Val = *(p++);
        val.Val <<= 8;
        *(uint16_t *) ((uint32_t)( HBI_SRAM_ADDR+(addr<<1))) = val.Val;
        addr++;
        Count--;
    }
    if(Count%2 != 0)
    {
        end_Flag = 1;
        Count--;
    }
    for(i=0;i<Count/2;i++)
    {
        val.byte.LB = *(p++);
        val.byte.HB = *(p++);
        *(uint16_t *) ((uint32_t)( HBI_SRAM_ADDR+(addr<<1))) = val.Val;
        addr += 2;
    }
    if(end_Flag != 0)
        *(uint8_t *) ((uint32_t)( HBI_SRAM_ADDR+(addr<<1))) = *(p++); //偶地址,单字节读写，NBL1会置高
}





/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

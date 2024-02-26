/**
  ******************************************************************************
  * @file           : ethercat_fsmc.c
  * @author         : h
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "ethercat_fsmc.h"
#include "ecat_def.h"
#include "el9800hw.h"

void SRAM_WriteBuffer(uint16_t* pBuffer, uint32_t WriteAddr, uint32_t NumHalfwordToWrite)
{
  for (; NumHalfwordToWrite != 0; NumHalfwordToWrite--) /* while there is data to write */
  {
    /* Transfer data to the memory */
    *(uint16_t *) (Bank1_SRAM1_ADDR + WriteAddr) = *pBuffer++;

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
    *pBuffer++ = *(__IO uint16_t*) (Bank1_SRAM1_ADDR + ReadAddr);

    /* Increment the address*/
    ReadAddr += 2;
  }
}

void PMPReadWord (uint16_t Address, uint16_t *Buffer, uint16_t Count)
{
   //for (; Count != 0; Count--) /* while there is data to read */
  //{
    /* Read a half-word from the memory */
    *Buffer++ = *(__IO uint16_t*) (Bank1_SRAM1_ADDR + Address);

    /* Increment the address*/
  //  Address += 2;
 // }
}

uint32_t PMPReadDWord (uint16_t Address)
{
    
    UINT32_VAL res;
	  
		//res.Val = (uint32_t)(*(volatile uint32_t *)(Bank1_SRAM1_ADDR+(Address)));
	  res.w[0]= *(__IO uint16_t*) (Bank1_SRAM1_ADDR + Address);
    /* Increment the address*/
    Address += 2;
	 res.w[1]= *(__IO uint16_t*) (Bank1_SRAM1_ADDR + Address);
	

    return res.Val;
}





void PMPWriteWord (uint16_t Address, uint16_t *Buffer, uint16_t Count)
{
    
	//for (; Count != 0; Count--) /* while there is data to write */
	//	{
       /* Transfer data to the memory */
        *(uint16_t *) (Bank1_SRAM1_ADDR + Address) = *Buffer++;

      /* Increment the address*/
 //      Address += 2;
	//	}
    
}

void PMPWriteDWord (uint16_t Address, uint32_t Val)
{
   
	UINT32_VAL res;
	res.Val=Val;
	
	/* Transfer data to the memory */
	
   // *(uint32_t *) (Bank1_SRAM1_ADDR + Address) = Val;
	*(uint16_t *) (Bank1_SRAM1_ADDR + Address) = res.w[0];

    /* Increment the address*/
    Address += 2;
	*(uint16_t *) (Bank1_SRAM1_ADDR + Address) = res.w[1];

  
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
        *(p++) = *(uint16_t *) ((uint32_t)( Bank1_SRAM1_ADDR+(addr<<1)));
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
        val.Val = *(uint16_t *) ((uint32_t)( Bank1_SRAM1_ADDR+(addr<<1)));
        *(p++) = val.byte.LB;
        *(p++) = val.byte.HB;
        addr += 2;
    }
    if(end_Flag != 0)
        *(p++) = *(uint16_t *) ((uint32_t)( Bank1_SRAM1_ADDR+(addr<<1)));
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
        *(uint16_t *) ((uint32_t)( Bank1_SRAM1_ADDR+(addr<<1))) = val.Val;
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
        *(uint16_t *) ((uint32_t)( Bank1_SRAM1_ADDR+(addr<<1))) = val.Val;
        addr += 2;
    }
    if(end_Flag != 0)
        *(uint8_t *) ((uint32_t)( Bank1_SRAM1_ADDR+(addr<<1))) = *(p++); //偶地址,单字节读写，NBL1会置高
}


/**
\addtogroup ESM EtherCAT State Machine
@{
*/

/**
\file bootmode.c
\author EthercatSSC@beckhoff.com
\brief Implementation

\version 4.20

<br>Changes to version - :<br>
V4.20: File created
*/

/*--------------------------------------------------------------------------------------
------
------    Includes
------
--------------------------------------------------------------------------------------*/
#include "ecat_def.h"
#include "stm32f4xx.h"
#if BOOTSTRAPMODE_SUPPORTED

#define    _BOOTMODE_ 1
#include "bootmode.h"
//#include "bsp_cpu_flash.h"
//#include "bsp_spi_flash.h"
#undef _BOOTMODE_


/* FOE下载指示灯 PD2*/
//#define  GPIO_RCC_LED           RCC_AHB1Periph_GPIOB
//#define  GPIO_PORT_LED          GPIOB
//#define  GPIO_PIN_LED	        GPIO_Pin_8


//uint32_t iFlashWriteOffset;
//uint32_t iFileTotalSize; /*接收的文件总大小，Byte*/

/* 文件名，ethercat下发的文件名会和定义的文件比较前4个字节即“HCFA” */
/* HCFA_ST01 */
const char  aFirmwareDownloadHeader[MAX_FILE_NAME_SIZE] = APP_NAME;
const char  aFPGADownloadHeader[MAX_FILE_NAME_SIZE] = FPGA_NAME;

/*
*********************************************************************************************************
*	函 数 名: LedRun
*	功能说明: LED指示。
*	形    参:  No : 指示灯序号，范围 0 - LED_NUM
*	返 回 值: 无
*********************************************************************************************************
*/
void LedRun(uint8_t State)
{
//	if(State)
//	{
//	    GPIO_PORT_LED->BSRRL = GPIO_PIN_LED;/* 点亮*/
//	}
//    else
//    {
//	    GPIO_PORT_LED->BSRRH = GPIO_PIN_LED;/* 关闭led */
//	}
}




////////////////////////	/////////////////////////////////////////////////////////////////
/**
 \param    State        Current state

 \brief Dummy BL_Start function
*////////////////////////////////////////////////////////////////////////////////////////
void BL_Start( UINT8 State)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param    password    download password

 \brief Dummy BL_StartDownload function
*////////////////////////////////////////////////////////////////////////////////////////
void BL_StartDownload(UINT8 ID,UINT32 password)
{
//	UINT8 i;
//
//	if(ID==0)/* 下载F407固件 */
//	{
//		iFlashWriteOffset=DOWNLOAD_ADDRESS;
//		iFileTotalSize=0;
//		//bsp_EraseCpuFlashSector(FLASH_Sector_5);
//		//bsp_EraseCpuFlashSector(FLASH_Sector_6);
//	}
//	else if(ID==1)/* 下载FPGA固件 */
//	{
//		GPIOA->BSRRH = GPIO_Pin_0;
//	    /* 初始化FPGA flash,SPI初始化、读取flash信息 */
//	    bsp_InitSFlash(FPGA_FLASH_ID);
//
//		for(i=0;i<16;i++) //16*64K=1MB擦除
//		{
//			 sf_EraseBlock(FPGA_FLASH_ID,i);
//		}
//
//		iFlashWriteOffset=FPGA_DOWNLOAD_ADDRESS;
//		iFileTotalSize=0;
//	}
}
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param    pData    Data pointer
 \param    Size    Data Length


 \return    FoE error code

 \brief Dummy BL_Data function
*////////////////////////////////////////////////////////////////////////////////////////
void LedRun(uint8_t State);
UINT16 BL_Data(UINT8 ID,UINT16 *pData,UINT16 Size)
{
//	if(ID==0)/* 下载F407固件 */
//	{
//		//bsp_WriteCpuFlash2(iFlashWriteOffset,(uint8_t*)pData,Size);
//		//iFlashWriteOffset=iFlashWriteOffset+(Size);
//		iFileTotalSize+=Size;
//	}
//	else if(ID==1)/* 下载FPGA固件 */
//	{
//		sf_WriteBufferNoCheck(FPGA_FLASH_ID,(uint8_t*)pData,iFlashWriteOffset,Size);
//	    iFlashWriteOffset=iFlashWriteOffset+(Size);
//		iFileTotalSize+=Size;
//	}
//
//	LedRun((*pData)&0x0001);
    return 0;
}

/**

\brief Called in the state transition from BOOT to Init
*////////////////////////////////////////////////////////////////////////////////////////
void BL_Stop(void)
{
//	uint32_t i;
//	uint8_t data;

//	for(i=0;i<iFileTotalSize;i++)
    {
//		bsp_ReadCpuFlash(DOWNLOAD_ADDRESS+i,&data,1);
    }
//	NVIC_SystemReset();
}

#endif //BOOTSTRAPMODE_SUPPORTED
/** @} */

/**
 * \addtogroup ESM EtherCAT State Machine
 * @{
 */

/**
\file bootmode.h
\author EthercatSSC@beckhoff.com
\brief Function prototypes for the Boot state

\version 5.11

<br>Changes to version V5.01:<br>
V5.11 ECAT10: change PROTO handling to prevent compiler errors<br>
<br>Changes to version - :<br>
V5.01 : Start file change log
 */

#ifndef _BOOTMODE_H_
#define _BOOTMODE_H_

/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/
#include "ecat_def.h"


/*-----------------------------------------------------------------------------------------
------
------    Defines and Types
------
-----------------------------------------------------------------------------------------*/

#endif //_BOOTMODE_H_

#define    _BOOTMODE_ 1


#define  APP_NAME                "HCQX" /* app�ļ��� �����ص��ļ���Ҫ���ڵ���6�ֽڣ���HCQXxxxx*/
#define  FPGA_NAME               "FPGA" /* FPGA�ļ��� �����ص��ļ���Ҫ���ڵ���6�ֽڣ���HCQXxxxx*/

#define  MAX_FILE_NAME_SIZE       100/* ����ļ������� */
#define  VALID_FILE_NAME_SIZE     2 /* ��Ч�ļ������� */

/* ECATCHANGE_START(V5.11) ECAT10*/
#if defined(_BOOTMODE_) && (_BOOTMODE_ == 1)
/* ECATCHANGE_END(V5.11) ECAT10*/
    #define PROTO

	/* Flash����ַ */
	#define USER_FLASH_END_ADDRESS        0x08100000 /* 1024 KBytes */

	/* ÿ��������С */
	#define FLASH_PAGE_SIZE               0x800      /* 2 Kbytes */   
	   
	/*������ */
	#define DOWNLOAD_ADDRESS             (uint32_t)0x08020000 /* ��128K��ʼ��ǰ128KΪBootLoader�洢�� */
	#define DOWNLOAD_SIZE                (128*1024)  /* ���ش洢����С128K */
    #define MCU_VERSION_ADD              (DOWNLOAD_ADDRESS+100*1024)
	
	/* APP�� */
	#define APP_ADDRESS                  (uint32_t)0x08040000/* APP���׵�ַ0x8040000 */
	#define APP_SIZE                     128*1024  /* APP��С128 */
	#define APP_VERSION_ADD              (APP_ADDRESS+100*1024)
	
	/* �����̼���  */
	#define FACTORY_ADDRESS              (uint32_t)0x08060000/* APP���׵�ַ0x8060000 */
	#define FACTORY_SIZE                 128*1024  /* APP��С128 */
	
	/*--------------------------------------FGPA-------------------------------------------*/
	/*������ */
	#define FPGA_DOWNLOAD_ADDRESS             (uint32_t)0x00000000 /* ��0��ַ��ʼ */
	#define FPGA_DOWNLOAD_SIZE                (128*1024)  /* ���ش洢����С128K */
    #define FPGA_MCU_VERSION_ADD              (DOWNLOAD_ADDRESS+100*1024)
	
	
#else
    #define PROTO extern
#endif
 /*-----------------------------------------------------------------------------------------
------
------    Global variables
------
-----------------------------------------------------------------------------------------*/
extern const char HUGE aFirmwareDownloadHeader[MAX_FILE_NAME_SIZE];
extern const char HUGE aFPGADownloadHeader[MAX_FILE_NAME_SIZE];

/*-----------------------------------------------------------------------------------------
------
------    Global functions
------
-----------------------------------------------------------------------------------------*/
PROTO    void BL_Start( UINT8 State);
PROTO    void BL_StartDownload(UINT8 ID,UINT32 password);
UINT16 BL_Data(UINT8 ID,UINT16 *pData,UINT16 Size);
PROTO    void BL_Stop(void);
#undef PROTO
/** @}*/

/*
* This source file is part of the EtherCAT Slave Stack Code licensed by Beckhoff Automation GmbH & Co KG, 33415 Verl, Germany.
* The corresponding license agreement applies. This hint shall not be removed.
*/

/**
* \addtogroup SSC-Device SSC-Device
* @{
*/

/**
\file Module0x00001001_Objects
\author ET9300Utilities.ApplicationHandler (Version 1.6.3.0) | EthercatSSC@beckhoff.com

\brief SSC-Device specific objects<br>
\brief NOTE : This file will be overwritten if a new object dictionary is generated!<br>
*/

#if defined(_SSC_DEVICE_) && (_SSC_DEVICE_ == 1)
#define PROTO
#else
#define PROTO extern
#endif
/******************************************************************************
*                    Object 0x1A00 : Input Data Buffer process data mapping
******************************************************************************/
/**
* \addtogroup 0x1A00 0x1A00 | Input Data Buffer process data mapping
* @{
* \brief Object 0x1A00 (Input Data Buffer process data mapping) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Reference to 0x6000.1<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDescModule1001Obj0x1A00[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }}; /* Subindex1 - Reference to 0x6000.1 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aNameModule1001Obj0x1A00[] = "Input Data Buffer process data mapping\000"
"SubIndex 001\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_DEVICE_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT32 SI1; /* Subindex1 - Reference to 0x6000.1 */
} OBJ_STRUCT_PACKED_END
TMODULE1001OBJ1A00;
#endif //#ifndef _SSC_DEVICE_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TMODULE1001OBJ1A00 Module1001InputDataBufferProcessDataMapping0x1A00
#if defined(_SSC_DEVICE_) && (_SSC_DEVICE_ == 1)
={1,0x60000108}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x6000 : Input Data Buffer
******************************************************************************/
/**
* \addtogroup 0x6000 0x6000 | Input Data Buffer
* @{
* \brief Object 0x6000 (Input Data Buffer) definition
*/
#ifdef _OBJD_
/**
* \brief Entry descriptions<br>
* 
* Subindex 0<br>
* Subindex 1 - n (the same entry description is used)<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDescModule1001Obj0x6000[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED8 , 0x08 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }};

/**
* \brief Object name definition<br>
* For Subindex 1 to n the syntax 'Subindex XXX' is used
*/
OBJCONST UCHAR OBJMEM aNameModule1001Obj0x6000[] = "Input Data Buffer\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_DEVICE_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16   u16SubIndex0;  /**< \brief Subindex 0 */
UINT8 aEntries[1];  /**< \brief Subindex 1 - 1 */
} OBJ_STRUCT_PACKED_END
TMODULE1001OBJ6000;
#endif //#ifndef _SSC_DEVICE_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TMODULE1001OBJ6000 Module1001InputDataBuffer0x6000
#if defined(_SSC_DEVICE_) && (_SSC_DEVICE_ == 1)
={1,{0}}
#endif
;
/** @}*/







#if COE_SUPPORTED
#ifdef _OBJD_
TOBJECT    OBJMEM Module00001001ObjDic[] = {
/* Object 0x1A00 */
{NULL , NULL ,  0x1A00 , {DEFTYPE_PDOMAPPING , 1 | (OBJCODE_REC << 8)} , asEntryDescModule1001Obj0x1A00 , aNameModule1001Obj0x1A00 , &Module1001InputDataBufferProcessDataMapping0x1A00 , NULL , NULL , 0x0000 },
/* Object 0x6000 */
{NULL , NULL ,  0x6000 , {DEFTYPE_UNSIGNED8 , 1 | (OBJCODE_ARR << 8)} , asEntryDescModule1001Obj0x6000 , aNameModule1001Obj0x6000 , &Module1001InputDataBuffer0x6000 , NULL , NULL , 0x0000 },
{NULL,NULL, 0xFFFF, {0, 0}, NULL, NULL, NULL, NULL}};
#endif    //#ifdef _OBJD_
#endif
#undef PROTO

/** @}*/

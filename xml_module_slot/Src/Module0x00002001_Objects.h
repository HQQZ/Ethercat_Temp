/*
* This source file is part of the EtherCAT Slave Stack Code licensed by Beckhoff Automation GmbH & Co KG, 33415 Verl, Germany.
* The corresponding license agreement applies. This hint shall not be removed.
*/

/**
* \addtogroup SSC-Device SSC-Device
* @{
*/

/**
\file Module0x00002001_Objects
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
*                    Object 0x1600 : Output Data Buffer process data mapping
******************************************************************************/
/**
* \addtogroup 0x1600 0x1600 | Output Data Buffer process data mapping
* @{
* \brief Object 0x1600 (Output Data Buffer process data mapping) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Reference to 0x7000.1<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDescModule2001Obj0x1600[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }}; /* Subindex1 - Reference to 0x7000.1 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aNameModule2001Obj0x1600[] = "Output Data Buffer process data mapping\000"
"SubIndex 001\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_DEVICE_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT32 SI1; /* Subindex1 - Reference to 0x7000.1 */
} OBJ_STRUCT_PACKED_END
TMODULE2001OBJ1600;
#endif //#ifndef _SSC_DEVICE_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TMODULE2001OBJ1600 Module2001OutputDataBufferProcessDataMapping0x1600
#if defined(_SSC_DEVICE_) && (_SSC_DEVICE_ == 1)
={1,0x70000108}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x7000 : Output Data Buffer
******************************************************************************/
/**
* \addtogroup 0x7000 0x7000 | Output Data Buffer
* @{
* \brief Object 0x7000 (Output Data Buffer) definition
*/
#ifdef _OBJD_
/**
* \brief Entry descriptions<br>
* 
* Subindex 0<br>
* Subindex 1 - n (the same entry description is used)<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDescModule2001Obj0x7000[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED8 , 0x08 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }};

/**
* \brief Object name definition<br>
* For Subindex 1 to n the syntax 'Subindex XXX' is used
*/
OBJCONST UCHAR OBJMEM aNameModule2001Obj0x7000[] = "Output Data Buffer\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_DEVICE_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16   u16SubIndex0;  /**< \brief Subindex 0 */
UINT8 aEntries[1];  /**< \brief Subindex 1 - 1 */
} OBJ_STRUCT_PACKED_END
TMODULE2001OBJ7000;
#endif //#ifndef _SSC_DEVICE_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TMODULE2001OBJ7000 Module2001OutputDataBuffer0x7000
#if defined(_SSC_DEVICE_) && (_SSC_DEVICE_ == 1)
={1,{0}}
#endif
;
/** @}*/







#if COE_SUPPORTED
#ifdef _OBJD_
TOBJECT    OBJMEM Module00002001ObjDic[] = {
/* Object 0x1600 */
{NULL , NULL ,  0x1600 , {DEFTYPE_PDOMAPPING , 1 | (OBJCODE_REC << 8)} , asEntryDescModule2001Obj0x1600 , aNameModule2001Obj0x1600 , &Module2001OutputDataBufferProcessDataMapping0x1600 , NULL , NULL , 0x0000 },
/* Object 0x7000 */
{NULL , NULL ,  0x7000 , {DEFTYPE_UNSIGNED8 , 1 | (OBJCODE_ARR << 8)} , asEntryDescModule2001Obj0x7000 , aNameModule2001Obj0x7000 , &Module2001OutputDataBuffer0x7000 , NULL , NULL , 0x0000 },
{NULL,NULL, 0xFFFF, {0, 0}, NULL, NULL, NULL, NULL}};
#endif    //#ifdef _OBJD_
#endif
#undef PROTO

/** @}*/

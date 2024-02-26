/**
 * \addtogroup CoE CAN Application Profile over EtherCAT
 * @{
 */

/**
\file coeappl.h
\author EthercatSSC@beckhoff.com

\version 5.11

<br>Changes to version V5.01:<br>
V5.11 ECAT10: change PROTO handling to prevent compiler errors<br>
<br>Changes to version - :<br>
V5.01 : Start file change log
 */
#ifndef _COEAPPL_H_
#define _COEAPPL_H_

/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/
#include "objdef.h"


extern UINT16 acRestore[2];

/*-----------------------------------------------------------------------------------------
------
------    Defines and Types
------
-----------------------------------------------------------------------------------------*/



#endif //_COEAPPL_H_

/* ECATCHANGE_START(V5.11) ECAT10*/
#if defined(_COEAPPL_) && (_COEAPPL_ == 1)
/* ECATCHANGE_END(V5.11) ECAT10*/
    #define PROTO
#else
    #define PROTO extern
#endif

	
	
/*-----------------------------------------------------------------------------------------
------
------    Struct
------
-----------------------------------------------------------------------------------------*/	
	
/**
 * \brief 0x1010 (Identity) object structure
 */
typedef struct OBJ_STRUCT_PACKED_START {
   UINT16   u16SubIndex0; /**< \brief SubIndex 0*/
   UINT32   u32ParAll; /**< \brief EtherCAT vendor ID */
   UINT32   u32Par1000; /**< \brief Product code*/
   UINT32   u32Par6000; /**< \brief Revision number*/
   UINT32   u32Par2000; /**< \brief Serial number*/
} OBJ_STRUCT_PACKED_END
TOBJ1010;
/*---------------------------------------------
-    0x1011 »Ö¸´²ÎÊý
-----------------------------------------------*/
/**
 * \brief 0x1011 (Identity) object structure
 */
typedef struct OBJ_STRUCT_PACKED_START {
   UINT16   u16SubIndex0; /**< \brief SubIndex 0*/
   UINT32   u32ParAll; /**< \brief EtherCAT vendor ID */
   UINT32   u32Par1000; /**< \brief Product code*/
   UINT32   u32Par6000; /**< \brief Revision number*/
   UINT32   u32Par2000; /**< \brief Serial number*/
} OBJ_STRUCT_PACKED_END
TOBJ1011;
/*-----------------------------------------------------------------------------------------
------
------    Global variables
------
-----------------------------------------------------------------------------------------*/
extern TOBJ1010 sStoreParameters;
extern TOBJ1011 sRestoreParameters;
/*-----------------------------------------------------------------------------------------
------
------    Global functions
------
-----------------------------------------------------------------------------------------*/
PROTO    void COE_ObjInit(void);
PROTO    void COE_Main(void);
PROTO UINT16 COE_ObjDictionaryInit(void);
PROTO UINT16 COE_AddObjectToDic(TOBJECT OBJMEM * pNewObjEntry);
PROTO void COE_RemoveDicEntry(UINT16 index);
PROTO void COE_ClearObjDictionary(void);
PROTO OBJCONST TOBJECT OBJMEM * COE_GetObjectDictionary(void);


#undef PROTO
/** @}*/

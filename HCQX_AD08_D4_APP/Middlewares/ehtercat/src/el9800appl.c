/**
\addtogroup EL9800Appl EL9800 application
@{
*/

/**
\file el9800appl.c
\author EthercatSSC@beckhoff.com
\brief Implementation

\version 5.11

<br>Changes to version V5.10:<br>
V5.11 ECAT11: create application interface function pointer, add eeprom emulation interface functions<br>
V5.11 EL9800 1: reset outputs on fallback from OP state<br>
<br>Changes to version V5.01:<br>
V5.10 ECAT6: Add "USE_DEFAULT_MAIN" to enable or disable the main function<br>
<br>Changes to version V5.0:<br>
V5.01 EL9800 2: Add TxPdo Parameter object 0x1802<br>
<br>Changes to version V4.30:<br>
V4.50 ECAT2: Create generic application interface functions. Documentation in Application Note ET9300.<br>
V4.50 COE2: Handle invalid PDO assign values.<br>
V4.30 : create file
*/


/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/
#include "ecat_def.h"

#if EL9800_APPLICATION

/* ECATCHANGE_START(V5.11) ECAT11*/
#include "applInterface.h"
/* ECATCHANGE_END(V5.11) ECAT11*/

//#include "el9800hw.h"
#define _EVALBOARD_
#include "el9800appl.h"
#undef _EVALBOARD_

#if MCI_HW
#include "mcihw.h"
#endif
#if EL9800_HW
#include "el9800hw.h"
#endif

#include "ecatfoe.h"

#include "Coeappl.h"
#include "ecatslv.h"
#include "applinterface.h"
#include <string.h>

uint8_t ESC_State = 0;
/*--------------------------------------------------------------------------------------
------
------    local types and defines
------
--------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
------
------    local variables and constants
------
-----------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
------
------    application specific functions
------
-----------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
------
------    generic functions
------
-----------------------------------------------------------------------------------------*/
/////////////////////////////////////////////////////////////////////////////////////////
/**
\param    u32ModuleId  Module ident of the request module

\return   Pointer to the module object dictionary

\brief   This function gets the module default object dictionary 
		  (not the module specific dictionary which is mapped to the device object dictionary)
*////////////////////////////////////////////////////////////////////////////////////////
TOBJECT OBJMEM* Module_GetObjectDictionary(UINT32 u32ModuleId)
{
    UINT16 u16Counter = 0;
    UINT16 u16NumModules = SIZEOF(ModuleOds) / SIZEOF(TMODULEODREF);

    for (u16Counter = 0; u16Counter < u16NumModules; u16Counter++)
    {
        if (ModuleOds[u16Counter].u32ModuleId == u32ModuleId)
        {
            return ModuleOds[u16Counter].pObjectDictionary;
        }
    }

    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\param    u32ModuleId  Module ident of the request module
\return   number of objects

\brief   This functions gets the number of objects defined by the module
*////////////////////////////////////////////////////////////////////////////////////////
UINT16 Module_GetObjectCount(UINT32 u32ModuleId)
{
    TOBJECT OBJMEM* pObject;
    UINT16 u16Counter = 0;

    pObject = Module_GetObjectDictionary(u32ModuleId);
    if (pObject != NULL)
    {
        while (pObject->Index != 0xFFFF)
        {
            u16Counter++;
            pObject++;
        }
    }

    return u16Counter;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\return   0 if all module objects are added to the device object dictionary

\brief   This functions add/remove the module related objects to/from the device object dictionary based on the configured module ident list (0xF030)
*////////////////////////////////////////////////////////////////////////////////////////
UINT16 Module_UpdateObjectDictionary()
{
    TOBJECT OBJMEM* pModuleObjEntry = NULL;
    UINT16 u16Counter = 0;
    UINT16 IndexIncrement = 0;
    UINT16 result = 0;

    for (u16Counter = 0; u16Counter < 16; u16Counter++)
    {
        UINT32 u32ModuleId = ConfiguredModuleIdentList0xF030.aEntries[u16Counter];

        /*clear module ident if the loop is out of the configured range*/
        if (u16Counter >= ConfiguredModuleIdentList0xF030.u16SubIndex0)
            u32ModuleId = 0;

        if (u32ModuleId != AssignedModules[u16Counter].u32ModuleId)
        {
            /*a new module is assigned to the current slot*/
            if (AssignedModules[u16Counter].u32ModuleId != 0)
            {
                /*Clear the assigned module objects*/
                pModuleObjEntry = AssignedModules[u16Counter].pObjectDictionary;
                while (pModuleObjEntry->Index != 0xFFFF)
                {
                    COE_RemoveDicEntry(pModuleObjEntry->Index);

                    /*free previously allocated memory*/
                    if (pModuleObjEntry->pVarPtr != NULL)
                    {
                        FREEMEM(pModuleObjEntry->pVarPtr);
                        pModuleObjEntry->pVarPtr = NULL;
                    }
                    pModuleObjEntry++;
                }//loop over the current assigned module OD

                FREEMEM(AssignedModules[u16Counter].pObjectDictionary);
            }

            AssignedModules[u16Counter].u32ModuleId = u32ModuleId;
            AssignedModules[u16Counter].pObjectDictionary = NULL;

            if (u32ModuleId != 0)
            {
                UINT16 u16NumObjects = 0;
                UINT16 u16OdSize = 0;
                u16NumObjects = Module_GetObjectCount(u32ModuleId);

                /*add 0xFFFF "object"*/
                u16OdSize = (SIZEOF(TOBJECT) * (u16NumObjects + 1));

                AssignedModules[u16Counter].pObjectDictionary = ALLOCMEM(u16OdSize);

                if (AssignedModules[u16Counter].pObjectDictionary == NULL)
                {
                    /*failed to allocate memory for module OD*/
                    AssignedModules[u16Counter].u32ModuleId = 0;

                    return 1;
                }

                pModuleObjEntry = Module_GetObjectDictionary(u32ModuleId);
                if (pModuleObjEntry == NULL)
                {
                    /*Module object dictionary not found*/
                    AssignedModules[u16Counter].u32ModuleId = 0;
                    FREEMEM(AssignedModules[u16Counter].pObjectDictionary);
                    AssignedModules[u16Counter].pObjectDictionary = NULL;

                    return 1;
                }

                MEMCPY(AssignedModules[u16Counter].pObjectDictionary, pModuleObjEntry, u16OdSize);

                pModuleObjEntry = AssignedModules[u16Counter].pObjectDictionary;
                /*loop over all module object dictionary entries and add to device object dictionary*/
                while (pModuleObjEntry->Index != 0xFFFF)
                {
                    /*Index increment is greater 0, create a new object dictionary entry*/
                    pModuleObjEntry->pPrev = NULL;
                    pModuleObjEntry->pNext = NULL;

                    if ((pModuleObjEntry->Index >= 0x1600 && pModuleObjEntry->Index <= 0x17FF)
                        || (pModuleObjEntry->Index >= 0x1A00 && pModuleObjEntry->Index <= 0x1BFF))
                    {
                        pModuleObjEntry->Index = pModuleObjEntry->Index + (u16Counter * MODULE_PDO_OBJECT_INCREMENT);
                    }
                    else if (pModuleObjEntry->Index >= 0x2000 && pModuleObjEntry->Index <= 0xBFFF)
                    {
                        pModuleObjEntry->Index = pModuleObjEntry->Index + (u16Counter * MODULE_OBJECT_INCREMENT);
                    }


                    /*allocate object data memory*/
                    if (pModuleObjEntry->pVarPtr != NULL)
                    {
                        void MBXMEM* pData = pModuleObjEntry->pVarPtr;
                        /*Get size of required object memory (Implicit compiler alignment is taken into account if OBJ_DWORD_ALIGN/OBJ_WORD_ALIGN are set)*/
                        UINT8 maxSubindex = (pModuleObjEntry->ObjDesc.ObjFlags & OBJFLAGS_MAXSUBINDEXMASK) >> OBJFLAGS_MAXSUBINDEXSHIFT;
                        OBJCONST TSDOINFOENTRYDESC OBJMEM* pEntry; 
                        UINT32 u32objsize = OBJ_GetEntryOffset(maxSubindex, pModuleObjEntry);
                        pEntry = OBJ_GetEntryDesc(pModuleObjEntry, maxSubindex);
                        u32objsize = BIT2BYTE((u32objsize + pEntry->BitLength)); 

                        pModuleObjEntry->pVarPtr = ALLOCMEM(u32objsize);

                        if (pModuleObjEntry->pVarPtr == NULL)
                        {
                            return 1;
                        }

                        /*copy initial value to new object*/
                        MEMCPY(pModuleObjEntry->pVarPtr, pData, u32objsize);

                        if ((pModuleObjEntry->Index >= 0x1600 && pModuleObjEntry->Index <= 0x17FF)
                            || (pModuleObjEntry->Index >= 0x1A00 && pModuleObjEntry->Index <= 0x1BFF))
                        {
                            UINT16 maxConfiguredSubindex = (pModuleObjEntry->ObjDesc.ObjFlags & OBJFLAGS_MAXSUBINDEXMASK) >> OBJFLAGS_MAXSUBINDEXSHIFT; //required to check if value for Subindex0 is valid
                            UINT32* pData = (UINT32*)(((UINT16*)pModuleObjEntry->pVarPtr) + (OBJ_GetEntryOffset(1, pModuleObjEntry) >> 4));/*Get PDO entry*/
                            UINT16 u16Idx;

                            while (maxConfiguredSubindex > 0)
                            {
                                u16Idx = (UINT16)((*pData) >> 16);

                                if (u16Idx >= 0x2000 && u16Idx <= 0xBFFF)
                                {
                                    u16Idx += (u16Counter * MODULE_OBJECT_INCREMENT);
                                    *pData = (*pData & 0x0000FFFF) | (((UINT32)u16Idx) << 16);
                                }
                                pData++;
                                maxConfiguredSubindex--;
                            }
                        }
                    }

                    if (pModuleObjEntry->Index <= 0xFFF)
                    {
                        /*skip data type/unit or enum definitions if they are already defined in the OD*/
                        OBJCONST TOBJECT OBJMEM *pObj = OBJ_GetObjectHandle(pModuleObjEntry->Index);
                        if (pObj == NULL)
                        {
                            result = COE_AddObjectToDic(pModuleObjEntry);

                            if (result != 0)
                            {
                                return result;
                            }
                        }
                    }
                    else
                    {
                        result = COE_AddObjectToDic(pModuleObjEntry);

                        if (result != 0)
                        {
                            return result;
                        }
                    }

                    if (result != 0)
                    {
                        return 1;
                    }

                    pModuleObjEntry++;
                } 
            }
        }
        else if(AssignedModules[u16Counter].pObjectDictionary != NULL)
        {
            /*verify that the enum definitions are available*/
            OBJCONST TOBJECT OBJMEM* pObj;
            pModuleObjEntry = AssignedModules[u16Counter].pObjectDictionary;
            /*loop over all module object dictionary entries and add to device object dictionary*/
            while ((pModuleObjEntry->Index != 0xFFFF) && (pModuleObjEntry->Index <= 0xFFF))
            {
                pObj = OBJ_GetObjectHandle(pModuleObjEntry->Index);
                if (pObj == NULL)
                {
                    result = COE_AddObjectToDic(pModuleObjEntry);

                    if (result != 0)
                    {
                        return result;
                    }
                }
                pModuleObjEntry++;
            }
        }
    }

    return 0;
}

//void ADC_UserLoop(void);
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief    The function is called when an error state was acknowledged by the master

*////////////////////////////////////////////////////////////////////////////////////////

void    APPL_AckErrorInd(UINT16 stateTrans)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    AL Status Code (see ecatslv.h ALSTATUSCODE_....)

 \brief    The function is called in the state transition from INIT to PREOP when
           all general settings were checked to start the mailbox handler. This function
           informs the application about the state transition, the application can refuse
           the state transition when returning an AL Status error code.
           The return code NOERROR_INWORK can be used, if the application cannot confirm
           the state transition immediately, in that case the application need to be complete
           the transition by calling ECAT_StateChange.

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StartMailboxHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, NOERROR_INWORK

 \brief    The function is called in the state transition from PREEOP to INIT
             to stop the mailbox handler. This functions informs the application
             about the state transition, the application cannot refuse
             the state transition.

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StopMailboxHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param    pIntMask    pointer to the AL Event Mask which will be written to the AL event Mask
                        register (0x204) when this function is succeeded. The event mask can be adapted
                        in this function
 \return    AL Status Code (see ecatslv.h ALSTATUSCODE_....)

 \brief    The function is called in the state transition from PREOP to SAFEOP when
             all general settings were checked to start the input handler. This function
             informs the application about the state transition, the application can refuse
             the state transition when returning an AL Status error code.
            The return code NOERROR_INWORK can be used, if the application cannot confirm
            the state transition immediately, in that case the application need to be complete
            the transition by calling ECAT_StateChange.
*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StartInputHandler(UINT16 *pIntMask)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, NOERROR_INWORK

 \brief    The function is called in the state transition from SAFEOP to PREEOP
             to stop the input handler. This functions informs the application
             about the state transition, the application cannot refuse
             the state transition.

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StopInputHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    AL Status Code (see ecatslv.h ALSTATUSCODE_....)

 \brief    The function is called in the state transition from SAFEOP to OP when
             all general settings were checked to start the output handler. This function
             informs the application about the state transition, the application can refuse
             the state transition when returning an AL Status error code.
           The return code NOERROR_INWORK can be used, if the application cannot confirm
           the state transition immediately, in that case the application need to be complete
           the transition by calling ECAT_StateChange.
*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StartOutputHandler(void)//OP状态
{
//	    ESC_State = 1;

    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, NOERROR_INWORK

 \brief    The function is called in the state transition from OP to SAFEOP
             to stop the output handler. This functions informs the application
             about the state transition, the application cannot refuse
             the state transition.

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StopOutputHandler(void)
{
//    u16 Data[4] = {0, 0, 0, 0};//20220510新增
//
//	sDOOutputs_hold = sDOOutputs;//保存最后值
//
//	APPL_OutputMapping(Data);//20220510新增

    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, invalid mapping error

 \brief    Assign all PDO mapping to the related SM assign objects

*////////////////////////////////////////////////////////////////////////////////////////
UINT16 APPL_UpdateAssignObjects()
{
    UINT16 PDOAssignEntryCnt = 0;
    OBJCONST TOBJECT OBJMEM* pAssignObj = OBJ_GetObjectHandle(0x1C12);
    UINT16 maxConfiguredSubindex = (pAssignObj->ObjDesc.ObjFlags & OBJFLAGS_MAXSUBINDEXMASK) >> OBJFLAGS_MAXSUBINDEXSHIFT;
    OBJCONST TOBJECT OBJMEM* pObjEntry = COE_GetObjectDictionary();

    /*Process 0x1C12*/
    do
    {
        if (pObjEntry->Index >= 0x1600 && pObjEntry->Index <= 0x17FF)
        {
            if (PDOAssignEntryCnt >= maxConfiguredSubindex)
            {
                return ALSTATUSCODE_INVALIDOUTPUTMAPPING;;
            }
            sRxPDOassign.aEntries[PDOAssignEntryCnt] = pObjEntry->Index;
            PDOAssignEntryCnt++;
        }

        /*abort loop if max index is reached (objects are sorted in ascending order)*/
        if (pObjEntry->Index >= 0x17FF)
        {
            break;
        }

        pObjEntry = pObjEntry->pNext;
    } while (pObjEntry != NULL);

    sRxPDOassign.u16SubIndex0 = PDOAssignEntryCnt;
    
    /*clear remaining entries*/
    for (; PDOAssignEntryCnt < maxConfiguredSubindex; PDOAssignEntryCnt++)
    {
        sRxPDOassign.aEntries[PDOAssignEntryCnt] = 0;
    }


    /*Process 0x1C13*/
    pAssignObj = OBJ_GetObjectHandle(0x1C13);
    maxConfiguredSubindex = (pAssignObj->ObjDesc.ObjFlags & OBJFLAGS_MAXSUBINDEXMASK) >> OBJFLAGS_MAXSUBINDEXSHIFT;
    PDOAssignEntryCnt = 0;

    while (pObjEntry != NULL)
    {
        if (pObjEntry->Index >= 0x1A00 && pObjEntry->Index <= 0x1BFF)
        {
            if (PDOAssignEntryCnt >= maxConfiguredSubindex)
            {
                return ALSTATUSCODE_INVALIDINPUTMAPPING;;
            }
            sTxPDOassign.aEntries[PDOAssignEntryCnt] = pObjEntry->Index;
            PDOAssignEntryCnt++;
        }

        /*abort loop if max index is reached (objects are sorted in ascending order)*/
        if (pObjEntry->Index >= 0x1BFF)
        {
            break;
        }

        pObjEntry = pObjEntry->pNext;
    } 

    sTxPDOassign.u16SubIndex0 = PDOAssignEntryCnt;

    /*clear remaining entries*/
    for (; PDOAssignEntryCnt < maxConfiguredSubindex; PDOAssignEntryCnt++)
    {
        sTxPDOassign.aEntries[PDOAssignEntryCnt] = 0;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\return     0(ALSTATUSCODE_NOERROR), NOERROR_INWORK
\param      pInputSize  pointer to save the input process data length
\param      pOutputSize  pointer to save the output process data length

\brief    This function calculates the process data sizes from the actual SM-PDO-Assign
            and PDO mapping
*////////////////////////////////////////////////////////////////////////////////////////
UINT16 APPL_GenerateMapping(UINT16 *pInputSize,UINT16 *pOutputSize)
{
    UINT16 result = ALSTATUSCODE_NOERROR;
    UINT16 InputSize = 0;
    UINT16 OutputSize = 0;

#if COE_SUPPORTED
    UINT16 PDOAssignEntryCnt = 0;
    OBJCONST TOBJECT OBJMEM * pPDO = NULL;
    UINT16 PDOSubindex0 = 0;
    UINT32 *pPDOEntry = NULL;
    UINT16 PDOEntryCnt = 0;
   
	result = Module_UpdateObjectDictionary();
    if (result != ALSTATUSCODE_NOERROR)
    {
        return ALSTATUSCODE_UNSPECIFIEDERROR;
    }

    result = APPL_UpdateAssignObjects();
    if (result != ALSTATUSCODE_NOERROR)
    {
        return result;
    }

#if MAX_PD_OUTPUT_SIZE > 0
    /*Scan object 0x1C12 RXPDO assign*/
    for(PDOAssignEntryCnt = 0; PDOAssignEntryCnt < sRxPDOassign.u16SubIndex0; PDOAssignEntryCnt++)
    {
        pPDO = OBJ_GetObjectHandle(sRxPDOassign.aEntries[PDOAssignEntryCnt]);
        if(pPDO != NULL)
        {
            PDOSubindex0 = *((UINT16 *)pPDO->pVarPtr);
            for(PDOEntryCnt = 0; PDOEntryCnt < PDOSubindex0; PDOEntryCnt++)
            {
                pPDOEntry = (UINT32 *)(((UINT16 *)pPDO->pVarPtr) + (OBJ_GetEntryOffset((PDOEntryCnt+1),pPDO)>>4));    //goto PDO entry
                // we increment the expected output size depending on the mapped Entry
                OutputSize += (UINT16) ((*pPDOEntry) & 0xFF);
            }
        }
        else
        {
            /*assigned PDO was not found in object dictionary. return invalid mapping*/
            OutputSize = 0;
            result = ALSTATUSCODE_INVALIDOUTPUTMAPPING;
            break;
        }
    }

    OutputSize = (OutputSize + 7) >> 3;
#endif

#if MAX_PD_INPUT_SIZE > 0
    if(result == 0)
    {
        /*Scan Object 0x1C13 TXPDO assign*/
        for(PDOAssignEntryCnt = 0; PDOAssignEntryCnt < sTxPDOassign.u16SubIndex0; PDOAssignEntryCnt++)
        {
            pPDO = OBJ_GetObjectHandle(sTxPDOassign.aEntries[PDOAssignEntryCnt]);
            if(pPDO != NULL)
            {
                PDOSubindex0 = *((UINT16 *)pPDO->pVarPtr);
                for(PDOEntryCnt = 0; PDOEntryCnt < PDOSubindex0; PDOEntryCnt++)
                {
                    pPDOEntry = (UINT32 *)(((UINT16 *)pPDO->pVarPtr) + (OBJ_GetEntryOffset((PDOEntryCnt+1),pPDO)>>4));    //goto PDO entry
                    // we increment the expected output size depending on the mapped Entry
                    InputSize += (UINT16) ((*pPDOEntry) & 0xFF);
                }
            }
            else
            {
                /*assigned PDO was not found in object dictionary. return invalid mapping*/
                InputSize = 0;
                result = ALSTATUSCODE_INVALIDINPUTMAPPING;
                break;
            }
        }
    }
    InputSize = (InputSize + 7) >> 3;
#endif

#else
#if _WIN32
   #pragma message ("Warning: Define 'InputSize' and 'OutputSize'.")
#else
    #warning "Define 'InputSize' and 'OutputSize'."
#endif
#endif

    *pInputSize = InputSize;
    *pOutputSize = OutputSize;
    return result;
}


/////////////////////////////////////////////////////////////////////////////////////////
/**
\param      pData  pointer to input process data
\brief      This function will copies the inputs from the local memory to the ESC memory
            to the hardware
*////////////////////////////////////////////////////////////////////////////////////////
#if _STM32_IO4 && AL_EVENT_ENABLED
/* the pragma interrupt_level is used to tell the compiler that these functions will not
   be called at the same time from the main function and the interrupt routine */
#pragma interrupt_level 1
#endif
void APPL_InputMapping(UINT16* pData)
{
    UINT16 j = 0;

    /* we go through all entries of the TxPDO Assign object to get the assigned TxPDOs */
    for (j = 0; j < sTxPDOassign.u16SubIndex0; j++)
    {
		
    }
}






/////////////////////////////////////////////////////////////////////////////////////////
/**
\param      pData  pointer to output process data

\brief    This function will copies the outputs from the ESC memory to the local memory
            to the hardware
*////////////////////////////////////////////////////////////////////////////////////////
#if _STM32_IO4  && AL_EVENT_ENABLED
/* the pragma interrupt_level is used to tell the compiler that these functions will not
   be called at the same time from the main function and the interrupt routine */
#pragma interrupt_level 1
#endif


//extern u8  HC_DAC_Channel;									/*用来表示正在使用哪个通道*/


void APPL_OutputMapping(UINT16* pData)
{
    UINT16 j = 0;

    /* we go through all entries of the RxPDO Assign object to get the assigned RxPDOs */
    for (j = 0; j < sRxPDOassign.u16SubIndex0; j++)
    {

    }

}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\brief    This function will called from the synchronisation ISR
            or from the mainloop if no synchronisation is supported
*////////////////////////////////////////////////////////////////////////////////////////


void APPL_Application(void)
{

#endif


//    /* we toggle the TxPDO Toggle after updating the data of the corresponding TxPDO */
//    sAIInputs.bTxPDOToggle ^= 1;

//    /* we simulate a problem of the analog input, if the Switch4 is on in this example,
//       in this case the TxPDO State has to set to indicate the problem to the master */
//    if ( sDIInputs.bSwitch4 )
//        sAIInputs.bTxPDOState = 1;
//    else
//        sAIInputs.bTxPDOState = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param     index               index of the requested object.
 \param     subindex            subindex of the requested object.
 \param     objSize             size of the requested object data, calculated with OBJ_GetObjectLength
 \param     pData               Pointer to the buffer where the data can be copied to
 \param     bCompleteAccess     Indicates if a complete read of all subindices of the
                                object shall be done or not

 \return    ABORTIDX_XXX

 \brief     Handles SDO read requests to TxPDO Parameter
*////////////////////////////////////////////////////////////////////////////////////////
//UINT8 ReadObject0x1802( UINT16 index, UINT8 subindex, UINT32 dataSize, UINT16 MBXMEM * pData, UINT8 bCompleteAccess )
//{

//    if(bCompleteAccess)
//        return ABORTIDX_UNSUPPORTED_ACCESS;

//    if(subindex == 0)
//    {
//        *pData = TxPDO1802Subindex0;
//    }
//    else if(subindex == 6)
//    {
//        /*clear destination buffer (no excluded TxPDO set)*/
//        if(dataSize > 0)
//            MBXMEMSET(pData,0x00,dataSize);
//    }
//    else if(subindex == 7)
//    {
//        /*min size is one Byte*/
//        UINT8 *pu8Data = (UINT8*)pData;
//
//        //Reset Buffer
//        *pu8Data = 0;

//        *pu8Data = sCh1Status.bTxPDOState;
//    }
//    else if(subindex == 9)
//    {
//        /*min size is one Byte*/
//        UINT8 *pu8Data = (UINT8*)pData;
//
//        //Reset Buffer
//        *pu8Data = 0;

//        *pu8Data = sCh1Status.bTxPDOToggle;
//    }
//    else
//        return ABORTIDX_SUBINDEX_NOT_EXISTING;

//    return 0;
//}

#if EXPLICIT_DEVICE_ID
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    The Explicit Device ID of the EtherCAT slave

 \brief     Calculate the Explicit Device ID
*////////////////////////////////////////////////////////////////////////////////////////
UINT16 APPL_GetDeviceID()
{
    UINT16 Value = (UINT16)(SWITCH_8<<7)|(SWITCH_7<<6)|(SWITCH_6<<5)|(SWITCH_5<<4)|(SWITCH_4<<3)|(SWITCH_3<<2)|(SWITCH_2<<1)|(SWITCH_1);
    return Value;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    0               Init CiA402 device successful
            ALSTATUSCODE_XX Init CiA402 device failed

 \brief    This function initializes the Axes structures
*////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////
/**

 \brief    This is the main function

*////////////////////////////////////////////////////////////////////////////////////////
int ecat_slave_main(void)
{
    /* initialize the Hardware and the EtherCAT Slave Controller */
    HW_Init();

    MainInit();
	
	Module_UpdateObjectDictionary();

    /*Create basic mapping*/
//    APPL_GenerateMapping(&nPdInputSize,&nPdOutputSize);

    return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////
///**

// \brief    This is the main function

//*////////////////////////////////////////////////////////////////////////////////////////



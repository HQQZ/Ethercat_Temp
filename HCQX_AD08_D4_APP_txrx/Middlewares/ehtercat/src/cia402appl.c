/**
\addtogroup CiA402appl CiA402 Sample Application
@{
*/

/**
\file cia402appl.c
\author EthercatSSC@beckhoff.com
\brief Implementation
This file contains all ciA402 specific functions

\version 5.11

<br>Changes to version V5.10:<br>
V5.11 ECAT11: create application interface function pointer, add eeprom emulation interface functions<br>
<br>Changes to version V5.01:<br>
V5.10 CIA402 1: Update complete access handling for 0xF030<br>
V5.10 ECAT6: Add "USE_DEFAULT_MAIN" to enable or disable the main function<br>
<br>Changes to version V5.0:<br>
V5.01 ESC2: Add missed value swapping<br>
<br>Changes to version V4.40:<br>
V5.0 CIA402 1: Syntax bugfix in dummy motion controller<br>
V5.0 CIA402 2: Handle 0xF030/0xF050 in correlation do PDO assign/mapping objects<br>
V5.0 CIA402 3: Trigger dummy motion controller if valid mode of operation is set.<br>
V5.0 CIA402 4: Change Axes structure handling and resources allocation.<br>
V5.0 ECAT2: Create generic application interface functions. Documentation in Application Note ET9300.<br>
<br>Changes to version V4.30:<br>
V4.40 CoE 6: add AL Status code to Init functions<br>
V4.40 CIA402 2: set motion control trigger depending on "Synchronisation", "mode of operation" and "cycle time"<br>
V4.40 CIA402 1: change behaviour and name of bit12 of the status word (0x6041) (WG CIA402 24.02.2010)<br>
V4.30 : create file (state machine; handling state transition options; input feedback)
*/

/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/
#include "ecat_def.h"


/* ECATCHANGE_START(V5.11) ECAT11*/
#include "applInterface.h"
/* ECATCHANGE_END(V5.11) ECAT11*/

#include "coeappl.h"

#define _CiA402_
#include "cia402appl.h"
#undef _CiA402_

#include "FPGA_Com.h"
#include "cia402homing.h"
#include "bsp_debug.h"
#include "bsp_IO.h"
#include "bsp_systick_timer.h"
#include "IO.h"

struct SERVO_CONTROL_WORDS_Bit {
    u16  Switch_On         :1;/* 启动 */
    u16  Enable_Voltage    :1;/* 电压输出 */
    u16  Quick_Stop        :1;/* 快速停止 */
    u16  Enable_operation  :1;/* 使能操作 */
    u16  Operation_mode    :3;/* 根据操作模式而定 */
    u16  Fault_Reset       :1;/* 错误复位 */
    u16  Halt              :1;/* 暂停 */
    u16  Reserve           :1;/*  */
    u16  Manufacturer_Specific :6;
};
union SERVO_CONTROL_WORDS {
    u16 all;
    struct SERVO_CONTROL_WORDS_Bit bit;
} Servo_control[4];


struct  SERVO_STATE_WORDS_Bit {
    u16  Ready_Switch     :1;
    u16  Switch_On         :1;
    u16  Serve_En_ok      :1;
    u16  Fault              :1;
    u16  Voltage_Enabled   :1;
    u16  Quick_Stop        :1;
    u16  Switch_Disabled  :1;
    u16  Warning         :1;
    u16  Reserved8      :1;
    u16  Remote          :1;
    u16  Target_Limit  :1;
    u16  Internal_Limit  :1;
    u16  Target_Value    :1;
    u16  Reserved13       :1;
    u16  Reserved14_15     :1;
};
union SERVO_STATE_WORDS {
    u16 all;
    struct SERVO_STATE_WORDS_Bit bit;
} Servo_State;


uint16_t statusword;

extern int16_t MaxPuleCout;/* 一个周期内可以发送的最大脉冲数 */
UINT8 AxisActiveBit;/* bit0:1 表示轴0 被配置、bit1:1 表示轴1 被配置、bit2:1 表示轴2 被配置、bit3:1 表示轴3 被配置、 */
//uint16_t CspRunFlagAfterReset;/* 复位后第一次运行CSP模式标记，如果是第一次不判断正负限位信号，0：复位后还没执行，1复位后执行过了 */
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
TCiA402Axis       LocalAxes[MAX_AXES];
/*-----------------------------------------------------------------------------------------
------
------    application specific functions
------
-----------------------------------------------------------------------------------------*/

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    0               Init CiA402 device successful
            ALSTATUSCODE_XX Init CiA402 device failed

 \brief    This function initializes the Axes structures
*////////////////////////////////////////////////////////////////////////////////////////
UINT16 CiA402_Init(void)
{
    UINT16 result = 0;
    UINT16 AxisCnt = 0;
    UINT16 j = 0;
    UINT32 ObjectOffset = 0x800;
    UINT8 PDOOffset = 0x10;
    TOBJECT OBJMEM *pDiCEntry;

    /* 添加自定义对象字典 */
    pDiCEntry=DefUserObjDic;
    while(pDiCEntry->Index != 0xFFFF)
    {
        result = COE_AddObjectToDic(pDiCEntry);

        if(result != 0)
        {
            return result;
        }

        pDiCEntry++;
    }
    if(result != 0)
    {
        return result;
    }

    for(AxisCnt = 0; AxisCnt < MAX_AXES ; AxisCnt++)
    {
        /*Reset Axis buffer*/
        HMEMSET(&LocalAxes[AxisCnt],0,SIZEOF(TCiA402Axis));

        LocalAxes[AxisCnt].bAxisIsActive = FALSE;
        LocalAxes[AxisCnt].bBrakeApplied = TRUE;
        LocalAxes[AxisCnt].bLowLevelPowerApplied = TRUE;
        LocalAxes[AxisCnt].bHighLevelPowerApplied = FALSE;
        LocalAxes[AxisCnt].bAxisFunctionEnabled = FALSE;
        LocalAxes[AxisCnt].bConfigurationAllowed = TRUE;

        LocalAxes[AxisCnt].i16State = STATE_NOT_READY_TO_SWITCH_ON;
        LocalAxes[AxisCnt].u16PendingOptionCode = 0x00;

        LocalAxes[AxisCnt].fCurPosition = 0;
        LocalAxes[AxisCnt].u32CycleTime = 0;


        /***********************************
            init objects
        *************************************/

        /*set default values*/
        HMEMCPY(&LocalAxes[AxisCnt].Objects,&DefCiA402ObjectValues,CIA402_OBJECTS_SIZE);

        /***set Object offset to PDO entries***/
        //每个轴的字典对象需要偏移0x800
        /*csv/csp RxPDO*/
        for(j =0; j < LocalAxes[AxisCnt].Objects.sRxPDOMap0.u16SubIndex0; j++)
            LocalAxes[AxisCnt].Objects.sRxPDOMap0.aEntries[j] += AxisCnt* (ObjectOffset<<16);

        /*csp RxPDO*/
        for(j =0; j < LocalAxes[AxisCnt].Objects.sRxPDOMap1.u16SubIndex0; j++)
            LocalAxes[AxisCnt].Objects.sRxPDOMap1.aEntries[j] += AxisCnt* (ObjectOffset<<16);

        /*csv RxPDO*/
        for(j =0; j < LocalAxes[AxisCnt].Objects.sRxPDOMap2.u16SubIndex0; j++)
            LocalAxes[AxisCnt].Objects.sRxPDOMap2.aEntries[j] += AxisCnt* (ObjectOffset<<16);


        /*csv/csp TxPDO*/
        for(j =0; j < LocalAxes[AxisCnt].Objects.sTxPDOMap0.u16SubIndex0; j++)
            LocalAxes[AxisCnt].Objects.sTxPDOMap0.aEntries[j] += AxisCnt* (ObjectOffset<<16);

        /*csp TxPDO*/
        for(j =0; j < LocalAxes[AxisCnt].Objects.sTxPDOMap1.u16SubIndex0; j++)
            LocalAxes[AxisCnt].Objects.sTxPDOMap1.aEntries[j] += AxisCnt* (ObjectOffset<<16);

        /*csv TxPDO*/
        for(j =0; j < LocalAxes[AxisCnt].Objects.sTxPDOMap2.u16SubIndex0; j++)
            LocalAxes[AxisCnt].Objects.sTxPDOMap2.aEntries[j] += AxisCnt* (ObjectOffset<<16);

        /***********************************
            init objects dictionary entries
        *************************************/
        LocalAxes[AxisCnt].ObjDic = (TOBJECT *) ALLOCMEM(SIZEOF(DefCiA402AxisObjDic));
        HMEMCPY(LocalAxes[AxisCnt].ObjDic,&DefCiA402AxisObjDic,SIZEOF(DefCiA402AxisObjDic));
        {
            pDiCEntry = LocalAxes[AxisCnt].ObjDic;

            /*adapt Object index and assign Var pointer*/
            while(pDiCEntry->Index != 0xFFFF)
            {
                switch(pDiCEntry->Index)
                {
                case 0x1600:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.sRxPDOMap0;
                    break;
                case 0x1601:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.sRxPDOMap1;
                    break;
                case 0x1602:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.sRxPDOMap2;
                    break;
                case 0x1A00:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.sTxPDOMap0;
                    break;
                case 0x1A01:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.sTxPDOMap1;
                    break;
                case 0x1A02:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.sTxPDOMap2;
                    break;
                case 0x603F:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objErrorCode;
                    break;
                case 0x6040:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objControlWord;
                    break;
                case 0x6041:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objStatusWord;
                    break;
                case 0x605A:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objQuickStopOptionCode;
                    break;
                case 0x605B:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objShutdownOptionCode;
                    break;
                case 0x605C:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objDisableOperationOptionCode;
                    break;
                case 0x605E:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objFaultReactionCode;
                    break;
                case 0x6060:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objModesOfOperation;
                    break;
                case 0x6061:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objModesOfOperationDisplay;
                    break;
                case 0x6064:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objPositionActualValue;
                    break;
                case 0x6065:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objFollowingErrorWindow;
                    break;
                case 0x606C:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objVelocityActualValue;
                    break;
                case 0x6077:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objTorqueActualValue;
                    break;
                case 0x607A:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objTargetPosition;
                    break;
                case 0x607C:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objHomingOffset;
                    break;
                case 0x607D:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objSoftwarePositionLimit;
                    break;
                case 0x6085:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objQuickStopDeclaration;
                    break;
                case 0x6098:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objHomingMethed;
                    break;
                case 0x6099:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objHomingVelocity;
                    break;
                case 0x609A:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objHomingAcceleration;
                    break;
                case 0x60B0:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objPositionOffset;
                    break;
                case 0x60C2:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objInterpolationTimePeriod;
                    break;
                case 0x60F4:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objFollowingErrActualValue;
                    break;
                case 0x60FD:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objDigitalInputs;
                    break;

                case 0x60FE:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objDigitalOutputs;
                    break;
                case 0x60FF:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objTargetVelocity;
                    break;
                case 0x6502:
                    pDiCEntry->pVarPtr = &LocalAxes[AxisCnt].Objects.objSupportedDriveModes;
                    break;
                default :
                    break;
                }//switch(pDiCEntry->Index)

                /*increment object index*/
                if(pDiCEntry->Index >= 0x1400 && pDiCEntry->Index <= 0x1BFF)    //PDO region
                {
                    pDiCEntry->Index += AxisCnt* PDOOffset;
                }
                else
                {
                    pDiCEntry->Index += AxisCnt* (UINT16)ObjectOffset;
                }

                pDiCEntry++;

            }//while(pDiCEntry->Index != 0xFFFF)

        }
    }// for "all active axes"

    return result;
}
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief    CiA402_DeallocateAxis
 \brief    Remove all allocated axes resources
*////////////////////////////////////////////////////////////////////////////////////////
void CiA402_DeallocateAxis(void)
{
    UINT8 cnt = 0;

    for(cnt = 0 ; cnt < MAX_AXES ; cnt++)
    {
        /*Remove object dictionary entries*/
        if(LocalAxes[cnt].ObjDic != NULL)
        {
            TOBJECT OBJMEM *pEntry = LocalAxes[cnt].ObjDic;

            while(pEntry->Index != 0xFFFF)
            {
                COE_RemoveDicEntry(pEntry->Index);

                pEntry++;
            }

            FREEMEM(LocalAxes[cnt].ObjDic);
        }

        nPdOutputSize = 0;
        nPdInputSize = 0;

    }

}
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief    CiA402-Statemachine
        This function handles the state machine for devices using the CiA402 profile.
        called cyclic from MainLoop()
        All described transition numbers are referring to the document
        "ETG Implementation Guideline for the CiA402 Axis Profile" located on the EtherCAT.org download section

*////////////////////////////////////////////////////////////////////////////////////////
void CiA402_StateMachine(void)
{
    TCiA402Axis *pCiA402Axis;
    UINT16 StatusWord = 0;
    UINT16 ControlWord6040 = 0;
    UINT16 counter = 0;

    for(counter = 0; counter < MAX_AXES; counter++)
    {
        if(!LocalAxes[counter].bAxisIsActive)
        {
            continue;
        }

        pCiA402Axis = &LocalAxes[counter];
        StatusWord = pCiA402Axis->Objects.objStatusWord;
        statusword  = StatusWord;
        ControlWord6040 = pCiA402Axis->Objects.objControlWord;

        /*clear statusword state and controlword processed complete bits*/
        StatusWord &= ~(STATUSWORD_STATE_MASK | STATUSWORD_REMOTE);

        /*skip state state transition if the previous transition is pending*/
        if(pCiA402Axis->u16PendingOptionCode!= 0x0)
        {
            return;
        }
        /*skip transition 1 and 2*/
        if(pCiA402Axis->i16State < STATE_READY_TO_SWITCH_ON && nAlStatus == STATE_OP)
            pCiA402Axis->i16State = STATE_READY_TO_SWITCH_ON;

        switch(pCiA402Axis->i16State)
        {
        case STATE_NOT_READY_TO_SWITCH_ON:
            StatusWord |= (STATUSWORD_STATE_NOTREADYTOSWITCHON);
            if(nAlStatus == STATE_OP)
            {
                // Automatic transition -> Communication shall be activated
                pCiA402Axis->i16State = STATE_SWITCH_ON_DISABLED; // Transition 1
            }
            else
            {
                /*
                CiA402 statemachine shall stay in "STATE_NOT_READY_TO_SWITCH_ON" if EtherCAT state is not OP.
                */
                pCiA402Axis->i16State = STATE_NOT_READY_TO_SWITCH_ON; // stay in current state
            }

            break;
        case STATE_SWITCH_ON_DISABLED:
            StatusWord |= (STATUSWORD_STATE_SWITCHEDONDISABLED);
            if ((ControlWord6040 & CONTROLWORD_COMMAND_SHUTDOWN_MASK) == CONTROLWORD_COMMAND_SHUTDOWN)
            {
                pCiA402Axis->i16State = STATE_READY_TO_SWITCH_ON; // Transition 2
            }
            break;
        case STATE_READY_TO_SWITCH_ON:
            StatusWord |= (STATUSWORD_STATE_READYTOSWITCHON);
            if (((ControlWord6040 & CONTROLWORD_COMMAND_QUICKSTOP_MASK) == CONTROLWORD_COMMAND_QUICKSTOP)
                    || ((ControlWord6040 & CONTROLWORD_COMMAND_DISABLEVOLTAGE_MASK) == CONTROLWORD_COMMAND_DISABLEVOLTAGE))
            {
                pCiA402Axis->i16State = STATE_SWITCH_ON_DISABLED; // Transition 7
            } else if (((ControlWord6040 & CONTROLWORD_COMMAND_SWITCHON_MASK) == CONTROLWORD_COMMAND_SWITCHON) ||
                       ((ControlWord6040 & CONTROLWORD_COMMAND_SWITCHON_ENABLEOPERATION_MASK) == CONTROLWORD_COMMAND_SWITCHON_ENABLEOPERATION))
            {
                pCiA402Axis->i16State = STATE_SWITCHED_ON;           // Transition 3
            }
            break;
        case STATE_SWITCHED_ON:
            StatusWord |= (STATUSWORD_STATE_SWITCHEDON);

            if ((ControlWord6040 & CONTROLWORD_COMMAND_SHUTDOWN_MASK) == CONTROLWORD_COMMAND_SHUTDOWN)
            {
                pCiA402Axis->i16State = STATE_READY_TO_SWITCH_ON; // Transition 6

            } else if (((ControlWord6040 & CONTROLWORD_COMMAND_QUICKSTOP_MASK) == CONTROLWORD_COMMAND_QUICKSTOP
                        || (ControlWord6040 & CONTROLWORD_COMMAND_DISABLEVOLTAGE_MASK) == CONTROLWORD_COMMAND_DISABLEVOLTAGE))
            {
                pCiA402Axis->i16State = STATE_SWITCH_ON_DISABLED; // Transition 10

            } else if ((ControlWord6040 & CONTROLWORD_COMMAND_ENABLEOPERATION_MASK) == CONTROLWORD_COMMAND_ENABLEOPERATION)
            {
                pCiA402Axis->i16State = STATE_OPERATION_ENABLED;  // Transition 4
                //The Axis function shall be enabled and all internal set-points cleared.
            }
            break;
        case STATE_OPERATION_ENABLED:
            StatusWord |= (STATUSWORD_STATE_OPERATIONENABLED);

            if ((ControlWord6040 & CONTROLWORD_COMMAND_DISABLEOPERATION_MASK) == CONTROLWORD_COMMAND_DISABLEOPERATION)
            {
                if(pCiA402Axis->Objects.objDisableOperationOptionCode!= DISABLE_DRIVE)
                {
                    /*disable operation pending*/
                    pCiA402Axis->u16PendingOptionCode = 0x605C; //STATE_TRANSITION (STATE_OPERATION_ENABLED,STATE_SWITCHED_ON);
                    return;
                }
                pCiA402Axis->i16State = STATE_SWITCHED_ON;           // Transition 5
            } else if ((ControlWord6040 & CONTROLWORD_COMMAND_QUICKSTOP_MASK) == CONTROLWORD_COMMAND_QUICKSTOP)
            {
                pCiA402Axis->i16State = STATE_QUICK_STOP_ACTIVE;  // Transition 11
            } else if ((ControlWord6040 & CONTROLWORD_COMMAND_SHUTDOWN_MASK) == CONTROLWORD_COMMAND_SHUTDOWN)
            {
                if(pCiA402Axis->Objects.objShutdownOptionCode != DISABLE_DRIVE)
                {
                    /*shutdown operation required*/
                    pCiA402Axis->u16PendingOptionCode = 0x605B; //STATE_TRANSITION (STATE_OPERATION_ENABLED,STATE_READY_TO_SWITCH_ON);
                    return;
                }

                pCiA402Axis->i16State = STATE_READY_TO_SWITCH_ON; // Transition 8

            } else if ((ControlWord6040 & CONTROLWORD_COMMAND_DISABLEVOLTAGE_MASK) == CONTROLWORD_COMMAND_DISABLEVOLTAGE)
            {
                pCiA402Axis->i16State = STATE_SWITCH_ON_DISABLED; // Transition 9
            }
            break;
        case STATE_QUICK_STOP_ACTIVE:
            StatusWord |= STATUSWORD_STATE_QUICKSTOPACTIVE;

            if((pCiA402Axis->Objects.objQuickStopOptionCode != DISABLE_DRIVE) &&
                    ((pCiA402Axis->Objects.objStatusWord & STATUSWORD_STATE_MASK)!= STATUSWORD_STATE_QUICKSTOPACTIVE))
            {
                /*Only execute quick stop action in state transition 11*/
                pCiA402Axis->u16PendingOptionCode = 0x605A;//STATE_TRANSITION (STATE_OPERATION_ENABLED,STATE_QUICK_STOP_ACTIVE);
                return;
            }

            if ((ControlWord6040 & CONTROLWORD_COMMAND_DISABLEVOLTAGE_MASK) == CONTROLWORD_COMMAND_DISABLEVOLTAGE)
            {
                pCiA402Axis->i16State = STATE_SWITCH_ON_DISABLED; // Transition 12
            }

            /*NOTE: it is not recommend to support transition 16 */
            break;
        case STATE_FAULT_REACTION_ACTIVE:
            StatusWord |= (STATUSWORD_STATE_FAULTREACTIONACTIVE);
            if(pCiA402Axis->Objects.objFaultReactionCode!= DISABLE_DRIVE)
            {
                /*fault reaction pending*/
                pCiA402Axis->u16PendingOptionCode = 0x605E;
                return;
            }

            // Automatic transition
            pCiA402Axis->i16State = STATE_FAULT;// Transition 14
            break;
        case STATE_FAULT:
            StatusWord |= (STATUSWORD_STATE_FAULT);

            if ((ControlWord6040 & CONTROLWORD_COMMAND_FAULTRESET_MASK) == CONTROLWORD_COMMAND_FAULTRESET)
            {
                CiA402_ClearLocalError(counter);
                pCiA402Axis->i16State = STATE_SWITCH_ON_DISABLED;// Transition 15
            }
            break;

        default:    //the sate variable is set to in invalid value => rest Axis
            StatusWord = (STATUSWORD_STATE_NOTREADYTOSWITCHON);
            pCiA402Axis->i16State = STATE_NOT_READY_TO_SWITCH_ON;
            break;

        }// switch(current state)



        /*Update operational functions (the low level power supply is always TRUE*/
        switch(pCiA402Axis->i16State)
        {
        case STATE_NOT_READY_TO_SWITCH_ON:
        case STATE_SWITCH_ON_DISABLED:
        case STATE_READY_TO_SWITCH_ON:
            pCiA402Axis->bBrakeApplied = TRUE;
            pCiA402Axis->bHighLevelPowerApplied =  TRUE;
            pCiA402Axis->bAxisFunctionEnabled = FALSE;
            pCiA402Axis->bConfigurationAllowed = TRUE;
            break;
        case STATE_SWITCHED_ON:
            pCiA402Axis->bBrakeApplied = TRUE;
            pCiA402Axis->bHighLevelPowerApplied =  TRUE;
            pCiA402Axis->bAxisFunctionEnabled = FALSE;
            pCiA402Axis->bConfigurationAllowed = TRUE;
            break;
        case STATE_OPERATION_ENABLED:
        case STATE_QUICK_STOP_ACTIVE:
        case STATE_FAULT_REACTION_ACTIVE:
            pCiA402Axis->bBrakeApplied = FALSE;
            pCiA402Axis->bHighLevelPowerApplied =  TRUE;
            pCiA402Axis->bAxisFunctionEnabled = TRUE;
            pCiA402Axis->bConfigurationAllowed = FALSE;
            break;
        case STATE_FAULT:
            pCiA402Axis->bBrakeApplied = TRUE;
            pCiA402Axis->bHighLevelPowerApplied =  FALSE;
            pCiA402Axis->bAxisFunctionEnabled = FALSE;
            pCiA402Axis->bConfigurationAllowed = TRUE;
            break;
        default:
            pCiA402Axis->bBrakeApplied = TRUE;
            pCiA402Axis->bHighLevelPowerApplied =  FALSE;
            pCiA402Axis->bAxisFunctionEnabled = FALSE;
            pCiA402Axis->bConfigurationAllowed = TRUE;
            break;
        }

        if(pCiA402Axis->bHighLevelPowerApplied == TRUE)
        {
            StatusWord |= STATUSWORD_VOLTAGE_ENABLED;
        }
        else
        {
            StatusWord &= ~STATUSWORD_VOLTAGE_ENABLED;
        }

        /*state transition finished set controlword complete bit and update status object 0x6041*/
        pCiA402Axis->Objects.objStatusWord = (StatusWord | STATUSWORD_REMOTE);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param ErrorCode

 \brief    CiA402_LocalError
 \brief this function is called if an error was detected
*////////////////////////////////////////////////////////////////////////////////////////
void CiA402_LocalError(UINT8 No,UINT16 ErrorCode)
{

    if(LocalAxes[No].bAxisIsActive)
    {
//		LocalAxes[No].i16State = STATE_FAULT_REACTION_ACTIVE;
        LocalAxes[No].i8ErrorHappened=1;
        LocalAxes[No].Objects.objErrorCode = ErrorCode;
    }
}

void CiA402_ClearLocalError(UINT8 No)
{
    if(LocalAxes[No].bAxisIsActive)
    {
        //LocalAxes[No].i16State = STATE_FAULT_REACTION_ACTIVE;
        LocalAxes[No].i8ErrorHappened=0;
        LocalAxes[No].Objects.objErrorCode = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief    CiA402_DummyMotionControl
 \brief this functions provides an simple feedback functionality
*////////////////////////////////////////////////////////////////////////////////////////
void CiA402_DummyMotionControl(TCiA402Axis *pCiA402Axis)
{
    UINT8 Ret=0;
    INT32 i32TargetVelocity = 0;
    INT16 PositionIncrement=0;/* 目标位置和当前位置的差值 */
    UINT16 FPGA_Status=0;
    UINT32 FollwingErrorValue=0;

    /*Motion Controller shall only be triggered if application is trigger by DC Sync Signals,
    and a valid mode of operation is set*/
#ifdef DEBUG_402

    float IncFactor    = (float)0.0010922 * (float) pCiA402Axis->u32CycleTime;
    /*calculate actual position */
    pCiA402Axis->fCurPosition += ((double)pCiA402Axis->Objects.objVelocityActualValue) * IncFactor;
    pCiA402Axis->Objects.objPositionActualValue = (INT32)(pCiA402Axis->fCurPosition);
#else
    float IncFactor    = (float)0.001 * (float) pCiA402Axis->u32CycleTime;	/* 微秒转换成毫秒 */
#endif

    if(pCiA402Axis->bAxisFunctionEnabled &&
            pCiA402Axis->bLowLevelPowerApplied &&
            pCiA402Axis->bHighLevelPowerApplied &&
            !pCiA402Axis->bBrakeApplied)
    {
        /* 超过最大或最小限位值后，看设置的目标位置，总之就是超出了能往回走 */
        if((pCiA402Axis->Objects.objSoftwarePositionLimit.i32MaxLimit> pCiA402Axis->Objects.objPositionActualValue/* 实际位置小于最大位置 */
                || pCiA402Axis->Objects.objPositionActualValue >= pCiA402Axis->Objects.objTargetPosition) && /* 实际位置大于目标位置 */
                (pCiA402Axis->Objects.objSoftwarePositionLimit.i32MinLimit < pCiA402Axis->Objects.objPositionActualValue/* 实际位置大于最小位置 */
                 || pCiA402Axis->Objects.objPositionActualValue <= pCiA402Axis->Objects.objTargetPosition)&&/* 实际位置小于目标位置 */
                ((pCiA402Axis->Objects.objDigitalInputs & 0x00010000)==0))/* 急停开关信号 */
        {
            pCiA402Axis->Objects.objStatusWord &= ~STATUSWORD_INTERNAL_LIMIT;/* 清除超限标记 */

            switch(pCiA402Axis->Objects.objModesOfOperationDisplay)
            {
            case CYCLIC_SYNC_POSITION_MODE:/* CSP 模式 */

                if(IncFactor != 0)
                {
                    PositionIncrement=pCiA402Axis->Objects.objTargetPosition - pCiA402Axis->i32TargetPosition;/* 增量位置=新的目标位置 - 最后一次发送的位置 */
                    if(pCiA402Axis->Objects.objTargetPosition>pCiA402Axis->i32TargetPosition)
                    {
                        if(PositionIncrement>MaxPuleCout)PositionIncrement=MaxPuleCout;
                    }
                    else
                    {
                        if(-PositionIncrement>MaxPuleCout)PositionIncrement=-MaxPuleCout;
                    }
                }
                else
                {
                    PositionIncrement=0;
                }

                if((pCiA402Axis->Objects.objDigitalInputs & 0x00000003)!=0)/* bit0:负限位开关信号,bit1:正限位开关信号，bit16:急停开关 */
                {
                    pCiA402Axis->Objects.objStatusWord |= STATUSWORD_INTERNAL_LIMIT;/* 标记超限位 */

                    if((pCiA402Axis->Objects.objDigitalInputs & 0x0000002)!=0)/* 正限位，允许向负方向运动，否则报警 */
                    {
                        if(PositionIncrement>0)/* 如果正限位有效，还向正运动就报警 */
                        {
                            CiA402_LocalError(pCiA402Axis->i8CurrentAxisNo,0x0002);

                            FPGA_SetPosition(pCiA402Axis->i8CurrentAxisNo,0);/* 发送增量位置 0 到FPGA */
                            break;
                        }
                    }

                    if((pCiA402Axis->Objects.objDigitalInputs & 0x0000001)!=0)/* 负限位，允许向正方向运动，否则报警 */
                    {
                        if(PositionIncrement<0)/* 如果负限位有效，还向负运动就报警 */
                        {
                            CiA402_LocalError(pCiA402Axis->i8CurrentAxisNo,0x0002);

                            FPGA_SetPosition(pCiA402Axis->i8CurrentAxisNo,0);/* 发送增量位置 0 到FPGA */
                            break;
                        }
                    }
                }


#if _DEBUG_RTT
                debug5("TargetPosition=%d\r\n",PositionIncrement);
#endif

                Ret=FPGA_SetPosition(pCiA402Axis->i8CurrentAxisNo,PositionIncrement);/* 发送增量位置到FPGA */

                if(Ret==1)/* 超最大频率 */
                {
                    CiA402_LocalError(pCiA402Axis->i8CurrentAxisNo,0x0003);
                }

                /* 跟随误差实际值 */
                pCiA402Axis->Objects.objFollowingErrActualValue = pCiA402Axis->Objects.objPositionActualValue - pCiA402Axis->Objects.objTargetPosition;
                /* 判断跟随误差 */
                (pCiA402Axis->Objects.objPositionActualValue > pCiA402Axis->Objects.objTargetPosition)?
                (FollwingErrorValue = pCiA402Axis->Objects.objPositionActualValue - pCiA402Axis->Objects.objTargetPosition):
                (FollwingErrorValue = pCiA402Axis->Objects.objTargetPosition - pCiA402Axis->Objects.objPositionActualValue);

                if(FollwingErrorValue > pCiA402Axis->Objects.objFollowingErrorWindow)
                {
                    CiA402_LocalError(pCiA402Axis->i8CurrentAxisNo,0x0004);/* 发生跟随误差错误 */
                }

                break;
            case HOMING_MODE:/* 回零模式 */
                FPGA_HomingRun(pCiA402Axis->i8CurrentAxisNo);
                break;

            default:
                if(IncFactor != 0)
                {
                    PositionIncrement=pCiA402Axis->Objects.objTargetPosition - pCiA402Axis->i32TargetPosition;/* 增量位置=新的目标位置 - 最后一次发送的位置 */
                    if(pCiA402Axis->Objects.objTargetPosition>pCiA402Axis->i32TargetPosition)
                    {
                        if(PositionIncrement>MaxPuleCout)PositionIncrement=MaxPuleCout;
                    }
                    else
                    {
                        if(-PositionIncrement>MaxPuleCout)PositionIncrement=-MaxPuleCout;
                    }
                }
                else
                {
                    PositionIncrement=0;
                }

                if((pCiA402Axis->Objects.objDigitalInputs & 0x00000003)!=0)/* bit0:负限位开关信号,bit1:正限位开关信号，bit16:急停开关 */
                {
                    pCiA402Axis->Objects.objStatusWord |= STATUSWORD_INTERNAL_LIMIT;/* 标记超限位 */

                    if((pCiA402Axis->Objects.objDigitalInputs & 0x0000002)!=0)/* 正限位，允许向负方向运动，否则报警 */
                    {
                        if(PositionIncrement>0)/* 如果正限位有效，还向正运动就报警 */
                        {
                            CiA402_LocalError(pCiA402Axis->i8CurrentAxisNo,0x0002);

                            FPGA_SetPosition(pCiA402Axis->i8CurrentAxisNo,0);/* 发送增量位置 0 到FPGA */
                            break;
                        }
                    }

                    if((pCiA402Axis->Objects.objDigitalInputs & 0x0000001)!=0)/* 负限位，允许向正方向运动，否则报警 */
                    {
                        if(PositionIncrement<0)/* 如果负限位有效，还向负运动就报警 */
                        {
                            CiA402_LocalError(pCiA402Axis->i8CurrentAxisNo,0x0002);

                            FPGA_SetPosition(pCiA402Axis->i8CurrentAxisNo,0);/* 发送增量位置 0 到FPGA */
                            break;
                        }
                    }
                }


#if _DEBUG_RTT
                debug5("TargetPosition=%d\r\n",PositionIncrement);
#endif

                Ret=FPGA_SetPosition(pCiA402Axis->i8CurrentAxisNo,PositionIncrement);/* 发送增量位置到FPGA */

                if(Ret==1)/* 超最大频率 */
                {
                    CiA402_LocalError(pCiA402Axis->i8CurrentAxisNo,0x0003);
                }

                /* 跟随误差实际值 */
                pCiA402Axis->Objects.objFollowingErrActualValue = pCiA402Axis->Objects.objPositionActualValue - pCiA402Axis->Objects.objTargetPosition;
                /* 判断跟随误差 */
                (pCiA402Axis->Objects.objPositionActualValue > pCiA402Axis->Objects.objTargetPosition)?
                (FollwingErrorValue = pCiA402Axis->Objects.objPositionActualValue - pCiA402Axis->Objects.objTargetPosition):
                (FollwingErrorValue = pCiA402Axis->Objects.objTargetPosition - pCiA402Axis->Objects.objPositionActualValue);

                if(FollwingErrorValue > pCiA402Axis->Objects.objFollowingErrorWindow)
                {
                    CiA402_LocalError(pCiA402Axis->i8CurrentAxisNo,0x0004);/* 发生跟随误差错误 */
                }

                break;
            }
        }
        else
        {
            pCiA402Axis->Objects.objStatusWord |= STATUSWORD_INTERNAL_LIMIT;/* 标记超限位 */

            if((pCiA402Axis->Objects.objDigitalInputs & 0x00010000)!=0)/* 急停信号有效 */
            {
                CiA402_LocalError(pCiA402Axis->i8CurrentAxisNo,0x0001);
            }
            else
            {
                CiA402_LocalError(pCiA402Axis->i8CurrentAxisNo,0x0002);/* 超软件限位 */
            }

            Ret=FPGA_SetPosition(pCiA402Axis->i8CurrentAxisNo,0);/* 发送增量位置 0 到FPGA */
        }
    }
    else
    {
        Ret=FPGA_SetPosition(pCiA402Axis->i8CurrentAxisNo,0);/* 发送增量位置 0 到FPGA */
    }

    pCiA402Axis->i32TargetPosition += PositionIncrement;/* 保存最后收到的目标位置 */
//	pCiA402Axis->i32TargetPosition = pCiA402Axis->Objects.objTargetPosition;/* 保存最后收到的目标位置 */

    /* 读当前速度 */
    i32TargetVelocity=FPGA_GetVelocity(pCiA402Axis->i8CurrentAxisNo);

    /* 读取实当前际位置/FPGA状态 */
    FPGA_GetPosition(pCiA402Axis->i8CurrentAxisNo,&pCiA402Axis->Objects.objPositionActualValue,&FPGA_Status);


    /* 实际速度 */
    pCiA402Axis->Objects.objVelocityActualValue= i32TargetVelocity;

    /*更新操作模式*/
    pCiA402Axis->Objects.objModesOfOperationDisplay = pCiA402Axis->Objects.objModesOfOperation;
}
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return TRUE if moving on predefined ramp is finished

 \brief    CiA402-TransitionAction
 \brief this function shall calculate the desired Axis input values to move on a predefined ramp
 \brief if the ramp is finished return TRUE
*////////////////////////////////////////////////////////////////////////////////////////
BOOL CiA402_TransitionAction(INT16 Characteristic,TCiA402Axis *pCiA402Axis)
{
    switch(Characteristic)
    {

    case SLOW_DOWN_RAMP:

        return TRUE;
    //    break;
    case QUICKSTOP_RAMP:

        return TRUE;
    //  break;
    case STOP_ON_CURRENT_LIMIT:

        return TRUE;
    //  break;
    case STOP_ON_VOLTAGE_LIMIT:

        return TRUE;
    //   break;
    default:
        break;
    }
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief    CiA402-Application
 \brief check if a state transition is pending and pass desired ramp-code to CiA402TransitionAction()
 \brief if this functions returns true the state transition is finished.
*////////////////////////////////////////////////////////////////////////////////////////
void CiA402_Application(TCiA402Axis *pCiA402Axis)
{
    /*clear "Drive follows the command value" flag if the target values from the master overwritten by the local application*/
    if(pCiA402Axis->bAxisFunctionEnabled)
    {
        if(pCiA402Axis->u16PendingOptionCode != 0 &&
                (pCiA402Axis->Objects.objModesOfOperationDisplay == CYCLIC_SYNC_POSITION_MODE ||
                 pCiA402Axis->Objects.objModesOfOperationDisplay == CYCLIC_SYNC_VELOCITY_MODE))
        {
            pCiA402Axis->Objects.objStatusWord &= ~ STATUSWORD_DRIVE_FOLLOWS_COMMAND;
        }
        else
        {
            pCiA402Axis->Objects.objStatusWord |= STATUSWORD_DRIVE_FOLLOWS_COMMAND;
        }
    }

    /* 有错误发生 */
    if(pCiA402Axis->i8ErrorHappened==1)
    {
        pCiA402Axis->i16State = STATE_FAULT_REACTION_ACTIVE;
        pCiA402Axis->i8ErrorHappened=0;
    }

    switch(pCiA402Axis->u16PendingOptionCode)
    {
    case 0x605A:
        /*state transition 11 is pending analyse shutdown option code (0x605A)*/
    {
        UINT16 ramp = pCiA402Axis->Objects.objQuickStopOptionCode;
        /*masked and execute specified quick stop ramp characteristic */
        if(pCiA402Axis->Objects.objQuickStopOptionCode > 4 && pCiA402Axis->Objects.objQuickStopOptionCode <9)
        {
            if(pCiA402Axis->Objects.objQuickStopOptionCode == 5)
                ramp = 1;
            if(pCiA402Axis->Objects.objQuickStopOptionCode == 6)
                ramp = 2;
            if(pCiA402Axis->Objects.objQuickStopOptionCode == 7)
                ramp = 3;
            if(pCiA402Axis->Objects.objQuickStopOptionCode == 8)
                ramp = 4;
        }

        if(CiA402_TransitionAction(ramp,pCiA402Axis))
        {
            /*quick stop ramp is finished complete state transition*/
            pCiA402Axis->u16PendingOptionCode = 0x0;
            if(pCiA402Axis->Objects.objQuickStopOptionCode > 0 && pCiA402Axis->Objects.objQuickStopOptionCode < 5)
            {
                pCiA402Axis->i16State = STATE_SWITCH_ON_DISABLED;    //continue state transition 12
            }
            else if(pCiA402Axis->Objects.objQuickStopOptionCode > 4 && pCiA402Axis->Objects.objQuickStopOptionCode < 9)
                pCiA402Axis->Objects.objStatusWord |= STATUSWORD_TARGET_REACHED;
        }
    }
    break;
    case 0x605B:
        /*state transition 8 is pending analyse shutdown option code (0x605B)*/
    {
        if(CiA402_TransitionAction(pCiA402Axis->Objects.objShutdownOptionCode,pCiA402Axis))
        {
            /*shutdown ramp is finished complete state transition*/
            pCiA402Axis->u16PendingOptionCode = 0x0;
            pCiA402Axis->i16State = STATE_READY_TO_SWITCH_ON;    //continue state transition 8
        }
    }
    break;
    case 0x605C:
        /*state transition 5 is pending analyse Disable operation option code (0x605C)*/
    {
        if(CiA402_TransitionAction(pCiA402Axis->Objects.objDisableOperationOptionCode,pCiA402Axis))
        {
            /*disable operation ramp is finished complete state transition*/
            pCiA402Axis->u16PendingOptionCode = 0x0;
            pCiA402Axis->i16State = STATE_SWITCHED_ON;    //continue state transition 5
        }
    }
    break;

    case 0x605E:
        /*state transition 14 is pending analyse Fault reaction option code (0x605E)*/
    {
        if(CiA402_TransitionAction(pCiA402Axis->Objects.objFaultReactionCode,pCiA402Axis))
        {
            /*fault reaction ramp is finished complete state transition*/
            pCiA402Axis->u16PendingOptionCode = 0x0;
            pCiA402Axis->i16State = STATE_FAULT;    //continue state transition 14
        }
    }
    break;
    default:
        //pending transition code is invalid => values from the master are used
        //pCiA402Axis->Objects.objStatusWord |= STATUSWORD_DRIVE_FOLLOWS_COMMAND;
        break;
    }

    if(bDcSyncActive
            && (pCiA402Axis->u32CycleTime != 0)
            //&& ((pCiA402Axis->Objects.objSupportedDriveModes >> (pCiA402Axis->Objects.objModesOfOperation - 1)) & 0x1)
      ) //Mode of Operation (0x6060) - 1 specifies the Bit within Supported Drive Modes (0x6502)
    {
        CiA402_DummyMotionControl(pCiA402Axis);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param     index                 index of the requested object.
 \param     subindex                subindex of the requested object.
 \param     dataSize                received data size of the SDO Download
 \param     pObjEntry            handle to the dictionary object returned by
                                     OBJ_GetObjectHandle which was called before
 \param    pData                    Pointer to the buffer where the written data can be copied from
 \param    bCompleteAccess    Indicates if a complete write of all subindices of the
                                     object shall be done or not

 \return    result of the write operation (0 (success) or an abort code (ABORTIDX_.... defined in
            sdosrv.h))

 \brief    This function writes "Configured Modules" Object 0xF030
*////////////////////////////////////////////////////////////////////////////////////////

UINT8 Write0xF030( UINT16 index, UINT8 subindex, UINT32 dataSize, UINT16 MBXMEM * pData, UINT8 bCompleteAccess )
{

    UINT8 FPGA_IOMode[4]= {1,1,1,1};
    UINT16 i = subindex;
    UINT16 maxSubindex = sConfiguredModuleIdentList.u16SubIndex0;
    OBJCONST TSDOINFOENTRYDESC OBJMEM *pEntry;
    /* lastSubindex is used for complete access to make loop over the requested entries
       to be read, we initialize this variable with the requested subindex that only
       one loop will be done for a single access */
    UINT8 lastSubindex = subindex;

    if ( bCompleteAccess )
    {
        if ( subindex == 0 )
        {
            /* we change the subindex 0 */
            maxSubindex = (UINT8) pData[0];
        }

        /* we write until the maximum subindex */
        lastSubindex = (UINT8)maxSubindex;
    }
    else if ( subindex > maxSubindex )
        /* the maximum subindex is reached */
        return ABORTIDX_SUBINDEX_NOT_EXISTING;
    else
    {
        /* we check the write access for single accesses here, a complete write access
           is allowed if at least one entry is writable (in this case the values for the
            read only entries shall be ignored) */
        /* we get the corresponding entry description */
        pEntry = &asEntryDesc0xF030[subindex];

        /* check if we have write access (bits 3-5 (PREOP, SAFEOP, OP) of ObjAccess)
           by comparing with the actual state (bits 1-3 (PREOP, SAFEOP, OP) of AL Status) */
        if ( ((UINT8) ((pEntry->ObjAccess & ACCESS_WRITE) >> 2)) < (nAlStatus & STATE_MASK) )
        {
            /* we don't have write access */
            if ( (pEntry->ObjAccess & ACCESS_WRITE) == 0 )
            {
                /* it is a read only entry */
                return ABORTIDX_READ_ONLY_ENTRY;
            }
            else
            {
                /* we don't have write access in this state */
                return ABORTIDX_IN_THIS_STATE_DATA_CANNOT_BE_READ_OR_STORED;
            }
        }
    }

    /* we use the standard write function */
    for (i = subindex; i <= lastSubindex; i++)
    {
        /* we have to copy the entry */
        if (i == 0)
        {
            /*check if the value for subindex0 is valid */
            if(MAX_AXES < (UINT8) pData[0])
            {
                return ABORTIDX_VALUE_TOO_GREAT;
            }

            sConfiguredModuleIdentList.u16SubIndex0 =  pData[0];

            /* we increment the destination pointer by 2 because the subindex 0 will be
                transmitted as UINT16 for a complete access */
            pData++;
        }
        else
        {
            UINT32 CurValue = sConfiguredModuleIdentList.aEntries[(i-1)];
            UINT16 MBXMEM *pVarPtr = (UINT16 MBXMEM *) &sConfiguredModuleIdentList.aEntries[(i-1)];

            pVarPtr[0] = pData[0];
            pVarPtr[1] = pData[1];
            pData += 2;

            /*Check if valid value was written*/
            if((sConfiguredModuleIdentList.aEntries[(i-1)] != (UINT32)CSV_CSP_MODULE_ID)
                    && (sConfiguredModuleIdentList.aEntries[(i-1)] != (UINT32)CSP_MODULE_ID)
                    && (sConfiguredModuleIdentList.aEntries[(i-1)] != (UINT32)CSV_MODULE_ID)
                    && (sConfiguredModuleIdentList.aEntries[(i-1)] != 0))
            {
                /*write previous value*/
                sConfiguredModuleIdentList.aEntries[(i-1)] = CurValue;

                /*reset subindex 0 (if required)*/
                if(sConfiguredModuleIdentList.aEntries[(i-1)] != 0)
                {
                    sConfiguredModuleIdentList.u16SubIndex0 = i;
                }
                else
                {
                    /*current entry is 0 => set subindex0 value i-1*/
                    sConfiguredModuleIdentList.u16SubIndex0 = (i-1);
                }


                return ABORTIDX_VALUE_EXCEEDED;
            }
        }
    }

    /*Update PDO assign objects and 0xF010 (Module Profile List)*/
    {
        UINT8 cnt = 0;

        /*Update 0xF010.0 */
        sModuleProfileInfo.u16SubIndex0 = sConfiguredModuleIdentList.u16SubIndex0;

        /*Update PDO assign SI0*/
        sRxPDOassign.u16SubIndex0 = sConfiguredModuleIdentList.u16SubIndex0+MAX_GlOABLE_PDO;/* MAX_GlOABLE_PDO:全局PDO个数 */
        sTxPDOassign.u16SubIndex0 = sConfiguredModuleIdentList.u16SubIndex0+MAX_GlOABLE_PDO;

        for (cnt = 0 ; cnt < sConfiguredModuleIdentList.u16SubIndex0; cnt++)
        {
            /*all Modules have the same profile number*/
            sModuleProfileInfo.aEntries[cnt] = DEVICE_PROFILE_TYPE;

            switch(sConfiguredModuleIdentList.aEntries[cnt])
            {
            case CSP_MODULE_ID:
                sRxPDOassign.aEntries[cnt+MAX_GlOABLE_PDO] = (0x1600 +(0x10*cnt));/* MAX_GlOABLE_PDO:全局PDO个数 */
                sTxPDOassign.aEntries[cnt+MAX_GlOABLE_PDO] = (0x1A00 +(0x10*cnt));
                break;

                {   /*
                    case CSV_CSP_MODULE_ID:
                       sRxPDOassign.aEntries[cnt] = (0x1600 +(0x10*cnt));
                       sTxPDOassign.aEntries[cnt] = (0x1A00 +(0x10*cnt));
                    break;
                    case CSP_MODULE_ID:
                       sRxPDOassign.aEntries[cnt] = (0x1601 +(0x10*cnt));
                       sTxPDOassign.aEntries[cnt] = (0x1A01 +(0x10*cnt));
                    break;
                    case CSV_MODULE_ID:
                       sRxPDOassign.aEntries[cnt] = (0x1602 +(0x10*cnt));
                       sTxPDOassign.aEntries[cnt] = (0x1A02 +(0x10*cnt));
                    break;
                    */
                }
            default:
                sRxPDOassign.aEntries[cnt+MAX_GlOABLE_PDO] = 0;/* MAX_GlOABLE_PDO:全局PDO个数 */
                sTxPDOassign.aEntries[cnt+MAX_GlOABLE_PDO] = 0;

                sModuleProfileInfo.aEntries[cnt] = 0;
                break;
            }

            if(sConfiguredModuleIdentList.aEntries[cnt]==CSP_MODULE_ID)
            {
                FPGA_IOMode[cnt]=0;/* 0:轴模式。这里根据F030子索引个数来判断启用的几轴，如子索引为4，说明启用了4个轴 */
            }
            else
            {
                FPGA_IOMode[cnt]=1;
            }
        }

        for (cnt = sConfiguredModuleIdentList.u16SubIndex0 ; cnt < 4; cnt++)
        {
            FPGA_IOMode[cnt]=1;
        }


        /* 设置FPGA IO模式 */
        __set_PRIMASK(1);  	/* 关中断 */
        FPGA_SetIOMode(FPGA_IOMode[0],FPGA_IOMode[1],FPGA_IOMode[2],FPGA_IOMode[3]);
        __set_PRIMASK(0);  	/* 开中断 */
    }

    return 0;
}

/*-----------------------------------------------------------------------------------------
------
------    generic functions
------
-----------------------------------------------------------------------------------------*/
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
    UINT32 Sync0CycleTime = 0;

    if(sConfiguredModuleIdentList.u16SubIndex0 == 0)
    {
        /* Object 0xF030 was not written before
        * => update object 0xF010 (Module profile list) and 0xF050 (Detected Module List)*/
        UINT8 cnt = 0;
        /*Update 0xF010.0 */
        sModuleProfileInfo.u16SubIndex0 = sRxPDOassign.u16SubIndex0-MAX_GlOABLE_PDO;/* 减1，是因为至少有一个总的(RXPD0 x17A0) */

        /*Update 0xF050.0*/
        sDetectedModuleIdentList.u16SubIndex0 = sRxPDOassign.u16SubIndex0-MAX_GlOABLE_PDO;/* 减1，是因为至少有一个总的(RXPD0 x17A0) */

        for (cnt = 0 ; cnt < sRxPDOassign.u16SubIndex0; cnt++)
        {
            if(sRxPDOassign.aEntries[cnt]==0x17A0)/* 0x17A0是系统的PDO，不属于module */
            {
                continue;
            }

            else
            {
                uint16_t AxisNo;
                AxisNo=((sRxPDOassign.aEntries[cnt]& 0x00F0)>>4);/* 判断第几通道，如1600：第0轴，1610：第1轴，1620：第二轴，1630：第三轴 */


                /*all Modules have the same profile number*/
                sModuleProfileInfo.aEntries[AxisNo] = DEVICE_PROFILE_TYPE;

                switch((sRxPDOassign.aEntries[AxisNo] & 0x000F)) //get only identification of the PDO mapping object
                {
                /* 本项目只支持CSP模式 */
                case 0x0:   //csv/csp PDO selected
                    sDetectedModuleIdentList.aEntries[AxisNo] = CSP_MODULE_ID;
                    break;
                    {   /*
                        case 0x0:   //csv/csp PDO selected
                        	sDetectedModuleIdentList.aEntries[cnt] = CSV_CSP_MODULE_ID;
                        break;
                        case 0x1:   //csp PDO selected
                        	sDetectedModuleIdentList.aEntries[cnt] = CSP_MODULE_ID;
                        break;
                        case 0x2:   //csv PDO selected
                        	sDetectedModuleIdentList.aEntries[cnt] = CSV_MODULE_ID;
                        break;
                        */
                    }
                }
            }
        }
    }

    HW_EscReadDWord(Sync0CycleTime, ESC_DC_SYNC0_CYCLETIME_OFFSET);
    Sync0CycleTime = SWAPDWORD(Sync0CycleTime);

    /*Init CiA402 structure if the device is in SM Sync mode
    the CiA402 structure will be Initialized after calculation of the Cycle time*/
    if(bDcSyncActive == TRUE)
    {
        UINT16 i;
        Sync0CycleTime = Sync0CycleTime / 1000; /*get cycle time in us,原始值单位为ns,这里除以1000，转换成微秒 */
        for(i = 0; i< MAX_AXES; i++)
        {
            if(LocalAxes[i].bAxisIsActive)
            {
                LocalAxes[i].u32CycleTime = Sync0CycleTime;
                FPGA_ConfigCycleTime(i,Sync0CycleTime);/* 发送到FPGA */
            }
        }
    }

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
void InputOutputInit(void);
UINT16 APPL_StartOutputHandler(void)
{
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
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\return     0(ALSTATUSCODE_NOERROR), NOERROR_INWORK
\param      pInputSize  pointer to save the input process data length
\param      pOutputSize  pointer to save the output process data length

\brief    This function calculates the process data sizes from the actual SM-PDO-Assign
            and PDO mapping
*////////////////////////////////////////////////////////////////////////////////////////
UINT16 APPL_GenerateMapping(UINT16* pInputSize,UINT16* pOutputSize)
{
    UINT16 result = ALSTATUSCODE_NOERROR;
    UINT16 PDOAssignEntryCnt = 0;
    UINT8 AxisIndex = 0;
    UINT16 u16cnt = 0;
    UINT16 InputSize = 0;
    UINT16 OutputSize = 0;
    TOBJECT OBJMEM *pDiCEntry = NULL;
    static UINT16 ModulePDOAssignEntryPosition=0;

    if(sRxPDOassign.u16SubIndex0 > MAX_AXES+MAX_GlOABLE_PDO)/* MAX_GlOABLE_PDO:全局PDO个数 */
        return ALSTATUSCODE_NOVALIDOUTPUTS;

    /*Update object dictionary according to activated axis
    which axes are activated is calculated by object 0x1C12*/
    AxisActiveBit=0;
    for(PDOAssignEntryCnt = 0; PDOAssignEntryCnt < sRxPDOassign.u16SubIndex0; PDOAssignEntryCnt++)
    {
        /* entry有可能是0x0000.twinCat平台上当slot0不添加模块，Slot1添加模块时，下发的PDO一次为0x17A0/0x0000/0x1610 */
        if((sRxPDOassign.aEntries[PDOAssignEntryCnt]==0x0000)||
                (sRxPDOassign.aEntries[PDOAssignEntryCnt]==0x17A0)
          )
        {
            continue;
        }

        /*The PDO mapping objects are specified with an 0x10 offset => get the axis index*/
        AxisIndex = (sRxPDOassign.aEntries[PDOAssignEntryCnt] & 0xF0) >> 4;
        AxisActiveBit|=(0x01<<AxisIndex);

        if(!LocalAxes[AxisIndex].bAxisIsActive)
        {
            /*add objects to dictionary*/
            pDiCEntry = LocalAxes[AxisIndex].ObjDic;

            while(pDiCEntry->Index != 0xFFFF)
            {
                result = COE_AddObjectToDic(pDiCEntry);

                if(result != 0)
                {
                    return result;
                }

                pDiCEntry++;    //get next entry
            }

            LocalAxes[AxisIndex].bAxisIsActive = TRUE;
        }
    }

//	for(PDOAssignEntryCnt = 0;PDOAssignEntryCnt<4;PDOAssignEntryCnt++)
//	{
//	    if( (LocalAxes[PDOAssignEntryCnt].bAxisIsActive==TRUE) &&
//			( 0==(AxisActiveBit&(0x01<<PDOAssignEntryCnt)) )
// 		  )
//		{
//		    /*add objects to dictionary*/
//			pDiCEntry = LocalAxes[PDOAssignEntryCnt].ObjDic;

//			while(pDiCEntry->Index != 0xFFFF)
//			{
//				COE_RemoveDicEntry(pDiCEntry->Index);

//				pDiCEntry++;    //get next entry
//			}

//			LocalAxes[PDOAssignEntryCnt].bAxisIsActive = FALSE;
//		}
//	}


    /*Scan object 0x1C12 RXPDO assign*/
    ModulePDOAssignEntryPosition=0;
    /* 整个设备的PDO */
    for(PDOAssignEntryCnt = 0; PDOAssignEntryCnt < sRxPDOassign.u16SubIndex0; PDOAssignEntryCnt++)
    {
        if((sRxPDOassign.aEntries[PDOAssignEntryCnt]==0x0000)||
                (sRxPDOassign.aEntries[PDOAssignEntryCnt]==0x17A0)
          )
        {
            switch(sRxPDOassign.aEntries[PDOAssignEntryCnt])
            {
            case 0x17A0:
                for(u16cnt =0 ; u16cnt < DeviceRxPDOMap.u16SubIndex0; u16cnt++)
                {
                    OutputSize +=(UINT16)DeviceRxPDOMap.aEntries[u16cnt]& 0xFF;
                }

                ModulePDOAssignEntryPosition++;
                break;
            default:
                break;
            }

        }
        else/* module的PDO */
        {
            uint16_t AxisNo;
            AxisNo=((sRxPDOassign.aEntries[PDOAssignEntryCnt]& 0x00F0)>>4);/* 判断第几通道，如1600：第0轴，1610：第1轴，1620：第二轴，1630：第三轴 */

            switch ((sRxPDOassign.aEntries[PDOAssignEntryCnt-ModulePDOAssignEntryPosition] & 0x000F))    /* 最低字节表示支持的模式，如1600表示CSP、1601表示PP、1602表示PV，本例中最低字节是00 */
            {
            /*drive mode supported  pp: bit 0
            						pv: bit 2
            						csp (cyclic sync position) : bit 7
            						csV (cyclic sync position) : bit 8
            						homing: bit5*/
            case 0:
                /*drive mode supported    homing : bit 5
                						csp (cyclic sync position) : bit 7*/
                LocalAxes[AxisNo].Objects.objSupportedDriveModes = 0x1A0;
                for(u16cnt =0 ; u16cnt < LocalAxes[AxisNo].Objects.sRxPDOMap0.u16SubIndex0; u16cnt++)
                {
                    OutputSize +=(UINT16)(LocalAxes[AxisNo].Objects.sRxPDOMap0.aEntries[u16cnt] & 0xFF);
                }
                break;

            case 1:
                /*drive mode supported    csp (cyclic sync position) : bit 7*/
                LocalAxes[AxisNo].Objects.objSupportedDriveModes = 0x80;
                for(u16cnt =0 ; u16cnt < LocalAxes[AxisNo].Objects.sRxPDOMap1.u16SubIndex0; u16cnt++)
                {
                    OutputSize +=(UINT16)(LocalAxes[AxisNo].Objects.sRxPDOMap1.aEntries[u16cnt] & 0xFF);
                }
                break;
            case 2:
                /*drive mode supported    csv(cyclic sync velocity) : bit 8*/
                LocalAxes[AxisNo].Objects.objSupportedDriveModes= 0x100;
                for(u16cnt =0 ; u16cnt < LocalAxes[AxisNo].Objects.sRxPDOMap2.u16SubIndex0; u16cnt++)
                {
                    OutputSize += (UINT16)(LocalAxes[AxisNo].Objects.sRxPDOMap2.aEntries[u16cnt] & 0xFF);;
                }
                break;

            }
        }
    }
    OutputSize = OutputSize >> 3;
    ModulePDOAssignEntryPosition=0;
    if(result == 0)
    {
        /*Scan Object 0x1C13 TXPDO assign*/

        /* 整个设备的PDO */
        for(PDOAssignEntryCnt = 0; PDOAssignEntryCnt < sTxPDOassign.u16SubIndex0; PDOAssignEntryCnt++)/* 1个PDO */
        {
            if((sTxPDOassign.aEntries[PDOAssignEntryCnt]==0x0000)||
                    (sTxPDOassign.aEntries[PDOAssignEntryCnt]==0x1BA0)
              )
            {
                switch(sTxPDOassign.aEntries[PDOAssignEntryCnt])
                {
                case 0x1BA0:
                    for(u16cnt =0 ; u16cnt < DeviceTxPDOMap.u16SubIndex0; u16cnt++)
                    {
                        InputSize +=(UINT16)DeviceTxPDOMap.aEntries[u16cnt]& 0xFF;
                    }

                    ModulePDOAssignEntryPosition++;
                    break;
                default:
                    break;
                }
            }
            else/* module的PDO */
            {
                uint8_t AxisNo;
                AxisNo=(sTxPDOassign.aEntries[PDOAssignEntryCnt-ModulePDOAssignEntryPosition]& 0x00F0)>>8;

                switch ((sTxPDOassign.aEntries[PDOAssignEntryCnt-ModulePDOAssignEntryPosition] & 0x000F))    //mask Axis type (supported modes)
                {
                case 0: /*csp/csv*/
                    for(u16cnt =0 ; u16cnt < LocalAxes[AxisNo].Objects.sTxPDOMap0.u16SubIndex0; u16cnt++)
                    {
                        InputSize +=(UINT16)(LocalAxes[AxisNo].Objects.sTxPDOMap0.aEntries[u16cnt] & 0xFF);
                    }
                    break;
                case 1: /*csp*/
                    for(u16cnt =0 ; u16cnt < LocalAxes[AxisNo].Objects.sTxPDOMap1.u16SubIndex0; u16cnt++)
                    {
                        InputSize +=(UINT16)(LocalAxes[AxisNo].Objects.sTxPDOMap1.aEntries[u16cnt] & 0xFF);
                    }
                    break;
                case 2: /*csv*/
                    for(u16cnt =0 ; u16cnt < LocalAxes[AxisNo].Objects.sTxPDOMap2.u16SubIndex0; u16cnt++)
                    {
                        InputSize +=(UINT16)(LocalAxes[AxisNo].Objects.sTxPDOMap2.aEntries[u16cnt] & 0xFF);
                    }
                    break;
                default:
                    break;
                }
            }
        }
        InputSize = InputSize >> 3;
    }

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
void APPL_InputMapping(UINT16* pData)
{
    UINT16 j = 0;
    UINT8 i;
    UINT16 *pTmpData = (UINT16 *)pData;
    UINT8 AxisIndex;
    UINT16 PdoIndex;/* PDO索引 */
    UINT8 PdoSbuIndex;/* PDO子索引 */
    UINT32* pPDO=NULL;
    UINT8 PDONum=0;


    for (j = 0; j < sTxPDOassign.u16SubIndex0; j++)/* 0x1c13的子索引个数，即表示有几个轴 */
    {
        if((sTxPDOassign.aEntries[j]==0x1BA0)||(sTxPDOassign.aEntries[j]==0x0000))/* 非module PDo*/
        {
            switch (sTxPDOassign.aEntries[j])
            {
            case 0x1BA0:
                ExternalInput=ReadInputAll();
                *pTmpData ++= ExternalInput; /* 外部输入 */

                *pTmpData ++= GlobalErrorStatus; /* 全局错误标记 */

                break;
            default:
                break;

            }
        }
        /* module PDO */
        else if((sTxPDOassign.aEntries[j]==0x1A00)||(sTxPDOassign.aEntries[j]==0x1A10)||(sTxPDOassign.aEntries[j]==0x1A20)||(sTxPDOassign.aEntries[j]==0x1A30))
        {
            /*The Axis index is based on the PDO mapping offset (0x10)*/
            AxisIndex = ((sTxPDOassign.aEntries[j] & 0xF0) >> 4);

            switch ((sTxPDOassign.aEntries[j]& 0x000F))
            {
            /* 本项目中只有 0，是有效的。因为PG模块只有一种模式,不需要选择 */
            case 0:    //copy csp/csv TxPDO entries
            {
                pPDO = &LocalAxes[AxisIndex].Objects.sTxPDOMap0.aEntries[0];/* 0x1600包含的第一个对象的地址 */
                PDONum = LocalAxes[AxisIndex].Objects.sTxPDOMap0.u16SubIndex0;/* 0x1600的子索引个数 */
                break;
            }

            {   /*
                case 1:
                {
                	pPDO = &LocalAxes[AxisIndex].Objects.sTxPDOMap1.aEntries[0];
                	PDONum = LocalAxes[AxisIndex].Objects.sTxPDOMap1.u16SubIndex0;
                	break;
                }
                case 2:
                {
                	pPDO = &LocalAxes[AxisIndex].Objects.sTxPDOMap2.aEntries[0];
                	PDONum = LocalAxes[AxisIndex].Objects.sTxPDOMap2.u16SubIndex0;
                	break;
                }
                case 3:
                {
                	pPDO = &LocalAxes[AxisIndex].Objects.sTxPDOMap3.aEntries[0];
                	PDONum = LocalAxes[AxisIndex].Objects.sTxPDOMap3.u16SubIndex0;
                	break;
                }*/
            }
            default:
                break;
            }

            for(i =0 ; i < PDONum; i++)
            {
                PdoIndex = (*(pPDO+i) >> 16)-(0x800*AxisIndex);/* 每个轴PDO偏移0x800 */
                PdoSbuIndex = ((*(pPDO+i)) & 0x0000FF00)>>8;

                switch(PdoIndex)
                {
                case 0x603F:/* 错误码，0x603F */
                {
                    *pTmpData ++= LocalAxes[AxisIndex].Objects.objErrorCode;
                    break;
                }
                case 0x6041: /* 状态字 0x6041 */
                {
                    *pTmpData ++=  SWAPWORD(LocalAxes[AxisIndex].Objects.objStatusWord);
                    break;
                }

                case 0x6061: /* 运行模式，0x6061 */
                {
                    *pTmpData++ = LocalAxes[AxisIndex].Objects.objModesOfOperationDisplay & 0x00FF;
                    break;
                }
                case 0x6064: /* 实际位置，0x6064 */
                {
                    *(int32_t *)pTmpData  = LocalAxes[AxisIndex].Objects.objPositionActualValue;
                    pTmpData+=2;
                    break;
                }
                case 0x606C: /* 实际速度，0x606C */
                {
                    *(int32_t *)pTmpData  = LocalAxes[AxisIndex].Objects.objVelocityActualValue;
                    pTmpData+=2;
                    break;
                }

                case 0x60F4: /* Following Error Actual Value  */
                {
                    *(int32_t *)pTmpData=LocalAxes[AxisIndex].Objects.objFollowingErrActualValue;
                    pTmpData+=2;
                    break;
                }
                case 0x60FD: /* 输入IO值 ，0x60FD  */
                {
                    LocalAxes[AxisIndex].Objects.objDigitalInputs=ReadInputStatus(AxisIndex);/*读输入状态*/

                    *(int32_t *)pTmpData=LocalAxes[AxisIndex].Objects.objDigitalInputs;
                    pTmpData+=2;
                    break;
                }
                case 0x6502: /* 支持的操作模式 (0x6502) */
                {
                    *(int32_t *)pTmpData=LocalAxes[AxisIndex].Objects.objSupportedDriveModes;
                    pTmpData+=2;
                    break;
                }

                default:
                    break;
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\param      pData  pointer to output process data

\brief    This function will copies the outputs from the ESC memory to the local memory
            to the hardware
*////////////////////////////////////////////////////////////////////////////////////////
uint16_t TestCount;
void APPL_OutputMapping(UINT16* pData)
{
    UINT16 j = 0;
    UINT8 i;
    // UINT16  Servo_controlall=0;
    UINT16 *pTmpData = (UINT16 *)pData;
    UINT8 AxisIndex;
    UINT16 PdoIndex;/* PDO索引 */
    UINT8 PdoSbuIndex;/* PDO子索引 */
    UINT32* pPDO=NULL;
    UINT8 PDONum=0;

    if(TestCount++>=2500)/* 模拟主站丢帧测试用 */
    {
        TestCount=0;
        return;
    }

    for (j = 0; j < sRxPDOassign.u16SubIndex0; j++)
    {
        if((sRxPDOassign.aEntries[j]==0x17A0)||(sRxPDOassign.aEntries[j]==0x0000))/* 非module PDo*/
        {
            switch (sRxPDOassign.aEntries[j])
            {
            case 0x17A0:
                ExternalOutput=SWAPWORD(*pTmpData++);/* 外部输出 */
                WriteOutputAll(ExternalOutput);

                ClearError=SWAPWORD(*pTmpData++);/* 错误清零 */
                break;
            default:
                break;
            }
        }
        /* module PDO */
        else if((sRxPDOassign.aEntries[j]==0x1600)||(sRxPDOassign.aEntries[j]==0x1610)||(sRxPDOassign.aEntries[j]==0x1620)||(sRxPDOassign.aEntries[j]==0x1630))
        {
            /*The Axis index is based on the PDO mapping offset (0x10)*/
            AxisIndex = ((sRxPDOassign.aEntries[j] & 0xF0) >> 4);
            /*1600 1610  1620*/

            switch ((sRxPDOassign.aEntries[j] & 0x000F))
            {
            /* 本项目中只有 0，是有效的。因为PG模块只有一种模式,不需要选择 */
            case 0:    //copy csp/csv TxPDO entries
            {
                pPDO = &LocalAxes[AxisIndex].Objects.sRxPDOMap0.aEntries[0];
                PDONum = LocalAxes[AxisIndex].Objects.sRxPDOMap0.u16SubIndex0;
                break;
            }

            {   /*
                case 1:
                {
                	pPDO = &LocalAxes[AxisIndex].Objects.sRxPDOMap1.aEntries[0];
                	PDONum = LocalAxes[AxisIndex].Objects.sRxPDOMap1.u16SubIndex0;
                	break;
                }
                case 2:
                {
                	pPDO = &LocalAxes[AxisIndex].Objects.sRxPDOMap2.aEntries[0];
                	PDONum = LocalAxes[AxisIndex].Objects.sRxPDOMap2.u16SubIndex0;
                	break;
                }
                case 3:
                {
                	pPDO = &LocalAxes[AxisIndex].Objects.sRxPDOMap3.aEntries[0];
                	PDONum = LocalAxes[AxisIndex].Objects.sRxPDOMap3.u16SubIndex0;
                	break;
                }*/
            }

            default:
                break;
            }

            for(i =0 ; i < PDONum; i++)
            {
                PdoIndex = (*(pPDO+i) >> 16)-(0x800*AxisIndex);/* 每个轴PDO偏移0x800 */
                PdoSbuIndex = ((*(pPDO+i)) & 0x0000FF00)>>8;
                switch(PdoIndex)
                {
                case 0x6040:/* 控制字 0x6040*/
                {
                    LocalAxes[AxisIndex].Objects.objControlWord = SWAPWORD(*pTmpData++);
                    Servo_control[AxisIndex].all = LocalAxes[AxisIndex].Objects.objControlWord;
                    break;
                }
                case 0x605A:/* 快速停止代码 0x605A*/
                {
                    LocalAxes[AxisIndex].Objects.objQuickStopOptionCode = SWAPWORD(*pTmpData++);
                    break;
                }
                case 0x6060:/* 0x6060 模式选择 */
                {
                    LocalAxes[AxisIndex].Objects.objModesOfOperation =SWAPWORD(*pTmpData++);
                    break;
                }
                case 0x6065: /* 跟随误差阀值 */
                {
                    *(uint32_t *)pTmpData  = LocalAxes[AxisIndex].Objects.objFollowingErrorWindow;
                    pTmpData+=2;
                    break;
                }
                case 0x607A:/* 目标位置 0x607A  csp/pp共用*/
                {
                    LocalAxes[AxisIndex].Objects.objTargetPosition  = *(uint32_t*)(pTmpData);
                    pTmpData+=2;
                    //debug1(" %d      %d   \r\n",LocalAxes[AxisIndex].Objects.objTargetPosition,LocalAxes[AxisIndex].Objects.objTargetPosition - TagetPosition);
                    //TagetPosition= LocalAxes[AxisIndex].Objects.objTargetPosition ;
                    break;
                }
                case 0x607C: /* 原点偏移 (0x607C) */
                {
                    LocalAxes[AxisIndex].Objects.objHomingOffset = *(uint32_t*)(pTmpData);
                    pTmpData+=2;
                    break;
                }
                case 0x607D: /* 软件限位 (0x607D) */
                {
                    if(PdoSbuIndex==0x01) /* 负向限位 */
                    {
                        LocalAxes[AxisIndex].Objects.objSoftwarePositionLimit.i32MinLimit = *(uint32_t*)(pTmpData);
                        pTmpData+=2;
                    }
                    else if(PdoSbuIndex==0x02)/* 正向限位 */
                    {
                        LocalAxes[AxisIndex].Objects.objSoftwarePositionLimit.i32MaxLimit = *(uint32_t*)(pTmpData);
                        pTmpData+=2;
                    }
                    break;
                }


                case 0x6085:/* 快速停止减速度 */
                {
                    LocalAxes[AxisIndex].Objects.objQuickStopDeclaration  = *(uint32_t*)(pTmpData);
                    pTmpData+=2;
                    break;
                }

                case 0x6098: /* 回零方法 (0x6098) */
                {
                    LocalAxes[AxisIndex].Objects.objHomingMethed = SWAPWORD(*pTmpData++);
                    break;
                }
                case 0x6099: /* 回零速度 (0x6099) */
                {
                    if(PdoSbuIndex==0x01)
                    {
                        LocalAxes[AxisIndex].Objects.objHomingVelocity.Fast = *(uint32_t*)(pTmpData);
                        pTmpData+=2;
                    }
                    else if(PdoSbuIndex==0x02)
                    {
                        LocalAxes[AxisIndex].Objects.objHomingVelocity.Slow = *(uint32_t*)(pTmpData);
                        pTmpData+=2;
                    }
                    break;
                }
                case 0x609A: /* 回零加速度 (0x609A) */
                {
                    LocalAxes[AxisIndex].Objects.objHomingAcceleration = *(uint32_t*)(pTmpData);
                    pTmpData+=2;
                    break;
                }

                case 0x60FE: /* 物理输出 (0x60FE) */
                {
                    if(PdoSbuIndex==0x01) /* 输出 */
                    {
                        LocalAxes[AxisIndex].Objects.objDigitalOutputs.u32PhysicalOutputs = *(uint32_t*)(pTmpData);
                        pTmpData+=2;
                    }
                    else if(PdoSbuIndex==0x02)/* 输出使能 */
                    {
                        LocalAxes[AxisIndex].Objects.objDigitalOutputs.u32BitMask = *(uint32_t*)(pTmpData);
                        pTmpData+=2;
                    }

                    /* 发送到IO模块 */
                    WriteOutputStatus(AxisIndex,LocalAxes[AxisIndex].Objects.objDigitalOutputs.u32BitMask,
                                      LocalAxes[AxisIndex].Objects.objDigitalOutputs.u32PhysicalOutputs);
                    break;
                }
                case 0x60FF:/* 目标速度 0x60FF pV*/
                {
                    LocalAxes[AxisIndex].Objects.objTargetVelocity  = *(uint32_t*)(pTmpData);
                    pTmpData+=2;
                    break;
                }
                default:
                    break;
                }
            }
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////
/**
\brief    This function will called from the synchronisation ISR
            or from the mainloop if no synchronisation is supported
*////////////////////////////////////////////////////////////////////////////////////////
void APPL_Application(void)
{
    UINT16 i;

//    GPIOB->BSRRL = GPIO_Pin_4;
//    for(i = 0; i < MAX_AXES;i++)
//    {
//        if(LocalAxes[i].bAxisIsActive)
//		{
//			LocalAxes[i].i8CurrentAxisNo=i;
//            CiA402_Application(&LocalAxes[i]);
//		}
//    }
//	GPIOB->BSRRH = GPIO_Pin_4;
}

/** @} */
/*
*********************************************************************************************************
*	函 数 名: CiA402_QuickStop
*	功能说明: 快速停止控制。
*	形    参:  *pCiA402Axis
*	返 回 值:
*********************************************************************************************************
*/
void CiA402_QuickStop(TCiA402Axis *pCiA402Axis)
{
    switch(pCiA402Axis->u16PendingOptionCode)
    {
    case 0x605A:
        /*state transition 11 is pending analyse shutdown option code (0x605A)*/
    {
        UINT16 ramp = pCiA402Axis->Objects.objQuickStopOptionCode;
        /*masked and execute specified quick stop ramp characteristic */
        if(pCiA402Axis->Objects.objQuickStopOptionCode > 4 && pCiA402Axis->Objects.objQuickStopOptionCode <9)
        {
            if(pCiA402Axis->Objects.objQuickStopOptionCode == 5)
                ramp = 1;
            if(pCiA402Axis->Objects.objQuickStopOptionCode == 6)
                ramp = 2;
            if(pCiA402Axis->Objects.objQuickStopOptionCode == 7)
                ramp = 3;
            if(pCiA402Axis->Objects.objQuickStopOptionCode == 8)
                ramp = 4;
        }

        if(CiA402_TransitionAction(ramp,pCiA402Axis))
        {
            /*quick stop ramp is finished complete state transition*/
            pCiA402Axis->u16PendingOptionCode = 0x0;
            if(pCiA402Axis->Objects.objQuickStopOptionCode > 0 && pCiA402Axis->Objects.objQuickStopOptionCode < 5)
            {
                pCiA402Axis->i16State = STATE_SWITCH_ON_DISABLED;    //continue state transition 12
            }
            else if(pCiA402Axis->Objects.objQuickStopOptionCode > 4 && pCiA402Axis->Objects.objQuickStopOptionCode < 9)
            {
                pCiA402Axis->Objects.objStatusWord |= STATUSWORD_TARGET_REACHED;
            }
        }
    }
    break;
    case 0x605B:
        /*state transition 8 is pending analyse shutdown option code (0x605B)*/
    {
        if(CiA402_TransitionAction(pCiA402Axis->Objects.objShutdownOptionCode,pCiA402Axis))
        {
            /*shutdown ramp is finished complete state transition*/
            pCiA402Axis->u16PendingOptionCode = 0x0;
            pCiA402Axis->i16State = STATE_READY_TO_SWITCH_ON;    //continue state transition 8
        }
    }
    break;
    case 0x605C:
        /*state transition 5 is pending analyse Disable operation option code (0x605C)*/
    {
        if(CiA402_TransitionAction(pCiA402Axis->Objects.objDisableOperationOptionCode,pCiA402Axis))
        {
            /*disable operation ramp is finished complete state transition*/
            pCiA402Axis->u16PendingOptionCode = 0x0;
            pCiA402Axis->i16State = STATE_SWITCHED_ON;    //continue state transition 5
        }
    }
    break;

    case 0x605E:
        /*state transition 14 is pending analyse Fault reaction option code (0x605E)*/
    {
        if(CiA402_TransitionAction(pCiA402Axis->Objects.objFaultReactionCode,pCiA402Axis))
        {
            /*fault reaction ramp is finished complete state transition*/
            pCiA402Axis->u16PendingOptionCode = 0x0;
            pCiA402Axis->i16State = STATE_FAULT;    //continue state transition 14
        }
    }
    break;
    default:
        //pending transition code is invalid => values from the master are used
        pCiA402Axis->Objects.objStatusWord |= STATUSWORD_DRIVE_FOLLOWS_COMMAND;
        break;
    }
}

/*
*********************************************************************************************************
*	函 数 名: Cia402_GetAxisState
*	功能说明：读轴状态
*	形    参:no:轴号0-3
*	返 回 值:1：使能，0：未使能
*   说    明
*********************************************************************************************************
*/
uint8_t Cia402_GetAxisState(uint8_t no)
{
    if(no>=MAX_AXES)return 0;

    return LocalAxes[no].bAxisFunctionEnabled;
}
/*
*********************************************************************************************************
*	函 数 名: Cia402_GetAxisCount
*	功能说明：配置的轴个数
*	形    参:无
*	返 回 值:
*   说    明：返回4个轴中有几个轴被使用
*********************************************************************************************************
*/
uint8_t Cia402_GetAxisCount(void)
{
    uint8_t cout;

//	cout=(uint8_t)sConfiguredModuleIdentList.u16SubIndex0;
    cout=AxisActiveBit;

    return cout;
}
/*
*********************************************************************************************************
*	函 数 名: Cia402_IsAxisActive
*	功能说明: 读轴配置状态
*	形    参: axis_no:轴号0-3
*	返 回 值: 0:轴未被配置 1：轴以配置
*   说    明:
*********************************************************************************************************
*/
uint8_t Cia402_IsAxisActive(uint8_t axis_no)
{
    if(axis_no>3) return 0;

    return LocalAxes[axis_no].bAxisIsActive;
}



/////////////////////////////////////////////////////////////////////////////////////////
/**

 \brief    This is the main function

*////////////////////////////////////////////////////////////////////////////////////////
//int ecat_slave_main(void)
//{
//	/* initialize the Hardware and the EtherCAT Slave Controller */
//    HW_Init();

//    MainInit();
//
//	CiA402_Init();/* 要放在读EEPROM之前 */
//
//    /*Create basic mapping*/
//    APPL_GenerateMapping(&nPdInputSize,&nPdOutputSize);
//
//    return 0;
//}




















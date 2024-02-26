#ifndef sgm51652hx_H            /* prevent circular inclusions */
#define sgm51652hx_H            /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
/********* Add Your Header Here ******************/
#include "main.h"

/******* Macro Definitions *******************/
#define __SGM51652H4__  0

#define ERR_OK                         0
#define ERR_INVALIDPARAMETER          -1
#define ERR_TIMEOUT                   -2

#define SGM51652_VREF_MV			4096


typedef struct{
    void *spi;    
    void *rstPort;
    uint16_t rstPin;
    void *csPort;
    uint16_t csPin;   
}sgm51652hxDevice_t;

typedef enum{
    NO_OP    = 0,
    STDBY    = 0x8200,
    PWR_DN   = 0x8300,
    RST      = 0x8500,
    AUTO_RST = 0xA000,
    MAN_CH_0 = 0xC000,
    MAN_CH_1 = 0xC400,
    MAN_CH_2 = 0xC800,
    MAN_CH_3 = 0xCC00,
#if __SGM51652H4__ == 0
    MAN_CH_4 = 0xD000,
    MAN_CH_5 = 0xD400,
    MAN_CH_6 = 0xD800,
    MAN_CH_7 = 0xDC00,
#endif
    MAN_AUX  = 0xE000,
    MAX_CMD_NUM
}sgm51652hxCmd_e;

typedef enum{
    AUTO_SEQ_EN    = 1,
    CHANNEL_PD,
    FEATURE_SEL,
    RANGE_CH0 = 5,
    RANGE_CH1,
    RANGE_CH2,
    RANGE_CH3,
#if __SGM51652H4__ == 0
    RANGE_CH4,
    RANGE_CH5,
    RANGE_CH6,
    RANGE_CH7,
#endif
    FLOATING_DETECTION_EN = 0x0D,
    FLOATING_DETECTION_STATUS,
    CMD_READ_BK = 0x3F,
    
}sgm51652hxReg_e;

typedef enum{
    AUTO_CH0_EN    = 0x01,
    AUTO_CH1_EN    = 0x02,
    AUTO_CH2_EN    = 0x04,
    AUTO_CH3_EN    = 0x08,
#if __SGM51652H4__ == 0
	  AUTO_CH4_EN    = 0x10,
	  AUTO_CH5_EN    = 0x20,
	  AUTO_CH6_EN    = 0x40,
	  AUTO_CH7_EN    = 0x80,
#endif   
}sgm51652AutoSEQ_en;

typedef enum{
    INPUT_plusminus_2P5VREF     = 0x00,
    INPUT_plusminus_1P25VREF    = 0x01,
    INPUT_plusminus_0P625VREF   = 0x02,
    INPUT_plus_2P5VREF         	= 0x05,
    INPUT_plus_1P25VREF   			= 0x06,	
#endif   
}sgm51652Input_range;

extern sgm51652hxDevice_t sgm5162h8;

int sgm51652hx_CreateDevice(sgm51652hxDevice_t *device, void *spiPtr, void *rstPort, uint16_t rstPin,
                         void *csPort, uint16_t csPin);
int sgm51652hx_HwReset(sgm51652hxDevice_t *device);
int sgm51652hx_CmdSet(sgm51652hxDevice_t *device, sgm51652hxCmd_e sgm51652hxCmd);
int sgm51652hx_ProgramRegWrite(sgm51652hxDevice_t *device, sgm51652hxReg_e addr, uint8_t val);
int sgm51652hx_ProgramRegRead(sgm51652hxDevice_t *device, sgm51652hxReg_e addr, uint8_t *val);
int sgm51652hx_SignalChannelRead(sgm51652hxDevice_t *device, uint8_t channel, int samples, uint16_t *adcval);
int sgm51652hx_SignalChannelReadWithExtension(sgm51652hxDevice_t *device, uint8_t channel, int samples,
															uint16_t *adcval, uint8_t *channel_addr, uint8_t *device_addr, uint8_t *Input_range);
int sgm51652hx_AutoChannelRead(sgm51652hxDevice_t *device, uint8_t channel,uint8_t channel_num, int samples, uint16_t *adcval);
int sgm51652hx_AutoChannelReadWithExtension(sgm51652hxDevice_t *device, uint8_t channel,uint8_t channel_num, int samples, 
																			uint16_t *adcval, uint8_t *channel_addr, uint8_t *device_addr, uint8_t *Input_range);
int sgm51652hx_InputFloatingStatusRead(sgm51652hxDevice_t *device, uint8_t channel, uint8_t *Float_status);
int sgm5162hx_init(void);

#ifdef __cplusplus
}
#endif /* end of protection macro */
/**
* @} End of "addtogroup common_io_interfacing_apis".
*/

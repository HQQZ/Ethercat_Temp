#include "sgm51652hx.h"
#include "spi.h"
#include "cmsis_os.h"
#include "main.h"

sgm51652hxDevice_t sgm5162h8;

void sgm51652hx_HalDelay_ms(uint32_t ms)
{
	/* Add your code here */
    osDelay(ms);
}

void sgm51652hx_HalGpioPinWrite(void *port, uint16_t pinNum, uint8_t val)
{
	/* Add your code here */
    HAL_GPIO_WritePin((GPIO_TypeDef*)port, pinNum, (GPIO_PinState)val); 
}

int sgm51652hx_HalSpiWriteBytes(void *spiHandler, uint8_t *data, uint8_t len)
{
	int status = ERR_OK;
	if(spiHandler == NULL || data == NULL)    return ERR_INVALIDPARAMETER;
	/* Add your code here */
	status = (int)HAL_SPI_Transmit((SPI_HandleTypeDef*)spiHandler, data, (uint16_t)len, 1000);
	return status;
}

int sgm51652hx_HalSpiReadBytes(void *spiHandler, uint8_t *data, uint8_t len)
{
	int status = ERR_OK;
	if(spiHandler == NULL || data == NULL)    return ERR_INVALIDPARAMETER;
/* Add your code here */
	status = (int)HAL_SPI_Receive((SPI_HandleTypeDef*)spiHandler, data, (uint16_t)len, 1000);
	return status;
}

int sgm51652hx_CreateDevice(sgm51652hxDevice_t *device, void *spiPtr, void *rstPort, uint16_t rstPin,
                         void *csPort, uint16_t csPin)
{
	if(device ==NULL || spiPtr == NULL || rstPort == NULL || csPort == NULL)    return -1;
	device->spi = spiPtr;    
	device->rstPort = rstPort;
	device->rstPin = rstPin;
	device->csPort = csPort;
	device->csPin = csPin;                             
	return 0;
}

int sgm51652hx_HwReset(sgm51652hxDevice_t *device)
{
	sgm51652hx_HalGpioPinWrite(device->rstPort, device->rstPin, 0);
	HAL_Delay(100);
	sgm51652hx_HalGpioPinWrite(device->rstPort, device->rstPin, 1);
	HAL_Delay(10);
	sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 0);
	HAL_Delay(100);
	sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 1);
	return 0;
}

int sgm51652hx_CmdSet(sgm51652hxDevice_t *device, sgm51652hxCmd_e sgm51652hxCmd)
{
	uint8_t cmd[2] = {sgm51652hxCmd>>8, sgm51652hxCmd};
	int status = 0;
	if(device == NULL || sgm51652hxCmd >= MAX_CMD_NUM)    return -1;
	sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 0);
	status = sgm51652hx_HalSpiWriteBytes(device->spi, cmd, 2);
	sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 1);
	return status;
}

int sgm51652hx_ProgramRegWrite(sgm51652hxDevice_t *device, sgm51652hxReg_e addr, uint8_t regval)
	{
	uint8_t cmd[4] = {((uint8_t)addr << 1) | 1, regval, 0, 0};
	int status = 0;
	if(device == NULL || addr > CMD_READ_BK)    return -1;
	sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 0);
	status = sgm51652hx_HalSpiWriteBytes(device->spi, cmd, 4);
	sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 1);
	return status;
}

int sgm51652hx_ProgramRegRead(sgm51652hxDevice_t *device, sgm51652hxReg_e addr, uint8_t *regval)
{
	uint8_t data[2] = { (uint8_t)addr<<1, 0 };
	int status = 0;
	if(device == NULL || addr > CMD_READ_BK)    return -1;
	sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 0);
	status = sgm51652hx_HalSpiWriteBytes(device->spi, data, 1);
	if(status != 0)    return status;
	status = sgm51652hx_HalSpiReadBytes(device->spi, data, 2);
	sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 1);
	*regval = data[1];
	return status;
}

int sgm51652hx_SignalChannelRead(sgm51652hxDevice_t *device, uint8_t channel, int samples, uint16_t *adcval)
{
	int status = 0, i = 0;
	uint8_t readVal[4] = { 0 };
	if(device == NULL || adcval == NULL)    return -1;
	status = sgm51652hx_CmdSet(device, (sgm51652hxCmd_e)(MAN_CH_0 + channel * 0x400));
	if(status != 0)    return status;
	for(i=0;i<samples;i++)
	{
		sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 0);
		status = sgm51652hx_HalSpiReadBytes(device->spi, readVal, 4);
		sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 1);
		adcval[i] = (readVal[2] <<8) | readVal[3];
	}
	return status;
}

int sgm51652hx_SignalChannelReadWithExtension(sgm51652hxDevice_t *device, uint8_t channel, int samples,
															uint16_t *adcval, uint8_t *channel_addr, uint8_t *device_addr, uint8_t *Input_range)
{
	int status = 0, i = 0;
	uint8_t readVal[6] = { 0 };
	if(device == NULL || adcval == NULL)    return -1;
	status = sgm51652hx_CmdSet(device, (sgm51652hxCmd_e)(MAN_CH_0 + channel * 0x400));
	if(status != 0)    return status;
	for(i=0;i<samples;i++)
	{
		sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 0);
		status = sgm51652hx_HalSpiReadBytes(device->spi, readVal, 6);
		sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 1);
		adcval[i] = (readVal[2] <<8) | readVal[3];
		channel_addr[i] = readVal[4] >> 4;
		device_addr[i] = (readVal[4] & 0x0C) >> 2;
		Input_range[i] = ((readVal[4] & 0x03) << 1)| (readVal[5] >> 7);
	}
	return status;
}

int sgm51652hx_AutoChannelRead(sgm51652hxDevice_t *device, uint8_t channel,uint8_t channel_num, int samples, uint16_t *adcval)
{
	int status = 0, i = 0;
	uint8_t readVal[4] = { 0 };
	if(device == NULL || adcval == NULL)    return -1;
	status = sgm51652hx_ProgramRegWrite(device, AUTO_SEQ_EN, channel);
	if(status != 0)    return status;
	status = sgm51652hx_ProgramRegWrite(device, CHANNEL_PD, ~channel); //you can power down channels that not selecet to save power
	if(status != 0)    return status;
	
	status = sgm51652hx_CmdSet(device, AUTO_RST);
	if(status != 0)    return status;
	for(i=0;i<samples*channel_num;i++)
	{
		sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 0);
		status = sgm51652hx_HalSpiReadBytes(device->spi, readVal, 4);
		sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 1);
		adcval[i] = (readVal[2] <<8) | readVal[3];
	}
	return status;
}

int sgm51652hx_AutoChannelReadWithExtension(sgm51652hxDevice_t *device, uint8_t channel,uint8_t channel_num, int samples, 
																			uint16_t *adcval, uint8_t *channel_addr, uint8_t *device_addr, uint8_t *Input_range)
{
	int status = 0, i = 0;
	uint8_t readVal[6] = { 0 };
	if(device == NULL || adcval == NULL)    return -1;
	status = sgm51652hx_ProgramRegWrite(device, AUTO_SEQ_EN, channel);
	if(status != 0)    return status;
	status = sgm51652hx_ProgramRegWrite(device, CHANNEL_PD, ~channel); //you can power down channels that not selecet to save power
	if(status != 0)    return status;
	
	status = sgm51652hx_CmdSet(device, AUTO_RST);
	if(status != 0)    return status;
	for(i=0;i<samples*channel_num;i++)
	{
		sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 0);
		status = sgm51652hx_HalSpiReadBytes(device->spi, readVal, 6);
		sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 1);
		adcval[i] = (readVal[2] <<8) | readVal[3];
		channel_addr[i] = readVal[4] >> 4;
		device_addr[i] = (readVal[4] & 0x0C) >> 2;
		Input_range[i] = ((readVal[4] & 0x03) << 1)| (readVal[5] >> 7);
	}
	return status;
}

int sgm51652hx_InputFloatingStatusRead(sgm51652hxDevice_t *device, uint8_t channel, uint8_t *Float_status)
{
	int status = 0, i = 0;
	uint8_t readVal[4] = { 0 };
	if(device == NULL || Float_status == NULL)    return -1;
	status = sgm51652hx_CmdSet(device, (sgm51652hxCmd_e)(MAN_CH_0 + channel * 0x400));
	if(status != 0)    return status;
	status = sgm51652hx_ProgramRegWrite(device, FLOATING_DETECTION_EN, 0x80);
	for(i=0;i<256;i++)
	{
		sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 0);
		status = sgm51652hx_HalSpiReadBytes(device->spi, readVal, 4);
		sgm51652hx_HalGpioPinWrite(device->csPort, device->csPin, 1);
	}
	status = sgm51652hx_ProgramRegRead(device, FLOATING_DETECTION_STATUS, Float_status);
	return status;
	
}

int sgm5162hx_init(void)
{
	sgm51652hx_HwReset(&sgm5162h8);
	HAL_Delay(100);
	sgm51652hx_CreateDevice(&sgm5162h8, &hspi1, ADC_RST_GPIO_Port, ADC_RST_Pin, ADC_CS_GPIO_Port, ADC_CS_Pin);
}


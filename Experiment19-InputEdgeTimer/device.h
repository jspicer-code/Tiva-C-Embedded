// File:  device.h
// Author: JS
// Date:  7/14/18
// Purpose: I2C device module
// Hardware:  TM4C123 and TM4C1294 Tiva board

#include "hal/HAL.h"

typedef struct {
		
	SysClkFreq_t sysClkFreq;
	
	struct {
		PinDef_t			rsPin;
		PinDef_t			rwPin;
		PinDef_t			enablePin;
		PinDef_t			dataPins[4];
		TimerBlock_t	waitTimer;
	} lcd;
	
	TimerBlock_t edgeTimer;
	PinDef_t edgeTimerPin;
	
} DeviceConfig_t;


int Run(DeviceConfig_t* pConfig);

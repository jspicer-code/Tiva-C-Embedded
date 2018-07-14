// File:  device.h
// Author: JS
// Date:  7/14/18
// Purpose: I2C device module
// Hardware:  TM4C123 and TM4C1294 Tiva board

#include "hal/HAL.h"

typedef struct {
		
	SysClkFreq_t sysClkFreq;
	I2C_Module_t i2cModule;
	PinDef_t slavePins[2];
	PinDef_t buttons[2];
	PinDef_t patternLeds[4];
	PinDef_t colorLeds[3];
	PinDef_t dipSwitches[8];
} DeviceConfig_t;


int Run(DeviceConfig_t* pConfig);

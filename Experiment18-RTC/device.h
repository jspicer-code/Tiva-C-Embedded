// File:  device.h
// Author: JS
// Date:  7/14/18
// Purpose: I2C device module
// Hardware:  TM4C123 and TM4C1294 Tiva board

#include "hal/HAL.h"
#include "drivers/LCD.h"
#include "drivers/Switch5Position.h"

typedef struct {
		
	SysClkFreq_t sysClkFreq;
	I2C_Module_t i2cModule;
	
	struct {
		PinDef_t			rsPin;
		PinDef_t			rwPin;
		PinDef_t			enablePin;
		PinDef_t			dataPins[4];
	} lcd;
	
	Switch5Config_t	switches;
	
} DeviceConfig_t;


int Run(DeviceConfig_t* pConfig);

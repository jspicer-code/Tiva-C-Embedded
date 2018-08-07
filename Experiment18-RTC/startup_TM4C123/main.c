
#include "../Device.h"

DeviceConfig_t config_;

int main()
{
	
	config_.sysClkFreq = SYSCLK_80;
	config_.i2cModule = I2C2;
	
	Run(&config_);
}

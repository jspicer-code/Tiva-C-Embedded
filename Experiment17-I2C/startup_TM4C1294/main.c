
#include "../device.h"

DeviceConfig_t config_;

int main()
{
	
	config_.sysClkFreq = SYSCLK_80;
	config_.i2cModule = I2C2;
	
	config_.slavePins[0] = PINDEF(PORTQ, PIN0);
	config_.slavePins[1] = PINDEF(PORTP, PIN4);
	
	config_.buttons[0] = PINDEF(PORTJ, PIN0);
	config_.buttons[1] = PINDEF(PORTJ, PIN1);
	
	config_.patternLeds[0] = PINDEF(PORTN, PIN1);
	config_.patternLeds[1] = PINDEF(PORTN, PIN0);
	config_.patternLeds[2] = PINDEF(PORTF, PIN4);
	config_.patternLeds[3] = PINDEF(PORTF, PIN0);
	
	config_.colorLeds[0] = PINDEF(PORTD, PIN2);
	config_.colorLeds[1] = PINDEF(PORTP, PIN0);
	config_.colorLeds[2] = PINDEF(PORTP, PIN1);
	
	config_.dipSwitches[7] = PINDEF(PORTM, PIN7);
	config_.dipSwitches[6] = PINDEF(PORTP, PIN5);
	config_.dipSwitches[5] = PINDEF(PORTA, PIN7);
	config_.dipSwitches[4] = PINDEF(PORTA, PIN3);
	config_.dipSwitches[3] = PINDEF(PORTA, PIN2);
	config_.dipSwitches[2] = PINDEF(PORTP, PIN3);
	config_.dipSwitches[1] = PINDEF(PORTQ, PIN1);
	config_.dipSwitches[0] = PINDEF(PORTM, PIN6);
	
	Run(&config_);
}


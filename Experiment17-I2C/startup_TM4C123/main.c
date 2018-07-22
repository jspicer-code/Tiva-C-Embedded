
#include "../device.h"

DeviceConfig_t config_;

int main()
{
	
	config_.sysClkFreq = SYSCLK_80;
	config_.i2cModule = I2C2;
	
	config_.slavePins[0] = PINDEF(PORTB, PIN2);
	config_.slavePins[1] = PINDEF(PORTE, PIN0);
	
	config_.buttons[0] = PINDEF(PORTF, PIN4);
	config_.buttons[1] = PINDEF(PORTF, PIN0);
	
	config_.patternLeds[0] = PINDEF(PORTA, PIN3);
	config_.patternLeds[1] = PINDEF(PORTA, PIN4);
	config_.patternLeds[2] = PINDEF(PORTB, PIN6);
	config_.patternLeds[3] = PINDEF(PORTB, PIN7);
	
	config_.colorLeds[0] = PINDEF(PORTF, PIN1);
	config_.colorLeds[1] = PINDEF(PORTF, PIN3);
	config_.colorLeds[2] = PINDEF(PORTF, PIN2);
	
	config_.dipSwitches[7] = PINDEF(PORTA, PIN2);
	config_.dipSwitches[6] = PINDEF(PORTB, PIN5);
	config_.dipSwitches[5] = PINDEF(PORTB, PIN0);
	config_.dipSwitches[4] = PINDEF(PORTB, PIN1);
	config_.dipSwitches[3] = PINDEF(PORTB, PIN4);
	config_.dipSwitches[2] = PINDEF(PORTA, PIN5);
	config_.dipSwitches[1] = PINDEF(PORTA, PIN6);
	config_.dipSwitches[0] = PINDEF(PORTA, PIN7);
	
	Run(&config_);
}

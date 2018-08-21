
#include "../Device.h"

DeviceConfig_t config_;

int main()
{
	
	config_.sysClkFreq = SYSCLK_80;
	config_.i2cModule = I2C2;
	
	// The data pin assignments 0-3 will be mapped to DB4-DB7 of the display
	config_.lcd.dataPins[0] = PINDEF(PORTD, PIN3);
	config_.lcd.dataPins[1] = PINDEF(PORTC, PIN7);
	config_.lcd.dataPins[2] = PINDEF(PORTB, PIN2);
	config_.lcd.dataPins[3] = PINDEF(PORTB, PIN3);
	config_.lcd.rsPin = PINDEF(PORTE, PIN4);
	config_.lcd.rwPin = PINDEF(PORTC, PIN4);
	config_.lcd.enablePin = PINDEF(PORTC, PIN5);
	
	config_.switches.rightPin = PINDEF(PORTD, PIN2);
	config_.switches.downPin = PINDEF(PORTP, PIN0);
	config_.switches.leftPin = PINDEF(PORTP, PIN1);
	config_.switches.upPin = PINDEF(PORTQ, PIN0);
	config_.switches.centerPin = PINDEF(PORTP, PIN4);
		
	Run(&config_);
}



#include "../Device.h"

DeviceConfig_t config_;

int main()
{
	
	config_.sysClkFreq = SYSCLK_80;
	config_.i2cModule = I2C1;
	
		// The data pin assignments 0-3 will be mapped to DB4-DB7 of the display
	config_.lcd.dataPins[0] = PINDEF(PORTB, PIN6);
	config_.lcd.dataPins[1] = PINDEF(PORTA, PIN4);
	config_.lcd.dataPins[2] = PINDEF(PORTA, PIN3);
	config_.lcd.dataPins[3] = PINDEF(PORTA, PIN2);
	config_.lcd.rsPin = PINDEF(PORTB, PIN2);
	config_.lcd.rwPin = PINDEF(PORTE, PIN0);
	config_.lcd.enablePin = PINDEF(PORTB, PIN7);
	
	config_.switches.rightPin = PINDEF(PORTB, PIN5);
	config_.switches.downPin = PINDEF(PORTB, PIN0);
	config_.switches.leftPin = PINDEF(PORTB, PIN1);
	config_.switches.upPin = PINDEF(PORTB, PIN4);
	config_.switches.centerPin = PINDEF(PORTA, PIN5);
	
	Run(&config_);
}

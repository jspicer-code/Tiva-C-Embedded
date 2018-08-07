#include "RTC.h"


// This is the DS1307's slave address.
#define RTC_SLAVE_ADDRESS		0x68

static I2C_Module_t i2cModule_;

void RTC_Read(RTC_Clock_t* clock)
{
	char data[7];
	
	// Set the current address register to 0x00.  Do not issue a stop condition.
	data[0] = 0x00;
	I2C_MasterWrite(i2cModule_, RTC_SLAVE_ADDRESS, data, 1, false, false); 
	
	// Issue a repeated start, and read all date/time registers starting from address 0x00.
	int error = I2C_MasterRead(i2cModule_, RTC_SLAVE_ADDRESS, data, 7, true, true); 	
	if (!error) {
		
		clock->seconds = 10 * ((data[0] & 0x70) >> 4);
		clock->seconds += (data[0] & 0x0F);
		
		clock->minutes = 10 * ((data[1] & 0x70) >> 4);
		clock->minutes += (data[1] & 0x0F);

		clock->am_pm = (data[2] & 0x20) >> 5;
		
		clock->hours = 10 * ((data[2] & 0x10) >> 4);
		clock->hours += (data[2] & 0x0F);
		
		clock->day = data[3] & 0x07;
		
		clock->date = 10 * ((data[4] & 0x30) >> 4);
		clock->date += (data[4] & 0x0F);

		clock->month = 10 * ((data[5] & 0x10) >> 4);
		clock->month += (data[5] & 0x0F);

		clock->year = 10 * ((data[6] & 0xF0) >> 4);
		clock->year += (data[6] & 0x0F);

	}
}

void RTC_Write(const RTC_Clock_t* clock)
{
	char data[8];

	// The first by written is the starting address register, set to 0x0 here.
	// The remaining seven bytes are the register values.
	data[0] = 0x00;
	data[1] = ((clock->seconds / 10) << 4) | (clock->seconds % 10);
	data[2] = ((clock->minutes / 10) << 4) | (clock->minutes % 10);
	data[3] = 0x40 |  (clock->am_pm << 5) | ((clock->hours / 10) << 4) | clock->hours % 10;
	data[4] = clock->day;
	data[5] = ((clock->date / 10) << 4) | (clock->date % 10);
	data[6] = ((clock->month / 10) << 4) | (clock->month % 10);
	data[7] = ((clock->year / 10) << 4) | (clock->year % 10);
	
	I2C_MasterWrite(i2cModule_, RTC_SLAVE_ADDRESS, data, 8, false, true); 
	
}


void RTC_Init(I2C_Module_t i2cModule)
{
	
	i2cModule_ = i2cModule;
	
	// TODO: read the seconds register and if the CH bit is 1, then clear it to start the oscillator.

	// Seems like this only needs to be done once if there is a battery.
//	char data[2];
// Note that with the battery this will clear the seconds register which is not good.
//	data[0] = 0x00; // Seconds Register
//	data[1] = 0x00; // Clear the Clock Halt (CH) bit to enable the oscillator
//	I2C_MasterWrite(pConfig->i2cModule, RTC_SLAVE_ADDRESS, data, 2, false, true); 
	
//	data[0] = 0x07; // Control Register
//	data[1] = 0x10; // Set the SQWE bit to enable the oscillator output for 1Hz
//	I2C_MasterWrite(pConfig->i2cModule, RTC_SLAVE_ADDRESS, data, 2, false, true); 
}



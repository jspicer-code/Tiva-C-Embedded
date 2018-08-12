#include "RTC.h"


// This is the DS1307's slave address.
#define RTC_SLAVE_ADDRESS		0x68

static I2C_Module_t i2cModule_;

bool RTC_Read(RTC_Clock_t* clock)
{
	char data[7];
	bool success = false;
	
	// Set the current address register to 0x00.  Do not issue a stop condition.
	data[0] = 0x00;
	I2C_MasterWrite(i2cModule_, RTC_SLAVE_ADDRESS, data, 1, false, false); 
	
	// Issue a repeated start, and read all date/time registers starting from address 0x00.
	int error = I2C_MasterRead(i2cModule_, RTC_SLAVE_ADDRESS, data, 7, true, true); 	
	if (!error) {
		
		clock->clockHalt = (data[0] & 0x80) >> 7;
		
		clock->seconds = 10 * ((data[0] & 0x70) >> 4);
		clock->seconds += (data[0] & 0x0F);
		
		clock->minutes = 10 * ((data[1] & 0x70) >> 4);
		clock->minutes += (data[1] & 0x0F);

		clock->am_pm = (data[2] & 0x20) >> 5;
		clock->hourMode = (data[2] & 0x40) >> 6;
	
		clock->hours = 10 * ((data[2] & 0x10) >> 4);
		clock->hours += (data[2] & 0x0F);
		
		clock->day = data[3] & 0x07;
		
		clock->date = 10 * ((data[4] & 0x30) >> 4);
		clock->date += (data[4] & 0x0F);

		clock->month = 10 * ((data[5] & 0x10) >> 4);
		clock->month += (data[5] & 0x0F);

		clock->year = 10 * ((data[6] & 0xF0) >> 4);
		clock->year += (data[6] & 0x0F);
		
		success = true;

	}
	
	return success;
}

bool RTC_Write(const RTC_Clock_t* clock)
{
	char data[8];

	// The first by written is the starting address register, set to 0x0 here.
	// The remaining seven bytes are the register values.
	data[0] = 0x00;
	data[1] = (clock->clockHalt << 7) | ((clock->seconds / 10) << 4) | (clock->seconds % 10);
	data[2] = ((clock->minutes / 10) << 4) | (clock->minutes % 10);
	data[3] = (clock->hourMode << 6) |  (clock->am_pm << 5) | ((clock->hours / 10) << 4) | clock->hours % 10;
	data[4] = clock->day;
	data[5] = ((clock->date / 10) << 4) | (clock->date % 10);
	data[6] = ((clock->month / 10) << 4) | (clock->month % 10);
	data[7] = ((clock->year / 10) << 4) | (clock->year % 10);
	
	int error = I2C_MasterWrite(i2cModule_, RTC_SLAVE_ADDRESS, data, 8, false, true); 
	return (error == 0);
}


bool RTC_Init(I2C_Module_t i2cModule, bool enableOutput, bool* isReset)
{
	bool success = false;
	
	i2cModule_ = i2cModule;	
	*isReset = false;
	
	// Read the seconds register and if the CH bit is 1, then clear it
	// to start the oscillator.  The clock will be halted if it has lost power.
	RTC_Clock_t clock;
	if (RTC_Read(&clock)) {

		// Is the CH (Clock Halt) bit clear?  If so, the oscillator is running
		//	and the registers don't need to be initialized.
		if (!clock.clockHalt) {
			success = true;
		}
		else {		
			
			// Clear the CH bit to start the oscillator.
			clock.clockHalt = 0;
						
			// Enable 12-hour mode.
			clock.hourMode = 1;
		
			success = RTC_Write(&clock);

			// Let the caller know that the RTC chip is coming out of a reset state.
			if (success) {
				*isReset = true;
			}
		}
		
		// Set or clear the SQWE bit to enable/disable the SQW/OUT pin oscillator output.
		// Set the OUT bit to zero so that the SQW/OUT pin will be zero when SQWE is disabled.
		// Set RS1 and RS0 to zero for 1Hz.
		if (success) {
			char data[2];	
			data[0] = 0x07; // Control Register address.
			data[1] = enableOutput ? 0x10 : 0x00;
			success = (I2C_MasterWrite(i2cModule, RTC_SLAVE_ADDRESS, data, 2, false, true) == 0);
		}
		
	}
	
	return success;

}



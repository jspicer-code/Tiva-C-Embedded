// File:  Display.c
// Author: JSpicer
// Date:  11/18/17
// Purpose: Multiplexed display service module.
// Hardware:  TM4C123 Tiva board

#include "Display.h"
#include "HAL.h"

// Holds the SSI module used by the display
static SSIModule_t ssiModule_;

// Current display value and blank status.
static uint32_t displayValue = 0;
static uint32_t blank = 0;

// Extract the four digits of a number into an array.
void ExtractDigits(uint32_t value, uint32_t digits[4])
{
	// Break the value into individual decimal digits.
	
	digits[0] = value / 1000;
	value -= (digits[0] * 1000);
	
	// Blank leading digits that are zero.
	if (digits[0] == 0) {
		digits[0] = 0xF;
	}

	digits[1] = value / 100;
	value -= (digits[1] * 100);

	// Blank leading digits that are zero.
	if (digits[1] == 0 && digits[0] == 0xF) {
		digits[1] = 0xF;
	}
	
	digits[2] = value / 10;
	value -= (digits[2] * 10);

	// Blank leading digits that are zero.
	if (digits[2] == 0 && digits[1] == 0xF) {
		digits[2] = 0xF;
	}
	
	digits[3] = value; 

}

// This function is called back by the timer interrupt every millisecond.
//	It is responsible for the multiplexing of the display, and writes
//	(via SPI) an encoded data byte for a different digit including its 
//	BCD value and a bit that will turn on its common anode.
void TimerCallback(void)
{
	static int activeDigit = -1;
	static uint32_t digitValues[4] = { 0, 0, 0, 0 };
	
	activeDigit++;
	
	// If the last digit has been displayed, then acquire the new display value,
	//	extract the digit values, and start over at digit zero.
	if (activeDigit >= 4) {
		
		if (blank) {
			// The BCD decoder blanks the segments when the value is 0xF;
			digitValues[0] = digitValues[1] = digitValues[2] = digitValues[3] = 0xF;
		}
		else {
			ExtractDigits(displayValue, digitValues);
		}
		
		activeDigit = 0;	
	}

	// The upper nibble decides which of the four digits will be turned on
	//	and the bottom nibble contains its BCD value.
	uint8_t value = (0x1 << (activeDigit + 4)) | digitValues[activeDigit];
	
	// Serialize the digit info the external shift register.
	SPI_Write(ssiModule_, value);	

}

//----------------------- Display_Initialize --------------------------
// Initializes the display module.
// Inputs:  ssiModule - the SSI module to use in the HAL for SPI 
//            communication with the display shift register.
//          timerBlock - the HAL timer block to use for the refresh cycle.
// Outputs:  none
void Display_Initialize(SSIModule_t ssiModule, TimerBlock_t timerBlock)
{
	// The SSI module will be used by the timer callback, so save a copy.
	ssiModule_ = ssiModule;
	
	// Configure SPI.
	SPI_Enable(ssiModule);
	
	// There are 80000 system ticks in one millisecond.
	Timer_EnableTimerPeriodic(timerBlock, 80000, 0, TimerCallback);

}

//----------------------- Display_Update --------------------------
// Assigns a new value to be shown on the display.
// Inputs:  value - an integer value (0-9999) to show on the displayl.
// Outputs:  none.
void Display_Update(uint32_t value)
{
	// Store the new display value.  It will get picked up by the
	//	interrupt callback after the current value is finished
	//	being display.
	displayValue = value;
	
	// Disable blanking.
	blank = 0;
}

//----------------------- Display_Blank --------------------------
// Blanks the entire display (all digits off).
// Inputs:  none
// Outputs:  none
void Display_Blank(void)
{
	displayValue = 0;
	
	// Enable blanking.
	blank = 1;
}


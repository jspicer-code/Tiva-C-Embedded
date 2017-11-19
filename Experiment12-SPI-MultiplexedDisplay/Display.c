// File:  Display.c
// Author: JS
// Date:  11/18/17
// Purpose: Multiplexed display service module.
// Hardware:  TM4C123 Tiva board


#include "Display.h"
#include "HAL.h"

// Current display value.
static uint32_t displayValue = 0;
static uint32_t blank = 0;

void ExtractDigits(uint32_t value, uint32_t digits[4])
{
	// Break the value into individual decimal digits.
	
	digits[0] = value / 1000;
	value -= (digits[0] * 1000);
	
// Uncomment to blank leading digits that are zero.
//	if (digits[0] == 0) {
//		digits[0] = 0xF;
//	}

	digits[1] = value / 100;
	value -= (digits[1] * 100);

// Uncomment to blank leading digits that are zero.
//	if (digits[1] == 0 && digits[0] == 0xF) {
//		digits[1] = 0xF;
//	}
	
	digits[2] = value / 10;
	value -= (digits[2] * 10);

// Uncomment to blank leading digits that are zero.
//	if (digits[2] == 0 && digits[1] == 0xF) {
//		digits[2] = 0xF;
//	}
	
	digits[3] = value; 

}

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
	SPI_Write(SSI1, value);	

}

void Display_Initialize(void)
{
	
	// Configure SPI.
	SPI_Enable(SSI1);
	
	// There are 80000 system ticks in one millisecond.
	Timer_EnableTimerPeriodic(TIMER2, 80000, TimerCallback);
}


void Display_Update(uint32_t value)
{
	// Store the new display value.  It will get picked up by the
	//	interrupt callback after the current value is finished
	//	being display.
	displayValue = value;
	
	// Disable blanking.
	blank = 0;
}


void Display_Blank(void)
{
	displayValue = 0;
	
	// Enable blanking.
	blank = 1;
}


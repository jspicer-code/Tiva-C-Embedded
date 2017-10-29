// File:  Display.c
// Author: JS
// Date:  10/28/17
// Purpose: Multiplexed display service module.
// Hardware:  TM4C123 Tiva board


#include "Display.h"
#include "HAL.h"

// Current display value.
static uint32_t displayValue = 0;

// Addresses of io ports used by the display.
static volatile Display_IOPorts_t ioPorts;

void TurnOffDisplay()
{
	*(ioPorts.digit0Enable) = 0;
	*(ioPorts.digit1Enable) = 0;
	*(ioPorts.digit2Enable) = 0;
	*(ioPorts.digit3Enable) = 0;
}

void TurnOnDigit(int digit)
{
	switch (digit) {
		case 0:
			*(ioPorts.digit0Enable) = 1;
			break;
		
		case 1:
			*(ioPorts.digit1Enable) = 1;
			break;
		
		case 2:
			*(ioPorts.digit2Enable) = 1;
			break;
		
		case 3:
			*(ioPorts.digit3Enable) = 1;
			break;
	}
	
}

void WriteBCDDigit(uint8_t bcd)
{
	*(ioPorts.bcdA) = (bcd & 0x01) >> 0;
	*(ioPorts.bcdB) = (bcd & 0x02) >> 1;
	*(ioPorts.bcdC) = (bcd & 0x04) >> 2;
	*(ioPorts.bcdD) = (bcd & 0x08) >> 3;
}

void ExtractDigits(uint32_t value, uint32_t digits[4])
{
	// Break the value into individual decimal digits.
	
	digits[0] = value / 1000;
	value -= (digits[0] * 1000);
	
	digits[1] = value / 100;
	value -= (digits[1] * 100);
	
	digits[2] = value / 10;
	value -= (digits[2] * 10);
	
	digits[3] = value; 
}

void TimerCallback(void)
{
	static int activeDigit = -1;
	static uint32_t digitValues[4] = { 0, 0, 0, 0 };
	
	activeDigit++;
	
	// If the last digit has been display, then acquire the new display value,
	//	extract the digit values, and start over at digit zero.
	if (activeDigit >= 4) {
		ExtractDigits(displayValue, digitValues);
		activeDigit = 0;	
	}
	
	TurnOffDisplay();
		
	WriteBCDDigit(digitValues[activeDigit]);
			
	TurnOnDigit(activeDigit);
	
}

void Display_Initialize(const Display_IOPorts_t* io)
{
	// Copy the io ports into a module variable.
	ioPorts = *io;
	
	// There are 80000 system ticks in one millisecond.
	Timer_EnableTimerPeriodic(TIMER2, 80000, TimerCallback);
}


void Display_Update(uint32_t value)
{
	// Store the new display value.  It will get picked up by the
	//	interrupt callback after the current value is finished
	//	being display.
	displayValue = value;
}

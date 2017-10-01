// File:  VoltMeter.c
// Author: JS
// Date:  9/30/17
// Purpose: VoltMeter service.
// Hardware:  TM4C123 Tiva board

#include "project.h"
#include <stdio.h>
#include <stdint.h>
#include "SevenSegDisplay.h"

const float resolution = (3.3f / 4096.0f);

void InitVoltMeter(void)
{
		PLL_Init80MHz();

		GPIO_InitPort(PORTA);
		GPIO_EnableDO(PORTA, PIN_2 | PIN_3 | PIN_4 | PIN_5 | PIN_6 | PIN_7, DRIVE_2MA);

		GPIO_InitPort(PORTB);
		GPIO_EnableDO(PORTB, PIN_2 | PIN_3 | PIN_4 | PIN_5 | PIN_6 | PIN_7, DRIVE_2MA);

		GPIO_InitPort(PORTE);
		GPIO_EnableDO(PORTE, PIN_0 | PIN_1 | PIN_4 | PIN_5, DRIVE_2MA);

		ADC_Enable(ADC0, AIN0);
}


void WriteOnesDigit(uint8_t pattern)
{
	PA2 = (pattern >> 7) & 0x01;
	PA3 = (pattern >> 6) & 0x01;
	PA4 = (pattern >> 5) & 0x01;
	PA5 = (pattern >> 4) & 0x01;
	PA6 = (pattern >> 3) & 0x01;
	PA7 = (pattern >> 2) & 0x01;
	PE0 = (pattern >> 1) & 0x01;
	PE1 = (pattern >> 0) & 0x01;
}

void WriteTenthsDigit(uint8_t pattern)
{
	PE4 = (pattern >> 7) & 0x01;
	PE5 = (pattern >> 6) & 0x01;
	PB2 = (pattern >> 5) & 0x01;
	PB3 = (pattern >> 4) & 0x01;
	PB4 = (pattern >> 3) & 0x01;
	PB5 = (pattern >> 2) & 0x01;
	PB6 = (pattern >> 1) & 0x01;
	PB7 = (pattern >> 0) & 0x01;
}

void RunVoltMeter(void)
{
	uint32_t adcSample;
	float voltage;
	uint8_t onesPlace;
	uint8_t tenthsPlace;
	
	adcSample = ADC_Sample(ADC0);
	voltage = (float)adcSample * resolution;
	onesPlace = (uint8_t)voltage;
	tenthsPlace = (voltage - (float)onesPlace + 0.05f) * 10;

	// Always enable the decimal point for the one's place digit.
	WriteOnesDigit(DIGITS[onesPlace]| SEG_DP);
	
	WriteTenthsDigit(DIGITS[tenthsPlace]);
	
}


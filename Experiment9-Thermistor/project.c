// File:  project.c
// Author: JS
// Date:  10/4/17
// Purpose: PWM Experiment
// Hardware:  TM4C123 Tiva board

#include "HAL.h"
#include "Utilities.h"
#include "Thermistor.h"


void InitHardware()
{

	PLL_Init80MHz();
	
	// PE3=ADC
	ADC_Enable(ADC0, AIN0);
	
	// PE4=Rx, PE5=Tx
	UART_Init(UART5, 9600);
	
	SysTick_Init();
	
}

void DisplayTemps(int fahrenheit, int celsius)
{

	char strTemp[4];
	
	parseInt(fahrenheit, strTemp);
	UART_WriteString(UART5, "\rF: ");	
	UART_WriteString(UART5, strTemp);

	parseInt(celsius, strTemp);
	UART_WriteString(UART5, "  C: ");	
	UART_WriteString(UART5, strTemp);

	// Add some space to clear out the previous message.
	UART_WriteString(UART5, "     ");	
				
}


int main()
{
	uint32_t adcSample;
	int temperatureF = 0;
	int temperatureC = 0;

	InitHardware();

	while (1) 
	{			
		// Sample the ADC.
		adcSample = ADC_Sample(ADC0);	
		
		// Convert sample to temperatures.
		temperatureF = Therm_GetTemperature(adcSample, THERM_FAHRENHEIT);	
		temperatureC = Therm_GetTemperature(adcSample, THERM_CELSIUS);
		
		// Write to serial port.
		DisplayTemps(temperatureF, temperatureC);
		
		// Wait 500 ms.
		SysTick_Wait10ms(50);
		
	}
	
}

	

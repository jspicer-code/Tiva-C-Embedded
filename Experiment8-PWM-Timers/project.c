// File:  project.c
// Author: JS
// Date:  10/4/17
// Purpose: PWM Experiment
// Hardware:  TM4C123 Tiva board

#include "HAL.h"
#include "Utilities.h"
#include <string.h>

// At 80MHz, this is the number of bus cycles for a 25kHz PWM frequency.
#define PWM_PERIOD		3200

// The timer ISR stores the tach pulse count in this variable.
static uint32_t tachPulseCount;

void DisplayRPM()
{
	static uint32_t prevTach = 0;
	
	uint32_t newTach = tachPulseCount;
	
	// Compare the newTach value from the ISR to the previous value
	//	displayed.  Display again only if there is a change.
	if (newTach != prevTach) {
	
		// The Tachometer outputs 2 pulses per revolution.  So, divide by 2 to get
		//	revolutions per seconds, then multiply by 60 secs. to approximate RPM.
		uint32_t rpm = (newTach / 2) * 60;

		char message[32] = "\rRPM: ";
	
		char strCount[16];
		itoa(rpm, strCount);
		strcat(message, strCount);
		
		// Add some space to clear out the previous value.
		strcat(message, "     ");
		
		UART_WriteString(UART5, message);	
		
		prevTach = newTach;
		
	}	
		
}


void TimerCallback(void)
{

	// Read the accumulated pulse count.
	tachPulseCount = Timer_ReadCounterValue(TIMER1);
	
	// Toggle the red onboard LED.
	PF1 = !PF1;
	
	// Reset the timer to zero and start over.
	Timer_ResetInputCounter(TIMER1);

}	
	

void InitHardware()
{
	__disable_irq();
	
	PLL_Init80MHz();
	
	// Enable the the onboard LED.
	GPIO_InitPort(PORTF);
	GPIO_EnableDO(PORTF, PIN_1, DRIVE_2MA);
	
	ADC_Enable(ADC0, AIN0);
	
	// The PWM period is calculated to be the number of ticks to achieve
	//	a frequency of 25kHz.
	PWM_Enable(PWMModule0, PWM3, PWM_PERIOD, PWM_PERIOD / 2);
	
	// There are 80 million system ticks in one second.
	Timer_EnableTimerPeriodic(TIMER2, 80000000, TimerCallback);
	Timer_InitInputCounter(TIMER1);
	Timer_ResetInputCounter(TIMER1);
	
	UART_Init(UART5, 9600);

	__enable_irq();
	
}


void RunFan(void)
{
	
	volatile uint32_t adcSample;
	volatile float percentage;
	volatile uint32_t duty;
	
	// Sample the ADC.
	adcSample = ADC_Sample(ADC0);	
	
	// Convert it to a percentage of its maximum range.
	//	Don't allow it to go to 100% or the fan speed drops, 
	//	apparently unable to find an input pulse.
	percentage = ((float)adcSample / 4095.0f);
	if (percentage > 0.999f) {
		percentage = 0.999f;
	};
	
	// Apply the percentage to the PWM period to obtain the duty cycle.
	duty = (uint32_t)(percentage * (float)PWM_PERIOD);

	// Set the duty cycle
	PWM_SetDuty(PWMModule0, PWM3, duty);
		
	// Read the later tach count and display it.
	DisplayRPM();
	
}


int main()
{
	
	InitHardware();
	
	while (1) 
	{		

		RunFan();
		
	}
	
}

	

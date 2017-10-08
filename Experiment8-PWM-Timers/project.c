// File:  project.c
// Author: JS
// Date:  10/4/17
// Purpose: PWM Experiment
// Hardware:  TM4C123 Tiva board

#include "HAL.h"
#include "Utilities.h"
#include <string.h>

#define PWM_PERIOD		3200

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
	
	GPIO_InitPort(PORTF);
	GPIO_EnableDO(PORTF, PIN_1, DRIVE_2MA);
	
	ADC_Enable(ADC0, AIN0);
		
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
	
	// Convert it to a percentage of it maximum range.
	percentage = ((float)adcSample / 4095.0f);
	
	// Apply the percentage to the PWM period to obtain the duty cycle.
	duty = (uint32_t)(percentage * (float)PWM_PERIOD);

	PWM_SetDuty(PWMModule0, PWM3, duty);
		
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

	

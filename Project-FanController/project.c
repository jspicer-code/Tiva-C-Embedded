// File:  project.c
// Author: JS
// Date:  10/4/17
// Purpose: Multiplexed Display
// Hardware:  TM4C123 Tiva board

#include "HAL.h"
#include "Utilities.h"
#include "Display.h"
#include "Thermistor.h"

// At 80MHz, this is the number of bus cycles for a 25kHz PWM frequency.
#define PWM_PERIOD		3200


static volatile uint32_t* displayMode;
static volatile uint32_t* autoMode;
static int temperature = 0;
	

static void TimerCallback(void)
{
	uint32_t displayValue;
	uint32_t tachPulseCount;
	
	// If the counter mode button is pressed then display the tach count.
	//	Otherwise, display the temperature.
	if (*displayMode) {
			
			// Read the accumulated pulse count.
			tachPulseCount = Timer_ReadCounterValue(TIMER1);
		
			// The Tachometer outputs 2 pulses per revolution.  So, divide by 2 to get
			//	revolutions per seconds, then multiply by 60 secs. to approximate RPM.
			displayValue = (tachPulseCount / 2) * 60;
	}
	else {
			displayValue = temperature;
	}			
	
	// Reset the timer to zero and start over.
	Timer_ResetInputCounter(TIMER1);
	
	// Toggle the red onboard LED.
	PF1 = !PF1;
	
	// Update the display.
	Display_Update(displayValue);
	
	UART_WriteString(UART5, "Hello!\n\r");	
}	


void InitHardware()
{
	__disable_irq();
	
	PLL_Init80MHz();
	
	// Enable the the onboard LED.
	GPIO_InitPort(PORTF);
	GPIO_EnableDO(PORTF, PIN_1, DRIVE_2MA, PULL_DOWN);

	GPIO_InitPort(PORTA);
	GPIO_EnableDI(PORTA, PIN_2 | PIN_4, PULL_DOWN);
	GPIO_EnableDO(PORTA, PIN_3, DRIVE_2MA, PULL_DOWN);
	
	GPIO_InitPort(PORTB);
	GPIO_EnableDO(PORTB, PIN_2 | PIN_3 | PIN_6 | PIN_7, DRIVE_2MA, PULL_DOWN);
	
	GPIO_InitPort(PORTC);
	GPIO_EnableDO(PORTC, PIN_4 | PIN_5 | PIN_6 | PIN_7, DRIVE_2MA, PULL_UP);
		
	// These GPIO ports will be used for the display...
	Display_IOPorts_t displayPorts;
	displayPorts.bcdA = &PC4;
	displayPorts.bcdB = &PC5;
	displayPorts.bcdC = &PC6;
	displayPorts.bcdD = &PC7;
	displayPorts.digit0Enable = &PB2;
	displayPorts.digit1Enable = &PB3;
	displayPorts.digit2Enable = &PB6;
	displayPorts.digit3Enable = &PB7;
	Display_Initialize(&displayPorts);
	
	// PE3=ADC Pot
	ADC_Enable(ADC0, AIN3);
	
	// PE2=ADC Thermistor
	ADC_Enable(ADC1, AIN1);
	
	// The PWM period is calculated to be the number of ticks to achieve
	//	a frequency of 25kHz.
	PWM_Enable(PWMModule0, PWM3, PWM_PERIOD, PWM_PERIOD / 2);
	
	// There are 80 million system ticks in one second.
	Timer_EnableTimerPeriodic(TIMER0, 80000000, TimerCallback);
		
	// Initialize the tach pulse input counter.
	Timer_InitInputCounter(TIMER1);
	Timer_ResetInputCounter(TIMER1);
	
	UART_Init(UART5, 9600);
	
	// This will be the display mode (temp or RPM) switch port.
	displayMode = &PA2;
	
	// This will be the fan mode (auto or manual) switch port.
	autoMode = &PA4;
	
	__enable_irq();
	
}


int main()
{
	//uint32_t counter = 0;
	volatile uint32_t adcTherm = 0;
	volatile uint32_t adcSpeed = 0;
	volatile float percentage;
	volatile uint32_t duty;
	
	InitHardware();

	while (1) 
	{			

		adcSpeed = ADC_Sample(ADC0);  // Pot
		adcTherm = ADC_Sample(ADC1);	// Therm
	
		// Convert sample to temperature.
		temperature = Therm_GetTemperature(adcTherm, THERM_FAHRENHEIT);	
			
		if (*autoMode) {
			
			PA3 = (temperature >= 50) ? 1 : 0;
			percentage = (float)(temperature - 50) / (float)(100 - 50);
		
		}
		else {
			
			// Turn on the fan relay.
			PA3 = (adcSpeed > 200) ? 1 : 0;
			
			// Convert it to a percentage of its maximum range.
			//	Don't allow it to go to 100% or the fan speed drops, 
			//	apparently unable to find an input pulse.
			percentage = ((float)(adcSpeed - 200) / 3895.0f);
					
		}
		
		if (percentage > 0.999f) {
			percentage = 0.999f;
		}
		else if (percentage < 0.0f) {
			percentage = 0.0f;
		}
		
		// Apply the percentage to the PWM period to obtain the duty cycle.
		duty = (uint32_t)(percentage * (float)PWM_PERIOD);

			// Set the duty cycle
		PWM_SetDuty(PWMModule0, PWM3, duty);
			
		
	}
	
}

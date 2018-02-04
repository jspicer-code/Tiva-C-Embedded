// File:  project.c
// Author: JSpicer
// Date:  2/4/18
// Purpose: Main project file.  This program experiments with FreeRTOS and 
//	create periodic tasks to toggle LEDs and an aperiodic task to play
//	a musical note sequence using PWM.
// Hardware:  TM4C123 Tiva board

#include <HAL.h>
#include "LCD.h"
#include "Strings.h"
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>

// These are musical notes converted to PWM periods at the chosen bus frequency of 80MHz.
// The calculation is:  (bus freq. / pwm clock divisor) / (note frequency)
// Example:  Middle-C (C4)
//    
//		(80000000Hz / 64) / 261.63Hz = 4778
//   
// See https://pages.mtu.edu/~suits/notefreqs.html for note frequencies.
//
#define A3	5682
#define C4	4778
#define E4	3792
#define G4	3189
#define A4	2841
#define C5	2389

typedef struct {
	
	// Task period in milliseconds
	int period;
	
	// LED bit-band addresses
	volatile uint32_t* led;
	
} PeriodicParams_t;


// SystemCoreClock is referenced in FreeRTOSConfig.h.  It will be defined here and assigned
//	the bus clock frequency when the hardware is initialized.
uint32_t SystemCoreClock;

static SemaphoreHandle_t switchSemaphore_;
static uint8_t switchPressed_;

// These are the period task creation parameters.
static PeriodicParams_t redParams_ = { 1000, NULL };
static PeriodicParams_t yellowParams_ = { 500, NULL };
static PeriodicParams_t greenParams_ = { 250, NULL };

// The arpeggio note sequences.  -1 marks the end of the sequence.
static int16_t majorNotes_[] = { C4, E4, G4, C5, G4, E4, C4, -1};
static int16_t minorNotes_[] = { A3, C4, E4, A4, E4, C4, A3, -1};

// Call when there is a catastophic problem.
static void ErrHandler(void)
{
	while (1) {}
}


static void PlayNote(int16_t note)
{
	// A zero note means silence.
	if (note == 0) {
		PWM_Disable(PWMModule0, PWM4);
	}
	else {
		// The note values are PWM periods.  For each we create 50% duty cycle.
		PWM_Configure(PWMModule0, PWM4, note, note / 2);
		PWM_Enable(PWMModule0, PWM4);
	}
}

void SwitchHandler(uint32_t pinMap)
{
	
	// Disable interrupts for both switches.
	GPIO_DisarmInterrupt(&PINDEF(PORTF, (PinName_t)(PIN0 | PIN4)));
	
	// Record the switch state for the switch task.
	switchPressed_ = (uint8_t)pinMap;
	
	// This will attempt a wake the higher priority SwitchTask and continue
	//	execution there.
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	
	// Give the semaphore and unblock the SwitchTask.
	xSemaphoreGiveFromISR(switchSemaphore_, &xHigherPriorityTaskWoken);

	// If the SwitchTask was successfully woken, then yield execution to it
	//	and go there now (instead of changing context to another task).
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}


void vSwitchTask(void *pvParameters)
{

	for (;;) {
		
		// Block until the switch ISR has signaled a switch press event...
		BaseType_t taken = xSemaphoreTake(switchSemaphore_, portMAX_DELAY);
		if (taken == pdPASS) {
		
			// Establish the delay between notes...
			const TickType_t xDelay = pdMS_TO_TICKS(200);
			TickType_t xLastWakeTime = xTaskGetTickCount();
			
			// Play the minor arpeggio for SW1 and major arpeggio for SW2.
			int16_t* notes = majorNotes_;
			if (switchPressed_ & PIN4) {
				notes = minorNotes_;
			}
	
			// Play each note.  The notes sequence will end with -1.
			for (int noteIndex = 0; notes[noteIndex] != -1; noteIndex++) {

				// Play a note, then sleep until its time to play the next note.
				PlayNote(notes[noteIndex]);
				vTaskDelayUntil(&xLastWakeTime, xDelay);
			}
			
			// Turn off the sound.
			PWM_Disable(PWMModule0, PWM4);
			
			// Rearm interrupts for both switches.
			GPIO_RearmInterrupt(&PINDEF(PORTF, (PinName_t)(PIN0 | PIN4)));
		
		}
	
	}
	
}


void vPeriodicTask(void *pvParameters)
{
	
	PeriodicParams_t* params = (PeriodicParams_t*)pvParameters;

	// Establish the task's period.
	const TickType_t xDelay = pdMS_TO_TICKS(params->period);
	TickType_t xLastWakeTime = xTaskGetTickCount();
	
	for (;;) {

		// Toggle the task's LED...
		*params->led = !*params->led;
		
		// Block until the next release time.
		vTaskDelayUntil(&xLastWakeTime, xDelay);
	}

}	


// Initialize the hardware and peripherals...
static int InitHardware(void)
{

	__disable_irq();
	
	PLL_Init80MHz();

	// Must store the frequency in SystemCoreClock for FreeRTOS to use.
	SystemCoreClock = PLL_GetBusClockFreq();

	// These are the digital outputs for the LEDs.
	GPIO_EnableDO(PORTC, PIN5 | PIN6 | PIN7, DRIVE_2MA, PULL_DOWN);
	
	// Get the LED bit band addresses.
	redParams_.led = GPIO_GetBitBandIOAddress(&PINDEF(PORTC, PIN5));
	yellowParams_.led = GPIO_GetBitBandIOAddress(&PINDEF(PORTC, PIN6));
	greenParams_.led = GPIO_GetBitBandIOAddress(&PINDEF(PORTC, PIN7));
	
	// These are the digital intputs for the onboard buttons.
	GPIO_EnableDI(PORTF, PIN0 | PIN4, PULL_UP);
	
	// Enable interrupts for SW1 & SW2.
	GPIO_EnableInterrupt(&PINDEF(PORTF, PIN0), 7, INT_TRIGGER_FALLING_EDGE, SwitchHandler);
	GPIO_EnableInterrupt(&PINDEF(PORTF, PIN4), 7, INT_TRIGGER_FALLING_EDGE, SwitchHandler);
	
	// We are going to slow down the PWM clock frequency in order to play notes
	//	in the neighborhood of middle-C.
	PWM_SetClockDivisor(64);
	
	__enable_irq();
	
	return 0;
	
}


int main()
{

	if (InitHardware() < 0) {
		ErrHandler();
	}
	
	// Create the semaphore that the switch ISR and task will synchronize on.
	switchSemaphore_ = xSemaphoreCreateBinary();
	
	if (switchSemaphore_ != NULL) {
		
		// The switch task has a higher priority of 2, whereas the periodic tasks are priority 1.
		xTaskCreate(vSwitchTask, "Switch Task", 256, NULL, 2, NULL);
		
		xTaskCreate(vPeriodicTask, "Red Task", 256, &redParams_, 1, NULL);

		xTaskCreate(vPeriodicTask, "Yellow Task", 256, &yellowParams_, 1, NULL);

		xTaskCreate(vPeriodicTask, "Green Task", 256, &greenParams_, 1, NULL);
	
		// Startup of the FreeRTOS scheduler.  The program should block here.  
		vTaskStartScheduler();
	
	}	
		
	// The following line should never be reached.  Failure to allocate enough
	//	memory from the heap would be one reason.
	for (;;);

}

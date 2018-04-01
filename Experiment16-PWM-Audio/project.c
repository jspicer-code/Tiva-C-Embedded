// File:  project.c
// Author: jspicer-ltu
// Date:  3/31/18
// Purpose: Main project file.  This experiment produces audio output
//	using up to four channels of PWM. It reads MIDI data from midi-tune.c
//	which has been exported from the exportmidi program.  It creates a
//	FreeRTOS task for each MIDI track and plays them in parallel.
// Hardware:  TM4C123 Tiva board

#include <HAL.h>
#include "LCD.h"
#include "Strings.h"
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>
#include "midi.h"


// SystemCoreClock is referenced in FreeRTOSConfig.h.  It will be defined here and assigned
//	the bus clock frequency when the hardware is initialized.
uint32_t SystemCoreClock;

static SemaphoreHandle_t playSemaphore_;
static SemaphoreHandle_t trackSemaphore_;
static uint8_t switchPressed_;

extern MidiFile_t midi_tune;

typedef struct {
	TaskHandle_t taskHandle;
	PWMModule pwmModule; 
	PWMChannel pwmChannel;
	const MidiTrack_t* track;
	uint8_t isPlaying;
} TrackParams_t;



// These are the track task creation parameters.
static TrackParams_t trackParams_[4] = { 
	{ NULL, PWMModule0, PWM0, NULL, 0 },
	{ NULL, PWMModule0, PWM3, NULL, 0 },
	{ NULL, PWMModule0, PWM4, NULL, 0 },
	{ NULL, PWMModule1, PWM3, NULL, 0 }
};


// Call when there is a catastophic problem.
static void ErrHandler(void)
{
	while (1) {}
}


static void PlayNote(const MidiNoteEvent_t* note, PWMModule pwmModule, PWMChannel pwmChannel)
{
	// A zero note or velocity means silence.
	if (note->key == 0 || note->velocity == 0) {
		PWM_Disable(pwmModule, pwmChannel);
	}
	else {
	
		// For each note we create 50% duty cycle.
		uint16_t period = Midi_NotePwmPeriods[note->key];
		uint16_t duty = period / 2;
		
		PWM_Configure(pwmModule, pwmChannel, period, duty);
		PWM_Enable(pwmModule, pwmChannel);
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
	xSemaphoreGiveFromISR(playSemaphore_, &xHigherPriorityTaskWoken);
	
	// If the SwitchTask was successfully woken, then yield execution to it
	//	and go there now (instead of changing context to another task).
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}

int IsPlaying()
{
	// If anyone track is still playing, then the entire tune is playing.
	uint8_t isPlaying = 0;
	for (int i = 0; i < midi_tune.numTracks; i++ ) {		
		isPlaying |= trackParams_[i].isPlaying;
	}
	return isPlaying;
}

void vSwitchTask(void *pvParameters)
{
	TickType_t debounceDelay = pdMS_TO_TICKS(250);
	
	for (;;) {
		
		// Block until the switch ISR has signaled a switch press event...
		BaseType_t taken = xSemaphoreTake(playSemaphore_, portMAX_DELAY);
		if (taken == pdPASS) {
			
			// Has SW1 (Play/Stop) been pressed?
			if (switchPressed_ & PIN4) {
			
				// Unblock the track tasks or abort them to play/stop.
				if (!IsPlaying()) {
					for (int i = 0; i < midi_tune.numTracks; i++ ) {		
						trackParams_[i].isPlaying = 1;
						xSemaphoreGive(trackSemaphore_);
					}
				}
				else {
					for (int i = 0; i < midi_tune.numTracks; i++ ) {	
						trackParams_[i].isPlaying = 0;
						xTaskAbortDelay(trackParams_[i].taskHandle);
					}
				}			
			}
		
			// Wait a bit to debounce the switch.
			vTaskDelay(debounceDelay);
	
			// Rearm interrupts for both switches.
			GPIO_RearmInterrupt(&PINDEF(PORTF, (PinName_t)(PIN0 | PIN4)));
				
		}
		
	}
	
}

void vTrackTask(void *pvParameters)
{
	TrackParams_t* params = (TrackParams_t*)pvParameters;

	for (;;) {
		
		// Block until its time to play...
		BaseType_t taken = xSemaphoreTake(trackSemaphore_, portMAX_DELAY);
		if (taken == pdPASS) {
		
			TickType_t xLastWakeTime = xTaskGetTickCount();
			
			const MidiNoteEvent_t* notes = params->track->notes;
			
			// Play each note.  The notes sequence will end with -1 or when playing 
			//	has been stopped (task aborted).
			for (int noteIndex = 0; notes[noteIndex].deltaTime != -1; noteIndex++) {

				int deltaTime = notes[noteIndex].deltaTime;
				if (deltaTime != 0) {
					
					// Wait until its time to run the event 
					vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(deltaTime));	
					
					// Stop playing if the track has been aborted.
					if (!params->isPlaying) {
						break;
					}
				}
					
				// Mask out the channel bits (lower nibble).
				uint8_t status = 0xF0 & notes[noteIndex].status;
				switch (status) {
					
					case NOTE_ON:
						PlayNote(&notes[noteIndex], params->pwmModule, params->pwmChannel);
						break;

					case NOTE_OFF:
					default:
						PWM_Disable(params->pwmModule, params->pwmChannel);
						break;
				}

			}
			
			// Turn off the sound.
			PWM_Disable(params->pwmModule, params->pwmChannel);
			
			// Indicate that the track has finished.
			// TODO:  Changing isPlaying here could cause a race condition.  Might want
			//	to synchronize access with the SwitchTask.
			params->isPlaying = 0;
			
		}
	
	}
	
}


// Initialize the hardware and peripherals...
static int InitHardware(void)
{
	__disable_irq();
	
	PLL_Init80MHz();

	// Must store the frequency in SystemCoreClock for FreeRTOS to use.
	SystemCoreClock = PLL_GetBusClockFreq();

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
	
	playSemaphore_ = xSemaphoreCreateBinary();
	trackSemaphore_ = xSemaphoreCreateCounting(4, 0);
	
	if (playSemaphore_ && trackSemaphore_) {
			
		// The switch task will have a higher priority than the track tasks.
		xTaskCreate(vSwitchTask, "Switch Task", 100, NULL, 2, NULL);
		
		for (int i = 0; i < midi_tune.numTracks && i < 4; i++ ) {		
			trackParams_[i].track = &midi_tune.tracks[i];
			xTaskCreate(vTrackTask, "Track Task", 100, &trackParams_[i], 1, &trackParams_[i].taskHandle);
		}	
		
		// Startup of the FreeRTOS scheduler.  The program should block here.  
		vTaskStartScheduler();
	
	}	
		
	// The following line should never be reached.  Failure to allocate enough
	//	memory from the heap would be one reason.
	for (;;);

}


#include "HAL.h"
#include "FrequencyTimer.h"
	
#define CYCLE_UNDETECTED 	0
#define CYCLE_STARTED			1	
#define CYCLE_DETECTED		2

static void TimerEventCallback(TimerBlock_t block, const TimerEventArgs_t* args)
{
	FrequencyTimer_t* freqTimer = (FrequencyTimer_t*)args->callbackData;
	uint32_t currentCount = (freqTimer->timeoutCounter << 24) | args->eventData.timerCount;
	uint32_t interval = freqTimer->previousCount - currentCount;
	
	if (args->eventType == TIMER_TIMEOUT_EVENT) {		
		freqTimer->timeoutCounter--;
	}
	else if (args->eventType == TIMER_EDGE_EVENT) {
		
		if (freqTimer->cycleStatus == CYCLE_UNDETECTED) {
			freqTimer->cycleStatus = CYCLE_STARTED;
		}
		else {
			freqTimer->lastInterval = interval;
			freqTimer->cycleStatus = CYCLE_DETECTED;
		}
	
		freqTimer->previousCount = currentCount;
	}
	
	// If the interval exceeds the maximum interval (minimum frequency), then an edge has not occurred
	// for a "very long" time.  It will mean there is no signal input, so stop the timer.
	if (interval > freqTimer->maxInterval || 
		(freqTimer->cycleStatus == CYCLE_DETECTED && freqTimer->lastInterval < freqTimer->minInterval)) {
		Timer_StopEdgeTimer(block);		
	}
	
}

static void StartTimer(FrequencyTimer_t* freqTimer)
{
		freqTimer->timeoutCounter = 255;
		freqTimer->previousCount = 0;
		freqTimer->lastInterval = 0;
		freqTimer->cycleStatus = CYCLE_UNDETECTED;
	
		Timer_StartEdgeTimer(freqTimer->timer);
}

double FrequencyTimer_GetFrequency(FrequencyTimer_t* freqTimer)
{
	double frequency = 0.0;
	
	// Copy the instantaneous edge interval into a local variable before calculating the frequency.
	// The disassembly shows that copying happens via an LDR instruction from the freqTimer field
	// address to a register (r0). Since the current machine instruction is finished before an
	// interrupt can occur, the LDR instruction will complete, and the frequency will be calculated
	// based on the last interval value stored in the register, even if a timer interrupt occurs before
	// the calculation completes.  In other words, a race condition should not be an issue for concern here.
	uint32_t lastInterval = freqTimer->lastInterval;
	
	// This calulates the instantaneous frequency based on the last recorded edge interval.
	// If the last interval is zero it means that two edges have not been detected.
	if (lastInterval) {
			frequency = (double)PLL_BusClockFreq / (double)lastInterval;
	}
	
	if (!Timer_IsTimerStopped(freqTimer->timer)) {
			StartTimer(freqTimer);
	}
	
	return frequency;	
}


int FrequencyTimer_Enable(TimerBlock_t timer, const PinDef_t* pin, uint32_t maxInterval, uint8_t priority, FrequencyTimer_t* freqTimer)
{
	// Initialize timer state.
	freqTimer->timer = timer;
	freqTimer->maxInterval = maxInterval;
	freqTimer->minInterval = 80000; // 1 kHz
	
	// Initialize the hardware timer for edge timer mode.
	TimerIRQConfig_t irqConfig = {priority, TimerEventCallback, (void*)freqTimer};
	if (Timer_Init(timer, TIMER_EDGE_TIMER, &irqConfig, pin)) {
		return -1;
	}
	
	// Start the hardware timer...
	StartTimer(freqTimer);
	
	return 0;
}


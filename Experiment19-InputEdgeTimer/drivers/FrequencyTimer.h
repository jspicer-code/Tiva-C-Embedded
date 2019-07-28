// File: FrequencyTimer.h
// Author: JSpicer
// Date:  07/28/19
// Purpose: Measures the frequency of a periodic pulse input.
// Hardware:  TM4C123/TM4C129 Tiva C Launchpad board

#ifndef FREQUENCY_TIMER_H
#define FREQUENCY_TIMER_H

#include "HAL.h"

// Holds the internal state of the frequency timer.
typedef struct {
	TimerBlock_t 	timer;
	uint8_t 			timeoutCounter;
	uint8_t				cycleStatus;
	uint32_t 			previousCount;
	uint32_t 			lastInterval;
	uint32_t 			maxInterval;
	uint32_t			highFreqInterval;
} FrequencyTimer_t;

// Enables the frequency timer for given time block, interrupt priority, I/O pin, and minimum frequency
int FrequencyTimer_Enable(TimerBlock_t timer, uint8_t priority, const PinDef_t* pin, float minFrequency, FrequencyTimer_t* freqTimer);

// Polls for the most recently measured pulse frequency.
double FrequencyTimer_GetFrequency(FrequencyTimer_t* freqTimer);

#endif

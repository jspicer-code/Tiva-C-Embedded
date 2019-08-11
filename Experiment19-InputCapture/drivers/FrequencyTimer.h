// File: FrequencyTimer.h
// Author: JSpicer
// Date:  07/28/19
// Purpose: Measures the frequency of a periodic pulse input.
// Hardware:  TM4C123/TM4C129 Tiva C Launchpad board

#ifndef FREQUENCY_TIMER_H
#define FREQUENCY_TIMER_H

#include "HAL.h"

typedef struct {
	TimerBlock_t 	timer;
	PinDef_t 			pin;
	uint8_t 			priority;
} FrequencyTimerConfig_t;

#define FREQTIMER_BUFFER_SIZE		100

typedef struct {
	uint32_t time;
	uint32_t status;
} Capture_t;

typedef struct {
	Capture_t captures[FREQTIMER_BUFFER_SIZE];
	uint32_t count;
} CaptureBuffer_t;

typedef struct {
	TimerBlock_t timer;
	CaptureBuffer_t	buffers[2];
	int readIndex;
	int writeIndex;
	uint8_t timerCycle;
} FrequencyTimer_t;


// Enables the frequency timer for given time block, interrupt priority, I/O pin, and minimum frequency
int FrequencyTimer_Enable(const FrequencyTimerConfig_t* config, FrequencyTimer_t* freqTimer);

// Polls for the most recently measured pulse frequency.
float FrequencyTimer_GetFrequency(FrequencyTimer_t* freqTimer);

#endif

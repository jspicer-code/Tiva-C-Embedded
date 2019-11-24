#include "HAL.h"
#include "FrequencyTimer.h"
#include <stdlib.h>
	
#if (halCONFIG_1294 == 1)
#include "tm4c1294ncpdt.h"
#else  // TM4C123
#include "tm4c123gh6pm.h"
#endif

#define FREQTIMER_BUFFER_SIZE		100

typedef struct {
	uint32_t time;
	uint32_t status;
} Capture_t;

struct FrequencyTimer {
	TimerBlock_t timer;
	Capture_t captures[FREQTIMER_BUFFER_SIZE];
	int readIndex;
	int writeIndex;
	uint8_t timerCycle;

};

void FrequencyTimer_TimeoutHandler()
{
	TimerHandlerInfo_t* isrData = (TimerHandlerInfo_t*)NVIC_GetActiveISRData();
 	volatile TimerRegs_t* regs = isrData->regs; 
	
	// Clear the timer timeout flag and read to force clearing of the interrupt cancel flag.
	regs->ICR = TIMER_MIS_TBTOMIS;
	
	// TODO:  Is this necessary given that the interrupt won't occur again for many cycles?
	volatile uint32_t readback = regs->ICR;
	
	FrequencyTimer_t* freqTimer = (FrequencyTimer_t*)isrData->callbackData;
	freqTimer->timerCycle--;
}

void FrequencyTimer_CaptureHandler()
{
	TimerHandlerInfo_t* isrData = (TimerHandlerInfo_t*)NVIC_GetActiveISRData();
 	volatile TimerRegs_t* regs = isrData->regs; 
	
	// Clear the capture flag and read back to force clearing of the interrupt cancel flag.
	regs->ICR = TIMER_MIS_CAEMIS;	
	volatile uint32_t readback = regs->ICR;
	
	FrequencyTimer_t* freqTimer = (FrequencyTimer_t*)isrData->callbackData;
	
	// Capture the 32-bit "time"
	// From the TM4C123 user guide:
	// 	In the 16-bit Input Edge Count, Input Edge Time, and PWM modes, bits 15:0 contain the value of
	//	the counter and bits 23:16 contain the value of the prescaler, which is the upper 8 bits of the count.
	//	Bits 31:24 always read as 0
	uint32_t time = (freqTimer->timerCycle << 24) | regs->TAR;

	int next = freqTimer->writeIndex + 1;
	if (next >= FREQTIMER_BUFFER_SIZE) {
		next = 0;
	}
	
	if (next == freqTimer->readIndex) {
		// mask the input capture interrupt.
		regs->IMR &= ~TIMER_IMR_CAEIM;
	}
	else {
		// Store the current capture time and raw interrupt status bits.
		Capture_t* capture = &freqTimer->captures[freqTimer->writeIndex];
		capture->time = time;
		capture->status = regs->RIS;
	
		// Save the new buffer count.
		freqTimer->writeIndex = next;
	}
		
}

static void Reset(FrequencyTimer_t* freqTimer)
{	
	freqTimer->writeIndex = 0;
	freqTimer->readIndex = 0;	
}

static uint32_t GetCaptureTime(Capture_t* capture)
{
	uint32_t time = capture->time;
	uint8_t cycle = (uint8_t)(time >> 24);
	uint32_t counter = 0xFFFFFF & time;
	if (capture->status & TIMER_RIS_TBTORIS && counter >= 0x800000) {
		time = (--cycle << 24) | counter;
	}
	return time;
}

float FrequencyTimer_GetFrequency(FrequencyTimer_t* freqTimer)
{
	float frequency = 0.0;
	float sum = 0.0f;
	uint32_t status = 0;
	int cycleCount = 0;
	
	Capture_t* capture1 = NULL;
	Capture_t* capture2 = NULL;
	
	// Make copies the write and read indices to prevent a race condition within the capture ISR. 
	int writeIndex = freqTimer->writeIndex;
	int readIndex = freqTimer->readIndex;
	
	// Advance the read index until it reaches the end of available data (i.e. buffer empty).
	while (readIndex != writeIndex) {
		
		if (capture1 == NULL) {
			capture1 = &freqTimer->captures[readIndex];
		}
		else {
			capture2 = &freqTimer->captures[readIndex];
			
			uint32_t time1 = GetCaptureTime(capture1);
			uint32_t time2 = GetCaptureTime(capture2);
			uint32_t interval = time1 - time2;
			sum += (float)interval;
			
			capture1 = capture2;
			cycleCount++;
		}
		
		status |= freqTimer->captures[readIndex].status;
		
		if (++readIndex >= FREQTIMER_BUFFER_SIZE) {
			readIndex = 0;
		}
		
	}

	// Was there at least two captures in the buffer?
	if (cycleCount > 0) {
	
		// Decrement the readIndex so that it points to the last capture.  This means the last
		// capture will remain in the buffer to become the start of the next cycle the next time
		// the buffer is read.
		if (--readIndex < 0) {
			readIndex = FREQTIMER_BUFFER_SIZE - 1;
		}
		
		// This assignment will be an atomic instruction (see dissassembly), avoiding a race condition.
		freqTimer->readIndex = readIndex;
			
		// If at least one capture had a pending capture interrupt then the input frequency it too high to measure.
		if (status & TIMER_RIS_CAERIS) {
			frequency = -1.0;
		}
		else {
			float average = sum / (float)cycleCount;
			frequency = (float)PLL_BusClockFreq / average;
		}
	}
	
	// Set the timer interrupt mask if it has been cleared.
	volatile TimerRegs_t* regs = Timer_GetRegisters(freqTimer->timer);
	if (!(regs->IMR & TIMER_IMR_CAEIM)) {
		Reset(freqTimer);
		regs->ICR = TIMER_MIS_CAEMIS;	
		regs->IMR |= TIMER_IMR_CAEIM;
	}

	return frequency;	
}


FrequencyTimer_t* FrequencyTimer_Enable(const FrequencyTimerConfig_t* config)
{
	// TODO:  maybe have IRQ config be separate from Timer_Init so this malloc does
	// not have to occur first.
	FrequencyTimer_t* freqTimer = (FrequencyTimer_t*)malloc(sizeof(FrequencyTimer_t));
	if (freqTimer) {

		// Initialize frequency timer state.
		freqTimer->timer = config->timer;
		freqTimer->timerCycle = 0xFF;
		Reset(freqTimer);
		
		// Initialize the hardware timer for edge timer mode.
		TimerIRQConfig_t irqConfig = {config->priority, (void*)0, (void*)freqTimer};
		if (Timer_Init(config->timer, TIMER_EDGE_TIME, &irqConfig, &config->pin)) {
			free(freqTimer);
			freqTimer = 0;
		}
		else {
			// Enable it.
			Timer_EnableEdgeTimeTimer(config->timer);
		}
	
	}
	
	return freqTimer;
}

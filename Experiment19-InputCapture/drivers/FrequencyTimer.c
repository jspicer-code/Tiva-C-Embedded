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

typedef struct {
	Capture_t captures[FREQTIMER_BUFFER_SIZE];
	uint32_t count;
} CaptureBuffer_t;


struct FrequencyTimer {
	TimerBlock_t timer;
	CaptureBuffer_t	buffers[2];
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
	// From the 123 user guide:
	// 	In the 16-bit Input Edge Count, Input Edge Time, and PWM modes, bits 15:0 contain the value of
	//	the counter and bits 23:16 contain the value of the prescaler, which is the upper 8 bits of the count.
	//	Bits 31:24 always read as 0
	uint32_t time = (freqTimer->timerCycle << 24) | regs->TAR;

	// Get the current write buffer
	CaptureBuffer_t* writeBuffer = &freqTimer->buffers[freqTimer->writeIndex];
	
	// Is this buffer being read?  If so, flip buffers.
	if (freqTimer->readIndex == freqTimer->writeIndex) {

		// Flip buffer indices and buffers.
		freqTimer->writeIndex ^= 1;
		CaptureBuffer_t* newBuffer = &freqTimer->buffers[freqTimer->writeIndex];
		
		// Copy the most recent capture from the old buffer into the new buffer.
		newBuffer->captures[0] = writeBuffer->captures[writeBuffer->count - 1];
		newBuffer->count = 1;
		
		writeBuffer = newBuffer;
	}
		
	int count = writeBuffer->count + 1;
	if (count >= FREQTIMER_BUFFER_SIZE) {
		// mask interrupt.
		regs->IMR &= ~TIMER_IMR_CAEIM;
	}	
	
	// Store the current capture time.
	Capture_t* capture = &writeBuffer->captures[count - 1];
	capture->time = time;

	// Save the raw interrupt status bits before leaving.
	capture->status = regs->RIS;
	
	// This must be atomic.  Must be the last instruction.
	writeBuffer->count = count;
		
}

static void Reset(FrequencyTimer_t* freqTimer)
{	
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < FREQTIMER_BUFFER_SIZE; j++) {
			freqTimer->buffers[i].captures[j].time = 0;
			freqTimer->buffers[i].captures[j].status = 0;
		}
		freqTimer->buffers[i].count = 0;
	}
	
	freqTimer->writeIndex = 0;
	freqTimer->readIndex = 1;	
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
	
	CaptureBuffer_t* buffer = &freqTimer->buffers[freqTimer->writeIndex];
	
	// Must be atomic.
	if (buffer->count > 1) {
		
		// This will cause the capture ISR to flip buffers, making the current
		// write buffer safe to read.  Must be atomic.
		freqTimer->readIndex = freqTimer->writeIndex;
		
		float sum = 0.0f;
		uint32_t status = buffer->captures[0].status;
		
		// Note that the buffer count is being loaded again in case it changed after checking above.
		for (int i = 1; i < buffer->count; i++) {

			Capture_t* capture1 = &buffer->captures[i-1];	
			Capture_t* capture2 = &buffer->captures[i];
						
			uint32_t time1 = GetCaptureTime(capture1);
			uint32_t time2 = GetCaptureTime(capture2);
			uint32_t interval = time1 - time2;
			sum += interval;

			status |= capture2->status;	
		}
		
		// If at least one capture had a pending capture interrupt...
		if (status & TIMER_RIS_CAERIS) {
			// Whoa there!  Frequency too high.
			frequency = -1.0;
		}
		else {
			// The number of input cycles measured is one less than the buffer count.
			float average = sum / (float)(buffer->count - 1);
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
	// TODO:  maybe have IRQ config config be separate from Timer_Init so this malloc does
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

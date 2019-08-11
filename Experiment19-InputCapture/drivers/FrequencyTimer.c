#include "HAL.h"
#include "FrequencyTimer.h"
	
#if (halCONFIG_1294 == 1)
#include "tm4c1294ncpdt.h"
#else  // TM4C123
#include "tm4c123gh6pm.h"
#endif


void FrequencyTimer_TimeoutHandler()
{
	TimerHandlerInfo_t* isrData = (TimerHandlerInfo_t*)NVIC_GetActiveISRData();
 	volatile TimerRegs_t* regs = isrData->regs; 
	
	// Clear the timer timeout flag and read to force clearing of the interrupt cancel flag.
	regs->ICR = TIMER_MIS_TBTOMIS;
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
	// From the 1294 user guide:
	// 	In capture down count modes, bits 15:0 contain the value of the counter and bits 23:16 contain
	// 	the current, free-running value of the prescaler, which is the upper 8 bits of the count.
	// 	In one-shot or periodic down count modes, the prescaler stored in 23:16 is a true prescaler, 
	// 	meaning bits 23:16 count down before decrementing the value in bits 15:0.
	uint32_t time = (freqTimer->timerCycle << 24) | (0xFFFFFF & regs->TAR);

	// Get the current write buffer
	CaptureBuffer_t* writeBuffer = &freqTimer->buffers[freqTimer->writeIndex];
	
	// Is this buffer being read?  If so, flip buffers.
	if (freqTimer->readIndex == freqTimer->writeIndex) {

		// Flip buffer indices and buffers.
		freqTimer->writeIndex ^= 1;
		CaptureBuffer_t* newBuffer = &freqTimer->buffers[freqTimer->writeIndex];
		
		// Copy the previous capture into the new buffer.  Must be capture 0.
		newBuffer->captures[0] = writeBuffer->captures[(writeBuffer->count & 1) ^ 1];
		newBuffer->count = 1;
	
		writeBuffer = newBuffer;
	}
	
	// Index for this capture.
	int captureIndex = writeBuffer->count & 1;
	
	// Store the current capture time.
	Capture_t* capture = &writeBuffer->captures[captureIndex];
	capture->time = time;
	
	int count = writeBuffer->count + 1;
	if (count > 100 /* limit */) {
		// mask interrupt.
		regs->IMR &= ~TIMER_IMR_CAEIM;
	}
	
	// Save the interrupt status bits before leaving.
	capture->status = regs->RIS;
	
	// This must be atomic.  Must be the last instruction.
	writeBuffer->count = count;
		
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


static void Reset(FrequencyTimer_t* freqTimer)
{	
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			freqTimer->buffers[i].captures[j].time = 0;
			freqTimer->buffers[i].captures[j].status = 0;
		}
		freqTimer->buffers[i].count = 0;
	}
	
	freqTimer->writeIndex = 0;
	freqTimer->readIndex = 1;	
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
		
		// Note that the buffer count is being loaded again in case it changed after checking above.
		int capture1Index = buffer->count & 1;
		Capture_t* capture1 = &buffer->captures[capture1Index];
		Capture_t* capture2 = &buffer->captures[capture1Index ^ 1];
			
		if (capture1->status & TIMER_RIS_CAERIS || capture2->status & TIMER_RIS_CAERIS) {
			// Whoa there!  Frequency too high.
			frequency = -1.0;
		}
		else {
		
			uint32_t time1 = GetCaptureTime(capture1);
			uint32_t time2 = GetCaptureTime(capture2);
			uint32_t interval = time1 - time2;
			frequency = (float)PLL_BusClockFreq / (float)interval;		
		}
	}

	// Set the timer interrupt mask if it has been cleared.
	volatile TimerRegs_t* regs = Timer_GetRegisters(freqTimer->timer);
	if (!(regs->IMR & TIMER_IMR_CAEIM)) {
		Reset(freqTimer);
		regs->IMR |= TIMER_IMR_CAEIM;
	}

	return frequency;	
}


int FrequencyTimer_Enable(const FrequencyTimerConfig_t* config, FrequencyTimer_t* freqTimer)
{
	
	// Initialize the hardware timer for edge timer mode.
	TimerIRQConfig_t irqConfig = {config->priority, (void*)0, (void*)freqTimer};
	if (Timer_Init(config->timer, TIMER_EDGE_TIME, &irqConfig, &config->pin)) {
		return -1;
	}

	// Initialize the frequency timer state.
	freqTimer->timer = config->timer;
	freqTimer->timerCycle = 0xFF;
	Reset(freqTimer);

	// Enable it.
	if (Timer_EnableEdgeTimeTimer(freqTimer->timer)) {
		return -1;
	}
		
	return 0;
}


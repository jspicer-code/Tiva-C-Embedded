#include "HAL.h"
#include "CaptureTimer.h"
#include <stdlib.h>
	
#if (halCONFIG_1294 == 1)
#include "tm4c1294ncpdt.h"
#else  // TM4C123
#include "tm4c123gh6pm.h"
#endif

typedef uint8_t		BufferIndex_t;
#define CAPTIMER_BUFFER_SIZE		(1 << (8 * sizeof(BufferIndex_t)))

typedef struct {
	uint32_t time;
	uint16_t level;
	uint16_t status;
} Capture_t;

struct CaptureTimer {
	
	// Hardware timer and timeout counter
	TimerBlock_t timer;
	uint8_t timerCycle;

	// Configuration options.
	bool measureDutyCycle;
	volatile uint32_t* levelPin;
	int maxPeriod;
	
	// Buffer polling state.
	int noSignalDuration;
	float lastFrequency;
	float lastDutyCycle;
	
	// Circular buffer.
	Capture_t captures[CAPTIMER_BUFFER_SIZE];
	BufferIndex_t readIndex;
	BufferIndex_t writeIndex;
};

static const uint32_t LEVEL_HIGH = 1;

void CaptureTimer_TimeoutHandler()
{
	TimerISRData_t* isrData = (TimerISRData_t*)NVIC_GetActiveISRData();
 	volatile TimerRegs_t* regs = isrData->timerRegs; 
	
	// Clear the timer timeout flag and read to force clearing of the interrupt cancel flag.
	regs->ICR = TIMER_MIS_TBTOMIS;
	
	// TODO:  Is this necessary given that the interrupt won't occur again for many cycles?
	volatile uint32_t readback = regs->ICR;
	
	CaptureTimer_t* capTimer = (CaptureTimer_t*)isrData->timerState;
	capTimer->timerCycle--;
}

void CaptureTimer_CaptureHandler()
{
	TimerISRData_t* isrData = (TimerISRData_t*)NVIC_GetActiveISRData();
 	volatile TimerRegs_t* regs = isrData->timerRegs; 
	
	// Clear the capture flag.  NOTE:  reading back to force clearing of the interrupt cancel flag
	// doesn't seem to be necessary here and it also increases the time spent in the ISR, so that
	// step is ommitted.
	regs->ICR = TIMER_MIS_CAEMIS;	
	
	CaptureTimer_t* capTimer = (CaptureTimer_t*)isrData->timerState;
	
	// Capture the 32-bit "time"
	// From the TM4C123 user guide:
	// 	In the 16-bit Input Edge Count, Input Edge Time, and PWM modes, bits 15:0 contain the value of
	//	the counter and bits 23:16 contain the value of the prescaler, which is the upper 8 bits of the count.
	//	Bits 31:24 always read as 0
	uint32_t time = (capTimer->timerCycle << 24) | regs->TAR;

	// As an optimzation, this performs a modulo 256 operation when BufferIndex_t is uint8_t.
	BufferIndex_t next = capTimer->writeIndex + 1;
	
	if (next == capTimer->readIndex) {
		// mask the input capture interrupt.
		regs->IMR &= ~TIMER_IMR_CAEIM;
	}
	else {
		// Store the current capture time and raw interrupt status bits.
		Capture_t* capture = &capTimer->captures[capTimer->writeIndex];
		capture->time = time;
		capture->status = regs->RIS; //
		capture->level = *capTimer->levelPin;
	
		// Save the new buffer count.
		capTimer->writeIndex = next;
	}
		
}

static void ResetBuffer(CaptureTimer_t* capTimer)
{	
	capTimer->writeIndex = 0;
	capTimer->readIndex = 0;	
}

static uint32_t GetCaptureTime(Capture_t* capture)
{
	// Separate the capture time into the cycle and 24-bit counter.
	uint32_t time = capture->time;
	uint8_t cycle = (uint8_t)(time >> 24);
	uint32_t counter = 0xFFFFFF & time;
	
	// If there was a timeout pending while the input was captured, then check whether the counter is in the
	// first (upper) half of the timer cycle and decrement the cycle accordingly.  In other words, the capture
	// occurred in the cycle subsequent to the one for which is was recorded.  This situation can (in theory)
	// occur when the timeout ISR is not called before the capture ISR due to the capture interrupt having higher
	// IRQ priority.  
	if (capture->status & TIMER_RIS_TBTORIS && counter >= 0x800000) {
		time = (--cycle << 24) | counter;
	}
	return time;
}

CaptureTimer_PulseStatus_t ReadBuffer(CaptureTimer_t* capTimer, CaptureTimer_Pulse_t* pulse)
{
	float periodSum = 0.0f;
	float dutyCycleSum = 0.0f;
	int cycleCount = 0;

	// These pointers keep track of the edges within one input cycle.
	Capture_t* risingCapture1 = NULL;
	Capture_t* risingCapture2 = NULL;
	Capture_t* fallingCapture = NULL;
	
	// Make copies the write and read indices to prevent a race condition within the capture ISR.
	// Per the disassembly, these instructions are (and must be) atomic.
	BufferIndex_t writeIndex = capTimer->writeIndex;
	BufferIndex_t readIndex = capTimer->readIndex;

	// Maintain an index that points to the last rising edge in the buffer.
	BufferIndex_t lastRisingIndex;
	
	CaptureTimer_PulseStatus_t status = CAPTIMER_PULSE_VALID;
	
	// Advance the read index until it reaches the end of available data (i.e. buffer empty)
	// or an invalid condition is detected.
	while (readIndex != writeIndex && status != CAPTIMER_PULSE_INVALID) {
				
		Capture_t* capture = &capTimer->captures[readIndex];
		
		if (capture->status & TIMER_RIS_CAERIS) {
			// A capture interrupt was pending while the ISR was executing for this capture, which likely means
			// that the frequency was too high or the pulse too narrow.  In other words, the ISR can't execute
			// fast enough to keep up with the incoming edges.
 			status = CAPTIMER_PULSE_INVALID;
		}
		else if (risingCapture1 == NULL) {
			// Is this the first rising edge in the buffer?  If so, mark it and start cycle analysis here.
			if (capture->level) {
				risingCapture1 = capture;
			}
		}
		else if (!capture->level) {
			// If a low level was detected but the timer is not configured to detect both edges then
			// something may be wrong with the signal.  Also, if two or more low levels were detected
			// in succession, then likewise the signal will be considered invalid.
			if (!capTimer->measureDutyCycle || fallingCapture != NULL) {
				status = CAPTIMER_PULSE_INVALID;
			}
			else {
				// This falling edge capture immediately follows a rising edge, so mark it.
				fallingCapture = capture;
			}
		}
		else {
			
			// Now there are two rising edges, which represents one cycle.
			risingCapture2 = &capTimer->captures[readIndex];
			
			// Get the differences in their capture times (i.e. the cycle period) and add this to the total.
			uint32_t risingTime1 = GetCaptureTime(risingCapture1);
			uint32_t risingTime2 = GetCaptureTime(risingCapture2);
			uint32_t period = risingTime1 - risingTime2;
			periodSum += (float)(period);
			
			if (capTimer->measureDutyCycle) {

				// If a falling edge is expected between the two rising edges but was not detected,
				// then the signal is invalid.
				if (fallingCapture == NULL) {
					status = CAPTIMER_PULSE_INVALID;
				}
				else {
					// Calculate the duty cycle and add to the total.
					uint32_t fallingTime = GetCaptureTime(fallingCapture);
					uint32_t pulseWidth = risingTime1 - fallingTime;
					float dutyCycle = (float)pulseWidth / (float)period;
					dutyCycleSum += dutyCycle;
				}
			}
			
			risingCapture1 = risingCapture2;
			fallingCapture = NULL;
			cycleCount++;

			// Save the index of the last rising edge.
			lastRisingIndex = readIndex;
		}
						
		// As an optimzation, this performs a modulo 256 operation when BufferIndex_t is uint8_t.
		readIndex++;		
	}

	// Were all captures valid?
	if (status == CAPTIMER_PULSE_VALID) {
		
		if (cycleCount == 0) {
			// No cycles were detected.
			status = CAPTIMER_PULSE_NOSIGNAL;
		}
		else {
			
			// Update the readIndex so that it points to the last rising edge capture.  This means the last
			// capture(s) will remain in the buffer to become the start of the next cycle when the buffer
			// is read again. NOTE:  This assignment will be an atomic instruction (see dissassembly),
			// avoiding a race condition.
			capTimer->readIndex = lastRisingIndex;
				
			// Calculate the average frequency and duty cycle.
			float periodAverage = periodSum / (float)cycleCount;
			pulse->frequency = (float)PLL_BusClockFreq / periodAverage;
			pulse->dutyCycle = dutyCycleSum / (float)cycleCount;

		}
	}
	
	// Reset the timer and interrupt mask if the buffer overflowed and/or the signal was invalid.
	volatile TimerRegs_t* regs = Timer_GetRegisters(capTimer->timer);
	if (status == CAPTIMER_PULSE_INVALID || !(regs->IMR & TIMER_IMR_CAEIM)) {
		ResetBuffer(capTimer);
		regs->ICR = TIMER_MIS_CAEMIS;	
		regs->IMR |= TIMER_IMR_CAEIM;
	}

	return status;
	
}

CaptureTimer_PulseStatus_t CaptureTimer_Poll(CaptureTimer_t* capTimer, int pollInterval, CaptureTimer_Pulse_t* pulse)
{
	
	const float minFrequency = 1000.0f / (float)capTimer->maxPeriod;
			
	CaptureTimer_PulseStatus_t status = ReadBuffer(capTimer, pulse);
	
	switch (status) {
	
		case CAPTIMER_PULSE_VALID:

			if (pulse->frequency >= minFrequency) {
				capTimer->lastFrequency = pulse->frequency;
				capTimer->lastDutyCycle = pulse->dutyCycle;
			}
			else {
				// A frequency was detected but below the minimum, so change the status to "no signal".
				status = CAPTIMER_PULSE_NOSIGNAL;
			}
			capTimer->noSignalDuration = 0;
			
			break;
			
		case CAPTIMER_PULSE_NOSIGNAL:
			
			if (capTimer->noSignalDuration < capTimer->maxPeriod) {
		
				pulse->frequency = capTimer->lastFrequency;
				pulse->dutyCycle = capTimer->lastDutyCycle;
				capTimer->noSignalDuration += pollInterval;
				
				status = CAPTIMER_PULSE_VALID;
			}
		
			break;
			
		case CAPTIMER_PULSE_INVALID:
		default:
			// At least one capture in the buffer was invalid or out of range.  Set the no signal
			// duration to the maximum to prevent returning a valid status in the case above.
			capTimer->noSignalDuration = capTimer->maxPeriod;
			break;
	}
	
	return status;
			
}


CaptureTimer_t* CaptureTimer_Init(const CaptureTimer_Config_t* config)
{
	
	CaptureTimer_t* capTimer = (CaptureTimer_t*)malloc(sizeof(CaptureTimer_t));
	if (capTimer) {

		// Initialize the hardware timer for edge time mode.
		TimerEventType_t eventType = (config->measureDutyCycle) ? TIMER_EVENT_BOTH_EDGES : TIMER_EVENT_RISING_EDGE;
		TimerIRQConfig_t irqConfig = {config->irqPriority, (void*)capTimer};
		if (Timer_InitEdgeTimeTimer(config->timer, eventType, &config->timerPin, &irqConfig)) {
			free(capTimer);
			capTimer = NULL;
		}
		else {
	
			// Initialize the capture timer's state.	
			capTimer->noSignalDuration = 0;
			capTimer->lastFrequency = 0.0f;
			capTimer->lastDutyCycle = 0.0f;
			capTimer->timer = config->timer;
			capTimer->timerCycle = 0xFF;
			capTimer->maxPeriod = config->maxPeriod;
			if (capTimer->maxPeriod <= 0) {
				capTimer->maxPeriod = 1000;
			}
			
			// If configured to measure the duty cycle then enable the corresponding GPIO pin for level checking.
			// If not, point the level pin field to a constant "HIGH" value to simplify/optimize the capture ISR.
			capTimer->measureDutyCycle = config->measureDutyCycle;			
			if (config->measureDutyCycle) {
				GPIO_EnableDI(config->levelPin.port, config->levelPin.pin, PULL_NONE);
				capTimer->levelPin = GPIO_GetBitBandIOAddress(&config->levelPin);
			}
			else {
				capTimer->levelPin = (volatile uint32_t*)&LEVEL_HIGH;
			}

			ResetBuffer(capTimer);
		
			// Enable the hardware timer.
			Timer_EnableEdgeTimeTimer(config->timer);
		}
	
	}
	
	return capTimer;
}

void CaptureTimer_Terminate(CaptureTimer_t* capTimer)
{
	if (capTimer) {
		
		Timer_DisableEdgeTimeTimer(capTimer->timer);
		
		free(capTimer);
	}
}

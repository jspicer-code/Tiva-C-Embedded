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
	
	// Hardware timer and cycle counter
	TimerBlock_t timer;
	uint8_t timerCycle;

	// Configuration options
	bool measureDutyCycle;
	volatile uint32_t* timerBit;
	
	// Circular buffer
	Capture_t buffer[CAPTIMER_BUFFER_SIZE];
	BufferIndex_t readIndex;
	BufferIndex_t writeIndex;
};

typedef struct {
	int cycleCount;
	float periodSum;
	float dutyCycleSum;
} BufferTotals_t;

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
	// step is omitted.
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
		// Mask the input capture interrupt and stop capturing when the buffer is full.
		regs->IMR &= ~TIMER_IMR_CAEIM;
	}
	else {
		// Store the current capture time and raw interrupt status bits.
		Capture_t* capture = &capTimer->buffer[capTimer->writeIndex];
		capture->time = time;
		capture->status = regs->RIS; //
		capture->level = *capTimer->timerBit;
	
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

static void AnalyzeCycle(CaptureTimer_t* capTimer, BufferIndex_t edges[3], BufferTotals_t* totals)
{
	Capture_t* risingCapture1 = &capTimer->buffer[edges[0]];
	Capture_t* risingCapture2 = &capTimer->buffer[edges[2]];
	
	// Get the differences in their capture times (i.e. the cycle period) and add this to the total.
	uint32_t risingTime1 = GetCaptureTime(risingCapture1);
	uint32_t risingTime2 = GetCaptureTime(risingCapture2);
	uint32_t period = risingTime1 - risingTime2;
	totals->periodSum += (float)(period);
	
	if (capTimer->measureDutyCycle) {

		Capture_t* fallingCapture = &capTimer->buffer[edges[1]];
		
		// Calculate the duty cycle and add to the total.
		uint32_t fallingTime = GetCaptureTime(fallingCapture);
		uint32_t pulseWidth = risingTime1 - fallingTime;
		float dutyCycle = (float)pulseWidth / (float)period;
		totals->dutyCycleSum += dutyCycle;
	}
	
	totals->cycleCount++;
}


static void ReadBuffer(CaptureTimer_t* capTimer, BufferTotals_t* totals)
{
	enum { STATE_RESET, STATE_RISING, STATE_FALLING } state = STATE_RESET;
	enum { ACTION_RESET, ACTION_RISING, ACTION_FALLING, ACTION_DETECTED } action = ACTION_RESET;
	enum InputType { INPUT_NODUTY_LOW = 0, INPUT_NODUTY_HIGH = 1, INPUT_DUTY_LOW = 2, INPUT_DUTY_HIGH = 3 } input;
	
	BufferIndex_t edges[3];

	// Make copies the write and read indices to prevent a race condition within the capture ISR.
	// Per the disassembly, these instructions are (and must be) atomic, i.e. non-interruptible.
	BufferIndex_t writeIndex = capTimer->writeIndex;
	BufferIndex_t readIndex = capTimer->readIndex;
	BufferIndex_t finalIndex = readIndex;
	
	for (;readIndex != writeIndex; readIndex++) {
		
		Capture_t* capture = &capTimer->buffer[readIndex];
		input = (enum InputType) ((capTimer->measureDutyCycle << 1) | capture->level);
		
		if (capture->status & TIMER_RIS_CAERIS) {
			// A capture interrupt was pending while the ISR was executing for this capture, which likely means
			// that the frequency was too high or the pulse too narrow.  In other words, the ISR isn't executing
			// fast enough to keep up with the incoming edges.
 			state = STATE_RESET;
			action = ACTION_RESET;
		}
		else {
			
			switch (state) {
				
				case STATE_RESET:
					
					switch (input) {
						
						case INPUT_NODUTY_HIGH:
						case INPUT_DUTY_HIGH:
							state = STATE_RISING;
							action = ACTION_RISING;
							break;
						
						default:
							action = ACTION_RESET;
							break;
					}
					
					break;

				case STATE_RISING:
					
					switch (input) {
						
						case INPUT_NODUTY_LOW:
							state = STATE_RESET;
							action = ACTION_RESET;
							break;
						
						case INPUT_NODUTY_HIGH:
							state = STATE_RISING;
							action = ACTION_DETECTED;
							break;
						
						case INPUT_DUTY_LOW:
							state = STATE_FALLING;
							action = ACTION_FALLING;
							break;
						
						case INPUT_DUTY_HIGH:
							action = ACTION_RISING;
							break;
						
					}
					
					break;
				
				case STATE_FALLING:
					
					switch (input) {
						
						case INPUT_NODUTY_LOW:
						case INPUT_DUTY_LOW:
							state = STATE_RESET;
							action = ACTION_RESET;
							break;
						
						case INPUT_NODUTY_HIGH:
						case INPUT_DUTY_HIGH:
							state = STATE_RISING;
							action = ACTION_DETECTED;
							break;
					}
			
					break;
	
			}
			
		}
		
		switch (action) {
			
			case ACTION_RESET:
				finalIndex = readIndex;
				break;
			
			case ACTION_RISING:
				edges[0] = readIndex;
				finalIndex = readIndex;
				break;
			
			case ACTION_FALLING:
				edges[1] = readIndex;
				break;
			
			case ACTION_DETECTED:
				edges[2] = readIndex;
				AnalyzeCycle(capTimer, edges, totals);
			  edges[0] = readIndex;
				finalIndex = readIndex;
				break;
			
		}

	}
	
	// Update the readIndex so that it points to the beginning of the last potential cycle.  This means the
	// final capture(s) will remain in the buffer to become the start of the next cycle when the buffer is
	// read again.  This assignment is an atomic instruction (see dissassembly), avoiding a race condition.
	capTimer->readIndex = finalIndex;

}

bool CaptureTimer_GetPulse(CaptureTimer_t* capTimer, CaptureTimer_PulseInfo_t* pulse)
{
	
	BufferTotals_t totals = { 0 };
	ReadBuffer(capTimer, &totals);
	
	// Did the buffer overflow and the ISR disable itself?
	volatile TimerRegs_t* regs = Timer_GetRegisters(capTimer->timer);
	if (!(regs->IMR & TIMER_IMR_CAEIM)) {
		// The ISR is disabled, so reset the timer and reenable the ISR.  It is necessary to reset the buffer, 
		// i.e. empty it, because the last capture(s) will be older and not in sequence with the input signal
		// once capturing resumes.
		ResetBuffer(capTimer);
		regs->ICR = TIMER_MIS_CAEMIS;	
		regs->IMR |= TIMER_IMR_CAEIM;
	}	
	
	pulse->frequency = pulse->dutyCycle = 0.0f;
	
	if (totals.cycleCount > 0) {
		// Calculate the average frequency and duty cycle.
		float periodAverage = totals.periodSum / (float)totals.cycleCount;
		pulse->frequency = (float)PLL_BusClockFreq / periodAverage;
		pulse->dutyCycle = totals.dutyCycleSum / (float)totals.cycleCount;
	}
		
	return (totals.cycleCount > 0);
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
			capTimer->timer = config->timer;
			capTimer->timerCycle = 0xFF;
			
			// If configured to measure the duty cycle then enable the corresponding GPIO pin for level checking.
			// If not, point the level pin field to a constant "HIGH" value to simplify/optimize the capture ISR.
			capTimer->measureDutyCycle = config->measureDutyCycle;
			if (config->measureDutyCycle) {
				capTimer->timerBit = GPIO_GetBitBandIOAddress(&config->timerPin);
			}
			else {
				capTimer->timerBit = (volatile uint32_t*)&LEVEL_HIGH;
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

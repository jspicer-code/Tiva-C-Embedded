// File:  HAL_Timer.c
// Author: JSpicer
// Date:  10/04/17
// Purpose: PWM utilities
// Hardware:  TM4C123 Tiva board

#include <stdint.h>
#include <assert.h>
#include "HAL.h"
#include "tm4c123gh6pm.h"

// These are the base IO addresses of the timer blocks.
#define TIMER0_REG_BASE 	((volatile uint32_t *)0x40030000)
#define TIMER1_REG_BASE		((volatile uint32_t *)0x40031000)
#define TIMER2_REG_BASE		((volatile uint32_t *)0x40032000)
#define TIMER3_REG_BASE		((volatile uint32_t *)0x40033000)
#define TIMER4_REG_BASE		((volatile uint32_t *)0x40034000)
#define TIMER5_REG_BASE		((volatile uint32_t *)0x40035000)	

// This structure represents the registers associated with a timer block.
//	It will be overlayed on top of IO memory so that the structure fields
//	map to the registers.  (See the datasheet for field/register descriptions).
typedef struct {
	uint32_t  CFG;
	uint32_t  TAMR;
	uint32_t  TBMR;
	uint32_t  CTL;
	uint32_t  SYNC;
	uint32_t  RESERVED;
	uint32_t  IMR;
	uint32_t  RIS;
	uint32_t  MIS;
	uint32_t  ICR;
	uint32_t  TAILR;
	uint32_t  TBILR;
	uint32_t  TAMATCHR;
	uint32_t  TBMATCHR;
	uint32_t  TAPR;
	uint32_t  TBPR;
	uint32_t  TAPMR;
	uint32_t  TBPMR;
	uint32_t  TAR;
	uint32_t  TBR;
	uint32_t  TAV;
	uint32_t  TBV;
	uint32_t  RTCPD;
	uint32_t  TAPS;
	uint32_t  TBPS;
	uint32_t  TAPV;
	uint32_t  TBPV;
	uint32_t  RESERVED1[981];
	uint32_t  PP;
} TimerRegs_t;

// This array is a look up table to resolve the timer block name to its base address.
const volatile uint32_t * TimerBaseAddress[] = {
	TIMER0_REG_BASE,
	TIMER1_REG_BASE,
	TIMER2_REG_BASE,
	TIMER3_REG_BASE,
	TIMER4_REG_BASE,
	TIMER5_REG_BASE,
};

// These hold the function pointers to the callback functions invoked in the interrupt handlers.
static PFN_TimerCallback Timer0_Callback;
static PFN_TimerCallback Timer1_Callback;
static PFN_TimerCallback Timer2_Callback;

//------------------ Timer_EnableTimerPeriodic --------------------------
// Initializes a timer block for periodic (count down) mode.  
//	Each time the timer expires it invokes the callback function
//	at restarts.
// Inputs:  block - the hardware time block to use.
//          interval - the number of bus cycles per countdown interval.
//          priority - interrupt priority.
//          callback - function to callback when the timer expires.
// Outputs:  none.
int Timer_EnableTimerPeriodic(TimerBlock_t block, uint32_t interval, uint8_t priority, PFN_TimerCallback callback)
{
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	// Shift left once, preserving bits 1-3 to left align for the assigment below.
	priority = (priority << 1) & 0x0E;
	
	// Enable the clock to the timer block.
	uint32_t clockBit = (uint32_t)(0x1 << (uint32_t)block);
	SYSCTL_RCGCTIMER_R |= clockBit;	
	
	// The textbooks and datasheet don't seem to mention it, but there is a peripheral ready 
	//	bit	that must be checked.  If the code doesn't wait for this set it will hang up
	//	when it attempts to set the CFG register below.
	while (!(SYSCTL_PRTIMER_R & clockBit)) {};
	
	// Disable while configuring.
	timer->CTL = 0;

	// 32-bit mode.
	timer->CFG = 0;

	// periodic down-counter.
	timer->TAMR = 0x02;

	// No scaler
	timer->TAPR = 0;

	// Load interval register
	timer->TAILR = interval;

	// Clear timeout flag
	timer->ICR = 0x1;

	// Enable timeout interrupt
	timer->IMR = 0x1;
	
	switch (block) {
		
		case TIMER0:
			//
			// This is IRQ 19.  Note that the handler function (in this file) must be assigned to the
			// Vectors table in the startup_*.S file and specified in that file with an EXTERN directive
			// (so the linker can find it).
			NVIC_EnableIRQ(INT_TIMER0A - 16, priority);
			Timer0_Callback = callback;
			
			break;
		
		case TIMER1:
			//
			// This is IRQ 21.  Note that the handler function (in this file) must be assigned to the
			// Vectors table in the startup_*.S file and specified in that file with an EXTERN directive
			// (so the linker can find it).
			NVIC_EnableIRQ(INT_TIMER1A - 16, priority);
			Timer1_Callback = callback;
		
			break;
		
		case TIMER2:
			//
			// This is IRQ 23.  Note that the handler function (in this file) must be assigned to the
			// Vectors table in the startup_*.S file and specified in that file with an EXTERN directive
			// (so the linker can find it).
			NVIC_EnableIRQ(INT_TIMER2A - 16, priority);
			Timer2_Callback = callback;
			
			break;
		
		default:
			// TODO: Implement other timers as needed.
			abort();
			return -1;
	}
	
	// Enable timer
	timer->CTL |= 0x01;
	
	return 0;
}	

//------------------ Timer_EnableInputCounter --------------------------
// Initializes a timer block for input edge counter mode.
//	The timer keeps an accumulator of the number of times
//	a rising edge pulse is detected on its associated input.  
// Inputs:  block - the hardware timer block to use.
// Outputs:  none.
int Timer_EnableInputCounter(TimerBlock_t block)
{
	switch (block) {
		
		case TIMER1:
			SYSCTL_RCGCTIMER_R |= 2;	
			GPIO_EnableAltDigital(PORTB, PIN4, 0x7);
			break;
		
		case TIMER2:
			SYSCTL_RCGCTIMER_R |= 4;	
			GPIO_EnableAltDigital(PORTB, PIN0, 0x7);
			break;
		
		default:
			// TODO: Implement other timers as needed.
			abort();
			return -1;
	}
	
	return 0;
}	

//------------------ Timer_ResetInputCounter --------------------------
// Resets the accumulator of a timer configured for input edge counter mode.
// Inputs:  block - the hardware timer block to use.
// Outputs:  none.
void Timer_ResetInputCounter(TimerBlock_t block)
{
	
	// NOTE:  Assumes that the timer has been already been initialized!
	
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	// Disable while configuring.  This also sets the TAEVENT bits to 0 for rising edge trigger.
	timer->CTL = 0;

	// 16-bit mode.
	timer->CFG = 4;

	// up-count, edge-count, capture mode
	timer->TAMR = 0x13;

	// Set the count limit in the match and pre-scale match registers for a 24-bit limit.
	timer->TAMATCHR = 0xFFFF;
	timer->TAPMR = 0xFF;
	
	// Clear the value.
	timer->TAV = 0;
	timer->TAPV = 0;
	
	// Enable timer
	timer->CTL |= 0x01;
	
}

//------------------ Timer_ReadCounterValue --------------------------
// Reads the accumulator of a timer configured for input edge counter mode.
// Inputs:  block - the hardware timer block to use.
// Outputs:  accumlator value.
uint32_t Timer_ReadCounterValue(TimerBlock_t block)
{
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[block];
	volatile uint32_t count = 0x00FFFFFF & ((timer->TAPR << 16) | timer->TAR);
	return count; 
}

// Generic handler (called by the interrupt handles) which clears the interrupt flags 
//	and invokes the user callback as needed.
static void TimerHandler(volatile TimerRegs_t* timer, PFN_TimerCallback callback)
{
	volatile int readback;

	if (timer->MIS & 0x1) {
		
		// Clear the timer timeout flag.
		timer->ICR = 0x1;
		
		// Read to force clearing of the interrupt flag.
		readback = timer->ICR;
		
		// Invoke the call back if registered.
		if (callback) {
			callback();
		}

	}
	else {
		
		// Shoudn't get here, but just in case...
		timer->ICR = timer->MIS;
		readback = timer->ICR;

	}
}

// Timer0 interrupt handler.  Must be configured in the IRQ vector table.
void Timer_Timer0Handler(void)
{
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[TIMER0];
	TimerHandler(timer, Timer0_Callback);
}

// Timer1 interrupt handler.  Must be configured in the IRQ vector table.
void Timer_Timer1Handler(void)
{
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[TIMER1];
	TimerHandler(timer, Timer1_Callback);
}

// Timer2 interrupt handler.  Must be configured in the IRQ vector table.
void Timer_Timer2Handler(void)
{
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[TIMER2];
	TimerHandler(timer, Timer2_Callback);
}

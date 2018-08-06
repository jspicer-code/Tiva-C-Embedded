// File:  HAL_Timer.c
// Author: JSpicer
// Date:  10/04/17
// Purpose: PWM utilities
// Hardware:  TM4C123 & TM4C1294 Tiva board

#include <stdint.h>
#include <assert.h>
#include "HAL.h"

#if (halCONFIG_1294 == 1)
#include "tm4c1294ncpdt.h"
#else  // TM4C123
#include "tm4c123gh6pm.h"
#endif

#if (halCONFIG_1294 == 1)
#define TIMER_MAX_BLOCKS	8
#else  // TM4C123
#define TIMER_MAX_BLOCKS	6
#endif

// These are the base IO addresses of the timer blocks.
#define TIMER0_REG_BASE 	((volatile uint32_t *)0x40030000)
#define TIMER1_REG_BASE		((volatile uint32_t *)0x40031000)
#define TIMER2_REG_BASE		((volatile uint32_t *)0x40032000)
#define TIMER3_REG_BASE		((volatile uint32_t *)0x40033000)
#define TIMER4_REG_BASE		((volatile uint32_t *)0x40034000)
#define TIMER5_REG_BASE		((volatile uint32_t *)0x40035000)	
#define TIMER6_REG_BASE		((volatile uint32_t *)0x400E0000)
#define TIMER7_REG_BASE		((volatile uint32_t *)0x400E1000)

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
	
#if (halCONFIG_1294 == 1)
	uint32_t  RESERVED1[2];
	uint32_t  DMAEV;
	uint32_t  ADCEV;
	uint32_t  RESERVED2[979];
	uint32_t  PP;
	uint32_t  RESERVED3;
	uint32_t  CC;
#else // TM4C123
	uint32_t  TAPV;
	uint32_t  TBPV;
	uint32_t  RESERVED1[981];
	uint32_t  PP;
#endif

} TimerRegs_t;

// This array is a look up table to resolve the timer block name to its base address.
const volatile uint32_t * TimerBaseAddress[TIMER_MAX_BLOCKS] = {
	TIMER0_REG_BASE,
	TIMER1_REG_BASE,
	TIMER2_REG_BASE,
	TIMER3_REG_BASE,
	TIMER4_REG_BASE,
	TIMER5_REG_BASE,
#if (halCONFIG_1294 == 1)
	TIMER6_REG_BASE,
	TIMER7_REG_BASE
#endif
};

#define DEFINE_IRQ_HANDLER(name) \
static PFN_TimerCallback name##_Callback; \
void Timer_##name##Handler(void) \
{ \
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[name]; \
	HandleInterrupt(timer, name##_Callback); \
}

#define ENABLE_IRQ_HANDLER(name, priority, callback) \
{ \
	NVIC_EnableIRQ(INT_##name##A - 16, priority); \
	name##_Callback = callback; \
}

// Forward declaration, defined at the bottom of this file.
static void HandleInterrupt(volatile TimerRegs_t* timer, PFN_TimerCallback callback);

#if (halUSE_TIMER0 == 1)
#define IRQ_HANDLER_DEFINED
// TIMER0A interrupt handler.  
// INT# 35, IRQ 19.  Same for both TM4C123 and TM4C1294.
// Must be assigned in the IRQ vector table.
DEFINE_IRQ_HANDLER(TIMER0)
#endif

#if (halUSE_TIMER1 == 1)
#define IRQ_HANDLER_DEFINED
// TIMER1A interrupt handler.
// INT# 37, IRQ 21.  Same for both TM4C123 and TM4C1294.
// Must be assigned in the IRQ vector table.
DEFINE_IRQ_HANDLER(TIMER1)
#endif

#if (halUSE_TIMER2 == 1)
#define IRQ_HANDLER_DEFINED
// I2C2 interrupt handler.  
// INT# 39, IRQ 23.  Same for both TM4C123 and TM4C1294.
// Must be assigned in the IRQ vector table.
DEFINE_IRQ_HANDLER(TIMER2)
#endif

void Timer_Wait(TimerBlock_t block, uint32_t count, uint32_t interval)
{	
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	for (uint32_t i = 0; i < count; i++) {
	
		Timer_Start(block, interval);

		// Wait for the time-out raw interrupt bit to be set.
		while ((timer->RIS & 0x1) == 0);

		// Clear the raw interrupt flag.
		timer->ICR = 0x1;
		
	}
}


int Timer_Init(TimerBlock_t block, TimerMode_t mode, uint8_t priority, PFN_TimerCallback callback)
{
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	// Enable the clock to the timer block.
	uint32_t clockBit = (uint32_t)(0x1 << (uint32_t)block);
	SYSCTL_RCGCTIMER_R |= clockBit;	
	
	// The peripheral ready bit	that must be checked.  If the code doesn't wait
	//	for this set it will hang up when it attempts to set the CFG register below.
	while (!(SYSCTL_PRTIMER_R & clockBit)) {};
	
	// Disable while configuring.
	timer->CTL = 0;

	// 32-bit mode.
	timer->CFG = 0;

	// D1:D0 bits: 0x02 periodic, 0x01 one shot.
	timer->TAMR = (mode == TIMER_PERIODIC) ? TIMER_TAMR_TAMR_PERIOD : TIMER_TAMR_TAMR_1_SHOT;

	// No scaler
	timer->TAPR = 0;

	// Clear timeout flag
	timer->ICR = TIMER_ICR_TATOCINT;
	
	if (!callback) {
		// Disable timeout interrupt
		timer->IMR = 0;
	}
	else {

		// Enable timeout interrupt
		timer->IMR = 0x1;
		
		switch (block) {
			
#if (halUSE_TIMER0 == 1)
			case TIMER0:
				ENABLE_IRQ_HANDLER(TIMER0, priority, callback);
				break;
#endif
			
#if (halUSE_TIMER1 == 1)
			case TIMER1:
				ENABLE_IRQ_HANDLER(TIMER1, priority, callback);
				break;
#endif

#if (halUSE_TIMER2 == 1)
			case TIMER2:
				ENABLE_IRQ_HANDLER(TIMER2, priority, callback);
				break;
#endif
	
			default:
				// TODO: Implement other timers as needed.
				abort();
				return -1;
		}
	
	}

	return 0;
}	


int Timer_Start(TimerBlock_t block, uint32_t interval)
{
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[block];
	
	// Disable while configuring.
	timer->CTL = 0;

	// Load interval register
	timer->TAILR = interval - 1;

	// Clear timeout flag
	timer->ICR = TIMER_ICR_TATOCINT;
	
	// Enable timer
	timer->CTL |= TIMER_CTL_TAEN;
		
	return 0;
}	


void Timer_Stop(TimerBlock_t block)
{	
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[block];	
	timer->CTL = 0;
}


int Timer_EnableInputCounter(TimerBlock_t block, PinDef_t pinDef)
{
	// TODO:  The input counter functions have not been tested since refactored for the 1294.
	
	int retVal = 0;
	
#if (halCONFIG_1294 == 1)
	uint8_t ctl = 0x3;
#else	// TM4C123
	uint8_t ctl = 0x7;
#endif
	
	switch (block) {

#if (halUSE_TIMER1 == 1)
		case TIMER1:
			SYSCTL_RCGCTIMER_R |= 2;	
			break;
#endif
	
#if (halUSE_TIMER2 == 1)		
		case TIMER2:
			SYSCTL_RCGCTIMER_R |= 4;	
			break;
#endif
		
		default:
			// TODO: Implement other timers as needed.
			abort();
			retVal = -1;
	}
	
	if (retVal == 0) {
		GPIO_EnableAltDigital(pinDef.port, pinDef.pin, ctl, false);
	}
	
	return retVal;
}	

void Timer_ResetInputCounter(TimerBlock_t block)
{
	// NOTE:  Assumes that the timer has been already been initialized!
	// TODO:  The input counter functions have not been tested since refactored for the 1294.
	
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
	
#if (halCONFIG_1294 != 1)	// TM4C123
	timer->TAPV = 0;
#endif 

	// Enable timer
	timer->CTL |= 0x01;
	
}

uint32_t Timer_ReadCounterValue(TimerBlock_t block)
{
	volatile TimerRegs_t* timer = (volatile TimerRegs_t*)TimerBaseAddress[block];
	volatile uint32_t count = 0x00FFFFFF & ((timer->TAPR << 16) | timer->TAR);
	return count; 
}

#ifdef IRQ_HANDLER_DEFINED
// Generic handler (called by the interrupt handlers) which clears the interrupt flags 
//	and invokes the user callback as needed.
static void HandleInterrupt(volatile TimerRegs_t* timer, PFN_TimerCallback callback)
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
#endif

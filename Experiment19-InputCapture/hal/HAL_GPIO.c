// File:  HAL_GPIO.c
// Author: JSpicer
// Date:  9/9/17
// Purpose: GPIO utilities
// Hardware:  TM4C123, TM4C1294 Tiva board

#include <stdint.h>
#include "HAL_Config.h"

#if (halCONFIG_1294 == 1)
#include "tm4c1294ncpdt.h"
#else // TM4C123
#include "tm4c123gh6pm.h"
#endif

#include "HAL.h"

// This structure represents the registers associated with the GPIO port.
//	It will be overlayed on top of IO memory so that the structure fields
//	map to the registers.  (See the datasheet for field/register descriptions).
typedef struct {
	uint32_t DATA_BITS[255];
	uint32_t DATA;
	uint32_t DIR;
	uint32_t IS;
	uint32_t IBE;
	uint32_t IEV;
	uint32_t IM;
	uint32_t RIS;
	uint32_t MIS;
	uint32_t ICR;
	uint32_t AFSEL;
	uint32_t RESERVED[55];
	uint32_t DR2R;
	uint32_t DR4R;
	uint32_t DR8R;
	uint32_t ODR;
	uint32_t PUR;
	uint32_t PDR;
	uint32_t SLR;
	uint32_t DEN;
	uint32_t LOCK;
	uint32_t CR;
	uint32_t AMSEL;
	uint32_t PCTL;
	uint32_t ADCCTL;
	uint32_t DMACTL;
	
#if (halCONFIG_1294 == 1)	
  uint32_t SI;
  uint32_t DR12R;
  uint32_t WAKEPEN;
  uint32_t WAKELVL;
  uint32_t WAKESTAT;
  uint32_t RESERVED2[669];
  uint32_t PP;
  uint32_t PC;
#endif	
	
} GPIORegs_t;


// A lookup up table for base addresses of the ports
//	Use a PortName_t value for an index.
#if (halCONFIG_1294 == 1)
const volatile uint32_t * GPIOBaseAddress[] = {
	GPIO_PORTA_AHB_DATA_BITS_R,
	GPIO_PORTB_AHB_DATA_BITS_R,
	GPIO_PORTC_AHB_DATA_BITS_R,
	GPIO_PORTD_AHB_DATA_BITS_R,
	GPIO_PORTE_AHB_DATA_BITS_R,
	GPIO_PORTF_AHB_DATA_BITS_R,
	GPIO_PORTG_AHB_DATA_BITS_R,
	GPIO_PORTH_AHB_DATA_BITS_R,
	GPIO_PORTJ_AHB_DATA_BITS_R,
	GPIO_PORTK_DATA_BITS_R,
	GPIO_PORTL_DATA_BITS_R,
	GPIO_PORTM_DATA_BITS_R,
	GPIO_PORTN_DATA_BITS_R,
	GPIO_PORTP_DATA_BITS_R,
	GPIO_PORTQ_DATA_BITS_R
};
#else // TM4C123
const volatile uint32_t * GPIOBaseAddress[] = {
	GPIO_PORTA_DATA_BITS_R,
	GPIO_PORTB_DATA_BITS_R,
	GPIO_PORTC_DATA_BITS_R,
	GPIO_PORTD_DATA_BITS_R,
	GPIO_PORTE_DATA_BITS_R,
	GPIO_PORTF_DATA_BITS_R
};
#endif

// These hold the function pointers to the callback functions invoked in the interrupt handlers.
static PFN_GPIOCallback PortE_Callback;
static PFN_GPIOCallback PortF_Callback;

// Gets the pin number from the PinDef_t structure (which contains pin bit flags).
static int GetPinNumber(const PinDef_t* pinDef) 
{
	for (int i = 0; i < 8; i++) {
			if ((pinDef->pin >> i) & 0x1) {
				return i;
			}			
	}
	return 0;
}

//------------------------- GPIO_GetBitBandIOAddress( --------------------
// Gets the bit-band IO address for a pin definition.
// Inputs:  pinDef - pin definition
// Outputs:  pointer with value of bit band address.
volatile uint32_t* GPIO_GetBitBandIOAddress(const PinDef_t* pinDef)
{	
	const volatile uint32_t * baseAddress = GPIOBaseAddress[pinDef->port];
	
	int pinNumber = GetPinNumber(pinDef);
	
	// Add 0x3FC (data register offset) and clear the top byte to get the data address.
	uint32_t dataAddress = 0x00FFFFFF & ((uint32_t)baseAddress + 0x3FC);
	
	return (volatile uint32_t *)(0x42000000 + (32*dataAddress) + (4*pinNumber));
}


//------------------------- GPIO_InitPort ----------------------------
// Initializes a GPIO port for IO usage.
// Inputs:  port - the name of the port to initialize.
// Outputs:  none.
void GPIO_InitPort(PortName_t port)
{
	
	uint32_t portBit = (0x1 << (uint8_t)port);
	
	if (!(SYSCTL_PRGPIO_R & portBit)) {
	
		// Clock gating control register - enables the GPIO function for the port.
		SYSCTL_RCGCGPIO_R |= portBit; 
	
		// Perpheral ready register - wait until the correponding port bit is set (enabled).
		while (!(SYSCTL_PRGPIO_R & portBit )){}
	}
}

//------------------------- GPIO_EnableDO ----------------------------
// Enables a set of pins on a GPIO port for digital output.
// Inputs:  port - the name of the port to initialize.
//	        pinMap - a map of pins to enable.
//          drive - the output drive strength to configure
//          pull - the internal pull-up resistor type to configure.
// Outputs:  none.
void GPIO_EnableDO(PortName_t port, uint8_t pinMap, GPIO_Drive_t drive, GPIO_Pull_t pull)
{
		
	GPIO_InitPort(port);
		
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[port];
	
#if (halCONFIG_1294 != 1)	 // TM4C123
	if (port == PORTF && pinMap & 0x01) {
		// PORTF PIN_0 is normally locked.  Need to write a special passcode and commit it.
		gpio->LOCK = GPIO_LOCK_KEY;
		gpio->CR = 0x01;
	}
#endif
	
	// Set pins for output
	gpio->DIR |= pinMap;
	
	// Disable alternate functions
	gpio->AFSEL &= ~pinMap;	
	
	// Set the output drive strength.
	switch (drive) {
		case DRIVE_2MA:
			gpio->DR2R |= pinMap;
			break;
		
		case DRIVE_4MA:
			gpio->DR4R |= pinMap;
			break;
		
		case DRIVE_8MA:
			gpio->DR8R |= pinMap;
			break;
	};
	
	// Set the pull up/down mode.
	switch (pull) {
		case PULL_NONE:
			gpio->PUR &= ~pinMap;	
		  gpio->PDR &= ~pinMap;	
			break;
		
		case PULL_UP:
			// Enable weak pull resistors.
			gpio->PUR |= pinMap;	
			break;
		
		case PULL_DOWN:
			// Enable weak down resistors.
			gpio->PDR |= pinMap;	
			break;
	};
	
	// Enable pins for digital I/O and clear for analog I/O.
	gpio->DEN |= pinMap;
	gpio->AMSEL &= ~pinMap;
	
}


//------------------------- GPIO_EnableDI ----------------------------
// Enables a set of pins on a GPIO port for digital input.
// Inputs:  port - the name of the port to enable.
//	        pinMap - a map of pins to enable.
//          pull - the internal pull-up resistor type to configure.
// Outputs:  none.
void GPIO_EnableDI(PortName_t port, uint8_t pinMap, GPIO_Pull_t pull)
{
	
	GPIO_InitPort(port);
	
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[port];
	
#if (halCONFIG_1294 != 1) // TM4C123
	if (port == PORTF && pinMap & 0x01)  {
		// PORTF PIN_0 is normally locked.  Need to write a special passcode and commit it.
		gpio->LOCK = GPIO_LOCK_KEY;
		gpio->CR = 0x01;
	}
#endif
	
	// Clear pins for input
	gpio->DIR &= ~pinMap;
	
	// Disable alternate functions
	gpio->AFSEL &= ~pinMap;	
	
	// Set the pull up/down mode.
	switch (pull) {
		case PULL_NONE:
			gpio->PUR &= ~pinMap;	
		  gpio->PDR &= ~pinMap;	
			break;
		
		case PULL_UP:
			// Enable weak pull resistors.
			gpio->PUR |= pinMap;	
			break;
		
		case PULL_DOWN:
			// Enable weak down resistors.
			gpio->PDR |= pinMap;	
			break;
	};
	
	// Enable pins for digital I/O and clear for analog I/O.
	gpio->DEN |= pinMap;
	gpio->AMSEL &= ~pinMap;
	
}

//------------------------- GPIO_EnableAltDigital --------------------
// Enables a set of pins on a GPIO port for an alternate digital function.
// Inputs:  port - the name of the port to enable.
//	        pinMap - a map of pins to enable.
//          ctl - the alternate digital function id (see datasheet).
//					openDrain - configure the pin for open drain.
// Outputs:  none.
void GPIO_EnableAltDigital(PortName_t port, uint8_t pinMap, uint8_t ctl, bool openDrain)
{
	GPIO_InitPort(port);
	
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[port];
	uint32_t pctlValue = 0;
	int i;
	
	// Enable pins for digital and disable for analog.
	gpio->DEN |= pinMap;
	gpio->AMSEL &= ~pinMap;
	
	// Open drain mode is used by I2CxSCL.
	if (openDrain) {
		gpio->ODR |= pinMap;
	}
	else {
		gpio->ODR &= ~pinMap;
	}
	
	// Enable pins for alternate function.
	gpio->AFSEL |= pinMap;
	
	// Shift pin control values into a single variable to assign below.
	for (i = 0; i < 8; i++) {
		if (pinMap & (0x1 << i)) {
			pctlValue |= (ctl << (4*i));
		}
	}
	
	// Set the pin control function for each pin.
	gpio->PCTL |= pctlValue;

}

//------------------------- GPIO_EnableAltAnalog --------------------
// Enables a set of pins on a GPIO port for analog usage.
// Inputs:  port - the name of the port to enable.
//	        pinMap - a map of pins to enable.
// Outputs:  none.
void GPIO_EnableAltAnalog(PortName_t port, uint8_t pinMap)
{
	GPIO_InitPort(port);
	
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[port];

	// Enable pins for analog and disable for digital.
	gpio->DEN &= ~pinMap;
	gpio->AMSEL |= pinMap;
	
	// Not seemingly necessary, but disable pull ups/downs anyway.
	gpio->PUR &= ~pinMap;	
	gpio->PDR &= ~pinMap;	
	
	// Enable pins for alternate function.
	gpio->AFSEL |= pinMap;
}


int GPIO_EnableInterrupt(const PinDef_t* pinDef, uint8_t priority, GPIO_IntTrigger_t trigger, PFN_GPIOCallback callback)
{
	
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[pinDef->port];
	
	switch (trigger) {
		
		case INT_TRIGGER_LOW_LEVEL:
			
			// IS is the Interrupt Sense register, which decides level or edge.
			// 0 = edge
			// 1 = level
			gpio->IS |= pinDef->pin;
			
			// IEV is the Interrupt Event register, which decides low-level, 
			// high-level, falling, or rising edge.
			// 0 = falling/low
			// 1 = rising/high
			gpio->IEV &= ~pinDef->pin;
			
			// IBE is the Interrupt Both Edges register, which is set when both edge
			// interrupt is desired and cleared otherwise. IBE overrites the decision in IEV.
			// 0 = disable
			// 1 = both edges
			gpio->IBE &= ~pinDef->pin;		
			
			break;
		
		case INT_TRIGGER_HIGH_LEVEL:
			gpio->IS |= pinDef->pin;
			gpio->IEV |= pinDef->pin;
			gpio->IBE &= ~pinDef->pin;		
			break;
		
		case INT_TRIGGER_FALLING_EDGE:
			gpio->IS &= ~pinDef->pin;
			gpio->IEV &= ~pinDef->pin;
			gpio->IBE &= ~pinDef->pin;		
			break;
		
		case INT_TRIGGER_RISING_EDGE:
			gpio->IS &= ~pinDef->pin;
			gpio->IEV |= pinDef->pin;
			gpio->IBE &= ~pinDef->pin;		
			break;
		
		case INT_TRIGGER_BOTH_EDGES:
			gpio->IS &= ~pinDef->pin;
			gpio->IBE |= pinDef->pin;		
			break;
		
		default:
			break;
		
	}
	
	// ICR is the Interrupt Clear Register, which must be cleared before returning
	//	from the interrupt handler (otherwise, the interrupt appears as if it is
	//	still pending and the interrupt handler will be execute again).
	// IM is the GPIO Interrupt Mask register.  Each bit enables (if set) enables
	//	interrupt capability for the corresponding GPIO pin.
	//
	// Clear any prior interrupt and unmask interrupt for the selected pin.
	//
	gpio->ICR |= pinDef->pin;
	gpio->IM |= pinDef->pin;
		
	// IRQ numbers are 16 less than the INT number.	
	switch (pinDef->port) {
		
		case PORTE:
			NVIC_EnableIRQ(INT_GPIOE - 16, priority, (void*)0);
			PortE_Callback = callback;
			break;
		
		case PORTF:
			NVIC_EnableIRQ(INT_GPIOF - 16, priority, (void*)0);
			PortF_Callback = callback;
			break;
		
		default:
			// TODO:  Implement other ports as needed.
			return -1;
		
	}
	
	return 0;
}


void GPIO_DisarmInterrupt(const PinDef_t* pinDef) 
{
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[pinDef->port];
	
	// Note that pending interrupts must be acknowledged before reenabling the interrupt
	//	mask or they will be raised immediately.  Example:  disarm, press an (edge
	//  triggered) switch, then rearm => the pending interrupt will occur unless ICR is ack'd
	//	before setting IM.
	
	// Acknowledge any pending interrupts.
	// Read to force clearing of the interrupt flag.
	gpio->ICR |= pinDef->pin;
	volatile int readback = gpio->ICR;
	
	// Clear interrupt mask bit for pin.
	gpio->IM &= ~pinDef->pin;
	
}

void GPIO_RearmInterrupt(const PinDef_t* pinDef) 
{
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[pinDef->port];
	
	// Acknowledge any pending interrupts.
	// Read to force clearing of the interrupt flag.
	gpio->ICR |= pinDef->pin;
	volatile int readback = gpio->ICR;
	
	// Set interrupt mask bit for pin.
	gpio->IM |= pinDef->pin;
	
}

// Generic handler (called by the interrupt handles) which clears the interrupt flags 
//	and invokes the user callback as needed.
static void HandleInterrupt(volatile GPIORegs_t* gpio, PortName_t port, PFN_GPIOCallback callback)
{
	volatile int readback;

	// Save a copy of the status register before clearing.
	uint32_t status = gpio->MIS;

	// Clear the interrupt flag for the pin in the MIS.
	gpio->ICR |= status;
		
	// Read to force clearing of the interrupt flag.
	readback = gpio->ICR;

	// Check if ANY of the bits in the masked interrupt status register are set.
	//	If so, then one of the pins on the GPIO port has triggered an interrupt.
	//  Invoke the call back if registered.
	if ((status & 0xFF) && callback) {
			callback(port, status);
	}
	
}

// PortE interrupt handler.  INT# 20, IRQ 4.  Must be assigned in the IRQ vector table.
void GPIO_PortEHandler(void)
{
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[PORTE];
	HandleInterrupt(gpio, PORTE, PortE_Callback);	
}

// PortF interrupt handler.  INT# 46, IRQ 30.  Must be assigned in the IRQ vector table.
void GPIO_PortFHandler(void)
{
	volatile GPIORegs_t* gpio = (volatile GPIORegs_t*)GPIOBaseAddress[PORTF];
	HandleInterrupt(gpio, PORTF, PortF_Callback);	
}


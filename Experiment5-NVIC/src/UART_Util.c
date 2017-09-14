// File:  UART_Util.c
// Author: JS
// Date:  9/9/17
// Purpose: UART configuration and read/write utilities
// Hardware:  TM4C123 Tiva board

#include <stdint.h>
#include "UART_Util.h"
#include "tm4c123gh6pm.h"

void UART_UART5_Init(void)
{
	
	// Provide clock to UART5
	SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R5;
		
	// Enable clock to PORTE
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
	
	// Disable while configuring.
	UART5_CTL_R = 0;
	
	// Baud rate will be configured for 9600.  Note: this assumes an 80MHz bus clock!
	// 	The High-Speed Enable feature will be disabled, so the system clock will be 
	//	divided by 16. Therefore:
	//
	//	Integer Part (16 bits) = Floor(80MHz/(16 * 9600)) = Floor(520.83333) = 520 = 0x208
	//  Fraction Part (6 bits) = Floor((0.83333 * 64) + 0.5) = Floor(53.83312) = 53 = 0x35
	//
	// TODO:  Make it possible to pass the baud rate in as a parameter.
	//
	UART5_IBRD_R = 0x208; 
	UART5_FBRD_R = 0x35;
	
	// Use the system clock for the clock source.
	UART5_CC_R = 0;
	
	// 8-bit, no parity, 1 stop bit, no FIFO.
	UART5_LCRH_R = UART_LCRH_WLEN_8;
	
	// Enable UART along with RX and TX.
	UART5_CTL_R = UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN;
	
	// Now we enable TX5 and RX5 on PORTE pins 4 & 5.
	// U5TX == PE5
	// U5RX == PE4

	// Enable pins 4 & 5 for digital and disable for analog.
	GPIO_PORTE_DEN_R |= 0x30;
	GPIO_PORTE_AMSEL_R &= ~0x30;
	
	// Enable pins 4 & 5 for alternate function.
	GPIO_PORTE_AFSEL_R |= 0x30;
	
	// Set the pin control function to 0x1 for each pin.
	//	PE4 bits are 19:16
	//	PE5 bits are 23:20
	GPIO_PORTE_PCTL_R |= 0x00110000;
	
}


void UART_UART5_WriteChar(char c)
{
	// Monitor the Transmit FIFO Full flag until it clears.
	while (UART5_FR_R & UART_FR_TXFF) {}
	
	// Write character to the data register.
	UART5_DR_R = c;	

}

void UART_UART5_WriteString(char* sz)
{
	// Transmit each string character until the trailing null is found.
	for (; *sz; sz++) {
		UART_UART5_WriteChar(*sz);
	}
}


char UART_UART5_ReadChar(void)
{
	// Monitor the Receive FIFO Empty flag until it clears.
	// NOTE that this is blocking.
	while (UART5_FR_R & UART_FR_RXFE) {}
		
	// Return the character in the data register.
	return UART5_DR_R;
}




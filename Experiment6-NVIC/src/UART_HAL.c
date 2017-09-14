// File:  UART_Util.c
// Author: JS
// Date:  9/9/17
// Purpose: UART configuration and read/write utilities
// Hardware:  TM4C123 Tiva board

#include <stdint.h>
#include "HAL.h"
#include "tm4c123gh6pm.h"

#define UART0_REG_BASE 	((volatile uint32_t *)0x4000C000)
#define UART1_REG_BASE	((volatile uint32_t *)0x4000D000)
#define UART2_REG_BASE	((volatile uint32_t *)0x4000E000)
#define UART3_REG_BASE	((volatile uint32_t *)0x4000F000)
#define UART4_REG_BASE	((volatile uint32_t *)0x40010000)
#define UART5_REG_BASE	((volatile uint32_t *)0x40011000)
#define UART6_REG_BASE	((volatile uint32_t *)0x40012000)
#define UART7_REG_BASE	((volatile uint32_t *)0x40013000)

typedef struct {     
	uint32_t  DR;  
	uint32_t  RSR_ECR_;
	uint32_t  RESERVED[4];
	uint32_t  FR;  
	uint32_t  RESERVED1;
	uint32_t  ILPR;
	uint32_t  IBRD;
	uint32_t  FBRD;
	uint32_t  LCRH; 
	uint32_t  CTL;
	uint32_t  IFLS;
	uint32_t  IM;
	uint32_t  RIS;
	uint32_t  MIS;
	uint32_t  ICR;
	uint32_t  DMACTL; 
	uint32_t  RESERVED2[22];
	uint32_t  _9BITADDR; 
	uint32_t  _9BITAMASK;
	uint32_t  RESERVED3[965];
	uint32_t  PP;
	uint32_t  RESERVED4;
	uint32_t  CC; 
} UARTRegs_t;


const volatile uint32_t * UARTBaseAddress[] = {
	UART0_REG_BASE,
	UART1_REG_BASE,
	UART2_REG_BASE,
	UART3_REG_BASE,
	UART4_REG_BASE,
	UART5_REG_BASE,
	UART6_REG_BASE,
	UART7_REG_BASE
};


int UART_Init(UART_ID_t uartId, uint32_t baud)
{
	uint32_t baudRateDivisor;
	
	// FAIL if bus clock not initialized.
	if (BusClockFreq == 0) {
		return -1;
	}
	
	// Provide clock to UART
	SYSCTL_RCGCUART_R |= (0x1 << uartId);
	
	// Enable clock to GPIO port(s)...
	switch (uartId) {
		
		case UART5:
			GPIO_Init_Port(PORTE);
			break;
		
		default:
			// TODO:  Implement additional UART ports as needed.
			return -1;
	}
	
	volatile UARTRegs_t* uart = (volatile UARTRegs_t*)UARTBaseAddress[uartId];

	// Disable UART while configuring.
	uart->CTL = 0;
	
	// Configure the baud rate.  The High-Speed Enable feature will be disabled, so the system
	//	clock will be  divided by 16. Example for 9600 baud and 80MHz clock:
	//
	//	Integer Part (16 bits) = Floor(80MHz/(16 * 9600)) = Floor(520.83333) = 520 = 0x208
	//  Fraction Part (6 bits) = Floor((0.83333 * 64) + 0.5) = Floor(53.83312) = 53 = 0x35
	//
	baudRateDivisor = (16*baud);
	uart->IBRD = (BusClockFreq / baudRateDivisor); // 0x208; 
	uart->FBRD = (((BusClockFreq % baudRateDivisor) * 64) / baudRateDivisor); //  0x35;
	
	
	// Use the system clock for the clock source.
	uart->CC = 0;
	
	// 8-bit, no parity, 1 stop bit, no FIFO.
	uart->LCRH = UART_LCRH_WLEN_8;
	
	// Enable UART along with RX and TX.
	uart->CTL = UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN;
	
	// Now enable the alternate pin function for TX and RX.
	switch (uartId) {
		
		case UART5:
			GPIO_EnableAltDigital(PORTE, PIN_4 | PIN_5, 0x1);
			break;
		
		default:
			// TODO:  Implement additional UART ports as needed.
			return -1;
		
	}
	
	return 0;
}

void UART_WriteChar(UART_ID_t uartId, char c)
{
	volatile UARTRegs_t* uart = (volatile UARTRegs_t*)UARTBaseAddress[uartId];
	
	// Monitor the Transmit FIFO Full flag until it clears.
	while (uart->FR & UART_FR_TXFF) {}
	
	// Write character to the data register.
	uart->DR = c;	

}

void UART_WriteString(UART_ID_t uartId, char* sz)
{
	// Transmit each string character until the trailing null is found.
	for (; *sz; sz++) {
		UART_WriteChar(uartId, *sz);
	}
}


char UART_ReadChar(UART_ID_t uartId)
{
	volatile UARTRegs_t* uart = (volatile UARTRegs_t*)UARTBaseAddress[uartId];
	
	// Monitor the Receive FIFO Empty flag until it clears.
	// NOTE that this is blocking.
	while (uart->FR & UART_FR_RXFE) {}
		
	// Return the character in the data register.
	return uart->DR;
}



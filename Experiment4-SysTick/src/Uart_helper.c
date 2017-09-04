#include "Uart_helper.h"

//Writing this code involved 3 things 
//1 - Looking at the TIVA driver library documentaion
//2 - Looking at the source code for the uart.c in the driver folder
//3 - Doing some google to debug errors
void UartSetup()
{
		//
		// Enable the UART0 module.
		//
		SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
		//
		// Wait for the UART0 module to be ready.
		//
		while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0))
		{
		}
		
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); // activate port A
		
		// enable alt funct on PA1-0 and enable digital I/O on PA1-0
		GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_1);
		
		// Initialize the UART. Set the baud rate, number of data bits, turn off
		// parity, number of stop bits, and stick mode. The UART is enabled by the
		//
		UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 38400,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));
		//
		// Check for characters. Spin here until a character is placed
}


// this function does the same thing as UartSetup(), but uses direct register access method

// This function involved 2 things
// 1 - Reading the appropriate chapters in the data sheet
// 2 - Looking at the 
void UartSetup2()
{
	SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART0; // activate UART0
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA; // activate port A
  UART0_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
  UART0_IBRD_R = 27;                    // IBRD = int(50,000,000 / (16 * 115,200)) = int(27.1267)
  UART0_FBRD_R = 8;                     // FBRD = int(0.1267 * 64 + 0.5) = 8
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART0_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART0_CTL_R |= UART_CTL_UARTEN;       // enable UART
  GPIO_PORTA_AFSEL_R |= 0x03;           // enable alt funct on PA1-0
  GPIO_PORTA_DEN_R |= 0x03;             // enable digital I/O on PA1-0
}

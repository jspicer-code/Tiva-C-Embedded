// File:  HAL_UART.h
// Author: JSpicer
// Date:  9/9/17
// Purpose: UART configuration and read/write utilities
// Hardware:  TM4C123 Tiva board

#ifndef UART_HAL_H
#define UART_HAL_H

#include <stdint.h>

// Signature of the Receive (Rx) character callback function 
typedef void (*PFN_RxCallback)(char c);
	
// These are service layer names for the ports.
typedef enum {
	UART0,
	UART1,
	UART2,
	UART3,
	UART4,
	UART5,
	UART6,
	UART7
} UART_ID_t;

int UART_Enable(UART_ID_t uartId, uint32_t baud);
void UART_WriteChar(UART_ID_t uartId, char c);
void UART_WriteString(UART_ID_t uartId, char* sz);
char UART_ReadChar(UART_ID_t uartId);
int UART_EnableRxInterrupt(UART_ID_t uartId, uint8_t priority, PFN_RxCallback callback);

#endif



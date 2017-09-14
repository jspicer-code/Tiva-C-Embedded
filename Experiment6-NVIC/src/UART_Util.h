// File:  UART_Util.h
// Author: JS
// Date:  9/9/17
// Purpose: UART configuration and read/write utilities
// Hardware:  TM4C123 Tiva board

void UART_UART5_Init(void);
void UART_UART5_WriteChar(char c);
void UART_UART5_WriteString(char* sz);
char UART_UART5_ReadChar(void);

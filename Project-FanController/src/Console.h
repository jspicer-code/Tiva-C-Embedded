// File:  Console.h
// Author: JSpicer
// Date:  11/25/17
// Purpose: Console interaction for Temperature Control Setting
// Hardware:  TM4C123 Tiva board

#include "HAL_UART.h"
#include "ControlSettings.h"

void Console_Init(UART_ID_t uart);
void Console_HandleInput(char c, ControlSettings_t* settings);

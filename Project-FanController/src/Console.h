// File:  Console.h
// Author: JSpicer
// Date:  11/25/17
// Purpose: Console interaction module, primarly for changing Temperature Settings
// Hardware:  TM4C123 Tiva board

#include "HAL_UART.h"
#include "TemperatureSettings.h"


//-------------------------- Console_Init----------------------------
// Initializes the Console module.  Must be called once during program 
//	start up.
// Inputs:  uart - The HAL UART_ID that will be used for console I/O.
// Outputs:  none
void Console_Init(UART_ID_t uart);

//----------------------- Console_HandleInput -----------------------
// Handles the input of single character typed to the console.
// Inputs:  c - the character typed.
//          settings - a pointer to the TemperatureSettings structure
//		         to display and save to Flash.
// Outputs: none
void Console_HandleInput(char c, TemperatureSettings_t* settings);

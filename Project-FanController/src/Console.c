// File:  Console.h
// Author: JSpicer
// Date:  11/25/17
// Purpose: Console interaction for Temperature Control Setting
// Hardware:  TM4C123 Tiva board

#include "HAL.h"
#include "Strings.h"
#include "ControlSettings.h"

#define MAX_BUFFER				16
#define CHAR_ESCAPE				0x1B
#define CHAR_ENTER				'\r'
#define CHAR_BACKSPACE		0x7F

enum {
	SCREEN_MAIN,
	SCREEN_SCALE,
	SCREEN_CALIBRATION,
	SCREEN_LIMIT_RANGE,
	SCREEN_LIMIT_LOW,
	SCREEN_LIMIT_HIGH
} currentScreen_;
	
	
static UART_ID_t uart_;

void Console_Init(UART_ID_t uart)
{
	uart_ = uart;
	currentScreen_ = SCREEN_MAIN;
}


// Prints the main console menu.
static void PrintMainScreen(void)
{
	UART_WriteString(uart_, "\n\r\n\r");		
	UART_WriteString(uart_, "Temperature Control Settings:\n\r");
	UART_WriteString(uart_, " 1) Scale\n\r");
	UART_WriteString(uart_, " 2) Calibration Offset\n\r");
	UART_WriteString(uart_, " 3) Automatic Mode Limits\n\r");
	UART_WriteString(uart_, ">");	
}

// Prints the temperature scale screen.
static void PrintScaleScreen(ControlSettings_t* settings)
{
	UART_WriteString(uart_, "\n\r\n\r");		
	UART_WriteString(uart_, "Select the temperature scale (ESC for previous menu):\n\r");
	UART_WriteChar(uart_, (settings->scale == THERM_FAHRENHEIT) ? '*' : ' ');
	UART_WriteString(uart_, "1) Fahrenheit\n\r");
	UART_WriteChar(uart_, (settings->scale == THERM_CELSIUS) ? '*' : ' ');
	UART_WriteString(uart_, "2) Celsius\n\r");
	UART_WriteString(uart_, ">");	
}

// Prints the calibration screen.
static void PrintCalibrationScreen(ControlSettings_t* settings)
{
	char offset[16];
	itoa(settings->calibrationOffset, offset);

	UART_WriteString(uart_, "\n\r\n\r");		
	
	UART_WriteString(uart_, "Calibration Offset: ");
	UART_WriteString(uart_, offset);
	UART_WriteString(uart_,"\n\r");
	
	UART_WriteString(uart_, "Enter an integer to offset the measured temperature (ESC for previous menu):\n\r");
	UART_WriteString(uart_, ">");	
}

// Prints the calibration screen.
static void PrintRangeScreen(ControlSettings_t* settings)
{
	char low[16];
	char high[16];
	itoa(settings->lowTemp, low);
	itoa(settings->highTemp, high);

	UART_WriteString(uart_, "\n\r\n\r");	
	
	UART_WriteString(uart_, "Automatic Mode Limits: ");
	UART_WriteString(uart_, low);
	UART_WriteString(uart_, "-");
	UART_WriteString(uart_, high);
	UART_WriteString(uart_,"\n\r");
		
	UART_WriteString(uart_, "Select the temperature limit to change (ESC for previous menu):\n\r");
	UART_WriteString(uart_, " 1) Low (Turn Off)\n\r");
	UART_WriteString(uart_, " 2) High (Maximum Speed)\n\r");
	UART_WriteString(uart_, ">");	
}

// Prints the low temperature limit.
static void PrintLowLimitScreen(ControlSettings_t* settings)
{
	char low[16];
	itoa(settings->lowTemp, low);

	UART_WriteString(uart_, "\n\r\n\r");		
	
	UART_WriteString(uart_, "Low Limit: ");
	UART_WriteString(uart_, low);
	UART_WriteString(uart_,"\n\r");
	
	UART_WriteString(uart_, "Enter an integer to set the low temperature limit (ESC for previous menu):\n\r");
	UART_WriteString(uart_, ">");	
}

// Prints the high temperature limit.
static void PrintHighLimitScreen(ControlSettings_t* settings)
{
	char high[16];
	itoa(settings->highTemp, high);

	UART_WriteString(uart_, "\n\r\n\r");		
	
	UART_WriteString(uart_, "High Limit: ");
	UART_WriteString(uart_, high);
	UART_WriteString(uart_,"\n\r");
	
	UART_WriteString(uart_, "Enter an integer to set the high temperature limit (ESC for previous menu):\n\r");
	UART_WriteString(uart_, ">");	
}

void HandleMainScreen(const char* input)
{
	
	int selection = atoi(input);
	
	switch (selection) {
		
		case 1:
			currentScreen_ = SCREEN_SCALE;
			break;
		
		case 2:
			currentScreen_ = SCREEN_CALIBRATION;
			break;
		
		case 3:
			currentScreen_ = SCREEN_LIMIT_RANGE;
			break;
		
		default:
			break;
	}

}


void HandleScaleScreen(const char* input, ControlSettings_t* settings)
{
	
	int selection = atoi(input);
	
	switch (selection) {
		
		case 1:
			settings->scale = THERM_FAHRENHEIT;
			break;
		
		case 2:
			settings->scale = THERM_CELSIUS;
			break;
		
		default:
			break;
	}

}


void HandleCalibrationScreen(const char* input, ControlSettings_t* settings)
{
	if (strlen(input) > 0) {
		settings->calibrationOffset = atoi(input);
	}
}


void HandleRangeScreen(const char* input, ControlSettings_t* settings)
{
	
	int selection = atoi(input);
	
	switch (selection) {
		
		case 1:
			currentScreen_ = SCREEN_LIMIT_LOW;
			break;
		
		case 2:
			currentScreen_ = SCREEN_LIMIT_HIGH;
			break;
		
		default:
			break;
	}

}

void HandleLowLimitScreen(const char* input, ControlSettings_t* settings)
{
	if (strlen(input) > 0) {
		settings->lowTemp = atoi(input);
	}
}

void HandleHighLimitScreen(const char* input, ControlSettings_t* settings)
{
	if (strlen(input) > 0) {
		settings->highTemp = atoi(input);
	}
}


void PrintCurrentScreen(ControlSettings_t* settings)
{
	
	switch (currentScreen_) {
			
		case SCREEN_MAIN:
			PrintMainScreen();
			break;
		
		case SCREEN_SCALE:
			PrintScaleScreen(settings);
			break;
		
		case SCREEN_CALIBRATION:
			PrintCalibrationScreen(settings);
			break;
		
		case SCREEN_LIMIT_RANGE:
			PrintRangeScreen(settings);
			break;
		
		case SCREEN_LIMIT_LOW:
			PrintLowLimitScreen(settings);
			break;
	
		case SCREEN_LIMIT_HIGH:
			PrintHighLimitScreen(settings);
			break;
		
		default:
			break;
	}
	
}

void Console_HandleInput(char c, ControlSettings_t* settings)
{
	static char buffer[MAX_BUFFER]; // Up to (MAX_BUFFER - 1) chars + null byte.
	static int bufferLen = 0;
	
	if (c == CHAR_ESCAPE) {
		
		switch (currentScreen_) {
				
			case SCREEN_SCALE:
			case SCREEN_CALIBRATION:
			case SCREEN_LIMIT_RANGE:	
				currentScreen_ = SCREEN_MAIN;
				break;
			
			case SCREEN_LIMIT_LOW:
			case SCREEN_LIMIT_HIGH:	
				currentScreen_ = SCREEN_LIMIT_RANGE;
			
			default:
				break;
		}
		
		PrintCurrentScreen(settings);
		
	}
	else if (c == CHAR_ENTER) {
		
		switch (currentScreen_) {
			
			case SCREEN_MAIN:
				HandleMainScreen(buffer);
				break;
			
			case SCREEN_SCALE:
				HandleScaleScreen(buffer, settings);
				break;
			
			case SCREEN_CALIBRATION:
				HandleCalibrationScreen(buffer, settings);
				break;
			
			case SCREEN_LIMIT_RANGE:
				HandleRangeScreen(buffer, settings);
				break;
			
			case SCREEN_LIMIT_LOW:
				HandleLowLimitScreen(buffer, settings);
				break;
			
			case SCREEN_LIMIT_HIGH:
				HandleHighLimitScreen(buffer, settings);
				break;
			
		}
	
		PrintCurrentScreen(settings);
		
		buffer[0] = '\0';
		bufferLen = 0;
		
	}
	else if (c == CHAR_BACKSPACE) { 
		
		// If this doesn't back up before the start of the buffer (and prompt '>' sign)
		//	then remove and echo back...
		if (bufferLen > 0) {	
			buffer[--bufferLen] = '\0';
			UART_WriteChar(uart_, c);
		}	
	}
	else {
		
		// Stop buffering the input if max. length exceeded.
		if (bufferLen < MAX_BUFFER - 1) {
			buffer[bufferLen++] = c;
			buffer[bufferLen] = '\0';
		}
		
		// Echo back the character...
		UART_WriteChar(uart_, c);
	
	}
	
}



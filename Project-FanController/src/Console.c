// File:  Console.h
// Author: JSpicer
// Date:  11/25/17
// Purpose: Serial port console interaction for Temperature Control settings
// Hardware:  TM4C123 Tiva board

#include "HAL.h"
#include "Strings.h"
#include "TemperatureSettings.h"

// This is the maximum size of the input buffer.  If the user types
//	more characters than this they will be discarded.  Make sure the
//	input does require more than this. 
#define MAX_BUFFER				16

// These characters trigger specific console actions.
#define CHAR_ESCAPE				0x1B
#define CHAR_ENTER				'\r'
#define CHAR_BACKSPACE		0x7F

// These are monikers for the console screens.
enum {
	SCREEN_MAIN,
	SCREEN_SCALE,
	SCREEN_CALIBRATION,
	SCREEN_LIMIT_RANGE,
	SCREEN_LIMIT_LOW,
	SCREEN_LIMIT_HIGH
} currentScreen_;
	
	
// The UART id will bet stored here during Init for other module functions to use.
static UART_ID_t uart_;

//-------------------------- Console_Init----------------------------
// Initializes the Console module.  Must be called once during program 
//	start up.
// Inputs:  uart - The HAL UART_ID that will be used for console I/O.
// Outputs:  none
void Console_Init(UART_ID_t uart)
{
	
	// Store the UART id for other functions to use.
	uart_ = uart;
	
	// Start at the main screen.
	currentScreen_ = SCREEN_MAIN;
}


// Prints the main console menu.
static void PrintMainScreen(void)
{
	UART_WriteString(uart_, "\n\r\n\r");		
	UART_WriteString(uart_, "Temperature Control Settings:\n\r");
	UART_WriteString(uart_, " 1) Scale\n\r");
	UART_WriteString(uart_, " 2) Calibration Offset\n\r");
	UART_WriteString(uart_, " 3) Temperature Limits\n\r");
	UART_WriteString(uart_, ">");	
}

// Prints the temperature scale screen.
static void PrintScaleScreen(TemperatureSettings_t* settings)
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
static void PrintCalibrationScreen(TemperatureSettings_t* settings)
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
static void PrintRangeScreen(TemperatureSettings_t* settings)
{
	char low[16];
	char high[16];
	itoa(settings->lowTemp, low);
	itoa(settings->highTemp, high);

	UART_WriteString(uart_, "\n\r\n\r");	
	
	UART_WriteString(uart_, "Temperature Limits: ");
	UART_WriteString(uart_, low);
	UART_WriteString(uart_, "-");
	UART_WriteString(uart_, high);
	UART_WriteString(uart_,"\n\r");
		
	UART_WriteString(uart_, "Select the temperature limit to change (ESC for previous menu):\n\r");
	UART_WriteString(uart_, " 1) Low (Minimum Speed)\n\r");
	UART_WriteString(uart_, " 2) High (Maximum Speed)\n\r");
	UART_WriteString(uart_, ">");	
}

// Prints the low temperature limit.
static void PrintLowLimitScreen(TemperatureSettings_t* settings)
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
static void PrintHighLimitScreen(TemperatureSettings_t* settings)
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

// Handles input for the main screen.
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

// Handles input for the Scale screen.
int HandleScaleScreen(const char* input, TemperatureSettings_t* settings)
{
	
	int save = 0;
	int selection = atoi(input);
	
	switch (selection) {
		
		case 1:
			settings->scale = THERM_FAHRENHEIT;
			save = 1;
			break;
		
		case 2:
			settings->scale = THERM_CELSIUS;
			save = 1;
			break;
		
		default:
			break;
	}

	return save;
	
}

// Handles input for the calibration screen.
int HandleCalibrationScreen(const char* input, TemperatureSettings_t* settings)
{
	if (strlen(input) > 0) {
		settings->calibrationOffset = atoi(input);
		return 1;
	}
	
	return 0;
}

// Handles input for the temperature range screen.
void HandleRangeScreen(const char* input)
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

// Handles input for the LOW limit screen.
int HandleLowLimitScreen(const char* input, TemperatureSettings_t* settings)
{
	if (strlen(input) > 0) {
		settings->lowTemp = atoi(input);
		return 1;
	}
	
	return 0;
}

// Handles input for the HIGH limit screen.
int HandleHighLimitScreen(const char* input, TemperatureSettings_t* settings)
{
	if (strlen(input) > 0) {
		settings->highTemp = atoi(input);
		return 1;
	}
	
	return 0;
}

// Selection logic to decide which screen to print.Prints the current screen.
void PrintCurrentScreen(TemperatureSettings_t* settings)
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

//----------------------- Console_HandleInput -----------------------
// Handles the input of single character typed to the console.
// Inputs:  c - the character typed.
//          settings - a pointer to the TemperatureSettings structure
//		         to display and save to Flash.
// Outputs: none
void Console_HandleInput(char c, TemperatureSettings_t* settings)
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
		
		int save = 0;
		
		switch (currentScreen_) {
			
			case SCREEN_MAIN:
				HandleMainScreen(buffer);
				break;
			
			case SCREEN_SCALE:
				save = HandleScaleScreen(buffer, settings);
				break;
			
			case SCREEN_CALIBRATION:
				save = HandleCalibrationScreen(buffer, settings);
				break;
			
			case SCREEN_LIMIT_RANGE:
				HandleRangeScreen(buffer);
				break;
			
			case SCREEN_LIMIT_LOW:
				save = HandleLowLimitScreen(buffer, settings);
				break;
			
			case SCREEN_LIMIT_HIGH:
				save = HandleHighLimitScreen(buffer, settings);
				break;
			
		}
		
		// If the handler assigned a setting, then write the settings to Flash.
		if (save) {
			Flash_Write(settings, sizeof(TemperatureSettings_t) / 4);
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



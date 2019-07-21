// File:  LCD.h
// Author: JSpicer
// Date:  12/23/17
// Purpose: LCD driver for 16x1, 16x2, and 20x4 displays.  Augments and simplifies the raw driver.
// Hardware:  TM4C123 Tiva board

#include "LCD.h"
#include "Strings.h"

int LCD_Initialize(LCDDisplay_t* display, const LCDPinConfig_t* pinConfig, int rows, int columns)
{
	
	if (LCD_RawInitialize(&display->raw, pinConfig,  LCD_LINEMODE_2, LCD_FONTMODE_5X8) < 0) {
		return -1;
	}
	
	display->rows = rows;
	display->columns = columns;
	
	LCD_RawSetDisplayControl(&display->raw, 1, 1, 0);
	LCD_RawSetEntryMode(&display->raw, 1, 0);
	LCD_RawClearDisplay(&display->raw);
	
	return 0;
}

static int GetDDRAMAddress(LCDDisplay_t* display, int row, int column)
{

	// Column out of bounds
	if (column < 0 || column >= display->columns) {
		return -1;
	}
	
	if (row < 0 || row >= display->rows) {
		return -1;
	}
	
	int address = -1;
	
	switch (row) {
			
		case 0: 
	
			if (display->rows == 1 && column >= 8) {
				address = 0x40 + (column - 8);
			}
			else {
				address = column;
			}
			break;
			
		case 1:
			address = 0x40 + column;
			break;
		
		case 2:
			address = 0x14 + column;
			break;
		
		case 3:
			address = 0x54 + column;
			break;
		
		default:
			break;
	}
	
	return address;
	
}

void LCD_SetCursorPosition(LCDDisplay_t* display, int row, int column)
{

	// For columns, wrap around to the other side if out of bounds.
	if (column < 0) {
		column = display->columns - 1;
	}
	else if (column >= display->columns) {
		column = 0;
	}
	
	// For rows, do not wrap around but clip instead.
	if (row < 0) {
		row = 0;
	}
	else if (row >= display->rows) {
		row = display->rows - 1;
	}
	
	int address = GetDDRAMAddress(display, row, column);
	if (address >= 0) {
		LCD_RawSetDDRAMAddress(&display->raw, (uint8_t)address);
		display->cursorRow = row;
		display->cursorColumn = column;
	}
}

void LCD_ShiftCursor(LCDDisplay_t* display, LCDCursorDirection_t direction) 
{
	
	int row = display->cursorRow;
	int column = display->cursorColumn;	
	
	switch (direction) {
		case LCD_CURSOR_RIGHT:
			column++;
			break;
		case LCD_CURSOR_LEFT:
			column--;
			break;
		case LCD_CURSOR_DOWN:
			row++;
			break;
		case LCD_CURSOR_UP:
			row--;
			break;
	}
	
	LCD_SetCursorPosition(display, row, column);
	
}

void LCD_PutChar(LCDDisplay_t* display, char c, int row, int column)
{
	int address = GetDDRAMAddress(display, row, column);
	if (address >= 0) {
		
		LCD_RawSetDDRAMAddress(&display->raw, (uint8_t)address);
		LCD_RawPutChar(&display->raw, c);
		
		// Set the cursor back to its original position.
		LCD_SetCursorPosition(display, display->cursorRow, display->cursorColumn);
	}
}

void LCD_PutString(LCDDisplay_t* display, const char* s, int row, int column)
{
	int length = strlen(s);
	
	// The string will be truncated if it is greater than the display width.
	for (int i = 0; i < length; i++) {
		
		int address = GetDDRAMAddress(display, row, column+i);
		if (address < 0) {
			break;
		}
		
		LCD_RawSetDDRAMAddress(&display->raw, (uint8_t)address);
		
		char c = ' ';
		if (i < length) {
			c = s[i];
		}
		
		LCD_RawPutChar(&display->raw, c);
			
	}
	
	// Set the cursor back to its original position.
	LCD_SetCursorPosition(display, display->cursorRow, display->cursorColumn);
	
}


void LCD_EnableCursor(LCDDisplay_t* display, uint8_t enable, uint8_t blinking)
{
	LCD_RawSetDisplayControl(&display->raw, 1, enable, blinking);
}


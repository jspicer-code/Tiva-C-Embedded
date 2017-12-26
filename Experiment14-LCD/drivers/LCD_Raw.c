// File:  LCD_Raw.h
// Author: JSpicer
// Date:  12/23/17
// Purpose: LCD driver for Hitachi HD44780 controller variants.
// Hardware:  TM4C123 Tiva board
//
// Tested with:  	Lumex LCM-X01601DSR (16x1)
//								Techstar TS1620A-21/B (16x2)		

#include "LCD_Raw.h"
#include "Strings.h"

#define LINE_RAM_SIZE		0x40

static void SetDataDirection(LCDRawDisplay_t* display, enum DataDirection_t direction)
{
	
	if (direction != display->dataDirection) {
		
		// Set direction for the top four pins if using 4-bit data length, otherwise set all eight.
		int startPin = (display->pinConfig.dataLen == LCD_DATALEN_4) ? 4 : 0;
		
		for (int i = startPin; i < 8; i++) {
			if (direction == DATA_WRITE) {
				GPIO_EnableDO(display->pinConfig.dataPins[i].port, display->pinConfig.dataPins[i].pin, DRIVE_2MA, PULL_DOWN);
			}
			else if (direction == DATA_READ) {
				GPIO_EnableDI(display->pinConfig.dataPins[i].port, display->pinConfig.dataPins[i].pin, PULL_DOWN);			
			}
		}
		
		display->dataDirection = direction;
	}
	
}

// In 4-bit data length mode, this transfers the upper nibble of value, 
//	otherwise it transfers all 8 bits of value.
static void Transfer(LCDRawDisplay_t* display, uint8_t value, uint8_t isData)
{
	SetDataDirection(display, DATA_WRITE);
	
	// RS=0 instruction register, RS=1 data register
	*display->rsPinAddress = isData;
	
	// Perform write operation.
	*display->rwPinAddress = 0;
	
	// Wait at least 40ns before asserting the ENABLE pin.
	SysTick_Wait10us(1);
	
	// Assert ENABLE
	*display->enablePinAddress = 1;
	
	// For four-bit mode, transfer the upper nibble, otherwise transfer the whole byte.
	int startPin = (display->pinConfig.dataLen == LCD_DATALEN_4) ? 4 : 0;
	
	// Write the data to the data pins.
	for (int i = startPin; i < 8; i++) {
		*display->dataPinAddresses[i] = 0x01 & (value >> i);
	}
	
	// Wait at least 80ns before de-asserting the ENABLE pin.
	SysTick_Wait10us(1);
	
	// De-assert ENABLE.  The falling edge should latch the value.
	*display->enablePinAddress = 0;
	
	// Wait at least 10ns before another value is written. 
	SysTick_Wait10us(1);
}


void TransferByte(LCDRawDisplay_t* display, uint8_t value, uint8_t isData)
{
	
	if (display->pinConfig.dataLen == LCD_DATALEN_4) {
		
		// Transfer the upper nibble.  (The lower nibble is ignored).
		Transfer(display, value, isData);
		
		// Shift the lower nibble into the upper and transfer it.
		Transfer(display, value << 4, isData);
		
	}
	else {
		// Transfer all 8 bits.
		Transfer(display, value, isData);
	}
	
}


void LCD_RawSetFunction(LCDRawDisplay_t* display, uint8_t dataBitMode, uint8_t lineMode, uint8_t fontMode)
{
	dataBitMode = (dataBitMode) ? 0x10 : 0;
	lineMode = (lineMode) ? 0x08 : 0;
	fontMode = (fontMode) ? 0x04 : 0;
	
	// Function Set, DL=1 8-bit mode, N=1 1-line mode, F=0 5x8 font
	TransferByte(display, 0x20 | dataBitMode | lineMode | fontMode, 0);
	
	// Wait 40us.
	SysTick_Wait10us(4);
	
}

void LCD_RawSetDisplayControl(LCDRawDisplay_t* display, uint8_t displayOn, uint8_t cursorOn, uint8_t blinkOn)
{
	
	displayOn = (displayOn) ? 0x04 : 0;
	cursorOn = (cursorOn) ? 0x02 : 0;
	blinkOn = (blinkOn) ? 0x01 : 0;
	
	TransferByte(display, 0x08 | displayOn | cursorOn | blinkOn, 0);
	
	// Wait 40us.
	SysTick_Wait10us(4);
	
}


void LCD_RawClearDisplay(LCDRawDisplay_t* display)
{
	TransferByte(display, 0x01, 0);
	
	// Wait 2ms
	SysTick_Wait10us(200);
}


void LCD_RawShiftCursor(LCDRawDisplay_t* display, uint8_t right)
{
	
	right = (right) ? 0x04 : 0;
	
	TransferByte(display, 0x10 | right, 0);
	
	// Wait 40us.
	SysTick_Wait10us(4);
	
}


void LCD_RawShiftDisplay(LCDRawDisplay_t* display, uint8_t right)
{
	
	right = (right) ? 0x04 : 0;
	
	TransferByte(display, 0x18 | right, 0);
	
	// Wait 40us.
	SysTick_Wait10us(4);
	
}


void LCD_RawSetEntryMode(LCDRawDisplay_t* display, uint8_t incrementMode, uint8_t shiftOn)
{
	
	incrementMode = (incrementMode) ? 0x02 : 0;
	shiftOn = (shiftOn) ? 0x01 : 0;
	
	TransferByte(display, 0x04 | incrementMode | shiftOn, 0);
	
	// Wait 40us
	SysTick_Wait10us(4);
}


void LCD_RawPutChar(LCDRawDisplay_t* display, char c)
{
	TransferByte(display, (uint8_t)c, 1);
	
	// Wait 50us.
	SysTick_Wait10us(5);
}

void LCD_RawPutString(LCDRawDisplay_t* display, const char* s)
{
	int length = strlen(s);
		
	// The string will be truncated if it is greater than the single line RAM size.
	// TODO:  May need to check the starting RAM address and prevent it from wrapping around 
	//	pass address 0x40
	for (int i = 0; i < length && i < LINE_RAM_SIZE; i++) {
		LCD_RawPutChar(display, s[i]);
	}
	
}


void LCD_RawSetDDRAMAddress(LCDRawDisplay_t* display, uint8_t address)
{
	TransferByte(display, 0x80 | address, 0);
	
	// Wait 40us
	SysTick_Wait10us(4);
}


void LCD_RawReturnHome(LCDRawDisplay_t* display)
{
	TransferByte(display, 0x02, 0);
	
	// Wait 2ms
	SysTick_Wait10us(200);
}


int LCD_RawInitialize(LCDRawDisplay_t* display, const LCDPinConfig_t* config, LCDLineMode_t lineMode, LCDFontMode_t fontMode)
{
	
	// Store a copy of the configuration.
	display->pinConfig = *config;
	
	// Initial value...
	display->dataDirection = DATA_UNCONFIGURED;
	
	// For four-bit mode, we only care about the top four data pins in the configuration.
	int startPin = (config->dataLen == LCD_DATALEN_4) ? 4 : 0;
	
	// Get the bit band aliases...
	for (int i = startPin; i < 8; i++) {
		display->dataPinAddresses[i] = GPIO_GetBitBandIOAddress(&config->dataPins[i]);
	}
	
	display->rsPinAddress = GPIO_GetBitBandIOAddress(&config->rsPin);
	display->rwPinAddress = GPIO_GetBitBandIOAddress(&config->rwPin);
	display->enablePinAddress = GPIO_GetBitBandIOAddress(&config->enablePin);

	// The RS, RW, and E pins will always be digital outputs.
	GPIO_EnableDO(config->rsPin.port, config->rsPin.pin, DRIVE_2MA, PULL_DOWN);
	GPIO_EnableDO(config->rwPin.port, config->rwPin.pin, DRIVE_2MA, PULL_DOWN);
	GPIO_EnableDO(config->enablePin.port, config->enablePin.pin, DRIVE_2MA, PULL_DOWN);

	// Wait 100ms for LCD input voltage to stabilize.
	SysTick_Wait10ms(10);
	
	// This initialization sequence is required when the power conditions for correctly
	//	operating the internal reset circuit were NOT met.  This follows the steps for
	//  "initialization by instructions" in the Hitachi datasheet.  Note that performing
	//	these instructions when the display has already done an internal reset can cause the
	//	display to enter an incomplete state, and ignore subsequent instructions.  
	//	Therefore, set "initByInstruction" only when needed.
	if (config->initByInstruction) {

		Transfer(display, 0x30, 0);
	
		// Wait 5ms...
		SysTick_Wait10us(500);
		
		// Again...
		Transfer(display, 0x30, 0);
		
		// Wait 100us...
		SysTick_Wait10us(10);

		// And a third time...
		Transfer(display, 0x30, 0);
	
	}
		
	// ************************************************************
	// At this point, the display should be initialized internally, 
	//	so begin the regular initialization sequence...
	// ************************************************************
	
	// Set Function with the caller's data (bus) length, line, and font mode configurations.
	LCD_RawSetFunction(display, (uint8_t)config->dataLen, (uint8_t)lineMode, (uint8_t)fontMode);
	
	// Turn the display, cursor, and blink off.
	LCD_RawSetDisplayControl(display, 0, 0, 0);
	
	// Clear the display.
	LCD_RawClearDisplay(display);
	
	// Turn increment mode on and display shift off.
	LCD_RawSetEntryMode(display, 1, 0);
	
	return 0;
}




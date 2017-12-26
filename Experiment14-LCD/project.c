// File:  project.c
// Author: JSpicer
// Date:  12/23/17
// Purpose: Experiment to test out Hitachi HD44780 based LCDs.
//          The program operates a 16x1 and 16x2 LCD simultaneously.
//          A 5-position switch navigates the cursor and entry modes.
//          A 2-position switch directs input to one display or the other.
//          Holding down one of the onboard buttons creates a scrolling marquee effect.
// Hardware:  TM4C123 Tiva board

#include <HAL.h>
#include "LCD.h"
#include "Switch5Position.h"
#include "Strings.h"

// Character choices for input entry mode...
static const char choicesLower[] = " abcdefghijklmnopqrstuvwxyz1234567890.+/=:\"";
static const char choicesUpper[] = "_ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*(),-|?;\'";
	
// Holds the content and edit state for one display.
typedef struct {
	
	char line1[17];
	char line2[17];
	
	uint8_t editing;
	uint8_t upperOn;
	
} DisplayContent_t;

// Switch bit-band aliases
static volatile uint32_t* heartbeatLED_;
static volatile uint32_t* displaySwitch_;
static volatile uint32_t* resetSwitch_;
static volatile uint32_t* marqueeSwitch_;

// Display contexts.
static LCDDisplay_t display1_;
static LCDDisplay_t display2_;

// Display content.
static DisplayContent_t content1_;
static DisplayContent_t content2_;

// Indicates which of the two display is currently active (i.e. cursor on, taking input).
static int activeDisplay_ = 0;

// Call when there is a catastophic problem.
void ErrHandler(void)
{
	while (1) {}
}

// Examines the state of the 5-position switch moves the cursor.
void ShiftCursor(LCDDisplay_t* display, uint32_t sw)
{

	LCDCursorDirection_t direction;

	// The switch is rotated 90 degrees clockwise on the breadboard, so the left contact is UP, 
	//	the up contact is RIGHT, etc. 
	if (SWITCH5_HASCLOSED(sw, SWITCH5_UP)) {
		direction = LCD_CURSOR_RIGHT;
	}
	else if (SWITCH5_HASCLOSED(sw, SWITCH5_DOWN)) {
		direction = LCD_CURSOR_LEFT;
	}
	else if (SWITCH5_HASCLOSED(sw, SWITCH5_LEFT)) {
		direction = LCD_CURSOR_UP;
	}
	else {
		direction = LCD_CURSOR_DOWN;
	}

	LCD_ShiftCursor(display, direction);
}

// Resets the displays to there default content.
void ResetContent(void) 
{
	content1_.editing = 0;
	content2_.editing = 0;
	
	strncpy(content1_.line1, "Hello...        ", 16);
	strncpy(content1_.line2, "      Display 1!", 16);
	strncpy(content2_.line1, "Hello Display 2!", 16);
	strncpy(content2_.line2, "                ", 16);
	
	LCD_PutString(&display1_, content1_.line1, 0, 0);
	LCD_PutString(&display1_, content1_.line2, 1, 0);
	LCD_PutString(&display2_, content2_.line1, 0, 0);	
	
	LCD_SetCursorPosition(&display1_, 0, 0);
	LCD_SetCursorPosition(&display2_, 0, 0);

	// This will cause the cursor to be enabled on whichever display
	//	is selected (when display switch is processed).
	activeDisplay_ = 0;
	
}

// Handles character edit mode.
void EditCharacter(LCDDisplay_t* display, DisplayContent_t* content, uint32_t sw)
{

	// Get the current line and character.
	char* line = (display->cursorRow == 0) ? content->line1 : content->line2;
	char c = line[display->cursorColumn];
	
	// Locate the character in the character choices array(s).
	int choiceIndex = -1;
	int choiceLength = strlen(choicesLower);
	for (int i = 0; i < choiceLength; i++) {

		if (choicesUpper[i] == c || choicesLower[i] == c) {
			choiceIndex = i;
			content->upperOn = (choicesUpper[i] == c);
			break;
		}
	
	}
	
	// The switch is rotated 90 degrees clockwise on the breadboard, so the left contact is UP, 
	//	the up contact is RIGHT, etc. 
	if (SWITCH5_HASCLOSED(sw, SWITCH5_UP)) {
		choiceIndex++;
	}
	else if (SWITCH5_HASCLOSED(sw, SWITCH5_DOWN)) {
		choiceIndex--;
	}
	else if (SWITCH5_HASCLOSED(sw, SWITCH5_LEFT)) {
		content->upperOn = 1;
	}
	else {
		content->upperOn = 0;
	}

	if (choiceIndex < 0) {
		choiceIndex = choiceLength - 1;
	}
	else if (choiceIndex >= choiceLength) {
		choiceIndex = 0;
	}
	
	// Get the chosen character and subsitute it into the line.
	c = (content->upperOn) ? choicesUpper[choiceIndex] : choicesLower[choiceIndex];
	line[display->cursorColumn] = c;
	
	// Display the chosen character.
	LCD_PutChar(display, c, display->cursorRow, display->cursorColumn);
	
}

// Causes the displays to side scroll as if they are one, 32-character display.
void ScrollMarquee(void)
{
	static int marqueeCounter = 0;
	
	// Shift the content every N intervals of the timer.
	marqueeCounter = ++marqueeCounter % 3;
	
	if (marqueeCounter == 0) {
		
		char line1[33];
		char line2[33];
	
		// Copy the line content into single character arrays.
		for (int i = 0; i < 16; i++) {
			line1[i] = content1_.line1[i];
			line1[16 + i] = content2_.line1[i];
			line2[i] = content1_.line2[i];
			line2[16 + i] = content2_.line2[i];
		}
		
		// Shift/copy the lines back into their original arrays. 
		for (int i = 0; i < 16; i++) {
			
			int index1 = (i + 1) % 32;
			content1_.line1[i] = line1[index1];
			content1_.line2[i] = line2[index1];
			
			int index2 = (i + 17) % 32;
			content2_.line1[i] = line1[index2];
			content2_.line2[i] = line2[index2];
			
		}
		
		// Display the shifted content...
		LCD_PutString(&display1_, content1_.line1, 0, 0);
		LCD_PutString(&display1_, content1_.line2, 1, 0);
		LCD_PutString(&display2_, content2_.line1, 0, 0);
		
	}
	
}

// Check the state of all switches and take actions accordingly.
void ProcessSwitchState(uint32_t sw) 
{

	if (*displaySwitch_ && activeDisplay_ != 2) {
		
		// Turn off the cursor for display 1
		content1_.editing = 0;
		LCD_EnableCursor(&display1_, 0, 0); 
		
		// Turn on the cursor for display 2
		LCD_EnableCursor(&display2_, 1, content2_.editing);
	
		activeDisplay_ = 2;
	
	}
	else if (!*displaySwitch_ && activeDisplay_ != 1) {
		
		// Turn off the cursor for display 2
		content2_.editing = 0;
		LCD_EnableCursor(&display2_, 0, 0); 
		
		// Turn on the cursor for display 1
		LCD_EnableCursor(&display1_, 1, content1_.editing);

		activeDisplay_ = 1;
	}
	
	// Reference the active display and content...
	LCDDisplay_t* display = (activeDisplay_ == 1) ? &display1_ : &display2_;
	DisplayContent_t* content = (activeDisplay_ == 1) ? &content1_ : &content2_;

	if (!*resetSwitch_) {
		ResetContent();
	}
	if (!*marqueeSwitch_) {
		ScrollMarquee();
	}
	else if (SWITCH5_HASCLOSED(sw, SWITCH5_CENTER)) {
		
		// Toggle editing mode and turn cursor blinking on/off...
		content->editing = !content->editing;
		LCD_EnableCursor(display, 1, content->editing);
		
	}
	else if (SWITCH5_HASCLOSED(sw, SWITCH5_UP | SWITCH5_DOWN | SWITCH5_LEFT | SWITCH5_RIGHT)) {

		// Shift the cursor or a choose a new character selection depending on the editing state.
		if (!content->editing) {
			ShiftCursor(display, sw);
		}
		else {
			EditCharacter(display, content, sw);
		}			
		
	}
	
}

// Called periodically by the timer ISR.
void TimerCallback(void)
{
	uint32_t sw = Switch5_Poll();
		
	ProcessSwitchState(sw);
}

// Initialize the hardware and peripherals...
int InitHardware(void)
{

	__disable_irq();
	
	PLL_Init80MHz();

	if (SysTick_Init() < 0) {
		return -1;
	}
	
	GPIO_EnableDO(PORTF, PIN3, DRIVE_2MA, PULL_DOWN);
	heartbeatLED_ = GPIO_GetBitBandIOAddress(&PINDEF(PORTF, PIN3));
	
	GPIO_EnableDI(PORTF, PIN0 | PIN4, PULL_UP);
	resetSwitch_ = GPIO_GetBitBandIOAddress(&PINDEF(PORTF, PIN4));
	marqueeSwitch_ = GPIO_GetBitBandIOAddress(&PINDEF(PORTF, PIN0));
	
	GPIO_EnableDI(PORTE, PIN0, PULL_NONE);
	displaySwitch_ = GPIO_GetBitBandIOAddress(&PINDEF(PORTE, PIN0));
	
	// Display 1 is 16x2.  Using a 4-bit bus, only need to set data pins 4-7.
	LCDPinConfig_t pinConfig1;
	pinConfig1.dataPins[4] = PINDEF(PORTE, PIN4);
	pinConfig1.dataPins[5] = PINDEF(PORTE, PIN5);
	pinConfig1.dataPins[6] = PINDEF(PORTA, PIN6);
	pinConfig1.dataPins[7] = PINDEF(PORTA, PIN7);	
	pinConfig1.rsPin = PINDEF(PORTD, PIN0);
	pinConfig1.rwPin = PINDEF(PORTD, PIN1);
	pinConfig1.enablePin = PINDEF(PORTD, PIN2);
	pinConfig1.dataLen = LCD_DATALEN_4;
	pinConfig1.initByInstruction = 0;
	if (LCD_Initialize(&display1_, &pinConfig1, 2, 16) < 0) {
		return -1;
	}
	
	// Display 2 is 16x1.  Using a 4-bit bus, only need to set data pins 4-7.
	LCDPinConfig_t pinConfig2;
	pinConfig2.dataPins[4] = PINDEF(PORTB, PIN5);	
	pinConfig2.dataPins[5] = PINDEF(PORTE, PIN1);
	pinConfig2.dataPins[6] = PINDEF(PORTE, PIN2);
	pinConfig2.dataPins[7] = PINDEF(PORTE, PIN3);
	pinConfig2.rsPin = PINDEF(PORTA, PIN2);
	pinConfig2.rwPin = PINDEF(PORTA, PIN3);
	pinConfig2.enablePin = PINDEF(PORTA, PIN4);
	pinConfig2.dataLen = LCD_DATALEN_4;
	pinConfig2.initByInstruction = 0;
	if (LCD_Initialize(&display2_, &pinConfig2, 1, 16) < 0) {
		return -1;
	}
		
	Switch5Config_t switch5Config;
	switch5Config.centerPin = PINDEF(PORTB, PIN0);
	switch5Config.upPin = PINDEF(PORTB, PIN1);
	switch5Config.downPin = PINDEF(PORTD, PIN3);
	switch5Config.leftPin = PINDEF(PORTB, PIN4);
	switch5Config.rightPin = PINDEF(PORTA, PIN5);
	if (Switch5_Initialize(&switch5Config) < 0) {
		return -1;
	}
	
	ResetContent();
	
	// Calls back every 100ms (with 80MHz bus clock).
	Timer_EnableTimerPeriodic(TIMER0, 8000000, 7, TimerCallback);
	
	__enable_irq();
	
	return 0;
	
}


int main()
{

	if (InitHardware() < 0) {
		ErrHandler();
	}

	while (1) {
		
		// Toggle the heartbeat LED...
		*heartbeatLED_ = !*heartbeatLED_;
		
		// Wait one second...
		SysTick_Wait10ms(100);
	
	}

}

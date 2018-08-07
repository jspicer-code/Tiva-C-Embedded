#include "DisplayManager.h"
#include "Strings.h"

#define NUM_FIELDS		8	

typedef struct {
	int row;
	int column;
} FieldPosition_t;

typedef enum {
	FIELD_DAY = 0,
	FIELD_MONTH,	
	FIELD_DATE,
	FIELD_YEAR,
	FIELD_HOUR,
	FIELD_MINUTE,
	FIELD_SECOND,
	FIELD_AMPM
} FieldName_t;

static const FieldPosition_t fieldPositions_[NUM_FIELDS] = {
	{ 0, 0 }, 	// Day
	{ 0, 5 },		// Month
	{ 0, 8 },		// Date
	{ 0, 13 },	// Year
	{ 1, 1 },		// Hour
	{ 1, 4 },		// Minute
	{ 1, 7 },		// Second
	{ 1, 9 }		// AM/PM
};

static LCDDisplay_t display_;
static RTC_Clock_t clock_;
static int currentField_;

void FormatDigits(char* s, int value) {
	
	char digits[3];
	int position = 0;
	
	int length = itoa(value, digits);
	if (length == 1) {
		s[position++] = '0';
	}
	strcpy(&s[position], digits);
}

static void RefreshDisplay(void)
{
	char line[17];
	int position = 0;
	char * day;
	
	switch (clock_.day) {
		case 1: 
			day = "Sun"; 
			break;
		case 2: 
			day = "Mon"; 
			break;
		case 3: 
			day = "Tue"; 
			break;
		case 4: 
			day = "Wed"; 
			break;
		case 5: 
			day = "Thu"; 
			break;
		case 6: 
			day = "Fri"; 
			break;
		case 7:
		default: 
			day = "Sat"; 
			break;
	}
	
	strcpy(&line[position], day);
	position += 3;
	
	strcpy(&line[position], " ");
	position++;
	
	FormatDigits(&line[position], clock_.month);
	position += 2;

	strcpy(&line[position], "/");
	position++;
	
	FormatDigits(&line[position], clock_.date);
	position += 2;
	
	strcpy(&line[position], "/");
	position++;
	
	FormatDigits(&line[position], 1970 + clock_.year);
	position += 2;
	
	LCD_PutString(&display_, line, 0, 0);

	position = 0;
	
	FormatDigits(&line[position], clock_.hours);
	position += 2;

	strcpy(&line[position], ":");
	position++;
	
	FormatDigits(&line[position], clock_.minutes);
	position += 2;
	
	strcpy(&line[position], ":");
	position++;
	
	FormatDigits(&line[position], clock_.seconds);
	position += 2;
	
	strcpy(&line[position], " ");
	position++;
	
	char * am_pm = (clock_.am_pm) ? "PM" : "AM";
	strcpy(&line[position], am_pm);
	position += 2;

	LCD_PutString(&display_, line, 1, 0);
}

void Display_UpdateClock(const RTC_Clock_t* clock)
{
	clock_ = *clock;
	RefreshDisplay();
}

void Display_GetClock(RTC_Clock_t* clock)
{
	*clock = clock_;
}

int Display_Init(const LCDPinConfig_t* lcdConfig)
{
	if (Timer_Init(lcdConfig->waitTimer, TIMER_ONESHOT, 0, (PFN_TimerCallback)0)) {
		return -1;
	}
	
	if (LCD_Initialize(&display_, lcdConfig, lcdConfig->rows, lcdConfig->columns) < 0) {
		return -1;
	}
	
	// Disable the cursor.
	LCD_EnableCursor(&display_, 0, 0);
	LCD_SetCursorPosition(&display_, 0, 0);
	
	return 0;
}


void Display_EnableCursor(bool enable)
{
	LCD_EnableCursor(&display_, enable, enable);	
}


static void SetCursorPosition(int field)
{
	if (field >= NUM_FIELDS) {
		field = 0;
	}
	else if (field < 0) {
		field = NUM_FIELDS - 1;
	}
	
	const FieldPosition_t* position = &fieldPositions_[field];
	LCD_SetCursorPosition(&display_, position->row, position->column);
	
	currentField_ = field;
}

void Display_ResetCursorPosition(void)
{
	SetCursorPosition(0);
}

void Display_NextField(void) 
{	
	SetCursorPosition(currentField_ + 1);
}

void Display_PrevField(void) 
{	
	SetCursorPosition(currentField_ - 1);
}

int GetValue(FieldName_t fieldName) 
{
	switch (fieldName) {
		
		case FIELD_DAY:
			return clock_.day;
				
		case FIELD_MONTH:
			return clock_.month;
		
		case FIELD_DATE:
			return clock_.date;
			
		case FIELD_YEAR:
			return clock_.year; 
			
		case FIELD_HOUR:
			return clock_.hours;
		
		case FIELD_MINUTE:
			return clock_.minutes;
			
		case FIELD_SECOND:
			return clock_.seconds;
			
		case FIELD_AMPM:
		default:
			return clock_.am_pm;
		
	}
	
}

void SetValue(FieldName_t fieldName, int value) 
{
	switch (fieldName) {
		
		case FIELD_DAY:
				
			if (value < 1) {
				value = 7;
			}
			else if (value > 7) {
				value = 1;
			}
			clock_.day = value;
			break;
				
		case FIELD_MONTH:
				
			if (value < 0) {
				value = 12;
			}
			else if (value > 12) {
				value = 1;
			}
			clock_.month = value;
			break;
		
		case FIELD_DATE:
			
		if (value < 0) {
				value = 31;
			}
			else if (value > 31) {
				value = 1;
			}
			clock_.date = value;
			break;
			
		case FIELD_YEAR:
			
			if (value < 0) {
				value = 0;
			}
			else if (value > 99) {
				value = 0;
			}
			clock_.year = value;
			break; 
			
		case FIELD_HOUR:
		
			if (value < 1) {
				value = 12;
			}
			else if (value > 12) {
				value = 1;
			}
			clock_.hours = value;
			break;
		
		case FIELD_MINUTE:
			
			if (value < 0) {
				value = 59;
			}
			else if (value > 59) {
				value = 0;
			}
			clock_.minutes = value;
			break;
			
		case FIELD_SECOND:
			
			if (value < 0) {
				value = 59;
			}
			else if (value > 59) {
				value = 0;
			}
			clock_.seconds = value;
			break;
			
		case FIELD_AMPM:

			if (value < 0) {
				value = 1;
			}
			else if (value > 1) {
				value = 0;
			}
			clock_.am_pm = value;
			break;
		
	}
	
}

void Display_NextValue(void) 
{	
	FieldName_t fieldName = (FieldName_t)currentField_;
	SetValue(fieldName, GetValue(fieldName) + 1);
	RefreshDisplay();
}

void Display_PrevValue(void) 
{	
	FieldName_t fieldName = (FieldName_t)currentField_;
	SetValue(fieldName, GetValue(fieldName) - 1);
	RefreshDisplay();
}



#include "DisplayManager.h"
#include "Strings.h"

static LCDDisplay_t display_;

void PadStringInPlace(char* s, int size)
{
	int sourceLength = strlen(s);
	for (int i = sourceLength; i < size - 1; i++) {
		s[i] = ' ';
	}
	s[size - 1] = '\0';
}

void Display_ShowFileNotLoadedScreen(void)
{
	LCD_PutString(&display_, "                    ", 0, 0);
	LCD_PutString(&display_, "  File Not Loaded   ", 1, 0);
	LCD_PutString(&display_, "                    ", 2, 0);
	LCD_PutString(&display_, "                    ", 3, 0);
}


void Display_ShowSongScreen(const Song_t* song, int songNumber, int songCount)
{

	int length;
	char line[21];
		
	strncpy(line, song->header->name, 21);
	PadStringInPlace(line, 21);
	LCD_PutString(&display_, line, 0, 0);

	strcpy(line, "Song ");
	length = 5;
	length += itoa(songNumber, &line[length]);
	strcpy(&line[length], " of ");
	length += 4;
	length += itoa(songCount, &line[length]);
	PadStringInPlace(line, 21);
	LCD_PutString(&display_, line, 1, 0);
	
	length = itoa(song->header->numTracks, line);
	strcpy(&line[length], " tracks ");
	length += 8;
	length += itoa(song->header->size, &line[length]); 
	strcpy(&line[length], " bytes");
	PadStringInPlace(line, 21);
	LCD_PutString(&display_, line, 2, 0);

}


void Display_ShowDownloadingScreen(int percentage)
{
	int length;
	char line[21];
	char digits[21];
	
	LCD_PutString(&display_, "                    ", 0, 0);
	LCD_PutString(&display_, "   Downloading...   ", 1, 0);

	length = itoa(percentage, digits);
	int digitPosition = 10 - length;
	for (int i = 0; i < digitPosition; i++) {
		line[i] = ' ';
	}
	strcpy(&line[digitPosition], digits);
	strcpy(&line[digitPosition + length], "%");
	PadStringInPlace(line, 21);
	LCD_PutString(&display_, line, 2, 0);
	
	LCD_PutString(&display_, "                    ", 3, 0);
	
}

static int FormatDuration(char* s, int duration)
{
	int length;
	char digits[6];
	int position = 0;
	
	int minutes = duration / 60;
	int seconds = duration % 60;
	
	length = itoa(minutes, digits);
	for (int i = 0; i < length; i++) {
		s[position++] = digits[i];
	}
	
	s[position++] = ':';
	
	length = itoa(seconds, digits);
	if (length == 1) {
		s[position++] = '0';
	}
	
	for (int i = 0; i < length; i++) {
		s[position++] = digits[i];
	}
	
	s[position] = '\0';
	
	return position;
}

void Display_ShowPlayDuration(int playDuration, int songDuration)
{
	int length;
	char duration[10];
	char line[21];
	int position = 0;

	length = FormatDuration(duration, playDuration);
	strncpy(&line[position], duration, 21);
	position += length;
	
	line[position++] = '/';
	
	length = FormatDuration(duration, songDuration);
	strncpy(&line[position], duration, 21);
	position += length;
	
	PadStringInPlace(line, 21);
	LCD_PutString(&display_, line, 3, 0);
	
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

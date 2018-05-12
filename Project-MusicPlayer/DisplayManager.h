#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include "LCD.h"
#include "Song.h"

int Display_Init(const LCDPinConfig_t* lcdConfig);
void Display_ShowSongScreen(const Song_t* song, int songNumber, int songCount);
void Display_ShowDownloadingScreen(int percentage);
void Display_ShowFileNotLoadedScreen(void);
void Display_ShowPlayDuration(int playDuration, int songDuration);

#endif

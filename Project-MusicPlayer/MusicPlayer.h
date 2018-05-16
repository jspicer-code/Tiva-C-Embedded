// File:  MusicPlayer.h
// Author: JSpicer
// Date:  5/15/18
// Purpose: Player service layer module
// Hardware:  TM4C123 Tiva board

#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <stdint.h>
#include "HAL.h"
#include "LCD.h"

// This structure identifies all hardware HAL ports, pins, and modules to use.
typedef struct {
	
	PinDef_t	playButton;
	PinDef_t	nextButton;
	PinDef_t	prevButton;
	
	PWMDef_t 	trackPWM[4];
	PinDef_t	trackLED[4];
	
	UART_ID_t	uart;
	
	LCDPinConfig_t lcd;
	
	volatile uint32_t* flashBaseAddress;
	uint32_t	flashBlocks;	
	
	TimerBlock_t noiseTimer;
	
} MusicPlayer_Config_t;


//----------------------- MusicPlayer_Run --------------------------
// Runs the Player.  This is the main entry point.  Blocks and 
//   doesn't return.  Should be called by the main() function upon startup.
// Inputs:  pConfig - a pointer to the configuration that the 
//   MusicPlayer should use.
// Outputs:  none
void MusicPlayer_Run(MusicPlayer_Config_t* pConfig);


#endif

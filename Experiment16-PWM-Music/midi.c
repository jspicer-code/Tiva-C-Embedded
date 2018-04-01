// File:  midi.c
// Author: jspicer-ltu
// Date:  3/31/18
// Purpose: Contains MIDI data types.
// Hardware:  TM4C123 Tiva board
#include <stdint.h>

// These are musical notes converted to PWM periods at the chosen bus frequency of 80MHz.
// The calculation is:  (bus freq. / pwm clock divisor) / (note frequency)
// Example:  Middle-C (C4)
//    
//		(80000000Hz / 64) / 261.63Hz = 4778
//   
// See https://pages.mtu.edu/~suits/notefreqs.html for note frequencies.
//
const uint16_t Midi_NotePwmPeriods[] = {
		0,	//	
		0,	//	
		0,	//	
		0,	//	
		0,	//	
		0,	//	
		0,	//	
		0,	//	
		0,	//	
		0,	//	
		0,	//	
		0,	//	
		0,	//	C0
		0,	//	CS0 
		0,	//	D0
		0,	//	DS0 
		0,	//	E0
		0,	//	F0
		0,	//	FS0 
		0,	//	G0
		0,	//	GS0 
		0,	//	A0
		0,	//	AS0 
		0,	//	B0
		0,	//	C1
		0,	//	CS1 
		0,	//	D1
		32142,	//	DS1 
		30340,	//	E1
		28637,	//	F1
		27027,	//	FS1 
		25510,	//	G1
		24080,	//	GS1 
		22727,	//	A1
		21452,	//	AS1 
		20246,	//	B1
		19110,	//	C2
		18038,	//	CS2 
		17025,	//	D2
		16071,	//	DS2 
		15168,	//	E2
		14317,	//	F2
		13514,	//	FS2 
		12755,	//	G2
		12039,	//	GS2 
		11364,	//	A2
		10726,	//	AS2 
		10124,	//	B2
		9556,	//	C3
		9019,	//	CS3 
		8513,	//	D3
		8035,	//	DS3 
		7584,	//	E3
		7159,	//	F3
		6757,	//	FS3 
		6378,	//	G3
		6020,	//	GS3 
		5682,	//	A3
		5363,	//	AS3 
		5062,	//	B3
		4778,	//	C4
		4510,	//	CS4 
		4257,	//	D4
		4018,	//	DS4 
		3792,	//	E4
		3579,	//	F4
		3378,	//	FS4 
		3189,	//	G4
		3010,	//	GS4 
		2841,	//	A4
		2681,	//	AS4 
		2531,	//	B4
		2389,	//	C5
		2255,	//	CS5 
		2128,	//	D5
		2009,	//	DS5 
		1896,	//	E5
		1790,	//	F5
		1689,	//	FS5 
		1594,	//	G5
		1505,	//	GS5 
		1420,	//	A5
		1341,	//	AS5 
		1265,	//	B5
		1194,	//	C6
		1127,	//	CS6 
		1064,	//	D6
		1004,	//	DS6 
		948,  //	E6
		895,	//	F6
		845,	//	FS6
		797,	//	G6
		752,	//	GS6 
		710,	//	A6
		670,	//	AS6 
		633,	//	B6
		597,	//	C7
		564,	//	CS7
		532,	//	D7
		502,	//	DS7
		474,	//	E7
		447,	//	F7
		422,	//	FS7
		399,	//	G7
		376,	//	GS7
		355,	//	A7
		335,	//	AS7
		316,	//	B7
		299,	//	C8
		282,	//	CS8
		266,	//	D8
		251,	//	DS8
		237,	//	E8
		224,	//	F8
		211,	//	FS8
		199,	//	G8
		188,	//	GS8
		178,	//	A8
		168,	//	AS8
		158		//	B8
};

// File:  midi.h
// Author: jspicer-ltu
// Date:  3/31/18
// Purpose: Contains MIDI data types.
// Hardware:  TM4C123 Tiva board
#include <stdint.h>

typedef enum {
	MIDI_C0		=	12,
	MIDI_CS0	=	13,
	MIDI_D0		=	14,
	MIDI_DS0	=	15,
	MIDI_E0		=	16,
	MIDI_F0		=	17,
	MIDI_FS0	=	18,
	MIDI_G0		=	19,
	MIDI_GS0	=	20,
	MIDI_A0		=	21,
	MIDI_AS0	=	22,
	MIDI_B0		=	23,
	MIDI_C1		=	24,
	MIDI_CS1	=	25,
	MIDI_D1		=	26,
	MIDI_DS1	=	27,
	MIDI_E1		=	28,
	MIDI_F1		=	29,
	MIDI_FS1	=	30,
	MIDI_G1		=	31,
	MIDI_GS1	=	32,
	MIDI_A1		=	33,
	MIDI_AS1	=	34,
	MIDI_B1		=	35,
	MIDI_C2		=	36,
	MIDI_CS2	=	37,
	MIDI_D2		=	38,
	MIDI_DS2	=	39,
	MIDI_E2		=	40,
	MIDI_F2		=	41,
	MIDI_FS2	=	42,
	MIDI_G2		=	43,
	MIDI_GS2	=	44,
	MIDI_A2		=	45,
	MIDI_AS2	=	46,
	MIDI_B2		=	47,
	MIDI_C3		=	48,
	MIDI_CS3	=	49,
	MIDI_D3		=	50,
	MIDI_DS3	=	51,
	MIDI_E3		=	52,
	MIDI_F3		=	53,
	MIDI_FS3	=	54,
	MIDI_G3		=	55,
	MIDI_GS3	=	56,
	MIDI_A3		=	57,
	MIDI_AS3	=	58,
	MIDI_B3		=	59,
	MIDI_C4		=	60,
	MIDI_CS4	=	61,
	MIDI_D4		=	62,
	MIDI_DS4	=	63,
	MIDI_E4		=	64,
	MIDI_F4		=	65,
	MIDI_FS4	=	66,
	MIDI_G4		=	67,
	MIDI_GS4	=	68,
	MIDI_A4		=	69,
	MIDI_AS4	=	70,
	MIDI_B4		=	71,
	MIDI_C5		=	72,
	MIDI_CS5	=	73,
	MIDI_D5		=	74,
	MIDI_DS5	=	75,
	MIDI_E5		=	76,
	MIDI_F5		=	77,
	MIDI_FS5	=	78,
	MIDI_G5		=	79,
	MIDI_GS5	=	80,
	MIDI_A5		=	81,
	MIDI_AS5	=	82,
	MIDI_B5		=	83,
	MIDI_C6		=	84,
	MIDI_CS6	=	85,
	MIDI_D6		=	86,
	MIDI_DS6	=	87,
	MIDI_E6		=	88,
	MIDI_F6		=	89,
	MIDI_FS6	=	90,
	MIDI_G6		=	91,
	MIDI_GS6	=	92,
	MIDI_A6		=	93,
	MIDI_AS6	=	94,
	MIDI_B6		=	95,
	MIDI_C7		=	96,
	MIDI_CS7	=	97,
	MIDI_D7		=	98,
	MIDI_DS7	=	99,
	MIDI_E7		=	100,
	MIDI_F7		=	101,
	MIDI_FS7	=	102,
	MIDI_G7		=	103,
	MIDI_GS7	=	104,
	MIDI_A7		=	105,
	MIDI_AS7	=	106,
	MIDI_B7		=	107,
	MIDI_C8		=	108,
	MIDI_CS8	=	109,
	MIDI_D8		=	110,
	MIDI_DS8	=	111,
	MIDI_E8		=	112,
	MIDI_F8		=	113,
	MIDI_FS8	=	114,
	MIDI_G8		=	115,
	MIDI_GS8	=	116,
	MIDI_A8		=	117,
	MIDI_AS8	=	118,
	MIDI_B8		=	119

} MidiNotes_t;

typedef enum MidiEventStatus
{
	NOTE_ON = 0x90,
	NOTE_OFF = 0x80
} EventStatus_t;

typedef struct MidiNoteEvent
{
	int16_t deltaTime;
	uint8_t	status;
	uint8_t key;
	uint8_t velocity;
} MidiNoteEvent_t;

typedef struct  
{
	const MidiNoteEvent_t* notes;
} MidiTrack_t;


typedef struct 
{
	int	numTracks;
	const MidiTrack_t* tracks;
} MidiFile_t;


extern const uint16_t Midi_NotePwmPeriods[]; 

		

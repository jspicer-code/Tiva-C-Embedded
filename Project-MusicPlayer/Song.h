// File:  Song.h
// Author: jspicer
// Date:  3/31/18
// Purpose: Contains Song data types.
// Hardware:  TM4C123 Tiva board
#ifndef SONG_H
#define SONG_H

#include <stdint.h>
#include "midi.h"

#define SONG_MAX_TRACKS				4
#define SONG_MAX_FILE_SIZE		65535
#define SONG_MAX_ALBUM_SIZE		0x38000	// 224 Kib
#define SONG_FILE_SIGNATURE		0xABBA
#define SONG_ALBUM_SIGNATURE	0xACDC

// These are the minimum and maximum notes that the player can produce.
// 	The lower note frequencies are out of range for the PWM configuration.
#define SONG_MIN_NOTE			MIDI_DS1
#define SONG_MAX_NOTE			MIDI_B8	

typedef struct {
	uint16_t signature;
	char name[22];
	uint16_t numTracks;
	uint16_t duration;	// in seconds
	uint32_t size;
} SongFileHeader_t;

typedef struct {
	uint16_t id;
	uint16_t offset;
	uint16_t eventCount;
} SongTrackHeader_t;

typedef struct
{
	int16_t deltaTime;
	uint8_t	status;
	uint8_t key;
	uint8_t velocity;
	uint8_t reserved;
} SongNoteEvent_t;

typedef struct
{
	uint16_t signature;
	uint16_t numSongs;
} SongAlbumHeader_t;

typedef struct  
{
	SongNoteEvent_t* notes;
} SongTrack_t;

typedef struct 
{
	SongFileHeader_t* header;
	SongTrack_t tracks[SONG_MAX_TRACKS];
} Song_t;

typedef struct
{
	SongAlbumHeader_t* header;
	uint32_t* songOffsets; // Points to the array of uint32_t that follows the album header.
} SongAlbum_t;

extern const uint16_t Song_NotePwmPeriods[]; 

#endif

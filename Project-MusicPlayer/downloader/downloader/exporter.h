#pragma once
#include <stdint.h>
#include <iostream>
#include <sstream>
#include <memory>

#include "../../Song.h"

#define MAX

#define EXPORT_SUCCESS				(0)
#define EXPORT_FILE_UNREADABLE		(-1);
#define EXPORT_NO_TRACKS			(-2);
#define EXPORT_STREAM_SIZE_EXCEEDED	(-3);


struct ExportEvent
{
	int deltaTime;
	int status;
	int key;
	int velocity;
};

struct ExportTrack
{
	int trackId;
	std::string title;
	std::vector<ExportEvent> events;
};

struct ExportSong
{
	std::string name;
	int size;
	int duration;
	int midiTrackCount;
	std::vector<ExportTrack> tracks;
	std::stringstream stream;
};

struct ExportAlbum
{
	std::vector<ExportSong*> files;
	std::stringstream stream;
};

class Exporter
{

public:	
	static int ExportMidiFile(const char* midiFilePath, ExportSong* song);
private:
	static int GenerateDownloadStream(ExportSong* song);
};


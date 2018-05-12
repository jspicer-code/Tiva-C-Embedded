#include "../include/MidiFile.h"
#include "../include/Options.h"
#include "exporter.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdint.h>
#include <assert.h>

using namespace std;


int Exporter::GenerateDownloadStream(ExportSong* song)
{
	std::vector<SongTrackHeader_t> trackHeaders;

	std::stringstream& stream = song->stream;
	stream.str("");
	stream.clear();
	stream.seekp(0, std::ios::beg);

	SongFileHeader_t fileHeader = { 0 };
	stream.write(reinterpret_cast<const char*>(&fileHeader), sizeof(SongFileHeader_t));

	assert(song->tracks.size() <= SONG_MAX_TRACKS);

	// Don't exceed the maximum number of tracks allowed!
	for (size_t i = 0; i < song->tracks.size(); i++) {

		const ExportTrack& track = song->tracks[i];
		SongTrackHeader_t trackHeader = { track.trackId, 0, track.events.size() };
		stream.write(reinterpret_cast<const char*>(&trackHeader), sizeof(SongTrackHeader_t));
		trackHeaders.push_back(trackHeader);
	}

	for (size_t i = 0; i < trackHeaders.size(); i++) {

		SongTrackHeader_t& trackHeader = trackHeaders[i];
		const ExportTrack& track = song->tracks[i];
	
		trackHeader.offset = stream.tellp();
		for (size_t j = 0; j < track.events.size(); j++) {
			const ExportEvent& event = track.events[j];
			SongNoteEvent_t noteEvent = { 0 };
			noteEvent.deltaTime = (uint16_t)event.deltaTime;
			noteEvent.key = (uint8_t)event.key;
			noteEvent.status = (uint8_t)event.status;
			noteEvent.velocity = (uint8_t)event.velocity;
			stream.write(reinterpret_cast<const char*>(&noteEvent), sizeof(SongNoteEvent_t));
		}

		SongNoteEvent_t noteEvent = { -1, 0, 0, 0, 0 };
		stream.write(reinterpret_cast<const char*>(&noteEvent), sizeof(SongNoteEvent_t));
	}

	song->size = stream.tellp();
	if (song->size >= SONG_MAX_FILE_SIZE) {
		return EXPORT_STREAM_SIZE_EXCEEDED;
	}

	stream.seekp(0, std::ios::beg);

	fileHeader.signature = SONG_FILE_SIGNATURE;
	std::strncpy(fileHeader.name, song->name.c_str(), 20);
	fileHeader.numTracks = trackHeaders.size();
	fileHeader.duration = (uint16_t)song->duration;
	fileHeader.size = song->size;
	stream.write(reinterpret_cast<const char*>(&fileHeader), sizeof(SongFileHeader_t));

	// Write out the track headers again with the offsets.
	for (size_t i = 0; i < trackHeaders.size(); i++) {
		SongTrackHeader_t& trackHeader = trackHeaders[i];
		stream.write(reinterpret_cast<const char*>(&trackHeader), sizeof(SongTrackHeader_t));
	}

	stream.flush();


	return EXPORT_SUCCESS;
}


int Exporter::ExportMidiFile(const char* midiFilePath, ExportSong* song) {

	MidiFile midifile;
	
	song->tracks.clear();
	song->duration = 0;
	song->size = 0;

	if (!midifile.read(midiFilePath)) {
		return EXPORT_FILE_UNREADABLE;
	}

	song->duration = (int)std::round(midifile.getTotalTimeInSeconds());
	
	// Change to delta ticks instead of absolute ticks.
	//midifile.deltaTicks();

	int tracks = midifile.getTrackCount();
	
	double currentTempo = 120.0;
	for (int track = 0; track < tracks; track++) {
	
		std::vector<ExportEvent> events;

		int absPlayerTime = 0;
		int lastKeyOn = 0;
		int percussionTrack = -1;
		bool skip = false;
		for (int event = 0; event < midifile[track].size() && song->tracks.size() < SONG_MAX_TRACKS && !skip; event++) {

			MidiEvent& ev = midifile[track][event];
			
			int channel = ev.getChannel();
			if (channel == 9) {
				if (percussionTrack == -1) {
					percussionTrack = track;
				}
				else if (track != percussionTrack) {
					skip = true;
					continue;
				}
			}
		
			// Calculate the absolute time in milliseconds, then subtract the last delay time to
			//	get the necessary delta in whole milliseconds.
			double absTime = midifile.getTimeInSeconds(ev.tick) * 1000.0;
			int delay = (int)round(absTime - (double)absPlayerTime);
			absPlayerTime += delay;
		
			if (ev.isNote() && ev.getKeyNumber() != 0) {

				// Remove note events that occur simultaneously.  These are notes played at the same time, e.g. chords.
				//	Export only the first note.
				bool push = false;
				if (ev.isNoteOff() && ev.getKeyNumber() == lastKeyOn) {
					lastKeyOn = 0;
					push = true;
				}
				else if (delay != 0 || event == 0 || lastKeyOn == 0) {
					lastKeyOn = ev.getKeyNumber();
					push = true;
				}

				if (push) {
					events.push_back({ delay, ev.getCommandByte(), ev.getKeyNumber(), ev.getVelocity() });
				}
			}
		}

		if (!skip && events.size()) {
			ExportTrack playerTrack; 
			playerTrack.trackId = track;
			playerTrack.events = std::move(events);
			song->tracks.emplace_back(playerTrack);
		}
	}

	if (song->tracks.size() > 0) {
		song->midiTrackCount = tracks;
		return GenerateDownloadStream(song);
	}
	
	return EXPORT_NO_TRACKS;
}
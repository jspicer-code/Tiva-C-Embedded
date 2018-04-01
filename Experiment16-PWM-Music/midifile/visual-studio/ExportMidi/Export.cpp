#include "MidiFile.h"
#include "Options.h"
#include <iostream>
#include <iomanip>

using namespace std;

struct PlayerEvent
{
	int deltaTime;
	int status;
	int key;
	int velocity;
};


struct PlayerTrack
{
	int trackId;
	std::vector<PlayerEvent> events;
};


void generatePlayerFile(const std::string& midiFilePath, const std::vector<PlayerTrack>& tracks)
{
	ofstream outFile;
	outFile.open("midi-tune.c");
	outFile << "// " << midiFilePath << "\n";
	outFile << "#include \"midi.h\"\n";

	int trackCount = 0;
	for (size_t i = 0; i < tracks.size() && trackCount < 4; i++) {

		if (tracks[i].events.size() == 0) {
			continue;
		}

		const std::vector<PlayerEvent>& events = tracks[i].events;

		outFile << "static MidiNoteEvent_t track" << tracks[i].trackId << "Notes_[]={\n";
		for (size_t i = 0; i < events.size(); i++) {
			outFile << "\t{" 
					<< events[i].deltaTime << ", 0x" << hex << events[i].status << dec << ", " 
					<< events[i].key << ", " << events[i].velocity << "},\n";
		}

		outFile << "\t{-1, 0, 0}\n";
		outFile << "};\n";

		trackCount++;
	}

	outFile << "\n";
	outFile << "static MidiTrack_t tracks_[] = {";

	trackCount = 0;
	for (size_t i = 0; i < tracks.size() && trackCount < 4; i++) {

		if (tracks[i].events.size() == 0) {
			continue;
		}
		
		if (trackCount > 0) {
			outFile << ", ";
		}
		outFile << "track" << tracks[i].trackId << "Notes_";
		trackCount++;
	}
	outFile << "};\n";

	outFile << "MidiFile_t midi_tune = {" << trackCount << ", tracks_};\n";
	outFile.close();
}

int main(int argc, char** argv) {

	Options options;
	options.process(argc, argv);

	MidiFile midifile;

	if (options.getArgCount() < 1) {
		cout << "Missing commandline argument:  arg(1) midi filepath.\n";
		return -1;
	}
	else if (!midifile.read(options.getArg(1))) {
		cout << "Failed to open/read midi file.  Verify that the file path is correct.\n";
		return -1;
	}

	std::string midiFilePath = options.getArg(1);

	// Change to delta ticks instead of absolute ticks.
	midifile.deltaTicks();

	int tracks = midifile.getTrackCount();
	cout << "TicksPerQuarterNote: " << midifile.getTicksPerQuarterNote() << endl;
	cout << "TRACKS: " << tracks << endl;

	std::vector<PlayerTrack> playerTracks;

	for (int track = 0; track < tracks; track++) {
	
		std::vector<PlayerEvent> events;

		int lastKeyOn = 0;
		for (int event = 0; event < midifile[track].size(); event++) {

			MidiEvent& ev = midifile[track][event];
		
			// Ignore percussion channels.
			if (ev.getChannel() >= 9 && ev.getChannel() <= 16) {
				continue;
			}

			int delay = (int)(midifile.getTimeInSeconds(ev.tick) * 1000.0);

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

			cout << dec << ev.tick;
			cout << '\t';

			cout << dec << (int)(midifile.getTimeInSeconds(ev.tick) * 1000.0);
			cout << '\t' << hex;

			for (unsigned int i = 0; i < midifile[track][event].size(); i++) {
				cout << (int)midifile[track][event][i] << ' ';
			}
			cout << endl;
		}

		if (events.size()) {
			PlayerTrack playerTrack; 
			playerTrack.trackId = track;
			playerTrack.events = std::move(events);
			playerTracks.emplace_back(playerTrack);
		}
	}

	generatePlayerFile(midiFilePath, playerTracks);

	return 0;
}
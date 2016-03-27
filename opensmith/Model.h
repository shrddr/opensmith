#pragma once
#include <vector>
#include "Audio/Audio.h"
#include "Audio/NoteDetector.h"
#include "Audio/VorbisDecoder.h"
#include "View.h"
#include "Controller.h"
#include "Settings/Settings.h"
#include "PsarcReader/PSARC.h"
#include "PsarcReader/Sng.h"

class Model
{
public:
	Model(View& v, Controller& c);
	~Model();
	void update(float currentTime);
private:
	View& v;
	Controller& c;
	Sng s;

	// OGG related

	std::vector<char> oggStorage;
	VorbisDecoder* decoder;
	VorbisBuffer* buffer;
	NoteDetector* noteDetector;
	InOut* io;
	Audio* audio;
	float audioStartTime;

	// Song parsing

	void preloadSNG();
	std::vector<Sng::BPM>::iterator itBeat;
	std::vector<Sng::PhraseIteration>::iterator itIteration;

	// Model-View interface

	Hud hud;

};


#include "Model.h"
#include <iostream>
#include <algorithm>
#include "Settings/Settings.h"
#include "Wem/Wem.h"
#include "Audio/notes.h"

Model::Model(View& v, Controller& c):
	v(v),
	c(c)
{
	PSARC* psarc = new PSARC(o.psarcFile.c_str());
	std::cout << glfwGetTime() << " > PSARC header parsed" << std::endl;

	int entryAudio = -1;
	uint64_t entryAudioLength = 0;

	for (size_t i = 0; i < psarc->Entries.size(); i++)
	{
		auto e = psarc->Entries[i];
		if (e->name.find(".wem") != std::string::npos)
			if (e->Length > entryAudioLength)
			{
				entryAudioLength = e->Length;  // HACK: pick the largest
				entryAudio = i;
			}
	}

	bool isBass = (psarc->Entries[o.sngEntry]->name.find("bass.sng") != std::string::npos);

	// get sng

	std::vector<char> sngEntryStorage;
	psarc->Entries[o.sngEntry]->Data->readTo(sngEntryStorage);

	if (o.dumpSng)
	{
		std::ofstream outSngFile("../resources/temp.sng", std::ios::binary);
		outSngFile.write(sngEntryStorage.data(), sngEntryStorage.size());
		outSngFile.close();
	}

	std::cout << glfwGetTime() << " > SNG extracted from PSARC" << std::endl;

	std::vector<char> sngStorage;
	SngReader::readTo(sngEntryStorage, sngStorage);
	s.parse(sngStorage);
	convertTuningAbsolute(s.metadata.tuning);

	std::cout << glfwGetTime() << " > SNG parsed" << std::endl;

	std::cout << "Tuning: " << tuningString(s.metadata.tuning) << std::endl;

	preloadSNG();

	// get audio

	std::vector<char> audioEntryStorage;
	psarc->Entries[entryAudio]->Data->readTo(audioEntryStorage);

	if (o.dumpWem)
	{
		std::ofstream outWemFile("../resources/temp.wem", std::ios::binary);
		outWemFile.write(audioEntryStorage.data(), audioEntryStorage.size());
		outWemFile.close();
	}
	
	std::cout << glfwGetTime() << " > WEM extracted from PSARC" << std::endl;

	Wem w(audioEntryStorage);
	w.generateOgg(oggStorage);

	std::cout << glfwGetTime() << " > WEM to OGG" << std::endl;

	decoder = new VorbisDecoder(oggStorage);
	std::cout << glfwGetTime() << " > VorbisDecoder init" << std::endl;

	buffer = new VorbisBuffer(*decoder);

	// init detector

	noteDetector = new NoteDetector(w.getSampleRate(), s.metadata.tuning, isBass);
	std::cout << glfwGetTime() << " > NoteDetector init" << std::endl;

	// start portaudio

	io = new InOut{buffer, noteDetector, o.playbackVolume , o.instrumentVolume };
	audio = new Audio(*io, w.getSampleRate());
	std::cout << glfwGetTime() << " > Audio init" << std::endl;

	double paTime = audio->start();
	io->startTime = paTime;
	audioStartTime = glfwGetTime();

	std::cout << audioStartTime << " > Audio start" <<  std::endl;

	delete psarc;
}

Model::~Model()
{
	delete audio;
	delete io;
	delete noteDetector;
	delete buffer;
	delete decoder;
}

void Model::preloadSNG()
{
	itBeat = s.BPMs.begin();
	itIteration = s.PhraseIterations.begin();

	for (auto iteration : s.PhraseIterations)
	{
		int thisDifficulty = std::min(o.difficulty, iteration.difficulty[2]);
		float barHeight = thisDifficulty / (float)s.metadata.maxDifficulty;
		bool isMax = (thisDifficulty == iteration.difficulty[2]);
		Hud::Iteration newIteration = { iteration.startTime, 0, barHeight, false, false, isMax };
		if (!hud.iterations.empty())
			hud.iterations.back().endTime = iteration.startTime;
		hud.iterations.push_back(newIteration);
	}
	hud.iterations.back().endTime = s.metadata.songLength;
	hud.initTimeline(s.metadata.songLength, 200, 980, 80);
	hud.initNotes();
}

void Model::update(float glfwTime)
{
	float currentTime = glfwTime - audioStartTime;
	float maxTime = currentTime + o.visualsPreloadTime;

	// add what's coming
	while (itBeat != s.BPMs.end() && itBeat->time < maxTime)
	{
		Visuals::Beat newBeat = { itBeat->time };
		visuals.beats.push_back(newBeat);
		++itBeat;
	}

	while (itIteration != s.PhraseIterations.end() && itIteration->startTime < maxTime)
	{
		int difficulty = std::min(o.difficulty, itIteration->difficulty[2]);
		Sng::Arrangement a = s.Arrangements[difficulty];

		for (auto anchor : a.Anchors)
		{
			if (anchor.endBeatTime < itIteration->startTime) continue;
			if (anchor.startBeatTime > itIteration->nextPhraseTime) break;
			// find out fret later
			Visuals::Anchor o = { -1, anchor.width, anchor.startBeatTime, anchor.endBeatTime };

			// cut anchors to iteration length
			if (anchor.startBeatTime < itIteration->startTime)
				o.startTime = itIteration->startTime;
			if (anchor.endBeatTime > itIteration->nextPhraseTime)
				o.endTime = itIteration->nextPhraseTime;

			for (auto existingAnchor : visuals.anchors)
			{
				if (o.startTime < existingAnchor.endTime)
				{
					std::cout << "anchor overlay: existing "
						<< existingAnchor.startTime << " " << existingAnchor.endTime
						<< " new "
						<< o.startTime << " " << o.endTime << std::endl;
				}
			}
			
			visuals.anchors.push_back(o);
		}

		for (auto n : a.Notes)
		{
			if (n.time < itIteration->startTime) continue;
			if (n.time > itIteration->nextPhraseTime) break;
			// TODO: should use this instead: if (n.phraseIterationId)

			// simple note
			if (n.stringIndex != -1)
			{
				Visuals::Note newNote = { n.fretId, n.stringIndex, n.time, n.anchorFretId };
				visuals.notes.push_back(newNote);

				if (n.sustain > 0)
				{
					Visuals::Sustain newSustain = { n.fretId, 0, n.stringIndex, n.time, n.sustain, n.anchorFretId };
					if (n.slideTo != -1 || n.slideUnpitchTo != -1)
						newSustain.deltaFret = n.slideTo - n.fretId;
					visuals.sustains.push_back(newSustain);
				}
			}

			// chord reference
			if (n.stringIndex < 0 && n.chordId >= 0)
			{
				for (int i = 0; i < 6; i++)
				{
					if (s.Chords[n.chordId].frets[i] >= 0)
					{
						Visuals::Note newNote = { s.Chords[n.chordId].frets[i], i, n.time, n.anchorFretId };
						visuals.notes.push_back(newNote);
						if (n.sustain > 0)
						{
							Visuals::Sustain newSustain = { s.Chords[n.chordId].frets[i], 0, i, n.time, n.sustain, n.anchorFretId };
							visuals.sustains.push_back(newSustain);
						}
					}
				}
			}

			// set anchor fret
			// TODO: excessive looping here; maybe save previous position to speed up?
			for (auto& anchor : visuals.anchors)
			{
				if (anchor.fret != -1) continue;
				if (anchor.endTime < n.time) continue;
				if (anchor.startTime > n.time) break;
				anchor.fret = n.anchorFretId;
			}
		}
	
		itIteration++;
	}

	noteDetector->analyze(hud.detected);

	for (auto note = visuals.notes.begin(); note != visuals.notes.end() && note->time - o.detectionTimeWindow / 2 < currentTime; /* nop */)
		if (note->hit)
			note = visuals.notes.erase(note);
		else
		{
			note->hit = noteDetector->confirm(note->string, note->fret, note->time);
			if (note->hit)
			{
				Visuals::Ghost newGhost = { note->fret, note->string, note->time, true };
				visuals.ghosts.push_back(newGhost);
			}
			++note;
		}

	v.show(visuals, hud, currentTime);

	// clean what we already passed

	while (!visuals.beats.empty() && visuals.beats.front().time < currentTime)
		visuals.beats.pop_front();

	while (!visuals.notes.empty() && visuals.notes.front().time + o.detectionTimeWindow / 2 < currentTime)
	{
		Visuals::Ghost newGhost = { visuals.notes.front().fret, visuals.notes.front().string, visuals.notes.front().time + o.detectionTimeWindow / 2, false };
		visuals.ghosts.push_back(newGhost);
		visuals.notes.pop_front();
	}
		
	while (!visuals.sustains.empty() && visuals.sustains.front().time + visuals.sustains.front().length < currentTime)
		visuals.sustains.pop_front();
		
	while (!visuals.anchors.empty() && visuals.anchors.front().endTime < currentTime)
		visuals.anchors.pop_front();

	while (!visuals.ghosts.empty() && visuals.ghosts.front().time + o.ghostStayTime < currentTime)
		visuals.ghosts.pop_front();
}





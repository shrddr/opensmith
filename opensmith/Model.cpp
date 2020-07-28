#include "Model.h"
#include <iostream>
#include <algorithm>
#include "Settings/Settings.h"
#include "Wem/Wem.h"
#include "Audio/notes.h"
#include "Vocals/Vocals.h"

Model::Model(View& v, Controller& c):
	v(v),
	c(c)
{
	std::cout << glfwGetTime() << " > parsing PSARC header" << std::endl;
	PSARC* psarc = new PSARC(o.psarcFile.c_str());
	psarc->DetectEntries();

	std::cout << glfwGetTime() << " > extracting SNG from PSARC" << std::endl;

	std::vector<char> sngEntryStorage;
	psarc->Entries[o.sngEntry]->Data->readTo(sngEntryStorage);

	if (o.dumpSng)
	{
		std::ofstream outSngFile("../resources/temp.sng", std::ios::binary);
		outSngFile.write(sngEntryStorage.data(), sngEntryStorage.size());
		outSngFile.close();
	}

	

	std::cout << glfwGetTime() << " > parsing SNG\n";
	std::vector<char> sngStorage;
	SngReader::readTo(sngEntryStorage, sngStorage);
	s.parse(sngStorage);
	convertTuningAbsolute(s.metadata.tuning);

	std::cout << "Tuning: " << notesNames(s.metadata.tuning) << "\n";

	preloadSNG();

	// get audio

	std::vector<char> audioEntryStorage;
	psarc->entry_audio->Data->readTo(audioEntryStorage);

	if (o.dumpWem)
	{
		std::ofstream outWemFile("../resources/temp.wem", std::ios::binary);
		outWemFile.write(audioEntryStorage.data(), audioEntryStorage.size());
		outWemFile.close();
	}
	
	std::cout << glfwGetTime() << " > extracting WEM from PSARC\n";
	Wem w(audioEntryStorage);
	std::cout << glfwGetTime() << " > WEM to OG\n";
	w.generateOgg(oggStorage);

	std::cout << glfwGetTime() << " > VorbisDecoder init\n";
	decoder = new VorbisDecoder(oggStorage);

	buffer = new VorbisBuffer(*decoder);

	// get vocals

	/*std::vector<char> vocalsEntryStorage;
	if (psarc->entry_vocals != NULL)
	{
		psarc->entry_vocals->Data->readTo(vocalsEntryStorage);
		Vocals v(vocalsEntryStorage);
	}*/

	// init detector

	std::cout << glfwGetTime() << " > NoteDetector init\n";
	noteDetector = new NoteDetector(w.getSampleRate(), s.metadata.tuning, o.role == bass);

	// start portaudio

	io = new InOut{ buffer, noteDetector, o.playbackVolume, o.instrumentVolume };
	std::cout << glfwGetTime() << " > Audio init (" << w.getSampleRate() << ")\n";
	audio = new Audio(*io, w.getSampleRate());
	
	double paTime = audio->start();
	io->startTime = paTime;
	audioStartTime = glfwGetTime();

	std::cout << audioStartTime << " > Audio start\n";

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
		Beat newBeat = { itBeat->time };
		v.beats.push_back(newBeat);
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
			Anchor a = { -1, anchor.width, anchor.startBeatTime, anchor.endBeatTime };

			// cut anchors to iteration length
			if (anchor.startBeatTime < itIteration->startTime)
				a.startTime = itIteration->startTime;
			if (anchor.endBeatTime > itIteration->nextPhraseTime)
				a.endTime = itIteration->nextPhraseTime;
			
			v.anchors.push_back(a);
		}

		for (auto n : a.Notes)
		{
			if (n.time < itIteration->startTime) continue;
			if (n.time > itIteration->nextPhraseTime) break;
			// TODO: should use this instead: if (n.phraseIterationId)

			// simple note
			if (n.stringIndex != -1)
			{
				Note newNote = { n.fretId, n.stringIndex, n.time, n.anchorFretId, n.noteMask };
				v.notes.push_back(newNote);

				if (n.sustain > 0)
				{
					Sustain newSustain = { n.fretId, 0, n.stringIndex, n.time, n.sustain, n.anchorFretId };
					if (n.slideTo != -1 || n.slideUnpitchTo != -1)
						newSustain.deltaFret = n.slideTo - n.fretId; // TODO: Take care of Unpitch
					v.sustains.push_back(newSustain);
				}
			}

			// chord reference
			// TODO: push a chord object to draw the blue glow
			if (n.stringIndex < 0 && n.chordId >= 0)
			{
				for (char i = 0; i < 6; i++)
				{
					if (s.Chords[n.chordId].frets[i] >= 0) //frets,fingers,notes,name
					{
						Note newNote = { s.Chords[n.chordId].frets[i], i, n.time, n.anchorFretId, n.noteMask };

						// noteMasks,bendDatas,slides,vibratos are stored separately because they can differ between chord notes
						//if (n.chordNotesId >= 0 && s.ChordNotes[n.chordNotesId].noteMask[i] >= 0)
						//	newNote.mask = s.ChordNotes[n.chordNotesId].noteMask[i];

						v.notes.push_back(newNote);
						if (n.sustain > 0)
						{
							Sustain newSustain = { s.Chords[n.chordId].frets[i], 0, i, n.time, n.sustain, n.anchorFretId };

							if (n.chordNotesId >= 0)
								if (s.ChordNotes[n.chordNotesId].slideTo[i] != -1 || s.ChordNotes[n.chordNotesId].slideUnpitchTo[i] != -1)
									newSustain.deltaFret = s.ChordNotes[n.chordNotesId].slideTo[i] - s.Chords[n.chordId].frets[i];

							v.sustains.push_back(newSustain);
						}
					}
				}
			}

			// set anchor fret
			// TODO: excessive looping here; maybe save previous position to speed up?
			for (auto& anchor : v.anchors)
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

	for (auto note = v.notes.begin(); note != v.notes.end() && note->time - o.detectionTimeWindow / 2 < currentTime; /* nop */)
		if (note->hit)
			note = v.notes.erase(note);
		else
		{
			note->hit = noteDetector->confirm(note->string, note->fret, note->time);
			if (note->hit)
			{
				Ghost newGhost = { note->fret, note->string, note->time, true };
				v.ghosts.push_back(newGhost); // hit-ghost
			}
			++note;
		}

	v.show(hud, currentTime);

	// clean what we already passed

	while (!v.beats.empty() && v.beats.front().time < currentTime)
		v.beats.pop_front();

	while (!v.notes.empty() && v.notes.front().time + o.detectionTimeWindow / 2 < currentTime)
	{
		Ghost newGhost = { v.notes.front().fret, v.notes.front().string, v.notes.front().time + o.detectionTimeWindow / 2, false };
		v.ghosts.push_back(newGhost); // miss-ghost
		v.notes.pop_front();
	}
		
	while (!v.sustains.empty() && v.sustains.front().time + v.sustains.front().length < currentTime)
		v.sustains.pop_front();
		
	while (!v.anchors.empty() && v.anchors.front().endTime < currentTime)
		v.anchors.pop_front();

	while (!v.ghosts.empty() && v.ghosts.front().time + o.ghostStayTime < currentTime)
		v.ghosts.pop_front();
}





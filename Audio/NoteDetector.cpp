#include "NoteDetector.h"
#include <cmath>
#include <iostream>
#include "Settings/Settings.h"

NoteDetector::NoteDetector(size_t sampleRate, std::vector<int>& sngTuning, bool isBass):
	sampleRate(sampleRate),
	samples(inputSize)
{
	for (size_t i = 0; i < 6; i++)
		tuning[i] = sngTuning[i] - (isBass ? 12 : 0);
	noteFirst = tuning[0];

	for (size_t note = 0; note < noteCount; note++)
	{
		int midiNote = noteFirst + note;
		noteFreq[note] = FREQ_A4 * pow(NOTE_DISTANCE, midiNote - MIDI_A4);
		for (size_t sample = 0; sample < inputSize; sample++)
		{
			noteSin[note][sample] = sin(2 * PI * noteFreq[note] / sampleRate * sample) / sqrt(inputSize);
			noteCos[note][sample] = cos(2 * PI * noteFreq[note] / sampleRate * sample) / sqrt(inputSize);
		}
	}
	//log.open("detector.log");
}

void NoteDetector::setPCM(const float in)
{
	samples.push_back(in);
}

float NoteDetector::rms()
{
	float res = 0.0f;
	for (size_t sample = 0; sample < inputSize; sample++)
		res += samples[sample] * samples[sample];
	res = sqrt(res / inputSize);
	res = 20 * log10(res);
	return res;
}

void NoteDetector::analyze(float(&results)[144])
{
	for (size_t note = 0; note < noteCount; note++)
	{
		float amplitudeSin = 0;
		float amplitudeCos = 0;
		for (size_t sample = 0; sample < inputSize; sample++)
		{
			amplitudeSin += samples[sample] * noteSin[note][sample];
			amplitudeCos += samples[sample] * noteCos[note][sample];
		}
		float power = sqrt(amplitudeSin*amplitudeSin + amplitudeCos*amplitudeCos);
		float delta = power - powers[note];
		bool up = (delta > deltas[note]);
		deltas[note] = delta;
		powers[note] = power;
	}

	for (size_t i = 0; i < 24; i++)
	{
		results[i]		= powers[tuning[0] - noteFirst + i];
		results[24 + i] = powers[tuning[1] - noteFirst + i];
		results[48 + i] = powers[tuning[2] - noteFirst + i];
		results[72 + i] = powers[tuning[3] - noteFirst + i];
		results[96 + i] = powers[tuning[4] - noteFirst + i];
		results[120 + i]= powers[tuning[5] - noteFirst + i];
	}
}

bool NoteDetector::confirm(int string, int fret, float time)
{
	int note = tuning[string] - noteFirst + fret;
	bool yes = (deltas[note] > o.detectionMinPower) && (deltas[note] > deltas[note - 1]) && (deltas[note] > deltas[note + 1]);

	/*log << time << "\t" << string << "\t" << fret << "\t" << deltas[note - 1] << "\t" << deltas[note] << "\t" << deltas[note + 1] << std::endl;
	if (yes)
		log << "!" << std::endl;*/

	return yes;
}

NoteDetector::~NoteDetector()
{
}

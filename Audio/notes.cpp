#include "notes.h"
#include <iostream>
#include <cmath>

void convertTuningAbsolute(std::vector<int>& tuning)
{
	tuning[0] += 40;
	tuning[1] += 45;
	tuning[2] += 50;
	tuning[3] += 55;
	tuning[4] += 59;
	tuning[5] += 64;
}

std::string noteName(int note)
{
	return noteNames[note % 12];
}

std::string notesNames(std::vector<int>& tuning)
{
	std::string s;
	for (auto note : tuning)
		s += noteName(note);
	return s;
}

float noteFreq(float base, int note)
{
	return base * powf(2, (note - 69) / 12.0f);
}

float freqShift(float freq, float cents)
{
	return freq * powf(2, (cents / 1200));
}

float centDifference(float freq1, float freq2)
{
	return 1200 * log2(freq1 / freq2);
}

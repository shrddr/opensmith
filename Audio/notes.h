#pragma once
#include <string>
#include <vector>

const std::string noteNames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

void convertTuningAbsolute(std::vector<int>& tuning);
std::string noteName(int note);
std::string notesNames(std::vector<int>& tuning);

float noteFreq(float base, int note);
float freqShift(float freq, float cents);
float centDifference(float freq1, float freq2);
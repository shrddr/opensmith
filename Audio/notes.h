#pragma once
#include <string>
#include <vector>

const std::string noteNames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

void convertTuningAbsolute(std::vector<int>& tuning);
std::string tuningString(std::vector<int>& tuning);
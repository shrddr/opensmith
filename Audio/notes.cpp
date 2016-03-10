#include "notes.h"
#include <iostream>

void convertTuningAbsolute(std::vector<int>& tuning)
{
	tuning[0] += 40;
	tuning[1] += 45;
	tuning[2] += 50;
	tuning[3] += 55;
	tuning[4] += 59;
	tuning[5] += 64;
}

std::string tuningString(std::vector<int>& tuning)
{
	std::string s;
	for (auto string : tuning)
		s += noteNames[string % 12];
	return s;
}

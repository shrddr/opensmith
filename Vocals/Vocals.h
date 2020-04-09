#pragma once
#include <vector>

struct VocalsEntry
{
	float time;
	float tlen;
	const char* text;
};

class Vocals
{
public:
	Vocals(std::vector<char> vocalsEntryStorage);
	size_t Count();
private:
	std::vector<VocalsEntry> records;
};

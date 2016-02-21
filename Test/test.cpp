#include <vector>
#include <iostream>
#include "Wem/Wem.h"
#include "Audio/VorbisDecoder.h"

int main()
{
	std::vector<char> oggStorage;
	Wem w("temp.wem");
	w.generateOgg(oggStorage);
	VorbisDecoder decoder(oggStorage);

	float** data;
	int64_t pos;
	int size;

	while (true)
	{
		bool success = decoder.getPCM(&data, pos, size);
		if (!success)
			break;
		decoder.gotPCM(size);
	}

	std::cout << "end";
	std::cin.get();
}
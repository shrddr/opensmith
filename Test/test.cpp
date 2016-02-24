#include <vector>
#include <iostream>
#include "Wem/Wem.h"
#include "PsarcReader/PSARC.h"
#include "PsarcReader/Sng.h"
#include "Audio/VorbisDecoder.h"

// test if end of file is handled correctly
bool testDecoderEof(const char* fileName)
{
	std::vector<char> oggStorage;
	Wem w(fileName);
	w.generateOgg(oggStorage);
	VorbisDecoder decoder(oggStorage);

	float** data;
	int64_t pos;
	int size;
	int emptyReads = 0;

	while (true)
	{
		bool success = decoder.getPCM(&data, pos, size);
		if (!success)
			break;
		decoder.gotPCM(size);

		if (size < 1)
			emptyReads++;
		if (emptyReads > 1000) // stuck in a loop
			return false;
	}

	return true;
}

// test correct psarc header parsing
bool testPsarc(const char* fileName, const size_t entries)
{
	try
	{
		PSARC psarc(fileName);
		return (entries == psarc.Entries.size());
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
		return false;
	}
}

// test correct sng parsing
bool testSng(const char* fileName)
{
	try
	{
		std::ifstream inSngFile;
		inSngFile.open(fileName, std::ios::binary);
		std::vector<char> sngStorage;
		SngReader::readTo(inSngFile, sngStorage);
		Sng s;
		s.parse(sngStorage);
		return (s.Arrangements.size() > 0);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
		return false;
	}
}

int main()
{
	//std::cout << testDecoderEof("../resources/temp.wem") << std::endl;
	std::cout << testPsarc("../../../rhcp_p.psarc", 24) << std::endl;
	std::cout << testPsarc("../../../rhcp_m.psarc", 24) << std::endl;
	std::cout << testSng("../resources/temp_p.sng") << std::endl;
	std::cout << testSng("../resources/temp_m.sng") << std::endl;
	std::cin.get();
}
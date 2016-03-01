#include <vector>
#include <iostream>
#include <chrono>
#include "Wem/Wem.h"
#include "PsarcReader/PSARC.h"
#include "PsarcReader/Sng.h"
#include "Audio/VorbisDecoder.h"

// test if end of file is handled correctly
bool testDecoderEof(const char* fileName)
{
	Wem w(fileName);
	std::vector<char> oggStorage;
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
		std::ifstream file;
		file.open(fileName, std::ios::binary);
		file.seekg(0, std::ios_base::end);
		std::streampos fileSize = file.tellg();

		std::vector<char> vec;
		vec.resize(fileSize);
		file.seekg(0, std::ios_base::beg);
		file.read(&vec[0], fileSize);

		std::vector<char> sngStorage;
		SngReader::readTo(vec, sngStorage);
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

bool testPsarcWemStream(const char* fileName, const int entry)
{
	try
	{
		PSARC psarc(fileName);
		size_t i = -1;
		while (i < psarc.Entries.size())
		{
			if (psarc.Entries[i++]->name.find(".wem") != std::string::npos)
				break;
		}
		std::vector<char> audioEntryStorage;
		psarc.Entries[i]->Data->readTo(audioEntryStorage);

		return true;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
		return false;
	}
}

void testWemPerformance(const char* fileName)
{
	for (size_t i = 0; i < 100; i++)
	{
		Wem w(fileName);
		std::vector<char> oggStorage;
		w.generateOgg(oggStorage);
	}
}

void testWemMPerformance(const char* fileName)
{
	std::ifstream file;
	file.open(fileName, std::ios::binary);
	file.seekg(0, std::ios_base::end);
	std::streampos fileSize = file.tellg();

	std::vector<char> vec;
	vec.resize(fileSize);
	file.seekg(0, std::ios_base::beg);
	file.read(&vec[0], fileSize);

	for (size_t i = 0; i < 100; i++)
	{
		try
		{
			std::vector<char> oggStorage;
			Wem w(vec);
			w.generateOgg(oggStorage);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what();
		}
	}
}

int main()
{
	//std::cout << testDecoderEof("../resources/temp.wem") << std::endl;

	//std::cout << testPsarc("../../../rhcp_p.psarc", 24) << std::endl;
	//std::cout << testPsarc("../../../rhcp_m.psarc", 24) << std::endl;
	//std::cout << testSng("../resources/temp_p.sng") << std::endl;
	//std::cout << testSng("../resources/temp.sng") << std::endl;

	//std::cout << testPsarcWemStream("../../../rhcp_p.psarc", 0) << std::endl;

	auto t1 = std::chrono::high_resolution_clock::now();
	testWemPerformance("../resources/temp.wem");
	auto t2 = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count() << std::endl;
	testWemMPerformance("../resources/temp.wem");
	auto t3 = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t2).count() << std::endl;

	std::cin.get();
}
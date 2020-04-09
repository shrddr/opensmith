#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include "StreamReader.h"

class PSARC
{
public:
	/* Open file, parse TOC and fill Entries vector */
	PSARC(char const* fileName);
	~PSARC();

	class Entry
	{
	public:
		Entry() {}
		~Entry()
		{
			delete Data;
		}
		int id;
		char MD5[16];
		uint32_t zIndex;
		uint64_t Length;
		uint64_t Offset;
		std::string name;

		class DataPointer
		{
		public:
			DataPointer(Entry& E, StreamReaderBE& R, std::vector<uint32_t>& z, uint32_t b) :
				_Entry(E),
				Reader(R),
				zLengths(z),
				blockSize(b) {}
			~DataPointer() {}
			/* Unzips file entry into memory */
			void readTo(std::vector<char>& storage);
		private:
			Entry& _Entry;
			StreamReaderBE& Reader;
			std::vector<uint32_t>& zLengths;
			uint32_t blockSize;
		};

		DataPointer* Data;
	};

	std::ifstream stream;
	StreamReaderBE Reader;
	std::vector<Entry*> Entries;
	std::vector<uint32_t> zLengths;

	void DetectEntries();

	Entry* entry_audio;
	Entry* entry_vocals;

private:
	const int headerSize = 32;

	static void blockXor(char* out, char const* buf, int blockSize = 16);
	static void decryptTOC(char* output, long len);
	static int inflateStream(std::istream& source, int srcLen, std::vector<char>& dest);
};


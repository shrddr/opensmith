#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include "BigEndianReader.h"


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
			DataPointer(Entry& E, BigEndianReader& R, std::vector<uint32_t>& z, uint32_t b) :
				Entry(E),
				Reader(R),
				zLengths(z),
				blockSize(b) {}
			~DataPointer() {}
			/* Unzips file entry into memory */
			void readTo(std::vector<char>& storage);
		private:
			Entry& Entry;
			BigEndianReader& Reader;
			std::vector<uint32_t>& zLengths;
			uint32_t blockSize;
		};

		DataPointer* Data;
	};

	std::ifstream stream;
	BigEndianReader Reader;
	std::vector<Entry*> Entries;
	std::vector<uint32_t> zLengths;

private:
	const int headerSize = 32;

	static void blockXor(char* out, char const* buf, int blockSize = 16);
	static void decryptTOC(char* output, long len);
	static int inflateStream(std::istream& source, int srcLen, std::vector<char>& dest);
};


#include <cassert>
#include <cstring>
#include <fstream>
#include <algorithm>
#include "Rijndael/rijndael.h"
#include "zlib/zlib.h"
#include "PSARC.h"
#include "Keys.h"

struct membuf : std::streambuf
{
	membuf(char* begin, char* end) 
	{
		this->setg(begin, begin, end);
	}
};

PSARC::PSARC(char const* fileName):
	Reader(stream)
{
	stream.open(fileName, std::ifstream::in | std::ifstream::binary);
	if (!stream.good())
		throw std::runtime_error("PSARC file not found.\n");

	uint32_t MagicNumber = Reader.readUint32();
	uint32_t VersionNumber = Reader.readUint32();
	uint32_t CompressionMethod = Reader.readUint32();
	uint32_t TotalTOCSize = Reader.readUint32();
	uint32_t TOCEntrySize = Reader.readUint32();
	uint32_t numFiles = Reader.readUint32();
	uint32_t blockSize = Reader.readUint32();
	uint32_t archiveFlags = Reader.readUint32();

	if (MagicNumber != 1347633490) throw "no PSARC header";
	if (CompressionMethod != 2053925218) throw "no zlib header";

	int cipherSize = TotalTOCSize - headerSize;

	char* bufferTOC = new char[cipherSize];
	membuf membufTOC(bufferTOC, bufferTOC + cipherSize);
	std::istream streamTOC(&membufTOC);

	stream.read(bufferTOC, cipherSize);
	if (archiveFlags == 4)
	{
		decryptTOC(bufferTOC, cipherSize);
	}
		
	BigEndianReader ReaderTOC(streamTOC);

	uint32_t numFile = 0;
	while (numFile < numFiles)
	{
		Entry* e = new Entry();
		e->id = numFile;
		ReaderTOC.readBytes(e->MD5, 16);
		e->zIndex = ReaderTOC.readUint32();
		e->Length = ReaderTOC.readUint40();
		e->Offset = ReaderTOC.readUint40();
		Entries.push_back(e);
		numFile++;
	}

	uint32_t b = 1;
	if (blockSize > 0x100) b = 2; else
	if (blockSize > 0x10000) b = 3; else
	if (blockSize > 0x1000000) b = 4;

	int decMax = (archiveFlags == 4) ? 32 : 0;
	int zLengthsCount = (TotalTOCSize - ((int)ReaderTOC.pBaseStream.tellg() + decMax)) / b;
	zLengths.reserve(zLengthsCount);

	for (int i = 0; i < zLengthsCount; i++)
	{
		if (b == 2) zLengths.push_back(ReaderTOC.readUint16()); else
		if (b == 3) zLengths.push_back(ReaderTOC.readUint24()); else
		if (b == 4) zLengths.push_back(ReaderTOC.readUint32());
	}

	// don't need ReaderTOC anymore
	delete[] bufferTOC;

	std::for_each(Entries.begin(), Entries.end(), [&](Entry* e)
	{
		e->Data = new Entry::DataPointer(
			*e,
			Reader,
			zLengths,
			blockSize
			);
	});

	{
		Entries[0]->name = "NamesBlock.bin";
		std::vector<char> data;
		Entries[0]->Data->readTo(data);

		int index = 1;

		auto it1 = data.begin();
		for (auto it2 = data.begin(); it2 != data.end(); ++it2)
		{
			if (*it2 == '\n')
			{
				Entries[index++]->name = std::string(it1, it2);
				it1 = it2 + 1;
			}
		}
		Entries[index]->name = std::string(it1, data.end());
		delete *Entries.begin();
		Entries.erase(Entries.begin());
	}
}

PSARC::~PSARC()
{
	for (auto it = Entries.begin(); it != Entries.end(); ++it)
	{
		delete *it;
	}
}

void PSARC::blockXor(char* out, char const* buf, int blockSize)
{
	for (int i = 0; i < blockSize; i++)
		*(out++) ^= *(buf++);
}

void PSARC::decryptTOC(char* output, long len)
{
	const int keyBits = 256;
	const int blockSize = 16;

	unsigned long rk[RKLENGTH(keyBits)];
	int nrounds = rijndaelSetupEncrypt(rk, keys::psarc, keyBits);

	int i;
	char* p;
	char chain[blockSize] = { 0 };
	char temp[blockSize];

	// inplace CFB decryption
	for (i = 0, p = output; i < len / blockSize; i++)
	{
		rijndaelEncrypt(rk, nrounds, (unsigned char*)chain, (unsigned char*)temp);
		std::memcpy(chain, p, blockSize);
		blockXor(p, temp);
		p += blockSize;
	}
	int leftover = len - i * blockSize;
	rijndaelEncrypt(rk, nrounds, (unsigned char*)chain, (unsigned char*)temp);
	blockXor(p, temp, leftover);
}

int PSARC::inflateStream(std::istream& source, int srcLen, std::vector<char>& dest)
{
#define CHUNK 16384

	int ret;
	unsigned have;
	z_stream strm;
	char* in = new char[srcLen]; // reads everything at once; maybe use chunks instead?
	unsigned char out[CHUNK];

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return ret;

	do {
		source.read(in, srcLen);
		strm.avail_in = (uInt)source.gcount();
		if (source.bad()) {
			(void)inflateEnd(&strm);
			return Z_ERRNO;
		}
		if (strm.avail_in == 0)
			break;
		strm.next_in = (Bytef*)in;

		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
			}
			have = CHUNK - strm.avail_out;
			dest.insert(dest.end(), out, out + have);
		} while (strm.avail_out == 0);

	} while (ret != Z_STREAM_END);

	delete[]in;
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}


void PSARC::Entry::DataPointer::readTo(std::vector<char>& storage)
{
	if (_Entry.Length != 0)
	{
		Reader.setPos(_Entry.Offset);
		storage.reserve(_Entry.Length);

		for (size_t i = _Entry.zIndex; storage.size() < _Entry.Length; i++)
		{
			if (zLengths[i] == 0)
			{
				int oldSize = storage.size();
				storage.resize(storage.size() + blockSize);
				Reader.pBaseStream.read(storage.data() + oldSize, blockSize);
			}
			else
			{
				uint32_t header = Reader.readUint16();
				Reader.addPos(-2);
				if (header == 30938)
				{
					inflateStream(Reader.pBaseStream, zLengths[i], storage);
				}
				else
				{
					int oldSize = storage.size();
					storage.resize(storage.size() + zLengths[i]);
					Reader.pBaseStream.read(storage.data() + oldSize, zLengths[i]);
				}
			}
		}
	}
};

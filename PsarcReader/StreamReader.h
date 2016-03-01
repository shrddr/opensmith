#pragma once
#include <istream>

struct membuf : std::streambuf
{
	membuf(char* begin, char* end)
	{
		pos = 0;
		this->setg(begin, begin, end);
	}
	membuf(char* begin, size_t size)
	{
		this->setg(begin, begin, begin+size);
	}
	std::streampos seekpos(std::streampos sp, std::ios_base::openmode which)
	{
		pos = sp;
		this->setg(this->eback(), this->eback() + pos, this->egptr());
		return pos;
	}
	std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which)
	{
		pos += off;
		this->setg(this->eback(), this->eback() + pos, this->egptr());
		return pos;
	}
	std::streampos pos;
};

class StreamReaderLE
{
public:
	StreamReaderLE(std::istream& input) :
		pBaseStream(input) 
	{
		pos = 0; 
	}
	~StreamReaderLE() {}
	int pos; // for memory streams
	void setPos(uint64_t position);
	void addPos(int delta);
	std::istream& pBaseStream;
	void readBytes(char* dest, int count);
	uint32_t readUint16();
	uint32_t readUint24();
	uint32_t readUint32();
	uint64_t readUint40();
	float readSingle();
	float readDouble();
	int8_t readInt8();
	int16_t readInt16();
	int32_t readInt32();
};

class StreamReaderBE : public StreamReaderLE
{
public:
	StreamReaderBE(std::istream& input) :
		StreamReaderLE(input) {}
	uint32_t readUint16();
	uint32_t readUint24();
	uint32_t readUint32();
	uint64_t readUint40();
};
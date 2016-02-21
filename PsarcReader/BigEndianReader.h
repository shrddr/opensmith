#pragma once
#include <istream>
#include <array>

class BinaryReader
{
public:
	BinaryReader(std::istream& input) :
		pBaseStream(input) 
	{
		pos = 0; 
	}
	~BinaryReader() {}
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

class BigEndianReader : public BinaryReader
{
public:
	BigEndianReader(std::istream& input) :
		BinaryReader(input) {}
	uint32_t readUint16();
	uint32_t readUint24();
	uint32_t readUint32();
	uint64_t readUint40();
};
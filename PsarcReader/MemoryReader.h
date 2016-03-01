#pragma once
#include <vector>
#include <cstdint>
#include <cstdlib>
class MemoryReaderLE
{
public:
	MemoryReaderLE(char* data) : data(data), pos(0) {};
	MemoryReaderLE(std::vector<char>& storage) : data(storage.data()), pos(0) {};
	void setPos(uint64_t position) { pos = position; }
	void readBytes(char* dest, int count);
	int8_t readInt8();
	int32_t readInt32();
	uint32_t readUint16();
	uint32_t readUint32();
	char* data;
	size_t pos;
};

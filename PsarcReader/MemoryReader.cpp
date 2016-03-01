#include "MemoryReader.h"
#include <cstring>

void MemoryReaderLE::readBytes(char* dest, int count)
{
	std::memcpy(dest, data + pos, count);
	pos += count;
}

uint32_t MemoryReaderLE::readUint16()
{
	uint16_t* result = (uint16_t*)(data + pos);
	pos += 2;
	return *result;
}

uint32_t MemoryReaderLE::readUint32()
{
	uint32_t* result = (uint32_t*)(data + pos);
	pos += 4;
	return *result;
}
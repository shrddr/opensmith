#include "MemoryReader.h"
#include <cstring>

void MemoryReaderLE::readBytes(char* dest, int count)
{
	std::memcpy(dest, data + pos, count);
	pos += count;
}

int8_t MemoryReaderLE::readInt8()
{
	int8_t* result = (int8_t*)(data + pos);
	pos += 1;
	return *result;
}

int32_t MemoryReaderLE::readInt32()
{
	int32_t* result = (int32_t*)(data + pos);
	pos += 4;
	return *result;
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
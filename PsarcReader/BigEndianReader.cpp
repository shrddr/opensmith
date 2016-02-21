#include "BigEndianReader.h"

void BinaryReader::setPos(uint64_t position)
{
	pBaseStream.seekg(position);
	pos = pBaseStream.tellg();
}

void BinaryReader::addPos(int delta)
{
	pBaseStream.seekg(delta, pBaseStream.cur);
	pos = pBaseStream.tellg();
}

void BinaryReader::readBytes(char* dest, int count)
{
	pos += count;
	pBaseStream.read(dest, count);
}

uint32_t BinaryReader::readUint16()
{
	pos += 2;
	char buf[4];
	buf[3] = buf[2] = 0;
	pBaseStream.read(buf, 2);
	uint32_t* result = (uint32_t*)buf;
	return *result;
}

uint32_t BinaryReader::readUint24()
{
	pos += 3;
	char buf[4];
	buf[3] = 0;
	pBaseStream.read(buf, 3);
	uint32_t* result = (uint32_t*)buf;
	return *result;
}

uint32_t BinaryReader::readUint32()
{
	pos += 4;
	char buf[4];
	pBaseStream.read(buf, 4);
	uint32_t* result = (uint32_t*)buf;
	return *result;
}

uint64_t BinaryReader::readUint40()
{
	pos += 5;
	char buf[8];
	buf[7] = buf[6] = buf[5] = 0;
	pBaseStream.read(buf, 5);
	uint64_t* result = (uint64_t*)buf;
	return *result;
}

float BinaryReader::readSingle()
{
	pos += 4;
	char buf[4];
	pBaseStream.read(buf, 4);
	float* result = (float*)buf;
	return *result;
}

float BinaryReader::readDouble()
{
	pos += 8;
	char buf[8];
	pBaseStream.read(buf, 8);
	double* result = (double*)buf;
	return *result;
}

int8_t BinaryReader::readInt8()
{
	pos += 1;
	char buf[1];
	pBaseStream.read(buf, 1);
	int8_t* result = (int8_t*)buf;
	return *result;
}

int16_t BinaryReader::readInt16()
{
	pos += 2;
	char buf[2];
	pBaseStream.read(buf, 2);
	int16_t* result = (int16_t*)buf;
	return *result;
}

int32_t BinaryReader::readInt32()
{
	pos += 4;
	char buf[4];
	pBaseStream.read(buf, 4);
	int32_t* result = (int32_t*)buf;
	return *result;
}

//---------------------

uint32_t BigEndianReader::readUint16()
{
	char buf[4];
	pBaseStream.read(buf, 2);
	uint32_t result = 0;
	unsigned char *src = (unsigned char *)&buf;
	unsigned char *dst = (unsigned char *)&result;
	dst[0] = src[1];
	dst[1] = src[0];
	return result;
}

uint32_t BigEndianReader::readUint24()
{
	char buf[4];
	pBaseStream.read(buf, 3);
	uint32_t result = 0;
	unsigned char *src = (unsigned char *)&buf;
	unsigned char *dst = (unsigned char *)&result;
	dst[0] = src[2];
	dst[1] = src[1];
	dst[2] = src[0];
	return result;
}

uint32_t BigEndianReader::readUint32()
{
	char buf[4];
	pBaseStream.read(buf, 4);
	uint32_t result;
	unsigned char *src = (unsigned char *)&buf;
	unsigned char *dst = (unsigned char *)&result;
	dst[0] = src[3];
	dst[1] = src[2];
	dst[2] = src[1];
	dst[3] = src[0];
	return result;
}

uint64_t BigEndianReader::readUint40()
{
	char buf[8];
	pBaseStream.read(buf, 5);
	uint64_t result = 0;
	unsigned char *src = (unsigned char *)&buf;
	unsigned char *dst = (unsigned char *)&result;
	dst[0] = src[4];
	dst[1] = src[3];
	dst[2] = src[2];
	dst[3] = src[1];
	dst[4] = src[0];
	return result;
}
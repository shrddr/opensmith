#pragma once
#include <stdint.h>
#include <vector>
#include <fstream>
#include "crc.h"

class OggStream
{
public:
	OggStream(std::vector<char>& storage);
	void put(bool bit);
	void flushBits(void);
	void flushPage(bool next_continued = false, bool last = false);
	void setGranule(uint32_t g) { granule = g; }
private:
	std::vector<char>& outStorage;
	enum { header_bytes = 27, max_segments = 255, segment_size = 255 };
	char bitBuffer;
	int bitsStored;
	char pageBuffer[header_bytes + max_segments + segment_size * max_segments];
	int pageBytes;
	bool first, continued;
	uint32_t granule;
	uint32_t seqno;
};

class BitStream
{
public:
	BitStream(std::istream& inStream) :
		inStream(inStream),
		bitBuffer(0),
		bitsLeft(0),
		totalBitsRead(0) {}
	bool get();
private:
	std::istream& inStream;
	char bitBuffer;
	int bitsLeft;
	long totalBitsRead;
};

template <unsigned int BITSIZE>
class BitUint
{
public:
	BitUint() :
		value(0) {}
	BitUint(unsigned int v) :
		value(v) {}
	operator unsigned int() const { return value; }

	friend BitStream& operator>> (BitStream& bstream, BitUint& bui)
	{
		bui.value = 0;
		for (int i = 0; i < BITSIZE; i++)
			if (bstream.get()) bui.value |= (1U << i);
		return bstream;
	}

	friend OggStream& operator<< (OggStream& bstream, const BitUint& bui)
	{
		for (int i = 0; i < BITSIZE; i++)
			bstream.put((bui.value & (1U << i)) != 0);
		return bstream;
	}

private:
	unsigned int value;
};

class BitUintRuntime {
public:
	explicit BitUintRuntime(unsigned int s) :
		size(s),
		value(0) {}

	BitUintRuntime(unsigned int s, unsigned int v) :
		size(s),
		value(v) {}

	operator unsigned int() const { return value; }

	friend BitStream& operator>> (BitStream& bstream, BitUintRuntime& bui) {
		bui.value = 0;
		for (unsigned int i = 0; i < bui.size; i++)
			if (bstream.get()) bui.value |= (1U << i);
		return bstream;
	}

	friend OggStream& operator<< (OggStream& bstream, const BitUintRuntime& bui) {
		for (unsigned int i = 0; i < bui.size; i++)
			bstream.put((bui.value & (1U << i)) != 0);
		return bstream;
	}
private:
	unsigned int size;
	unsigned int value;
};

// used for codebooks only
class array_streambuf : public std::streambuf
{
	// Intentionally undefined
	array_streambuf& operator=(const array_streambuf& rhs);
	array_streambuf(const array_streambuf &rhs);

	char* arr;

public:
	array_streambuf(const char* a, int l) : arr(0)
	{
		arr = new char[l];
		for (int i = 0; i < l; i++)
			arr[i] = a[i];
		setg(arr, arr, arr + l);
	}
	~array_streambuf()
	{
		delete[] arr;
	}
};
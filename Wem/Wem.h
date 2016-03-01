#pragma once
#include "bitstream.h"
#include "PsarcReader/StreamReader.h"

class VorbisPacketHeader
{
public:
	explicit VorbisPacketHeader(uint8_t t) : type(t) {}
	friend OggStream& operator << (OggStream& bstream, const VorbisPacketHeader& vph)
	{
		BitUint<8> t(vph.type);
		bstream << t;

		for (int i = 0; i < 6; i++)
		{
			BitUint<8> c(vorbis_str[i]);
			bstream << c;
		}

		return bstream;
	}
private:
	uint8_t type;
	static const char vorbis_str[6];
};

class Wem
{
public:
	Wem(char const* fileName);
	Wem(std::vector<char>& input);
	~Wem();
	void init();
	void generateOgg(std::vector<char>& storage);
	void generateOggHeader(OggStream& os, bool*& modeBlockflag, int& modeBits);
	uint32_t getSampleRate() { return sampleRate; }
private:
	std::istream* inStream;
	membuf* data;
	long fileSize;
	bool littleEndian;
	long riffSize;
	long fmtOffset, dataOffset;
	long fmtSize, dataSize;
	// RIFF fmt
	uint16_t channels;
	uint32_t sampleRate;
	uint32_t avgBytesPerSecond;
	// RIFF extended fmt
	uint16_t _ext_unk;
	uint32_t _subtype;
	// vorbis info
	uint32_t sampleCount;
	uint32_t setupPacketOffset;
	uint32_t firstAudioPacketOffset;
	uint32_t _uid;
	uint8_t _blocksize_0_pow;
	uint8_t _blocksize_1_pow;
	bool noGranule, modPackets;
};
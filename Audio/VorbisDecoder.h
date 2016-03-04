#pragma once
#include <vector>
#include <vorbis/codec.h>
#include <ogg/ogg.h>
#include <fstream>
#include "Audio/Audio.h"
#include <iostream>

class VorbisDecoder
{
public:
	VorbisDecoder(std::vector<char>& inStorage);
	~VorbisDecoder();
	bool getPCM(float*** data, ogg_int64_t& timestamp, int& count);
	void gotPCM(int count);
	bool ready() { return eos == notEos; }
	long getRate() { return vi.rate; }
private:
	std::vector<char>& data;
	size_t dataPos;
	enum { notEos, expectedEos, unexpectedEos } eos;
	void getData();
	void getPage();
	void getPacket();
	ogg_int64_t granulePos;
	size_t lastBlockSize;

	ogg_sync_state syncState;
	ogg_stream_state streamState;
	ogg_packet packet;
	ogg_page page;
	vorbis_info vi;
	vorbis_comment vc;
	vorbis_dsp_state vds;
	vorbis_block vb;

	std::ofstream log;
};

class VorbisBuffer : public AudioProducer
{
public:
	VorbisBuffer(VorbisDecoder& d) : 
		decoder(d),
		streamPos(0),
		size(0),
		pos(0) {}
	void getPCM(const float time, float& left, float& right)
	{
		while (true)
		{
			if (pos == size)
			{
				ready = decoder.getPCM(&pcm, streamPos, size);
				pos = 0;
			}

			if (ready)
			{
				float timestamp = (float)(streamPos + pos) / decoder.getRate();

				left = pcm[0][pos];
				right = pcm[1][pos];
				pos++;
				if (pos == size)
					decoder.gotPCM(size);

				// TODO: this is dumb and only seeks forward. Move into Decoder class and allow seek to any position
				if (timestamp > time)
					break;
			}
			else
			{
				left = 0;
				right = 0;
				break;
			}
		}
	}
private:
	VorbisDecoder& decoder;
	bool ready;
	float** pcm;
	int64_t streamPos;
	int size;
	int pos;
};
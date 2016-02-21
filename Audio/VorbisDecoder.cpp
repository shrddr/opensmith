#include "VorbisDecoder.h"
#include <iostream>
#include <fstream>

void VorbisDecoder::getData()
{
	char* buffer = ogg_sync_buffer(&syncState, 4096); //can return NULL on error
	
	static size_t pos = 0;
	size_t numRead = 4096;
	if (pos + 4096 > data.size()) numRead = data.size() - pos;
	if (numRead == 0)
	{
		eos = unexpectedEos;
		return;
	}

	memcpy(buffer, &data[pos], numRead);
	pos += numRead;

	ogg_sync_wrote(&syncState, numRead);
}


VorbisDecoder::VorbisDecoder(std::vector<char>& inStorage):
	data(inStorage),
	eos(notEos),
	granulePos(0),
	lastBlockSize(0)
{
	//log.open("decoderLog.txt");

	ogg_sync_init(&syncState);
	vorbis_info_init(&vi);

	getData();
	
	int res = ogg_sync_pageout(&syncState, &page);
	//log << "_pageout " << res << std::endl;

	ogg_stream_init(&streamState, ogg_page_serialno(&page));
	ogg_stream_pagein(&streamState, &page);
	res = ogg_stream_packetout(&streamState, &packet);
	//log << "_packetout " << res << std::endl;

	
	vorbis_comment_init(&vc);
	vorbis_synthesis_headerin(&vi, &vc, &packet);

	int packets = 1;
	while (packets < 3)
	{
		int res = ogg_sync_pageout(&syncState, &page);
		//log << "_pageout " << res << std::endl;
		if (res == 0)
		{
			getData();
			continue;
		}
		if (res == 1)
		{
			ogg_stream_pagein(&streamState, &page);
			while (packets < 3)
			{
				int res = ogg_stream_packetout(&streamState, &packet);
				//log << "_packetout " << res << std::endl;
				if (res == 0)
					break;
				vorbis_synthesis_headerin(&vi, &vc, &packet);
				packets++;
			}
		}
	}

	vorbis_synthesis_init(&vds, &vi);
	vorbis_block_init(&vds, &vb);

}

void VorbisDecoder::getPage()
{
	while (eos == notEos)
	{
		int res = ogg_sync_pageout(&syncState, &page);
		//log << "pageout " << res << std::endl;

		if (res == 0)
		{
			getData();
			continue;
		}
		if (ogg_page_eos(&page))
			eos = expectedEos;

		ogg_stream_pagein(&streamState, &page);
		return;
	}
}

void VorbisDecoder::getPacket()
{
	while ((ogg_stream_packetout(&streamState, &packet)) < 1)
	{
		if (eos == notEos)
			getPage();
		else
			return;
	}

	int blockSize = vorbis_packet_blocksize(&vi, &packet);
	if (lastBlockSize)
		granulePos += (lastBlockSize + blockSize) / 4;
	lastBlockSize = blockSize;

	//log << "packetout " << res << " (" << packet.granulepos << ")" << std::endl;
}

bool VorbisDecoder::getPCM(float*** data, ogg_int64_t& timestamp, int& count)
{
	if (eos != notEos)
		return false;

	while ((count = vorbis_synthesis_pcmout(&vds, data)) < 1)
	{
		//log << "pcmout " << count << std::endl;
		getPacket();
		vorbis_synthesis(&vb, &packet);
		vorbis_synthesis_blockin(&vds, &vb);
	}

	timestamp = granulePos;
	return true;
}

void VorbisDecoder::gotPCM(int count)
{
	vorbis_synthesis_read(&vds, count);
}
#include "bitstream.h"

void writeInt32(char* buf, uint32_t v)
{
	*(uint32_t*)buf = v;
}

OggStream::OggStream(std::vector<char>& storage) :
	outStorage(storage),
	bitBuffer(0),
	bitsStored(0),
	pageBytes(0),
	first(true),
	continued(false),
	granule(0),
	seqno(0),
	pageBuffer("")
{

}

void OggStream::put(bool bit)
{
	if (bit)
		bitBuffer |= 1 << bitsStored;
	bitsStored++;
	if (bitsStored == 8)
		flushBits();
};

void OggStream::flushBits(void) {
	if (bitsStored != 0) {
		if (pageBytes == segment_size * max_segments)
		{
			throw std::exception("ran out of space in an Ogg packet");
			flushPage(true);
		}

		pageBuffer[header_bytes + max_segments + pageBytes] = bitBuffer;
		pageBytes++;

		bitsStored = 0;
		bitBuffer = 0;
	}
}

void OggStream::flushPage(bool next_continued, bool last) {
	if (pageBytes != segment_size * max_segments)
		flushBits();

	if (pageBytes != 0)
	{
		unsigned int segments = (pageBytes + segment_size) / segment_size;  // intentionally round up
		if (segments == max_segments + 1) segments = max_segments; // at max eschews the final 0

																   // move payload back
		for (int i = 0; i < pageBytes; i++)
			pageBuffer[header_bytes + segments + i] = pageBuffer[header_bytes + max_segments + i];

		pageBuffer[0] = 'O';
		pageBuffer[1] = 'g';
		pageBuffer[2] = 'g';
		pageBuffer[3] = 'S';
		pageBuffer[4] = 0;					// stream_structure_version
		pageBuffer[5] = (continued ? 1 : 0) | (first ? 2 : 0) | (last ? 4 : 0); // header type
		writeInt32(pageBuffer + 6, granule);  // granule low bits
		writeInt32(pageBuffer + 10, 0);       // granule high bits
		if (granule == 0xFFFFFFFF)
			writeInt32(pageBuffer + 10, 0xFFFFFFFF);
		writeInt32(pageBuffer + 14, 1);       // stream serial number
		writeInt32(pageBuffer + 18, seqno);   // page sequence number
		writeInt32(pageBuffer + 22, 0);       // checksum (later)
		pageBuffer[26] = segments;          // segment count

											// segment sizes
		for (unsigned int i = 0, bytesLeft = pageBytes; i < segments; i++)
		{
			if (bytesLeft >= segment_size)
			{
				bytesLeft -= segment_size;
				pageBuffer[27 + i] = segment_size;
			}
			else
				pageBuffer[27 + i] = bytesLeft;
		}

		writeInt32(pageBuffer + 22,
			checksum(pageBuffer, header_bytes + segments + pageBytes)
			);

		// output to ostream
		outStorage.insert(outStorage.end(), pageBuffer, pageBuffer + header_bytes + segments + pageBytes);

		seqno++;
		first = false;
		continued = next_continued;
		pageBytes = 0;
	}
}

bool BitStream::get()
{
	class Out_of_bits {};

	if (bitsLeft == 0)
	{
		int c = inStream.get();
		if (c == EOF) throw Out_of_bits();
		bitBuffer = c;
		bitsLeft = 8;
	}
	totalBitsRead++;
	bitsLeft--;
	return ((bitBuffer & (0x80 >> bitsLeft)) != 0);
}

#include <fstream>
#include "codebook.h"
#include "bitstream.h"
#include "PsarcReader/BigEndianReader.h"

unsigned int _book_maptype1_quantvals(unsigned int entries, unsigned int dimensions) {
	/* get us a starting hint, we'll polish it below */
	int bits = ilog(entries);
	int vals = entries >> ((bits - 1)*(dimensions - 1) / dimensions);

	while (1) {
		unsigned long acc = 1;
		unsigned long acc1 = 1;
		unsigned int i;
		for (i = 0; i<dimensions; i++) {
			acc *= vals;
			acc1 *= vals + 1;
		}
		if (acc <= entries && acc1>entries) {
			return(vals);
		}
		else {
			if (acc>entries) {
				vals--;
			}
			else {
				vals++;
			}
		}
	}
}

CodebookLibrary::CodebookLibrary(const char* fileName):
	codebookData(0),
	codebookOffsets(0),
	codebookCount(0)
{
	std::ifstream is(fileName, std::ios::binary);
	if (!is) throw std::string(fileName);

	is.seekg(0, std::ios::end);
	long file_size = is.tellg();

	BinaryReader r(is);
	r.setPos(file_size - 4);

	long offsetOffset = r.readInt32();
	codebookCount = (file_size - offsetOffset) / 4;

	codebookData = new char[offsetOffset];
	codebookOffsets = new long[codebookCount];

	is.seekg(0);
	for (long i = 0; i < offsetOffset; i++)
		codebookData[i] = is.get();

	for (long i = 0; i < codebookCount; i++)
		codebookOffsets[i] = r.readInt32();

	// so first we read from file to memory
	// then istream >> from memory? oh well
}

CodebookLibrary::~CodebookLibrary()
{
	delete[] codebookData;
	delete[] codebookOffsets;
}

void CodebookLibrary::rebuild(int i, OggStream& bos)
{
	const char* cb = getCodebook(i);
	unsigned long cb_size;

	{
		long signed_cb_size = getCodebookSize(i);
		if (!cb || -1 == signed_cb_size)
			throw std::string("wrong codebook id");
		cb_size = signed_cb_size;
	}

	array_streambuf asb(cb, cb_size);
	std::istream is(&asb);
	BitStream bis(is);

	rebuild(bis, cb_size, bos);
}

void CodebookLibrary::rebuild(BitStream &bis, unsigned long cb_size, OggStream& bos)
{
	/* IN: 4 bit dimensions, 14 bit entry count */

	BitUint<4> dimensions;
	BitUint<14> entries;

	bis >> dimensions >> entries;
	bos << BitUint<24>(0x564342) << BitUint<16>(dimensions) << BitUint<24>(entries);

	// gather codeword lengths
	BitUint<1> ordered;
	bis >> ordered;
	bos << ordered;
	if (ordered)
	{
		BitUint<5> initialLength;
		bis >> initialLength;
		bos << initialLength;

		unsigned int currentEntry = 0;
		while (currentEntry < entries)
		{
			/* IN/OUT: ilog(entries-current_entry) bit count w/ given length */
			BitUintRuntime number(ilog(entries - currentEntry));
			bis >> number;
			bos << number;
			currentEntry += number;
		}
		if (currentEntry > entries)
			throw std::string("current_entry out of range");
	}
	else
	{
		BitUint<3> codewordLengthLength;
		BitUint<1> sparse;
		bis >> codewordLengthLength >> sparse;

		if (0 == codewordLengthLength || 5 < codewordLengthLength)
			throw std::string("nonsense codeword length");

		bos << sparse;

		for (int i = 0; i < entries; i++)
		{
			bool presentBool = true;

			if (sparse)
			{
				BitUint<1> present;
				bis >> present;
				bos << present;
				presentBool = (0 != present);
			}

			if (presentBool)
			{
				BitUintRuntime codewordLength(codewordLengthLength);
				bis >> codewordLength;
				bos << BitUint<5>(codewordLength);
			}
		}
	} // done with lengths


	// lookup table
	BitUint<1> lookup_type;
	bis >> lookup_type;
	bos << BitUint<4>(lookup_type);

	if (0 == lookup_type)
	{
		//cout << "no lookup table" << endl;
	}
	else if (1 == lookup_type)
	{
		BitUint<32> min, max;
		BitUint<4> valueLength;
		BitUint<1> sequenceFlag;
		bis >> min >> max >> valueLength >> sequenceFlag;
		bos << min << max << valueLength << sequenceFlag;

		unsigned int quantvals = _book_maptype1_quantvals(entries, dimensions);
		for (unsigned int i = 0; i < quantvals; i++)
		{
			BitUintRuntime val(valueLength + 1);
			bis >> val;
			bos << val;
		}
	}
	else if (2 == lookup_type)
		throw std::string("didn't expect lookup type 2");
	else
		throw std::string("invalid lookup type");

	/* check that we used exactly all bytes */
	/* note: if all bits are used in the last byte there will be one extra 0 byte */
	//if (0 != cb_size && bis.get_total_bits_read() / 8 + 1 != cb_size)
	//	throw Size_mismatch(cb_size, bis.get_total_bits_read() / 8 + 1);
}

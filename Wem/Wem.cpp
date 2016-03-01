/* Original code by https://github.com/hcs64/ww2ogg */

#include "Wem.h"
#include <fstream>
#include <cstring>
#include "codebook.h"

class Packet
{
public:
	Packet(std::istream& i, long o, bool little_endian, bool no_granule = false) : 
		_offset(o),
		_size(-1),
		_absoluteGranule(0),
		_noGranule(no_granule) 
	{
		i.seekg(_offset);

		if (little_endian)
		{
			StreamReaderLE r(i);
			_size = r.readUint16();
			if (!_noGranule)
				_absoluteGranule = r.readUint32();
		}
		else
		{
			StreamReaderBE r(i);
			_size = r.readUint16();
			if (!_noGranule)
				_absoluteGranule = r.readUint32();
		}
	}

	long header_size(void) { return _noGranule ? 2 : 6; }
	long offset(void) { return _offset + header_size(); }
	uint16_t size(void) { return _size; }
	uint32_t granule(void) { return _absoluteGranule; }
	long next_offset(void) { return _offset + header_size() + _size; }
private:
	long _offset;
	uint16_t _size;
	uint32_t _absoluteGranule;
	bool _noGranule;
};

Wem::Wem(char const* fileName)
{
	data = 0;
	inStream = new std::ifstream;
	std::ifstream* f = (std::ifstream*)inStream;
	f->open(fileName, std::ifstream::in | std::ifstream::binary);
	if (!f->good())
		throw std::runtime_error("WEM file not found.\n");

	f->seekg(0, std::ifstream::end);
	fileSize = f->tellg();
	f->seekg(0);
	init();
}

Wem::Wem(std::vector<char>& input)
{
	data = new membuf(input.data(), input.data() + input.size());
	inStream = new std::istream(data);
	fileSize = input.size();
	init();
}

Wem::~Wem()
{
	if (data != 0)
		delete data;
	delete inStream;
}

void Wem::init()
{
	fmtOffset = -1;
	dataOffset = -1;
	fmtSize = -1;
	dataSize = -1;
	noGranule = false;

	StreamReaderLE r(*inStream);

	// check RIFF header
	{
		char riffHeader[4], waveHeader[4];
		r.readBytes(riffHeader, 4);

		if (std::memcmp(riffHeader, "RIFF", 4))
			throw std::runtime_error("missing RIFF");
		else
			littleEndian = true;

		riffSize = r.readInt32() + 8;
		if (riffSize > fileSize)
			throw std::runtime_error("RIFF truncated");

		r.readBytes(waveHeader, 4);
		if (std::memcmp(waveHeader, "WAVE", 4))
			throw std::runtime_error("missing WAVE");
	}

	// read chunks
	long chunkOffset = 12;
	while (chunkOffset < riffSize)
	{
		r.setPos(chunkOffset);

		if (chunkOffset + 8 > riffSize)
			throw std::runtime_error("chunk header truncated");

		char chunkType[4];
		r.readBytes(chunkType, 4);

		uint32_t chunkSize;
		chunkSize = r.readInt32();

		if (!std::memcmp(chunkType, "fmt ", 4))
		{
			fmtOffset = r.pos;
			fmtSize = chunkSize;
		}
		else if (!std::memcmp(chunkType, "data", 4))
		{
			dataOffset = r.pos;
			dataSize = chunkSize;
		}

		chunkOffset = r.pos + chunkSize;
	}

	if (chunkOffset > riffSize)
		throw std::runtime_error("chunk truncated");

	// check that we have the chunks we're expecting
	if (-1 == fmtOffset && -1 == dataOffset)
		throw std::runtime_error("expected fmt, data chunks");

	// read fmt
	if (0x42 != fmtSize)
		throw std::runtime_error("expected 0x42 fmt if vorb missing");	

	r.setPos(fmtOffset);
	if (0xFFFF != r.readUint16())
		throw std::runtime_error("bad codec id");

	// RIFF fmt
	channels = r.readUint16();
	sampleRate = r.readUint32();
	avgBytesPerSecond = r.readUint32();
	if (0U != r.readUint16())
		throw std::runtime_error("bad block align");
	if (0U != r.readUint16())
		throw std::runtime_error("expected 0 bps");
	if (fmtSize - 0x12 != r.readUint16())
		throw std::runtime_error("bad extra fmt length");

	// RIFF extended fmt
	if (fmtSize - 0x12 >= 2)
	{
		_ext_unk = r.readUint16();
		if (fmtSize - 0x12 >= 6)
			_subtype = r.readUint32();
	}

	// read vorb (from now on assuming vorbSize == -1)
	long vorbOffset = fmtOffset + 0x18;
	r.setPos(vorbOffset + 0x00);
	sampleCount = r.readUint32();
	noGranule = true;

	r.setPos(vorbOffset + 0x4);
	uint32_t mod_signal = r.readUint32();
	if (0x4A != mod_signal &&
		0x4B != mod_signal &&
		0x69 != mod_signal &&
		0x70 != mod_signal) {
		modPackets = true;
	}
	r.setPos(vorbOffset + 0x10);
	setupPacketOffset = r.readUint32();
	firstAudioPacketOffset = r.readUint32();

	r.setPos(vorbOffset + 0x24);
	_uid = r.readUint32();
	_blocksize_0_pow = r.readInt8();
	_blocksize_1_pow = r.readInt8();
}

void Wem::generateOgg(std::vector<char>& storage)
{
	OggStream os(storage);

	bool* modeBlockflag = NULL;
	int modeBits = 0;
	bool prevBlockflag = false;

	generateOggHeader(os, modeBlockflag, modeBits);

	// Audio pages
	{
		long offset = dataOffset + firstAudioPacketOffset;
		while (offset < dataOffset + dataSize)
		{
			Packet audioPacket(*inStream, offset, littleEndian, noGranule);
			long packetHeaderSize = audioPacket.header_size();
			long size = audioPacket.size();
			long packetPayloadOffset = audioPacket.offset();
			long granule = audioPacket.granule();
			long nextOffset = audioPacket.next_offset();

			if (offset + packetHeaderSize > dataOffset + dataSize)
				throw std::runtime_error("page header truncated");

			offset = packetPayloadOffset;

			inStream->seekg(offset);

			// HACK: don't know what to do here
			if (granule == UINT32_C(0xFFFFFFFF))
				os.setGranule(1);
			else
				os.setGranule(granule);

			// first byte
			if (modPackets)
			{
				// need to rebuild packet type and window info
				if (!modeBlockflag)
					throw std::runtime_error("didn't load mode_blockflag");

				// OUT: 1 bit packet type (0 == audio)
				BitUint<1> packetType(0);
				os << packetType;

				BitUintRuntime* modeNumberP = 0;
				BitUintRuntime* remainderP = 0;

				{
					// collect mode number from first byte
					BitStream ss(*inStream);

					modeNumberP = new BitUintRuntime(modeBits);
					ss >> *modeNumberP;
					os << *modeNumberP;

					remainderP = new BitUintRuntime(8 - modeBits);
					ss >> *remainderP;
				}

				if (modeBlockflag[*modeNumberP])
				{
					// long window, peek at next frame

					inStream->seekg(nextOffset);
					bool nextBlockflag = false;
					if (nextOffset + packetHeaderSize <= dataOffset + dataSize)
					{
						Packet audioPacket(*inStream, nextOffset, littleEndian, noGranule);
						uint32_t nextPacketSize = audioPacket.size();
						if (nextPacketSize > 0)
						{
							inStream->seekg(audioPacket.offset());

							BitStream ss(*inStream);
							BitUintRuntime nextModeNumber(modeBits);

							ss >> nextModeNumber;

							nextBlockflag = modeBlockflag[nextModeNumber];
						}
					}

					BitUint<1> prev_window_type(prevBlockflag);
					os << prev_window_type;

					BitUint<1> next_window_type(nextBlockflag);
					os << next_window_type;

					// fix seek for rest of stream
					inStream->seekg(offset + 1);
				}

				prevBlockflag = modeBlockflag[*modeNumberP];
				delete modeNumberP;

				// OUT: remaining bits of first (input) byte
				os << *remainderP;
				delete remainderP;
			}
			else
			{
				// nothing unusual for first byte
				int v = inStream->get();
				if (v < 0)
					throw std::runtime_error("file truncated");
				BitUint<8> c(v);
				os << c;
			}

			// remainder of packet
			for (int i = 1; i < size; i++)
			{
				int v = inStream->get();
				if (v < 0)
					throw std::runtime_error("file truncated");
				BitUint<8> c(v);
				os << c;
			}

			offset = nextOffset;
			os.flushPage(false, (offset == dataOffset + dataSize));
		}

		if (offset > dataOffset + dataSize)
			throw std::runtime_error("page truncated");
	}

	delete[] modeBlockflag;
}

const char VorbisPacketHeader::vorbis_str[6] = { 'v','o','r','b','i','s' };

void Wem::generateOggHeader(OggStream& os, bool*& modeBlockflag, int& modeBits)
{
	// generate identification packet
	{
		VorbisPacketHeader vhead(1);
		os << vhead;
		BitUint<32> version(0);
		os << version;
		BitUint<8> ch(channels);
		os << ch;
		BitUint<32> srate(sampleRate);
		os << srate;
		BitUint<32> bitrate_max(0);
		os << bitrate_max;
		BitUint<32> bitrate_nominal(avgBytesPerSecond * 8);
		os << bitrate_nominal;
		BitUint<32> bitrate_minimum(0);
		os << bitrate_minimum;
		BitUint<4> blocksize_0(_blocksize_0_pow);
		os << blocksize_0;
		BitUint<4> blocksize_1(_blocksize_1_pow);
		os << blocksize_1;
		BitUint<1> framing(1);
		os << framing;
		// identification packet on its own page
		os.flushPage();
	}

	// generate comment packet
	{
		VorbisPacketHeader vhead(3);
		os << vhead;

		static const char vendor[] = "converted from Audiokinetic Wwise by ww2ogg 0.24";
		BitUint<32> vendorSize(strlen(vendor));
		os << vendorSize;

		for (int i = 0; i < vendorSize; i++) {
			BitUint<8> c(vendor[i]);
			os << c;
		}

		BitUint<32> userCommentCount(0);
		os << userCommentCount;

		BitUint<1> framing(1);
		os << framing;

		os.flushPage();
	}

	// generate setup packet
	{
		VorbisPacketHeader vhead(5);
		os << vhead;

		Packet setup_packet(*inStream, dataOffset + setupPacketOffset, littleEndian, noGranule);
		if (setup_packet.granule() != 0)
			throw std::runtime_error("setup packet granule != 0");

		inStream->seekg(dataOffset + setupPacketOffset + (noGranule ? 2 : 6));
		BitStream is(*inStream);

		// codebook count
		BitUint<8> codebook_count_less1;
		is >> codebook_count_less1;
		int codebook_count = codebook_count_less1 + 1;
		os << codebook_count_less1;

		// rebuild codebooks
		{
			/* external codebooks */

			CodebookLibrary cbl("../resources/wem/packed_codebooks_aoTuV_603.bin");

			for (int i = 0; i < codebook_count; i++)
			{
				BitUint<10> codebook_id;
				is >> codebook_id;
				cbl.rebuild(codebook_id, os);		
			}
		}

		// Time Domain transforms (placeholder)
		BitUint<6> timeCountLess1(0);
		os << timeCountLess1;
		BitUint<16> dummyTimeValue(0);
		os << dummyTimeValue;

		// assuming not full setup
		// floor count
		BitUint<6> floor_count_less1;
		is >> floor_count_less1;
		unsigned int floor_count = floor_count_less1 + 1;
		os << floor_count_less1;

		// rebuild floors
		for (unsigned int i = 0; i < floor_count; i++)
		{
			// Always floor type 1
			BitUint<16> floorType(1);
			os << floorType;

			BitUint<5> floor1Partitions;
			is >> floor1Partitions;
			os << floor1Partitions;

			unsigned int* floor1_partition_class_list = new unsigned int[floor1Partitions];

			unsigned int maximum_class = 0;
			for (unsigned int j = 0; j < floor1Partitions; j++)
			{
				BitUint<4> floor1_partition_class;
				is >> floor1_partition_class;
				os << floor1_partition_class;

				floor1_partition_class_list[j] = floor1_partition_class;

				if (floor1_partition_class > maximum_class)
					maximum_class = floor1_partition_class;
			}

			unsigned int * floor1_class_dimensions_list = new unsigned int[maximum_class + 1];

			for (unsigned int j = 0; j <= maximum_class; j++)
			{
				BitUint<3> class_dimensions_less1;
				is >> class_dimensions_less1;
				os << class_dimensions_less1;

				floor1_class_dimensions_list[j] = class_dimensions_less1 + 1;

				BitUint<2> class_subclasses;
				is >> class_subclasses;
				os << class_subclasses;

				if (0 != class_subclasses)
				{
					BitUint<8> masterbook;
					is >> masterbook;
					os << masterbook;

					if (masterbook >= codebook_count)
						throw std::runtime_error("invalid floor1 masterbook");
				}

				for (unsigned int k = 0; k < (1U << class_subclasses); k++)
				{
					BitUint<8> subclass_book_plus1;
					is >> subclass_book_plus1;
					os << subclass_book_plus1;

					int subclass_book = static_cast<int>(subclass_book_plus1) - 1;
					if (subclass_book >= 0 && static_cast<unsigned int>(subclass_book) >= codebook_count)
						throw std::runtime_error("invalid floor1 subclass book");
				}
			}

			BitUint<2> floor1_multiplier_less1;
			is >> floor1_multiplier_less1;
			os << floor1_multiplier_less1;

			BitUint<4> rangebits;
			is >> rangebits;
			os << rangebits;

			for (unsigned int j = 0; j < floor1Partitions; j++)
			{
				unsigned int current_class_number = floor1_partition_class_list[j];
				for (unsigned int k = 0; k < floor1_class_dimensions_list[current_class_number]; k++)
				{
					BitUintRuntime X(rangebits);
					is >> X;
					os << X;
				}
			}

			delete[] floor1_class_dimensions_list;
			delete[] floor1_partition_class_list;
		}

		// residue count
		BitUint<6> residue_count_less1;
		is >> residue_count_less1;
		unsigned int residue_count = residue_count_less1 + 1;
		os << residue_count_less1;

		// rebuild residues
		for (unsigned int i = 0; i < residue_count; i++)
		{
			BitUint<2> residue_type;
			is >> residue_type;
			os << BitUint<16>(residue_type);

			if (residue_type > 2)
				throw std::runtime_error("invalid residue type");

			BitUint<24> residue_begin, residue_end, residue_partition_size_less1;
			BitUint<6> residue_classifications_less1;
			BitUint<8> residue_classbook;

			is >> residue_begin >> residue_end >> residue_partition_size_less1 >> residue_classifications_less1 >> residue_classbook;
			unsigned int residue_classifications = residue_classifications_less1 + 1;
			os << residue_begin << residue_end << residue_partition_size_less1 << residue_classifications_less1 << residue_classbook;

			if (residue_classbook >= codebook_count)
				throw std::runtime_error("invalid residue classbook");

			unsigned int * residue_cascade = new unsigned int[residue_classifications];

			for (unsigned int j = 0; j < residue_classifications; j++)
			{
				BitUint<5> high_bits(0);
				BitUint<3> low_bits;

				is >> low_bits;
				os << low_bits;

				BitUint<1> bitflag;
				is >> bitflag;
				os << bitflag;
				if (bitflag)
				{
					is >> high_bits;
					os << high_bits;
				}

				residue_cascade[j] = high_bits * 8 + low_bits;
			}

			for (unsigned int j = 0; j < residue_classifications; j++)
			{
				for (unsigned int k = 0; k < 8; k++)
				{
					if (residue_cascade[j] & (1 << k))
					{
						BitUint<8> residue_book;
						is >> residue_book;
						os << residue_book;

						if (residue_book >= codebook_count)
							throw std::runtime_error("invalid residue book");
					}
				}
			}

			delete[] residue_cascade;
		}

		// mapping count
		BitUint<6> mapping_count_less1;
		is >> mapping_count_less1;
		unsigned int mapping_count = mapping_count_less1 + 1;
		os << mapping_count_less1;

		for (unsigned int i = 0; i < mapping_count; i++)
		{
			// always mapping type 0, the only one
			BitUint<16> mapping_type(0);

			os << mapping_type;

			BitUint<1> submaps_flag;
			is >> submaps_flag;
			os << submaps_flag;

			unsigned int submaps = 1;
			if (submaps_flag)
			{
				BitUint<4> submaps_less1;

				is >> submaps_less1;
				submaps = submaps_less1 + 1;
				os << submaps_less1;
			}

			BitUint<1> square_polar_flag;
			is >> square_polar_flag;
			os << square_polar_flag;

			if (square_polar_flag)
			{
				BitUint<8> coupling_steps_less1;
				is >> coupling_steps_less1;
				unsigned int coupling_steps = coupling_steps_less1 + 1;
				os << coupling_steps_less1;

				for (unsigned int j = 0; j < coupling_steps; j++)
				{
					BitUintRuntime magnitude(ilog(channels - 1)), angle(ilog(channels - 1));

					is >> magnitude >> angle;
					os << magnitude << angle;

					if (angle == magnitude || magnitude >= channels || angle >= channels)
						throw std::runtime_error("invalid coupling");
				}
			}

			// a rare reserved field not removed by Ak!
			BitUint<2> mapping_reserved;
			is >> mapping_reserved;
			os << mapping_reserved;
			if (0 != mapping_reserved)
				throw std::runtime_error("mapping reserved field nonzero");

			if (submaps > 1)
			{
				for (unsigned int j = 0; j < channels; j++)
				{
					BitUint<4> mapping_mux;
					is >> mapping_mux;
					os << mapping_mux;

					if (mapping_mux >= submaps)
						throw std::runtime_error("mapping_mux >= submaps");
				}
			}

			for (unsigned int j = 0; j < submaps; j++)
			{
				// Another! Unused time domain transform configuration placeholder!
				BitUint<8> time_config;
				is >> time_config;
				os << time_config;

				BitUint<8> floor_number;
				is >> floor_number;
				os << floor_number;
				if (floor_number >= floor_count)
					throw std::runtime_error("invalid floor mapping");

				BitUint<8> residue_number;
				is >> residue_number;
				os << residue_number;
				if (residue_number >= residue_count)
					throw std::runtime_error("invalid residue mapping");
			}
		}

		// mode count
		BitUint<6> mode_count_less1;
		is >> mode_count_less1;
		unsigned int mode_count = mode_count_less1 + 1;
		os << mode_count_less1;

		modeBlockflag = new bool[mode_count];
		modeBits = ilog(mode_count - 1);

		//cout << mode_count << " modes" << endl;

		for (unsigned int i = 0; i < mode_count; i++)
		{
			BitUint<1> block_flag;
			is >> block_flag;
			os << block_flag;

			modeBlockflag[i] = (block_flag != 0);

			// only 0 valid for windowtype and transformtype
			BitUint<16> windowtype(0), transformtype(0);
			os << windowtype << transformtype;

			BitUint<8> mapping;
			is >> mapping;
			os << mapping;
			if (mapping >= mapping_count)
				throw std::runtime_error("invalid mode mapping");
		}

		BitUint<1> framing(1);
		os << framing;

		// not_full_setup

		os.flushPage();

	}
}


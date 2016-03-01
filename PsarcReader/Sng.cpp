#include <vector>
#include <cassert>
#include <stdexcept>
#include "StreamReader.h"
#include "MemoryReader.h"
#include "Rijndael/rijndael.h"
#include "zlib/zlib.h"
#include "Keys.h"
#include "Sng.h"

int SngReader::readTo(std::vector<char>& input, std::vector<char>& storage)
{
	MemoryReaderLE SourceReader(input);
	uint32_t head = SourceReader.readUint32();
	if (0x4A != head) // pc file -> little endian, xbox file -> big endian
		throw std::runtime_error("SNG: PC header not found.\n");

	size_t fileSize = input.size();
	SourceReader.setPos(headerSize);

	char iv[blockSize];
	SourceReader.readBytes(iv, blockSize);
	int cipherSize = fileSize - headerSize - blockSize;
	char* plainText = new char[cipherSize];

	uint32_t dataSize;
	bool decryptSuccess = false;
	for (size_t i = 0; !decryptSuccess && i < keys::sngCount; i++) // try both pc and mac keys
	{
		SourceReader.setPos(headerSize + blockSize);
		SourceReader.readBytes(plainText, cipherSize);
		decrypt(plainText, cipherSize, iv, keys::sng[i]);

		MemoryReaderLE PlaintextReader(plainText);
		dataSize = PlaintextReader.readUint32();
		uint16_t xU = PlaintextReader.readUint16();
		if (xU == 0xDA78 || xU == 0x78DA) // RFC 1950 CMF+FLG
			decryptSuccess = true;
	}

	if (!decryptSuccess)
		throw std::runtime_error("SNG: zlib header not found.\n");

	storage.reserve(dataSize);
	int ret = inflateBytes(plainText + 4, cipherSize - 4, storage);
	if (ret != Z_OK)
		throw std::runtime_error("SNG: inflate fail\n");

	delete [] plainText;
}

int SngReader::inflateBytes(char* source, int srcLen, std::vector<char>& dest)
{
#define CHUNK 65536

	int ret;
	unsigned have;
	z_stream strm;
	unsigned char out[CHUNK];

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return ret;

	do {
		strm.avail_in = srcLen;
		if (strm.avail_in == 0)
			break;
		strm.next_in = (Bytef*)source;

		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
			}
			have = CHUNK - strm.avail_out;
			dest.insert(dest.end(), out, out + have);
		} while (strm.avail_out == 0);

	} while (ret != Z_STREAM_END);

	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

void SngReader::decrypt(char* output, long len, char* iv, const unsigned char* key)
{
	const int keyBits = 256;
	const int blockSize = 16;

	unsigned long rk[RKLENGTH(keyBits)];
	int nrounds = rijndaelSetupEncrypt(rk, key, keyBits);

	int i;
	char* p;
	char chain[blockSize];
	char temp[blockSize];

	for (size_t i = 0; i < blockSize; i++)
		chain[i] = iv[i];

	// inplace CTR decryption
	for (i = 0, p = output; i < len / blockSize; i++)
	{
		rijndaelEncrypt(rk, nrounds, (unsigned char*)chain, (unsigned char*)temp);
		blockXor(p, temp);
		blockIncrement(chain);
		p += blockSize;
	}
	int leftover = len - i * blockSize;
	rijndaelEncrypt(rk, nrounds, (unsigned char*)chain, (unsigned char*)temp);
	blockXor(p, temp, leftover);
}

void SngReader::blockXor(char* out, char const* buf, int blockSize)
{
	for (int i = 0; i < blockSize; i++)
		*(out++) ^= *(buf++);
}

void SngReader::blockIncrement(char* out, int blockSize)
{
	for (int i = blockSize - 1, carry = true; i >= 0 && carry; i--)
		carry = ((out[i] = (uint8_t)(out[i] + 1)) == 0);
}

// RocksmithToolkitLib\Sng\Sng2014File.cs ReadSng() -> Read()
void Sng::parse(std::vector<char>& storage)
{
	membuf m(storage.data(), storage.data() + storage.size());
	std::istream s(&m);
	StreamReaderLE r(s);

	//std::cout << r.pos << std::endl;

	BPMs.resize(r.readUint32());
	for (size_t i = 0; i < BPMs.size(); i++)
	{
		BPMs[i] = 
		{
			r.readSingle(),
			r.readInt16(),
			r.readInt16(),
			r.readInt32(),
			r.readInt32()
		};
	}

	//std::cout << "BPMs " << r.pos << std::endl;

	Phrases.resize(r.readUint32());
	for (size_t i = 0; i < Phrases.size(); i++)
	{
		Phrases[i] = 
		{
			r.readInt8(),
			r.readInt8(),
			r.readInt8(),
			r.readInt8(),
			r.readInt32(),
			r.readInt32()
		};
		r.readBytes(Phrases[i].name, 32);
	}
	
	//std::cout << "Phrases " << r.pos << std::endl;

	Chords.resize(r.readUint32());
	for (size_t i = 0; i < Chords.size(); i++)
	{
		Chords[i].mask = r.readUint32();
		r.readBytes(Chords[i].frets, 6);
		r.readBytes(Chords[i].fingers, 6);
		for (size_t j = 0; j < 6; j++)
			Chords[i].notes[j] = r.readInt32();
		r.readBytes(Chords[i].name, 32);
	}

	//std::cout << "Chords " << r.pos << std::endl;

	ChordNotes.resize(r.readUint32());
	for (size_t i = 0; i < ChordNotes.size(); i++)
	{
		for (size_t j = 0; j < 6; j++)
			ChordNotes[i].noteMask[j] = r.readUint32();
		for (size_t j = 0; j < 6; j++)
		{
			for (size_t k = 0; k < 32; k++)
				ChordNotes[i].bendData[j].bendSteps[k] = {
					r.readSingle(),
					r.readSingle(),
					r.readInt16(),
					r.readInt8(),
					r.readInt8()
				};
			ChordNotes[i].bendData[j].usedCount = r.readInt32();
		}		
		r.readBytes(ChordNotes[i].slideTo, 6);
		r.readBytes(ChordNotes[i].slideUnpitchTo, 6);
		for (size_t j = 0; j < 6; j++)
			ChordNotes[i].vibrato[j] = r.readInt16();
	}
	
	//std::cout << "ChordNotes " << r.pos << std::endl;

	Vocals.resize(r.readUint32());
	for (size_t i = 0; i < Vocals.size(); i++)
	{
		Vocals[i] = 
		{
			r.readSingle(),
			r.readInt32(),
			r.readSingle()
		};
		r.readBytes(Vocals[i].lyric, 48);
	}

	//std::cout << "Vocals " << r.pos << std::endl;

	if (Vocals.size() > 0)
	{
		SymbolsHeaders.resize(r.readUint32());
		for (size_t i = 0; i < SymbolsHeaders.size(); i++)
		{
			for (size_t j = 0; j < 8; j++)
				SymbolsHeaders[i].unk4[j] = r.readInt32();
		}

		SymbolsTextures.resize(r.readUint32());
		for (size_t i = 0; i < SymbolsTextures.size(); i++)
		{
			r.readBytes(SymbolsTextures[i].font, 128);
			SymbolsTextures[i].fontPathLength = r.readInt32();
			SymbolsTextures[i].unk5 = r.readInt32();
			SymbolsTextures[i].width = r.readInt32();
			SymbolsTextures[i].height = r.readInt32();
		}

		SymbolDefinitions.resize(r.readUint32());
		for (size_t i = 0; i < SymbolDefinitions.size(); i++)
		{
			r.readBytes(SymbolDefinitions[i].text, 12);
			SymbolDefinitions[i].outer.yMin = r.readSingle();
			SymbolDefinitions[i].outer.xMin = r.readSingle();
			SymbolDefinitions[i].outer.yMax = r.readSingle();
			SymbolDefinitions[i].outer.xMax = r.readSingle();
			SymbolDefinitions[i].inner.yMin = r.readSingle();
			SymbolDefinitions[i].inner.xMin = r.readSingle();
			SymbolDefinitions[i].inner.yMax = r.readSingle();
			SymbolDefinitions[i].inner.xMax = r.readSingle();
		}
	}

	//std::cout << "Symbols " << r.pos << std::endl;
	
	PhraseIterations.resize(r.readUint32());
	for (size_t i = 0; i < PhraseIterations.size(); i++)
	{
		PhraseIterations[i] = 
		{
			r.readInt32(),
			r.readSingle(),
			r.readSingle()
		};
		for (size_t j = 0; j < 3; j++)
			PhraseIterations[i].difficulty[j] = r.readInt32();
	}

	//std::cout << "PhraseIterations " << r.pos << std::endl;

	PhraseExtraInfos.resize(r.readUint32());
	for (size_t i = 0; i < PhraseExtraInfos.size(); i++)
	{
		PhraseExtraInfos[i] =
		{
			r.readInt32(),
			r.readInt32(),
			r.readInt32(),
			r.readInt8(),
			r.readInt16(),
			r.readInt8()
		};
	}

	//std::cout << "PhraseExtraInfos " << r.pos << std::endl;

	NLinkedDifficulties.resize(r.readUint32());
	for (size_t i = 0; i < NLinkedDifficulties.size(); i++)
	{
		NLinkedDifficulties[i].levelBreak = r.readInt32();
		NLinkedDifficulties[i].phraseCount = r.readInt32();
		for (size_t j = 0; j < NLinkedDifficulties[i].phraseCount; j++)
			NLinkedDifficulties[i].NLD_phrase.push_back(r.readInt32());
	}

	//std::cout << "NLinkedDifficulties " << r.pos << std::endl;

	Actions.resize(r.readUint32());
	for (size_t i = 0; i < Actions.size(); i++)
	{
		Actions[i].time = r.readSingle();
		r.readBytes(Actions[i].name, 256);
	}

	//std::cout << "Actions " << r.pos << std::endl;

	Events.resize(r.readUint32());
	for (size_t i = 0; i < Events.size(); i++)
	{
		Events[i].time = r.readSingle();
		r.readBytes(Events[i].name, 256);
	}

	//std::cout << "Events " << r.pos << std::endl;

	Tones.resize(r.readUint32());
	for (size_t i = 0; i < Tones.size(); i++)
	{
		Tones[i].time = r.readSingle();
		Tones[i].id = r.readUint32();
	}

	//std::cout << "Tones " << r.pos << std::endl;

	DNAs.resize(r.readUint32());
	for (size_t i = 0; i < DNAs.size(); i++)
	{
		DNAs[i].time = r.readSingle();
		DNAs[i].id = r.readUint32();
	}

	//std::cout << "DNAs " << r.pos << std::endl;

	Sections.resize(r.readUint32());
	for (size_t i = 0; i < Sections.size(); i++)
	{
		r.readBytes(Sections[i].name, 32);
		Sections[i].number = r.readUint32();
		Sections[i].startTime = r.readSingle();
		Sections[i].endTime = r.readSingle();
		Sections[i].startPhraseIterationId = r.readUint32();
		Sections[i].endPhraseIterationId = r.readUint32();
		r.readBytes(Sections[i].stringMask, 36);
	}

	//std::cout << "Sections " << r.pos << std::endl;

	Arrangements.resize(r.readUint32());
	for (size_t i = 0; i < Arrangements.size(); i++)
	{
		Arrangements[i].difficulty = r.readInt32();

		Arrangements[i].Anchors.resize(r.readUint32());
		for (size_t j = 0; j < Arrangements[i].Anchors.size(); j++)
		{
			Arrangements[i].Anchors[j] =
			{
				r.readSingle(),
				r.readSingle(),
				r.readSingle(),
				r.readSingle(),
				r.readInt8()
			};
			r.readBytes(Arrangements[i].Anchors[j].padding, 3);
			Arrangements[i].Anchors[j].width = r.readInt32();
			Arrangements[i].Anchors[j].phraseIterationId = r.readInt32();
		}

		//std::cout << "-Anchors " << r.pos << std::endl;
		
		Arrangements[i].AnchorExtensions.resize(r.readUint32());
		for (size_t j = 0; j < Arrangements[i].AnchorExtensions.size(); j++)
		{
			Arrangements[i].AnchorExtensions[j] =
			{
				r.readSingle(),
				r.readInt8(),
				r.readInt32(),
				r.readInt16(),
				r.readInt8()
			};
		}

		//std::cout << "-AnchorExtensions " << r.pos << std::endl;

		Arrangements[i].Fingerprints1.resize(r.readUint32());
		for (size_t j = 0; j < Arrangements[i].Fingerprints1.size(); j++)
		{
			Arrangements[i].Fingerprints1[j] =
			{
				r.readInt32(),
				r.readSingle(),
				r.readSingle(),
				r.readSingle(),
				r.readSingle()
			};
		}

		//std::cout << "-Fingerprints1 " << r.pos << std::endl;

		Arrangements[i].Fingerprints2.resize(r.readUint32());
		for (size_t j = 0; j < Arrangements[i].Fingerprints2.size(); j++)
		{
			Arrangements[i].Fingerprints2[j] =
			{
				r.readInt32(),
				r.readSingle(),
				r.readSingle(),
				r.readSingle(),
				r.readSingle()
			};
		}

		//std::cout << "-Fingerprints2 " << r.pos << std::endl;

		Arrangements[i].Notes.resize(r.readUint32());
		for (size_t j = 0; j < Arrangements[i].Notes.size(); j++)
		{
			Arrangements[i].Notes[j] =
			{
				r.readUint32(),
				r.readUint32(),
				r.readUint32(),
				r.readSingle(),
				r.readInt8(),
				r.readInt8(),
				r.readInt8(),
				r.readInt8(),
				r.readInt32(),
				r.readInt32(),
				r.readInt32(),
				r.readInt32(),
				r.readInt16(),
				r.readInt16(),
				r.readInt16(),
				r.readInt16(),
				r.readInt16(),
				r.readInt8(),
				r.readInt8(),
				r.readInt8(),
				r.readInt8(),
				r.readInt8(),
				r.readInt8(),
				r.readInt8(),
				r.readInt16(),
				r.readSingle(),
				r.readSingle()
			};
			Arrangements[i].Notes[j].bendData.resize(r.readUint32());
			for (size_t k = 0; k < Arrangements[i].Notes[j].bendData.size(); k++)
			{
				Arrangements[i].Notes[j].bendData[k] =
				{
					r.readSingle(),
					r.readSingle(),
					r.readInt16(),
					r.readInt8(),
					r.readInt8()
				};
			}
		}

		//std::cout << "-Notes " << r.pos << std::endl;

		Arrangements[i].phraseCount = r.readUint32();
		Arrangements[i].AverageNotesPerIteration.resize(Arrangements[i].phraseCount);
		for (size_t j = 0; j < Arrangements[i].AverageNotesPerIteration.size(); j++)
			Arrangements[i].AverageNotesPerIteration[j] = r.readSingle();

		Arrangements[i].phraseIterationCount1 = r.readUint32();
		Arrangements[i].NotesInIteration1.resize(Arrangements[i].phraseIterationCount1);
		for (size_t j = 0; j < Arrangements[i].NotesInIteration1.size(); j++)
			Arrangements[i].NotesInIteration1[j] = r.readUint32();

		Arrangements[i].phraseIterationCount2 = r.readUint32();
		Arrangements[i].NotesInIteration2.resize(Arrangements[i].phraseIterationCount2);
		for (size_t j = 0; j < Arrangements[i].NotesInIteration2.size(); j++)
			Arrangements[i].NotesInIteration2[j] = r.readUint32();
	}

	//std::cout << "Arrangements " << r.pos << std::endl;

	metadata = 
	{
		r.readDouble(),
		r.readDouble(),
		r.readDouble(),
		r.readDouble(),
		r.readSingle(),
		r.readSingle(),
		r.readInt8(),
	};
	r.readBytes(metadata.lastConversionDateTime, 32);
	metadata.part = r.readInt16();
	metadata.songLength = r.readSingle();
	metadata.stringCount = r.readInt32();
	metadata.tuning.resize(metadata.stringCount);
	for (size_t i = 0; i < metadata.tuning.size(); i++)
		metadata.tuning[i] = r.readInt16();
	metadata.unk1_firstNoteTime = r.readSingle();
	metadata.unk2_firstNoteTime = r.readSingle();
	metadata.maxDifficulty = r.readInt32();

	//std::cout << "Metadata " << r.pos << std::endl;
}

Sng::~Sng()
{

}
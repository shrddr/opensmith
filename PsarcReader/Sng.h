#pragma once
#include <iostream>
#include <fstream>

#define NOTE_MASK_CHORD 0x00000002
#define NOTE_MASK_OPEN 0x00000004
#define NOTE_MASK_FRETHANDMUTE 0x00000008
#define NOTE_MASK_TREMOLO 0x00000010
#define NOTE_MASK_HARMONIC 0x00000020
#define NOTE_MASK_PALMMUTE 0x00000040
#define NOTE_MASK_SLAP 0x00000080
#define NOTE_MASK_PLUCK 0x00000100
#define NOTE_MASK_POP 0x00000100
#define NOTE_MASK_HAMMERON 0x00000200
#define NOTE_MASK_PULLOFF 0x00000400
#define NOTE_MASK_SLIDE 0x00000800
#define NOTE_MASK_BEND 0x00001000
#define NOTE_MASK_SUSTAIN 0x00002000
#define NOTE_MASK_TAP 0x00004000
#define NOTE_MASK_PINCHHARMONIC 0x00008000
#define NOTE_MASK_VIBRATO 0x00010000
#define NOTE_MASK_MUTE 0x00020000
#define NOTE_MASK_IGNORE 0x00040000
#define NOTE_MASK_LEFTHAND 0x00080000
#define NOTE_MASK_RIGHTHAND 0x00100000
#define NOTE_MASK_HIGHDENSITY 0x00200000
#define NOTE_MASK_SLIDEUNPITCHEDTO 0x00400000
#define NOTE_MASK_SINGLE 0x00800000
#define NOTE_MASK_CHORDNOTES 0x01000000
#define NOTE_MASK_DOUBLESTOP 0x02000000
#define NOTE_MASK_ACCENT 0x04000000
#define NOTE_MASK_PARENT 0x08000000
#define NOTE_MASK_CHILD 0x10000000
#define NOTE_MASK_ARPEGGIO 0x20000000
#define NOTE_MASK_STRUM 0x80000000

namespace SngReader
{
	const int headerSize = 8;
	const int blockSize = 16;

	void readTo(std::vector<char>& input, std::vector<char>& storage);
	int inflateBytes(char* source, int srcLen, std::vector<char>& dest);

	//move out to crypto.h
	void decrypt(char* output, long len, char* iv, const unsigned char* key);
	void blockIncrement(char* out, int blockSize = 16);
	void blockXor(char* out, char const* buf, int blockSize = 16);
}

class Sng
{
public:
	Sng() {};
	void parse(std::vector<char>& storage);
	~Sng();

	struct BendStep
	{
		float time;
		float step;
		int16_t unk1;
		int8_t unk2;
		int8_t unk3;
	};

	struct BendData
	{
		BendStep bendSteps[32];
		int32_t usedCount;
	};

	struct Rect
	{
		float yMin;
		float xMin;
		float yMax;
		float xMax;
	};

	struct Anchor
	{
		float startBeatTime;
		float endBeatTime;
		float firstNoteTime;
		float lastNoteTime;
		char padding[3];
		int32_t width;
		int32_t phraseIterationId;
	};

	struct AnchorExtension
	{
		float beatTime;
		char fretId;
		int32_t unk1;
		int16_t unk2;
		int8_t unk3;
	};

	struct Fingerprint
	{
		int32_t chordId;
		float startTime;
		float endTime;
		float firstNoteTime;
		float lastNoteTime;
	};

	struct Note
	{
		uint32_t noteMask;
		uint32_t noteFlags;
		uint32_t hash;
		float time;
		char stringIndex;
		char fretId;
		char anchorFretId;
		char anchorWidth;
		int32_t chordId;
		int32_t chordNotesId;
		int32_t phraseId;
		int32_t phraseIterationId;
		int16_t fingerprint1;
		int16_t fingerprint2;
		int16_t nextIterNote;
		int16_t prevIterNote;
		int16_t parentPrevNote;
		char slideTo;
		char slideUnpitchTo;
		char leftHand;
		char tap;
		char pickDirection;
		char slap;
		char pluck;
		int16_t vibrato = 0;
		float sustain;
		float maxBend;
		std::vector<BendData> bendData;
	};

	/////

	struct BPM
	{
		float time;
		int16_t measure;
		int16_t beat;
		int32_t phraseIteration;
		int32_t mask;
	};

	struct Phrase
	{
		int8_t solo;
		int8_t disparity;
		int8_t ignore;
		int8_t padding;
		int32_t maxDifficulty;
		int32_t phraseIterationLinks;
		char name[32];
	};

	struct Chord
	{
		uint32_t mask;
		char frets[6];
		char fingers[6];
		int32_t notes[6];
		char name[32];
	};

	struct ChordNote
	{
		uint32_t noteMask[6];
		BendData bendData[6];
		char slideTo[6];
		char slideUnpitchTo[6];
		int16_t vibrato[6];
	};

	struct ChordNotes
	{
		uint32_t noteMask[6];
		BendData bendData[6];
		char slideTo[6];
		char slideUnpitchTo[6];
		int16_t vibrato;
	};

	struct Vocal
	{
		float time;
		int32_t note;
		float length;
		char lyric[48];
	};

	struct SymbolsHeader
	{
		int32_t unk4[8];
	};

	struct SymbolsTexture
	{
		char font[128];
		int32_t fontPathLength;
		int32_t unk5;
		int32_t width;
		int32_t height;
	};

	struct SymbolDefinition
	{
		char text[12];
		Rect outer;
		Rect inner;
	};

	struct PhraseIteration
	{
		int32_t phraseId;
		float startTime;
		float nextPhraseTime;
		int32_t difficulty[3];
	};

	struct PhraseExtraInfo
	{
		int32_t phraseId;
		int32_t difficulty;
		int32_t empty;
		int8_t levelJump;
		int32_t redundant;
		int8_t padding;
	};

	struct NLinkedDifficulty
	{
		int32_t levelBreak;
		int32_t phraseCount;
		std::vector<int32_t> NLD_phrase;
	};

	struct Action
	{
		float time;
		char name[256];
	};

	struct Event
	{
		float time;
		char name[256];
	};

	struct Tone
	{
		float time;
		uint32_t id;
	};

	struct DNA
	{
		float time;
		uint32_t id;
	};

	struct Section
	{
		char name[32];
		uint32_t number;
		float startTime;
		float endTime;
		uint32_t startPhraseIterationId;
		uint32_t endPhraseIterationId;
		char stringMask[36];
	};

	struct Arrangement
	{
		int32_t difficulty;
		std::vector<Anchor> Anchors;
		std::vector<AnchorExtension> AnchorExtensions;
		std::vector<Fingerprint> Fingerprints1;
		std::vector<Fingerprint> Fingerprints2;
		std::vector<Note> Notes;
		uint32_t phraseCount;
		std::vector<float> AverageNotesPerIteration;
		uint32_t phraseIterationCount1;
		std::vector<uint32_t> NotesInIteration1;
		uint32_t phraseIterationCount2;
		std::vector<uint32_t> NotesInIteration2;
	};

	struct Metadata
	{
		double maxScore{};
		double maxNotesAndChords{};
		double maxNotesAndChords_Real{};
		double pointsPerNote{};
		float firstBeatLength{};
		float startTime{};
		char capoFretId{};
		char lastConversionDateTime[32]{};
		int16_t part{};
		float songLength{};
		int32_t stringCount {};
		std::vector<int> tuning;
		float unk1_firstNoteTime{};
		float unk2_firstNoteTime{};
		int32_t maxDifficulty{};
	};

	std::vector<BPM> BPMs;
	std::vector<Phrase> Phrases;
	std::vector<Chord> Chords;
	std::vector<ChordNote> ChordNotes;
	std::vector<Vocal> Vocals;
	std::vector<SymbolsHeader> SymbolsHeaders;
	std::vector<SymbolsTexture> SymbolsTextures;
	std::vector<SymbolDefinition> SymbolDefinitions;
	std::vector<PhraseIteration> PhraseIterations;
	std::vector<PhraseExtraInfo> PhraseExtraInfos;
	std::vector<NLinkedDifficulty> NLinkedDifficulties;
	std::vector<Action> Actions;
	std::vector<Event> Events;
	std::vector<Tone> Tones;
	std::vector<DNA> DNAs;
	std::vector<Section> Sections;
	std::vector<Arrangement> Arrangements;
	Metadata metadata;
};

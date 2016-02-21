#pragma once
#include "Audio.h"
#include "CircularBuffer.h"
#include <fstream>

/* MIDI note codes:
standard guitar tuning EADGBE = 40 45 50 55 59 64..88
standard bass tuning   EADG   = 28 33 38 43..67 */

/* note frequencies:
E 330 349 370 392 415
B 247 261 277 293 311
G 196 208 220 233 246
D 147 156 165 175 185
A 110 117 123 131 139
E 082 087 092 098 104
   0   1   2   3   4 */

#define MIDI_A4 69
#define FREQ_A4 440.0f
#define NOTE_DISTANCE 1.059463094359
#define PI 3.14159265358979323846

const int tuning[] = { 40, 45, 50, 55, 59, 64 };
const size_t noteFirst = tuning[0] - 1; // edge note to compare the lowest E
const size_t noteCount = 50; // 4 octaves + 2 edge notes
const size_t inputSize = 3200;

class NoteDetector : public AudioConsumer
{
public:
	NoteDetector(size_t sampleRate);
	void setPCM(const float in);
	float rms();
	void analyze(float(&)[144]);
	bool confirm(int string, int fret, float time);
	~NoteDetector();
private:
	size_t sampleRate;
	CircularBuffer<float> samples;

	float noteFreq[noteCount];
	float noteSin[noteCount][inputSize];
	float noteCos[noteCount][inputSize];
	
	float powers[noteCount];
	float deltas[noteCount];
	bool up[noteCount];

	std::ofstream log;
};


#pragma once
#include <vector>
#include "GameState.h"
#include "Audio/Audio.h"

class TunerDetector : public AudioInputBuffer
{
public:
	TunerDetector(size_t sampleRate, size_t bufferSize, size_t bufferCount);
	void prepare(int note);
	float analyze();
private:
	size_t sampleRate;
	size_t inputSize;
	float frequency;
	static const int cents[];
	static const int centsCount;
	std::vector<float> sinTables;
	std::vector<float> cosTables;
	void updateTable(size_t table, float freq);
	CircularBuffer<float> results;
};

class Tuner : public GameState
{
public:
	Tuner(std::vector<int> tuning, bool returnToMenu = false);
	void Tuner::keyPressed(int key);
	void Tuner::draw(double time);
	~Tuner();
private:
	static const size_t sampleRate = 48000;
	std::vector<int> notes;
	std::vector<int>::iterator note;

	bool hit;
	double hitStart;
	void nextNote();

	std::vector<float> vertices;
	GLuint vertexArrayId;
	GLuint vertexBufferId;
	GLuint programId;

	Text2D text;

	Silence s;
	TunerDetector d;
	InOut io{ &s, &d, 1, 1 };
	Audio* a;

	bool returnToMenu;
};


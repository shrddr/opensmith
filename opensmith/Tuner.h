#pragma once
#include <vector>
#include "GameState.h"
#include "Audio/Audio.h"
#include "Logger.h"

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

	Logger L;
};

class Tuner : public GameState
{
public:
	Tuner(std::vector<int> tuning, bool returnToMenu = false);
	void keyPressed(int key);
	void draw(double time);
	~Tuner();
private:
	std::vector<Sprite*> sprites;
	void drawBackground();
	const float neckHeight = 200.0f;

	static const size_t sampleRate = 48000;
	std::vector<int> notes;
	size_t currentNote;

	bool hit;
	double hitStart;
	void nextNote();

	const float needleLength = 400.0f;
	std::vector<float> gaugePositions;
	std::vector<float> gaugeVertices;
	GLuint vertexArrayId;
	GLuint vertexBufferId;
	GLuint programId;
	GLuint uniformTintId;

	Text2D text;

	Silence s;
	TunerDetector d;
	InOut io{ &s, &d, 1, 1 };
	Audio* a;

	bool returnToMenu;
};


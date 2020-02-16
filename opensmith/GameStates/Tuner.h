#pragma once
#include <vector>
#include "GameState.h"
#include "Audio/Audio.h"
#include "../Logger.h"

class TunerDetector : public AudioInputBuffer
{
public:
	TunerDetector();
	void prepare(float targetFrequency);
	void analyze();
	float result();
private:
	enum { lookup, autocorr } method;
	CircularBuffer<float> history;

	void analyzeLookup();
	float targetFrequency;
	std::vector<float> offsets;
	std::vector<float> sinTables;
	std::vector<float> cosTables;
	void updateTable(size_t table, float freq);

	void analyzeAuto();
	int minPeriod;
	int maxPeriod;
};

class Tuner : public GameState
{
public:
	Tuner(std::vector<int> tuning, bool returnToMenu = false);
	void keyPressed(int key);
	void draw(double time);
	~Tuner();
private:
	SpriteSet sprites;
	std::pair<size_t, size_t> stringSprites;
	void drawBackground();
	const float neckHeight = 200.0f;

	std::vector<int> notes;
	size_t currentNote;
	float targetFrequency;

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


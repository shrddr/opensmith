#include <algorithm>
#include <GL/glew.h>
#include "Tuner.h"
#include "Menu.h"
#include "util.h"
#include "Settings/Settings.h"
#include "Audio/notes.h"

Tuner::Tuner(std::vector<int> tuning, bool returnToMenu):
	notes(tuning),
	d(sampleRate, 3200, 10),
	text("../resources/textures/text_Inconsolata29.dds"),
	hit(false),
	returnToMenu(returnToMenu)
{
	note = notes.begin();
	d.prepare(*note);

	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(vertexArrayId);
	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	vertices.push_back(960);
	vertices.push_back(540);
	vertices.push_back(960 + 400);
	vertices.push_back(540);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	programId = loadShaders("../resources/shaders/gauge.vs",
		"../resources/shaders/gauge.fs");

	o.load();
	a = new Audio(io, sampleRate);
	a->start();
}

Tuner::~Tuner()
{
	a->stop();
	delete a;
	glDeleteProgram(programId);
	glDeleteBuffers(1, &vertexBufferId);
	glDeleteVertexArrays(1, &vertexArrayId);
}

void Tuner::nextNote()
{
	++note;
	if (note == notes.end())
	{
		o.lastTuning = notes;

		bool returnToMenuSafe = returnToMenu;

		delete gameState;

		// can't use Tuner members now
		if (returnToMenuSafe)
			gameState = new MainMenu;
		else
			gameState = new Session;
		return;
	}
	d.prepare(*note);
}

void Tuner::keyPressed(int key)
{
	if (key == GLFW_KEY_ESCAPE)
	{
		delete gameState;
		gameState = new MainMenu;
	}
}

void Tuner::draw(double time)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	text.print(noteNames[*note % 12].c_str(), 960 - 140, 540 - 16, 32);

	float cents = d.analyze();
	char textBuf[64];
	sprintf(textBuf, "%.0f", cents);
	text.print(textBuf, 960 - 100, 540 - 16, 32);
	
	glUseProgram(programId);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	// clamp to -50 .. +50
	float clamped = std::min(std::max(cents, -50.0f), 50.0f);
	// scale to -45° .. +45°
	float angle = clamped / 50.0f * PI / 4;
	vertices[2] = 960 + 400 * cos(angle);
	vertices[3] = 540 + 400 * sin(angle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_LINES, 0, 2);
	glDisableVertexAttribArray(0);

	if (!hit && fabs(cents) < o.precisionCentDelta)
	{
		hit = true;
		hitStart = time;
	}
	if (hit && fabs(cents) > o.precisionCentDelta)
	{
		hit = false;
	}
	if (hit && time - hitStart > o.precisionHoldTime)
	{
		nextNote();
	}
}

const int TunerDetector::cents[] = { -500, -200, -100, -50, -20, -10, -5, -2, 0, 2, 5, 10, 20, 50, 100, 200, 500 };
const int TunerDetector::centsCount = sizeof(cents) / sizeof(cents[0]);

TunerDetector::TunerDetector(size_t sampleRate, size_t bufferSize, size_t bufferCount) :
	sampleRate(sampleRate),
	inputSize(bufferSize),
	AudioInputBuffer(bufferSize),
	results(bufferCount)
{
	sinTables.resize(centsCount * bufferSize);
	cosTables.resize(centsCount * bufferSize);
}

void TunerDetector::prepare(int note)
{
	frequency = 440 * pow(2.0, (note - 69) / 12.0);

	for (size_t table = 0; table < centsCount; table++)
	{
		float stepFrequency = frequency * pow(2, (cents[table] / 1200.0f));
		updateTable(table, stepFrequency);
	}

	for (size_t i = 0; i < results.size(); i++)
		results.push_back(-512.0f);
}

void TunerDetector::updateTable(size_t table, float freq)
{
	for (size_t sample = 0; sample < inputSize; sample++)
	{
		sinTables[table * inputSize + sample] = sin(2 * PI * freq / sampleRate * sample) / sqrt(inputSize);
		cosTables[table * inputSize + sample] = cos(2 * PI * freq / sampleRate * sample) / sqrt(inputSize);
	}
}

float TunerDetector::analyze()
{
	// TODO: don't analyze if the RMS is too low
	size_t maxTable = 0;
	float maxPower = 0;
	std::vector<float> powers;

	for (size_t table = 0; table < centsCount; table++)
	{
		float amplitudeSin = 0;
		float amplitudeCos = 0;
		for (size_t sample = 0; sample < inputSize; sample++)
		{
			amplitudeSin += buf[sample] * sinTables[table * inputSize + sample];
			amplitudeCos += buf[sample] * cosTables[table * inputSize + sample];
		}
		float power = sqrt(amplitudeSin * amplitudeSin + amplitudeCos * amplitudeCos);
		powers.push_back(power);

		if (power > maxPower)
		{
			maxPower = power;
			maxTable = table;
		}
	}

	float result = cents[maxTable];
	results.push_back(result);

	float average = 0;
	for (size_t i = 0; i < results.size(); i++)
		average += results[i];

	average /= results.size();

	return average;
}


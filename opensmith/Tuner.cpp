#include <algorithm>
#include <GL/glew.h>
#include "Tuner.h"
#include "Menu.h"
#include "util.h"
#include "Settings/Settings.h"
#include "Audio/notes.h"

Tuner::Tuner(std::vector<int> tuning, bool returnToMenu):
	notes(tuning),
	currentNote(0),
	d(sampleRate, 3200, 10),
	text("../resources/textures/text_Inconsolata29.dds"),
	hit(false),
	returnToMenu(returnToMenu)
{
	d.prepare(notes[currentNote]);

	float x = 700;
	float y = 540 + 0.5 * neckHeight;
	float w = 1000;
	float h = 2;
	size_t stringCount = tuning.size();
	float stringSpacing = neckHeight / (stringCount - 1);

	for (size_t stringId = 0; stringId < stringCount; stringId++)
	{
		gaugePositions.push_back(y);
		sprites.push_back(new Sprite(
			x - h / 2,
			y,
			w,
			h,
			glm::vec3(
				o.stringColors[3 * stringId],
				o.stringColors[3 * stringId + 1],
				o.stringColors[3 * stringId + 2])
		));
		y -= stringSpacing;
	}

	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(vertexArrayId);
	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	gaugeVertices.push_back(960);
	gaugeVertices.push_back(gaugePositions[currentNote]);
	gaugeVertices.push_back(960 + needleLength);
	gaugeVertices.push_back(gaugePositions[currentNote]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * gaugeVertices.size(), gaugeVertices.data(), GL_STATIC_DRAW);
	programId = loadShaders("../resources/shaders/gauge.vs",
		"../resources/shaders/gauge.fs");
	uniformTintId = glGetUniformLocation(programId, "tint");

	o.load();
	a = new Audio(io, sampleRate);
	a->start();
}

Tuner::~Tuner()
{
	for (auto sprite : sprites) delete sprite;
	Sprite::clear();
	a->stop();
	delete a;
	glDeleteProgram(programId);
	glDeleteBuffers(1, &vertexBufferId);
	glDeleteVertexArrays(1, &vertexArrayId);
}

void Tuner::nextNote()
{
	if (++currentNote == notes.size())
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
	gaugeVertices[1] = gaugePositions[currentNote];
	d.prepare(notes[currentNote]);
}

void Tuner::keyPressed(int key)
{
	if (key == GLFW_KEY_ESCAPE)
	{
		delete gameState;
		gameState = new MainMenu;
	}
}

void Tuner::drawBackground()
{
	glBufferData(GL_ARRAY_BUFFER, Sprite::getSize() * sizeof(glm::vec2), Sprite::getVertices(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	for (auto sprite : sprites)
	{
		glUniform3f(uniformTintId, sprite->tint.r, sprite->tint.g, sprite->tint.b);
		glDrawArrays(GL_TRIANGLES, sprite->getOffset(), sprite->getCount());
	}
}

void Tuner::draw(double time)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	text.print(noteNames[notes[currentNote] % 12].c_str(), 960 - 140, 540 - 16, 32);

	float cents = d.analyze();
	char textBuf[64];
	sprintf(textBuf, "%.0f", cents);
	text.print(textBuf, 960 - 100, 540 - 16, 32);
	
	glUseProgram(programId);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);

	drawBackground();

	glUniform3f(uniformTintId, 1, 1, 1);
	// clamp to -50 .. +50
	float clamped = std::min(std::max(cents, -50.0f), 50.0f);
	// scale to -45° .. +45°
	float angle = clamped / 50.0f * PI / 4;
	gaugeVertices[2] = 960 + needleLength * cos(angle);
	gaugeVertices[3] = gaugePositions[currentNote] + needleLength * sin(angle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * gaugeVertices.size(), gaugeVertices.data(), GL_STATIC_DRAW);
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
	results(bufferCount),
	L("tuner.log")
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
	{
		L.info(std::to_string(results[i]));
		average += results[i];
	}
	
	average /= results.size();
	L.info("!" + std::to_string(average));

	return average;
}


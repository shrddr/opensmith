#include <algorithm>
#include <GL/glew.h>
#include "Tuner.h"
#include "Menu.h"
#include "util.h"
#include "Settings/Settings.h"
#include "Audio/notes.h"

#define SAMPLE_RATE 48000

Tuner::Tuner(std::vector<int> tuning, bool returnToMenu):
	notes(tuning),
	currentNote(-1),
	text("../resources/textures/text_Inconsolata29.dds"),
	hit(false),
	returnToMenu(returnToMenu)
{
	float x = 700;
	float y = 540 + 0.5f * neckHeight;
	float w = 1000;
	float h = 2;
	size_t stringCount = tuning.size();
	float stringSpacing = neckHeight / (stringCount - 1);

	stringSprites.first = sprites.getCount();

	for (size_t id = 0; id < stringCount; id++)
	{
		gaugePositions.push_back(y);
		glm::vec3 tint(o.stringColors[3 * id],
				o.stringColors[3 * id + 1],
				o.stringColors[3 * id + 2]);
		sprites.add(
			x - h / 2,
			y,
			w,
			h,
			tint
		);
		y -= stringSpacing;
	}

	stringSprites.second = sprites.getCount();

	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(vertexArrayId);
	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	gaugeVertices.push_back(960);
	gaugeVertices.push_back(540);
	gaugeVertices.push_back(960 + needleLength);
	gaugeVertices.push_back(540);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * gaugeVertices.size(), gaugeVertices.data(), GL_STATIC_DRAW);
	programId = loadShaders("../resources/shaders/gauge.vs",
		"../resources/shaders/gauge.fs");
	uniformTintId = glGetUniformLocation(programId, "tint");

	nextNote();
	o.load();
	a = new Audio(io, SAMPLE_RATE);
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
	hit = false;
	gaugeVertices[1] = gaugePositions[currentNote];
	targetFrequency = noteFreq(440, notes[currentNote]);
	d.prepare(targetFrequency);
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
	glBufferData(GL_ARRAY_BUFFER, sprites.getSize(), sprites.getVertices(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	for (size_t id = stringSprites.first; id < stringSprites.second; id++)
	{
		glUniform3f(uniformTintId, sprites.tint(id).r, sprites.tint(id).g, sprites.tint(id).b);
		sprites.draw(id);
	}
}

void Tuner::draw(double time)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	text.print(noteName(notes[currentNote]), 960 - 140, 540 - 16, 32);

	float level = d.rms();
	// scale to 0 .. 1
	level = (level - o.noiseRMS) / (o.signalRMS - o.noiseRMS); 

	d.analyze();
	float detectedFrequency = d.result();
	float cents = centDifference(detectedFrequency, targetFrequency);
	
	char textBuf[64];
	sprintf(textBuf, "%.0f", cents);
	text.print(textBuf, 960 - 100, 540 - 16, 32);
	sprintf(textBuf, "%.2f", level);
	text.print(textBuf, 960 - 400, 540 - 16, 32);
	
	glUseProgram(programId);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);

	drawBackground();

	glUniform3f(uniformTintId, level, level, level);
	// clamp to -50 .. +50
	float clamped = std::min(std::max(cents, -50.0f), 50.0f);
	// scale to -45° .. +45°
	float angle = clamped / 50 * PI / 4;
	gaugeVertices[2] = 960 + needleLength * cosf(angle);
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

TunerDetector::TunerDetector() :
	AudioInputBuffer(3200),			// specify buffer size to fit at least two periods (2*48000/82 -> 1200, even more for bass)
	history(10)						// result() returns the average of last 10 analyze() calls
{
	offsets = { -500, -200, -100, -50, -20, -10, -5, -2, 0, 2, 5, 10, 20, 50, 100, 200, 500 };
	sinTables.resize(offsets.size() * buf.size());
	cosTables.resize(offsets.size() * buf.size());
}

void TunerDetector::prepare(float targetFrequency)
{
	method = (targetFrequency < 200) ? autocorr : lookup;

	if (method == lookup)
	{
		TunerDetector::targetFrequency = targetFrequency;
		for (size_t table = 0; table < offsets.size(); table++)
		{
			float stepFrequency = freqShift(targetFrequency, offsets[table]);
			updateTable(table, stepFrequency);
		}
	}
	
	else if (method == autocorr)
	{
		float targetPeriod = SAMPLE_RATE / targetFrequency;
		// TODO: check against buf.size() here

		const float fiveHalfsteps = 1.3348f;
		maxPeriod = (int)ceil(SAMPLE_RATE / targetFrequency * fiveHalfsteps);
		minPeriod = (int)round(SAMPLE_RATE / targetFrequency / fiveHalfsteps);
	}
}

void TunerDetector::updateTable(size_t table, float freq)
{
	for (size_t sample = 0; sample < buf.size(); sample++)
	{
		sinTables[table * buf.size() + sample] = sin(2 * PI * freq / SAMPLE_RATE * sample) / sqrt(buf.size());
		cosTables[table * buf.size() + sample] = cos(2 * PI * freq / SAMPLE_RATE * sample) / sqrt(buf.size());
	}
}

void TunerDetector::analyze()
{
	// TODO: don't analyze if the RMS is too low
	if (method == autocorr)
		analyzeAuto();
	if (method == lookup)
		analyzeLookup();
}

float TunerDetector::result()
{
	float average = 0;
	for (size_t i = 0; i < history.size(); i++)
		average += history[i];
	average /= history.size();
	return average;
}

void TunerDetector::analyzeLookup()
{
	size_t maxTable = 0;
	float maxPower = 0;

	for (size_t offset = 0; offset < offsets.size(); offset++)
	{
		float amplitudeSin = 0;
		float amplitudeCos = 0;
		for (size_t sample = 0; sample < buf.size(); sample++)
		{
			amplitudeSin += buf[sample] * sinTables[offset * buf.size() + sample];
			amplitudeCos += buf[sample] * cosTables[offset * buf.size() + sample];
		}
		float power = sqrt(amplitudeSin * amplitudeSin + amplitudeCos * amplitudeCos);
		if (power > maxPower)
		{
			maxPower = power;
			maxTable = offset;
		}
	}

	float offset = offsets[maxTable];
	float freq = freqShift(targetFrequency, offset);
	history.push_back(freq);
}

void TunerDetector::analyzeAuto()
{
	const float threshold = 0.7f;

	float reference = 0;
	for (size_t sample = 0; sample < buf.size(); sample++)
		reference += buf[sample] * buf[sample];

	enum PeakDetect { rise, fall } target = rise;
	float sum = reference;
	float sumPrev;

	std::map<size_t, float> peaks;

	for (size_t period = minPeriod; period < maxPeriod; period++)
	{
		sumPrev = sum;
		sum = 0;
		for (size_t sample = 0; sample < buf.size() - period; sample++)
			sum += buf[sample] * buf[sample + period];

		if (target == fall && sum < sumPrev)
		{
			peaks[period - 1] = sum;
			target = rise;
		}

		if (target == rise && sum > reference * threshold && sum > sumPrev)
			target = fall;
	}

	if (peaks.empty()) return;

	size_t maxIndex = 0;
	float maxValue = 0;
	for (auto peak : peaks)
	{
		if (peak.second > maxValue)
		{
			maxIndex = peak.first;
			maxValue = peak.second;
		}
	}

	float freq = (float)SAMPLE_RATE / maxIndex;
	history.push_back(freq);
}
#pragma once
#include <vector>
#include "GameState.h"

class FreqDetector : public AudioConsumer
{
public:
	FreqDetector(size_t bufferSize) : buf(bufferSize) {}
	void setPCM(const float in) { buf.push_back(in); }
protected:
	CircularBuffer<float> buf;
};

class Wave : public FreqDetector
{
public:
	Wave(size_t bufferSize) : FreqDetector(bufferSize)
	{
		vertexBuffer.resize(buf.size() * 4);
	}
	void* data()
	{
		float x = 100;
		float y = 540 + 540 * buf[0];
		for (size_t sample = 0, v = 0; sample < buf.size(); sample++)
		{
			vertexBuffer[v++] = x;
			vertexBuffer[v++] = y;
			x = x + 1;
			y = 540 + 540 * buf[sample];
			vertexBuffer[v++] = x;
			vertexBuffer[v++] = y;
		}	
		return vertexBuffer.data();
	}
	int size() { return vertexBuffer.size() * sizeof(float); }
	int vertexCount() { return buf.size() * 2; }
private:
	std::vector<float> vertexBuffer;
};

class Tuner : public GameState
{
public:
	Tuner(std::vector<int> notes);
	void Tuner::keyPressed(int key);
	void Tuner::draw(double time);
	~Tuner();
private:
	std::vector<int>& notes;
	std::vector<int>::iterator note;
	

	GLuint vertexArrayId;
	GLuint vertexBufferId;
	GLuint programId;

	Silence s;
	Wave w;
	InOut io{ &s, &w, 1, 1 };
	Audio* a;
};


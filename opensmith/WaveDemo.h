#pragma once
#include <vector>
#include "GameState.h"
#include "Audio/Audio.h"

class Wave : public AudioInputBuffer
{
public:
	Wave(size_t bufferSize) : AudioInputBuffer(bufferSize)
	{
		vertexBuffer.resize(buf.size() * 4);
	}
	void* data()
	{
		float x = 0;
		float y = buf[0];
		for (size_t sample = 0, v = 0; sample < buf.size(); sample++)
		{
			vertexBuffer[v++] = x;
			vertexBuffer[v++] = y;
			x = sample / (float)buf.size();
			y = buf[sample];
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

class WaveDemo : public GameState
{
public:
	WaveDemo();
	void WaveDemo::keyPressed(int key);
	void WaveDemo::draw(double time);
	~WaveDemo();
private:

	GLuint vertexArrayId;
	GLuint vertexBufferId;
	GLuint programId;

	Silence s;
	Wave w;
	InOut io{ &s, &w, 1, 1 };
	Audio* a;
};


#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Text2D.h"
#include "Sprite.h"

class Hud
{
public:
	Hud();
	void initialize(float songLength);
	void paint(float currentTime);
	~Hud();

	struct Iteration
	{
		float startTime;
		float endTime;
		int maxDifficulty;
		bool current;
		bool passed;
	};

	size_t currentIteration;
	std::vector<Iteration> iterations;
	float detected[144];

private:
	const float screenWidth = 1920;
	const float screenHeight = 1080;
	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> UVs;
	float songLength;

	unsigned int VertexBufferID;
	unsigned int UVBufferID;
	unsigned int ShaderID;
	unsigned int TextureID;
	unsigned int UniformTextureID;
	unsigned int UniformTintID;
	
	std::vector<Sprite*> bars;
	std::vector<Sprite*> notes;

	Text2D text;
};


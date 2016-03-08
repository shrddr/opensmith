#include "Hud.h"
#include <GL/glew.h>
#include "util.h"

Hud::Hud():
	text("../resources/textures/text_Inconsolata29.dds"),
	currentIteration(0)
{
	glGenBuffers(1, &VertexBufferID);
	glGenBuffers(1, &UVBufferID);
	ShaderID = loadShaders("../resources/shaders/Hud.vs",
		"../resources/shaders/Hud.fs");
	TextureID = loadTexture("../resources/textures/bar.dds");
	UniformTextureID = glGetUniformLocation(ShaderID, "myTextureSampler");
	UniformTintID = glGetUniformLocation(ShaderID, "tint");
}

void Hud::initTimeline(float songLength, float left, float bottom, float height)
{
	for (auto bar : bars)
		delete bar;
	bars.clear();
	Sprite::clear();

	this->songLength = songLength;

	float spaceWidth = 3;
	float freeWidth = screenWidth - left * 2 - (spaceWidth * iterations.size());

	for (auto it: iterations)
	{
		float barWidth = freeWidth * (it.endTime - it.startTime) / songLength;
		float barHeight = height * it.difficulty;
		glm::vec3 tint = it.maxDifficulty ? glm::vec3(1, 0, 1) : glm::vec3(1, 0.5, 0);

		bars.push_back(new Sprite(
			left,
			bottom,
			barWidth,
			barHeight,
			tint
		));

		left += barWidth + spaceWidth;
	}

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, Sprite::getSize() * sizeof(glm::vec2), Sprite::getVertices(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, UVBufferID);
	glBufferData(GL_ARRAY_BUFFER, Sprite::getSize() * sizeof(glm::vec2), Sprite::getUVs(), GL_STATIC_DRAW);
}

void Hud::initNotes()
{
	for (size_t string = 0; string < 6; string++)
	{
		float bottom = 800 + string * 25;
		for (size_t fret = 0; fret < 24; fret++)
		{
			float left = 100 + fret * 25;
			notes.push_back(new Sprite(
				left,
				bottom,
				24,
				24,
				glm::vec3(1, 1, 1)
				));
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, Sprite::getSize() * sizeof(glm::vec2), Sprite::getVertices(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, UVBufferID);
	glBufferData(GL_ARRAY_BUFFER, Sprite::getSize() * sizeof(glm::vec2), Sprite::getUVs(), GL_STATIC_DRAW);
}

void Hud::drawTime(float currentTime)
{
	char textBuf[64];
	sprintf(textBuf, "%.2fs", currentTime);
	text.print(textBuf, 0 + 2, 1080 - 2 - 32, 32);
}

void Hud::drawTimeline(float currentTime)
{
	if (currentIteration < iterations.size() - 1 &&
		currentTime > iterations[currentIteration].endTime)
	{
		++currentIteration;
		bars[currentIteration]->tint *= 0.5;
	}

	glUseProgram(ShaderID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureID);
	glUniform1i(UniformTextureID, 0);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, UVBufferID);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	for (auto bar : bars)
	{
		glUniform3f(UniformTintID, bar->tint.r, bar->tint.g, bar->tint.b);
		glDrawArrays(GL_TRIANGLES, bar->getOffset(), bar->getCount());
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void Hud::drawNotes()
{
	glUseProgram(ShaderID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureID);
	glUniform1i(UniformTextureID, 0);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, UVBufferID);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	int i = 0;
	for (auto note : notes)
	{
		note->tint = glm::vec3(detected[i++]);
		glUniform3f(UniformTintID, note->tint.r, note->tint.g, note->tint.b);
		glDrawArrays(GL_TRIANGLES, note->getOffset(), note->getCount());
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

Hud::~Hud()
{
	for (auto bar : bars) delete bar;
	for (auto note : notes) delete note;
	glDeleteBuffers(1, &VertexBufferID);
	glDeleteBuffers(1, &UVBufferID);
	glDeleteTextures(1, &TextureID);
	glDeleteProgram(ShaderID);
}

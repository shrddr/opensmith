#include <GL/glew.h>
#include "WaveDemo.h"
#include "GameStates/Menu.h"
#include "util.h"

WaveDemo::WaveDemo() :
	w(1024)
{
	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(vertexArrayId);
	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, w.size(), w.data(), GL_STATIC_DRAW);
	programId = loadShaders("../resources/shaders/waveform.vs",
		"../resources/shaders/waveform.fs");

	a = new Audio(io, 48000);
	a->start();
}

WaveDemo::~WaveDemo()
{
	a->stop();
	delete a;
	glDeleteProgram(programId);
	glDeleteBuffers(1, &vertexBufferId);
	glDeleteVertexArrays(1, &vertexArrayId);
}

void WaveDemo::keyPressed(int key)
{
	if (key == GLFW_KEY_ESCAPE)
	{
		delete gameState;
		gameState = new MainMenu;
	}
}
void WaveDemo::draw(double time)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO : render to texture, blur and stick it into the scene

	glUseProgram(programId);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, w.size(), w.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_LINES, 0, w.vertexCount());
	glDisableVertexAttribArray(0);
	
}
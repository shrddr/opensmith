#include <GL/glew.h>
#include "Tuner.h"
#include "Menu.h"
#include "util.h"

Tuner::Tuner(std::vector<int> notes):
	notes(notes),
	w(1024)
{
	note = notes.begin();
	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(vertexArrayId);
	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, w.size(), w.data(), GL_STATIC_DRAW);
	programId = loadShaders("../resources/shaders/waveform.vs",
		"../resources/shaders/waveform.fs");

	o.load();
	a = new Audio(io, 48000);
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

	glUseProgram(programId);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, w.size(), w.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_LINES, 0, w.vertexCount());
	glDisableVertexAttribArray(0);
	
}
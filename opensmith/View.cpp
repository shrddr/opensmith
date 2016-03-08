#include <iostream>
#include <fstream>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "View.h"
#include "util.h"

static glm::mat4 unity = glm::mat4(1.0f);

View::View(GLFWwindow& window):
	window(window),
	camera(window)
{
	glClearColor(0.0f, 0.02f, 0.02f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	// HACK: when sliding down, triangles switch facing
	//glEnable(GL_CULL_FACE);

	createVertexBuffer();

	programId = loadShaders("../resources/shaders/main.vs",
		"../resources/shaders/main.fs");
	uniformLocationMVP = glGetUniformLocation(programId, "MVP");
	uniformLocationTex = glGetUniformLocation(programId, "myTextureSampler");
	uniformLocationTint = glGetUniformLocation(programId, "tint");
	
	anchorTexture = loadTexture("../resources/textures/anchor.dds");
	noteTexture = loadTexture("../resources/textures/note.dds");
	missTexture = loadTexture("../resources/textures/note_miss.dds");
	openSustainTexture = loadTexture("../resources/textures/open_sustain.dds");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	lastFrameTime = glfwGetTime();
}

View::~View()
{
	delete anchorMesh;
	delete beatMesh;
	delete noteMesh;
	delete noteSustainMesh;
	delete noteSlideMesh;
	delete noteOpenMesh;
	delete noteOpenSustainMesh;
	delete stringMesh;
	delete fretMesh;
	glDeleteBuffers(1, &vertexBufferId);
	glDeleteVertexArrays(1, &vertexArrayId);
	glDeleteProgram(programId);
	glDeleteTextures(1, &anchorTexture);
	glDeleteTextures(1, &noteTexture);
	glDeleteTextures(1, &missTexture);
	glDeleteTextures(1, &openSustainTexture);
}

void View::createVertexBuffer()
{
	// Vertex Array Object
	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(vertexArrayId);

	anchorMesh = new Mesh("../resources/models/anchor.obj"); // TODO: dynamic allocation not needed
	beatMesh = new Mesh("../resources/models/beat.obj");
	noteMesh = new Mesh("../resources/models/note.obj");
	noteSustainMesh = new Mesh("../resources/models/note_sustain.obj");
	noteSlideMesh = new Mesh("../resources/models/note_slide1.obj");
	noteOpenMesh = new Mesh("../resources/models/note_open.obj");
	noteOpenSustainMesh = new Mesh("../resources/models/note_open_sustain.obj");
	stringMesh = new Mesh("../resources/models/string.obj");
	fretMesh = new Mesh("../resources/models/fret.obj");

	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, Mesh::getSize() * sizeof(glm::vec3), Mesh::getVertices(), GL_STATIC_DRAW);

	glGenBuffers(1, &uvBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
	glBufferData(GL_ARRAY_BUFFER, Mesh::getSize() * sizeof(glm::vec2), Mesh::getUVs(), GL_STATIC_DRAW);
}

void View::drawAnchor(float x, int df, float z, float dz)
{
	glm::mat4 Model = glm::translate(unity, glm::vec3(-o.noteStep / 2 + x, 0, z));
	Model = glm::scale(Model, glm::vec3(o.noteStep * df, 1, dz));

	glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
	glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
	glUniform3f(uniformLocationTint, 0.0f, 0.1f, 0.2f);

	glBindTexture(GL_TEXTURE_2D, anchorTexture);
	glDrawArrays(GL_TRIANGLES, anchorMesh->getOffset(), anchorMesh->getCount());
	glBindTexture(GL_TEXTURE_2D, noteTexture);
}

void View::drawBeat(float z)
{
	glm::mat4 Model = glm::translate(unity, glm::vec3(0, 0, z));
	glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
	glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
	glUniform3f(uniformLocationTint, 1, 1, 1);

	glDrawArrays(GL_TRIANGLES, beatMesh->getOffset(), beatMesh->getCount());
}

void View::drawNote(float x, float y, float z, int tint)
{
	glm::mat4 Model = glm::translate(unity, glm::vec3(x, y, z));
	glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
	glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
	setTint(tint);
	
	glDrawArrays(GL_TRIANGLES, noteMesh->getOffset(), noteMesh->getCount());
}

void View::drawGhost(float x, float y, float z, int tint, bool hit, float t)
{
	glm::mat4 Model = glm::translate(unity, glm::vec3(x, y, z)); 
	glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
	glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
	
	if (hit)
	{
		glBindTexture(GL_TEXTURE_2D, noteTexture);
		setTint(tint, 1.0 / (0.33 + t * 10));		// 3 -> 1
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, missTexture);
		setTint(tint, 1.0 / (1.0 + t * 10));		// 1 -> 1/3
	}
		
	glDrawArrays(GL_TRIANGLES, noteMesh->getOffset(), noteMesh->getCount());
	glBindTexture(GL_TEXTURE_2D, noteTexture);
}

void View::drawSustain(float x, int df, float y, float z, float dz, int tint)
{
	glm::mat4 Model = glm::translate(unity, glm::vec3(x, y, z));
	if (df == 0)
		Model = glm::scale(Model, glm::vec3(1, 1, dz)); // straight sustain
	else
		Model = glm::scale(Model, glm::vec3(df, 1, dz)); // slide sustain
	

	glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
	glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
	setTint(tint);

	if (df == 0)
		glDrawArrays(GL_TRIANGLES, noteSustainMesh->getOffset(), noteSustainMesh->getCount());
	else
		glDrawArrays(GL_TRIANGLES, noteSlideMesh->getOffset(), noteSlideMesh->getCount());
}

void View::drawOpenNote(float x, float y, float z, int tint)
{
	glm::mat4 Model = glm::translate(unity, glm::vec3(x, y, z));
	glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
	glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
	setTint(tint);

	glDrawArrays(GL_TRIANGLES, noteOpenMesh->getOffset(), noteOpenMesh->getCount());
}

void View::drawOpenSustain(float x, float y, float z, float dz, int tint)
{
	glm::mat4 Model = glm::translate(unity, glm::vec3(x, y, z));
	if (z + dz)
	Model = glm::scale(Model, glm::vec3(1, 1, dz));
	glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
	glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
	setTint(tint);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, openSustainTexture);
	glDrawArrays(GL_TRIANGLES, noteOpenSustainMesh->getOffset(), noteOpenSustainMesh->getCount());
	glBindTexture(GL_TEXTURE_2D, noteTexture);
	glDisable(GL_BLEND);
}

void View::drawStrings(float z)
{
	for (size_t stringNum = 0; stringNum < 6; stringNum++)
	{
		glm::mat4 Model = glm::translate(unity, glm::vec3(0, stringNum * 1.5f, z));
		glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
		glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
		setTint(stringNum);

		glDrawArrays(GL_TRIANGLES, stringMesh->getOffset(), stringMesh->getCount());
	}
}

void View::drawFrets(float z)
{
	for (size_t fretNum = 0; fretNum < 24; fretNum++)
	{
		glm::mat4 Model = glm::translate(unity, glm::vec3(fretNum * 3.0f, 0, z));
		glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
		glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);

		glUniform3f(uniformLocationTint, 0.7f, 0.6f, 0.3f);

		glDrawArrays(GL_TRIANGLES, fretMesh->getOffset(), fretMesh->getCount());
	}
}

void View::setTint(int string, float b)
{
	switch (string)
	{
	case 0:
		glUniform3f(uniformLocationTint, 1*b, 0.1f*b, 0.1f*b);
		break;
	case 1:
		glUniform3f(uniformLocationTint, 1 * b, 1 * b, 0.1f*b);
		break;
	case 2:
		glUniform3f(uniformLocationTint, 0.2f*b, 0.2f*b, 1 * b);
		break;
	case 3:
		glUniform3f(uniformLocationTint, 1 * b, 0.7f*b, 0.1f*b);
		break;
	case 4:
		glUniform3f(uniformLocationTint, 0.1f*b, 1 * b, 0.1f*b);
		break;
	case 5:
		glUniform3f(uniformLocationTint, 0.7f*b, 0.1f*b, 0.7f*b);
		break;
	default:
		glUniform3f(uniformLocationTint, 0.5f*b, 0.5f*b, 0.5f*b);
		break;
	}
}

void View::show(Visuals& visuals, Hud& hud, float currentTime)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(programId);

	camera.setZ(-20 + currentTime * o.zSpeed);
	camera.update();

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, noteTexture);
	glUniform1i(uniformLocationTex, 0);

	for (auto it : visuals.anchors)
		drawAnchor(it.fret * o.noteStep,
			it.width,
			it.startTime * o.zSpeed,
			(it.endTime - it.startTime) * o.zSpeed);

	// these are ugly
	//for (auto it : visuals.beats)
	//	drawBeat(it.time * o.zSpeed);

	for (auto it : visuals.notes)
		if (it.time > currentTime)		// notes do go past the string - symmetrical detection window; just hide them
			if (it.fret == 0)
				drawOpenNote(it.anchorFret * o.noteStep,
					it.string * o.stringStep,
					it.time * o.zSpeed,
					it.string);
			else
				drawNote(it.fret * o.noteStep,
					it.string * o.stringStep,
					it.time * o.zSpeed,
					it.string);

	for (auto it : visuals.sustains)
		if (it.startFret == 0)
			drawOpenSustain(it.anchorFret * o.noteStep,
				it.string * o.stringStep,
				it.time > currentTime ? it.time * o.zSpeed : currentTime * o.zSpeed,
				it.time > currentTime ? it.length * o.zSpeed : (it.length + it.time - currentTime) * o.zSpeed,
				it.string);
		else
			drawSustain(it.startFret * o.noteStep,
				it.deltaFret,
				it.string * o.stringStep,
				it.time > currentTime ? it.time * o.zSpeed : currentTime * o.zSpeed,
				it.time > currentTime ? it.length * o.zSpeed : (it.length + it.time - currentTime) * o.zSpeed,
				it.string);

	for (auto it : visuals.ghosts)
		drawGhost(it.fret * o.noteStep,
			it.string * o.stringStep,
			currentTime * o.zSpeed,
			it.string,
			it.hit,
			currentTime - it.time);

	drawStrings(currentTime * o.zSpeed);
	drawFrets(currentTime * o.zSpeed);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	hud.drawTime(currentTime);
	hud.drawTimeline(currentTime);
	hud.drawNotes();
}



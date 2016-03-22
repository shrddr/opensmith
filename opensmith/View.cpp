#include <iostream>
#include <fstream>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "View.h"
#include "util.h"
#include "PsarcReader/Sng.h"

static glm::mat4 unity = glm::mat4(1.0f);

View::View(GLFWwindow& window):
	window(window),
	camera(window)
{
	stringCount = (o.role == bass) ? 4 : 6;

	glClearColor(0.0f, 0.02f, 0.02f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	createVertexBuffer();

	programId = loadShaders("../resources/shaders/main.vs",
		"../resources/shaders/main.fs");
	uniformLocationMVP = glGetUniformLocation(programId, "MVP");
	uniformLocationTex = glGetUniformLocation(programId, "myTextureSampler");
	uniformLocationTint = glGetUniformLocation(programId, "tint");
	
	anchorTexture = loadTexture("../resources/textures/anchor.dds");
	noteTexture = loadTexture("../resources/textures/note.dds");
	noteMuteTexture = loadTexture("../resources/textures/note_fretmute.dds");
	notePalmMuteTexture = loadTexture("../resources/textures/note_palmmute.dds");
	missTexture = loadTexture("../resources/textures/note_miss.dds");
	fretNumTexture = loadTexture("../resources/textures/fretnumbers_Inconsolata128.dds");
	sustainTexture = loadTexture("../resources/textures/sustain.dds");
	openSustainTexture = loadTexture("../resources/textures/open_sustain.dds");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	lastFrameTime = glfwGetTime();
}

View::~View()
{
	glDeleteBuffers(1, &vertexBufferId);
	glDeleteVertexArrays(1, &vertexArrayId);
	glDeleteProgram(programId);
	glDeleteTextures(1, &anchorTexture);
	glDeleteTextures(1, &noteTexture);
	glDeleteTextures(1, &noteMuteTexture);
	glDeleteTextures(1, &notePalmMuteTexture);
	glDeleteTextures(1, &missTexture);
	glDeleteTextures(1, &fretNumTexture);
	glDeleteTextures(1, &sustainTexture);
	glDeleteTextures(1, &openSustainTexture);
}

void View::createVertexBuffer()
{
	// Vertex Array Object
	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(vertexArrayId);

	anchorMesh = m.loadMesh("../resources/models/anchor.obj");
	beatMesh = m.loadMesh("../resources/models/beat.obj");
	noteMesh = m.loadMesh("../resources/models/note.obj");
	noteSustainMesh = m.loadMesh("../resources/models/note_sustain.obj");
	noteOpenMesh = m.loadMesh("../resources/models/note_open.obj");
	noteOpenSustainMesh = m.loadMesh("../resources/models/note_open_sustain.obj");
	stringMesh = m.loadMesh("../resources/models/string.obj");
	fretMesh = m.loadMesh("../resources/models/fret.obj");
	
	std::vector<int> deltas;
	for (size_t i = 1; i < 25; i++)
		deltas.push_back(i);
	for (size_t i = 1; i < 25; i++)
		deltas.push_back(-i);
	noteSlideMeshes = m.generateSlideMeshes(deltas);
	fretNumMeshes = m.generateTiledQuads(32);

	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, m.getSize() * sizeof(glm::vec3), m.getVertices(), GL_STATIC_DRAW);

	glGenBuffers(1, &uvBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
	glBufferData(GL_ARRAY_BUFFER, m.getSize() * sizeof(glm::vec2), m.getUVs(), GL_STATIC_DRAW);
}

void View::drawAnchor(float x, int df, float z, float dz)
{
	glm::mat4 Model = glm::translate(unity, glm::vec3(-o.fretStep / 2 + x, 0, z));
	Model = glm::scale(Model, glm::vec3(o.fretStep * df, 1, dz));

	glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
	glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
	glUniform3f(uniformLocationTint, 0.0f, 0.1f, 0.2f);

	glBindTexture(GL_TEXTURE_2D, anchorTexture);
	m.draw(anchorMesh);
}

void View::drawBeat(float z)
{
	glm::mat4 Model = glm::translate(unity, glm::vec3(0, 0, z));
	glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
	glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
	glUniform3f(uniformLocationTint, 1, 1, 1);

	glBindTexture(GL_TEXTURE_2D, noteTexture);
	m.draw(beatMesh);
}

void View::drawNote(float x, float y, float z, int tint, uint32_t mask)
{
	glm::mat4 Model = glm::translate(unity, glm::vec3(x, y, z));
	glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
	glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
	setTint(tint);
	
	if (mask & NOTE_MASK_PALMMUTE)
		glBindTexture(GL_TEXTURE_2D, notePalmMuteTexture);
	else if (mask & NOTE_MASK_FRETHANDMUTE)
		glBindTexture(GL_TEXTURE_2D, noteMuteTexture);
	else
		glBindTexture(GL_TEXTURE_2D, noteTexture);
	m.draw(noteMesh);
}

void View::drawOpenNote(float x, float y, float z, int tint, uint32_t mask)
{
	glm::mat4 Model = glm::translate(unity, glm::vec3(x, y, z));
	glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
	glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
	setTint(tint);

	glBindTexture(GL_TEXTURE_2D, noteTexture);
	m.draw(noteOpenMesh);
}

void View::drawSustain(float x, int df, float y, float z, float dz, int tint)
{
	glm::mat4 Model = glm::translate(unity, glm::vec3(x, y, z));
	if (df == 0)
		Model = glm::scale(Model, glm::vec3(1, 1, dz));
	else
		Model = glm::scale(Model, glm::vec3(o.fretStep, 1, dz));

	glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
	glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
	setTint(tint);

	glBindTexture(GL_TEXTURE_2D, sustainTexture);

	if (df == 0)
		m.draw(noteSustainMesh); // straight sustain
	else if (df > 0)
		m.draw(noteSlideMeshes + df - 1); // slide sustain up
	else
		m.draw(noteSlideMeshes + 23 - df);	// slide sustain down
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
	m.draw(noteOpenSustainMesh);
	glDisable(GL_BLEND);
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

	m.draw(noteMesh);
}

void View::drawStrings(float z)
{
	for (size_t stringNum = 0; stringNum < stringCount; stringNum++)
	{
		glm::mat4 Model = unity;
		Model = glm::translate(Model, glm::vec3(0, stringNum * o.stringStep, z));
		Model = glm::scale(Model, glm::vec3(o.fretStep * 25, 1, 1));
		glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
		glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
		setTint(stringNum);

		glBindTexture(GL_TEXTURE_2D, noteTexture);
		m.draw(stringMesh);
	}
}

void View::drawFrets(float z)
{
	for (size_t fretNum = 0; fretNum < 25; fretNum++)
	{
		glm::mat4 Model = unity;	
		Model = glm::translate(Model, glm::vec3(fretNum * o.fretStep, -o.stringStep / 2, z));
		Model = glm::scale(Model, glm::vec3(1, stringCount * o.stringStep, 1));
		glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
		glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
		glUniform3f(uniformLocationTint, 0.7f, 0.6f, 0.3f);
		glBindTexture(GL_TEXTURE_2D, noteTexture);
		m.draw(fretMesh);
	}

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	for (size_t fretNum = 1; fretNum < 25; fretNum++)
	{
		glm::mat4 Model = glm::translate(unity, glm::vec3(fretNum * o.fretStep, stringCount * o.stringStep, z));
		glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
		glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
		glBindTexture(GL_TEXTURE_2D, fretNumTexture);
		m.draw(fretNumMeshes + fretNum);
	}
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void View::setTint(int string, float brightness)
{
	if (string < 0 || string > 5)
		glUniform3f(uniformLocationTint, 0.5f * brightness, 0.5f * brightness, 0.5f * brightness);
	else
		glUniform3f(uniformLocationTint,
			o.stringColors[3 * string] * brightness,
			o.stringColors[3 * string + 1] * brightness,
			o.stringColors[3 * string + 2] * brightness);
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
		drawAnchor(it.fret * o.fretStep,
			it.width,
			it.startTime * o.zSpeed,
			(it.endTime - it.startTime) * o.zSpeed);

	// TODO: these are ugly; better modify anchor texture instead
	//for (auto it : visuals.beats)
	//	drawBeat(it.time * o.zSpeed);

	for (auto note : visuals.notes)
		if (note.time > currentTime)		// notes do go past the string - symmetrical detection window; just hide them
			if (note.fret == 0)
				drawOpenNote(note.anchorFret * o.fretStep,
					note.string * o.stringStep,
					note.time * o.zSpeed,
					note.string,
					note.mask);
			else
				drawNote(note.fret * o.fretStep,
					note.string * o.stringStep,
					note.time * o.zSpeed,
					note.string,
					note.mask);

	for (auto it : visuals.sustains)
		if (it.startFret == 0)
			drawOpenSustain(it.anchorFret * o.fretStep,
				it.string * o.stringStep,
				it.time > currentTime ? it.time * o.zSpeed : currentTime * o.zSpeed,
				it.time > currentTime ? it.length * o.zSpeed : (it.length + it.time - currentTime) * o.zSpeed,
				it.string);
		else
			drawSustain(it.startFret * o.fretStep,
				it.deltaFret,
				it.string * o.stringStep,
				it.time > currentTime ? it.time * o.zSpeed : currentTime * o.zSpeed,
				it.time > currentTime ? it.length * o.zSpeed : (it.length + it.time - currentTime) * o.zSpeed,
				it.string);

	for (auto it : visuals.ghosts)
		drawGhost(it.fret * o.fretStep,
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



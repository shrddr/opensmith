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
	for (size_t i = 0; i < stringCount; i++)
		strings.push_back(String(i));
	for (size_t i = 0; i < 25; i++)
		frets.push_back(Fret(i));
	for (size_t i = 1; i < 25; i++)
		fretLabels.push_back(FretLabel(i));

	glClearColor(0.0f, 0.02f, 0.02f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_BLEND);

	createVertexBuffer();

	programId = loadShaders("../resources/shaders/main.vs",
		"../resources/shaders/main.fs");
	uniformLocationMVP = glGetUniformLocation(programId, "MVP");
	uniformLocationTex = glGetUniformLocation(programId, "myTextureSampler");
	uniformLocationTint = glGetUniformLocation(programId, "tint");

	lastFrameTime = glfwGetTime();
	Drawable::v = this;
}

View::~View()
{
	glDeleteBuffers(1, &vertexBufferId);
	glDeleteVertexArrays(1, &vertexArrayId);
	glDeleteProgram(programId);
	Drawable::v = nullptr;
}

void View::createVertexBuffer()
{
	// Vertex Array Object
	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(vertexArrayId);

	meshes.loadMesh("../resources/models/string.obj");
	meshes.loadMesh("../resources/models/fret.obj");
	meshes.generateTiledQuads("fretlabels", 32);
	meshes.loadMesh("../resources/models/anchor.obj");
	meshes.loadMesh("../resources/models/note.obj");
	meshes.loadMesh("../resources/models/note_sustain.obj");
	meshes.loadMesh("../resources/models/note_open.obj");
	meshes.loadMesh("../resources/models/note_open_sustain.obj");

	std::vector<int> deltas;
	for (int i = 1; i < 25; i++)
		deltas.push_back(i);
	for (int i = 1; i < 25; i++)
		deltas.push_back(-i);
	meshes.generateSlideMeshes("slides", deltas);

	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, meshes.getSize() * sizeof(glm::vec3), meshes.getVertices(), GL_STATIC_DRAW);

	glGenBuffers(1, &uvBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
	glBufferData(GL_ARRAY_BUFFER, meshes.getSize() * sizeof(glm::vec2), meshes.getUVs(), GL_STATIC_DRAW);
}

void View::setModel(glm::mat4& Model)
{
	glm::mat4 MVP = camera.getProjection() * camera.getView() * Model;
	glUniformMatrix4fv(uniformLocationMVP, 1, GL_FALSE, &MVP[0][0]);
}

void View::setTexture(GLuint textureId)
{
	glBindTexture(GL_TEXTURE_2D, textureId);
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

void View::setTint(float r, float g, float b)
{
	glUniform3f(uniformLocationTint, r, g, b);
}

void View::setActiveAnchor(char fret, char width)
{
	for (auto& fretLabel : fretLabels)
		fretLabel.active = (fretLabel.id >= fret && fretLabel.id < fret + width);
}

void View::show(Hud& hud, float currentTime)
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
	glUniform1i(uniformLocationTex, 0);

	for (auto anchor : anchors)
	{
		anchor.draw(currentTime);
		if (anchor.startTime < currentTime && anchor.endTime > currentTime)
			setActiveAnchor(anchor.fret, anchor.width);
	}
		
	for (auto note : notes)
		note.draw(currentTime);

	for (auto sustain : sustains)
		sustain.draw(currentTime);

	for (auto ghost : ghosts)
		ghost.draw(currentTime);

	for (auto string : strings)
		string.draw(currentTime);

	for (auto fret : frets)
		fret.draw(currentTime);

	for (auto fretLabel : fretLabels)
		fretLabel.draw(currentTime);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	hud.drawTime(currentTime);
	hud.drawTimeline(currentTime);
	hud.drawNotes();
}

View* Drawable::v = nullptr;

void Anchor::draw(float currentTime)
{
	float x = (fret - 0.5f) * o.fretStep;
	float z = startTime * o.zSpeed;
	float dx = width * o.fretStep;
	float dz = (endTime - startTime) * o.zSpeed;

	glm::mat4 Model = glm::translate(unity, glm::vec3(x, 0, z));
	Model = glm::scale(Model, glm::vec3(dx, 1, dz));

	v->setModel(Model);
	v->setTexture(v->textures.get("../resources/textures/anchor.dds"));
	v->setTint(0.0f, 0.1f, 0.2f);
	v->meshes.draw(v->meshes.get("../resources/models/anchor.obj"));
}

void Note::draw(float currentTime)
{
	if (time < currentTime) // notes do go past the string - symmetrical detection window; just hide them
		return;

	float x = (fret > 0 ? fret : anchorFret) * o.fretStep;
	float y = string * o.stringStep;
	float z = time * o.zSpeed;

	glm::mat4 Model = glm::translate(unity, glm::vec3(x, y, z));

	v->setModel(Model);
	v->setTint(string);

	if (mask & NOTE_MASK_PALMMUTE)
		v->setTexture(v->textures.get("../resources/textures/note_palmmute.dds"));
	else if (mask & NOTE_MASK_FRETHANDMUTE)
		v->setTexture(v->textures.get("../resources/textures/note_fretmute.dds"));
	else
		v->setTexture(v->textures.get("../resources/textures/note.dds"));

	if (fret > 0)
		v->meshes.draw(v->meshes.get("../resources/models/note.obj"));
	else
		v->meshes.draw(v->meshes.get("../resources/models/note_open.obj"));
}

void Sustain::draw(float currentTime)
{
	float x = (startFret == 0 ? anchorFret : startFret) * o.fretStep;
	float y = string * o.stringStep;
	float z = time > currentTime ? time * o.zSpeed : currentTime * o.zSpeed;
	float dz = time > currentTime ? length * o.zSpeed : (length + time - currentTime) * o.zSpeed;

	glm::mat4 Model = glm::translate(unity, glm::vec3(x, y, z));
	Model = glm::scale(Model, glm::vec3((deltaFret == 0 ? 1 : o.fretStep), 1, dz));

	v->setModel(Model);
	v->setTint(string);

	if (startFret == 0)
		v->setTexture(v->textures.get("../resources/textures/open_sustain.dds"));
	else
		v->setTexture(v->textures.get("../resources/textures/sustain.dds"));

	if (startFret == 0)
	{
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		v->meshes.draw(v->meshes.get("../resources/models/note_open_sustain.obj"));
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	else if (deltaFret == 0)
		v->meshes.draw(v->meshes.get("../resources/models/note_sustain.obj")); // straight sustain
	else if (deltaFret > 0)
		v->meshes.draw(v->meshes.get("slides") + deltaFret - 1); // slide sustain up
	else
		v->meshes.draw(v->meshes.get("slides") + 23 - deltaFret); // slide sustain down
}

void Ghost::draw(float currentTime)
{
	float x = fret * o.fretStep;
	float y = string * o.stringStep;
	float z = currentTime * o.zSpeed;
	float t = currentTime - time;

	glm::mat4 Model = glm::translate(unity, glm::vec3(x, y, z));
	v->setModel(Model);

	if (hit)
	{
		v->setTexture(v->textures.get("../resources/textures/note.dds"));
		v->setTint(string, 1.0 / (0.33 + t * 10));		// 3 -> 1
	}
	else
	{
		v->setTexture(v->textures.get("../resources/textures/note_miss.dds"));
		v->setTint(string, 1.0 / (1.0 + t * 10));		// 1 -> 1/3
	}

	v->meshes.draw(v->meshes.get("../resources/models/note.obj"));
}

void String::draw(float currentTime)
{
	glm::mat4 Model = unity;
	Model = glm::translate(Model, glm::vec3(0, id * o.stringStep, currentTime * o.zSpeed));
	Model = glm::scale(Model, glm::vec3(o.fretStep * 25, 1, 1));
	v->setModel(Model);
	v->setTint(id);
	v->setTexture(v->textures.get("../resources/textures/string.dds"));
	v->meshes.draw(v->meshes.get("../resources/models/string.obj"));
}

void Fret::draw(float currentTime)
{
	glm::mat4 Model = unity;
	Model = glm::translate(Model, glm::vec3(id * o.fretStep, -o.stringStep / 2, currentTime * o.zSpeed));
	Model = glm::scale(Model, glm::vec3(1, v->stringCount * o.stringStep, 1));
	v->setModel(Model);
	v->setTint(0.7f, 0.6f, 0.3f);
	v->setTexture(v->textures.get("../resources/textures/fret.dds"));
	v->meshes.draw(v->meshes.get("../resources/models/fret.obj"));
}

void FretLabel::draw(float currentTime)
{
	glm::mat4 Model = glm::translate(unity, glm::vec3(id * o.fretStep, v->stringCount * o.stringStep, currentTime * o.zSpeed));
	v->setModel(Model);
	v->setTexture(v->textures.get("../resources/textures/fretnumbers_Inconsolata128.dds"));

	if (active)
		v->setTint(0.7f, 0.6f, 0.3f);
	else
		v->setTint(0.2f, 0.2f, 0.2f);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	v->meshes.draw(v->meshes.get("fretlabels") + id);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

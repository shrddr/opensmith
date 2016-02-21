#pragma once
#include <vector>
#include <deque>
#include <GLFW/glfw3.h>
#include "Settings/Settings.h"
#include "Camera.h"
#include "Mesh.h"
#include "Hud.h"

struct Visuals
{
	struct Beat
	{
		float time;
	};

	struct Note
	{
		int fret;
		int string;
		float time;
		int anchorFret;
		bool hit;
	};

	struct Sustain
	{
		int startFret;
		int deltaFret;
		int string;
		float time;
		float length;
		int anchorFret;
	};

	struct Anchor
	{
		int fret;
		int width;
		float startTime;
		float endTime;
	};

	struct Ghost
	{
		int fret;
		int string;
		float time;
		bool hit;
	};

	std::deque<Anchor> anchors;
	std::deque<Beat> beats;
	std::deque<Note> notes;
	std::deque<Sustain> sustains;
	std::deque<Ghost> ghosts;
};

class View
{
public:
	View(GLFWwindow& window);
	~View();
	void show(Visuals& visuals, Hud& hud, float currentTime);
private:
	GLFWwindow& window;
	Camera camera;
	void createVertexBuffer();

	void drawAnchor(float x, int df, float z, float dz);
	void drawBeat(float z);
	void drawNote(float x, float y, float z, int tint);
	void drawGhost(float x, float y, float z, int tint, bool hit, float t);
	void drawSustain(float x, int df, float y, float z, float dz, int tint);
	void drawOpenNote(float x, float y, float z, int tint);
	void drawOpenSustain(float x, float y, float z, float dz, int tint);
	void drawStrings(float z);
	void drawFrets(float z);
	void setTint(int string);

	GLuint vertexArrayId;
	GLuint vertexBuffer;
	GLuint uvBuffer;
	GLuint programID;
	GLuint noteTexture;
	GLuint missTexture;
	GLuint openSustainTexture;

	GLuint uniformLocationMVP;
	GLuint uniformLocationTex;
	GLuint uniformLocationTint;

	Mesh* beatMesh;
	Mesh* noteMesh;
	Mesh* noteSustainMesh;
	Mesh* noteSlideMesh;
	Mesh* noteOpenMesh;
	Mesh* noteOpenSustainMesh;
	Mesh* stringMesh;
	Mesh* fretMesh;
	Mesh* anchorMesh;

	double lastFrameTime;
};

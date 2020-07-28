#pragma once
#include <vector>
#include <deque>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Settings/Settings.h"
#include "Camera.h"
#include "Mesh.h"
#include "Texture.h"
#include "Hud.h"

class View;

class Drawable
{
public:
	virtual void draw(float currentTime) {};
	static View* v;
};

struct Anchor : public Drawable
{
	Anchor(int fret, int width, float startTime, float endTime) :
		fret(fret),
		width(width),
		startTime(startTime),
		endTime(endTime) {};
	void draw(float currentTime);

	float startTime;
	float endTime;
	int fret;
	int width;
};

struct Beat
{
	float time;
};

struct Note : public Drawable
{
	Note(char fret, char string, float time, char anchorFret, uint32_t mask) :
		fret(fret),
		string(string),
		time(time),
		anchorFret(anchorFret),
		mask(mask),
		hit(false) {};
	void draw(float currentTime);

	float time;
	uint32_t mask;
	char fret;
	char string;
	char anchorFret;
	bool hit;
};

struct Sustain : public Drawable
{
	Sustain(char startFret, char deltaFret, char string, float time, float length, char anchorFret) :
		startFret(startFret),
		deltaFret(deltaFret),
		string(string),
		time(time),
		length(length),
		anchorFret(anchorFret) {};
	void draw(float currentTime);

	float time;
	float length;
	char startFret;
	char deltaFret;
	char string;
	char anchorFret;
};

struct Ghost : public Drawable
{
	Ghost(char fret, char string, float time, bool hit) :
		fret(fret),
		string(string),
		time(time),
		hit(hit) {};
	void draw(float currentTime);

	float time;
	char fret;
	char string;
	bool hit;
};

struct String : public Drawable
{
	String(char id) :
		id(id) {};
	void draw(float currentTime);

	char id;
};

struct Fret : public Drawable
{
	Fret(char id) :
		id(id) {};
	void draw(float currentTime);

	char id;
};

struct FretLabel : public Drawable
{
	FretLabel(char id) :
		id(id) {};
	void draw(float currentTime);

	char id;
	bool active = false;
};

class View
{
public:
	View(GLFWwindow& window);
	~View();
	void show(Hud& hud, float currentTime);
	void setModel(glm::mat4& Model);
	void setTexture(GLuint textureId);
	void setTint(int string, float brightness = 1);
	void setTint(float r, float g, float b);
	void setActiveAnchor(char fret, char width);
	Camera camera;
	MeshSet meshes;
	TextureSet textures;
	char stringCount;

	std::deque<Anchor> anchors;
	std::deque<Beat> beats;
	std::deque<Note> notes;
	std::deque<Sustain> sustains;
	std::deque<Ghost> ghosts;
	std::vector<String> strings;
	std::vector<Fret> frets;
	std::vector<FretLabel> fretLabels;
private:
	GLFWwindow& window;
	void createVertexBuffer();

	GLuint vertexArrayId;
	GLuint vertexBufferId;
	GLuint uvBufferId;
	GLuint programId;

	GLuint uniformLocationMVP;
	GLuint uniformLocationTex;
	GLuint uniformLocationTint;

	double lastFrameTime;
};

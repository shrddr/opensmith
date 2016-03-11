#pragma once
#include "Menu.h"
#include "Audio/Audio.h"
#include "WaveDemo.h"

class Setup: public Menu
{
public:
	Setup();
	~Setup();
	void keyEnter();
	void keyEsc();
	void draw(double time);
private:
	std::vector<int> itemData;
	enum { api, in, out, test } stage;

	void showApis();
	void showInputDevices();
	void showOutputDevices();
	void showConfirm();
	
	int hostApi;

	std::string inLatency;
	std::string outLatency;

	Silence s;
	Wave w;
	InOut io{ &s, &w, 1, 1 };
	Audio* a;

	GLuint vertexArrayId;
	GLuint vertexBufferId;
	GLuint programId;
};
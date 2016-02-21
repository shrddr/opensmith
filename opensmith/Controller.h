#pragma once
#include "GLFW/glfw3.h"
#include "Settings/Settings.h"

class Controller
{
public:
	Controller(GLFWwindow& window);
	~Controller();
	void key_callback(int key);
private:
	GLFWwindow& window;
};


#include "Controller.h"

Controller::Controller(GLFWwindow& window) :
	window(window)
{
}

Controller::~Controller()
{
}

void Controller::key_callback(int key)
{
	if (key == GLFW_KEY_MINUS)
		o.zSpeed /= 2;
	if (key == GLFW_KEY_EQUAL)
		o.zSpeed *= 2;
	if (key == GLFW_KEY_PAGE_DOWN)
		o.difficulty = (o.difficulty - 1) & 31;
	if (key == GLFW_KEY_PAGE_UP)
		o.difficulty = (o.difficulty + 1) & 31;	
}
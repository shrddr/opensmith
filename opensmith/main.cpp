#include <iostream>
#include <cstring>

#include "GL/glew.h"
#include <GLFW/glfw3.h>

#include "Audio/Audio.h"
#include "Settings/Settings.h"
#include "View.h"
#include "Model.h"
#include "Controller.h"


static Controller* pController = NULL;

static void error_callback(int error, const char* description)
{
	std::cerr << "GLFW Error: " << description;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (action == GLFW_PRESS)
		pController->key_callback(key);
	//if (action == GLFW_REPEAT)
	//	pController->key_callback(key);
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "usage: opensmith songfile.psarc [-rhythm] [-d##] [-f]" << std::endl;
		exit(-1);
	}

	const char* paramSongFile = argv[1];
	bool paramFullsreen = false;
	SngRole paramRole = lead;

	for (size_t i = 2; i < argc; i++)
	{
		if (strcmp("-rhythm", argv[i]) == 0)
			paramRole = rhythm;
		if (strcmp("-f", argv[i]) == 0)
			paramFullsreen = true;
		sscanf(argv[i], "-d%i", &o.difficulty);
	}

	glfwSetErrorCallback(error_callback);
	if ( !glfwInit() )
		exit(-1);

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	GLFWwindow* window;
	if (paramFullsreen)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		window = glfwCreateWindow(mode->width, mode->height, "opensmith", glfwGetPrimaryMonitor(), NULL);
	}
	else
		window = glfwCreateWindow(1024, 768, "opensmith", NULL, NULL);
	
	if (!window)
	{
		glfwTerminate();
		exit(-1);
	}
	glfwSetWindowPos(window, 700, 100);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	std::cout << glfwGetTime() << " > GLFW loaded (GL " 
		<< glGetString(GL_VERSION) << ")" << std::endl;

	glewExperimental = true;
	GLenum res = glewInit();
	if (res != GLEW_OK)
	{
		std::cerr << " > GLEW Error: " << glewGetErrorString(res);
		exit(-1);
	}
	
	std::cout << glfwGetTime() << " > GLEW loaded" << std::endl;
	
	try
	{
		o.load();
		View v(*window);
		Controller c(*window);
		Model m(v, c, paramSongFile, paramRole);

		pController = &c;
		glfwSetKeyCallback(window, key_callback);

		// main loop

		while (!glfwWindowShouldClose(window))
		{
			double preFrame = glfwGetTime();
			m.update(preFrame);
			double frameTime = glfwGetTime() - preFrame;
			/*if (frameTime > 1 / 60.0f)
				std::cout << preFrame << " > " << frameTime << std::endl;*/
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
		exit(-1);
	}

	// cleanup

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(0);
}

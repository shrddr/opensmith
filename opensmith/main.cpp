#include <iostream>
#include <cstring>

#include "GL/glew.h"
#include <GLFW/glfw3.h>

#include "Audio/Audio.h"
#include "Settings/Settings.h"
#include "GameState.h"
#include "Menu.h"

GLFWwindow* GameState::window = 0;
GameState* GameState::gameState = 0;

static void error_callback(int error, const char* description)
{
	std::cerr << "GLFW Error: " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
		GameState::gameState->keyPressed(key);
}

int main(int argc, char** argv)
{
	for (size_t i = 1; i < argc; i++)
	{
		if (strcmp("-song", argv[i]) == 0)
			o.psarcFile = o.psarcDirectory + argv[i + 1];
		if (strcmp("-rhythm", argv[i]) == 0)
			o.role = rhythm;
		if (strcmp("-bass", argv[i]) == 0)
			o.role = bass;
		if (strcmp("-f", argv[i]) == 0)
			o.fullScreen = true;
		if (strcmp("-t", argv[i]) == 0)
			o.skipTuner = true;
		sscanf(argv[i], "-d%i", &o.difficulty);
	}

	glfwSetErrorCallback(error_callback);
	if ( !glfwInit() )
		exit(-1);

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window;
	if (o.fullScreen)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		window = glfwCreateWindow(mode->width, mode->height, "opensmith", glfwGetPrimaryMonitor(), NULL);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}
	else
		window = glfwCreateWindow(960, 540, "opensmith", NULL, NULL);
	
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
	
	GameState::window = window;
	if (o.psarcFile.empty())
		GameState::gameState = new MainMenu;
	else
		GameState::gameState = new RoleMenu;

	glfwSetKeyCallback(window, key_callback);

	// main loop

	while (!glfwWindowShouldClose(window) && GameState::gameState != 0)
	{
		GameState::gameState->draw(glfwGetTime());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// cleanup

	delete GameState::gameState;
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(0);
}

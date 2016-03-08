#include "GameState.h"
#include "GLFW/glfw3.h"
#include "Menu.h"
#include "Settings/Settings.h"
#include "View.h"
#include "Model.h"
#include "Controller.h"

Session::Session()
{
	try
	{
		o.load();
		v = new View(*window);
		c = new Controller(*window);
		m = new Model(*v, *c);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
		exit(-1);
	}
}

Session::~Session()
{
	delete m;
	delete c;
	delete v;
}

void Session::keyPressed(int key)
{
	if (key == GLFW_KEY_ESCAPE)
	{
		delete gameState;
		gameState = new MainMenu;
	}
	if (key == GLFW_KEY_MINUS)
		o.zSpeed /= 2;
	if (key == GLFW_KEY_EQUAL)
		o.zSpeed *= 2;
	if (key == GLFW_KEY_PAGE_DOWN)
		o.difficulty = (o.difficulty - 1) & 31;
	if (key == GLFW_KEY_PAGE_UP)
		o.difficulty = (o.difficulty + 1) & 31;
}

void Session::draw(double time)
{
	m->update(time);
}

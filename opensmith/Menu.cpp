#include "Menu.h"
#include "GLFW/glfw3.h"
#include "GameState.h"

void Menu::keyPressed(int key)
{
	if (key == GLFW_KEY_W || key == GLFW_KEY_UP)
		currentItem = (currentItem - 1) % items.size();
	if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN)
		currentItem = (currentItem + 1) % items.size();
}

void Menu::draw(double time)
{
	size_t i = 0;
	const char* currentMarker = ">";

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto item : items)
	{
		text.print(item.c_str(), 100, 980 - i * 32, 32);
		if (i == currentItem)
			text.print(currentMarker, 84, 980 - i * 32, 32);
		i++;
	}
}

////

MainMenu::MainMenu()
{
	items.push_back("Play");
	items.push_back("File");
	items.push_back("Setup");
	items.push_back("Tuner");
	items.push_back("Exit");
}

void MainMenu::keyPressed(int key)
{
	Menu::keyPressed(key);

	if (key == GLFW_KEY_ENTER && currentItem == 0)
	{
		delete gameState;
		gameState = new Session;
	}
	if (key == GLFW_KEY_ENTER && currentItem == 1)
	{
		delete gameState;
		gameState = new FileMenu;
	}
	if ((key == GLFW_KEY_ENTER && currentItem == 4) || key == GLFW_KEY_ESCAPE)
	{
		delete gameState;
		gameState = 0;
	}
}

////

FileMenu::FileMenu()
{
	items.push_back("song1");
	items.push_back("song2");
	items.push_back("song3");
}

void FileMenu::keyPressed(int key)
{
	Menu::keyPressed(key);

	if (key == GLFW_KEY_ESCAPE)
	{
		delete gameState;
		gameState = new MainMenu;
	}
}

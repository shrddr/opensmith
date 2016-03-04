#pragma once
#include <vector>
#include "GameState.h"
#include "Text2D.h"

class Menu : public GameState
{
public:
	Menu() :
		text("../resources/textures/text_Inconsolata29.dds"),
		currentItem(0) {}
	void keyPressed(int key);
	void draw(double time);
protected:
	Text2D text;
	std::vector<std::string> items;
	size_t currentItem;
};

class MainMenu : public Menu
{
public:
	MainMenu();
	void keyPressed(int key);
};

class FileMenu : public Menu
{
public:
	FileMenu();
	void keyPressed(int key);
};
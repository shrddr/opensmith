#pragma once
#include <vector>
#include <map>
#include <memory>
#include "GameState.h"
#include "Text2D.h"

class Menu : public GameState
{
public:
	Menu();
	void keyPressed(int key);
	void draw(double time);
protected:
	Text2D text;
	std::vector<std::string> items;
	int selectedItem;
	int topItem;
private:
	void checkScroll();
	const float menuLeft = 100;
	const float menuTop = 980;
	const float menuFont = 32;
	const int pageLines = 880 / 32;
	const char* menuMarker = ">";
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

class RoleMenu : public Menu
{
public:
	RoleMenu(std::shared_ptr<PSARC> psarc);
	void keyPressed(int key);
private:
	std::shared_ptr<PSARC> psarc;
	std::map<int, int> itemEntry;
};

class DiffMenu : public Menu
{
public:
	DiffMenu(std::shared_ptr<Sng> sng);
	void keyPressed(int key);
	void draw(double time);
private:
	std::shared_ptr<Sng> sng;
	void updateTimeline();
	Hud hud;
};
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
	virtual void keyPressed(int key);
	virtual void keyUp();
	virtual void keyDown();
	virtual void keyPgUp();
	virtual void keyPgDown();
	virtual void keyEnter();
	virtual void keyEsc();
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
	const float fontSize = 32;
	const int pageLines = 880 / 32;
	const char* menuMarker = ">";
};

class MainMenu : public Menu
{
public:
	MainMenu();
	void keyEnter();
	void keyEsc();
};

class FileMenu : public Menu
{
public:
	FileMenu();
	void keyEnter();
	void keyEsc();
};

class RoleMenu : public Menu
{
public:
	RoleMenu(std::shared_ptr<PSARC> psarc);
	void keyEnter();
	void keyEsc();
private:
	std::shared_ptr<PSARC> psarc;
	std::map<int, int> itemEntry;
};

class DiffMenu : public Menu
{
public:
	DiffMenu(std::shared_ptr<Sng> sng);
	void keyUp();
	void keyDown();
	void keyEnter();
	void keyEsc();
	void draw(double time);
private:
	std::shared_ptr<Sng> sng;
	void updateTimeline();
	Hud hud;
};

class TuningMenu : public Menu
{
public:
	TuningMenu();
	void keyEnter();
	void keyEsc();
};
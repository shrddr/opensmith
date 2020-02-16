#pragma once
#include <vector>
#include <map>
#include <memory>
#include "GameState.h"
#include "../Text2D.h"

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
	std::string header;
	std::vector<std::string> items;
	int selectedItem;
	int topItem;
private:
	void checkScroll();
	const float headerLeft = 150;
	const float itemsLeft = 100;
	const float headerTop = 1080 - 90;
	const float itemsTop = 1080 - 170;
	const float fontSize = 32;
	const int pageItems = 880 / 32;
	const char* itemMarker = ">";
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
	RoleMenu();
	void keyEnter();
	void keyEsc();
private:
	PSARC psarc;
	std::vector<int> itemEntry;
	std::vector<SngRole> itemRole;
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
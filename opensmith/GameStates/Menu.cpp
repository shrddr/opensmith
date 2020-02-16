#include "Menu.h"
#include <algorithm>
#include "GLFW/glfw3.h"
#include "../Filesystem.h"
#include "GameState.h"
#include "Session.h"
#include "Setup.h"
#include "Tuner.h"
#include "Audio/notes.h"

Menu::Menu() :
	text("../resources/textures/text_Inconsolata29.dds"),
	selectedItem(0),
	topItem(0)
{

}

void Menu::keyPressed(int key)
{
	if (key == GLFW_KEY_W || key == GLFW_KEY_UP)
		keyUp();

	if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN)
		keyDown();
		
	if (key == GLFW_KEY_PAGE_UP)
		keyPgUp();

	if (key == GLFW_KEY_PAGE_DOWN)
		keyPgDown();

	if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER)
		keyEnter();

	if (key == GLFW_KEY_ESCAPE)
		keyEsc();
}

void Menu::keyUp()
{
	selectedItem--;
	if (selectedItem < 0)
		selectedItem += items.size();
	checkScroll();
}

void Menu::keyDown()
{
	selectedItem++;
	selectedItem %= items.size();
	checkScroll();
}

void Menu::keyPgUp()
{
	selectedItem -= pageItems;
	if (selectedItem < 0)
		selectedItem += items.size();
	checkScroll();
}

void Menu::keyPgDown()
{
	selectedItem += pageItems;
	selectedItem %= items.size();
	checkScroll();
}

void Menu::keyEnter()
{
}

void Menu::keyEsc()
{
}

void Menu::checkScroll()
{
	if (selectedItem >= topItem + pageItems)
		topItem = selectedItem - pageItems + 1;
	if (selectedItem < topItem)
		topItem = selectedItem;
}

void Menu::draw(double time)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	text.print(header.c_str(), headerLeft, headerTop, fontSize);

	size_t line = 0;
	for (size_t item = topItem; item < items.size(); item++)
	{
		if (item == selectedItem)
			text.print(itemMarker, itemsLeft - fontSize / 2, itemsTop - line * fontSize, fontSize);
		text.print(items[item].c_str(), itemsLeft, itemsTop - line * fontSize, fontSize);
		line++;
		if (line == pageItems)
			break;
	}
}

// Main menu

MainMenu::MainMenu()
{
	header = "opensmith";
	items.push_back("Learn");
	items.push_back("Setup");
	items.push_back("Tuner");
	items.push_back("Exit");
}

void MainMenu::keyEnter()
{
	switch (selectedItem)
	{
		case 0: 
			delete gameState;
			gameState = new FileMenu;
			break;
		case 1:
			delete gameState;
			gameState = new Setup;
			break;
		case 2:
			delete gameState;
			gameState = new TuningMenu();
			break;
		case 3:
			delete gameState;
			gameState = 0;
			break;
	}
}

void MainMenu::keyEsc()
{
	delete gameState;
	gameState = 0;
}

// File menu: show all psarc files in dlc directory, select one

FileMenu::FileMenu()
{
	header = "pick a song";
	getFiles(items);
}

void FileMenu::keyEnter()
{
	o.psarcFile = o.psarcDirectory + items[selectedItem];
	delete gameState;
	gameState = new RoleMenu();
}

void FileMenu::keyEsc()
{
	delete gameState;
	gameState = new MainMenu;
}

// Role menu: show available SNG's, select one (lead/rhythm/bass)

RoleMenu::RoleMenu() :
	psarc(o.psarcFile.c_str())
{
	header = "select your path";

	for (size_t i = 0; i < psarc.Entries.size(); i++)
	{
		auto e = psarc.Entries[i];
		if (e->name.find("lead.sng") != std::string::npos)
		{
			itemEntry.push_back(i);
			itemRole.push_back(lead);
			items.push_back("lead");
		}
		if (e->name.find("rhythm.sng") != std::string::npos)
		{
			itemEntry.push_back(i);
			itemRole.push_back(rhythm);
			items.push_back("rhythm");
		}
		if (e->name.find("bass.sng") != std::string::npos)
		{
			itemEntry.push_back(i);
			itemRole.push_back(bass);
			items.push_back("bass");
		}
	}
}

void RoleMenu::keyEnter()
{
	o.sngEntry = itemEntry[selectedItem];
	o.role = itemRole[selectedItem];

	std::vector<char> sngEntryStorage;
	psarc.Entries[o.sngEntry]->Data->readTo(sngEntryStorage);
	std::vector<char> sngStorage;
	SngReader::readTo(sngEntryStorage, sngStorage);
	std::shared_ptr<Sng> s(new Sng);
	s->parse(sngStorage);
	convertTuningAbsolute(s->metadata.tuning);

	delete gameState;
	gameState = new DiffMenu(s);
}

void RoleMenu::keyEsc()
{
	delete gameState;
	gameState = new FileMenu;
}

// Difficulty menu: show SNG overview, select difficulty

DiffMenu::DiffMenu(std::shared_ptr<Sng> sng) :
	sng(sng)
{
	header = "adjust difficulty";
	updateTimeline();
}

void DiffMenu::keyUp()
{
	o.difficulty++;
	updateTimeline();
}

void DiffMenu::keyDown()
{
	if (o.difficulty > 0)
		o.difficulty--;
	updateTimeline();
}

void DiffMenu::keyEnter()
{
	bool needTuner;
	if (o.lastTuning.empty())
		needTuner = true;
	else
	{
		needTuner = false;
		for (size_t n = 0; n < sng->metadata.tuning.size(); n++)
		{
			if (sng->metadata.tuning[n] != o.lastTuning[n])
			{
				needTuner = true;
				break;
			}	
		}
	}
	
	GameState* newState;

	if (!o.skipTuner && needTuner)
		newState = new Tuner(sng->metadata.tuning);
	else
		newState = new Session;

	delete gameState;
	gameState = newState;
}

void DiffMenu::keyEsc()
{
	delete gameState;
	gameState = new MainMenu;
}

void DiffMenu::draw(double time)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	hud.drawTimeline(0);
	std::string message = "difficulty: " + std::to_string(o.difficulty);
	text.print(message.c_str(), 100, 680, 32);
}

void DiffMenu::updateTimeline()
{
	// TODO: this code belongs in hud
	hud.iterations.clear();
	for (auto iteration : sng->PhraseIterations)
	{
		int thisDifficulty = std::min(o.difficulty, iteration.difficulty[2]);
		float barHeight = thisDifficulty / (float)sng->metadata.maxDifficulty;
		bool isMax = (thisDifficulty == iteration.difficulty[2]);
		Hud::Iteration newIteration = { iteration.startTime, 0, barHeight, false, false, isMax };
		if (!hud.iterations.empty())
			hud.iterations.back().endTime = iteration.startTime;
		hud.iterations.push_back(newIteration);
	}
	hud.iterations.back().endTime = sng->metadata.songLength;
	hud.initTimeline(sng->metadata.songLength, 100, 780, 200);
}

// Tuning menu - select tuning before running standalone tuner

TuningMenu::TuningMenu()
{
	header = "tuning?";
	items.push_back("E Standard");
	items.push_back("Eb Standard");
	items.push_back("Drop D");
	items.push_back("Drop C#");
	items.push_back("Drop C");
}

void TuningMenu::keyEnter()
{
	switch (selectedItem)
	{
	case 0:
		delete gameState;
		gameState = new Tuner({ 40, 45, 50, 55, 59, 64 }, true);
		break;
	case 1:
		delete gameState;
		gameState = new Tuner({ 39, 44, 49, 54, 58, 63 }, true);
		break;
	case 2:
		delete gameState;
		gameState = new Tuner({ 38, 45, 50, 55, 59, 64 }, true);
		break;
	case 3:
		delete gameState;
		gameState = new Tuner({ 37, 44, 49, 54, 58, 63 }, true);
		break;
	case 4:
		delete gameState;
		gameState = new Tuner({ 36, 43, 48, 53, 57, 62 }, true);
		break;
	}
}

void TuningMenu::keyEsc()
{
	delete gameState;
	gameState = new MainMenu;
}
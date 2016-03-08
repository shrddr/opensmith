#include "Menu.h"
#include <algorithm>
#include "GLFW/glfw3.h"
#include "GameState.h"
#include "Filesystem.h"
#include "Tuner.h"

Menu::Menu() :
	text("../resources/textures/text_Inconsolata29.dds"),
	selectedItem(0),
	topItem(0)
{

}

void Menu::keyPressed(int key)
{
	if (key == GLFW_KEY_W || key == GLFW_KEY_UP)
	{
		selectedItem--;
		if (selectedItem < 0)
			selectedItem += items.size();
		checkScroll();
	}

	if (key == GLFW_KEY_PAGE_UP)
	{
		selectedItem -= pageLines;
		if (selectedItem < 0)
			selectedItem += items.size();
		checkScroll();
	}

	if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN)
	{
		selectedItem++;
		selectedItem %= items.size();
		checkScroll();
	}
		
	if (key == GLFW_KEY_PAGE_DOWN)
	{
		selectedItem += pageLines;
		selectedItem %= items.size();
		checkScroll();
	}
}

void Menu::checkScroll()
{
	if (selectedItem >= topItem + pageLines)
		topItem = selectedItem - pageLines + 1;
	if (selectedItem < topItem)
		topItem = selectedItem;
}

void Menu::draw(double time)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	size_t line = 0;
	for (size_t item = topItem; item < items.size(); item++)
	{
		if (item == selectedItem)
			text.print(menuMarker, menuLeft - menuFont / 2, menuTop - line * menuFont, menuFont);
		text.print(items[item].c_str(), menuLeft, menuTop - line * menuFont, menuFont);
		line++;
		if (line == pageLines)
			break;
	}
}

// Main menu

MainMenu::MainMenu()
{
	items.push_back("Learn");
	items.push_back("Setup");
	items.push_back("Tuner");
	items.push_back("Exit");
}

void MainMenu::keyPressed(int key)
{
	Menu::keyPressed(key);

	if (key == GLFW_KEY_ENTER && selectedItem == 0)
	{
		delete gameState;
		gameState = new FileMenu;
	}
	if (key == GLFW_KEY_ENTER && selectedItem == 2)
	{
		delete gameState;
		gameState = new Tuner(std::vector<int>(0));
	}
	if ((key == GLFW_KEY_ENTER && selectedItem == 3) || key == GLFW_KEY_ESCAPE)
	{
		delete gameState;
		gameState = 0;
	}
}

// File menu: show all psarc files in dlc directory, select one

FileMenu::FileMenu()
{
	getFiles(items);
}

void FileMenu::keyPressed(int key)
{
	Menu::keyPressed(key);

	if (key == GLFW_KEY_ENTER)
	{
		o.psarcFile = o.psarcDirectory + items[selectedItem];
		std::shared_ptr<PSARC> psarc(new PSARC(o.psarcFile.c_str()));

		delete gameState;
		gameState = new RoleMenu(psarc);
	}

	if (key == GLFW_KEY_ESCAPE)
	{
		delete gameState;
		gameState = new MainMenu;
	}
}

// Role menu: show available SNG's, select one (lead/rhythm/bass)

RoleMenu::RoleMenu(std::shared_ptr<PSARC> psarc) :
	psarc(psarc)
{
	for (size_t i = 0; i < psarc->Entries.size(); i++)
	{
		auto e = psarc->Entries[i];
		if (e->name.find("lead.sng") != std::string::npos)
		{
			itemEntry[items.size()] = i;
			items.push_back("lead");
		}
		if (e->name.find("rhythm.sng") != std::string::npos)
		{
			itemEntry[items.size()] = i;
			items.push_back("rhythm");
		}
		if (e->name.find("bass.sng") != std::string::npos)
		{
			itemEntry[items.size()] = i;
			items.push_back("bass");
		}
	}
}

void RoleMenu::keyPressed(int key)
{
	Menu::keyPressed(key);

	if (key == GLFW_KEY_ENTER)
	{
		o.sngEntry = itemEntry[selectedItem];

		std::vector<char> sngEntryStorage;
		psarc->Entries[o.sngEntry]->Data->readTo(sngEntryStorage);
		std::vector<char> sngStorage;
		SngReader::readTo(sngEntryStorage, sngStorage);
		std::shared_ptr<Sng> s(new Sng);
		s->parse(sngStorage);

		delete gameState;
		gameState = new DiffMenu(s);
	}

	if (key == GLFW_KEY_ESCAPE)
	{
		delete gameState;
		gameState = new FileMenu;
	}
}

// Difficulty menu: show SNG overview, select difficulty

DiffMenu::DiffMenu(std::shared_ptr<Sng> sng) :
	sng(sng)
{
	updateTimeline();
}

void DiffMenu::keyPressed(int key)
{
	if (key == GLFW_KEY_W || key == GLFW_KEY_UP)
	{
		o.difficulty++;
		updateTimeline();
	}

	if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN)
	{
		if (o.difficulty > 0)
			o.difficulty--;
		updateTimeline();
	}

	if (key == GLFW_KEY_ENTER)
	{
		delete gameState;
		gameState = new Session;
	}

	if (key == GLFW_KEY_ESCAPE)
	{
		delete gameState;
		gameState = new MainMenu;
	}
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

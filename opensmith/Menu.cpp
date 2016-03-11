#include "Menu.h"
#include <algorithm>
#include "GLFW/glfw3.h"
#include "GameState.h"
#include "Filesystem.h"
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
	selectedItem -= pageLines;
	if (selectedItem < 0)
		selectedItem += items.size();
	checkScroll();
}

void Menu::keyPgDown()
{
	selectedItem += pageLines;
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
			text.print(menuMarker, menuLeft - fontSize / 2, menuTop - line * fontSize, fontSize);
		text.print(items[item].c_str(), menuLeft, menuTop - line * fontSize, fontSize);
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

void MainMenu::keyEnter()
{
	if (selectedItem == 0)
	{
		delete gameState;
		gameState = new FileMenu;
	}
	if (selectedItem == 1)
	{
		delete gameState;
		gameState = new Setup;
	}
	if (selectedItem == 2)
	{
		delete gameState;
		gameState = new TuningMenu();
	}
	if (selectedItem == 3)
	{
		delete gameState;
		gameState = 0;
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
	getFiles(items);
}

void FileMenu::keyEnter()
{
	o.psarcFile = o.psarcDirectory + items[selectedItem];
	std::shared_ptr<PSARC> psarc(new PSARC(o.psarcFile.c_str()));

	delete gameState;
	gameState = new RoleMenu(psarc);
}

void FileMenu::keyEsc()
{
	delete gameState;
	gameState = new MainMenu;
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

void RoleMenu::keyEnter()
{
	o.sngEntry = itemEntry[selectedItem];

	std::vector<char> sngEntryStorage;
	psarc->Entries[o.sngEntry]->Data->readTo(sngEntryStorage);
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

	if (needTuner)
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
	items.push_back("E Standard");
	items.push_back("Eb Standard");
	items.push_back("Drop D");
}

void TuningMenu::keyEnter()
{
	if (selectedItem == 0)
	{
		delete gameState;
		std::vector<int> tuning;
		tuning.push_back(40);
		tuning.push_back(45);
		tuning.push_back(50);
		tuning.push_back(55);
		tuning.push_back(59);
		tuning.push_back(64);
		gameState = new Tuner(tuning, true);
	}
	if (selectedItem == 1)
	{
		delete gameState;
		std::vector<int> tuning;
		tuning.push_back(39);
		tuning.push_back(44);
		tuning.push_back(49);
		tuning.push_back(54);
		tuning.push_back(58);
		tuning.push_back(63);
		gameState = new Tuner(tuning, true);
	}
	if (selectedItem == 2)
	{
		delete gameState;
		std::vector<int> tuning;
		tuning.push_back(38);
		tuning.push_back(45);
		tuning.push_back(50);
		tuning.push_back(55);
		tuning.push_back(59);
		tuning.push_back(64);
		gameState = new Tuner(tuning, true);
	}
}

void TuningMenu::keyEsc()
{
	delete gameState;
	gameState = new MainMenu;
}
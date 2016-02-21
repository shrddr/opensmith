#include "Settings.h"
#include <fstream>

Settings::Settings()
{
	// gameplay
	difficulty = 8;
	visualsPreloadTime = 30.0f;
	ghostStayTime = 0.2f;
	detectionTimeWindow = 0.15f;
	detectionMinPower = 0.1f; // needs calibrataion

	// visual
	noteStep = 3.0f;
	stringStep = 1.5f;
	zSpeed = 40.0f;

	cameraX = 18.0f;
	cameraY = -12.0f;
	cameraZ = -30.0f;
	cameraHorizontalAngle = 3.0f;
	cameraVerticalAngle = 2.8f;

	// audio
	inputDevice = 0;
	inputDevice = 0;
	playbackVolume = 0.2f;
	instrumentVolume = 1.0f;
}

void Settings::save()
{
	std::ofstream file;
	file.open("settings.txt");

	file << o.inputDevice << std::endl;
	file << o.outputDevice << std::endl;

	file.close();
}

void Settings::load()
{
	std::ifstream file;
	file.open("settings.txt");
	if (!file.good())
		throw std::runtime_error("Settings file not found. Run setup first.");

	file >> inputDevice;
	file >> o.outputDevice;

	file.close();
}

Settings o;
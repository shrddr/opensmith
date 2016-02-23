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
	signalRMS = 0;
	noiseRMS = 0;
	playbackVolume = 0.2f;
	instrumentVolume = 1.0f;
}

void Settings::save()
{
	std::ofstream file;
	file.open("../resources/settings");

	file << inputDevice << std::endl;
	file << outputDevice << std::endl;
	file << signalRMS << std::endl;
	file << noiseRMS << std::endl;

	file.close();
}

void Settings::load()
{
	std::ifstream file;
	file.open("../resources/settings");
	if (!file.good())
		throw std::runtime_error("Settings file not found. Run setup first.\n");

	file >> inputDevice;
	file >> outputDevice;
	file >> signalRMS;
	file >> noiseRMS;

	file.close();
}

Settings o;

#include "Settings.h"
#include <fstream>
#include <stdexcept>

Settings::Settings()
{
	// gameplay
	preferredDifficulty = 30;
	visualsPreloadTime = 30.0f;
	ghostStayTime = 0.2f;
	detectionTimeWindow = 0.15f;
	detectionMinPower = 0.1f; // needs calibrataion

	// visual
	fullScreen = false;

	stringColors[0] = 1.0f; stringColors[1] = 0.1f; stringColors[2] = 0.1f;
	stringColors[3] = 1.0f; stringColors[4] = 1.0f; stringColors[5] = 0.1f;
	stringColors[6] = 0.2f; stringColors[7] = 0.2f; stringColors[8] = 1.0f;
	stringColors[9] = 1.0f; stringColors[10] = 0.7f; stringColors[11] = 0.1f;
	stringColors[12] = 0.1f; stringColors[13] = 1.0f; stringColors[14] = 0.1f;
	stringColors[15] = 0.7f; stringColors[16] = 0.1f; stringColors[17] = 0.7f;

	fretStep = 3.0f;
	stringStep = 1.5f;
	zSpeed = 40.0f;

	cameraX = 18.0f;
	cameraY = -10.0f;
	cameraZ = -30.0f;
	cameraHorizontalAngle = 3.1f;
	cameraVerticalAngle = 2.8f;

	// audio
	inputDevice = 0;
	inputDevice = 0;
	signalRMS = 0;
	noiseRMS = 0;
	playbackVolume = 0.2f;
	instrumentVolume = 1.0f;

	// debug
	dumpSng = false;
	dumpWem = false;
	skipTuner = false;

	// files
	psarcDirectory = "../resources/dlc/";
	psarcFile = "";
	sngEntry = -1;
	difficulty = 30;
	role = any;

	// tuner
	lastTuning.clear();
	precisionCentDelta = 5.0f;
	precisionHoldTime = 3.0f;
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

#pragma once
#include <string>
#include <vector>

enum SngRole { lead, rhythm, bass, any };

struct Settings
{
	Settings();
	void save();
	void load();

	// gameplay
	int preferredDifficulty;
	float visualsPreloadTime;
	float ghostStayTime;
	float detectionTimeWindow;
	float detectionMinPower;

	// visual
	float stringColors[6*3];

	float noteStep;
	float stringStep;
	float zSpeed;

	float cameraX;
	float cameraY;
	float cameraZ;
	float cameraHorizontalAngle;
	float cameraVerticalAngle;

	// audio
	int inputDevice;
	int outputDevice;
	float signalRMS;
	float noiseRMS;
	float playbackVolume;
	float instrumentVolume;

	// debug
	bool dumpSng;
	bool dumpWem;

	// files
	std::string psarcDirectory;
	std::string psarcFile;
	int sngEntry;
	int difficulty;
	SngRole role;

	// tuner
	std::vector<int> lastTuning;
	float precisionCentDelta;
	float precisionHoldTime;
};

extern Settings o;

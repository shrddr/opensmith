#pragma once

enum SngRole { lead, rhythm, bass };

struct Settings
{
	Settings();
	void save();
	void load();

	// gameplay
	int difficulty;
	float visualsPreloadTime;
	float ghostStayTime;
	float detectionTimeWindow;
	float detectionMinPower;

	// visual
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

	// last session
	const char* paramSongFile;
	SngRole paramRole;
};

extern Settings o;

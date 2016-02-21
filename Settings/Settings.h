#pragma once

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
	float playbackVolume;
	float instrumentVolume;
};

extern Settings o;

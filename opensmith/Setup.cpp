#include <GL/glew.h>
#include "Setup.h"
#include <string>
#include <sstream>
#include <algorithm>
#include "util.h"

Setup::Setup() :
	stage(api),
	w(1024),
	a(nullptr),
	testStartTime(0)
{
	PaError err = Pa_Initialize();
	if (err != paNoError)
		throw std::runtime_error("Cannot initialize PortAudio.\n");

	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(vertexArrayId);
	glGenBuffers(1, &vertexBufferId);
	programId = loadShaders("../resources/shaders/waveform.vs",
		"../resources/shaders/waveform.fs");

	showApis();
}

Setup::~Setup()
{
	//a->stop();
	if (a) delete a;

	glDeleteProgram(programId);
	glDeleteBuffers(1, &vertexBufferId);
	glDeleteVertexArrays(1, &vertexArrayId);
}

void Setup::showApis()
{
	header = "audio API";
	PaHostApiIndex HostApi;
	PaHostApiIndex HostApiCount = Pa_GetHostApiCount();
	PaHostApiIndex DefaultHostApi = Pa_GetDefaultHostApi();
	const PaHostApiInfo* HostApiInfo;
	for (int i = 0; i < HostApiCount; i++)
	{
		HostApiInfo = Pa_GetHostApiInfo(i);
		std::stringstream item;
		item << i << ". " << HostApiInfo->name;
		if (i == DefaultHostApi)
		{
			item << " [default]";
			selectedItem = items.size();
		}
		items.push_back(item.str());
		itemData.push_back(i);
	}
}

void Setup::showInputDevices()
{
	header = "input device";
	items.clear();
	itemData.clear();
	PaDeviceIndex DeviceCount = Pa_GetDeviceCount();
	const PaDeviceInfo *deviceInfo;
	for (int i = 0; i < DeviceCount; i++)
	{
		deviceInfo = Pa_GetDeviceInfo(i);
		if ((deviceInfo->hostApi == hostApi) && (deviceInfo->maxInputChannels > 0))
		{
			std::stringstream item;
			item << i << ". " << deviceInfo->name << " [" << deviceInfo->maxInputChannels << "ch]";
			if (i == Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultInputDevice)
			{
				item << " [default]";
				selectedItem = items.size();
			}
			items.push_back(item.str());
			itemData.push_back(i);
		}
	}
}

void Setup::showOutputDevices()
{
	header = "output device";
	items.clear();
	itemData.clear();
	PaDeviceIndex DeviceCount = Pa_GetDeviceCount();
	const PaDeviceInfo *deviceInfo;
	for (int i = 0; i < DeviceCount; i++)
	{
		deviceInfo = Pa_GetDeviceInfo(i);
		if ((deviceInfo->hostApi == hostApi) && (deviceInfo->maxOutputChannels > 0))
		{
			std::stringstream item;
			item << i << ". " << deviceInfo->name << " [" << deviceInfo->maxOutputChannels << "ch]";
			if (i == Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultOutputDevice)
			{
				item << " [default]";
				selectedItem = items.size();
			}
			items.push_back(item.str());
			itemData.push_back(i);
		}
	}
}

void Setup::showTestMax()
{
	header = "make some noise!";
	items.clear();

	a = new Audio(io, 48000);
	a->start();

	const PaStreamInfo* info = a->info();
	if (info == 0)
		throw std::runtime_error("Something went wrong. Probably unsupported device combination.\n");

	inLatency = "Input latency: " + std::to_string(info->inputLatency * 1000) + "ms";
	outLatency = "Output latency: " + std::to_string(info->outputLatency * 1000) + "ms";
}

void Setup::showTestMin()
{
	header = "mute the strings";
}

void Setup::showConfirm()
{
	header = "setup complete";
	items.clear();
	items.push_back("Save");
	items.push_back("Choose again");
}

void Setup::keyEnter()
{
	if (stage == api)
	{
		hostApi = itemData[selectedItem];
		stage = in;
		showInputDevices();
		return;
	}
	if (stage == in)
	{
		o.inputDevice = itemData[selectedItem];
		stage = out;
		showOutputDevices();
		return;
	}
	if (stage == out)
	{
		o.outputDevice = itemData[selectedItem];
		Pa_Terminate();
		stage = testMax;
		o.signalRMS = -200;
		o.noiseRMS = 0;
		showTestMax();
		return;
	}
	if (stage == testMax)
	{
		// wait to complete
	}
	if (stage == confirm)
	{
		if (selectedItem == 0) // ok
		{
			o.save();
			keyEsc();
		}
		if (selectedItem == 1) // restart
		{
			delete gameState;
			gameState = new Setup;
		}
	}
}

void Setup::keyEsc()
{
	delete gameState;
	gameState = new MainMenu;
}

void Setup::draw(double time)
{
	Menu::draw(time);

	if (stage == testMax)
	{
		if (testStartTime == 0)
			testStartTime = time;

		float RMS = w.rms();
		o.signalRMS = std::max(RMS, o.signalRMS);
		signalLevel = "Signal level: " + std::to_string(o.signalRMS) + "db";

		if (time - testStartTime > 5)
		{
			stage = testMin;
			testStartTime = time;
			showTestMin();
		}
	}
		
	if (stage == testMin)
	{
		float RMS = w.rms();
		o.noiseRMS = std::min(RMS, o.signalRMS);
		noiseLevel = "Noise level: " + std::to_string(o.noiseRMS) + "db";

		if (time - testStartTime > 5)
		{
			stage = confirm;
			showConfirm();
		}
	}

	if (stage == testMax || stage == testMin || stage == confirm)
	{
		text.print(inLatency.c_str(), 100, 196, 32);
		text.print(outLatency.c_str(), 100, 164, 32);
		text.print(signalLevel.c_str(), 100, 132, 32);
		text.print(noiseLevel.c_str(), 100, 100, 32);

		glUseProgram(programId);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
		glBufferData(GL_ARRAY_BUFFER, w.size(), w.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_LINES, 0, w.vertexCount());
		glDisableVertexAttribArray(0);
	}	
}





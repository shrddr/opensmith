#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include "Audio/Audio.h"
#include "Audio/NoteDetector.h"
#include "Settings/Settings.h"

void askUser()
{
	std::cout << "Host APIs" << std::endl;
	std::cout << "---------------" << std::endl;
	PaHostApiIndex HostApi;
	PaHostApiIndex HostApiCount = Pa_GetHostApiCount();
	PaHostApiIndex DefaultHostApi = Pa_GetDefaultHostApi();
	const PaHostApiInfo* HostApiInfo;
	for (int i = 0; i < HostApiCount; i++)
	{
		HostApiInfo = Pa_GetHostApiInfo(i);
		std::cout << i << " = " << HostApiInfo->name;
		if (i == DefaultHostApi)
			std::cout << " [default]";
		std::cout << std::endl;
	}
	std::cout << "Select Host API: ";
	std::cin >> HostApi;
	std::cout << std::endl;

	std::cout << "Input Devices" << std::endl;
	std::cout << "---------------" << std::endl;
	PaDeviceIndex DeviceCount = Pa_GetDeviceCount();
	const PaDeviceInfo *deviceInfo;
	for (int i = 0; i < DeviceCount; i++)
	{
		deviceInfo = Pa_GetDeviceInfo(i);
		if ((deviceInfo->hostApi == HostApi) && (deviceInfo->maxInputChannels > 0))
		{
			std::cout << i << " = " << deviceInfo->name
				<< " [" << deviceInfo->maxInputChannels << "ch]";
			if (i == Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultInputDevice)
				std::cout << " [default]";
			std::cout << std::endl;
		}
	}
	std::cout << "Select Input Device: ";
	std::cin >> o.inputDevice;
	std::cout << std::endl;

	std::cout << "Output Devices" << std::endl;
	std::cout << "---------------" << std::endl;
	for (int i = 0; i < DeviceCount; i++)
	{
		deviceInfo = Pa_GetDeviceInfo(i);
		if ((deviceInfo->hostApi == HostApi) && (deviceInfo->maxOutputChannels > 0))
		{
			std::cout << i << " = " << deviceInfo->name
				<< " [" << deviceInfo->maxOutputChannels << "ch]";
			if (i == Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultOutputDevice)
				std::cout << " [default]";
			std::cout << std::endl;
		}
	}
	std::cout << "Select Output Device: ";
	std::cin >> o.outputDevice;
	std::cout << std::endl;
}

bool yes()
{
	char yn;
	std::cin >> yn;
	return (yn == 'y');
}

int main()
{
	Silence s;
	NoteDetector* d = new NoteDetector(48000); // stack overflow
	InOut io{ &s, d, 1, 1 };

	while (true)
	{
		PaError err = Pa_Initialize();
		if (err != paNoError)
		{
			std::cout << err;
			exit(-1);
		}

		askUser();
		Pa_Terminate();

		Audio a(io, 48000);

		const PaStreamInfo* info = a.info();
		if (info == 0)
			continue;
			
		std::cout << "input latency = " << info->inputLatency * 1000 << "ms" << std::endl;
		std::cout << "output latency = " << info->outputLatency * 1000 << "ms" << std::endl;

		a.start();

		std::cout << std::endl << "You should hear your guitar now. Use this device? (y/n) ";
		if (!yes())
			continue;

		const int timeTotal = 5000;
		const int timeDelta = 500;
		int time = 0;
		o.signalRMS = -200;
		o.noiseRMS = 0;

		std::cout << std::endl << "Make some noise!" << std::endl;
		while (time < timeTotal)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(timeDelta));
			time += timeDelta;
			float RMS = d->rms();
			std::cout << RMS << "db" << std::endl;
			o.signalRMS = std::max(RMS, o.signalRMS);
		}

		time = 0;
		std::cout << std::endl << "Mute the strings!" << std::endl;
		while (time < timeTotal)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(timeDelta));
			time += timeDelta;
			float RMS = d->rms();
			std::cout << RMS << "db" << std::endl;
			o.noiseRMS = std::min(RMS, o.noiseRMS);
		}

		float SNR = o.signalRMS - o.noiseRMS;
		std::cout << std::endl << "SNR: " << SNR << "db" << std::endl;
		std::cout << std::endl << "Good enough? (y/n) ";

		if (yes())
			break;
			
	}
		
	o.save();
	delete d;
}
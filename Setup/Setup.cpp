#include <iostream>
#include "Audio/Audio.h"
#include "Settings/Settings.h"

void askUser()
{
	std::cout << "Host APIs" << std::endl;
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
	Discard d;
	InOut io{ &s, &d, 1, 1 };

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

		std::cout << std::endl << "Try it out. Good enough? (y/n) ";

		if (yes())
			break;
	}

	o.save();
	
}
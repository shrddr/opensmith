#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include "Audio.h"
#include "Settings/Settings.h"

float AudioInputBuffer::rms()
{
	float res = 0.0f;
	for (size_t sample = 0; sample < buf.size(); sample++)
		res += buf[sample] * buf[sample];
	res = sqrt(res / buf.size());
	res = 20 * log10(res);
	return res;
}

/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int patestCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	// Cast data passed through stream to our structure.
	InOut* io = (InOut*)userData;
	AudioProducer* producer = io->producer;
	AudioConsumer* consumer = io->consumer;
	float *out = (float*)outputBuffer;
	const float *in = (const float*)inputBuffer;

	for (unsigned long i = 0; i < framesPerBuffer; i++)
	{
		consumer->setPCM(*in); // add timeInfo->inputBufferAdcTime
		float left;
		float right;
		producer->getPCM(timeInfo->outputBufferDacTime - io->startTime, left, right);
		*out++ = left * io->playbackVolume + *in * io->instrumentVolume;
		*out++ = right * io->playbackVolume + *in++ * io->instrumentVolume;
	}

	return 0;
}

Audio::Audio(InOut &io, uint32_t sampleRate)
{
	// Initialize library before making any other calls.
	err = Pa_Initialize();
	if (err != paNoError) report(err);

	PaStreamParameters inputParameters, outputParameters;
	inputParameters.device = o.inputDevice;
	inputParameters.channelCount = 1;
	inputParameters.sampleFormat = paFloat32;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

	outputParameters.device = o.outputDevice;
	outputParameters.channelCount = 2;
	outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;

	// Open an audio I/O stream.
	err = Pa_OpenStream(&stream,
		&inputParameters,
		&outputParameters,
		sampleRate,
		256,        // frames per buffer
		0,
		patestCallback,
		&io);

	if (err != paNoError)
	{
		std::cout << "Pa_OpenStream failed:\n" 
			<< "input device = " << inputParameters.device << "\n"
			<< "input ch = " << inputParameters.channelCount << "\n"
			<< "output device = " << outputParameters.device << "\n"
			<< "output ch = " << outputParameters.channelCount << "\n";
		report(err);
	}
}

double Audio::start()
{
	if (Pa_IsStreamStopped(stream))
	{
		err = Pa_StartStream(stream);
		startTime = Pa_GetStreamTime(stream);
		if (err != paNoError)
			report(err);
	}
	else
		throw std::exception("The stream to start is already running");
	return startTime;
}

void Audio::stop()
{
	err = Pa_StopStream(stream);
	if (err != paNoError) report(err);
}

const PaStreamInfo* Audio::info()
{
	return Pa_GetStreamInfo(stream);
}

Audio::~Audio()
{
	err = Pa_CloseStream(stream);
	if (err != paNoError) report(err);
	Pa_Terminate();
}

void Audio::report(PaError err)
{
	fprintf(stderr, "PA%d: %s\n", err, Pa_GetErrorText(err));
	exit(-1);
}


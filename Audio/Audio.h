#pragma once
#include "portaudio.h"
#include "stdint.h"

struct AudioProducer
{
	virtual void getPCM(const float time, float& left, float& right) = 0;
	virtual void setTime(double time) { };
};

struct Silence : public AudioProducer
{
	void getPCM(const float time, float& left, float& right) { left = right = 0; }
};

struct AudioConsumer
{
	virtual void setPCM(const float) = 0;
};

struct Discard : public AudioConsumer
{
	void setPCM(const float) {}
};

struct InOut
{
	AudioProducer* producer;
	AudioConsumer* consumer;
	float playbackVolume;
	float instrumentVolume;
	double startTime;
};

class Audio
{
public:
	Audio(InOut &io, uint32_t sampleRate);
	~Audio();
	double start();
	void stop();
	const PaStreamInfo* info();
private:
	PaStream* stream;
	PaError err;
	double startTime;
	void report(PaError err);
};


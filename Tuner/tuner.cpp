#include <thread>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <map>
#include "Audio/Audio.h"
#include "Audio/CircularBuffer.h"
extern "C" { 
#include "libfft.h"
}

#define MIDI_A4 69
#define FREQ_A4 440
#define NOTE_DISTANCE 1.059463094359

#define SAMPLE_RATE (48000)
#define FFT_SIZE (8192)
#define FFT_EXP_SIZE (13)

class FreqDetector : public AudioConsumer
{
public:
	FreqDetector(size_t bufferSize) : buf(bufferSize) {}
	void setPCM(const float in) { buf.push_back(in); }
protected:
	CircularBuffer<float> buf;
};

// Find repeating sample sequences using autocorellation, then f = 1/T.
// Resolution at low E string (82,4Hz):
// 48000/82.4=582,47 sample period
// 582/582,47 -> 1.4 cents sharp
// 583/582,47 -> 1.6 cents flat
// at high E string (330Hz):
// 48000/330=145,45 sample period
// 145/145,45 -> 7.3 cents sharp
// 146/145.45 -> 4.5 cents flat
class ACDetector: public FreqDetector
{
public:
	// specify buffer size to fit at least two periods (2*48000/82 -> 1200, even more for bass)
	ACDetector(size_t bufferSize) : 
		FreqDetector(bufferSize),
		bufferSize(bufferSize) {}
	float detect(float minFreq, float maxFreq)
	{
		const float threshold = 0.7f;

		float reference = 0;
		for (size_t sample = 0; sample < bufferSize; sample++)
			reference += buf[sample] * buf[sample];

		int minPeriod = SAMPLE_RATE / maxFreq;
		int maxPeriod = SAMPLE_RATE / minFreq;

		enum PeakDetect { rise, fall } target = rise;
		float sum = reference;
		float sumPrev;

		std::map<size_t, float> peaks;

		for (size_t period = minPeriod; period < maxPeriod; period++)
		{
			sumPrev = sum;
			sum = 0;
			for (size_t sample = 0; sample < bufferSize - period; sample++)
				sum += buf[sample] * buf[sample + period];	

			if (target == fall && sum < sumPrev)
			{
				peaks[period - 1] = sum;
				target = rise;
			}
				
			if (target == rise && sum > reference * threshold && sum > sumPrev)
				target = fall;
		}

		if (peaks.empty()) return NAN;

		size_t maxIndex = 0;
		float maxValue = 0;
		for (auto peak : peaks)
		{
			if (peak.second > maxValue)
			{
				maxIndex = peak.first;
				maxValue = peak.second;
			}
		}	
		
		return (float)SAMPLE_RATE / maxIndex;
	}
private:
	size_t bufferSize;
};

// Perform FFT, find max amplitude.
// This method has poor accuracy, especially at the lower end.
// 48000/8192 = 6hz resolution, at low E (82.4Hz) that's more than a half-step!
// Even at high E (330Hz) that's 31 cents.
// We can condense the frequency domain by decimating the input;
// however, 1/6 second worth of data always gives 6Hz resolution.
class FFTDetector : public FreqDetector
{
public:
	FFTDetector() : FreqDetector(FFT_SIZE)
	{
		for (size_t i = 0; i < FFT_SIZE; i++)
			freq[i] = SAMPLE_RATE * i / (float)FFT_SIZE;
	}
	float detect()
	{
		float dataRe[FFT_SIZE];
		float dataIm[FFT_SIZE];

		for (size_t i = 0; i < FFT_SIZE; i++)
		{
			dataRe[i] = buf[i];
			dataIm[i] = 0.0f;
		}

		fft = initfft(FFT_EXP_SIZE);
		applyfft(fft, dataRe, dataIm, false);

		float maxVal = -1;
		int maxIndex = -1;
		for (size_t i = 0; i < FFT_SIZE / 2; i++)
		{
			float v = dataRe[i] * dataRe[i] + dataIm[i] * dataIm[i];
			if (v > maxVal)
			{
				maxVal = v;
				maxIndex = i;
			}
		}
		float maxFreq = freq[maxIndex];
		return maxFreq;
	}
private:
	float freq[FFT_SIZE];
	void* fft = NULL;
};

int main()
{
	char* NOTES[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	float note[128];
	for (int i = 0; i < 128; i++)
	{
		double x = (i - 69) / 12.0;
		note[i] = 440 * pow(2.0, x);
	}

	Silence s;
	ACDetector d(4096);
	InOut io { &s, &d, 1, 1 };
	Audio a(io, SAMPLE_RATE);
	a.start();

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		float freq = d.detect(70, 400);
		if (isnan(freq))
			continue;

		float noteDiff = 9E9;
		int noteIndex = -1;
		for (size_t i = 0; i < 128; i++)
		{
			float v = fabs(note[i] - freq);
			if (v < noteDiff)
			{
				noteDiff = v;
				noteIndex = i;
			}
		}

		char* noteName = NOTES[noteIndex % 12];
		float cents = 1200 * log2(freq / note[noteIndex]);

		std::cout << freq << " " << noteName << " " << cents << std::endl;
	}
	

	std::cin.get();
	return 0;
}
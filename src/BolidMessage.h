/**
 * \file   BolidMessage.h
 * \author Jan Milik <milikjan@fit.cvut.cz>
 * \date   2014-03-18
 *
 * \brief  Header file for the BolidMessage class.
 */

#ifndef BOLIDMESSAGE_IO4904IL
#define BOLIDMESSAGE_IO4904IL


#include <ctime>


/**
 * \todo Write documentation for class BolidMessage.
 */
struct BolidMessage {
	float minFreq;
	float maxFreq;
	
	int startSample;
	int endSample;
	
	float peakFreq;
	float magnitude;
	float noise;
	
	BolidMessage() :
		minFreq(0), maxFreq(0),
		startSample(0), endSample(0),
		peakFreq(0), magnitude(0), noise(0)
	{
	}
	
	BolidMessage(float minFreq, float maxFreq, int startSample, int endSample,
			   float peakFreq, float magnitude, float noise) :
		minFreq(minFreq), maxFreq(maxFreq),
		startSample(startSample), endSample(endSample),
		peakFreq(peakFreq), magnitude(magnitude), noise(noise)
	{
	}
};


struct HeartBeatMessage {
	time_t timestamp;
};


#endif /* end of include guard: BOLIDMESSAGE_IO4904IL */


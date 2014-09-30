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

#include "WFTime.h"
#include "MessageDispatch.h"


struct NoiseMessage : public Message<NoiseMessage> {
	WFTime time;
	float  noise;
	float  peakFrequency;
	float  magnitude;

	NoiseMessage() :
		time(WFTime::now()),
		noise(0),
		peakFrequency(0),
		magnitude(0)
	{}
	
	NoiseMessage(WFTime time, float noise, float peakFrequency, float magnitude) :
		time(time),
		noise(noise),
		peakFrequency(peakFrequency),
		magnitude(magnitude)
	{}
	
	virtual string toString() {
		ostringstream ss;
		
		ss << "NoiseMessage(" <<
			noise << ", " <<
			peakFrequency << ", " <<
			magnitude << ")";
		
		return ss.str();
	}
};


/**
 * \todo Write documentation for class BolidMessage.
 */
struct BolidMessage : public NoiseMessage {
	float minFreq;
	float maxFreq;
	
	int startSample;
	int endSample;
	
	BolidMessage() :
		NoiseMessage(),
		minFreq(0), maxFreq(0),
		startSample(0), endSample(0)
	{
	}
	
	BolidMessage(WFTime time,
			   float noise, float peakFrequency, float magnitude,
			   float minFreq, float maxFreq, int startSample, int endSample) :
		NoiseMessage(time, noise, peakFrequency, magnitude),
		minFreq(minFreq), maxFreq(maxFreq),
		startSample(startSample), endSample(endSample)
	{
	}
};


struct HeartBeatMessage {
	time_t timestamp;
};


#endif /* end of include guard: BOLIDMESSAGE_IO4904IL */


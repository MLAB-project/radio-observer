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
	int minFreq;
	int maxFreq;
	
	int startSample;
	int endSample;
	
	BolidMessage() :
		minFreq(0), maxFreq(0),
		startSample(0), endSample(0)
	{
	}
	
	BolidMessage(int minFreq, int maxFreq, int startSample, int endSample) :
		minFreq(minFreq), maxFreq(maxFreq),
		startSample(startSample), endSample(endSample)
	{
	}
};


struct HeartBeatMessage {
	time_t timestamp;
};


#endif /* end of include guard: BOLIDMESSAGE_IO4904IL */


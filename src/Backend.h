/**
 * \file   Backend.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-04-23
 *
 * \brief Header file for the Backend class.
 */

#ifndef BACKEND_IFO2SX99
#define BACKEND_IFO2SX99

#include <vector>

using namespace std;

#include <cppapp/cppapp.h>

using namespace cppapp;

#include "WFTime.h"


typedef double SampleType;
typedef long   SampleCount;
typedef int    SampleRate;


/**
 * \brief Complex number.
 */
struct Complex {
	double real;
	double imag;
};


/**
 * \brief Basic metadata for a sample stream.
 */
struct StreamInfo {
	/// Flag signalling whether the total length (number of samples) of the stream is known.
	bool   knownLength;
	/// Length (number of samples) of the stream if it is known, 0 otherwise.
	int    length;
	/// Stream sample rate in samples per second (Hz).
	int    sampleRate;
	
	/// Time offset of the first sample in the stream.
	WFTime timeOffset;
	
	StreamInfo()
	{
		knownLength = false;
		length = 0;
		sampleRate = 48000;
		
		timeOffset = WFTime(0, 0);
	}
};


/**
 * \brief Basic metadata for a sample batch (finite number of samples provided at a time).
 */
struct DataInfo {
	/// Position of the first sample of the batch from the beginning of the stream.
	long   offset;
	
	/// Time offset of the first sample of the batch relative to the stream time offset.
	WFTime timeOffset;
	
	DataInfo()
	{
		offset = 0;
		timeOffset = WFTime(0, 0);
	}
};


/**
 * \brief Base class for backend that take I/Q data and process them.
 */
class Backend : public DIObject {
private:
	Backend(const Backend& other);

protected:
	StreamInfo streamInfo_;
	
public:
	Backend();
	virtual ~Backend() {}
	
	/**
	 * \brief Return the stream information passed to the backend through startStream().
	 */
	StreamInfo getStreamInfo() const { return streamInfo_; }
	
	virtual void startStream(StreamInfo info) { streamInfo_ = info; }
	virtual void process(const vector<Complex> &data, DataInfo info) = 0;
	virtual void endStream() {}
};

#endif /* end of include guard: BACKEND_IFO2SX99 */


/**
 * \file   BolidRecorder.cpp
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-11-03
 * 
 * \brief  Implementation file for the BolidRecorder class.
 */

#include "BolidRecorder.h"
#include "utils.h"


float BolidRecorder::average(float fromFq, float toFq)
{
	int lowBin = backend_->frequencyToBin(fromFq);
	int hiBin  = backend_->frequencyToBin(toFq);
	ORDER_PAIR(lowBin, hiBin);
	
	float *row = buffer_->at(buffer_->mark());
	int    width = hiBin - lowBin;
	
	return average(row + lowBin, width);
}


string BolidRecorder::getFileName(WFTime time)
{
	string typ("blid");
	string origin = backend_->getOrigin();
	return SnapshotRecorder::getFileName(typ, origin, time);
}


void BolidRecorder::start()
{
	LOG_INFO("Bolid detector starting...");
	LOG_INFO("Freq.: " << leftFrequency_ << "-" << rightFrequency_ <<
		    ", detect. freq.: " << minDetectFq_ << "-" << maxDetectFq_ <<
		    ", noise freq.: " << minNoiseFq_ << "-" << maxNoiseFq_
	);
	SnapshotRecorder::start();
}


void BolidRecorder::update()
{
	float *row = buffer_->at(buffer_->mark() - 1);
	
	float n = noise(row + lowNoiseBin_, noiseWidth_);
	int   p = peak(row + lowDetectBin_, detectWidth_);
	float a = average(
		row + lowDetectBin_ + (p - (int)(backend_->frequencyToBin(100) - backend_->frequencyToBin(0))),
		(int)(backend_->frequencyToBin(200) - backend_->frequencyToBin(0))
	);
	
	LOG_DEBUG("n = " << n << ",  p = " << p << ",  a = " << a << ",  detect = " << (a > (n * 6.3)));
	
	if (a > (n * 6.3)) {
		duration_ += 1;
		
		if (!bolidDetected_) {	
			nextSnapshot_.start = buffer_->mark() - (2 * backend_->getFFTSampleRate());
			bolidDetected_ = true;
			bolidRecord_   = true;
		}
	} else if (bolidDetected_) {
		bolidDetected_ = false;
		LOG_WARNING("********** METEOR DETECTED **********");
		LOG_INFO("Magnitude: "  << a <<
			    ", duration: " << duration_);
		duration_ = 0;
	}
	
	if (bolidRecord_) {
		if (buffer_->size(nextSnapshot_.start) >= snapshotRows_ + 2) {
			LOG_DEBUG("Recording meteor...");
			startWriting();
			bolidRecord_ = false;
		}
	}
}


int compareFloat(const void *a, const void *b)
{
	return ( *(float*)a - *(float*)b );
}


float BolidRecorder::noise(float *buffer, int length)
{
	qsort(buffer, length, sizeof(float), compareFloat);
	int quartile = length / 4;
	return buffer[quartile] * 2.0; // * 2 == 3dB
}


int BolidRecorder::peak(float *buffer, int length)
{
	assert(length > 0);
	
	int result = 0;
	
	for (int b = 0; b < length; b++) {
		if (buffer[b] >= buffer[result])
			result = b;
	}
	
	return result;
}


float BolidRecorder::average(float *buffer, int length)
{
	double result = 0.0;
	
	for (float *v = buffer; v < (buffer + length); v++)
		result += (double)*v;
	
	return (float)(result / (double)length);
}


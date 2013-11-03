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


void BolidRecorder::start()
{
	LOG_INFO("Bolid detector starting...");
	SnapshotRecorder::start();
}


void BolidRecorder::update()
{
	float *row = buffer_->at(buffer_->mark());
	
	float n = noise(row + lowNoiseBin_, noiseWidth_);
	float p = peak(row + leftBin_, rightBin_ - leftBin_);
	float a = average(p - 100, p + 100);
	
	if (a > (n * 6.3)) {
		LOG_WARNING("********** METEOR DETECTED **********");
		
		nextSnapshot_.start = buffer_->mark();
		bolidDetected_ = true;
	} else if (bolidDetected_) {
		if (buffer_->size(nextSnapshot_.start) >= snapshotRows_ + 2) {
			LOG_DEBUG("Recording meteor...");
			startWriting();
			bolidDetected_ = false;
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
	return buffer[quartile] * 2; // * 2 == 3dB
}


float BolidRecorder::peak(float *buffer, int length)
{
	assert(length > 0);
	
	float result = buffer[0];
	
	for (float *v = buffer + 1; v < (buffer + length); v++)
		result = max(result, *v);
	
	return result;
}


float BolidRecorder::average(float *buffer, int length)
{
	double result = 0.0;
	
	for (float *v = buffer; v < (buffer + length); v++)
		result += *v;
	
	return (float)(result / (double)length);
}


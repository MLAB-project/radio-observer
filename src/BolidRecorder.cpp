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
	int minFqBin = backend_->frequencyToBin(minDetectFq_);
	int maxFqBin = backend_->frequencyToBin(maxDetectFq_);
	ORDER_PAIR(minFqBin, maxFqBin);
	lowDetectBin_ = minFqBin;
	detectWidth_  = maxFqBin - minFqBin;
	
	minFqBin = backend_->frequencyToBin(minNoiseFq_);
	maxFqBin = backend_->frequencyToBin(maxNoiseFq_);
	
	lowNoiseBin_ = min(minFqBin, maxFqBin);
	noiseWidth_  = max(minFqBin, maxFqBin) - lowNoiseBin_;
	noiseBuffer_.resize(noiseWidth_);
	
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
	
	memcpy(&(noiseBuffer_[0]), row + lowNoiseBin_, sizeof(float) * noiseWidth_);
	float n = noise(&(noiseBuffer_[0]), noiseWidth_);
	//float n = noise(row + lowNoiseBin_, noiseWidth_);
	int   p = peak(row + lowDetectBin_, detectWidth_);
	float a = average(
		row + lowDetectBin_ + (p - (int)(backend_->frequencyToBin(20) - backend_->frequencyToBin(0))),
		(int)(backend_->frequencyToBin(40) - backend_->frequencyToBin(0))
	);
	
	float peak_f = backend_->binToFrequency(lowDetectBin_ + p);
	//if (a > (n * 6.3)) {
	//	std::cerr << "\rDETECTED  " << peak_f << " Hz         ";
	//} else {
	//	std::cerr << "\r          " << peak_f << " Hz         ";
	//}
	//bool detect = (a > (n + 8.0));
	//bool detect = (a > (n * 6.3));
	bool detect = (a > (n * 2.0));
	if (detect) {
		LOG_DEBUG("n = " << std::fixed << std::setprecision(5) << n <<
				",  p = " << std::setw(5) << p <<
				",  a = " << a <<
				",  \033[1;31mdetect = " << detect << "\033[0m");
		//*output_ << "\033[1;31m" << getFailureCount() << " failure(s)\033[0m, ";
	} else {
		LOG_DEBUG("n = " << std::fixed << std::setprecision(5) << n <<
				",  p = " << std::setw(5) << p <<
				",  a = " << a <<
				",  detect = " << detect);
	}
	
	//if (a > (n * 6.3)) {
	if (detect) {
		duration_ += 1;
		
		if (!bolidDetected_) {	
			nextSnapshot_.start = buffer_->mark() - (2 * backend_->getFFTSampleRate());
			bolidDetected_ = true;
			bolidRecord_   = true;
		}
	} else if (bolidDetected_) {
		bolidDetected_ = false;
		LOG_WARNING("********** METEOR DETECTED **********");
		LOG_INFO("Frequency: " << peak_f <<
			    ", magnitude: "  << a <<
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
	float c = ( *(float*)a - *(float*)b );
	if (c > 0.0)
		return 1;
	if (c < 0.0)
		return -1;
	return 0;
}


float BolidRecorder::noise(float *buffer, int length)
{
	qsort(buffer, length, sizeof(float), compareFloat);
	int quartile = length / 4;
	return buffer[quartile] * 2.0; // * 2 == 3dB
	//return log10(buffer[quartile] * 2.0); // * 2 == 3dB
	//return log10(buffer[quartile]) + 3.0; // * 2 == 3dB
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
		//result += log10((double)*v);
		result += (double)*v;
	
	return (float)(result / (double)length);
}


Ref<DIObject> BolidRecorder::make(Ref<DynObject> config, Ref<DIObject> parent)
{
	int   snapshotLength = config->getStrInt("snapshot_length",    60);
	float leftFrequency  = config->getStrDouble("low_freq",      9000);
	float rightFrequency = config->getStrDouble("hi_freq",      12000);
	
	float minDetectFq = config->getStrDouble("low_detect_freq", 10000);
	float maxDetectFq = config->getStrDouble("hi_detect_freq",  10900);
	
	float minNoiseFq = config->getStrDouble("low_noise_freq",    9000);
	float maxNoiseFq = config->getStrDouble("hi_noise_freq",    10000);
	
	return new BolidRecorder(
		parent,
		snapshotLength,
		leftFrequency,
		rightFrequency,
		
		minDetectFq,
		maxDetectFq,
		
		minNoiseFq,
		maxNoiseFq
	);
}

CPPAPP_DI_METHOD("bolid", BolidRecorder, make);



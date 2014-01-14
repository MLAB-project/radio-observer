/**
 * \file   FFTBackend.cpp
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-04-23
 * 
 * \brief Implementation file for the FFTBackend class.
 */

#include "FFTBackend.h"

#include <cassert>
#include <cmath>
#include <cstring>
using namespace std;

#include <cppapp/utils.h>


////////////////////////////////////////////////////////////////////////////////
// IQGainPhaseCorrection
////////////////////////////////////////////////////////////////////////////////


void IQGainPhaseCorrection::setGain(SampleType gain)
{
	gain_ = gain;
}


void IQGainPhaseCorrection::setPhaseShift(int phaseShift)
{
	phaseShift_ = phaseShift;
	buffer_.resize(phaseShift);
}


/**
 * \verbatim
 * section                   |   I    |          II          |  III   |
 * --------------------------+--------+----------------------+--------+
 * inData (I)                |-- 1 ---|---------- 2 ---------|
 * inData (Q)                         |---------- 3 ---------|-- 4 ---|
 * buffer (Q)   |---- 5 -----|-- 6 ---|                      |        |
 * --------------------------------------------------------------------
 * outData (I)               |-- 1 ---|---------- 2 ---------|
 * outData (Q)               |-- 6 ---|---------- 3 ---------|
 * buffer  (Q)                                  |---- 3 -----|-- 4 ---|
 * --------------------------+--------+--------------------------------
 *                           |        |
 *                           phase shift
 * \endverbatim
 */
void IQGainPhaseCorrection::process(const Complex *inData,
							 int            length,
							 Complex       *outData)
{
	// Initialization
	int remaining = length;
	int offset    = 0;
	
	// Section I
	int chunk = phaseShift_;
	if (remaining < chunk) chunk = remaining;
	remaining -= chunk;
	offset    += chunk;
	
	int mark = buffer_.head() - phaseShift_;
	
	for (int i = 0; i < chunk; i++) {
		outData[i].real = inData[i].real;
		outData[i].imag = buffer_.at(mark + i) + gain_;
	}
	
	// Section II
	chunk = remaining - phaseShift_;
	if (chunk > 0) {
		for (int i = 0; i < chunk; i++) {
			outData[offset + i].real = inData[offset + i].real;
			outData[offset + i].imag = inData[i].imag + gain_;
			buffer_.push(inData[i].imag);
		}
		remaining -= chunk;
		offset    += chunk;
	}
	
	// Section III
	if (remaining > 0) {
		for (int i = 0; i < remaining; i++) {
			buffer_.push(inData[offset + i].imag);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// FFTBackend
////////////////////////////////////////////////////////////////////////////////


const double FFTBackend::PI = 4.0 * atan(1.0);


FFTBackend::FFTBackend(int bins, int overlap) :
	Backend(),
	binOverlap_(overlap /* 32768 - 8192 */),
	bins_(bins /* 32768 */)
{
	if (binOverlap_ < 0) binOverlap_ = 0;
	if (binOverlap_ >= bins_) binOverlap_ = bins_ - 1;

	LOG_DEBUG("FFT backend: bins = " << bins_ << ", overlap = " << binOverlap_);
	
	bufferSize_ = sizeof(fftw_complex) * bins_;
	
	windowFn_ = new float[bufferSize_];
	
	window_ = (fftw_complex *) fftw_malloc(bufferSize_);
	in_ = (fftw_complex *) fftw_malloc(bufferSize_);
	out_ = (fftw_complex *) fftw_malloc(bufferSize_);
	fftPlan_ = fftw_plan_dft_1d(bins_, in_, out_, FFTW_FORWARD, FFTW_ESTIMATE);

	//inMark_ = in_ + binOverlap_;
	inMark_ = window_;
	inEnd_ = window_ + bins_;
	
	windowTimes_    = new WFTime[bins_];
	windowRawMarks_ = new int[bins_];
}


FFTBackend::~FFTBackend()
{
	delete [] windowFn_;
	
	fftw_destroy_plan(fftPlan_);
	fftw_free(window_);
	fftw_free(in_);
	fftw_free(out_);
	
	delete windowTimes_;
	windowTimes_ = NULL;
	
	delete windowRawMarks_;
	windowRawMarks_ = NULL;
}


void FFTBackend::startStream(StreamInfo info)
{
	Backend::startStream(info);
	
	inMark_ = window_;
	
	fftSampleRate_ = ((float)info.sampleRate /
				   (float)(bins_ - binOverlap_));
	
	info_ = DataInfo();
	
	rawBuffer_.resize(2, 1024 * 1024, getRawBufferSize());
	
	for (int i = 0; i < bins_; i++) {
		//windowFn_[i] = sin(((float)i / (float)bufferSize_) * PI);
		//windowFn_[i] = 0.5 * (
		//	1.0 -
		//	cos(
		//		2.0 * PI * (float)i /
		//		(float)(bins_ - 1)
		//	)
		//);
		
		float a0 = 0.355768;
		float a1 = 0.487396;
		float a2 = 0.144232;
		float a3 = 0.012604;
		
		windowFn_[i] = (
			a0 -
			a1 * cos(
				2.0 * PI * (float)i /
				(float)(bins_ - 1)
			) +
			a2 * cos(
				4.0 * PI * (float)i /
				(float)(bins_ - 1)
			) -
			a3 * cos(
				6.0 * PI * (float)i /
				(float)(bins_ - 1)
			)
		);
	}
	
	LOG_DEBUG("Starting FFT stream with time offset " << info.timeOffset << ", sample rate " << info.sampleRate << "Hz.");
}


void FFTBackend::process(const vector<Complex> &data, DataInfo info)
{
	assert(sizeof(Complex) == sizeof(in_[0]));
	//assert(binOverlap_ <= (bins_ - binOverlap_));
	
	int size = data.size();
	const Complex *src = &(data[0]);
	
	//for (int i = 0; i < size; i++) {
	//	int16_t *row = rawBuffer_.push();
	//	row[0] = (int16_t)(src[i].real * 0x7fff);
	//	row[1] = (int16_t)(src[i].imag * 0x7fff);
	//}
	
	WFTime timeOffset = info.timeOffset;
	
	// Loop while there is enough remaining data for another FFT window.
	while (size >= (inEnd_ - inMark_)) {
		int count = inEnd_ - inMark_;
		
		// Copy the incoming data to the window buffer
		//memcpy(inMark_, src, count * sizeof(in_[0]));
		correction_.process(src, count, (Complex*)inMark_);
		for (int i = 0; i < count; i++) {
			floatToInt(src[i], rawBuffer_.push());
			windowTimes_[inMark_ - window_ + i]    = timeOffset.addSamples(i, streamInfo_.sampleRate);
			windowRawMarks_[inMark_ - window_ + i] = rawBuffer_.mark();
		}
		
		info_.timeOffset = windowTimes_[0];
		
		// From the window buffer, copy the data to the FFT input buffer, aplying
		// the window function
		for (int i = 0; i < bins_; i++) {
			in_[i][0] = window_[i][0] * windowFn_[i];
			in_[i][1] = window_[i][1] * windowFn_[i];
		}
		
		// Execute FFT
		fftw_execute(fftPlan_);
		
		// Copy the overlap back to the beginning of the window buffer.
		memmove(window_, inEnd_ - binOverlap_, binOverlap_ * sizeof(in_[0]));
		memmove(windowTimes_,    windowTimes_ + bins_ - binOverlap_,    binOverlap_ * sizeof(windowTimes_[0]));
		memmove(windowRawMarks_, windowRawMarks_ + bins_ - binOverlap_, binOverlap_ * sizeof(windowRawMarks_[0]));
		
		// Update variables to keep track of the remaining data/work.
		inMark_ = window_ + binOverlap_;
		size -= count;
		src += count;
		
		// Pass the FFT data to the derived class.
		processFFT(out_, bins_, info_);
		
		timeOffset = timeOffset.addSamples(count, streamInfo_.sampleRate);
		info_.offset++;
	}
	
	// If there are any remaining I/Q samples (but not enough for a complete
	// window, copy them to the window buffer and move the mark.
	if (size > 0) {
		//memcpy(inMark_, src, size * sizeof(in_[0]));
		correction_.process(src, size, (Complex*)inMark_);
		for (int i = 0; i < size; i++) {
			floatToInt(src[i], rawBuffer_.push());
			windowTimes_[inMark_ - window_ + i]    = timeOffset.addSamples(i, streamInfo_.sampleRate);
			windowRawMarks_[inMark_ - window_ + i] = rawBuffer_.mark();
		}
		
		inMark_ += size;
	}
}


void FFTBackend::endStream()
{
	Backend::endStream();
	LOG_DEBUG("Ending FFT stream.");
}



/**
 * \file   FFTBackend.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-04-23
 *
 * \brief Header file for the FFTBackend class.
 */

#ifndef FFTBACKEND_QYQ7WJUZ
#define FFTBACKEND_QYQ7WJUZ

#include <fftw3.h>

#include "Backend.h"
#include "RingBuffer.h"


class IQGainPhaseCorrection {
private:
	SampleType             gain_;
	int                    phaseShift_;
	
	RingBuffer<SampleType> buffer_;

public:
	IQGainPhaseCorrection() :
		gain_(0.0f), phaseShift_(0)
	{}
	
	SampleType getGain() const { return gain_; }
	void setGain(SampleType gain);
	
	int getPhaseShift() const { return phaseShift_; }
	void setPhaseShift(int phaseShift);
	
	void process(const Complex *inData, int length, Complex *outData);
};


/**
 * \brief Base class for backends that compute and process FFT from I/Q signal.
 */
class FFTBackend : public Backend {
private:
	FFTBackend(const FFTBackend& other);
	
	static const double PI;
	
	//int bins_;
	int binOverlap_;
	int bufferSize_; //< buffer size in bytes
	
	IQGainPhaseCorrection correction_;
	
	float        *windowFn_;  //< table containing the values of the window function
	
	fftw_complex *window_;    //< buffer to store incoming I/Q samples
	fftw_complex *inMark_;    //< points to the current position in the FFTBackend::window_ buffer
	fftw_complex *inEnd_;     //< points to the item after the last in the FFTBackend::window_ buffer
	
	fftw_complex *in_, *out_; //< input and output FFT buffers
	fftw_plan     fftPlan_;
	
	DataInfo      info_;
	
protected:
	int   bins_; //< Number of FFT output bins.
	/// Number of FFT results per second (Hz).
	float fftSampleRate_;
	
	virtual void processFFT(const fftw_complex *data, int size, DataInfo info) {}
	
public:
	FFTBackend(int bins, int overlap);
	virtual ~FFTBackend();
	
	/**
	 * \brief Returns number of the FFT bins.
	 */
	int   getBins()          const { return bins_; }
	/**
	 * \brief Returns number of FFT samples (results) per second (in Hz).
	 */
	float getFFTSampleRate() const { return fftSampleRate_; }
	
	SampleType getGain() { return correction_.getGain(); }
	void       setGain(SampleType value) { correction_.setGain(value); }
	
	int        getPhaseShift() { return correction_.getPhaseShift(); }
	void       setPhaseShift(int value) { correction_.setPhaseShift(value); } 
	
	virtual void startStream(StreamInfo info);
	virtual void process(const vector<Complex> &data, DataInfo info);
	virtual void endStream();
	
	inline float binToFrequency(int bin) const
	{
		//return (
		//	(float)streamInfo_.sampleRate *
		//	((2.0 * ((float)bin / (float)bins_)) - 1.0)
		//);

		float b = (float)bin;
		float sr = (float)streamInfo_.sampleRate;
		float n = (float)bins_;
		
		return sr * (-0.5 + b / n);
		//return (b * sr) / n;
	}
	
	float binToFrequency() const
	{
		return (binToFrequency(1) - binToFrequency(0));
		
		//return (
		//	(float)streamInfo_.sampleRate *
		//	(2.0 / (float)bins_)
		//);
	}
	
	int frequencyToBin(float frequency) const
	{
		//int bin = (
		//	(float)bins_ * 0.5 *
		//	((frequency / (float)streamInfo_.sampleRate) + 1.0)
		//);
		//if (bin < 0) return 0;
		//if (bin >= bins_) return bins_ - 1;
		//return bin;
		
		float sr = (float)streamInfo_.sampleRate;
		float n = (float)bins_;
		
		int bin = n * ((frequency / sr) + 0.5);
		if (bin < 0) return 0;
		if (bin >= bins_) return bins_ - 1;
		return bin;
		
		//return (frequency * n) / sr;
	}
};


enum FFTCoordType {
	FFT_COORD_BIN,
	FFT_COORD_FREQ
};


struct FFTCoord {
	FFTCoordType type;
	union {
		float freq;
		int   bin;
	};
	
	FFTCoord() : type(FFT_COORD_BIN), bin(0) {}
	FFTCoord(float freq) : type(FFT_COORD_FREQ), freq(freq) {}
	FFTCoord(int   bin)  : type(FFT_COORD_BIN),  bin(bin)   {}
	
	inline float toFreq(const FFTBackend &backend) const {
		if (type == FFT_COORD_FREQ)
			return freq;
		
		return backend.binToFrequency(bin);
	}
	
	inline int toBin(const FFTBackend &backend) const {
		if (type == FFT_COORD_BIN)
			return bin;
		
		return backend.frequencyToBin(freq);
	}
};


#endif /* end of include guard: FFTBACKEND_QYQ7WJUZ */


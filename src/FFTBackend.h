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

/**
 * \todo Write documentation for class FFTBackend.
 */
class FFTBackend : public Backend {
private:
	FFTBackend(const FFTBackend& other);
	
	static const double PI;
	
	//int bins_;
	int binOverlap_;
	int bufferSize_;
	
	float        *windowFn_;
	
	fftw_complex *window_;
	fftw_complex *in_, *out_;
	fftw_complex *inMark_, *inEnd_;
	fftw_plan     fftPlan_;
	
	DataInfo      info_;
	
protected:
	int   bins_;
	/// Number of FFT results per second (Hz).
	float fftSampleRate_;
	
	virtual void processFFT(const fftw_complex *data, int size, DataInfo info) {}
	
public:
	FFTBackend(int bins, int overlap);
	virtual ~FFTBackend();
	
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

#endif /* end of include guard: FFTBACKEND_QYQ7WJUZ */


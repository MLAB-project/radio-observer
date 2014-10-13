/**
 * \file   FFTBackend.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-04-23
 *
 * \brief Header file for the FFTBackend class.
 */

#ifndef FFTBACKEND_QYQ7WJUZ
#define FFTBACKEND_QYQ7WJUZ


#include <cfloat>

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


struct RawDataHandle {
	int    mark; ///< points to a row \ref FFTBackend::rawBuffer_;
	WFTime time; ///< contains the 
	
	RawDataHandle() : mark(0) {}
	RawDataHandle(int mark, WFTime time) : mark(mark), time(time) {}
};


/**
 * \brief Base class for backends that compute and process FFT from I/Q signal.
 *
 * This class also buffers the raw I/Q data so that any subclasses (\ref
 * WaterfallBackend) can record them.
 */
class FFTBackend : public Backend {
public:
	typedef RingBuffer2D<float> IQBuffer;

private:
	FFTBackend(const FFTBackend& other);
	
	static const double PI;
	
	//int bins_;
	int binOverlap_;
	int bufferSize_; ///< buffer size in bytes
	
	IQGainPhaseCorrection correction_;
	
	float        *windowFn_;  ///< table containing the values of the window function
	
	fftw_complex *window_;    ///< buffer to store incoming I/Q samples
	fftw_complex *inMark_;    ///< points to the current position in the FFTBackend::window_ buffer
	fftw_complex *inEnd_;     ///< points to the item after the last in the FFTBackend::window_ buffer
	
	RawDataHandle *windowRaw_; ///< buffer containing pointers to raw data
	
	fftw_complex *in_, *out_; ///< input and output FFT buffers
	fftw_plan     fftPlan_;   ///< FFT plan
	
	DataInfo      info_; ///< FFT data stream info (as opposed to the raw data stream)
	
	RunningAverage2<double> processingTime_; ///< Running average of FFT calculation times.
	Stopwatch               processingStopwatch_; ///< Processing time stopwatch.
	
protected:
	int   bins_; ///< Number of FFT output bins.
	/// Number of FFT results per second (Hz).
	float fftSampleRate_;
	
	//RingBuffer2D<int16_t> rawBuffer_; ///< contains raw I/Q data
	IQBuffer rawBuffer_; ///< contains raw I/Q data
	
	//virtual int getRawBufferSize() { return 1024; }
	
	virtual void processFFT(const fftw_complex *data, int size, DataInfo info, int rawMark) {}
	
	void clearProcessingTime()
	{
		processingTime_.clear();
	}
	
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
	
	void resizeRawBuffer(int sampleCount)
	{
		rawBuffer_.resize(2, 1024 * 1024, sampleCount);
	}
	
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
	
	/**
	 * \brief Converts number of FFT samples to seconds.
	 *
	 * \param samples number of FFT samples
	 * \returns time in seconds
	 */
	double fftSamplesToTime(int samples)
	{
		return (double)samples / (double)fftSampleRate_;
	}
	
	/**
	 * \brief Converts time in seconds to number of FFT samples.
	 *
	 * \param time time in seconds
	 * \returns number of FFT samples
	 */
	int timeToFFTSamples(double time)
	{
		return time * fftSampleRate_;
	}
	
	double getAverageProcessingTime() { return processingTime_.getValue(); }
	double getMaxProcessingTime() { return processingTime_.max; }
	double getMinProcessingTime() { return processingTime_.min; }
	long   getProcessingCount() { return processingTime_.count; }

	inline static int16_t floatToInt(float f)
	{
		if (f > 1.0) f = 1.0;
		if (f < -1.0) f = -1.0;
		return (int16_t)(f * 0x7fff);
	}
	
	inline static void floatToInt(Complex c, int16_t *result)
	{
		result[0] = (int16_t)(c.real * 0x7fff);
		result[1] = (int16_t)(c.imag * 0x7fff);
	}
	
	inline static void floatToInt(Complex c, float *result)
	{
		result[0] = (float)c.real;
		result[1] = (float)c.imag;
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


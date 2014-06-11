/**
 * \file   BolidRecorder.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-11-03
 *
 * \brief  Header file for the BolidRecorder class.
 */

#ifndef BOLIDRECORDER_MKBLQWES
#define BOLIDRECORDER_MKBLQWES


#include "WaterfallBackend.h"
#include "MessageDispatch.h"
#include "BolidMessage.h"


/**
 * \brief Recorder for \ref WaterfallBackend class that detects and records bolids.
 *
 * This class analyzes FFT data passed to it from \ref WaterfallBackend,
 * attempts to detect bolids and when it does, records them to a FITS file.
 *
 * \note Recommended FFT bin settings for this recorder is: 32728 bins, 31704 bin (fft window) overlap.
 */
class BolidRecorder : public SnapshotRecorder {
public:
	/**
	 * \brief Lists states of the bolid detection finite state machine.
	 */
	enum State {
		STATE_INIT,        ///< Default state, no bolid is being detected.
		STATE_BOLID,       ///< Bolid is being detected.
		STATE_BOLID_ENDED, ///< Bolid has been detected, waiting for the timeout to run out. 
		
		STATE_COUNT        ///< Number of states.
	};

private:
	/**
	 * Copy constructor.
	 */
	BolidRecorder(const BolidRecorder& other);

protected:
	/**
	 * \name Frequency Bounds
	 */
	///@{
	float minNoiseFq_;
	float maxNoiseFq_;
	
	int   lowNoiseBin_;
	int   noiseWidth_;
	
	float minDetectFq_;
	float maxDetectFq_;
	
	int   lowDetectBin_;
	int   detectWidth_;
	///@}
	
	/**
    	 * \name Detection Algorithm Settings
	 */
	///@{
	double advanceTime_; ///< In seconds.
	int    advance_; ///< In FFT rows.
	double jitterTime_; ///< In seconds.
	int    jitter_; ///< In FFT rows.
	float  averageFrequencyRange_; ///< In Hz.
	int    averageBinRange_; ///< In bins.
	float  thresholdRatio_;
	
	string metadataPath_;
	///@}
	
	/**
	 * \name Detection Algorithm Variables
	 */
	///@{
	State state_;
	float peakFreq_;
	int   duration_;
	float noise_;
	float magnitude_;
	
	//bool  bolidDetected_;
	//bool  bolidRecord_;
	
	vector<float> noiseBuffer_;
	
	Ref<Output> metadataFile_;
	
	Ref<Output> getMetadataFile(WFTime time);
	///@}
	
	float average(float fromFq, float toFq);

public:
	/**
	 * \brief Constructor.
	 */
	BolidRecorder();
	/**
	 * \brief Constructor.
	 */
	BolidRecorder(Ref<WaterfallBackend>  backend,
			    int                    snapshotLength,
			    float                  leftFrequency,
			    float                  rightFrequency,
			    string                 outputDir,
			    string                 outputType,
			    bool                   compressOutput,
			    float                  minDetectionFq,
			    float                  maxDetectionFq,
			    float                  minNoiseFq,
			    float                  maxNoiseFq,
			    double                 advanceTime,
			    double                 jitterTime,
			    float                  averageFreqRange,
			    float                  threshold,
			    string                 metadataPath) :
		SnapshotRecorder(backend, snapshotLength, leftFrequency, rightFrequency, outputDir, outputType, compressOutput),
		minNoiseFq_(minNoiseFq),
		maxNoiseFq_(maxNoiseFq),
		lowNoiseBin_(0),
		noiseWidth_(0),
		minDetectFq_(minDetectionFq),
		maxDetectFq_(maxDetectionFq),
		advanceTime_(advanceTime),
		jitterTime_(jitterTime),
		averageFrequencyRange_(averageFreqRange),
		thresholdRatio_(threshold),
		metadataPath_(metadataPath),
		//bolidDetected_(false),
		//bolidRecord_(false),
		duration_(0)
	{
		writeUnfinished_ = false;
		
		ORDER_PAIR(minDetectFq_, maxDetectFq_);
		int minFqBin = backend_->frequencyToBin(minDetectFq_);
		int maxFqBin = backend_->frequencyToBin(maxDetectFq_);
		ORDER_PAIR(minFqBin, maxFqBin);
		lowDetectBin_ = minFqBin;
		detectWidth_  = maxFqBin - minFqBin;
		
		minFqBin = backend_->frequencyToBin(minNoiseFq_);
		maxFqBin = backend_->frequencyToBin(maxNoiseFq_);
		
		lowNoiseBin_ = min(minFqBin, maxFqBin);
		noiseWidth_  = max(minFqBin, maxFqBin) - lowNoiseBin_;
	}
	
	/**
	 * \brief Destructor.
	 */
	virtual ~BolidRecorder() {}
	
	virtual string getMetadataFileName(WFTime time);
	
	virtual void start();
	virtual void update();
	
	static float noise(float *buffer, int length);
	/**
	 * @brief Returns the index of a maximal value in a float buffer.
	 */
	static int   peak(float *buffer, int length);
	/**
	 * @brief Returns the average value of all elements of a float buffer.
	 */
	static float average(float *buffer, int length);
	
	/**
	 * \brief Factory method for \ref BolidRecorder.
	 */
	static Ref<DIObject> make(Ref<DynObject> config, Ref<DIObject> parent);
};


#endif /* end of include guard: BOLIDRECORDER_MKBLQWES */


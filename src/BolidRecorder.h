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


/**
 * \todo Write documentation for class BolidRecorder.
 */
class BolidRecorder : public SnapshotRecorder {
private:
	/**
	 * Copy constructor.
	 */
	BolidRecorder(const BolidRecorder& other);

protected:
	float minNoiseFq_;
	float maxNoiseFq_;
	
	int   lowNoiseBin_;
	int   noiseWidth_;
	
	bool  bolidDetected_;
	
	float average(float fromFq, float toFq);

public:
	/**
	 * Constructor.
	 */
	BolidRecorder();
	BolidRecorder(Ref<WaterfallBackend>  backend,
			    int                    snapshotLength,
			    float                  leftFrequency,
			    float                  rightFrequency,
			    float                  minNoiseFq,
			    float                  maxNoiseFq) :
		SnapshotRecorder(backend, snapshotLength, leftFrequency, rightFrequency),
		minNoiseFq_(minNoiseFq),
		maxNoiseFq_(maxNoiseFq),
		lowNoiseBin_(0),
		noiseWidth_(0),
		bolidDetected_(false)
	{
		int minFqBin = backend_->frequencyToBin(minNoiseFq_);
		int maxFqBin = backend_->frequencyToBin(maxNoiseFq_);
		
		lowNoiseBin_ = min(minFqBin, maxFqBin);
		noiseWidth_  = max(minFqBin, maxFqBin) - lowNoiseBin_;
	}
	
	/**
	 * Destructor.
	 */
	virtual ~BolidRecorder() {}
	
	virtual void start();
	virtual void update();
	
	static float noise(float *buffer, int length);
	static float peak(float *buffer, int length);
	static float average(float *buffer, int length);
};

#endif /* end of include guard: BOLIDRECORDER_MKBLQWES */


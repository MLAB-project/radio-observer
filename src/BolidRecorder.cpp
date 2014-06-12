/**
 * \file   BolidRecorder.cpp
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-11-03
 * 
 * \brief  Implementation file for the BolidRecorder class.
 */


#include "BolidRecorder.h"
#include "utils.h"


Ref<Output> BolidRecorder::getMetadataFile(WFTime time)
{
	string name = getMetadataFileName(time);
	if (metadataFile_.isNull() || (name != metadataFile_->getName())) {
		metadataFile_ = new FileOutput(name);
	}
	return metadataFile_;
}


float BolidRecorder::average(float fromFq, float toFq)
{
	int lowBin = backend_->frequencyToBin(fromFq);
	int hiBin  = backend_->frequencyToBin(toFq);
	ORDER_PAIR(lowBin, hiBin);
	
	float *row = buffer_->at(buffer_->mark());
	int    width = hiBin - lowBin;
	
	return average(row + lowBin, width);
}


string BolidRecorder::getMetadataFileName(WFTime time)
{
	ostringstream ss;
	ss <<
		time.getHour().format("%Y%m%d%H%M%S") <<
		"_" <<
		backend_->getOrigin() <<
		"_meta.csv";
	
	return Path::join(
		metadataPath_,
		ss.str()
		//SnapshotRecorder::getFileBasename(
		//	"meta",
		//	"csv",
		//	backend_->getOrigin(),
		//	time.getHour()
		//)
	);
}


void BolidRecorder::start()
{
	// Precalculate bins from frequencies and rows from times.
	
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
	
	CPPAPP_ASSERT(advanceTime_ >= 0.0);
	CPPAPP_ASSERT(jitterTime_ >= 0.0);
	CPPAPP_ASSERT(averageFrequencyRange_ > 0.0);
	advance_         = backend_->timeToFFTSamples(advanceTime_);
	jitter_          = backend_->timeToFFTSamples(jitterTime_);
	averageBinRange_ = backend_->frequencyToBin(averageFrequencyRange_) - backend_->frequencyToBin(0);
	CPPAPP_ASSERT(averageBinRange_ > 0);
	
	state_ = STATE_INIT;
	
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
	int   p = peak(row + lowDetectBin_, detectWidth_);
	float a = average(
		//row + lowDetectBin_ + (p - (int)(backend_->frequencyToBin(20) - backend_->frequencyToBin(0))),
		//(int)(backend_->frequencyToBin(40) - backend_->frequencyToBin(0))
		
		row + lowDetectBin_ + p - averageBinRange_ / 2,
		averageBinRange_
	);
	
	bool detect = (a > (n * 2.0));
	
	switch (state_) {
	case STATE_INIT:
		if (detect) {
			peakFreq_  = backend_->binToFrequency(lowDetectBin_ + p);
			noise_ = n;
			magnitude_ = a;
			duration_ = 1;
			nextSnapshot_.start = buffer_->mark() - advance_;
			nextSnapshot_.length = 2 * advance_;
			nextSnapshot_.fileName = getFileName(nextSnapshot_.start);
			state_ = STATE_BOLID;
		}
		break;
	
	case STATE_BOLID:
		if (detect) {
			duration_ += 1;
		} else {
			nextSnapshot_.length += duration_;
			duration_ = 1;
			state_ = STATE_BOLID_ENDED;
		}
		break;
	
	case STATE_BOLID_ENDED:
		duration_ += 1;
		
		if (detect) {
			state_ = STATE_BOLID;
		} else {
			if (duration_ >= jitter_) {
				WFTime t = WFTime::now();
				Ref<Output> metaf = getMetadataFile(t);
				float duration = (float)(nextSnapshot_.length - 2 * advance_) / (float)backend_->getFFTSampleRate();
				(*metaf->getStream())
					<< nextSnapshot_.fileName
					//<< metaf->getName()
					<< ";" << noise_
					<< ";" << peakFreq_
					<< ";" << magnitude_
					<< ";" << duration
					<< std::endl;
				metaf->getStream()->flush();
				
				MessageDispatch<BolidMessage>::getInstance().sendMessage(BolidMessage(
					peakFreq_ - (maxDetectFq_ - minDetectFq_) / 4,
					peakFreq_ + (maxDetectFq_ - minDetectFq_) / 4,
					//nextSnapshot_.start,
					//nextSnapshot_.start + nextSnapshot_.length,
					0,
					fftSamplesToRaw(nextSnapshot_.length),
					
					peakFreq_,
					magnitude_,
					noise_
				));
				
				LOG_WARNING("************** METEOR DETECTED **************");
				LOG_INFO("Duration: " << duration << "s" <<
					    "  |  Frequency: " << peakFreq_ << "Hz");
				nextSnapshot_.includeRawData = true;
				startWriting();
				state_ = STATE_INIT;
			}
		}
		break;
	
	default:
		LOG_ERROR("Internal error: unknown state " << state_ << " in bolid detector!");
		state_ = STATE_INIT;
		break;
	}
	
	//if (detect) {
	//	duration_ += 1;
	//	
	//	if (!bolidDetected_) {
	//		nextSnapshot_.start = buffer_->mark() - (2 * backend_->getFFTSampleRate());
	//		bolidDetected_ = true;
	//		bolidRecord_   = true;
	//	}
	//} else if (bolidDetected_) {
	//	bolidDetected_ = false;
	//	LOG_WARNING("********** METEOR DETECTED **********");
	//	LOG_INFO("Frequency: " << peak_f <<
	//		    ", magnitude: "  << a <<
	//		    ", duration: " << duration_);
	//	duration_ = 0;
	//}
	//
	//if (bolidRecord_) {
	//	if (buffer_->size(nextSnapshot_.start) >= snapshotRows_ + 2) {
	//		LOG_DEBUG("Recording meteor...");
	//		startWriting();
	//		bolidRecord_ = false;
	//	}
	//}
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


/**
 * The config values this method expects in \c parent are:
 * \li \c snapshot_length
 * \li \c low_freq
 * \li \c hi_freq
 * \li \c low_detect_freq
 * \li \c hi_detect_freq
 * \li \c low_noise_freq
 * \li \c hi_noise_freq
 */
Ref<DIObject> BolidRecorder::make(Ref<DynObject> config, Ref<DIObject> parent)
{
	string outputDir      = config->getStrString("output_dir", ".");
	string outputType     = config->getStrString("output_type", "blid");
	bool   compressOutput = config->getStrBool("compress_output", true);
	
	int    snapshotLength = config->getStrInt("snapshot_length",    60);
	float  leftFrequency  = config->getStrDouble("low_freq",      9000);
	float  rightFrequency = config->getStrDouble("hi_freq",      12000);
	
	float minDetectFq = config->getStrDouble("low_detect_freq", 10000);
	float maxDetectFq = config->getStrDouble("hi_detect_freq",  10900);
	
	float minNoiseFq = config->getStrDouble("low_noise_freq",    9000);
	float maxNoiseFq = config->getStrDouble("hi_noise_freq",    10000);
	
	double advanceTime      = config->getStrDouble("advance_time",     1);
	double jitterTime       = config->getStrDouble("jitter_time",      1);
	float  averageFreqRange = config->getStrDouble("avg_freq_range",  40);
	float  thresholdRatio   = config->getStrDouble("threshold",      2.0);
	
	string metadataPath     = config->getStrString("metadata_path",  ".");
	
	Ref<BolidRecorder> result = new BolidRecorder(
		parent,
		snapshotLength,
		leftFrequency,
		rightFrequency,
		
		outputDir,
		outputType,
		compressOutput,
		
		minDetectFq,
		maxDetectFq,
		
		minNoiseFq,
		maxNoiseFq,
		
		advanceTime,
		jitterTime,
		averageFreqRange,
		thresholdRatio,
		
		metadataPath
	);
	
	return result;
}

CPPAPP_DI_METHOD("bolid", BolidRecorder, make);



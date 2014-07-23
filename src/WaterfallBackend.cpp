/**
 * \file   WaterfallBackend.cpp
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-04-25
 * 
 * \brief Implementation file for the WaterfallBackend class.
 */

#include "WaterfallBackend.h"
#include "config.h"

#include <cppapp/Logger.h>

#include <iostream>
using namespace std;


////////////////////////////////////////////////////////////////////////////////
// RECORDER
////////////////////////////////////////////////////////////////////////////////


int Recorder::getSampleRate()
{
	return backend_->getStreamInfo().sampleRate;
}


int Recorder::getFFTSampleRate()
{
	return backend_->getFFTSampleRate();
}


int Recorder::fftMarkToRaw(int mark)
{
	int wrapped = wrap(mark, rawHandles_->size());
	return rawHandles_->at(wrapped).mark;
}


WFTime Recorder::fftMarkToTime(int mark)
{
	int wrapped = wrap(mark, rawHandles_->size());
	return rawHandles_->at(wrapped).time;
}


int Recorder::fftSamplesToRaw(int sampleCount)
{
	return ((double)sampleCount / (double)getFFTSampleRate()) * (double)getSampleRate();
}


////////////////////////////////////////////////////////////////////////////////
// SNAPSHOT RECORDER
////////////////////////////////////////////////////////////////////////////////


void* SnapshotRecorder::threadMethod()
{
	vector<Snapshot> received;
	bool             work = true;
	
	while (work) {
		work = snapshots_.drain(received);
		
		FOR_EACH(received, snapshot) {
			if (buffer_->mark() >= snapshot->end()) {
				write(*snapshot);
				if (snapshot->includeRawData)
					writeRaw(*snapshot);
				
				{
					MutexLock bufferLock(bufferMutex_);
					buffer_->freeReservation(snapshot->reservation);
				}
			} else {
				snapshots_.send(*snapshot);
			}
		}
		received.clear();
	}
	
	return NULL;
}


void SnapshotRecorder::startWriting()
{
	{
		MutexLock bufferLock(bufferMutex_);
		
		if (nextSnapshot_.length == 0)
			nextSnapshot_.length = buffer_->size(nextSnapshot_.start);
		if (snapshotRows_ < nextSnapshot_.length)
			nextSnapshot_.length = snapshotRows_;
		int end = nextSnapshot_.start + nextSnapshot_.length;
		
		nextSnapshot_.reservation = buffer_->reserve(nextSnapshot_.start, end);
	}
	
	snapshots_.send(nextSnapshot_);
	nextSnapshot_ = Snapshot(nextSnapshot_.end());
	nextSnapshot_.fileName = getFileName(nextSnapshot_.start);
}


/**
 * \brief Writes a specified snapshot to a FITS file.
 */
void SnapshotRecorder::write(Snapshot snapshot)
{
	WFTime time   = fftMarkToTime(snapshot.start);
	string origin = backend_->getOrigin();
	
	int start  = snapshot.start;
	int length = snapshot.length;
	
	float fftSampleRate = backend_->getFFTSampleRate();
	
	string fileName = "!";
	//fileName += getFileName(time);
	fileName += snapshot.fileName;
	
	LOG_INFO("Writing snapshot \"" << (fileName.c_str() + 1) << "\"...");
	
	FITSWriter w;
	
	if (compressOutput_)
		fileName = fileName + "[compress]";
	if (!w.open(fileName.c_str()))
		return;
	
	int width = rightBin_ - leftBin_;
	w.createImage(width, length, FLOAT_IMG);
	//w.createImage(width, length, SHORT_IMG);
	
	w.comment("File created by " PACKAGE_STRING ".");
	w.comment("See " PACKAGE_URL ".");
	w.writeHeader("ORIGIN", origin.c_str(), "");
	w.date();
	w.comment(WFTime::now().format("Local time: %Y-%m-%d %H:%M:%S %Z", true).c_str());
	w.writeHeader("DATE-OBS", time.format("%Y-%m-%dT%H:%M:%S").c_str(), "observation date (UTC)");
	
	w.writeHeader("CTYPE2", "TIME",                      "in seconds");
	w.writeHeader("CRPIX2", 1,                           ""          );
	w.writeHeader("CRVAL2", (float)time.seconds(),       ""          );
	w.writeHeader("CDELT2", 1.f / (float)fftSampleRate,  ""          );
	
	w.writeHeader("CTYPE1", "FREQ",                             "in Hz");
	w.writeHeader("CRPIX1", 1.f,                                ""     );
	w.writeHeader("CRVAL1", (float)leftFrequency_,              ""     );
	w.writeHeader("CDELT1", (float)backend_->binToFrequency(),  ""     );
	
	w.checkStatus("Error occured while writing FITS file header.");
	
	//int16_t *row = new int16_t[width];
	
	int rowIndex = start;
	for (int y = 0; y < length; y++, rowIndex++) {
		//float *srcRow = buffer_->at(rowIndex);
		//for (int x = 0; x < width; x++) {
		//	row[x] = FFTBackend::floatToInt(*(srcRow + leftBin_ + x) / (double)backend_->getBins());
		//}
		//w.write(y, 1, row);
		w.write(y, 1, (float*)(buffer_->at(rowIndex) + leftBin_));
	}
	
	//delete [] row;
	
	w.checkStatus("Error occured while writing data to a FITS file.");
	w.close();
	
	LOG_DEBUG("Finished writing snapshot.");
}


void SnapshotRecorder::writeRaw(Snapshot snapshot)
{
	int start  = fftMarkToRaw(snapshot.start);
	int length = fftSamplesToRaw(snapshot.length);
	
	WFTime time   = fftMarkToTime(snapshot.start);
	string origin = backend_->getOrigin();
	
	float fftSampleRate = backend_->getFFTSampleRate();
	
	string fileName = "!";
	fileName += getFileName("raws", "fits", time);
	
	LOG_INFO("Writing raw snapshot \"" << (fileName.c_str() + 1) << "\"...");
	
	FITSWriter w;
	
	//fileName = fileName + "[compress R 2,200]";
	if (!w.open(fileName.c_str()))
		return;
	
	w.createImage(2, length, FLOAT_IMG);
	//w.createImage(2, length, SHORT_IMG);
	
	w.comment("File created by " PACKAGE_STRING ".");
	w.comment("See " PACKAGE_URL ".");
	w.writeHeader("ORIGIN", origin.c_str(), "");
	w.date();
	w.comment(WFTime::now().format("Local time: %Y-%m-%d %H:%M:%S %Z", true).c_str());
	w.writeHeader("DATE-OBS", time.format("%Y-%m-%dT%H:%M:%S").c_str(), "observation date (UTC)");
	
	w.writeHeader("CTYPE2", "TIME",                      "in seconds");
	w.writeHeader("CRPIX2", 1,                           ""          );
	w.writeHeader("CRVAL2", (float)time.seconds(),       ""          );
	w.writeHeader("CDELT2", 1.f / (float)fftSampleRate,  ""          );
	
	w.writeHeader("CTYPE1", "FREQ",                             "in Hz");
	w.writeHeader("CRPIX1", 1.f,                                ""     );
	w.writeHeader("CRVAL1", (float)leftFrequency_,              ""     );
	w.writeHeader("CDELT1", (float)backend_->binToFrequency(),  ""     );
	
	w.checkStatus("Error occured while writing FITS file header.");
	
	int rowIndex = start;
	for (int y = 0; y < length; y++, rowIndex++) {
		w.write(y, 1, rawBuffer_->at(rowIndex));
	}
	
	w.checkStatus("Error occured while writing data to a FITS file.");
	w.close();
	
	LOG_DEBUG("Finished writing raw snapshot.");
}


string SnapshotRecorder::getFileName(int mark)
{
	WFTime time = fftMarkToTime(mark);
	return getFileName(time);
}


string SnapshotRecorder::getFileName(WFTime time)
{
	string origin = backend_->getOrigin();
	return getFileName(outputType_, origin, time);
}


string SnapshotRecorder::getFileName(const char *typ, const char *ext, WFTime time)
{
	string origin = backend_->getOrigin();
	return getFileBasename(typ, ext, origin, time);
}


string SnapshotRecorder::getFileName(const string &typ,
							  const string &origin,
							  WFTime       time)
{
	return getFileBasename(typ.c_str(), "fits", origin, time);

	//char fileName[1024];
	//sprintf(fileName, "%s%03d_%s_%s.fits",
	//	   time.format("%Y%m%d%H%M%S").c_str(),
	//	   (int)(time.microseconds() / 1000),
	//	   origin.c_str(),
	//	   typ.c_str());
	//return Path::join(
	//	outputDir_,
	//	string(fileName)
	//);
}


string SnapshotRecorder::getFileBasename(const char   *typ,
								 const char   *ext,
                                         const string &origin,
                                         WFTime        time)
{
	char fileName[1024];
	sprintf(fileName, "%s%03d_%s_%s.%s",
		   time.format("%Y%m%d%H%M%S").c_str(),
		   (int)(time.microseconds() / 1000),
		   origin.c_str(),
		   typ,
		   ext);
	return Path::join(
		outputDir_,
		string(fileName)
	);
}


int SnapshotRecorder::requestBufferSize()
{
	float fftSampleRate = backend_->getFFTSampleRate();
	snapshotRows_ = (int)ceil(snapshotLength_ * fftSampleRate);
	if (snapshotRows_ < 1) {
		snapshotRows_ = 1;
	}
	
	return snapshotRows_ * 8;
	
	//float realLength = (float)snapshotRows_ / fftSampleRate;
	//
	//int bufferSize = (int)ceil(snapshotLength_ * fftSampleRate_);
	//if (bufferSize < 1) {
	//	bufferSize = 1;
	//	LOG_WARNING("Waterfall backend: buffer size too small, using buffer size = " << bufferSize);
	//}
	//float realLength = (float)bufferSize / fftSampleRate_;
	//LOG_DEBUG("Waterfall backend: snapshot length = " << snapshotLength_ << "s" <<
	//		", FFT sample rate = " << fftSampleRate_ << "Hz" <<
	//		", buffer size (length * sample rate) = " << bufferSize << " samples" << 
	//		", real snapshot length = " << realLength << "s");
}


void SnapshotRecorder::start()
{
	LOG_INFO("Snapshot recording starting...");
	
	if (leftFrequency_ == rightFrequency_) {
		StreamInfo info = backend_->getStreamInfo();
		
		leftFrequency_ = -(float)info.sampleRate / 2.0;
		rightFrequency_ = (float)info.sampleRate / 2.0;
		leftBin_  = 0;
		rightBin_ = backend_->getBins();
	} else {
		leftBin_  = backend_->frequencyToBin(leftFrequency_);
		rightBin_ = backend_->frequencyToBin(rightFrequency_);
		//leftBin_  = frequencyToBin(leftFrequency_);
		//rightBin_ = frequencyToBin(rightFrequency_);
	}
	
	//if (leftFrequency_ == rightFrequency_) {
	//	StreamInfo info = backend_->getStreamInfo();
	//	
	//	leftFrequency_  = -(float)info.sampleRate;
	//	rightFrequency_ =  (float)info.sampleRate;
	//	leftBin_  = 0;
	//	rightBin_ = backend_->getBins();
	//} else {
	//	leftBin_  = backend_->frequencyToBin(leftFrequency_);
	//	rightBin_ = backend_->frequencyToBin(rightFrequency_);
	//}
	
	nextSnapshot_ = Snapshot(0);
	workerThread_ = new Thread(this, &SnapshotRecorder::threadMethod);
}


void SnapshotRecorder::stop()
{
	if ((buffer_->size(nextSnapshot_.start) >= 0) && writeUnfinished_)
		startWriting();
	
	snapshots_.close();
	
	// Wait for the worker thread to stop and release the
	// resources.
	workerThread_->join();
	delete workerThread_;
	workerThread_ = NULL;
}


void SnapshotRecorder::update()
{
	if (buffer_->size(nextSnapshot_.start) >= snapshotRows_ + 2) {
		LOG_DEBUG("Snapshot full [start_: " << nextSnapshot_.start <<
				", snapshotRows_: " << snapshotRows_ <<
				", snapshotLength_: " << snapshotLength_ <<
				", buffer_->size(start_): " << buffer_->size(nextSnapshot_.start) <<
				"].");
		startWriting();
	}
}


/**
 * The config values this method expects in \c parent are:
 * \li \c snapshot_length
 * \li \c low_freq
 * \li \c hi_freq
 */
Ref<DIObject> SnapshotRecorder::make(Ref<DynObject> config, Ref<DIObject> parent)
{
	string outputDir      = config->getStrString("output_dir", ".");
	string outputType     = config->getStrString("output_type", "snap");
	bool   compressOutput = config->getStrBool("compress_output", true);
	
	int    snapshotLength = config->getStrInt("snapshot_length", 60);
	float  leftFrequency  = config->getStrDouble("low_freq", 0);
	float  rightFrequency = config->getStrDouble("hi_freq",  0);
	
	return new SnapshotRecorder(
		parent,
		snapshotLength,
		leftFrequency,
		rightFrequency,
		outputDir,
		outputType,
		compressOutput
	);
}

CPPAPP_DI_METHOD("snapshot", SnapshotRecorder, make);


////////////////////////////////////////////////////////////////////////////////
// WATERFALL BACKEND
////////////////////////////////////////////////////////////////////////////////


void WaterfallBackend::processFFT(const fftw_complex *data, int size, DataInfo info, int rawMark)
{
	//float *row = inBuffer_.addRow(info.timeOffset);
	float *row = buffer_.push();
	int    halfSize = size / 2;
	
	// Left half (0 -- half)
	for (int i = 0; i < halfSize; i++) {
		row[halfSize + i] = sqrt(
			data[i][0] * data[i][0] +
			data[i][1] * data[i][1]
		);
	}
	
	// Right half (half -- size)
	for (int i = halfSize; i < size; i++) {
		row[i - halfSize] = sqrt(
			data[i][0] * data[i][0] +
			data[i][1] * data[i][1]
		);
	}

	rawHandles_[buffer_.mark()] = RawDataHandle(rawMark, info.timeOffset);
	
	//LOG_DEBUG("Data stream time: " << info.timeOffset.format("%Y-%m-%d  %H:%M:%S"));
	
	//// Left half (0 -- half)
	//for (int i = 0; i < halfSize; i++) {
	//	row[halfSize - i - 1] = sqrt(
	//		data[i][0] * data[i][0] +
	//		data[i][1] * data[i][1]
	//	);
	//}
	//
	//// Right half (half -- size)
	//for (int i = halfSize; i < size; i++) {
	//	row[size - (i - halfSize) - 1] = sqrt(
	//		data[i][0] * data[i][0] +
	//		data[i][1] * data[i][1]
	//	);
	//}
	
	//for (int i = 0; i < size; i++) {
	//	row[i] = sqrt(
	//		data[i][0] * data[i][0] +
	//		data[i][1] * data[i][1]
	//	);
	//}
	
	FOR_EACH(recorders_, it) {
		(*it)->update();
	}
	
	//if (inBuffer_.isFull()) {
	//	startSnapshot();
	//}
}


WaterfallBackend::WaterfallBackend(int    bins,
                                   int    overlap,
                                   string origin) :
	FFTBackend(bins, overlap),
	origin_(origin)
{
}


WaterfallBackend::~WaterfallBackend()
{
	FOR_EACH(recorders_, it) {
		*it = NULL;
	}
	recorders_.clear();
}


void WaterfallBackend::addRecorder(Ref<Recorder> recorder)
{
	recorders_.push_back(recorder);
	recorder->setBuffer(&buffer_, &rawBuffer_, &bufferMutex_, &rawHandles_);
}


/**
 *
 */
void WaterfallBackend::startStream(StreamInfo info)
{
	FFTBackend::startStream(info);
	
	int bufferSize = 1;
	FOR_EACH(recorders_, it) {
		int requested = (*it)->requestBufferSize();
		if (requested > bufferSize)
			bufferSize = requested;
	}
	
	// TODO: Make the chunk size an config option.
	buffer_.resize(getBins(), WATERFALL_BACKEND_CHUNK_SIZE, bufferSize);
	rawHandles_.resize(buffer_.getCapacity());
	
	resizeRawBuffer(fftSamplesToRaw(bufferSize));
	LOG_DEBUG("Number of raw samples in the buffer = " << fftSamplesToRaw(bufferSize));
	
	FOR_EACH(recorders_, it) {
		(*it)->start();
	}
}


/**
 *
 */
void WaterfallBackend::endStream()
{
	FFTBackend::endStream();
	
	FOR_EACH(recorders_, it) {
		(*it)->stop();
	}
}


bool WaterfallBackend::injectDependency(Ref<DIObject> obj, std::string key)
{
	if (key.compare("recorder") == 0) {
		addRecorder(obj.as<Recorder>());
	}
	
	return FFTBackend::injectDependency(obj, key);
}


Ref<DIObject> WaterfallBackend::make(Ref<DynObject> config, Ref<DIObject> parent)
{
	int bins      = config->getStrInt("bins",        32768);
	int overlap   = config->getStrInt("overlap",         0);
	string origin = config->getStrString("origin", "debug");
	
	Ref<WaterfallBackend> backend = new WaterfallBackend(
		bins,
		overlap,
		origin
	);
	
	backend->setGain(
		config->getStrDouble("iq_gain", 0));
	backend->setPhaseShift(
		config->getStrInt("iq_phase_shift", 0));
	
	return backend;
}

CPPAPP_DI_METHOD("waterfall", WaterfallBackend, make);



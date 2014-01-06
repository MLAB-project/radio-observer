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
// WATERFALL RECORDER
////////////////////////////////////////////////////////////////////////////////


void* WaterfallRecorder::workerThreadMethod()
{
	MutexLock lock(&mutex_);
	
	while (!exitWorkerThread_) {
		condition_.wait(mutex_);
		if (exitWorkerThread_) break;
		// TODO: Process data, make create new files as necessary.
	}
	
	return NULL;
}


void WaterfallRecorder::resize(int bins, long maxBufferSize)
{
	int rows = inputBuffer_.autoResize(bins, maxBufferSize);
	outputBuffer_.resize(rows, bins);
	
	currentRow_ = 0;
}


void WaterfallRecorder::start()
{
	workerThread_ = new Thread(this, &WaterfallRecorder::workerThreadMethod);
}


void WaterfallRecorder::stop()
{
	// Lock the mutex, necessary for the condition variable.
	MutexLock lock(&mutex_);
	
	// Signal the worker thread to exit the work loop.
	exitWorkerThread_ = true;
	condition_.signal();
	
	// Wait for the worker thread to stop and release the
	// resources.
	workerThread_->join();
	delete workerThread_;
	workerThread_ = NULL;
}


float* WaterfallRecorder::addRow(WFTime time)
{
	// If the input buffer is full, swap buffers
	// and send signal to worker thread that the 
	// output buffer is ready to be written to
	// file.
	if (inputBuffer_.isFull()) {
		MutexLock lock(&mutex_);
		
		inputBuffer_.swap(outputBuffer_);
		condition_.signal();
		
		inputBuffer_.rewind();
	}
	
	// Add row to the input buffer and return
	// pointer to it.
	return inputBuffer_.addRow(time);
}


////////////////////////////////////////////////////////////////////////////////
// RECORDER
////////////////////////////////////////////////////////////////////////////////


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
			write(*snapshot);

			{
				MutexLock bufferLock(bufferMutex_);
				buffer_->freeReservation(snapshot->reservation);
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
		
		nextSnapshot_.length = buffer_->size(nextSnapshot_.start);
		if (snapshotRows_ < nextSnapshot_.length)
			nextSnapshot_.length = snapshotRows_;
		int end = nextSnapshot_.start + nextSnapshot_.length;
		
		nextSnapshot_.reservation = buffer_->reserve(nextSnapshot_.start, end);
	}
	
	snapshots_.send(nextSnapshot_);
	nextSnapshot_ = Snapshot(nextSnapshot_.end());
}


void SnapshotRecorder::write(Snapshot snapshot)
{
	// WFTime time = outBuffer_.times[0];
	// TODO: This is a hotfix, needs to be debugged and fixed later.
	// Original code:
	// WFTime time = outBuffer_.times[0];
	// Hotfix:
	WFTime time   = WFTime::now();
	string origin = backend_->getOrigin();
	
	int start  = snapshot.start;
	int length = snapshot.length;
	
	float fftSampleRate = backend_->getFFTSampleRate();
	
	string fileName = getFileName(time);
	//char *fileName = new char[1024];
	//sprintf(fileName, "!snapshot_%s_%s.fits",
	//	   origin.c_str(),
	//	   time.format("%Y_%m_%d_%H_%M_%S").c_str());
	
	//int status = 0;
	//fitsfile *fptr;
	FITSWriter w;
	
	LOG_INFO("Writing snapshot \"" << (fileName.c_str() + 1) << "\"...");
	
	if (!w.open(fileName.c_str()))
		return;
	//fits_create_file(&fptr, fileName, &status);
	//if (status) {
	//	LOG_ERROR("Failed to create FITS file (code: " << status << ").");
	//	//cerr << "ERROR: Failed to create FITS file (code: " << status << ")." << endl;
	//	return;
	//}
	
	int width = rightBin_ - leftBin_;
	w.createImage(width, length, FLOAT_IMG);
	//long dimensions[2] = { width, snapshotLength_ };
	//fits_create_img(fptr, FLOAT_IMG, 2, dimensions, &status);
	//if (status) {
	//	LOG_ERROR("Failed to create primary HDU in FITS file (code: " << status << ").");
	//	//cerr << "ERROR: Failed to create primary HDU in FITS file (code: " <<
	//	//	status << ")." << endl;
	//}
	
	w.comment("File created by " PACKAGE_STRING ".");
	w.comment("See " PACKAGE_URL ".");
	w.writeHeader("ORIGIN", origin.c_str(), "");
	w.date();
	w.comment(WFTime::now().format("Local time: %Y-%m-%d %H:%M:%S %Z", true).c_str());
	w.writeHeader("DATE-OBS", time.format("%Y-%m-%dT%H:%M:%S").c_str(), "observation date (UTC)");
	//fits_write_comment(fptr, "File created by " PACKAGE_STRING ".", &status);
	//fits_write_comment(fptr, "See " PACKAGE_URL ".", &status);
	//writeHeader(fptr, "ORIGIN", origin.c_str(), "", &status);
	//fits_write_date(fptr, &status);
	//fits_write_comment(fptr, WFTime::now().format("Local time: %Y-%m-%d %H:%M:%S %Z", true).c_str(), &status);
	//writeHeader(fptr, "DATE-OBS", time.format("%Y-%m-%dT%H:%M:%S").c_str(), "observation date (UTC)", &status);
	
	w.writeHeader("CTYPE2", "TIME",                      "in seconds");
	w.writeHeader("CRPIX2", 1,                           ""          );
	w.writeHeader("CRVAL2", (float)time.seconds(),       ""          );
	w.writeHeader("CDELT2", 1.f / (float)fftSampleRate,  ""          );
	//writeHeader(fptr, "CTYPE2", "TIME",                      "in seconds", &status);
	//writeHeader(fptr, "CRPIX2", 1,                           "",           &status);
	//writeHeader(fptr, "CRVAL2", (float)time.seconds(),       "",           &status);
	//writeHeader(fptr, "CDELT2", 1.f / (float)fftSampleRate,  "",           &status);
	
	w.writeHeader("CTYPE1", "FREQ",                             "in Hz");
	w.writeHeader("CRPIX1", 1.f,                                ""     );
	w.writeHeader("CRVAL1", (float)leftFrequency_,              ""     );
	w.writeHeader("CDELT1", (float)backend_->binToFrequency(),  ""     );
	//writeHeader(fptr, "CTYPE1", "FREQ",                            "in Hz", &status);
	//writeHeader(fptr, "CRPIX1", 1.f,                               "",      &status);
	//writeHeader(fptr, "CRVAL1", (float)leftFrequency_,             "",      &status);
	//writeHeader(fptr, "CDELT1", (float)backend_->binToFrequency(), "",      &status);
	
	w.checkStatus("Error occured while writing FITS file header.");
	//if (status) {
	//	LOG_ERROR("Error occured while writing FITS file header (code: " << status << ").");
	//	//cerr << "ERROR: Error occured while writing FITS file header (code: " <<
	//	//	status << ")." << endl;
	//}
	
	int rowIndex = start;
	for (int y = 0; y < length; y++, rowIndex++) {
		w.write(y, 1, (float*)(buffer_->at(rowIndex) + leftBin_));
	}
	//long fpixel[2] = { 1, 1 };
	//for (int y = 0; y < outBuffer_.mark; y++) {
	//	fits_write_pix(fptr,
	//				TFLOAT,
	//				fpixel,
	//				width,
	//				(void*)(outBuffer_.getRow(y) + leftBin_),
	//				&status);
	//	fpixel[1]++;
	//	if (status) break;
	//}
	
	w.checkStatus("Error occured while writing data to a FITS file.");
	//if (status) {
	//	LOG_ERROR("Error occured while writing data to a FITS file (file name: " << (fileName + 1) <<
	//			", code: " << status << ").");
	//	
	//	//cerr << "ERROR: Error occured while writing data to FITS file (code: " <<
	//	//	status << ")." << endl;
	//}
	
	w.close();
	//fits_close_file(fptr, &status);
	//if (status) {
	//	LOG_ERROR("Failed to close FITS file (file name: " << (fileName + 1) <<
	//			", code: " << status << ").");
	//	
	//	//cerr << "ERROR: Failed to close FITS file (code: " << status << ")." << endl;
	//}
	
	//delete [] fileName;
	
	LOG_DEBUG("Finished writing snapshot.");
}


string SnapshotRecorder::getFileName(WFTime time)
{
	string typ("snap");
	string origin = backend_->getOrigin();
	return getFileName(typ, origin, time);
}


string SnapshotRecorder::getFileName(const string &typ,
							  const string &origin,
							  WFTime       time)
{
	char fileName[1024];
	sprintf(fileName, "!%s_%s_%s%3d.fits",
		   typ.c_str(),
		   origin.c_str(),
		   time.format("%Y%m%d%H%M%S").c_str(),
		   (int)(time.microseconds() / 1000));
	return string(fileName);
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
	if (buffer_->size(nextSnapshot_.start) >= 0)
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


Ref<DIObject> SnapshotRecorder::make(Ref<DynObject> config, Ref<DIObject> parent)
{
	int   snapshotLength = config->getStrInt("snapshot_length", 60);
	float leftFrequency  = config->getStrDouble("low_freq", 0);
	float rightFrequency = config->getStrDouble("hi_freq",  0);
	
	return new SnapshotRecorder(
		parent,
		snapshotLength,
		leftFrequency,
		rightFrequency
	);
}

CPPAPP_DI_METHOD("snapshot", SnapshotRecorder, make);


////////////////////////////////////////////////////////////////////////////////
// WATERFALL BACKEND
////////////////////////////////////////////////////////////////////////////////


void WaterfallBackend::processFFT(const fftw_complex *data, int size, DataInfo info)
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
	recorder->setBuffer(&buffer_, &bufferMutex_);
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



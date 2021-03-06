/**
 * \file   WaterfallBackend.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-04-25
 *
 * \brief Header file for the WaterfallBackend class.
 */

#ifndef WATERFALLBACKEND_YGIIIZR2
#define WATERFALLBACKEND_YGIIIZR2


#include "FFTBackend.h"
#include "FITSWriter.h"
#include "RingBuffer.h"
#include "Channel.h"
#include "BolidMessage.h"
#include "CsvLog.h"
#include "utils.h"

#include <cmath>

using namespace std;

#include <fitsio.h>


#define WATERFALL_BACKEND_CHUNK_SIZE (1024 * 1024)


////////////////////////////////////////////////////////////////////////////////
// RECORDER
////////////////////////////////////////////////////////////////////////////////


class WaterfallBackend;


/**
 * \brief Base class for FFT data recorders.
 */
class Recorder : public DIObject {
protected:
	Ref<WaterfallBackend>   backend_; ///< This recorder's backend.
	RingBuffer2D<float>    *buffer_; ///< FFT data buffer to record from.
	FFTBackend::IQBuffer   *rawBuffer_; ///< I/Q data buffer to record from.
	Mutex                  *bufferMutex_; ///< Controls access to \ref buffer_.
	vector<RawDataHandle>  *rawHandles_;
	
public:
	Recorder(Ref<WaterfallBackend>  backend):
		backend_(backend),
		buffer_(NULL),
		rawBuffer_(NULL),
		bufferMutex_(NULL),
		rawHandles_(NULL)
	{}
	
	virtual ~Recorder() {
		backend_     = NULL;
		buffer_      = NULL;
		bufferMutex_ = NULL;
	}
	
	void setBuffer(RingBuffer2D<float> *buffer,
				FFTBackend::IQBuffer *rawBuffer,
				Mutex *bufferMutex,
				vector<RawDataHandle> *rawHandles)
	{
		buffer_      = buffer;
		rawBuffer_   = rawBuffer;
		bufferMutex_ = bufferMutex;
		rawHandles_  = rawHandles;
	}
	
	int getSampleRate();
	int getFFTSampleRate();
	
	inline int fftMarkToRaw(int mark);
	inline WFTime fftMarkToTime(int mark);
	/**
	 * \brief Converts number of FFT samples to number raw I/Q samples.
	 */
	inline int fftSamplesToRaw(int sampleCount)
	{
		return ((double)sampleCount / (double)getFFTSampleRate()) * (double)getSampleRate();
	}
	
	virtual int requestBufferSize() { return 0; }
	
	/**
	 * \brief Callback called at the beginning of the FFT stream.
	 */
	virtual void start() {}
	/**
	 * \brief Callback called when the FFT stream terminates.
	 */
	virtual void stop() {}
	/**
	 * \brief Callback periodically called on FFT input.
	 */
	virtual void update() = 0;
};


////////////////////////////////////////////////////////////////////////////////
// SNAPSHOT RECORDER
////////////////////////////////////////////////////////////////////////////////


/**
 * \brief FFT data recorder which makes continuous snapshots of constants length.
 */
class SnapshotRecorder : public Recorder {
protected:
	/**
	 * \brief Specifies a snapshot within \ref Recorder::buffer_ buffer.
	 */
	struct Snapshot {
		int start; ///< Start position of the snapshot in \ref Recorder::buffer_ buffer.
		int length; ///< Length of the snapshot in number of FFT rows.
		
		int reservation; ///< Handle of the buffer reservation. See \ref RingBuffer2D for more information.
		
		bool includeRawData;

		string fileName;
		
		Snapshot() :
			start(0), length(0),
			reservation(-1),
			includeRawData(false)
		{}
		
		Snapshot(int start) :
			start(start), length(0),
			reservation(-1),
			includeRawData(false)
		{}
		
		/**
		 * \brief Returns the end of the reservation.
		 *
		 * The end is computed as \c start + \c length.
		 *
		 * \returns the position in \ref WaterfallBackend buffer after the last
		 *          row of this snapshot
		 */
		inline int end() const { return start + length; }
	};
	
	string outputDir_; ///< Directory to store the resulting snapshot files in.
	string outputType_; ///< Short string identifying the type of the output (snapshots/bolids).
	bool   compressOutput_; ///< Output compressed file.
	
	int   snapshotLength_;
	float leftFrequency_;
	float rightFrequency_;
	bool  writeUnfinished_;
	
	int      snapshotRows_;
	int      leftBin_;
	int      rightBin_;
	Snapshot nextSnapshot_;
	
	typedef MethodThread<void, SnapshotRecorder> Thread;
	Thread            *workerThread_;
	Channel<Snapshot>  snapshots_;
	
	void*        threadMethod();
	void         startWriting();
	virtual void writeHeader(FITSWriter *writer);
	virtual void write(Snapshot snapshot);
	virtual void writeRaw(Snapshot snapshot);
	
	bool        listenToNoise_;
	float       noise_;
	float       peakFrequency_;
	float       magnitude_;
	
	static void processNoiseMessage(const NoiseMessage &msg, void *data);
	
	string getFileName(int mark);

public:
	SnapshotRecorder(Ref<WaterfallBackend>  backend,
				  int                    snapshotLength,
				  float                  leftFrequency,
				  float                  rightFrequency,
				  string                 outputDir,
				  string                 outputType,
				  bool                   compressOutput,
				  bool                   listenToNoise) :
		Recorder(backend),
		outputDir_(outputDir),
		outputType_(outputType),
		compressOutput_(compressOutput),
		snapshotLength_(snapshotLength),
		leftFrequency_(leftFrequency),
		rightFrequency_(rightFrequency),
		writeUnfinished_(true),
		listenToNoise_(listenToNoise)
	{
		ORDER_PAIR(leftFrequency_, rightFrequency_);
		
		if (listenToNoise) {
			addListener<NoiseMessage>(&SnapshotRecorder::processNoiseMessage, (void*)this);
		}
	}
	
	virtual ~SnapshotRecorder() {}
	
	virtual string getFileName(WFTime time);
	virtual string getFileName(const char *typ, const char *ext, WFTime time);
	virtual string getFileName(const string &typ, const string &origin, WFTime time);
	
	virtual string getFileBasename(const char *typ, const char *ext, const string &origin, WFTime time);
	
	virtual int requestBufferSize();
	
	virtual void start();
	virtual void stop();
	virtual void update();
	
	static Ref<DIObject> make(Ref<DynObject> config, Ref<DIObject> parent);
};


////////////////////////////////////////////////////////////////////////////////
// WATERFALL BACKEND
////////////////////////////////////////////////////////////////////////////////


/**
 * \brief Represents a backend that calculates FFT from the input and records
 *        the result through multiple recorders.
 *
 */
class WaterfallBackend : public FFTBackend {
public:
	typedef RingBuffer2D<float> FFTBuffer;

private:
	WaterfallBackend(const WaterfallBackend& other);
	
	string                 origin_;
	
	FFTBuffer              buffer_;
	int                    bufferChunkSize_;
	Mutex                  bufferMutex_;
	vector<RawDataHandle>  rawHandles_;
	
	vector<Ref<Recorder> > recorders_;
	
	string                 metadataPath_;
	Ref<CsvLog>            metadataFile_;
	Mutex                  metadataFileLock_;

protected:
	virtual void processFFT(const fftw_complex *data, int size, DataInfo info, int rawMark);
	
public:
	WaterfallBackend(int bins,
				  int overlap,
				  string origin);
	virtual ~WaterfallBackend();
	
	string getOrigin() { return origin_; }
	
	void setMetadataPath(const string& path) { metadataPath_ = path; } 
	Ref<CsvLog> getMetadataFile();
	
	int getBufferChunkSize() { return bufferChunkSize_; }
	void setBufferChunkSize(int value) { bufferChunkSize_ = value; }
	
	void addRecorder(Ref<Recorder> recorder);
	
	virtual void startStream(StreamInfo info);
	virtual void endStream();
	
	virtual bool injectDependency(Ref<DIObject> obj, std::string key);

	static Ref<DIObject> make(Ref<DynObject> config, Ref<DIObject> parent);
	
	inline int fftSamplesToRaw(int sampleCount)
	{
		double sampleRate = getStreamInfo().sampleRate;
		return ((double)sampleCount / (double)getFFTSampleRate()) * (double)sampleRate;
	}
};


#endif /* end of include guard: WATERFALLBACKEND_YGIIIZR2 */


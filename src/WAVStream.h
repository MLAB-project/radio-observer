/**
 * \file   WAVStream.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-04-22
 *
 * \brief Header file for the WAVStream class.
 */

#ifndef WAVSTREAM_DWWNH2AB
#define WAVSTREAM_DWWNH2AB

#include <stdint.h>
#include <string>
#include <ostream>
#include <vector>

using namespace std;

#include <cppapp/Object.h>
#include <cppapp/Input.h>
#include <cppapp/Logger.h>

using namespace cppapp;

#include "Frontend.h"
#include "Backend.h"


struct WAVFormat {
	static const char *CHUNK_ID;
	static const char *CHUNK_FORMAT;
	
	static const char *FORMAT_SUBCHUNK_ID;
	static const char *INF1_SUBCHUNK_ID;
	static const char *DATA_SUBCHUNK_ID;
	
	static const int FORMAT_SUBCHUNK_SIZE = 76;
	
	int audioFormat;
	int channelCount;
	int sampleRate;
	int byteRate;
	int blockAlign;
	int bitsPerSample;
};


/**
 * \todo Write documentation for class WAVStream.
 */
class WAVStream : public Frontend {
private:
	Ref<Input>      input_;

	vector<char>    stringBuffer_;
	
	bool            formatRead_;
	bool            dataRead_;
	
	WAVFormat       format_;
	string          inf1_;
	
	int             dataBufferSize_;
	vector<int16_t> dataBuffer_;
	vector<Complex> outputBuffer_;
	
	template<class T>
	T readScalar()
	{
		T result;
		input_->getStream()->read((char*)&result, sizeof(T));
		return result;
	}
	
	uint32_t readUInt32();
	int32_t readInt32();
	int16_t readInt16();
	string readString(int length);
	
	bool readFormatSubchunk(int64_t size);
	bool readInf1Subchunk(int64_t size);
	bool readDataSubchunk(int64_t size);
	bool readUnknownSubchunk(int64_t size);
	int readSubchunk();
	
	WAVStream(const WAVStream& other);
	
public:
	WAVStream(Ref<Input> input);
	virtual ~WAVStream() {}
	
	void setBackend(Ref<Backend> backend) { backend_ = backend; }
	
	virtual void run();
};

#endif /* end of include guard: WAVSTREAM_DWWNH2AB */


/**
 * \file   WAVStream.cpp
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-04-22
 * 
 * \brief Implementation file for the WAVStream class.
 */

#include "WAVStream.h"


const char *WAVFormat::CHUNK_ID = "RIFF";
const char *WAVFormat::CHUNK_FORMAT = "WAVE";

const char *WAVFormat::FORMAT_SUBCHUNK_ID = "fmt ";
const char *WAVFormat::INF1_SUBCHUNK_ID = "inf1";
const char *WAVFormat::DATA_SUBCHUNK_ID = "data";


/**
 *
 */
uint32_t WAVStream::readUInt32()
{
	return readScalar<uint32_t>();
}


/**
 *
 */
int32_t WAVStream::readInt32()
{
	return readScalar<int32_t>();
}


int16_t WAVStream::readInt16()
{
	return readScalar<int16_t>();
}


/**
 *
 */
string WAVStream::readString(int length)
{
	if ((int)stringBuffer_.size() < (length + 1)) {
		stringBuffer_.resize(length + 1);
	}
	stringBuffer_[length] = 0;
	input_->getStream()->read(&(stringBuffer_[0]), length);
	string result(&(stringBuffer_[0]));
	return result;
}


/**
 *
 */
bool WAVStream::readFormatSubchunk(int64_t size)
{
	format_.audioFormat = readInt16();
	format_.channelCount = readInt16();
	format_.sampleRate = readInt32();
	format_.byteRate = readInt32();
	format_.blockAlign = readInt16();
	format_.bitsPerSample = readInt16();
	
	streamInfo_.sampleRate = format_.sampleRate;
	
	LOG_INFO(
		"WAV format: audioFormat=" << format_.audioFormat <<
		", channelCount="          << format_.channelCount <<
		", sampleRate="            << format_.sampleRate <<
		", byteRate="              << format_.byteRate <<
		", blockAlign="            << format_.blockAlign <<
		", bitsPerSample="         << format_.bitsPerSample
	);

	return true;
}


/**
 *
 */
bool WAVStream::readInf1Subchunk(int64_t size)
{
	inf1_ = readString(size);
	LOG_INFO("INF1 subchunk found: " << inf1_);
	
	return true;
}


/**
 *
 */
bool WAVStream::readDataSubchunk(int64_t size)
{
	if (format_.bitsPerSample != 16) {
		LOG_ERROR("Can only read 16 bits per sample! Stopping now.");
		return false;
	}
	
	int rawBufferSize = dataBufferSize_ * format_.blockAlign;
	int bufferCount = size / rawBufferSize;
	int bufferRemainder = size % rawBufferSize;
	
	dataBuffer_.resize(dataBufferSize_ * format_.channelCount);
	outputBuffer_.resize(dataBufferSize_);
	
	for (int i = 0; i < bufferCount; i++) {
		input_->getStream()->read((char*)&(dataBuffer_[0]), rawBufferSize);
		
		for (int sample = 0; sample < dataBufferSize_; sample++) {
			outputBuffer_[sample].real = (double)dataBuffer_[sample * 2];
			outputBuffer_[sample].imag = (double)dataBuffer_[sample * 2 + 1];
		}
		
		process(outputBuffer_);
	}
	
	if (bufferRemainder > 0) {
		int blockCount = bufferRemainder / format_.blockAlign;
		int sampleCount = blockCount * format_.channelCount;
		
		input_->getStream()->read((char*)&(dataBuffer_[0]), bufferRemainder);
		
		outputBuffer_.resize(sampleCount);
		for (int sample = 0; sample < sampleCount; sample++) {
			outputBuffer_[sample].real = (double)dataBuffer_[sample * 2];
			outputBuffer_[sample].imag = (double)dataBuffer_[sample * 2 + 1];
		}

		process(outputBuffer_);
	}
	
	return true;
}


/**
 *
 */
bool WAVStream::readUnknownSubchunk(int64_t size)
{
	input_->getStream()->ignore(size);
	return true;
}


/**
 *
 */
int WAVStream::readSubchunk()
{
	string subchunkId = readString(4);
	int64_t size = (int64_t)readUInt32();
	
	//cerr << "CHUNK " << subchunkId << ", SIZE = " << size << endl;
	LOG_DEBUG("WAVStream: Chunk " << subchunkId << ", SIZE = " << size << endl);
	
	if (subchunkId.compare(WAVFormat::FORMAT_SUBCHUNK_ID) == 0) {
		if (!readFormatSubchunk(size)) return -1;
	} else if (subchunkId.compare(WAVFormat::INF1_SUBCHUNK_ID) == 0) {
		if (!readInf1Subchunk(size)) return -1;
	} else if (subchunkId.compare(WAVFormat::DATA_SUBCHUNK_ID) == 0) {
		if (!dataRead_) {
			startStream();
			//if (backend_.isNotNull())
			//	backend_->startStream(streamInfo_);
			dataRead_ = true;
		}
		if (!readDataSubchunk(size)) return -1;
	} else {
		if (!readUnknownSubchunk(size)) return -1;
	}
	
	return size + 4;
}


/**
 * Constructor.
 */
WAVStream::WAVStream(Ref<Input> input) :
	input_(input), dataBufferSize_(1024)
{
}


/**
 *
 */
void WAVStream::run()
{
	//char chunkID[5];
	//chunkID[4] = 0;
	//input_->getStream()->read(chunkID, 4);
	
	//cout << "CHUNK ID: " << chunkID << endl;
	
	streamInfo_ = StreamInfo();
	
	string chunkId = readString(4);
	if (chunkId.compare(WAVFormat::CHUNK_ID) != 0) {
		LOG_ERROR("Invalid chunk ID. Stream may not be in WAV format." << endl);
		//cerr << "ERROR: Invalid chunk ID. Stream may not be in WAV format." << endl;
		return;
	}
	
	int64_t chunkSize = (int64_t)readUInt32();
	//if (chunkSize <= 0) {
	//	LOG_ERROR("Invalid chunk size: " << chunkSize << ". Exiting...");
	//	return;
	//}
	LOG_DEBUG("Chunk size: " << chunkSize << endl);
	
	string chunkFormat = readString(4);
	if (chunkFormat.compare(WAVFormat::CHUNK_FORMAT) != 0) {
		LOG_ERROR("Invalid chunk format. Stream may not be in WAV format." << endl);
		//cerr << "ERROR: Invalid chunk format. Stream may not be in WAV format." << endl;
		return;
	}
	
	chunkSize -= 4;
	
	formatRead_ = false;
	dataRead_ = false;
	
	dataInfo_ = DataInfo();
	
	while (chunkSize > 0) {
		int size = readSubchunk();
		if (size < 0) break;
		chunkSize -= size;
	}
	
	// Only end stream if it has been started in the first place.
	if (dataRead_)
		endStream();
}



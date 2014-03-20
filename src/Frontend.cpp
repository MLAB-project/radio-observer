/**
 * \file   Frontend.cpp
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-04-26
 * 
 * \brief Implementation file for the Frontend class.
 */

#include "Frontend.h"
#include <cppapp/utils.h>


/**
 *
 */
void Frontend::startStream()
{
	if (backend_.isNotNull()) {
		backend_->startStream(streamInfo_);
	}
	
	dataInfo_.offset = 0;
	dataInfo_.timeOffset = streamInfo_.timeOffset;
}


/**
 *
 */
void Frontend::endStream()
{
	if (backend_.isNotNull()) {
		backend_->endStream();
	}
}


/**
 *
 */
void Frontend::process(const vector<Complex> &data)
{
	if (backend_.isNotNull()) {
		backend_->process(data, dataInfo_);
	}
	
	dataInfo_.offset += data.size();
	dataInfo_.timeOffset = streamInfo_.timeOffset.addSamples(
		dataInfo_.offset,
		streamInfo_.sampleRate
	);
}


void Frontend::stop()
{
	stopping_ = true;
}


void Frontend::sendMessage(const char *msg, size_t length)
{
	cerr.write(msg, length);
	cerr.flush();
}


void Frontend::sendMessage(const char *msgType, const char *msgData, size_t dataLength)
{
	//size_t msgTypeLength = strlen(msgType);
	
	ostringstream buffer;
	buffer << msgType << ":";
	buffer.write(msgData, dataLength);
	
	sendMessage(buffer.str().c_str(), buffer.str().size());
}



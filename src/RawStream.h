/**
 * \file   RawStream.h
 * \author Martin Povišer <povik@mlab.cz>
 * \date   2015-07-28
 *
 * \brief Header file for the RawStream class.
 */

#ifndef RAWSTREAM_3I135DAZ
#define RAWSTREAM_3I135DAZ

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


/**
 * \brief Frontend class that reads raw I/Q samples from a file.
 */
class RawStream : public Frontend {
private:
	RawStream(const RawStream& other);

protected:
	int             fd_;
	int             sampleRate_;

public:
	RawStream(int fd_, int sampleRate);
	virtual ~RawStream() {};

	void setBackend(Ref<Backend> backend) { backend_ = backend; }

	void runFromFD();

	virtual void run();
};


class RawTCPStream : public RawStream {
private:
	Ref<Input>      input_;

	string          host_;
	int             port_;

	RawTCPStream(const RawStream& other);

public:
	RawTCPStream(string host, int port, int sampleRate);
	virtual ~RawTCPStream() {};

	virtual void run();
};

#endif /* end of include guard: RAWSTREAM_3I135DAZ */


/**
 * \file   Frontend.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-04-26
 *
 * \brief Header file for the Frontend class.
 */

#ifndef FRONTEND_OBVGMG1U
#define FRONTEND_OBVGMG1U

#include <cppapp/Object.h>

using namespace cppapp;

#include "Backend.h"

/**
 * \todo Write documentation for class Frontend.
 */
class Frontend : public Object {
private:
	Frontend(const Frontend& other);

protected:
	Ref<Backend> backend_;
	
	StreamInfo   streamInfo_;
	DataInfo     dataInfo_;
	
	bool         stopping_;
	
	void startStream();
	void endStream();
	void process(const vector<Complex> &data);
	
public:
	Frontend() : stopping_(false) {}
	virtual ~Frontend() {}
	
	void setBackend(Ref<Backend> backend) { backend_ = backend; }
	
	virtual void run() = 0;
	
	virtual void stop();
};

#endif /* end of include guard: FRONTEND_OBVGMG1U */


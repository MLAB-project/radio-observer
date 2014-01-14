/**
 * \file   JackFrontend.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-05-08
 *
 * \brief  Header file for the JackFrontend class.
 */

#ifndef JACKFRONTEND_J2RTK7C1
#define JACKFRONTEND_J2RTK7C1

#include "Frontend.h"

#include <jack/jack.h>

/**
 * \brief Frontend class that reads sound data from JACK server.
 */
class JackFrontend : public Frontend {
private:
	/**
	 * Copy constructor.
	 */
	JackFrontend(const JackFrontend& other);
	
	static int  onJackInput(jack_nframes_t nframes, void *arg);
	static void onJackShutdown(void *arg);
	
	bool        connect_;
	const char *leftInputName_;
	const char *rightInputName_;
	
	jack_port_t *leftPort_;
	jack_port_t *rightPort_;
	
	vector<Complex> outputBuffer_;

public:
	/**
	 * \brief Constructor.
	 */
	JackFrontend(bool connect, const char *leftInputName, const char *rightInputName) :
		connect_(connect),
		leftInputName_((leftInputName == NULL) ? "system:capture_1" : leftInputName),
		rightInputName_((rightInputName == NULL) ? "system:capture_2" : rightInputName),
		leftPort_(NULL),
		rightPort_(NULL)
	{}
	/**
	 * \brief Destructor.
	 */
	virtual ~JackFrontend() {}
	
	virtual void run();
};

#endif /* end of include guard: JACKFRONTEND_J2RTK7C1 */


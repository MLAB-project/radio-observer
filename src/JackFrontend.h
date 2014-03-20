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
#include "MessageDispatch.h"
#include "BolidMessage.h"

#include <string>
#include <deque>
using namespace std;

#include <cppapp/cppapp.h>
using namespace cppapp;

#include <jack/jack.h>
#include <jack/midiport.h>


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
	string      leftInputName_;
	string      rightInputName_;
	
	jack_port_t *leftPort_;
	jack_port_t *rightPort_;
	
	vector<Complex> outputBuffer_;
	
	jack_port_t     *midiPort_;
	deque<string*>   midiQueue_;
	bool             midiMessageWaiting_;
	Mutex            midiMutex_;

public:
	/**
	 * \brief Constructor.
	 */
	JackFrontend(bool connect, string leftInputName, string rightInputName) :
		connect_(connect),
		leftInputName_((leftInputName.size() == 0) ? "system:capture_1" : leftInputName),
		rightInputName_((rightInputName.size() == 0) ? "system:capture_2" : rightInputName),
		leftPort_(NULL),
		rightPort_(NULL)
	{}
	/**
	 * \brief Destructor.
	 */
	virtual ~JackFrontend() {}
	
	virtual void run();
	
	virtual void sendMessage(const char *msg, size_t length);
	
	void sendMidiMessage(const char *msg, size_t length);
};


class BolidMessageListener : public MessageListener<BolidMessage> {
private:
	Ref<JackFrontend> frontend_;

public:
	BolidMessageListener(Ref<JackFrontend> frontend) :
		frontend_(frontend)
	{ }
	
	virtual void sendMessage(const BolidMessage &msg);
};


class HeartBeatMessageListener : public MessageListener<HeartBeatMessage> {
private:
	Ref<JackFrontend> frontend_;

public:
	HeartBeatMessageListener(Ref<JackFrontend> frontend) :
		frontend_(frontend)
	{ }
	
	virtual void sendMessage(const HeartBeatMessage &msg);
};


#endif /* end of include guard: JACKFRONTEND_J2RTK7C1 */


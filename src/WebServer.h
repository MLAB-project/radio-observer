/**
 * \file   WebServer.h
 * \author Jan Milik <milikjan@fit.cvut.cz>
 * \date   2014-03-20
 *
 * \brief  Header file for the WebServer class.
 */

#ifndef WEBSERVER_XIYC92MT
#define WEBSERVER_XIYC92MT

#include <cppapp/cppapp.h>
using namespace cppapp;

#include "mongoose.h"

/**
 * \todo Write documentation for class WebServer.
 */
class WebServer : public DIObject {
private:
	/**
	 * \brief Copy constructor.
	 */
	WebServer(const WebServer& other);
	
	struct mg_server *server_;
	
	MethodThread<int, WebServer> *thread_;
	bool                          keepRunning_;
	
	int* threadWorker();

public:
	/**
	 * \brief Constructor.
	 */
	WebServer();
	/**
	 * \brief Destructor.
	 */
	virtual ~WebServer();
	
	void start();
	void stop();
	
	virtual bool injectDependency(Ref<DIObject> obj, std::string key);
	
	static Ref<DIObject> make(Ref<DynObject> config, Ref<DIObject> parent);
};

#endif /* end of include guard: WEBSERVER_XIYC92MT */


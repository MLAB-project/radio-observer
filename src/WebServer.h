/**
 * \file   WebServer.h
 * \author Jan Milik <milikjan@fit.cvut.cz>
 * \date   2014-03-20
 *
 * \brief  Header file for the WebServer class.
 */

#ifndef WEBSERVER_XIYC92MT
#define WEBSERVER_XIYC92MT

#include <deque>
#include <ctime>
using namespace std;

#include <cppapp/cppapp.h>
using namespace cppapp;

#include "config.h"
#include "mongoose.h"
#include "MessageDispatch.h"
#include "BolidMessage.h"


class App;
class Pipeline;


/**
 * \todo Write documentation for class WebServer.
 */
class WebServer : public DIObject {
private:
	/**
	 * \brief Copy constructor.
	 */
	WebServer(const WebServer& other);
	
	Ref<App>      app_;
	Ref<Pipeline> pipeline_;
	
	struct mg_server *server_;
	
	MethodThread<int, WebServer> *thread_;
	bool                          keepRunning_;
	
	deque<BolidMessage> bolids_;
	
	int* threadWorker();
	
	static int eventHandler(struct mg_connection *conn, enum mg_event ev);
	
	void respondMainPage(struct mg_connection *conn);
	
	void printSystemInfo(struct mg_connection *conn);

public:
	/**
	 * \brief Constructor.
	 */
	WebServer();
	/**
	 * \brief Destructor.
	 */
	virtual ~WebServer();
	
	Ref<App> getApp();
	void     setApp(Ref<App> app);
	
	Ref<Pipeline> getPipeline();
	void          setPipeline(Ref<Pipeline> pipeline);
	
	void start();
	void stop();
	
	virtual bool injectDependency(Ref<DIObject> obj, std::string key);
	
	static Ref<DIObject> make(Ref<DynObject> config, Ref<DIObject> parent);
	
	class BolidMessageListener : public MessageListener<BolidMessage> {
	private:
		Ref<WebServer> webServer_;
	
	public:
		BolidMessageListener(Ref<WebServer> server) : webServer_(server) {}
		virtual ~BolidMessageListener() {}
		
		virtual void sendMessage(const BolidMessage &msg);
	};
};

#endif /* end of include guard: WEBSERVER_XIYC92MT */


/**
 * \file   WebServer.cpp
 * \author Jan Milik <milikjan@fit.cvut.cz>
 * \date   2014-03-20
 * 
 * \brief  Implementation file for the WebServer class.
 */

#include "WebServer.h"
#include "App.h"


WebServer::WebServer()
{
	server_ = mg_create_server(NULL, &WebServer::eventHandler);
	mg_set_user_data(server_, this);
	mg_set_option(server_, "document_root",  ".");
	mg_set_option(server_, "listening_port", "8080");
}


WebServer::~WebServer()
{
	LOG_INFO("WebServer::~WebServer()");
	stop();
	mg_destroy_server(&server_);
}


Ref<App> WebServer::getApp()
{
	return app_;
}


void WebServer::setApp(Ref<App> app)
{
	app_ = app;
}


Ref<Pipeline> WebServer::getPipeline()
{
	return pipeline_;
}


void WebServer::setPipeline(Ref<Pipeline> pipeline)
{
	pipeline_ = pipeline;
}


void WebServer::start()
{
	keepRunning_ = true;
	thread_ = new MethodThread<int, WebServer>(this, &WebServer::threadWorker);
}


void WebServer::stop()
{
	keepRunning_ = false;
	if (thread_ != NULL) {
		thread_->join();
		delete thread_;
		thread_ = NULL;
	}
}


int* WebServer::threadWorker()
{
	while (keepRunning_) {
		mg_poll_server(server_, 1000);
	}
	
	return NULL;
}


int WebServer::eventHandler(struct mg_connection *conn, enum mg_event ev)
{
	WebServer *srv = (WebServer*)conn->user_data;
	
	switch (ev) {
	case MG_AUTH: return MG_TRUE;
	case MG_REQUEST:
		srv->respondMainPage(conn);
		return MG_TRUE;
	default: return MG_FALSE;
	}
}


void WebServer::respondMainPage(struct mg_connection *conn)
{
	mg_printf_data(conn, "<html>");
	mg_printf_data(conn, "<body>");
	
	mg_printf_data(conn, "<h1>%s</h1>", PACKAGE_STRING);
	mg_printf_data(conn, "<p><a href=\"%s\">%s</a></p>", PACKAGE_URL, PACKAGE_URL);
	
	if (pipeline_.isNotNull()) {
		Ref<WaterfallBackend> wb = pipeline_->getFrontend().as<WaterfallBackend>();
		
		mg_printf_data(conn, "<h2>Waterfall Backend</h2>");
		mg_printf_data(conn, "<p>Origin: %s</p>", wb->getOrigin().c_str());
	}
	
	mg_printf_data(conn, "</body>");
	mg_printf_data(conn, "</html>");
}


bool WebServer::injectDependency(Ref<DIObject> obj, std::string key)
{
	return false;
}


Ref<DIObject> WebServer::make(Ref<DynObject> config, Ref<DIObject> parent)
{
	Ref<WebServer> webServer = new WebServer();
	webServer->setPipeline(parent.as<Pipeline>());
	return webServer;
}

CPPAPP_DI_METHOD("webserver", WebServer, make);


void WebServer::BolidMessageListener::sendMessage(const BolidMessage &msg)
{
	
}



/**
 * \file   WebServer.cpp
 * \author Jan Milik <milikjan@fit.cvut.cz>
 * \date   2014-03-20
 * 
 * \brief  Implementation file for the WebServer class.
 */

#include "WebServer.h"


WebServer::WebServer()
{
	server_ = mg_create_server(NULL, &WebServer::eventHandler);
	mg_set_option(server_, "document_root",  ".");
	mg_set_option(server_, "listening_port", "8080");
}


WebServer::~WebServer()
{
	LOG_INFO("WebServer::~WebServer()");
	stop();
	mg_destroy_server(&server_);
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
	switch (ev) {
	case MG_AUTH: return MG_TRUE;
	case MG_REQUEST:
		mg_printf_data(conn, "<html>");
		mg_printf_data(conn, "<body>");
		mg_printf_data(conn, "<h1>%s</h1>", PACKAGE_STRING);
		mg_printf_data(conn, "<p><a href=\"%s\">%s</a></p>", PACKAGE_URL, PACKAGE_URL);
		mg_printf_data(conn, "</body>");
		mg_printf_data(conn, "</html>");
		return MG_TRUE;
	default: return MG_FALSE;
	}
}


bool WebServer::injectDependency(Ref<DIObject> obj, std::string key)
{
	return false;
}


Ref<DIObject> WebServer::make(Ref<DynObject> config, Ref<DIObject> parent)
{
	Ref<DIObject> webServer = new WebServer();
	return webServer;
}

CPPAPP_DI_METHOD("webserver", WebServer, make);



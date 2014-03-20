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
	server_ = mg_create_server(NULL, NULL);
	mg_set_option(server_, "document_root",  ".");
	mg_set_option(server_, "listening_port", "8080");
}


WebServer::~WebServer()
{
	mg_destroy_server(&server_);

	delete thread_;
	thread_ = NULL;
}


void WebServer::start()
{
	keepRunning_ = true;
	thread_ = new MethodThread<int, WebServer>(this, &WebServer::threadWorker);
}


void WebServer::stop()
{
	keepRunning_ = false;
	thread_->join();
	thread_ = NULL;
}


int* WebServer::threadWorker()
{
	while (keepRunning_) {
		mg_poll_server(server_, 1000);
	}
	
	return NULL;
}



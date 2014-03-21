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

	MessageDispatch<BolidMessage>::getInstance().addListener(new BolidMessageListener(this));
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
		if (srv == NULL) {
			mg_printf_data(conn, "<b>ERROR: Server not configured.</b>");
		} else {
			srv->respondMainPage(conn);
		}
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
	
	mg_printf_data(conn, "<table>");
	FOR_EACH(bolids_, it) {
		mg_printf_data(conn, "<tr><td>%d</td><td>%f.1</td></tr>", it->endSample - it->startSample, it->peakFreq);
	}
	mg_printf_data(conn, "</table>");
	
	printSystemInfo(conn);
	
	mg_printf_data(conn, "</body>");
	mg_printf_data(conn, "</html>");
}


void WebServer::printSystemInfo(struct mg_connection *conn)
{
	time_t timer = time(NULL);
	
	Ref<WaterfallBackend> wb = NULL;
	if (pipeline_.isNotNull()) {
		wb = pipeline_->getBackend().as<WaterfallBackend>();
	}
	
	mg_printf_data(
		conn,
		"<table border=\"1\">"
		"<tr><th colspan=\"2\">System Info</th></tr>"
		"<tr><th>System time</th><td>%s</td></tr>"
		"<tr><th>Radio-Observer version</th><td>%s</td></tr>",
		ctime(&timer),
		PACKAGE_VERSION
	);
	if (wb.isNotNull()) {
		mg_printf_data(
			conn,
			"<tr><th>Station name</th><td>%s</td></tr>",
			wb->getOrigin().c_str()
		);
	}
	mg_printf_data(conn, "</table>");
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
	webServer_->bolids_.push_back(msg);
}



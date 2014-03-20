/**
 * \file   Pipeline.cpp
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2014-01-04
 * 
 * \brief  Implementation file for the Pipeline class.
 */

#include "Pipeline.h"


void Pipeline::run()
{
	if (webServer_.isNotNull()) {
		webServer_->start();
	}
	
	frontend_->setBackend(backend_);
	frontend_->run();
}


void Pipeline::stop()
{
	if (webServer_.isNotNull()) {
		webServer_->stop();
	}
	
	frontend_->stop();
}


bool Pipeline::injectDependency(Ref<DIObject> obj, std::string key)
{
	if (key.compare("frontend") == 0) {
		setFrontend(obj);
	} else if (key.compare("backend") == 0) {
		setBackend(obj);
	} else if (key.compare("webserver") == 0) {
		webServer_ = obj;
	}
	
	return DIObject::injectDependency(obj, key);
}


Ref<DIObject> Pipeline::make(Ref<DynObject> config, Ref<DIObject> parent)
{
	return new Pipeline();
}

CPPAPP_DI_METHOD("pipeline", Pipeline, make);



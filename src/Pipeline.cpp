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
	FOR_EACH(agents_, agent) {
		(*agent)->start();
	}
	
	frontend_->setBackend(backend_);
	frontend_->run();
}


void Pipeline::stop()
{
	FOR_EACH(agents_, agent) {
		(*agent)->stop();
	}
	
	frontend_->stop();
	
	FOR_EACH(agents_, agent) {
		LOG_DEBUG("Waiting for agent " << (*agent)->getName() << " to stop...");
		(*agent)->join();
	}
}


bool Pipeline::injectDependency(Ref<DIObject> obj, std::string key)
{
	if (key.compare("frontend") == 0) {
		setFrontend(obj);
	} else if (key.compare("backend") == 0) {
		setBackend(obj);
	} else if (key.compare("agent") == 0) {
		addAgent(obj);
	}
	
	return DIObject::injectDependency(obj, key);
}


Ref<DIObject> Pipeline::make(Ref<DynObject> config, Ref<DIObject> parent)
{
	return new Pipeline();
}

CPPAPP_DI_METHOD("pipeline", Pipeline, make);



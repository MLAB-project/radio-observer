/**
 * \file   MetadataAgent.cpp
 * \author Jan Milik <milikjan@fit.cvut.cz>
 * \date   2014-10-12
 * 
 * \brief  Implementation file for the MetadataAgent class.
 */

#include "MetadataAgent.h"


void MetadataAgent::run()
{
	LOG_INFO("Metadata agent started. Thread " << Thread::getId() << ".");
}


//// DEPENDENCY INJECTION


// bool MetadataAgent::injectDependency(Ref<DIObject> obj, std::string key)
// {
// }


Ref<DIObject> MetadataAgent::make(Ref<DynObject> config, Ref<DIObject> parent)
{
	Ref<MetadataAgent> result = new MetadataAgent();
	
	result->setFileName(config->getStrString("file_name", "metadata.csv"));
	
	return result;
}


CPPAPP_DI_METHOD("metadata", MetadataAgent, make);


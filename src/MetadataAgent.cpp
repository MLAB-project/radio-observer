/**
 * \file   MetadataAgent.cpp
 * \author Jan Milik <milikjan@fit.cvut.cz>
 * \date   2014-10-12
 * 
 * \brief  Implementation file for the MetadataAgent class.
 */

#include "MetadataAgent.h"


bool MetadataAgent::runCycle()
{
	//int count = channel_.drainAll(&MetadataAgent::processMessage, (void*)this, true);
	channel_.drainAll(&MetadataAgent::processMessage, (void*)this, true);
	return !channel_.isClosing();
}


void MetadataAgent::processMessage(MemBlock buffer, void *data)
{
	MetadataAgent *self = (MetadataAgent*)data;
	
	MetadataTags *tag = (MetadataTags*)buffer.ptr;
	
	switch (*tag) {
	case METADATA_NOISE:
		self->processMessage((NoiseMetadata*)buffer.ptr);
		break;
	
	case METADATA_SNAPSHOT:
		self->processMessage((SnapshotMetadata*)buffer.ptr);
		break;
	
	case METADATA_METEOR:
		self->processMessage((MeteorMetadata*)buffer.ptr);
		break;
	
	default:
		LOG_ERROR("MetadataAgent: Received message with unknown tag: " << *tag << ".");
		break;
	}
}


void MetadataAgent::processMessage(NoiseMetadata *msg)
{
}


void MetadataAgent::processMessage(SnapshotMetadata *msg)
{
}


void MetadataAgent::processMessage(MeteorMetadata *msg)
{
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


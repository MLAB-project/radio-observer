/**
 * \file   MetadataAgent.h
 * \author Jan Milik <milikjan@fit.cvut.cz>
 * \date   2014-10-12
 *
 * \brief  Header file for the MetadataAgent class.
 */

#ifndef METADATAAGENT_AEO607C1
#define METADATAAGENT_AEO607C1


#include <cppapp/cppapp.h>

#include "Agent.h"
#include "WFTime.h"


enum MetadataTags {
	METADATA_NOISE,
	METADATA_SNAPSHOT,
	METADATA_METEOR
};


struct NoiseMetadata {
	MetadataTags tag;
	
	WFTime time;
	float  noise;
	float  peakFrequency;
	float  magnitude;
};


struct SnapshotMetadata {
	MetadataTags tag;
};


struct MeteorMetadata : public NoiseMetadata {
	float minFreq;
	float maxFreq;
	
	int startSample;
	int endSample;
};


/**
 * \todo Write documentation for class MetadataAgent.
 */
class MetadataAgent : public Agent {
private:
	MetadataAgent(const MetadataAgent& other);

	string fileName_;
	
	Ref<Output> output_;
	ByteChannel channel_;


protected:
	virtual bool runCycle();
	
	static void processMessage(MemBlock buffer, void *data);
	
	void processMessage(NoiseMetadata *msg);
	void processMessage(SnapshotMetadata *msg);
	void processMessage(MeteorMetadata *msg);


public:
	/**
	 * Constructor.
	 */
	MetadataAgent() :
		fileName_("metadata.csv"), channel_(1024)
	{}
	/**
	 * Destructor.
	 */
	virtual ~MetadataAgent() {}

	
	string getFileName() { return fileName_; }
	
	void setFileName(string fileName) { fileName_ = fileName; }
	
	
	//virtual bool injectDependency(Ref<DIObject> obj, std::string key);
	
	static Ref<DIObject> make(Ref<DynObject> config, Ref<DIObject> parent);
};


#endif /* end of include guard: METADATAAGENT_AEO607C1 */


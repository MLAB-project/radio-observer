/**
 * \file   MetadataAgent.h
 * \author Jan Milik <milikjan@fit.cvut.cz>
 * \date   2014-10-12
 *
 * \brief  Header file for the MetadataAgent class.
 */

#ifndef METADATAAGENT_AEO607C1
#define METADATAAGENT_AEO607C1


#include "Agent.h"


enum MetadataTags {
	METADATA_NOISE,
	METADATA_SNAPSHOT,
	METADATA_METEOR
};


struct NoiseMetadata {
};


struct SnapshotMetadata {
};


struct MeteorMetadata {
};


/**
 * \todo Write documentation for class MetadataAgent.
 */
class MetadataAgent : public Agent {
private:
	MetadataAgent(const MetadataAgent& other);

	string fileName_;


protected:
	virtual void run() {}


public:
	/**
	 * Constructor.
	 */
	MetadataAgent() :
		fileName_("metadata.csv")
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


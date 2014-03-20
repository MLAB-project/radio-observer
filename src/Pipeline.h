/**
 * \file   Pipeline.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2014-01-04
 *
 * \brief  Header file for the Pipeline class.
 */

#ifndef PIPELINE_E8U22EKJ
#define PIPELINE_E8U22EKJ

#include <cppapp/cppapp.h>
using namespace cppapp;

#include "Frontend.h"
#include "Backend.h"
#include "WebServer.h"


class App;


/**
 * \todo Write documentation for class Pipeline.
 */
class Pipeline : public DIObject {
private:
	Ref<Frontend> frontend_;
	Ref<Backend>  backend_;
	
	Ref<WebServer> webServer_;
	
	Pipeline(const Pipeline& other);
	
public:
	/**
	 * Constructor.
	 */
	Pipeline() {}
	/**
	 * Destructor.
	 */
	virtual ~Pipeline() {}
	
	void setApp(Ref<App> app);
	
	Ref<Frontend> getFrontend() { return frontend_; }
	void setFrontend(Ref<Frontend> frontend) { frontend_ = frontend; }
	
	Ref<Backend> getBackend() { return backend_; }
	void setBackend(Ref<Backend> backend) { backend_ = backend; }
	
	void run();
	void stop();
	
	virtual bool injectDependency(Ref<DIObject> obj, std::string key);
	
	static Ref<DIObject> make(Ref<DynObject> config, Ref<DIObject> parent);
};


#endif /* end of include guard: PIPELINE_E8U22EKJ */


/**
 * \file   App.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-04-22
 *
 * \brief Header file for the App class.
 */

#ifndef APP_TBSBM2YW
#define APP_TBSBM2YW

#include <cppapp/AppBase.h>
#include <cppapp/Input.h>
#include <cppapp/DynObject.h>

using namespace cppapp;

// TODO: Remove later.
#include "Pipeline.h"
#include "WAVStream.h"
#include "JackFrontend.h"
#include "WaterfallBackend.h"
#include "Signal.h"


/**
 * \brief Represents the application and its entry point.
 */
class App : public AppBase {
private:
	Ref<DynObject> config_;
	
	App(const App& other);

protected:
	virtual string getDefaultConfigFile();
	virtual void   readConfig();
	
	Ref<Pipeline> pipeline_;
	//Ref<Frontend> frontend_;
	//Ref<Backend>  backend_;
	
	Ref<Frontend> createFrontend();
	// Ref<Backend>  createBackend();
	
	virtual void setUp();
	virtual int onRun();
	
	void interruptHandler(int sigNum);

public:
	App();
	virtual ~App();
	
	Ref<DynObject> getConfig() { return config_; }
};


#endif /* end of include guard: APP_TBSBM2YW */


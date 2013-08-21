/**
 * \file   App.cpp
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-04-22
 * 
 * \brief Implementation file for the App class.
 */

#include "App.h"


Ref<Frontend> App::createFrontend()
{
	//Ref<Input> input;
	//
	//if (options().args().size() > 0) {
	//	input = new FileInput(options().args()[0]);
	//} else {
	//	input = new StandardInput();
	//}
	//
	//return new WAVStream(input);
	
	if (options().args().size() > 0) {
		string fileName = options().args()[0];
		LOG_INFO("Using WAV frontend, reading " << fileName << "...");
		return new WAVStream(new FileInput(fileName));
	} else {
		LOG_INFO("Using JACK frontend.");
		return new JackFrontend(
			config()->get("jack_left_port",  "system:capture_1")->asString().c_str(),
			config()->get("jack_right_port", "system:capture_2")->asString().c_str()
		);
	}
}


Ref<Backend> App::createBackend()
{
	Ref<Config> cfg = config();
	
	return new WaterfallBackend(
		cfg->get("fft_bins",    "32768")->asInteger(),
		cfg->get("fft_overlap", "24576")->asInteger(),
		
		config()->get("location_name",         "unknown")->asString(),
		// config()->get("waterfall_buffer_size", "10000")->asInteger(),
		config()->get("waterfall_snapshot_length", "1")->asFloat(),
		config()->get("waterfall_left_freq",   "0")->asFloat(),
		config()->get("waterfall_right_freq",  "0")->asFloat()
	);
}


/**
 *
 */
void App::setUp()
{
	AppBase::setUp();
}


/**
 *
 */
int App::onRun()
{
	AppBase::onRun();
	
	config()->dump(cerr);
	
	// if (options().args().size() > 0) {
	// 	input_ = new FileInput(options().args()[0]);
	// } else {
	// 	input_ = new StandardInput();
	// }
	
	// if (!(bool)options().get('o')) {
	// 	setOutput(new FileOutput(input_->getFileNameWithExt("png")));
	// }
	
	frontend_ = createFrontend();
	backend_  = createBackend();
	
	frontend_->setBackend(backend_);
	
	Signal::INT.install();
	Signal::INT.pushMethod(this, &App::interruptHandler);
	frontend_->run();
	Signal::INT.pop();
	Signal::INT.uninstall();
	
	// WAVStream stream(input_);
	// //Ref<Backend> backend = new SimpleWaterfallBackend(output(), 0.2, 0.1);
	// Ref<Backend> backend = new WaterfallBackend(
	// 	config()->get("location_name",         "unknown")->asString(),
	// 	config()->get("waterfall_buffer_size", "10000")->asInteger(),
	// 	config()->get("waterfall_left_freq", "0")->asFloat(),
	// 	config()->get("waterfall_right_freq", "0")->asFloat()
	// );
	// stream.setBackend(backend);
	// stream.run();
	
	return 0;
}


void App::interruptHandler(int sigNum)
{
	LOG_WARNING("Received INT signal, stopping the frontend.");
	frontend_->stop();
}


/**
 * Constructor.
 */
App::App() :
	AppBase()
{
}


/**
 * Destructor
 */
App::~App()
{
}



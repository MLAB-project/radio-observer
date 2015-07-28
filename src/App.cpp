/**
 * \file   App.cpp
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-04-22
 * 
 * \brief Implementation file for the App class.
 */

#include "App.h"

#include "BolidRecorder.h"
#include "git_version.h"


string App::getDefaultConfigFile()
{
	ostringstream s;
	s << getenv("HOME") <<
		"/." <<
		pathBasename(options().getExecutable()) <<
		".json";
	return s.str();
}


void App::readConfig()
{
	string configFileName = config()->get(CPPAPP_CONFIG_FILE_CFG_KEY,
								   getDefaultConfigFile())->asString();
	
	Ref<FileInput> configInput = new FileInput(configFileName);
	if (!configInput->exists()) {
		LOG_WARNING("Configuration file " << configFileName << " does not exist.");
		return;
	}
	
	JSONParser parser;
	config_ = parser.parse(configInput);
	configInput->close();
	
	if (config_->isError()) {
		Ref<DynError> err = config_.as<DynError>();
		LOG_ERROR("Syntax error in config file at " << err->getErrorLoc() << ": " << err->getString());
		exit(1);
	}
	
	//Ref<PrettyPrinter> printer = new PrettyPrinter();
	//config_->print(printer);
	//printer->print("\n");
	
	// Override the configuration read from the file by the command line
	// options.
	options().setConfigKeys(config_);
}


Ref<Frontend> App::createFrontend()
{
	if (options().args().size() > 0) {
		string fileName = options().args()[0];
		LOG_INFO("Using WAV frontend, reading " << fileName << "...");
		return new WAVStream(new FileInput(fileName));
	}

	string frontendName = config_->getStrString("frontend", "jack");

	if (frontendName == "tcp_raw") {
		LOG_INFO("Using raw TCP frontend.");

		return new RawTCPStream(
			config_->getStrString("tcp_host", "localhost"),
			config_->getStrInt("tcp_port", 4000),
			config_->getStrInt("raw_sample_rate", 96000)
		);
	} else if (frontendName == "jack") {
		LOG_INFO("Using JACK frontend.");
		
		if (config_->hasStrItem("jack_left_port") && !config_->hasStrItem("jack_right_port")) {
			LOG_WARNING("\"jack_left_port\" is specified, but \"jack_right_port\" is missing!");
		} else if (config_->hasStrItem("jack_right_port") && !config_->hasStrItem("jack_left_port")) {
			LOG_WARNING("\"jack_right_port\" is specified, but \"jack_left_port\" is missing!");
		}
		
		bool connect = (
			config_->hasStrItem("jack_left_port") || 
			config_->hasStrItem("jack_right_port")
		);
		
		return new JackFrontend(
			connect,
			pathBasename(options().getExecutable()),
			config_->getStrString("jack_left_port", "system:capture_1"),
			config_->getStrString("jack_right_port", "system:capture_2")
			
			//config()->get("jack_left_port",  "system:capture_1")->asString().c_str(),
			//config()->get("jack_right_port", "system:capture_2")->asString().c_str()
		);
	}

	LOG_ERROR("No frontend to use.");
	exit(1);
}


//Ref<Backend> App::createBackend()
//{
//	Ref<Config> cfg = config();
//	
//	Ref<WaterfallBackend> backend = new WaterfallBackend(
//		cfg->get("fft_bins",    "32768")->asInteger(),
//		cfg->get("fft_overlap", "24576")->asInteger(),
//		
//		config()->get("location_name",             "unknown")->asString()
//		// config()->get("waterfall_buffer_size",       "10000")->asInteger(),
//		// config()->get("waterfall_snapshot_length",       "1")->asFloat(),
//		// config()->get("waterfall_left_freq",          "8000")->asFloat(),
//		// config()->get("waterfall_right_freq",        "12000")->asFloat()
//	);
//	
//	backend->setGain(
//		config()->get("iq_gain", "0")->asFloat());
//	backend->setPhaseShift(
//		config()->get("iq_phase_shift", "0")->asInteger());
//	
//	if (config()->get("record_snapshots", "true")->asBool()) {
//		backend->addRecorder(new SnapshotRecorder(
//			backend,
//			config()->get("waterfall_snapshot_length",       "1")->asFloat(),
//			config()->get("waterfall_left_freq",          "8000")->asFloat(),
//			config()->get("waterfall_right_freq",        "12000")->asFloat()
//		));
//	}
//	
//	if (config()->get("detect_bolids", "true")->asBool()) {
//		backend->addRecorder(new BolidRecorder(
//			backend,
//			backend,
//			backend,
//			config()->get("waterfall_snapshot_length",     "1")->asFloat(),
//			config()->get("bolid_left_freq",            "9000")->asFloat(),
//			config()->get("bolid_right_freq",          "11000")->asFloat(),
//			config()->get("bolid_detect_left_freq",    "10300")->asFloat(),
//			config()->get("bolid_detect_right_freq",   "10900")->asFloat(),
//			config()->get("noise_low_freq",             "9000")->asFloat(),
//			config()->get("noise_hi_freq",              "9600")->asFloat()
//		));
//	}
//	
//	return backend;
//}


void App::setUp()
{
	AppBase::setUp();
	
	options().add('v',
			    "",
			    "Show program version.");
	
	//Logger::clearConfig();
	//Logger::addOutput(LOG_LVL_DEBUG, "waterfall.log");
}


/**
 * \todo Currently, this method exits prematurely if the config
 *       object is \c NULL (which happens when there is a syntax
 *       error in the config file or the file is missing). Better
 *       handling of default config must be implemented.
 */
int App::onRun()
{
	AppBase::onRun();
	
	// config()->dump(cerr);
	
	// if (options().args().size() > 0) {
	// 	input_ = new FileInput(options().args()[0]);
	// } else {
	// 	input_ = new StandardInput();
	// }
	
	// if (!(bool)options().get('o')) {
	// 	setOutput(new FileOutput(input_->getFileNameWithExt("png")));
	// }
	
	if (options().get('v')) {
		cout << PACKAGE_STRING << " " << GIT_VERSION << endl;
		// cout << PACKAGE_URL << endl;
		return EXIT_SUCCESS;
	}	
	
	// TODO: Implement better handling of missing config file.
	if (config_.isNull())
		return EXIT_NO_CONFIG;
	
	Ref<DynObject> loggingConfig = config_->getStrItem("logging");
	if (!loggingConfig.isNull()) {
		Logger::clearConfig();
		Logger::readConfig(loggingConfig);
	}
	
	LOG_INFO("***** Starting Radio Observer v" PACKAGE_VERSION " " GIT_VERSION " *****");
	
	string cfgName = config_->getStrString("configuration", "default");
	Injector::getInstance().makePlans(config_->getStrItem("configurations"));
	
	pipeline_ = Injector::getInstance().instantiateAs<Pipeline>(cfgName);
	if (pipeline_.isNull()) {
		LOG_ERROR("Initialization failed.");
		return EXIT_INIT_FAILED;
	}
	
	if (pipeline_->getFrontend().isNull())
		pipeline_->setFrontend(createFrontend());
	
	//frontend_ = createFrontend();
	//backend_  = createBackend();
	
	//frontend_->setBackend(backend_);
	
	Signal::INT.install();
	Signal::INT.pushMethod(this, &App::interruptHandler);
	Signal::TERM.install();
	Signal::TERM.pushMethod(this, &App::termHandler);
	//frontend_->run();
	pipeline_->run();
	Signal::TERM.pop();
	Signal::TERM.uninstall();
	Signal::INT.pop();
	Signal::INT.uninstall();
	
	LOG_INFO("Exiting.");
	
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
	pipeline_->stop();
	//frontend_->stop();
}


void App::termHandler(int sigNum)
{
	LOG_WARNING("TERM signal received, exiting.");
	exit(EXIT_TERM_RECEIVED);
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

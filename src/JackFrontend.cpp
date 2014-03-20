/**
 * \file   JackFrontend.cpp
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-05-08
 * 
 * \brief  Implementation file for the JackFrontend class.
 */

#include "JackFrontend.h"

#include <cppapp/cppapp.h>
using namespace cppapp;


int JackFrontend::onJackInput(jack_nframes_t nframes, void *arg)
{
	JackFrontend *self = (JackFrontend*)arg;

	jack_default_audio_sample_t *left =
		(jack_default_audio_sample_t *)jack_port_get_buffer(self->leftPort_, nframes);
	jack_default_audio_sample_t *right =
		(jack_default_audio_sample_t *)jack_port_get_buffer(self->rightPort_, nframes);
	
	//int offset = outputBuffer_.size();
	//outputBuffer_.resize(offset + nframes);
	int offset = 0;
	self->outputBuffer_.resize(nframes);
	
	for (int i = 0; i < (int)nframes; i++) {
		self->outputBuffer_[offset + i].real = left[i];
		self->outputBuffer_[offset + i].imag = right[i];
	}
	
	self->process(self->outputBuffer_);
	
	return 0;
}


void JackFrontend::onJackShutdown(void *arg)
{
	JackFrontend *self = (JackFrontend*)arg;

	self->endStream();
}


void JackFrontend::run()
{
	jack_options_t options = JackNullOption;
	jack_status_t  status;
	const char *client_name = "waterfall";
	jack_client_t *client = jack_client_open(client_name,
									 options,
									 &status,
									 /* server name */ NULL);
	
	if (client == NULL) {
		LOG_ERROR("Failed to connect to JACK (status = " << status << ")!");
		return;
	}
	
	if (status & JackServerStarted) {
		LOG_INFO("JACK server started.");
	}
	
	if (status & JackNameNotUnique) {
		client_name = jack_get_client_name(client);
		LOG_WARNING("Unique JACK client name \"" << client_name << "\" was assigned.");
	}
	
	streamInfo_ = StreamInfo();
	streamInfo_.sampleRate = jack_get_sample_rate(client);
	streamInfo_.timeOffset = WFTime::now();
	
	startStream();
	
	jack_set_process_callback(client, onJackInput, (void*)this);
	jack_on_shutdown(client, onJackShutdown, (void*)this);
	
	leftPort_ = jack_port_register(client,
							 "left",
							 JACK_DEFAULT_AUDIO_TYPE,
							 JackPortIsInput, 0);
	rightPort_ = jack_port_register(client,
							  "right",
							  JACK_DEFAULT_AUDIO_TYPE,
							  JackPortIsInput, 0);
	
	if ((leftPort_ == NULL) || (rightPort_ == NULL)) {
		LOG_ERROR("No more JACK ports available.");
		return;
	}
	
	if (jack_activate(client)) {
		LOG_ERROR("Cannot activate client.");
		return;
	}
	
	if (connect_) {
		if (jack_connect(client, leftInputName_.c_str(), jack_port_name(leftPort_))) {
			LOG_ERROR("Failed to connect left input port to \"" <<
					leftInputName_ << "\"!");
			return;
		}
		
		if (jack_connect(client, rightInputName_.c_str(), jack_port_name(rightPort_))) {
			LOG_ERROR("Failed to connect right input port to \"" <<
					rightInputName_ << "\"!");
			return;
		}
	}
	
	// Yep, active waiting. Pretty much.
	while (!stopping_) {
		sleep(2);
	}

	endStream();
	
	jack_client_close(client);
}



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
	
	void *midiPortBuffer = jack_port_get_buffer(self->midiPort_, nframes);
	jack_midi_clear_buffer(midiPortBuffer);
	
	if (self->midiMessageWaiting_) {
		if (self->midiMutex_.tryLock()) {
			if (self->midiQueue_.size() > 0) {
				string *msg = self->midiQueue_.front();
				self->midiQueue_.pop_front();
				
				LOG_INFO("JackFrontend: Sending MIDI message: " << *msg);
				
				unsigned char *buffer = jack_midi_event_reserve(
					midiPortBuffer,
					0,
					msg->size() + 3
				);
				buffer[0] = 0xf0;
				buffer[1] = 0x7d;
				memcpy(buffer + 2, msg->c_str(), msg->size());
				buffer[msg->size() + 2] = 0xf7;
				
				delete msg;
			}
			self->midiMessageWaiting_ = (self->midiQueue_.size() > 0);
			self->midiMutex_.unlock();
		}
	}
	
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
	jack_client_t *client = jack_client_open(clientName_.c_str(),
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
		const char *actualName = jack_get_client_name(client);
		LOG_WARNING("Unique JACK client name \"" << actualName << "\" was assigned.");
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
	
	midiPort_ = jack_port_register(client,
							 "midi_out",
							 JACK_DEFAULT_MIDI_TYPE,
							 JackPortIsOutput, 0);
	
	if ((leftPort_ == NULL) || (rightPort_ == NULL)) {
		LOG_ERROR("No more JACK ports available.");
		return;
	}
	
	if (midiPort_ == NULL) {
		LOG_ERROR("Cannot create MIDI output port.");
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
	
	MessageDispatch<BolidMessage>::getInstance().addListener(new BolidMessageListener(this));
	MessageDispatch<HeartBeatMessage>::getInstance().addListener(new HeartBeatMessageListener(this));
	
	// Yep, active waiting. Pretty much.
	while (!stopping_) {
		sleep(2);
	}

	endStream();
	
	jack_client_close(client);
}


void JackFrontend::sendMessage(const char *msg, size_t length)
{
	sendMidiMessage(msg, length);
}


void JackFrontend::sendMidiMessage(const char *msg, size_t length)
{
	MutexLock lock(&midiMutex_);
	
	midiQueue_.push_back(new string(msg, length));
	midiMessageWaiting_ = true;
}


void BolidMessageListener::sendMessage(const BolidMessage &msg)
{
	char buffer[1024];
	
	size_t length = sprintf(
		buffer,
		"mlab.aabb_event.meteor_echo:%d,%d,%f,%f,peak=%f mag=%f noise=%f",
		-msg.endSample,
		0,
		msg.minFreq,
		msg.maxFreq,
		msg.peakFrequency,
		msg.magnitude,
		msg.noise
	);
	
	frontend_->sendMidiMessage(buffer, length);
}


void HeartBeatMessageListener::sendMessage(const HeartBeatMessage &msg)
{
	char buffer[1024];
		
	size_t length = sprintf(
		buffer,
		"mlab.radio_event.heartbeat:%d",
		(int)msg.timestamp
	);
	
	frontend_->sendMidiMessage(buffer, length);
}



/**
 * \file   RawStream.h
 * \author Martin Povišer <povik@mlab.cz>
 * \date   2015-07-28
 *
 * \brief Implementation file for the RawStream class.
 */

#include "RawStream.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


/**
 * Constructor.
 */
RawStream::RawStream(int fd, int sampleRate) :
	fd_(fd), sampleRate_(sampleRate)
{
}


void RawStream::runFromFD()
{
	int bufferSize = 4096;
	int rawBufferSize = bufferSize * 2 * sizeof(float);

	vector<float> dataBuffer(bufferSize * 2);
	vector<Complex> outputBuffer(bufferSize);

	streamInfo_ = StreamInfo();
	streamInfo_.sampleRate = sampleRate_;
	streamInfo_.timeOffset = WFTime::now();

	startStream();

	while (true) {
		ssize_t ret = read(fd_, (char*)&(dataBuffer[0]), rawBufferSize);

		if (ret < 0) {
			LOG_ERROR("Input read error: " << strerror(errno) << endl);
			break;
		}

		if (ret == 0) {
			LOG_INFO("Reached end-of-file." << endl);
			break;
		}

		ret /= sizeof(float) * 2;
		for (int i = 0; i < ret; i++) {
			outputBuffer[i].real = dataBuffer[2*i];
			outputBuffer[i].imag = dataBuffer[2*i + 1];
		}

		process(outputBuffer);
	}

	endStream();
}


void RawStream::run()
{
	runFromFD();
}


RawTCPStream::RawTCPStream(string host, int port, int sampleRate)
	: RawStream(-1, sampleRate), host_(host), port_(port)
{
}


void RawTCPStream::run()
{
	struct sockaddr_in serv_addr;
	struct hostent *server;

	fd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_ < 0) {
		LOG_ERROR("TCP: Could not open socket: " << strerror(errno) << endl);
		return;
	}

	server = gethostbyname(host_.c_str());
	if (server == NULL) {
		LOG_ERROR("TCP: No such host: " << host_ << endl);
		close(fd_);
		return;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr,
		  (char *) &serv_addr.sin_addr.s_addr,
		  server->h_length);
	serv_addr.sin_port = htons(port_);

	if (connect(fd_,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		LOG_ERROR("TCP: Could not connect: " << strerror(errno) << endl);
		close(fd_);
		return;
	}

	LOG_INFO("TCP: Connected to " << host_ << ":" << port_ << endl);

	runFromFD();

	close(fd_);
}



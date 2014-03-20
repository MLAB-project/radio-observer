/**
 * \file   MessageDispatch.cpp
 * \author Jan Milik <milikjan@fit.cvut.cz>
 * \date   2014-03-17
 * 
 * \brief  Implementation file for the MessageDispatch class.
 */

#include "MessageDispatch.h"


void dummy() {
	string msg("Hello world.");
	MessageDispatch<string>::getInstance().sendMessage(msg);
}



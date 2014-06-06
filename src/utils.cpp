/**
 * \file   utils.cpp
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2014-06-06
 * 
 * \brief  Implementation file for the utils class.
 */

#include "utils.h"


int wrap(int value, int size)
{
	while (value < 0) {
		value += size;
	}
	return value % size;
}



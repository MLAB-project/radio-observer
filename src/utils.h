/**
 * \file   utils.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-11-03
 *
 * \brief  Header file for the utils class.
 */

#ifndef UTILS_Q7Z8RACX
#define UTILS_Q7Z8RACX


#include <utility>
using namespace std;

#include <cppapp/cppapp.h>


template<class T>
pair<T, T> orderPair(T a, T b)
{
	if (a > b)
		return make_pair(b, a);
	return make_pair(a, b);
}


#define ORDER_PAIR(a, b) { \
	VAR(p, orderPair((a), (b))); \
	(a) = p.first; \
	(b) = p.second; \
}


#endif /* end of include guard: UTILS_Q7Z8RACX */


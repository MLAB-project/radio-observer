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


#ifdef NDEBUG
#	define safeAdd(a, b) ((a) + (b))
#	define safeMul(a, b) ((a) * (b))
#else
template<class T>
T safeAdd(T a, T b)
{
	T c = a + b;
	
	if (b > 0 ? c < a : c > a) {
		LOG_ERROR("Addition overflow: " << a << " + " << b << " = " << c);
	}
	
	return c;
}

template<class T>
T safeMul(T a, T b)
{
	T c = a * b;
	
	return c;
}
#endif


template<class T>
pair<T, T> orderPair(T a, T b)
{
	if (a > b)
		return make_pair(b, a);
	return make_pair(a, b);
}


#define ORDER_PAIR(a, b) { \
	if ((a) > (b)) { \
		VAR(temp__, (a)); \
		(a) = (b); \
		(b) = temp__; \
	} \
}


#endif /* end of include guard: UTILS_Q7Z8RACX */


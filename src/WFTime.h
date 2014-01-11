/**
 * \file   WFTime.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-04-26
 *
 * \brief Header file for the WFTime class.
 */

#ifndef WFTIME_20LZQV24
#define WFTIME_20LZQV24

#include <ostream>
#include <ctime>
#include <sys/time.h>
using namespace std;

#include "utils.h"


#define MS_IN_SECOND 1000
#define US_IN_MS     1000
#define US_IN_SECOND (MS_IN_SECOND * US_IN_MS)


/**
 * \todo Write documentation for class WFTime.
 */
struct WFTime {
public:
	struct timeval time;
	
	inline time_t seconds() const { return time.tv_sec; }
	inline time_t microseconds() const { return time.tv_usec; }
	
	inline float toMilliseconds()
	{
		return (((float)seconds() * MS_IN_SECOND) +
			   ((float)microseconds() / (float)US_IN_MS));
	}
	
	WFTime()
	{
		time.tv_sec = 0;
		time.tv_usec = 0;
		//gettimeofday(&time, NULL);
	}
	
	WFTime(time_t seconds, suseconds_t microseconds)
	{
		time.tv_sec = seconds;
		time.tv_usec = microseconds;
	}
	
	WFTime(time_t miliseconds)
	{
		time.tv_sec = (miliseconds / MS_IN_SECOND);
		time.tv_usec = (miliseconds % MS_IN_SECOND) * US_IN_MS;
	}
	
	inline WFTime addMicroseconds(long us)
	{
		long sec  = time.tv_sec;
		long usec = safeAdd(((long)time.tv_usec), us);
		
		sec  = safeAdd(sec, (long)usec / (long)US_IN_SECOND);
		usec %= (long)US_IN_SECOND;
		
		return WFTime(sec, usec);
	}
	
	inline WFTime addSamples(int sampleCount, int sampleRate)
	{
		long microseconds = (((double)sampleCount / (double)sampleRate) *
						 (double)US_IN_SECOND);
		return addMicroseconds(microseconds);
		//return WFTime(
		//	microseconds / US_IN_SECOND,
		//	microseconds % US_IN_SECOND
		//);
	}
	
	/**
	 * Formats the time using a format string.
	 * 
	 * The syntax of the format string is the same as for the standard
	 * strftime function (see $ man strftime).
	 *
	 * \param  fmt   format string
	 * \param  local if true, assume the time is local, otherwise use UTC
	 * \return       formated date
	 */
	string format(const char *fmt, bool local = false) const;
	
	inline static WFTime now()
	{
		WFTime result;
		gettimeofday(&(result.time), NULL);
		return result;
	}
};


inline ostream& operator<<(ostream &output, const WFTime &rhs)
{
	output << "[" << rhs.time.tv_sec << "s, " << rhs.time.tv_usec << "us]";
	return output;
}


inline WFTime operator+(const WFTime& lhs, const WFTime& rhs)
{
	WFTime result;
	timeradd(&(lhs.time), &(rhs.time), &(result.time));
	return result;
}


inline WFTime operator-(const WFTime& lhs, const WFTime& rhs)
{
	WFTime result;
	timersub(&(lhs.time), &(rhs.time), &(result.time));
	return result;
}


inline bool operator>(const WFTime& lhs, const WFTime& rhs)
{
	return timercmp(&(lhs.time), &(rhs.time), >);
}


inline bool operator<(const WFTime& lhs, const WFTime& rhs)
{
	return timercmp(&(lhs.time), &(rhs.time), <);
}


inline bool operator!=(const WFTime& lhs, const WFTime& rhs)
{
	return timercmp(&(lhs.time), &(rhs.time), !=);
}


inline WFTime operator*(const WFTime& lhs, float rhs)
{
	WFTime result = lhs;
	result.time.tv_sec *= rhs;
	result.time.tv_usec *= rhs;
	return result;
}


#endif /* end of include guard: WFTIME_20LZQV24 */


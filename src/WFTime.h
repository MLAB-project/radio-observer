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
#include "common_types.h"


#define MS_IN_SECOND 1000
#define US_IN_MS     1000
#define US_IN_SECOND (MS_IN_SECOND * US_IN_MS)


/**
 * \brief Represents both specific time and time interval.
 */
struct WFTime {
public:
	struct Info {
		int year;
		int month;
		int day;

		int hour;
		int min;
		int sec;
		
		Info(int year = 0, int month = 0, int day = 0, int hour = 0, int min = 0, int sec = 0) :
			year(year), month(month), day(day), hour(hour), min(min), sec(sec)
		{}
	};
	
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

	inline WFTime add(long seconds, long microseconds)
	{
		long usec = safeAdd(time.tv_usec, microseconds % (long)US_IN_SECOND);
		
		long sec = safeAdd((long)time.tv_sec, seconds);
		sec = safeAdd(sec, microseconds / (long)US_IN_SECOND);
		
		sec = safeAdd(sec, usec / (long)US_IN_SECOND);
		usec %= (long)US_IN_SECOND;
		
		return WFTime(sec, usec);
	}
	
	inline WFTime addSamples(SampleCount sampleCount, SampleRate sampleRate)
	{
		assert(sampleCount >= 0);
		
		SampleCount seconds = sampleCount / (SampleCount)sampleRate;
		SampleCount remainder = sampleCount % (SampleCount)sampleRate;
		long microseconds = (((double)remainder / (double)sampleRate) *
						 (double)US_IN_SECOND);
		
		return add(seconds, microseconds);
		
		//long microseconds = (((double)sampleCount / (double)sampleRate) *
		//				 (double)US_IN_SECOND);
		//return addMicroseconds(microseconds);
	}
	
	//inline WFTime subSamples(int sampleCount, int sampleRate)
	//{
	//	long microseconds = (((double)sampleCount / (double)sampleRate) *
	//					 (double)US_IN_SECOND);
	//	return subMicroseconds(microseconds);
	//}
	
	inline Info getInfo(bool local = false)
	{
		time_t timestamp = seconds();
		struct tm *info = local ? localtime(&timestamp) : gmtime(&timestamp);
		
		return Info(
			info->tm_year,
			info->tm_mon,
			info->tm_mday,
			info->tm_hour,
			info->tm_min,
			info->tm_sec
		);
	}
	
	/**
	 * \brief Returns the same time rounded down to an hour.
	 */
	inline WFTime getHour(bool local = false)
	{
		// Get time info structure
		time_t timestamp = seconds();
		struct tm *timeInfo = local ? localtime(&timestamp) : gmtime(&timestamp);
		
		// Remove minute and second information
		timeInfo->tm_min = 0;
		timeInfo->tm_sec = 0;
		
		// Return the result
		timestamp = mktime(timeInfo);
		return WFTime(timestamp, 0);
	}
	
	/**
	 * \brief Formats the time using a format string.
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


inline bool operator==(const WFTime& lhs, const WFTime& rhs)
{
	return !(lhs != rhs);
}


inline WFTime operator*(const WFTime& lhs, float rhs)
{
	WFTime result = lhs;
	result.time.tv_sec *= rhs;
	result.time.tv_usec *= rhs;
	return result;
}


#endif /* end of include guard: WFTIME_20LZQV24 */


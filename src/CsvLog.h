/**
 * \file   CsvLog.h
 * \author Jan Milik <milikjan@fit.cvut.cz>
 * \date   2014-09-15
 *
 * \brief  Header file for the CsvLog class.
 */

#ifndef CSVLOG_27EUBFIA
#define CSVLOG_27EUBFIA


#include <string>
#include <fstream>
#include <sstream>

using namespace std;


#include <cppapp/cppapp.h>

using namespace cppapp;


#include "WFTime.h"


/**
 * \brief CSV logging class.
 */
class CsvLog : public Object {
private:
	CsvLog(const CsvLog& other);
	
	string      fileNameFormat_;
	string      header_;
	
	Ref<Output> output_;
	// Mutex       mutex_;
	
	Ref<Output> getOutput(WFTime);
	ostream&    getStream(WFTime);
	
public:
	/**
	 * Constructor.
	 */
	CsvLog(string fileNameFormat, string header);
	/**
	 * Destructor.
	 */
	virtual ~CsvLog() {}
	
	string getFileName(WFTime time);
	
	void write(WFTime time, string entry);
};


#define CSV_LOG_ENTRY(log, time, entry) { \
	WFTime t__ = (time); \
	std::stringstream buffer__; \
	(buffer__ << entry); \
	buffer__.flush(); \
	(log)->write(t__, buffer__.str()); \
}


#endif /* end of include guard: CSVLOG_27EUBFIA */


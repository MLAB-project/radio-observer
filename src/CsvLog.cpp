/**
 * \file   CsvLog.cpp
 * \author Jan Milik <milikjan@fit.cvut.cz>
 * \date   2014-09-15
 * 
 * \brief  Implementation file for the CsvLog class.
 */

#include "CsvLog.h"


Ref<Output> CsvLog::getOutput(WFTime time)
{
	string fileName = getFileName(time);
	if (output_.isNull() || fileName != output_->getName()) {
		output_ = new FileOutput(fileName);
		*(output_->getStream()) << "# " << header_ << std::endl;
		output_->getStream()->flush();
	}
	
	return output_;
}


ostream& CsvLog::getStream(WFTime time)
{
	return *(getOutput(time)->getStream());
}


CsvLog::CsvLog(string fileNameFormat, string header) :
	fileNameFormat_(fileNameFormat),
	header_(header)
{
}


string CsvLog::getFileName(WFTime time)
{
	return time.getHour().format(fileNameFormat_.c_str());
}


void CsvLog::write(WFTime time, string entry)
{
	// MutexLock lock(&mutex_);
	
	ostream& stream = *(getOutput(time)->getStream());
	stream << entry << std::endl;
	stream.flush();
}



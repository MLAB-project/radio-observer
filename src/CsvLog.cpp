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
		bool fileExists = FileInfo::exists(fileName);
		output_ = new FileOutput(fileName, ios_base::out | ios_base::binary | ios_base::app);
		if (!fileExists)
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
	return time.getHour(true).format(fileNameFormat_.c_str());
}


void CsvLog::write(WFTime time, string entry)
{
	MutexLock lock(&mutex_);
	
	ostream& stream = getStream(time);
	stream << entry << std::endl;
	stream.flush();
}



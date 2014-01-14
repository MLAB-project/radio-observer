/**
 * \file   FITSWriter.cpp
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-06-29
 * 
 * \brief  Implementation file for the FITSWriter class.
 */

#include "FITSWriter.h"


ostream& operator<<(ostream &output, const FITSStatus &status)
{
	output << status.value << " - " << status.getStatus();
	return output;
}


#define CHECK_STATUS(errorMsg) { \
	if (status_.isError() && (lastStatus_.value != status_.value)) { \
		LOG_ERROR(errorMsg << " (status: " << status_ << ")"); } \
	lastStatus_ = status_; \
}


FITSWriter::FITSWriter() :
	file_(NULL), status_(), lastStatus_(status_),
	dimCount_(0), dimensions_(NULL)
{ }


FITSWriter::~FITSWriter()
{
	delete [] dimensions_;
	dimCount_ = 0;
	dimensions_ = NULL;
}


bool FITSWriter::open(string fileName)
{
	fileName_ = fileName;

	fits_create_file(&file_, fileName_.c_str(), status_);
	CHECK_STATUS("Failed to open FITS file \"" << fileName_ << "\".");
	
	return status_.isOK();
}


void FITSWriter::close()
{
	fits_close_file(file_, status_);
	CHECK_STATUS("Failed to close FITS file \"" << fileName_ << "\".");
	
	file_       = NULL;
	status_     = FITSStatus();
	lastStatus_ = status_;
	
	dimCount_ = 0;
	delete [] dimensions_;
	dimensions_ = NULL;
}


void FITSWriter::createImage(long width, long height, int type)
{
	dimCount_ = 2;
	dimensions_ = new long[2];
	dimensions_[0] = width;
	dimensions_[1] = height;
	//long dimensions[2] = { width, height };
	fits_create_img(file_, type, 2, dimensions_, status_);
	CHECK_STATUS("Failed to create primary HDU in FITS file.");
}


void FITSWriter::writeHeader(const char *keyword,
					    int         type,
					    void       *value,
					    const char *comment)
{
	fits_write_key(file_, type, keyword, value, comment, status_);
	CHECK_STATUS("Failed to write FITS header \"" << keyword << "\".");
}


void FITSWriter::writeHeader(const char *keyword,
					    const char *value,
					    const char *comment)
{
	char valueBuffer[FLEN_VALUE];
	strcpy(valueBuffer, value);
	
	writeHeader(keyword, TSTRING, (void*)valueBuffer, comment);
}


void FITSWriter::writeHeader(const char *keyword,
					    float       value,
					    const char *comment)
{
	writeHeader(keyword, TFLOAT, (void*)&value, comment);
}


void FITSWriter::writeHeader(const char *keyword,
					    int         value,
					    const char *comment)
{
	writeHeader(keyword, TINT, (void*)&value, comment);
}


void FITSWriter::comment(const char *value)
{
	fits_write_comment(file_, value, status_);
	CHECK_STATUS("Failed to write FITS header comment (\"" << value << "\").");
}


void FITSWriter::date()
{
	fits_write_date(file_, status_);
	CHECK_STATUS("Failed to write FITS DATE header.");
}


void FITSWriter::write(long x, long y, long count, void *data, int type)
{
	long fpixel[2] = { x + 1, y + 1 };
	fits_write_pix(file_, type, fpixel, count, data, status_);
	CHECK_STATUS("Failed to write FITS pixel data.");
}


void FITSWriter::write(long y, long count, float *data)
{
	write(0, y, count * dimensions_[0], data, TFLOAT);
}


void FITSWriter::write(long y, long count, int16_t *data)
{
	write(0, y, count * dimensions_[0], data, TSHORT);
}


void FITSWriter::checkStatus(const char *errorMsg)
{
	CHECK_STATUS(errorMsg);
}



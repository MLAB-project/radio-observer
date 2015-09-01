/**
 * \file   FITSWriter.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-06-29
 *
 * \brief  Header file for the FITSWriter class.
 */

#ifndef FITSWRITER_N9AFZ3HN
#define FITSWRITER_N9AFZ3HN


#include <string>
using namespace std;

#include <fitsio.h>

#include <cppapp/cppapp.h>
using namespace cppapp;


/**
 * http://heasarc.gsfc.nasa.gov/fitsio/c/c_user/node28.html
 */
struct FITSStatus {
	int value;
	
	string getStatus() const
	{
		char buffer[FLEN_STATUS];
		fits_get_errstatus(value, buffer);
		return string(buffer);
	}
	
	bool getErrorMessage(string &msg) const
	{
		char buffer[FLEN_ERRMSG];
		
		if (fits_read_errmsg(buffer) == 0)
			return false;
		
		msg.assign(buffer);
		return true;
	}
	
	inline bool isOK() const { return value <= 0; } 
	inline bool isError() const { return !isOK(); }
	
	inline operator bool() { return isOK(); }
	inline operator int*() { return &value; }
};


ostream& operator<<(ostream &output, const FITSStatus &status);


/**
 * \brief Thin wrapper around some of FITSIO's FITS file writing functions.
 *
 * See http://heasarc.gsfc.nasa.gov/fitsio/ for more information on FITSIO
 * library. See http://heasarc.gsfc.nasa.gov/fitsio/c/c_user/ for FITSIO API
 * reference.
 */
class FITSWriter {
private:
	string      fileName_;
	
	fitsfile   *file_;
	FITSStatus  status_;
	FITSStatus  lastStatus_;
	
	int         dimCount_;
	long       *dimensions_;
	
	FITSWriter(const FITSWriter& other);
public:
	/**
	 * \brief Constructor.
	 *
	 * The constructor doesn't open any files. To open a file, call
	 * FITSWriter::open().
	 */
	FITSWriter();
	/**
	 * \brief Destructor.
	 */
	virtual ~FITSWriter();
	
	/**
	 * \brief Returns the status code of the last CFITSIO call.
	 */
	FITSStatus getStatus() const { return status_; }
	
	/**
	 * \brief Creates a new FITS file with specified file name.
	 *
	 * \param fileName file name of the created file
	 */
	bool open(string fileName);
	void close();
	void createImage(long width, long height, int type = FLOAT_IMG);
	
	void writeHeader(const char *keyword,
				  int         type,
				  void       *value,
				  const char *comment);
	
	void writeHeader(const char *keyword,
				  const char *value,
				  const char *comment);
	
	void writeHeader(const char *keyword,
				  float       value,
				  const char *comment);
	
	void writeHeader(const char *keyword,
				  double      value,
				  const char *comment);
	
	void writeHeader(const char *keyword,
				  int         value,
				  const char *comment);
	
	void writeHeader(const char *keyword,
				  long long   value,
				  const char *comment);

	void comment(const char *value);
	
	void date();
	
	/**
	 * \brief Write FITS pixel data to the current HDU.
	 *
	 * \note Unlike FITSIO API, this method uses zero-based indexing of the pixel position.
	 *
	 * \param x     Position of the first pixel in \c data in the first dimension.
	 * \param y     Position of the first pixel in \c data in the second dimension.
	 * \param count Number of pixels to write.
	 * \param data  Pointer to the data to write.
	 * \param type  Domain data type of the data (for example, \c TFLOAT for \c float).
	 */
	void write(long x, long y, long count, void *data, int type);
	void write(long y, long count, float *data);
	void write(long y, long count, int16_t *data);
	
	void checkStatus(const char *errorMsg);
};


#endif /* end of include guard: FITSWRITER_N9AFZ3HN */


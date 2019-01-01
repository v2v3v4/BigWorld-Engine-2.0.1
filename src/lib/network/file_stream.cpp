/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "cstdmf/bw_util.hpp"
#include "file_stream.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/bw_util.hpp"

DECLARE_DEBUG_COMPONENT2( "Network", 0 )

FileStream::FileStream( const char *path, const char *mode ) :
	path_( path ),
	mode_( mode ),
	readBuf_( new char[ INIT_READ_BUF_SIZE ] ),
	readBufSize_( INIT_READ_BUF_SIZE ),
	open_( false ),
	offset_( 0 ),
	it_( s_openFiles_.end() )
{
	this->open();
}

FileStream::~FileStream()
{
	this->close();
	delete [] readBuf_;
}

const char * FileStream::strerror() const
{
	if (errno)
		return ::strerror( errno );
	else
		return errorMsg_.c_str();
}

long FileStream::tell()
{
	if (!this->open())
		return -1;

	long result = ftell( file_ );
	if (result == -1)
		error_ = true;

	return result;
}

int FileStream::seek( long offset, int whence )
{
	if (!this->open())
		return -1;

	int result = fseek( file_, offset, whence );
	if (result == -1)
		error_ = true;

	return result;
}

/**
 *  Returns the size of this file on disk (i.e. doesn't include data not yet
 *  committed).
 */
long FileStream::length()
{
	if (!this->open())
		return -1;

	struct stat statinfo;
	if (fstat( bw_fileno( file_ ), &statinfo ))
	{
		error_ = true;
		return -1;
	}
	else
		return statinfo.st_size;
}

/**
 *  Using the output-streaming operators on FileStreams doesn't actually write
 *  the bytes to disk; it just inserts them into a memory buffer.  This method
 *  must be called to actually do the write (although all unwritten bytes are
 *  automatically committed to disk on destruction anyway).
 */
bool FileStream::commit()
{
	this->setMode( 'w' );
	if (!this->open())
		return false;

	errno = 0;

	if (fwrite( this->data(), 1, this->size(), file_ ) !=
		(size_t)this->size())
	{
		error_ = true;
		errorMsg_ = "Couldn't write all bytes to disk";
		return false;
	}

	if (fflush( file_ ) == EOF)
	{
		error_ = true;
		return false;
	}

	// Clear the internal memory buffer
	this->reset();
	return true;
}


const void * FileStream::retrieve( int nBytes )
{
	this->setMode( 'r' );

	if (readBufSize_ < nBytes)
	{
		delete [] readBuf_;
		readBuf_ = new char[ nBytes ];
		readBufSize_ = nBytes;
	}

	if (!this->open())
		return readBuf_;

	errno = 0;

	if (fread( readBuf_, 1, nBytes, file_ ) != (size_t)nBytes)
	{
		error_ = true;
		errorMsg_ = "Couldn't read desired number of bytes from disk";
	}

	return readBuf_;
}

int FileStream::stat( struct stat *statinfo )
{
	if (!this->open())
		return -1;

	return fstat( bw_fileno( file_ ), statinfo );
}

/**
 *  Prepares this stream for I/O operations in the specified mode (either 'r' or
 *  'w').  This is necessary due to the ANSI C requirement that file positioning
 *  operations are interleaved between reads and writes, and vice versa.
 */
void FileStream::setMode( char mode )
{
	if (!this->open())
		return;

	if ((mode == 'w' && lastAction_ == 'r') ||
		 (mode == 'r' && lastAction_ == 'w'))
	{
		error_ |= fseek( file_, 0, SEEK_CUR ) == -1;
	}

	lastAction_ = mode;
}

bool FileStream::open()
{
	// If we're already at the front of the open handles queue, just return now.
	// This hopefully makes repeated accesses on the same file a bit faster
	if (!s_openFiles_.empty() && s_openFiles_.front() == this)
		return true;

	// If already open but not at the front of the queue, remove the existing
	// reference to this file from the queue
	else if (open_)
	{
		this->remove();
	}

	// Otherwise, really open up the file again
	else
	{
		if ((file_ = bw_fopen( path_.c_str(), mode_.c_str() )) == NULL)
		{
			error_ = true;
			return false;
		}

		lastAction_ = 0;
		open_ = true;

		// Restore old position if it was set
		if (offset_)
		{
			if (fseek( file_, offset_, SEEK_SET ) == -1)
			{
				error_ = true;
				return false;
			}
		}
	}

	// Each time we call this, this file moves to the front of the queue
	s_openFiles_.push_front( this );
	it_ = s_openFiles_.begin();

	// If there are too many files open, purge the least-recently used one
	if (s_openFiles_.size() > MAX_OPEN_FILES)
	{
		s_openFiles_.back()->close();
	}

	return true;
}

void FileStream::close()
{
	if (!open_)
		return;

	if (pCurr_ != pBegin_)
		this->commit();

	// Save file offset in case we do an open() later on.  We can't use
	// this->tell() here because that would cause an open()
	offset_ = ftell( file_ );
	fclose( file_ );
	open_ = false;

	// Remove this object from the handle queue
	this->remove();
}

void FileStream::remove()
{
	s_openFiles_.erase( it_ );
	it_ = s_openFiles_.end();
}

FileStream::FileStreams FileStream::s_openFiles_;

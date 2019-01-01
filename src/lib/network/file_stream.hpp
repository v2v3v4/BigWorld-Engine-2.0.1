/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FILE_STREAM_HPP
#define FILE_STREAM_HPP

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <list>
#include "cstdmf/memory_stream.hpp"

/**
 *  This class provides a file stream that is compliant with the stream formats
 *  imposed by BinaryStreams.  Unfortunately, the similar class in cstdmf
 *  doesn't use the same format for packing string lengths and therefore
 *  couldn't be used where this exact stream format is required.
 *
 *  It provides automatic limiting of the number of
 *  simultaneously open file handles via an efficient most-recently-used caching
 *  system.
 *
 *  I/O errors encountered during disk operations should be checked for by
 *  calling good() or error() after any call to an I/O operation.
 */
class FileStream : public MemoryOStream
{
public:
	static const int INIT_READ_BUF_SIZE = 128;

	FileStream( const char *path, const char *mode );
	virtual ~FileStream();

	inline bool good() const { return !error_; }
	inline bool error() const { return error_; }
	const char * strerror() const;

	long tell();
	int seek( long offset, int whence = SEEK_SET );
	long length();
	bool commit();
	virtual const void *retrieve( int nBytes );
	int stat( struct stat *statinfo );

protected:
	void setMode( char mode );
	bool open();
	void close();
	void remove();

	std::string path_;
	std::string mode_;
	FILE *file_;
	char *readBuf_;
	int readBufSize_;
	std::string errorMsg_;

	// This is either 0, 'r', or 'w'
	char lastAction_;

	// Whether or not the underlying FILE* is actually open or not
	bool open_;

	// The file offset as at the last time close() was called
	long offset_;

	// Static management of maximum number of open FileStream handles
	typedef std::list< FileStream* > FileStreams;
	static FileStreams s_openFiles_;
	static const unsigned MAX_OPEN_FILES = 20;

	// An iterator pointing to this FileStream's position in s_openFiles_
	FileStreams::iterator it_;
};

#endif

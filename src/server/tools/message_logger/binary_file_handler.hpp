/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BINARY_FILE_HANDLER_HPP
#define BINARY_FILE_HANDLER_HPP

#include "file_handler.hpp"

#include "network/file_stream.hpp"

/**
 * Wraps binary-format files that are accessed via a FileStream.
 */
class BinaryFileHandler : public FileHandler
{
public:
	BinaryFileHandler();
	virtual ~BinaryFileHandler();

	bool init( const char *path, const char *mode );

	virtual long length();

protected:
	FileStream *pFile_;
};

#endif // BINARY_FILE_HANDLER_HPP

/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TEXT_FILE_HANDLER_HPP
#define TEXT_FILE_HANDLER_HPP

#include "file_handler.hpp"

#include <stdlib.h>

/**
 * Wraps line-based ASCII files.
 */
class TextFileHandler : public FileHandler
{
public:
	TextFileHandler();
	virtual ~TextFileHandler();

	bool init( const char *filename, const char *mode );
	bool close();

	virtual long length();

	virtual bool read();
	virtual bool handleLine( const char *line ) = 0;

	bool writeLine( const char *line );

protected:
	FILE *fp_;
};

#endif // TEXT_FILE_HANDLER_HPP

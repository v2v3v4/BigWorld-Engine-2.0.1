/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef UNARY_INTEGER_FILE_HPP
#define UNARY_INTEGER_FILE_HPP

#include "text_file_handler.hpp"

#include <stdlib.h>

/**
 * A file containing a single number represented in ascii.  The version file
 * and the uid file in each user log directory use this.
 */
class UnaryIntegerFile: public TextFileHandler
{
public:
	UnaryIntegerFile();

	using TextFileHandler::init;
	bool init( const char *path, const char *mode, int v );
	virtual bool handleLine( const char *line );
	virtual void flush();
	bool set( int v );

	int getValue() const;

	bool deleteFile();

protected:
	int v_;
};

#endif // UNARY_INTEGER_FILE_HPP

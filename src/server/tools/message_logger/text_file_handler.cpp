/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "text_file_handler.hpp"

#include "cstdmf/debug.hpp"

#include <errno.h>
#include <string.h>


TextFileHandler::TextFileHandler() :
	fp_( NULL )
{ }


TextFileHandler::~TextFileHandler()
{
	if (fp_)
		fclose( fp_ );
}


bool TextFileHandler::init( const char *filename, const char *mode )
{
	if (fp_ != NULL)
	{
		ERROR_MSG( "TextFileHandler::init: Unable to re-init, a file "
			"is already open\n" );
		return false;
	}

	fp_ = fopen( filename, mode );
	if (fp_ == NULL)
	{
		ERROR_MSG( "TextFileHandler::init: Unable to open file "
			"'%s' in mode '%s': %s\n", filename, mode, strerror( errno ) );
		return false;
	}

	return FileHandler::init( filename, mode );
}


bool TextFileHandler::close()
{
	if (fp_ == NULL)
	{
		ERROR_MSG( "TextFileHandler::close: Unable to close invalid file "
			"handle (%s)\n", this->filename() );
		return false;
	}

	fclose( fp_ );
	fp_ = NULL;
	return true;
}


bool TextFileHandler::read()
{
	char *line = NULL;
	size_t len = 0;
	bool status = true;

	if (fseek( fp_, 0, 0 ))
	{
		ERROR_MSG( "TextFileHandler::read: Unable to seek to start of file "
			"'%s': %s\n", filename_.c_str(), strerror( errno ) );
		return false;
	}

	while (getline( &line, &len, fp_ ) != -1)
	{
		// Chomp where necessary
		size_t slen = strlen( line );
		if (line[ slen-1 ] == '\n')
		{
			line[ slen-1 ] = '\0';
		}

		if (!this->handleLine( line ))
		{
			status = false;
			ERROR_MSG( "TextFileHandler::read: "
				"Aborting due to failure in handleLine()\n" );
			break;
		}
	}

	if (line)
	{
		free( line );
	}

	return status;
}


long TextFileHandler::length()
{
	if (!fp_)
	{
		return 0;
	}

	long pos = ftell( fp_ );
	fseek( fp_, 0, SEEK_END );

	long currsize = ftell( fp_ );
	fseek( fp_, pos, SEEK_SET );

	return currsize;
}


bool TextFileHandler::writeLine( const char *line )
{
	if (mode_.find( 'r' ) != std::string::npos)
	{
		ERROR_MSG( "TextFileHandler::writeLine: "
			"Can't write to file %s in mode '%s'\n",
			filename_.c_str(), mode_.c_str() );
		return false;
	}

	if (fprintf( fp_, "%s\n", line ) == -1)
	{
		ERROR_MSG( "TextFileHandler::writeLine: "
			"Unable to write line '%s' to file %s: %s\n",
			line, filename_.c_str(), strerror(errno) );
		return false;
	}

	fflush( fp_ );
	return true;
}

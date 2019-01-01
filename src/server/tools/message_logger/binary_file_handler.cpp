/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "binary_file_handler.hpp"


/**
 * Constructor.
 */
BinaryFileHandler::BinaryFileHandler() :
	pFile_( NULL )
{ }


/**
 * Destructor.
 */
BinaryFileHandler::~BinaryFileHandler()
{
	if (pFile_)
	{
		delete pFile_;
	}
}


/**
 * Initialises the BinaryFileHandler with a new FileStream.
 */
bool BinaryFileHandler::init( const char *path, const char *mode )
{
	if ( pFile_ != NULL )
	{
		ERROR_MSG( "BinaryFileHandler::init: Already initialised.\n" );
		return false;
	}

	pFile_ = new FileStream( path, mode );
	if (pFile_->error())
	{
		ERROR_MSG( "BinaryFileHandler::init: "
			"Unable to open '%s' in mode %s: %s\n",
			path, mode, pFile_->strerror() );
		return false;
	}

	return this->FileHandler::init( path, mode );
}


/**
 * Returns the length of the currently open file.
 *
 * @returns Current file length on success, -1 on error.
 */
long BinaryFileHandler::length()
{
	if (pFile_ == NULL)
	{
		return -1;
	}

	return pFile_->length();
}

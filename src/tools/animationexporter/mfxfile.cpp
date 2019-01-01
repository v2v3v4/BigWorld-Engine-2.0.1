/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#include "mfxexp.hpp"
#include "mfxfile.hpp"

#ifndef CODE_INLINE
#include "mfxfile.ipp"
#endif

MFXFile::MFXFile()
: stream_( NULL )
{
}

MFXFile::~MFXFile()
{
}

bool MFXFile::openForOutput( const std::string &filename )
{
	std::string loweredFilename = filename;
	int len = loweredFilename.length();
	if (len>4)
	{
		for (int i=len-3;i<len;i++)
		{
			loweredFilename[i] = tolower(loweredFilename[i]);
		}
	}

	stream_ = fopen( loweredFilename.c_str(), "wb" );
	if( stream_ )
		return true;
	return false;
}

bool MFXFile::openForInput( const std::string & filename )
{
	stream_ = fopen( filename.c_str(), "rb" );
	return stream_ != NULL;
}

bool MFXFile::close( void )
{
	if( stream_ )
	{
		if( 0 == fclose( stream_ ) )
			return true;
	}

	return false;
}


std::ostream& operator<<(std::ostream& o, const MFXFile& t)
{
	o << "MFXFile\n";
	return o;
}


/*mfx.cpp*/

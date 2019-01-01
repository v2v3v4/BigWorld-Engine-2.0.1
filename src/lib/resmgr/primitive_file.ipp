/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// primitive_file.ipp

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

#if 0	// ifdef'd out since functionality moved to BinSection
INLINE void PrimitiveFile::deleteBinary( const std::string & name )
{
	this->updateBinary( name, NULL );
}
#endif

// primitive_file.ipp

/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// bwresource.ipp

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

#ifdef EDITOR_ENABLED
// wide versions, editor-only
//-----------------------------------------------------------------------------
INLINE const std::wstring BWResource::getFilenameW( const std::string& file )
{
	return bw_utf8tow( getFilename( file ) );
}


//-----------------------------------------------------------------------------
INLINE const std::wstring BWResource::getFilenameW( const std::wstring& file )
{
	return bw_utf8tow( getFilename( bw_wtoutf8( file ) ) );
}


//-----------------------------------------------------------------------------
INLINE const std::wstring BWResource::getExtensionW( const std::string& file )
{
	return bw_utf8tow( getExtension( file ) );
}

//-----------------------------------------------------------------------------

INLINE const std::wstring BWResource::getExtensionW( const std::wstring& file )
{
	return bw_utf8tow( getExtension( bw_wtoutf8( file ) ) );
}


//-----------------------------------------------------------------------------
INLINE const std::wstring BWResource::removeExtensionW( const std::string& file )
{
	return bw_utf8tow( removeExtension( file ) );
}


//-----------------------------------------------------------------------------
INLINE const std::wstring BWResource::removeExtensionW( const std::wstring& file )
{
	return bw_utf8tow( removeExtension( bw_wtoutf8( file ) ) );
}


//-----------------------------------------------------------------------------
INLINE bool BWResource::validPathW( const std::wstring& file )
{
	return validPath( bw_wtoutf8( file ) );
}


//-----------------------------------------------------------------------------
INLINE void BWResource::defaultDriveW( const std::wstring& drive )
{
	return defaultDrive( bw_wtoutf8( drive ) );
}


//-----------------------------------------------------------------------------
INLINE const std::wstring BWResource::findFileW( const std::string& file )
{
	return bw_utf8tow( findFile( file ) );
}


//-----------------------------------------------------------------------------
INLINE const std::wstring BWResource::removeDriveW( const std::string& file )
{
	return bw_utf8tow( removeDrive( file ) );
}


//-----------------------------------------------------------------------------
INLINE const std::wstring BWResource::dissolveFilenameW( const std::string& file )
{
	return bw_utf8tow( dissolveFilename( file ) );
}


//-----------------------------------------------------------------------------
INLINE const std::wstring BWResource::resolveFilenameW( const std::string& file )
{
	return bw_utf8tow( resolveFilename( file ) );
}


//-----------------------------------------------------------------------------
INLINE const std::wstring BWResource::findFileW( const std::wstring& file )
{
	return bw_utf8tow( findFile( bw_wtoutf8( file ) ) );
}


//-----------------------------------------------------------------------------
INLINE const std::wstring BWResource::removeDriveW( const std::wstring& file )
{
	return bw_utf8tow( removeDrive( bw_wtoutf8( file ) ) );
}


//-----------------------------------------------------------------------------
INLINE const std::wstring BWResource::dissolveFilenameW( const std::wstring& file )
{
	return bw_utf8tow( dissolveFilename( bw_wtoutf8( file ) ) );
}


//-----------------------------------------------------------------------------
INLINE const std::wstring BWResource::resolveFilenameW( const std::wstring& file )
{
	return bw_utf8tow( resolveFilename( bw_wtoutf8( file ) ) );
}

#endif // EDITOR_ENABLED

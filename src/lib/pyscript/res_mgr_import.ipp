/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

/**
 *	Retrieve the least significant path component of a fully qualified module
 *	path.
 *
 *	@param fullModuleName	The fully qualified module name. If this is not a
 *							fully qualified module name, then a copy of this
 *							string is returned without change.
 */

/*static*/ inline const std::string PyResMgrImportLoader::getShortModuleName( 
		const std::string & fullModuleName )
{
	std::string::size_type dotPos = fullModuleName.rfind( "." );

	if (dotPos != std::string::npos)
	{
		return fullModuleName.substr( dotPos + 1 );
	}
	return fullModuleName;
}

// res_mgr_import.ipp

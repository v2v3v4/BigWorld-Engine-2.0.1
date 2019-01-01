/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PATHED_FILENAME__HPP
#define PATHED_FILENAME__HPP

#include "resmgr/datasection.hpp"

#include <string>

/**
 *	This is a helper class which assembles an absolute system filename
 *	based on a static filename and a dynamically detemermined base name
 *	(e.g. executable path or application data path).
 */
class PathedFilename
{
public:
	enum PathBase
	{
		BASE_APP_DATA,		// The user's application data folder.
		BASE_MY_DOCS,		// The user's My Documents folder.
		BASE_CWD,			// The current working directory.
		BASE_EXE_PATH,		// The directory containing the executable.
		BASE_RES_TREE		// The first res specified in paths.xml.
	};

public:
	PathedFilename();
	PathedFilename( const std::string& filename, PathBase base );
	PathedFilename( DataSectionPtr ds, 
					const std::string& defaultFilename, 
					PathBase defaultBase );

	void init( DataSectionPtr ds, 
				const std::string& defaultFilename, 
				PathBase defaultBase );

	std::string resolveName() const;

	static std::string resolveBase( PathBase baseType );

private:
	std::string filename_;
	PathBase base_;
};

#endif // PATHED_FILENAME__HPP

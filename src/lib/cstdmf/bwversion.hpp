/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BWVERSION_HPP
#define BWVERSION_HPP

#define BW_VERSION_MAJOR 2
#define BW_VERSION_MINOR 0
#define BW_VERSION_PATCH 0

#define BW_COPYRIGHT_NOTICE "(c) 1999 - 2010, BigWorld Pty. Ltd. All Rights Reserved"

#define STRINGIFY(X) #X
#define TOSTRING(X) STRINGIFY(X)

#define BW_VERSION_MSVC_RC BW_VERSION_MAJOR, BW_VERSION_MINOR, BW_VERSION_PATCH
#define BW_VERSION_MSVC_RC_STRING TOSTRING( BW_VERSION_MAJOR.BW_VERSION_MINOR.BW_VERSION_PATCH )

#ifndef RC_INVOKED

#include <string>

#include "stdmf.hpp"

namespace BWVersion
{
	uint16 majorNumber();
	uint16 minorNumber();
	uint16 patchNumber();
	
	void majorNumber( uint16 number );
	void minorNumber( uint16 number );
	void patchNumber( uint16 number );

	const std::string & versionString();
}

#endif // RC_INVOKED

#endif // BWVERSION_HPP

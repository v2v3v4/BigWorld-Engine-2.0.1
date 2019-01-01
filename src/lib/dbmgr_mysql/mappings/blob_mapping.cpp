/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "blob_mapping.hpp"

#include "cstdmf/base64.h"
#include "resmgr/datasection.hpp"

/**
 *	Constructor.
 */
BlobMapping::BlobMapping( const Namer & namer, const std::string & propName,
		bool isNameIndex, int length, DataSectionPtr pDefaultValue ) :
	StringLikeMapping( namer, propName, isNameIndex, length )
{
	if (pDefaultValue)
	{
		BlobMapping::decodeSection( defaultValue_, pDefaultValue );

		if (defaultValue_.length() > charLength_)
		{
			defaultValue_.resize( charLength_ );
			ERROR_MSG( "BlobMapping::BlobMapping: "
						"Default value for property %s has been truncated\n",
					propName.c_str() );
		}
	}
}


/**
 *	This method gets the section data as a base64 encoded string and decodes it,
 *	placing the result in output.
 */
void BlobMapping::decodeSection( std::string & output, DataSectionPtr pSection )
{
	output = pSection->as<std::string>();
	int len = output.length();
	if (len <= 256)
	{
		// Optimised for small strings.
		char decoded[256];
		int length = Base64::decode( output, decoded, 256 );
		output.assign(decoded, length);
	}
	else
	{
		char * decoded = new char[ len ];
		int length = Base64::decode( output, decoded, len );
		output.assign(decoded, length);
		delete [] decoded;
	}
}

// blob_mapping.cpp

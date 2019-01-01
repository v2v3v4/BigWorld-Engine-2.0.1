/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef COMPRESSION_TYPE_HPP
#define COMPRESSION_TYPE_HPP

#include "resmgr/datasection.hpp"

enum BWCompressionType
{
	BW_COMPRESSION_NONE,

	BW_COMPRESSION_ZIP_1,
	BW_COMPRESSION_ZIP_2,
	BW_COMPRESSION_ZIP_3,
	BW_COMPRESSION_ZIP_4,
	BW_COMPRESSION_ZIP_5,
	BW_COMPRESSION_ZIP_6,
	BW_COMPRESSION_ZIP_7,
	BW_COMPRESSION_ZIP_8,
	BW_COMPRESSION_ZIP_9,

	BW_COMPRESSION_ZIP_BEST_SPEED       = BW_COMPRESSION_ZIP_1,
	BW_COMPRESSION_ZIP_BEST_COMPRESSION = BW_COMPRESSION_ZIP_9,

	BW_COMPRESSION_DEFAULT_INTERNAL,
	BW_COMPRESSION_DEFAULT_EXTERNAL,
};


// Implemented in compression_stream.cpp
bool initCompressionTypes( DataSectionPtr pSection,
			BWCompressionType & internalCompressionType,
			BWCompressionType & externalCompressionType );
bool initCompressionType( DataSectionPtr pSection,
			BWCompressionType & compressionType );

#endif // COMPRESSION_TYPE_HPP

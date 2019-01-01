/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef META_DATA_CREATION_UTILITY_HPP
#define META_DATA_CREATION_UTILITY_HPP

#include "cstdmf/string_utils.hpp"
#include "cstdmf/guard.hpp"
#include "resmgr/datasection.hpp"
#include "meta_data_define.hpp"
#include <time.h>

namespace MetaData
{

inline void updateCreationInfo( DataSectionPtr dsRoot )
{
	BW_GUARD;

	time_t time = ::time( NULL );
	static std::string username;

	if (username.empty())
	{
		char buf[ 1024 ] = { 0 };
		DWORD size = sizeof( buf ) / sizeof( buf[0] );

		GetUserNameA( buf, &size );
		username = buf;
	}

	DataSectionPtr metaDS = dsRoot->openSection( METADATA_SECTION_NAME, true );

	if (!metaDS->openSection( METADATA_CREATED_BY ) ||
		!metaDS->openSection( METADATA_CREATED_ON ))
	{
		metaDS->writeString( METADATA_CREATED_BY, username );
		metaDS->writeInt64( METADATA_CREATED_ON, time );
	}

	metaDS->writeString( METADATA_MODIFIED_BY, username );
	metaDS->writeInt64( METADATA_MODIFIED_ON, time );
}

}


#endif//META_DATA_CREATION_UTILITY_HPP

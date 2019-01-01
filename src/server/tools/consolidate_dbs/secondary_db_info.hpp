/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONSOLIDATE_DBS_SECONDARY_DB_INFO_HPP
#define CONSOLIDATE_DBS_SECONDARY_DB_INFO_HPP

#include "cstdmf/stdmf.hpp"

#include <string>
#include <vector>

/**
 *	Stores information about the location of a secondary DB.
 */
class SecondaryDBInfo
{
public:
	SecondaryDBInfo() : 
			hostIP( 0 ), 
			location()
	{}

	SecondaryDBInfo( uint32 ip, const std::string & path ) :
		hostIP( ip ),
		location( path )
	{}
	
	uint32		hostIP;
	std::string location;
};

typedef std::vector< SecondaryDBInfo > SecondaryDBInfos;

#endif // CONSOLIDATE_DBS_SECONDARY_DB_INFO_HPP

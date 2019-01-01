/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "server/server_app_config.hpp"

class DBMgrConfig : public ServerAppConfig
{
public:
	static ServerAppOption< bool > allowEmptyDigest;
	static ServerAppOption< int > dumpEntityDescription;
	static ServerAppOption< uint32 > desiredBaseApps;
	static ServerAppOption< uint32 > desiredCellApps;
	static ServerAppOption< float > overloadLevel;
	static ServerAppOption< float > overloadTolerancePeriod;

	static ServerAppOption< std::string > type;

	static uint64 overloadTolerancePeriodInStamps()
	{
		return secondsToStamps( overloadTolerancePeriod() );
	}

	static bool postInit();
};

// dbmgr_config.hpp

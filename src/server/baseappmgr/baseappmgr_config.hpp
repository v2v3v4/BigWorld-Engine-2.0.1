/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "server/manager_app_config.hpp"

class BaseAppMgrConfig : public ManagerAppConfig
{
public:
	static ServerAppOption< float > baseAppOverloadLevel;
	static ServerAppOption< float > createBaseRatio;
	static ServerAppOption< float > updateCreateBaseInfoPeriod;
	static ServerAppOption< int > updateCreateBaseInfoPeriodInTicks;

	static ServerAppOption< bool > hardKillDeadBaseApps;

	static ServerAppOption< float > baseAppTimeout;
	static ServerAppOption< uint64 > baseAppTimeoutInStamps;

	static ServerAppOption< float > overloadTolerancePeriod;
	static ServerAppOption< uint64 > overloadTolerancePeriodInStamps;

	static ServerAppOption< int > overloadLogins;

	static bool postInit();
};

// baseappmgr_config.hpp

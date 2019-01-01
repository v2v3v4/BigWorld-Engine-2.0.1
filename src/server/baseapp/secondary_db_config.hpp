/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SECONDARY_DB_CONFIG_HPP
#define SECONDARY_DB_CONFIG_HPP

#include "server/server_app_option.hpp"


/**
 *	This contains configuration options for secondary db.
 */
class SecondaryDBConfig
{
public:
	static ServerAppOption< bool > enable;
	static ServerAppOption< float > maxCommitPeriod;
	static ServerAppOption< uint > maxCommitPeriodInTicks;

	static ServerAppOption< std::string > directory;

	static bool postInit();

private:
	SecondaryDBConfig();
};

#endif // SECONDARY_DB_CONFIG_HPP

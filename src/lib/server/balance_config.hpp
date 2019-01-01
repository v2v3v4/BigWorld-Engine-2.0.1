/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BALANCE_CONFIG_HPP
#define BALANCE_CONFIG_HPP

#include "server_app_option.hpp"

/**
 *	This class contains the configuration options for an EntityApp.
 */
class BalanceConfig
{
public:
	static ServerAppOption< float > maxCPUOffload;
	static ServerAppOption< int > minEntityOffload;
	static ServerAppOption< float > minMovement;
	static ServerAppOption< float > slowApproachFactor;
	static ServerAppOption< bool > demo;
	static ServerAppOption< float > demoNumEntitiesPerCell;

	// static bool postInit();
};

#endif // BALANCE_CONFIG_HPP

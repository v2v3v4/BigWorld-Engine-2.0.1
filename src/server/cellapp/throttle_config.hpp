/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef THROTTLE_CONFIG_HPP
#define THROTTLE_CONFIG_HPP

#include "server/server_app_option.hpp"


/**
 *	This contains configuration options for noise.
 */
class ThrottleConfig
{
public:
	static ServerAppOption< float > behindThreshold;
	static ServerAppOption< float > spareTimeThreshold;
	static ServerAppOption< float > scaleForwardTime;
	static ServerAppOption< float > scaleBackTime;
	static ServerAppOption< float > min;

private:
	ThrottleConfig();
};

#endif // THROTTLE_CONFIG_HPP

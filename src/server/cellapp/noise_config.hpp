/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NOISE_CONFIG_HPP
#define NOISE_CONFIG_HPP

#include "server/server_app_option.hpp"


/**
 *	This contains configuration options for noise.
 */
class NoiseConfig
{
public:
	static ServerAppOption< float > standardRange;
	static ServerAppOption< float > verticalSpeed;
	static ServerAppOption< float > horizontalSpeed;
	static ServerAppOption< float > horizontalSpeedSqr;

	static bool postInit();

private:
	NoiseConfig();
};

#endif // NOISE_CONFIG_HPP

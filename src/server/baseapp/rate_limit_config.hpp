/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RATE_LIMIT_CONFIG_HPP
#define RATE_LIMIT_CONFIG_HPP

#include "server/server_app_option.hpp"


/**
 *	This contains configuration options for RateLimitMessageFilter.
 */
class RateLimitConfig
{
public:
	static ServerAppOption< uint > warnMessagesPerSecond;
	static ServerAppOption< uint > maxMessagesPerSecond;
	static ServerAppOption< uint > warnBytesPerSecond;
	static ServerAppOption< uint > maxBytesPerSecond;

	static ServerAppOption< uint > warnMessagesPerTick;
	static ServerAppOption< uint > maxMessagesPerTick;
	static ServerAppOption< uint > warnBytesPerTick;
	static ServerAppOption< uint > maxBytesPerTick;

	static ServerAppOption< uint > warnMessagesBuffered;
	static ServerAppOption< uint > maxMessagesBuffered;
	static ServerAppOption< uint > warnBytesBuffered;
	static ServerAppOption< uint > maxBytesBuffered;

	static bool postInit();

private:
	RateLimitConfig();
};

#endif // RATE_LIMIT_CONFIG_HPP

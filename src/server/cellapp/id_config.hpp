/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ID_CONFIG_HPP
#define ID_CONFIG_HPP

#include "server/server_app_option.hpp"


/**
 *	This contains configuration options for ids.
 */
class IDConfig
{
public:
	static ServerAppOption< int > criticallyLowSize;
	static ServerAppOption< int > lowSize;
	static ServerAppOption< int > desiredSize;
	static ServerAppOption< int > highSize;

private:
	IDConfig();
};

#endif // ID_CONFIG_HPP

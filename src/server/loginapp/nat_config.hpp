/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "server/server_app_option.hpp"

class NATConfig
{
public:
	static ServerAppOption< std::string > localNetMask;
	static ServerAppOption< std::string > externalAddress;
};

// nat_config.hpp

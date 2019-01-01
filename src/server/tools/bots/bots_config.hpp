/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SERVER__TOOLS__BOTS__BOTS_CONFIG_HPP
#define SERVER__TOOLS__BOTS__BOTS_CONFIG_HPP

#include "server/server_app_config.hpp"


class BotsConfig : public ServerAppConfig
{
public:
	static ServerAppOption< std::string > username;
	static ServerAppOption< std::string > password;
	static ServerAppOption< std::string > tag;
	static ServerAppOption< std::string > serverName;
	static ServerAppOption< uint16 > port;
	static ServerAppOption< bool > shouldUseRandomName;
	static ServerAppOption< bool > shouldUseScripts;
	static ServerAppOption< std::string > standinEntity;
	static ServerAppOption< std::string > controllerType;
	static ServerAppOption< std::string > controllerData;
	static ServerAppOption< std::string > publicKey;
	static ServerAppOption< std::string > loginMD5Digest;

	static bool postInit()
		{ return true; }
	
};

#endif // SERVER__TOOLS__BOTS__BOTS_CONFIG_HPP

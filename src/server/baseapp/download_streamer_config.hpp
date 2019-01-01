/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DOWNLOAD_STREAMER_CONFIG_HPP
#define DOWNLOAD_STREAMER_CONFIG_HPP

#include "server/server_app_option.hpp"


/**
 *	This contains configuration options for DownloadStreamer.
 */
class DownloadStreamerConfig
{
public:
	static ServerAppOption< int > bitsPerSecondTotal;
	static ServerAppOption< int > bitsPerSecondPerClient;
	static ServerAppOption< int > rampUpRate;
	static ServerAppOption< int > backlogLimit;

	static ServerAppOption< int > bytesPerTickTotal;
	static ServerAppOption< int > bytesPerTickPerClient;
	static ServerAppOption< int > rampUpRateBytesPerTick;

	static bool postInit();

private:
	DownloadStreamerConfig();
};

#endif // DOWNLOAD_STREAMER_CONFIG_HPP

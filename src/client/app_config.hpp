/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef APP_CONFIG_HPP
#define APP_CONFIG_HPP


#include "resmgr/datasection.hpp"
#include "resmgr/dataresource.hpp"

/**
 *	This class provides access to the datasection of application
 *	configuration settings.
 */
class AppConfig
{
public:
	AppConfig();
	~AppConfig();

	bool init( DataSectionPtr configSection );

	DataSectionPtr pRoot() const			{ return pRoot_; }

	static AppConfig & instance();

private:
	AppConfig( const AppConfig& );
	AppConfig& operator=( const AppConfig& );

	DataSectionPtr	pRoot_;
};


#ifdef CODE_INLINE
#include "app_config.ipp"
#endif

#endif // APP_CONFIG_HPP
